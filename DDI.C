#define INCL_DOSPROCESS
#define INCL_DOSQUEUES
#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSMEMMGR
#include <os2.h>

#undef KB_HEAP

#define  BYTE unsigned char

typedef unsigned long DWORD;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <time.h>
#include <errno.h>
#include <exe.h>
#include <exe386.h>

#define DDI_C
#include "ddi.h"

#define ALLOC_INC 512
#define STACKSIZE 8192 * 4
#define DELTA_TIME 60
#define DELTA_SRC  5

static ULONG AddName          (PDDI pDdi, PSZ pszName);
static int   DdiDdiLineInfo   (ULONG eip, PDDI pDdi, PSZ psz, int i);
       int   DdiIsCurrent     (PSZ pszFile, PSZ pszFileLater, ULONG fl);
       int   DdiPerror        (PSZ psz);

       void  thrDdiRead       (PVOID pVoid);
static int EDC0804            (PVOID p);

BOOL NEED_COMMIT(int size, int inc)
{
   int needed, and;

   needed = size;
   and    = needed & (PAGE_SIZE - 1);
   needed = (and  + inc >= PAGE_SIZE) ? TRUE : FALSE;
   return needed;

}

PVOID  mrMalloc       (int size)
{
   APIRET rc;
   PVOID  p;

   rc = DosAllocMem(&p, size, PAG_READ | PAG_WRITE);
   if (rc) {
      p = NULL;
   } else {
      rc = DosSetMem(p, PAGE_SIZE, PAG_DEFAULT | PAG_COMMIT);
   } 
   return p;
}

PVOID  mrAlloc       (int size)
{
   APIRET rc;
   PVOID  p;

   rc = DosAllocMem(&p, size, PAG_READ | PAG_WRITE | PAG_COMMIT);
   if (rc) {
      p = NULL;
   } 
   return p;
}


PVOID  mrRealloc     (PVOID p, int size)
{
//   APIRET rc;

//   rc =
   DosSetMem((PBYTE) p + size, PAGE_SIZE, PAG_DEFAULT | PAG_COMMIT);

   return p;
}

APIRET mrFree        (PVOID p)
{
   APIRET rc;
 
   rc = DosFreeMem(p);
   return rc;

}

PCHAR Extention(PSZ pszFile, PSZ pszExtention)
{
   PCHAR pch;

   pch = strrchr(pszFile, '.');
   if (pch && pszExtention) {
      strcpy(pch, pszExtention);
      pch = pszFile;
   }
   return pch;
}

int DdiPerror(PSZ psz)
{
   PSZ pszErrNo;

   if (errno) {
      if (*psz != '!') {
         pszErrNo = strerror(errno);
         pszErrNo[strlen(pszErrNo)] = 0;
         sprintf(&psz[strlen(psz)], ": %s", pszErrNo);
         printf("%s", psz);
      } else {
         printf("%s", psz);
      } 
   } else {
      printf("%s\n", psz);
   }
   return 0;
}

int DdiIsCurrent(PSZ pszFile, PSZ pszFileLater, ULONG fl)
{
   int         rc = FALSE;
   FILESTATUS3 stat;
   FILESTATUS3 statLater;
   time_t      tim;
   time_t      timLater;
   struct tm   tm;

   if (!DosQueryPathInfo(pszFile, 1, &stat, sizeof(stat))) {
      tm.tm_sec   = stat.ftimeLastWrite.twosecs * 2;
      tm.tm_min   = stat.ftimeLastWrite.minutes;
      tm.tm_hour  = stat.ftimeLastWrite.hours;
      tm.tm_mday  = stat.fdateLastWrite.day;
      tm.tm_mon   = --stat.fdateLastWrite.month;
      tm.tm_year  = stat.fdateLastWrite.year + 80;
      tm.tm_wday  = 0;
      tm.tm_yday  = 0;
      tm.tm_isdst = 0;
      tim = mktime(&tm);
      if (!DosQueryPathInfo(pszFileLater, 1, &statLater, sizeof(statLater))) {
         tm.tm_sec   = statLater.ftimeLastWrite.twosecs * 2;
         tm.tm_min   = statLater.ftimeLastWrite.minutes;
         tm.tm_hour  = statLater.ftimeLastWrite.hours;
         tm.tm_mday  = statLater.fdateLastWrite.day;
         tm.tm_mon   = --statLater.fdateLastWrite.month;
         tm.tm_year  = statLater.fdateLastWrite.year + 80;
         tm.tm_wday  = 0;
         tm.tm_yday  = 0;
         tm.tm_isdst = 0;
         timLater = mktime(&tm);
         timLater += DELTA_TIME;

         if (timLater >= tim) {
            rc = TRUE;
         } else {
            if (fl) {
               printf("\nddI:      %s out of date in regard to %s",
                  pszFileLater, pszFile);

            }
         }
      }
   }

   return rc;
}

int DdiLineInfo  (USHORT cLib, PLIB aLib, ULONG eip, PSZ psz, int fl)
{
   int   c;
   int   i;
   ULONG lSize;
   ULONG lAddr;
   int   ich;
   PDDI  pDdi;

   *psz = 0;
   ich  = 0;

   for (c = 0; c < cLib; c++) {
      if (!aLib[c].pDdi) {
         DdiRead(1, &aLib[c], DDI_READSYNC);
      }

      if (fl & DDI_DMPINFO && aLib[c].pDdi != DDI_LIBNOINFO) {
         if (aLib[c].pAddr) {
            if (eip >= aLib[c].pAddr[0].lAddr &&
                eip <= aLib[c].pAddr[0].lAddr + aLib[c].pAddr[0].lSize) {
               ich += DdiDdiLineInfo(eip - aLib[c].pAddr[0].lAddr,
                  (PDDI) aLib[c].pDdi, psz, ich);
               if (ich) {
                  break;
               }
            }
         }

      } else if (fl & DDI_DMPMODULE) {
         for (i = 0; i < aLib[c].cAddr; i++) {
            lSize = aLib[c].pAddr[i].lSize;
            while (lSize) {
               lSize -= 0x00010000;
               lAddr = aLib[c].pAddr[i].lAddr + lSize;
               if ((eip & 0xFFFF0000) == lAddr) {
                  ich += sprintf(psz + ich, "<%s>", aLib[c].sz);
                  break;
               }

            }
         }

      } else if (fl & DDI_MAP && aLib[c].pDdi != DDI_LIBNOINFO) {
         lAddr = eip & 0x0000FFFF;
         pDdi = (PDDI) aLib[c].pDdi;
         for (lSize = 0;
              lSize <= (pDdi->pLine + pDdi->cLine -1)->eip;
              lSize += 0x10000) {
            lAddr += lSize;
            ich += DdiDdiLineInfo(lAddr, (PDDI) aLib[c].pDdi, psz, ich);
         } 

      } else if (fl & DDI_DMPANYTHING) {
         if (aLib[c].pDdi != DDI_LIBNOINFO) {
            if (aLib[c].pAddr) {
               if (eip >= aLib[c].pAddr[0].lAddr &&
                   eip <= aLib[c].pAddr[0].lAddr + aLib[c].pAddr[0].lSize) {
                  ich += DdiDdiLineInfo(eip - aLib[c].pAddr[0].lAddr,
                     (PDDI) aLib[c].pDdi, psz, ich);
                  if (ich) {
                     break;
                  }
               }
            }
         }
         for (i = 0; i < aLib[c].cAddr; i++) {
            lSize = aLib[c].pAddr[i].lSize;
            while (lSize) {
               lSize -= 0x00010000;
               lAddr = aLib[c].pAddr[i].lAddr + lSize;
               if ((eip & 0xFFFF0000) == lAddr) {
                  ich += sprintf(psz + ich, "<%s>", aLib[c].sz);
                  break;
               }
            }
         }
      }
   }
   return ich;
}

static int DdiDdiLineInfo(ULONG eip, PDDI pDdi, PSZ psz, int i)
{
   int      rc    = FALSE;
   PBYVALUE pByValue;
   PSOURCE  pSource;
   PLINE    pLine;

   if (eip >= pDdi->pLine->eip &&
       eip <= (pDdi->pLine + pDdi->cLine - 1)->eip) {

      pByValue = pDdi->pByValue + pDdi->cByValue;
      while (--pByValue != pDdi->pByValue) {
         if (eip >= pByValue->eip) {
            break;
         }
      }

      pSource = pDdi->pSource + pDdi->cSource;
      while (--pSource != pDdi->pSource) {
         pLine = pDdi->pLine + pSource->iLineBegin;
         if (eip >= pLine->eip) {
            break;
         }
      }

      pLine = pDdi->pLine + pSource->iLineEnd;
      while (--pLine != pDdi->pLine + pSource->iLineBegin) {
         if (eip >= pLine->eip) {
            break;
         }
      }

      rc = TRUE;
   }
   if (rc) {
      i += sprintf(psz + i, "%s%4i :%-35s \"%s\"",
         i ? "\n" : "",
         pLine->iLine,
         pDdi->aszName[pSource->iName],
         pDdi->aszName[pByValue->iName]);
#ifdef PREP
//      i += DdiDdiSourceInfo(psz + i, pLine->iLine,
//         pDdi->aszName[pSource->iName]);
#endif
      rc = i;
   }
   return rc;
}

int DdiSourceInfo (PSZ pszInfo, PSZ pszFile)
{
   USHORT iLine;
   CHAR   szFile[MAXPATH];

   FILE * f;
   FILE * fSrc;
   int    i = 0;
   CHAR   sz[256];
   int    c = 0;
   int    iLineStart;
   BOOL   fDone;

   f = fopen(pszFile, "a");
   if (f) {
      i = sscanf(pszInfo, "%i :%s", &iLine, szFile);
      if (i == 2) {
         fSrc = fopen(szFile, "r");
         if (fSrc) {
            iLineStart = iLine - DELTA_SRC;
            fprintf(f, "><%s\n", pszInfo);

            fDone = FALSE;
            while (!fDone) {
               fgets(sz, 256, fSrc);
               if (feof(fSrc)) {
                  fDone = TRUE;
               } else {
                  c++;
                  if (c >= iLineStart) {
                     if (c == iLine) {
                        fprintf(f, "==%s", sz);
                     } else if (c < iLine) {
                        fprintf(f, "<<%s", sz);
                     } else if (c <= iLineStart + DELTA_SRC * 2) {
                        fprintf(f, ">>%s", sz);
                     } else {
                        fDone = TRUE;
                     }
                  }
               }
            }
            fprintf(f, "><\n");
            fclose(fSrc);
         }
      }
      fclose(f);
   }

   return 0;
}

int DdiRead(USHORT cLib, PLIB aLib, ULONG fl)
{
   int    c;
   APIRET rc;
   PTIB   ptib;
   PPIB   ppib;
   CHAR   szQ   [80];
   HQUEUE hQ;
   PID    pid;
   TID    tid = 0;
   BOOL   fQd = FALSE;
   HMTX   hmtx;
   CHAR   sz[512];
   PCHAR  psz;

   DosGetInfoBlocks(&ptib, &ppib);
   sprintf(szQ, SZ_DDI_Q, ppib->pib_ulpid);
   rc = DosOpenQueue(&pid, &hQ, szQ);
   if (rc == ERROR_QUE_NAME_NOT_EXIST) {
      rc = DosCreateQueue(&hQ, QUE_FIFO | QUE_NOCONVERT_ADDRESS, szQ);
      if (!rc) {
         tid = (TID) _beginthread(thrDdiRead, NULL, STACKSIZE, NULL);
         if (tid != (TID) -1) {
            rc = DosSetPriority(PRTYS_THREAD, PRTYC_IDLETIME,
               PRTYD_MAXIMUM, tid);
            fQd = TRUE;
         } else {
            rc = DosCloseQueue(hQ);
         }

      } 
   } else if (!rc) {
      fQd = TRUE;
   }

   for (c = 0;
        c < cLib && !aLib[c].pDdi && aLib[c].pDdi != DDI_LIBNOINFO;
        c++) {
      if (!fQd) {
         thrDdiRead(&aLib[c]);
      } else {
         if (fl & DDI_READASYNC) {
            hmtx = 0;
            sprintf(sz, SZ_DDI_SEM, ppib->pib_ulpid, ptib->tib_ptib2->tib2_ultid, aLib[c].sz);
            psz = sz;
            while (*psz) {
               if (*psz == ':') {
                  *psz = '_';
               }
               psz++;
            }
            rc = DosCreateEventSem(sz, &hmtx, 0, FALSE);
            if (!rc) {
               rc = DosWriteQueue(hQ, ptib->tib_ptib2->tib2_ultid, sizeof(PLIB), &aLib[c], 0);
            }
            if (rc) {
               rc = DosCloseEventSem(hmtx);
               thrDdiRead(&aLib[c]);
            } 
         } else {
            sprintf(sz, SZ_DDI_SEM, ppib->pib_ulpid, ptib->tib_ptib2->tib2_ultid, aLib[c].sz);
            psz = sz;
            while (*psz) {
               if (*psz == ':') {
                  *psz = '_';
               }
               psz++;
            }
            hmtx = 0;
            rc = DosOpenEventSem(sz, &hmtx);
            if (!rc) {
               rc = DosWaitEventSem(hmtx, SEM_INDEFINITE_WAIT);
            }
            if (rc) {
               thrDdiRead(&aLib[c]);
            }
            rc = DosCloseEventSem(hmtx);
            rc = DosCloseEventSem(hmtx);
         }
      } 
   }
//   if (fQd && !tid) {
//      rc = DosCloseQueue(hQ);
//   }

   return 0;
}

void thrDdiRead(PVOID pVoid)
{
   PLIB        pLib;
   PDDI        pDdi;
   APIRET      rc   = FALSE;
   int         i;
   FILE *      f;
   CHAR        sz   [512];
   LIB         lib;
   PSZ         pszExe;
   PCHAR       pchNameSpace;
   REQUESTDATA QData;
   ULONG       ul;
   HQUEUE      hQ   = 0;
   CHAR        szQ  [80];
   PTIB        ptib;
   PPIB        ppib;
   PID         pid;
   BYTE        bPrio;
   HMTX        hmtx;
   BOOL        fDone = FALSE;
   PCHAR       psz;

   while (!fDone) {
      if (pVoid) {
         pLib   = pVoid;
         pszExe = pLib->sz;
      } else {
         pLib = NULL;
         if (!hQ) {
            DosGetInfoBlocks(&ptib, &ppib);
            sprintf(szQ, SZ_DDI_Q, ppib->pib_ulpid);
            rc = DosOpenQueue(&pid, &hQ, szQ);
            if (rc) {
               sprintf(sz, "\nddI:      Could not open queue %s : %i", sz, rc);
               DdiPerror(sz);
            } 
         }
         rc = DosReadQueue(hQ, &QData, &ul, (PVOID) &pLib, 0, DCWW_WAIT, &bPrio, 0);
         if (rc) {
            sprintf(sz, "\nddI:      Could not read queue : %i", rc);
            DdiPerror(sz);
            pszExe = NULL;
         } else {
            pszExe = pLib->sz;
         }
      }
      if (pszExe) {
         pDdi = calloc(sizeof(DDI), 1);
         f = fopen(pszExe, "rb");
         if (!f) {
            if (!strstr(pszExe, ".ddI")) {
               sprintf(sz, "\nddI:     Could not read %s", pszExe);
               DdiPerror(sz);
            }
      
         } else {
            fseek(f, -sizeof(ICC_MAGIC) - sizeof(LONG), SEEK_END);
            fread(sz, sizeof(CHAR), sizeof(ICC_MAGIC), f);
            if (QUERY_ICC_MAGIC(sz)) {
               fseek(f, -sizeof(ICC_MAGIC), SEEK_CUR);
            } else {
               fseek(f, 0, SEEK_END);
            }
            fseek(f, -(sizeof(pDdi->szMagic2) + sizeof(pDdi->lOffset)), SEEK_CUR);
      
            fread(pDdi->szMagic2, sizeof(pDdi->szMagic2), 1, f);
            fread(&pDdi->lOffset,  sizeof(pDdi->lOffset),  1, f);
      
            if (QUERY_MAGIC(pDdi->szMagic2)) {
               fseek(f, -pDdi->lOffset, SEEK_CUR);
               fread(pDdi, sizeof(DDI), 1, f);
      
               if (QUERY_MAGIC(pDdi->szMagic1) && QUERY_MAGIC(pDdi->szMagic2)) {
      
      
                  pDdi->pByValue     = mrAlloc(pDdi->cByValue * sizeof(BYVALUE));
                  pDdi->pLine        = mrAlloc(pDdi->cLine    * sizeof(LINE));
                  pDdi->pSource      = mrAlloc(pDdi->cSource  * sizeof(SOURCE));
                  pDdi->aszName      = mrAlloc(pDdi->cName    * sizeof(PSZ *));
                  pDdi->pchNameSpace = mrAlloc(pDdi->cchNameSpace);

                  fread(pDdi->pByValue,     sizeof(BYVALUE), pDdi->cByValue,     f);
                  fread(pDdi->pLine,        sizeof(LINE),    pDdi->cLine,        f);
                  fread(pDdi->pSource,      sizeof(SOURCE),  pDdi->cSource,      f);
                  fread(pDdi->pchNameSpace, sizeof(CHAR),    pDdi->cchNameSpace, f);
      
                  pchNameSpace = pDdi->pchNameSpace;
                  for (i = 0; i < pDdi->cName; i++) {
                     pDdi->aszName[i] = ++pchNameSpace;
                     pchNameSpace += *--pchNameSpace;
                     pchNameSpace++;
                  }
                  rc = TRUE;
               } else {
                  free(pDdi);
                  pDdi = NULL;
               }
            }
            fclose(f);
         }
      
         if (rc == FALSE) {
            DdiFree(pDdi);
            pDdi = NULL;
      
            strcpy(sz, pszExe);
      
            if (!strstr(pszExe,  ".ddI")) {
               Extention(sz, ".ddI");
      
               if (DdiIsCurrent(pszExe, sz, 0)) {
                  strcpy(lib.sz, sz);
      
                  thrDdiRead(&lib);
                  pDdi = lib.pDdi;
      
               }
            }
            if (pDdi == NULL) {
               Extention(sz, ".Map");
               rc = (APIRET) DdiBuild(sz, sz, &pDdi, 0);
      
               if (rc) {
                  Extention(sz, ".ddI");
                  f = fopen(sz, "w");
                  if (f) {
                     fclose(f);
                  }
                  DdiWrite(sz, pDdi);
               }
            }
         }

         pLib->pDdi = pDdi;
      } 

      if (pVoid) {
         fDone = TRUE;
      } else {
         sprintf(sz, SZ_DDI_SEM, ppib->pib_ulpid, QData.ulData, pLib->sz);
         psz = sz;
         while (*psz) {
            if (*psz == ':') {
               *psz = '_';
            }
            psz++;
         }
         hmtx = 0;
         rc = DosOpenEventSem(sz, &hmtx);
         rc = DosPostEventSem(hmtx);
         rc = DosCloseEventSem(hmtx);
      }
   } 

   return;
}

int DdiWrite(PSZ pszExe, PDDI pDdi)
{
   int         rc = TRUE;
   CHAR        sz    [256];
   int         i;
   FILE *      f;
   CHAR        cch;
   LONG        lOffset;
   LONG        lOffsetICC = 0;
   BOOL        fWriteDDI;

   fWriteDDI = (BOOL) (strstr(pszExe, ".ddI") ? TRUE : FALSE);

   f = fopen(pszExe, fWriteDDI ? "wb" : "rb+");
   if (!f) {
      sprintf(sz, "\nddI:      Could not write %s", pszExe);
      DdiPerror(sz);

      rc = FALSE;
   } else {
      if (!fWriteDDI) {
         fseek(f, -sizeof(ICC_MAGIC) - sizeof(LONG), SEEK_END);
         fread(sz, sizeof(CHAR), sizeof(ICC_MAGIC), f);
         if (QUERY_ICC_MAGIC(sz)) {
            fread(&lOffsetICC, sizeof(LONG), 1, f);
         } 
      } 
      fseek(f, 0, SEEK_END);
      lOffset = ftell(f);

      fwrite(pDdi,           sizeof(DDI),     1,              f);
      fwrite(pDdi->pByValue, sizeof(BYVALUE), pDdi->cByValue, f);
      fwrite(pDdi->pLine,    sizeof(LINE),    pDdi->cLine,    f);
      fwrite(pDdi->pSource,  sizeof(SOURCE),  pDdi->cSource,  f);
      for (i = 0; i < pDdi->cName; i++) {
         cch = (CHAR) (strlen(pDdi->aszName[i]) + 1);
         fwrite(&cch, sizeof(cch), 1, f);
         fwrite(pDdi->aszName[i],  sizeof(CHAR), cch, f);
      }
      fwrite(pDdi->szMagic2, sizeof(pDdi->szMagic2), 1, f);
      pDdi->lOffset = ftell(f) + sizeof(pDdi->lOffset);
      pDdi->lOffset -= lOffset;
      fwrite(&pDdi->lOffset,  sizeof(pDdi->lOffset),  1, f);

      if (lOffsetICC) {
         fseek(f, 0, SEEK_END);
         lOffset = pDdi->lOffset + lOffsetICC;
         lOffset += (sizeof(ICC_MAGIC) + sizeof(LONG));
         fwrite(ICC_MAGIC, sizeof(CHAR), sizeof(ICC_MAGIC), f);
         fwrite(&lOffset, sizeof(LONG), 1, f);
      } 
      fclose(f);
   }
   if (rc == FALSE) {
      if (!fWriteDDI) {
         strcpy(sz, pszExe);
         Extention(sz, ".ddI");
         rc = DdiWrite(sz, pDdi);
         if (rc) {
            printf("\nddI:      Written to      %s", sz);
         }
      }
   }
   return rc;
}

int DdiBuild(PSZ pszExe, PSZ pszMap, PDDI * ppDdi, ULONG fl)
{
   PDDI        pDdi = NULL;
   FILE *      f;
   ULONG       i;
   int         c;
   CHAR        sz   [256];
   CHAR        szSrc[ 80];
   USHORT      sel;
   ULONG       eip;
   ULONG       iLine;
   USHORT      fFlags;
   ULONG       iSource;
   ULONG       iPrevSource;
   int         rc = TRUE;
   PSZ         psz;

   strcpy(sz, pszExe);
   Extention(sz, ".ddI");
   if (fl) {
      printf("Building  %s..", sz);
   }

   rc = DdiIsCurrent(pszExe, pszMap, fl);

   if (rc) {
      f = fopen(pszMap, "r");
      if (!f) {
         rc = FALSE;
      } else {
         pDdi               = calloc(  1, sizeof(DDI));
         pDdi->pByValue     = mrMalloc(ONE_MEG);
         pDdi->pLine        = mrMalloc(ONE_MEG);
         pDdi->pSource      = mrMalloc(ONE_MEG);
         pDdi->aszName      = mrMalloc(ONE_MEG);
         pDdi->pchNameSpace = mrMalloc(ONE_MEG);

      }
   }

   if (rc) {
      rc = FALSE;
      while (!feof(f)) {
         fgets(sz, 256, f);
         if (!strcmp(sz, "  Address         Publics by Value\n")) {
            fgets(sz, 256, f);

            rc = TRUE;

            break;
         }
      }
   }
   if (rc) {
      while (!feof(f)) {
         fgets(sz, 256, f);
         c = sscanf(sz, " %04X:%08X       %s", &sel, &eip, szSrc);
         if (c == 3) {
            if (sel == 1) {
               pDdi->pByValue[pDdi->cByValue].sel   = sel;
               pDdi->pByValue[pDdi->cByValue].eip   = eip;
               pDdi->pByValue[pDdi->cByValue].iName = AddName(pDdi, szSrc);

               pDdi->cByValue++;

               if (NEED_COMMIT(sizeof(BYVALUE) * pDdi->cByValue, sizeof(BYVALUE))) {
                  pDdi->pByValue  = mrRealloc(pDdi->pByValue,
                     (pDdi->cByValue + 1) * sizeof(BYVALUE));
               }
            }
         } else {
            break;
         }
      }
   }
   if (rc) {
      while (!feof(f)) {
         fgets(sz, 256, f);
         if (!strncmp(sz, "Line numbers for " , 17)) {
            if (fl) {
               printf(".");
            }

            iSource = 0;

            fgets(sz, 256, f);
            fgets(sz, 256, f);
            fgets(sz, 256, f);
            fgets(sz, 256, f);
            while (!feof(f)) {
               fgets(sz, 256, f);
               if (sscanf(sz, "  %lu %lu %02X %4X:%08X ",
                   &iLine, &i, &fFlags, &sel, &eip) == 5 && i) {
                  iLine = (ULONG) atol(&sz[1]);
                  pDdi->pLine[pDdi->cLine].iLine = (USHORT) iLine;
                  pDdi->pLine[pDdi->cLine].eip   = eip;// + ulBase;

                  pDdi->cLine++;

                  if (NEED_COMMIT(sizeof(LINE) * pDdi->cLine, sizeof(LINE))) {
                     pDdi->pLine  = mrRealloc(pDdi->pLine,
                        (pDdi->cLine + 1) * sizeof(LINE));
                  }

                  if (!iSource) {
                     pDdi->pSource[pDdi->cSource].iName      = i | 0x80000000;
                     pDdi->pSource[pDdi->cSource].iLineBegin = pDdi->cLine - 1;

                     pDdi->cSource++;

                     iSource = pDdi->cSource;
                     iPrevSource = i;

                     if (NEED_COMMIT(sizeof(SOURCE) * pDdi->cSource, sizeof(SOURCE))) {
                        pDdi->pSource  = mrRealloc(pDdi->pSource,
                           (pDdi->cSource + 1) * sizeof(SOURCE));
                     }

                  } else {
                     if (i != iPrevSource) {

                        pDdi->pSource[pDdi->cSource].iName      =
                           i | 0x80000000;
                        pDdi->pSource[pDdi->cSource].iLineBegin =
                           pDdi->cLine - 1;

                        pDdi->pSource[pDdi->cSource - 1].iLineEnd =
                           pDdi->cLine - 1;

                        pDdi->cSource++;
                        iPrevSource = i;

                        if (NEED_COMMIT(sizeof(SOURCE) * pDdi->cSource, sizeof(SOURCE))) {
                           pDdi->pSource  = mrRealloc(pDdi->pSource,
                              (pDdi->cSource + 1) * sizeof(SOURCE));
                        }
                     }

                  }
               } else {
                  if (i) {
                     pDdi->pSource[pDdi->cSource - 1].iLineEnd =
                        pDdi->cLine;
                     break;
                  }
               }
            }
            fgets(sz, 256, f);
            fgets(sz, 256, f);
            fgets(sz, 256, f);

            i = 0;
            iSource--;
            while (!feof(f)) {
               i++;
               fgets(sz, 256, f);
               if (sscanf(sz, " %s", szSrc) == 1) {
                  if (!strcmp(szSrc, "File")) {
                     psz = sz;
                     while (*psz && *psz++ != ')') {;}
                     if (sscanf(psz, " %s", szSrc) == 1) {
                        for (c = (int) iSource; c < pDdi->cSource; c++) {
                           if (pDdi->pSource[c].iName == (i | 0x80000000)) {
                              pDdi->pSource[c].iName = AddName(pDdi, szSrc);
                           }
                        }
                     }
                  } else {
                     for (c = (int) iSource; c < pDdi->cSource; c++) {
                        if (pDdi->pSource[c].iName == (i | 0x80000000)) {
                           pDdi->pSource[c].iName = AddName(pDdi, szSrc);
                        }
                     }
                  }
               } else {
                  break;
               }
            }
         }
      }

      if (fl) {
         printf("\n");
      }
   }

   if (f) {
      fclose(f);
   }

   if (!rc) {
      if (fl & DDI_BUILD) {
         if (pDdi && pDdi != DDI_LIBNOINFO) {
            printf("\nddI:      Invalid %s: no %s", pszMap,
               !pDdi->cByValue 
                  ? "Functions"
                  : !pDdi->cSource 
                     ? "Sources"
                     : !pDdi->cLine
                       ? "Lines"
                       : "match");
         } 
         sprintf(sz, "\nddI:      Could not parse %s", pszMap);

         DdiPerror(sz);
      }
      DdiFree(pDdi);
      pDdi = DDI_LIBNOINFO;
   }

   if (pDdi && pDdi != DDI_LIBNOINFO) {
      if (pDdi->cByValue && pDdi->cSource && pDdi->cLine) {
         pDdi->cch = pDdi->cByValue * sizeof(BYVALUE) +
                     pDdi->cSource  * sizeof(SOURCE)  +
                     pDdi->cLine    * sizeof(LINE)    +
                     pDdi->cName    * sizeof(PSZ)     +
                     pDdi->cchNameSpace               +
                     sizeof(DDI);
   
         SET_MAGIC(pDdi->szMagic1);
         SET_MAGIC(pDdi->szMagic2);

      } else {
         if (fl & DDI_BUILD) {
            printf("\nddI:      Invalid %s, no %s", pszMap,
               !pDdi->cByValue 
                  ? "Functions"
                  : !pDdi->cSource 
                     ? "Sources"
                     : !pDdi->cLine
                       ? "Lines"
                       : "match");
            sprintf(sz, "\nddI:      Could not parse %s", pszMap);
   
            DdiPerror(sz);
         }
         DdiFree(pDdi);
         pDdi = DDI_LIBNOINFO;
         rc = FALSE;
      } 
   }

   *ppDdi = pDdi;

   if (rc && (fl & DDI_DELMAP)) {
      remove(pszMap);
   }

   return rc;
}

int DdiHeader(ULONG fl, PLIB pLib, PSZ psz)
{
   APIRET         rc = FALSE;
   struct exe     exe;
   struct e32_exe exe32;
   long           loffExe32;
   ULONG          ulsize;
   ULONG          cbAfterDebugInfo;
   PBYTE          pByte;
   PBYTE          pByte2;
   FILESTATUS3    fstat3;
   HFILE          hFile = 0;
   HFILE          hFileDeb = 0;
   ULONG          ulAction;
   ULONG          cb;
   ULONG          loff;
   int            i;
   struct o32_obj obj32;
   ULONG          flOpenFlags;
   CHAR           sz[256];
   ULONG          cbDebug;
   ULONG          cbDDI;
   PDDI           pDdi;

   pDdi = NULL;

   if (fl & (DDI_DROPDEBUGINFO | DDI_MOVEDEBUGINFO | DDI_ADDDEBUGINFO)) {
      flOpenFlags =
         OPEN_ACCESS_READWRITE | OPEN_FLAGS_SEQUENTIAL | OPEN_SHARE_DENYWRITE;
   } else {
      flOpenFlags =
         OPEN_ACCESS_READONLY  | OPEN_FLAGS_SEQUENTIAL | OPEN_SHARE_DENYWRITE;
   }

   rc = DosOpen(pLib->sz, &hFile, &ulAction, 0, 0,
      OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS, flOpenFlags, NULL);
   if (!rc) {
      DosRead(hFile, &exe, sizeof(struct exe), &cb);
      if (exe.eid == EXEID) {
         DosRead(hFile, &loffExe32, sizeof(loffExe32), &cb);
         DosRead(hFile, &loffExe32, sizeof(loffExe32), &cb);
         DosRead(hFile, &loffExe32, sizeof(loffExe32), &cb);
         DosRead(hFile, &loffExe32, sizeof(loffExe32), &cb);
         DosSetFilePtr(hFile, loffExe32, FILE_BEGIN, &loff);
         DosRead(hFile, &exe32, sizeof(struct e32_exe), &cb);

         if (E32_MAGIC1(exe32) == E32MAGIC1 &&
             E32_MAGIC2(exe32) == E32MAGIC2) {

            rc = TRUE;
            if (fl & DDI_MOVEDEBUGINFO) {
               if (E32_DEBUGLEN(exe32)) {
                  DosQueryFileInfo(hFile, FIL_STANDARD, &fstat3, sizeof(fstat3));
                  ulsize = fstat3.cbFile;
                  pByte = malloc(E32_DEBUGLEN(exe32));
                  if (pByte) {
                     DosSetFilePtr(hFile,
                        (LONG) E32_DEBUGINFO(exe32), FILE_BEGIN, &loff);
                     DosRead(hFile, pByte,  E32_DEBUGLEN(exe32), &cb);
                     strcpy(sz, pLib->sz);
                     Extention(sz, ".deB");
                     rc = DosOpen(sz, &hFileDeb, &ulAction, E32_DEBUGLEN(exe32), 0,
                        OPEN_ACTION_REPLACE_IF_EXISTS | OPEN_ACTION_CREATE_IF_NEW, flOpenFlags, NULL);
                     if (!rc) {
                        DosWrite(hFileDeb, pByte, E32_DEBUGLEN(exe32), &cb);
                        DosClose(hFileDeb);

                        printf("\nDebugInfo written to   %s", sz);
                        fl |= DDI_DROPDEBUGINFO;
                     } else {
                        printf("\nError (%i) writing to  %s", rc, sz);
                     }
                     free(pByte);
                  } else {
                     printf("\nError allocating DebugInfo, no space");
                  }
               } else {
                  printf("\nNo Debuginfo found in  %s", pLib->sz);
               }
            } 

            if (fl & DDI_DROPDEBUGINFO) {
               if (E32_DEBUGLEN(exe32)) {
                  DosQueryFileInfo(hFile, FIL_STANDARD, &fstat3, sizeof(fstat3));
                  ulsize = fstat3.cbFile;

                  cbAfterDebugInfo = ulsize -
                     E32_DEBUGINFO(exe32) - E32_DEBUGLEN(exe32);
                  cbDebug = E32_DEBUGLEN(exe32);
                  if (cbAfterDebugInfo) {
                     pByte = malloc(cbAfterDebugInfo);
                     if (pByte) {
                        DosSetFilePtr(hFile,
                           (LONG) (E32_DEBUGINFO(exe32) + E32_DEBUGLEN(exe32)),
                           FILE_BEGIN, &loff);
                        DosRead(hFile, pByte,  cbAfterDebugInfo, &cb);
                        DosSetFilePtr(hFile, (LONG) E32_DEBUGINFO(exe32),
                           FILE_BEGIN, &loff);
                        DosWrite(hFile, pByte, cbAfterDebugInfo, &cb);

                        free(pByte);
                     } else {
                        printf("\nError allocating cbAfterDebugInfo, no space");
                     }
                  }

                  fstat3.cbFile -= E32_DEBUGLEN(exe32);
                  E32_DEBUGINFO(exe32) = 0;
                  E32_DEBUGLEN(exe32)  = 0;
                  DosSetFilePtr(hFile, loffExe32, FILE_BEGIN, &loff);
                  DosWrite(hFile, &exe32, sizeof(struct e32_exe), &cb);

                  DosSetFileSize(hFile, fstat3.cbFile);

                  DosSetFilePtr(hFile,
                     -sizeof(ICC_MAGIC) - sizeof(LONG), FILE_END, &loff);
                  DosRead(hFile, sz, sizeof(ICC_MAGIC), &cb);
                  if (QUERY_ICC_MAGIC(sz)) {
                     DosSetFileSize(hFile,
                        fstat3.cbFile - sizeof(ICC_MAGIC) - sizeof(LONG));
                  }
                  printf("\nDebugInfo deleted from %s (%u bytes)", pLib->sz, cbDebug);
               } else {
                  printf("\nNo Debuginfo found in  %s", pLib->sz);
               }
            }

            if (fl & DDI_ADDDEBUGINFO) {
               if (!E32_DEBUGLEN(exe32)) {
                  strcpy(sz, pLib->sz);
                  Extention(sz, ".deB");
                  rc = DosOpen(sz, &hFileDeb, &ulAction, 0, 0,
                     OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                     flOpenFlags, NULL);
                  if (!rc) {
                     DosQueryFileInfo(hFileDeb, FIL_STANDARD, &fstat3, sizeof(fstat3));
                     cbDebug = fstat3.cbFile;
                     pByte = malloc(fstat3.cbFile);
                     if (pByte) {
                        DosRead(hFileDeb, pByte, fstat3.cbFile, &cb);
                     } else {
                        printf("\nError allocating cbDebug, no space");
                        rc = 2;
                     } 
                     DosClose(hFileDeb);

                  } else {
                     printf("\nCould not read         %s (%i)", sz, rc);
                  }
                  if (!rc) {
                     pByte2 = NULL;
                     DosSetFilePtr(hFile,
                        -(sizeof(pDdi->szMagic2) + sizeof(pDdi->lOffset)),
                        FILE_END, &loff);
                     DosRead(hFile, sz, sizeof(pDdi->szMagic2), &cb);
                     DosRead(hFile, &cbDDI,  sizeof(cbDDI),  &cb);
                     if (QUERY_MAGIC(sz)) {
                        DosSetFilePtr(hFile,
                           -(LONG) cbDDI, FILE_CURRENT, &loff);
                        pByte2 = malloc(cbDDI);
                        if (pByte2) {
                           DosRead(hFile, pByte2,  cbDDI, &cb);
                        } else {
                           cbDDI = 0;
                           printf("\nError allocating cbDDI, no space");
                        }
                     } else {
                        cbDDI = 0;
                     }
                     DosSetFilePtr(hFile, -(LONG) cbDDI, FILE_END, &loff);

                     DosWrite(hFile, pByte, cbDebug, &cb);
                     free(pByte);
                     if (pByte2) {
                        DosWrite(hFile, pByte2, cbDDI, &cb);
                        free(pByte2);
                     }

                     DosSetFilePtr(hFile, 0, FILE_CURRENT, &ulsize);
                     loff = cbDDI + cbDebug + sizeof(ICC_MAGIC) + sizeof(LONG);
                     DosWrite(hFile, ICC_MAGIC, sizeof(ICC_MAGIC), &cb);
                     DosWrite(hFile, &loff, sizeof(LONG), &cb);

                     loff = ulsize - cbDDI - cbDebug;
                     E32_DEBUGINFO(exe32) = loff;
                     E32_DEBUGLEN(exe32)  = cbDebug;
                     DosSetFilePtr(hFile, loffExe32, FILE_BEGIN, &loff);
                     DosWrite(hFile, &exe32, sizeof(struct e32_exe), &cb);
         
                     fstat3.cbFile = ulsize + sizeof(ICC_MAGIC) + sizeof(LONG);
                     DosSetFileSize(hFile, fstat3.cbFile);

                     printf("\nDebugInfo added to     %s (%u bytes)", pLib->sz, cbDebug);
                  } 
               } else {
                  printf("\nDebuginfo present in   %s (%u bytes)", pLib->sz, E32_DEBUGLEN(exe32));
               }
            } 

            if (fl & DDI_BUILDADDR) {
               pLib->pAddr = NULL;
               pLib->cAddr = 0;
               pLib->cObj  = E32_OBJCNT(exe32);
               if (pLib->cObj) {
                  DosSetFilePtr(hFile, (LONG) (loffExe32 + E32_OBJTAB(exe32)),
                     FILE_BEGIN, &loff);
                  pLib->pAddr = calloc(pLib->cObj, sizeof(ADDR));
                  pLib->cAddr = 0;
                  for (i = 0; i < pLib->cObj; i++) {
                     DosRead(hFile, &obj32, sizeof(obj32), &cb);
                     if (obj32.o32_size) {
                        pLib->pAddr[pLib->cAddr].lFlags = obj32.o32_flags;
                        pLib->cAddr++;
                     }
                  }
               }
            }
         }
      }
      DosClose(hFile);
   } else {
      sprintf(psz, "\nCould not read/write %s", pLib->sz);
      if (pDdi) {
         rc = FALSE;
      } else {
         rc = FALSE;
      } 
   }

   return (int) rc;
}

int   DdiShow      (PDDI pDdi, ULONG fl, PULONG pcSource, PULONG pcFunction,
   PULONG pcLine)
{
   ULONG i, j;

   if (pDdi) {
      if (pDdi != DDI_LIBNOINFO) {
         *pcSource   += pDdi->cSource;
         *pcFunction += pDdi->cByValue;
         *pcLine     += pDdi->cLine;

         printf("%5i Bytes", pDdi->cch);
         printf("\n%-5i Source%s", pDdi->cSource, pDdi->cSource == 1 ? "" : "s");

         if (fl & DDI_VERBOSE) {
            for (i = 0; i < pDdi->cSource; i++) {
               printf("\n    %-32s Start %5i End %5i",
                  pDdi->aszName[pDdi->pSource[i].iName],
                  pDdi->pSource[i].iLineBegin,
                  pDdi->pSource[i].iLineEnd);
            }
         }
         printf("\n%-5i Function%s", pDdi->cByValue, pDdi->cByValue == 1 ? "" : "s");

         if (fl & DDI_VERBOSE) {
            for (i = 0; i < pDdi->cByValue; i++) {
               printf("\n   %-32s EntryPoint %04X:%08X",
                  pDdi->aszName[pDdi->pByValue[i].iName],
                  pDdi->pByValue[i].sel,
                  pDdi->pByValue[i].eip);
            }
         }
         printf("\n%-5i Statement%s %s", pDdi->cLine, pDdi->cLine == 1 ? "" : "s",
            pDdi->cLine > 0 ? "or more" : "");

         if (fl & DDI_VERBOSE) {
            for (i = 0; i < pDdi->cSource; i++) {
               printf("\n    %-32s Start %5i End %5i",
                  pDdi->aszName[pDdi->pSource[i].iName],
                  pDdi->pSource[i].iLineBegin,
                  pDdi->pSource[i].iLineEnd);
               for (j = pDdi->pSource[i].iLineBegin;
                    j < pDdi->pSource[i].iLineEnd;
                    j++) {
                  printf("\n      %5i %08X",
                     pDdi->pLine[j].iLine,
                     pDdi->pLine[j].eip);
               }
            }
         }
         i = pDdi->cch;
      } else {
         printf("   No info");
         i = 0;
      }
   }
   EDC0804(NULL);
   return (int) i;
}


int DdiFree(PDDI pDdi)
{
   if (pDdi && pDdi != DDI_LIBNOINFO) {

      if (pDdi->pByValue) {
         mrFree(pDdi->pByValue);
      }
      if (pDdi->pLine) {
         mrFree(pDdi->pLine);
      }
      if (pDdi->pSource) {
         mrFree(pDdi->pSource);
      }
      if (pDdi->pchNameSpace) {
         mrFree(pDdi->pchNameSpace);
      }
      if (pDdi->aszName) {
         mrFree(pDdi->aszName );
      }
      free(pDdi);
   }
   return 0;
}

static ULONG AddName(PDDI pDdi, PSZ pszName)
{
   ULONG cchName;
   PCHAR pchNameSpace;

   cchName             = strlen(pszName) + 2;
   pDdi->cchNameSpace += cchName;

   if (NEED_COMMIT(pDdi->cchNameSpace, ALLOC_INC)) {
      pDdi->pchNameSpace = mrRealloc(pDdi->pchNameSpace, pDdi->cchNameSpace + ALLOC_INC);
   } 
   if (NEED_COMMIT(sizeof(PSZ *) * pDdi->cName, sizeof(PSZ *))) {
      pDdi->aszName  = mrRealloc(pDdi->aszName,
         (pDdi->cName + 1) * sizeof(PSZ *));
   }

   pchNameSpace  = pDdi->pchNameSpace + pDdi->cchNameSpace - cchName;
   *pchNameSpace = (CHAR) cchName;
   memcpy(++pchNameSpace, pszName, cchName - 1);
   pDdi->aszName[pDdi->cName] = pchNameSpace;

   pDdi->cName++;

   return pDdi->cName - 1;
}

static int EDC0804(PVOID p)
{
   if (p) {

      DdiLineInfo(0, NULL, 0, NULL, 0);
      DdiHeader(0, NULL, NULL);
      DdiShow(NULL, 0, NULL, NULL, NULL);
      DdiSourceInfo(NULL, NULL);
   }
   return (0);
}
