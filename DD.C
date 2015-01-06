// #define DUMP_ALLOC

#ifdef __IBMC__
#pragma pack(4)
#endif

#define PTRACE_C

#define INCL_DOSPROCESS
#define INCL_DOSSESMGR
#define INCL_DOSMODULEMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSMISC
#define INCL_DOSSIGNALS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSERRORS
#define INCL_WINPOINTERS
#include <os2.h>

#undef KB_HEAP

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
//#include <malloc.h>

#include <process.h>

#include "ddexec.h"
#include "dd.h"
#include "ddb.h"
#include "ddi.h"



#define STACKSIZE 8192 * 8

#define STK_DMPONCE     0x001
#define STK_DMPALL      0x002
#define STK_DMPSHOW     0x100


typedef struct _THRD {
   ULONG tid;
   ULONG ulESP;
} THRD;
typedef THRD * PTHRD;

typedef struct _DD {
   CHAR   pszExec [MAXPATH];
   CHAR   pszDmp  [MAXPATH];
   CHAR   pszSrc  [MAXPATH];
   CHAR   pszStack[MAXPATH];

   CHAR   pszMsg  [MSGSIZE * 2];
   ULONG  fl;
   PID    pid;
   APIRET rc;
   int    cDumped;
   int    fDumping;

   USHORT cLib;
   PLIB   pLib;

   USHORT cThrd;
   PTHRD  pThrd;
} DD;
typedef DD * PDD;


VOID     thrDD        (PVOID pVoid);
FNTHREAD thrDDShow;
APIRET   ddRcError    (APIRET rc, LONG lCmd, LONG lValue, PDD pDD, FILE * f);
APIRET   ddFree       (PDD pDD);
int      ddPrintf     (ULONG fl, FILE * f, char * szFormat, ...);
 
APIRET   ddEvent      (APIRET rc, LONG lCmd, LONG lValue,
                       const PDDBUF pddb, PDD pDD);

int  DumpReg        (PDD pDD, const PDDBUF pddb, ULONG ulEIP,
                     PCHAR pch, FILE * f);
int  DumpModule     (PDD pDD, const PDDBUF pddb, ULONG ulEIP,
                     PCHAR pch, FILE * f);
int  DumpStack      (PDD pDD, const PDDBUF pddb, ULONG ulEIP,
                     PCHAR pch, FILE * f, ULONG fl);
int  DumpStack2File (PDD pDD, const PDDBUF pddb, ULONG ulESP,
                     PBYTE pb, int cb);
BOOL IsCaller       (PDD pDD, const PDDBUF pddb, ULONG ulEIP, FILE * f);

int DumpAsso        (PDD pDD, PBYTE pb, int cb, PSZ psz, FILE * f,
                    int level, int * phit, ULONG ulAddr, PCHAR pch, int * pich);


PSZ  ExplainException(PCHAR psz, PEXCEPTIONREPORTRECORD pXReport,
                PCONTEXTRECORD pXContext);

PSZ   szMte            (USHORT cLib, PLIB aLib, HMODULE mte);
PCHAR szAsciiTimeStamp (void);

#define XC_CONTINUE_EXECUTION   lValue = (LONG) XCPT_CONTINUE_EXECUTION;\
                                lCmd   = DBG_C_Continue;                \
                                ulTid  = ddb.ulTid;

#define XC_CONTINUE_SEARCH      lValue = (LONG) XCPT_CONTINUE_SEARCH;   \
                                lCmd   = DBG_C_Continue;                \
                                ulTid  = ddb.ulTid;

#define XC_GO                   lCmd   = DBG_C_Go; ulTid  = 0;

#define XC_FREEZE               lCmd   = DBG_C_Freeze;

#define XC_NUM2ADDR(mte, num)   ddb.ulPid  = pddb->ulPid;               \
                                ddb.lCmd   = DBG_C_NumToAddr;           \
                                ddb.lValue = num;                       \
                                ddb.ulMTE  = mte;                       \
                                rc = DosDebug(&ddb);

#define XC_ADDR2OBJ(mte, addr)  ddb.ulPid  = pddb->ulPid;               \
                                ddb.lCmd   = DBG_C_AddrToObject;        \
                                ddb.ulAddr = addr;                      \
                                ddb.ulMTE  = mte;                       \
                                rc = DosDebug(&ddb);

#define XC_READMEMBUF(read, copy, c)  ddb.ulPid    = pddb->ulPid;       \
                                      ddb.lCmd     = DBG_C_ReadMemBuf;  \
                                      ddb.ulAddr   = (ULONG) (read);    \
                                      ddb.ulBuffer = (ULONG) (copy);    \
                                      ddb.ulLen    = (ULONG) (c);       \
                                      rc = DosDebug(&ddb);              

//                                      if (rc) {                         \
//                                         ddRcError(rc, ddb.lCmd,        \
//                                            ddb.lValue, pDD);           \
//                                      }

#define XC_READREG(tid)               ddb.ulPid    = pddb->ulPid;       \
                                      ddb.ulTid    = (ULONG) tid;       \
                                      ddb.lCmd     = DBG_C_ReadReg;     \
                                      rc = DosDebug(&ddb);              

//                                      if (rc) {                         \
//                                         ddRcError(rc, ddb.lCmd,        \
//                                            ddb.lValue, pDD);           \
//                                      }



APIRET DDbug(PID pid, PDDEXEC pDDExec, PTID pTid, ULONG fl, PSZ pszExe)
{
   APIRET rc  = 0;
   PDD    pDD = NULL;

   pDD = calloc(1, sizeof(DD));

   pDD->fl        = fl;
   pDD->pid       = pid;
   pDD->rc        = 0;
   pDD->pszMsg[0] = 0;

   pDD->cLib      = 0;
   pDD->pLib      = mrMalloc(ONE_MEG);

   pDD->cThrd     = 0;
   pDD->pThrd     = mrMalloc(ONE_MEG);

   strcpy(pDD->pszExec, strupr(pszExe));

   if (pDDExec && pDDExec->pszDDD && *pDDExec->pszDDD) {
      strcpy(pDD->pszDmp, pDDExec->pszDDD);
   } else {
      strcpy(pDD->pszDmp, pDD->pszExec);
   }
   strcpy(pDD->pszSrc,   pDD->pszDmp);
   strcpy(pDD->pszStack, pDD->pszDmp);

   Extention(pDD->pszDmp,   ".ddD");
   Extention(pDD->pszSrc,   ".ddS");
   Extention(pDD->pszStack, ".ddT");

   remove(pDD->pszDmp);
   remove(pDD->pszSrc);
   remove(pDD->pszStack);

   *pTid = (unsigned int) _beginthread(thrDD, NULL, STACKSIZE, (PVOID) pDD);

   if (*pTid  != (unsigned int) -1) {
      if (pDD->fl & EXEC_WAIT) {
         rc = DosWaitThread(pTid, DCWW_WAIT);
      }
   } else {
      rc = (ULONG) -1;
      if (pDD) {
         strcpy(pszExe, pDD->pszMsg);
      } else {
         strcpy(pszExe, "Could not _beginthread thrDD.");
      }
   }




   return rc;
}

VOID  thrDD    (PVOID pVoid)
{
   PDD       pDD      = pVoid;
   APIRET    rc       = 0;
   FILE *    f;
   DDBUF     ddb;
   DDBUF     ddb2;
   TID       ulTid;
   PID       ulPid;
   LONG      lValue;
   LONG      lCmd;
   LONG      lNotific;
   USHORT    fTracing = TRUE;

   setvbuf(stdout, NULL, _IONBF, 0);

   f = fopen(pDD->pszDmp, "a");
   if (f) {
      ddPrintf(pDD->fl, f, "\n%s", szAsciiTimeStamp());
      fclose(f);
   }

   lCmd       = DBG_C_Connect;
   lValue     = 1;

   ddb.ulPid  = pDD->pid;
   ddb.ulTid  = 0;
   ddb.lCmd   = lCmd;
   ddb.lValue = lValue;

   rc = DosDebug(&ddb);
   if (rc) {
      ddRcError(rc, lCmd, lValue, pDD, NULL);
      fTracing = FALSE;
   } else {
      ddEvent(rc, lCmd, lValue, &ddb, pDD);
      if (ddb.lCmd == DBG_N_ERROR) {   
         fTracing = FALSE;
      }
   }



   lCmd   = DBG_C_Go;
   lValue = 0;
   ulPid  = pDD->pid;
   ulTid  = 1;

   while (fTracing) {

      ddb.ulPid  = ulPid;
      ddb.ulTid  = ulTid;
      ddb.lValue = lValue;
      ddb.lCmd   = lCmd;

//      printf("\n! %-4lu %-4lu %-20s lValue %08lX",
//         ulPid, ulTid, szDebugCommand(lCmd),  lValue);

      rc = DosDebug(&ddb);

      if (rc) {
         ddRcError(rc, lCmd, lValue, pDD, NULL);

      } else {

         lNotific = ddb.lCmd;
         ddb2 = ddb;

         ddEvent(rc, lCmd, lValue, &ddb2, pDD);

         switch (lNotific) {
         case DBG_N_SUCCESS      :
            fTracing = FALSE;
            break;

         case DBG_N_ERROR        :
            fTracing = FALSE;
            break;

         case TRC_C_SIG_ret      :
         case TRC_C_TBT_ret      :
         case TRC_C_BPT_ret      :
            XC_CONTINUE_EXECUTION;
            break;

         case DBG_N_ModuleLoad   :
         case DBG_N_ThreadTerm   :
         case DBG_N_ThreadCreate :
         case DBG_N_AsyncStop    :
         case DBG_N_NewProc      :
         case DBG_N_AliasFree    :
         case DBG_N_Watchpoint   :
         case DBG_N_ModuleFree   :
         case DBG_N_RangeStep    :
            XC_CONTINUE_EXECUTION;
            break;

         case TRC_C_NMI_ret      :
         case DBG_N_Exception    :
         case DBG_N_CoError      :
            XC_CONTINUE_SEARCH;
            break;

         case DBG_N_ProcTerm     :
            XC_CONTINUE_EXECUTION;
            break;

         default :
            break;
         }

      }
   }

   f = fopen(pDD->pszDmp, "a");
   if (f) {
      ddPrintf(pDD->fl, f, "\n%s\n", szAsciiTimeStamp());
      fclose(f);
   }

   ddFree(pDD);

#ifdef DUMP_ALLOC
   printf("\n");
   _dump_allocated(79);
#endif

   _endthread();
   DDbug(0, NULL, NULL, 0, NULL);
   return;
}

APIRET ddFree(PDD pDD)
{
   int i;

   for (i = 0; i < pDD->cLib; i++) {
      DdiFree(pDD->pLib[i].pDdi);
      if (pDD->pLib[i].pAddr) {
         free(pDD->pLib[i].pAddr);
      } 
   } 
   mrFree(pDD->pLib);
   mrFree(pDD->pThrd);
   free(pDD);

   return 0;
}

APIRET ddRcError(APIRET rc, LONG lCmd, LONG lValue, PDD pDD, FILE * fdmp)
{
   FILE * f;

   if (fdmp) {
      f = fdmp;
   } else {
      f = fopen(pDD->pszDmp, "a");
   }

   if (f == NULL) {
      f = fopen(pDD->pszDmp, "a");
      f = stderr;
      ddPrintf(pDD->fl, f, "??Could not append %s", pDD->pszDmp);
   }

   if (rc) {
      ddPrintf(pDD->fl, f, "\n$$$        DosDebug rc = %s %08lX (%lu)",
         rc == ERROR_INVALID_PARAMETER
            ? "ERROR_INVALID_PARAMETER"
            : rc == ERROR_INTERRUPT
               ? "ERROR_INTERRUPT"
               : rc == ERROR_PROTECTION_VIOLATION
                  ? "ERROR_PROTECTION_VIOLATION"
                  : rc == ERROR_INVALID_DATA
                     ? "ERROR_INVALID_DATA"
                     : "ERROR",
         rc, rc); 

      ddPrintf(pDD->fl, f, "\n$$$ lCmd   was %s", szDebugComm(lCmd));
      ddPrintf(pDD->fl, f, "\n$$$ lValue was %08lX (%lu)", lValue, lValue);
   }
     
   if (f != stderr) {
      fclose(f);
   }
   return 0;
}

int ddPrintf(ULONG fl, FILE * f, char * szFormat, ...)
{
   va_list pArg;
   int     rc;

   fl = fl;
   va_start(pArg, szFormat);
   vfprintf(f, szFormat, pArg);
   if (!(fl & DDI_SILENT)) {
      rc = vprintf(szFormat, pArg);
   } 
   va_end(pArg);
   return rc;
}

APIRET ddEvent(APIRET rc, LONG lCmd, LONG lValue, 
               const PDDBUF pddb, PDD pDD)
{
   FILE *   f;
   CHAR     szBuf[1024];
   DDBUF    ddb;
   PLIB     pLib;
   int      i, j;
   ULONG    ulEIP;

   lValue = lValue;

   f = fopen(pDD->pszDmp, "a");

   if (f == NULL) {
      sprintf(szBuf, "??Could not append %s", pDD->pszDmp);
      perror(szBuf);
      f = stderr;
   }
   if (rc) {
      ddRcError(rc, lCmd, lValue, pDD, f);

   } else {
      if (pDD->fl & EXEC_VERBOSE) {
         ddPrintf(pDD->fl, f,     "\n$ %-10s %-4lu %-4lu%s",
            szDebugComm(lCmd),
            pddb->ulPid, pddb->ulTid, szDebugNotification(pddb->lCmd));

      } else {
         if (pddb->lCmd != DBG_N_ModuleLoad) {
            ddPrintf(pDD->fl, f,     "\n$ %-4lu %-4lu%s",
               pddb->ulPid, pddb->ulTid, szDebugNotification(pddb->lCmd));
         }
      }

      switch (pddb->lCmd) {
      case DBG_N_ERROR        :
         ddPrintf(pDD->fl, f,     szDebugNotific(pddb->lCmd));
         ddPrintf(pDD->fl, f,     "\n$$         %s returns %s %08lX (%lu)",
            szDebugComm(lCmd),
            pddb->lValue == ERROR_INVALID_PARAMETER
               ? "ERROR_INVALID_PARAMETER"
               : pddb->lValue == ERROR_INTERRUPT
                  ? "ERROR_INTERRUPT"
                  : pddb->lValue == ERROR_PROTECTION_VIOLATION
                     ? "ERROR_PROTECTION_VIOLATION"
                     : pddb->lValue == ERROR_INVALID_DATA
                        ? "ERROR_INVALID_DATA"
                        : "ERROR",
            pddb->lValue, pddb->lValue);
         break;

      case TRC_C_SIG_ret      :
      case TRC_C_TBT_ret      :
      case TRC_C_BPT_ret      :
         ddPrintf(pDD->fl, f,     szDebugNotific(pddb->lCmd), pddb->lValue);
         break;

      case DBG_N_SUCCESS      :
         if (lCmd == DBG_C_Connect) {
            ddPrintf(pDD->fl, f,     " Connected %s", pDD->pszExec);
         }
         break;

      case DBG_N_ModuleLoad   :
         pLib = pDD->pLib;
         for (i = 0; i < pDD->cLib; i++) {
            if (pLib[i].mte == pddb->lValue) {
               break;
            }
         }
         if (i == pDD->cLib) {
            pLib = &pDD->pLib[pDD->cLib++];

            if (NEED_COMMIT(sizeof(LIB) * pDD->cLib, sizeof(LIB))) {

               pDD->pLib  = mrRealloc(pDD->pLib, (ULONG)
                  (pDD->cLib + 1) * sizeof(LIB));
            }

            pLib->mte  = (ULONG) pddb->lValue;
            pLib->pDdi = NULL;

            rc = DosQueryModuleName(pLib->mte, MAXPATH, szBuf);
            if (rc) {
               ddPrintf(pDD->fl, f,     "\n$$ DosQueryModuleName(%lu) rc = %lu",
                  pLib->mte, rc);
               strcpy(pLib->sz, "NoName");
            } else {
               strcpy(pLib->sz, szBuf);

               if (pDD->fl & EXEC_VERBOSE) {
                  ddPrintf(pDD->fl, f,     szDebugNotific(pddb->lCmd), pLib->mte,
                     pLib->sz);
               } else {
                  if (!(!strncmp(&pLib->sz[1], ":\\OS2\\DLL",     9) ||
                        !strncmp(&pLib->sz[1], ":\\CMLIB\\DLL",  11) ||
                        !strncmp(&pLib->sz[1], ":\\MUGLIB\\DLL", 12) ||
                        !strncmp(&pLib->sz[1], ":\\SQLLIB\\DLL", 12) ||
                        !strncmp(&pLib->sz[1], ":\\IBMCOM\\DLL", 12))) {
                     ddPrintf(pDD->fl, f,     "\n$ %-4lu %-4lu%s",
                        pddb->ulPid, pddb->ulTid,
                        szDebugNotification(pddb->lCmd));
                     ddPrintf(pDD->fl, f,     szDebugNotific(pddb->lCmd), pLib->mte,
                        pLib->sz);
                  }
               }
            }

            DdiHeader(DDI_BUILDADDR, pLib, szBuf);

            if (pLib->cAddr) {
               for (i = 0, j = 0; i < pLib->cObj; i++) {
                  ULONG ulAddr;

                  XC_NUM2ADDR(pLib->mte, i + 1);
                  if (!rc && ddb.lCmd == DBG_N_SUCCESS) {
                     ulAddr = ddb.ulAddr;
                     XC_ADDR2OBJ(pLib->mte, ulAddr);
                     if (!rc                              &&
                         ddb.lCmd == DBG_N_SUCCESS        &&
                         ddb.lValue & DBG_O_OBJMTE        &&
                         ddb.ulLen                       ) {

                        pLib->pAddr[j].lAddr = ulAddr;         // Store
                        pLib->pAddr[j].lSize = ddb.ulLen;

                        if (pDD->fl & EXEC_VERBOSE) {
                           ddPrintf(pDD->fl, f, "\n%25i %08X %08X", j,
                              pLib->pAddr[j].lAddr,
                              pLib->pAddr[j].lSize);
                        }
                        j++;
                     }
                  }
               }
            }

            DdiRead(1, pLib, DDI_READASYNC);

         } else {
            if (pDD->fl & EXEC_VERBOSE) {
               ddPrintf(pDD->fl, f, szDebugNotific(pddb->lCmd), pLib[i].mte, pLib[i].sz);
               ddPrintf(pDD->fl, f, "\n%23s", "Duplicate");
            }
         }
         break;

      case DBG_N_ProcTerm     :
         ddPrintf(pDD->fl, f, szDebugNotific(pddb->lCmd),
            pddb->lValue, szProcTerm(pddb->ulIndex));
         break;

      case DBG_N_ThreadCreate :
         ddPrintf(pDD->fl, f,     szDebugNotific(pddb->lCmd));
         pddb->lCmd = DBG_C_ReadReg;
         DosDebug(pddb);
         for (i = 0; i < pDD->cThrd; i++) {
            if (!pDD->pThrd[i].tid) {
               break;
            }
         }
         if (i == pDD->cThrd) {
            pDD->cThrd++;
            if (NEED_COMMIT(pDD->cThrd * sizeof(THRD), sizeof(THRD))) {
               pDD->pThrd  = mrRealloc(pDD->pThrd, 
                  pDD->cThrd * sizeof(THRD));
            }
         }

         pDD->pThrd[i].tid   = pddb->ulTid;
         pDD->pThrd[i].ulESP = pddb->ulESP;

//         if (!DumpWhere(pDD, pddb, pddb->ulEIP, szBuf, f)) {
//            DumpStack(pDD, pddb, pddb->ulEIP, szBuf, f, FALSE);
//         }
         break;

      case DBG_N_ThreadTerm   :
         ddPrintf(pDD->fl, f,     szDebugNotific(pddb->lCmd), pddb->lValue);
         for (i = 0; i < pDD->cThrd; i++) {
            if (pDD->pThrd[i].tid == pddb->ulTid) {
               if (pddb->ulTid != 1) {
                  pDD->pThrd[i].tid = 0;
               }
               break;
            }
         }
         break;

      case DBG_N_ModuleFree   :
         ddPrintf(pDD->fl, f,     szDebugNotific(pddb->lCmd),
            pddb->lValue, szMte(pDD->cLib, pDD->pLib, (HMODULE) pddb->lValue));
         break;

      case DBG_N_CoError      :
      case DBG_N_AsyncStop    :
      case DBG_N_NewProc      :
      case DBG_N_AliasFree    :
      case DBG_N_Watchpoint   :
      case DBG_N_RangeStep    :
      case TRC_C_NMI_ret      :
         ddPrintf(pDD->fl, f,     szDebugNotific(pddb->lCmd));
         break;

      case DBG_N_Exception    :
         ulEIP = pddb->ulAddr;

         DumpModule(pDD, pddb, ulEIP, szBuf, f);
         ddPrintf(pDD->fl, f,     "  %s", szExceptValue(pddb->lValue));

         switch (pddb->lValue) {
         PEXCEPTIONREPORTRECORD pXReport;
         PCONTEXTRECORD         pXContext;

         case DBG_X_PRE_FIRST_CHANCE:
            ddPrintf(pDD->fl, f, " %s", " ", szException(pddb->ulBuffer));
            DumpStack(pDD, pddb, ulEIP, szBuf, f,
               STK_DMPALL | STK_DMPSHOW);
            break;

         case DBG_X_FIRST_CHANCE:
         case DBG_X_LAST_CHANCE:
            pXReport  = calloc(1, sizeof(EXCEPTIONREPORTRECORD));
            pXContext = calloc(1, sizeof(CONTEXTRECORD));

            XC_READMEMBUF(pddb->ulBuffer, (ULONG) pXReport,
               sizeof(EXCEPTIONREPORTRECORD));
            if (ddb.lCmd == DBG_N_SUCCESS) {
               XC_READMEMBUF(pddb->ulLen, (ULONG) pXContext,
                  sizeof(CONTEXTRECORD));
            }
            if (ddb.lCmd == DBG_N_SUCCESS) {
               ExplainException(szBuf, pXReport, pXContext);
            } else {
               sprintf(szBuf, "Could not readMem ");
            }
            ddPrintf(pDD->fl, f,     "\n%11s%s", " ", szBuf);

            switch (pXReport->ExceptionNum) {
            case XCPT_SIGNAL                  :
            case XCPT_PROCESS_TERMINATE       :
            case XCPT_ASYNC_PROCESS_TERMINATE :
               break;
         
            default :
               DumpStack(pDD, pddb, ulEIP, szBuf, f, 
               pXReport->ExceptionNum == XCPT_GUARD_PAGE_VIOLATION
                  ? STK_DMPALL 
                  : STK_DMPALL | STK_DMPSHOW);
               DumpReg(pDD, pddb, ulEIP, szBuf, f);
               break;
            }

            free(pXReport);
            free(pXContext);
            break;

         case DBG_X_STACK_INVALID:
            pXReport  = calloc(1, sizeof(EXCEPTIONREPORTRECORD));
            pXContext = calloc(1, sizeof(CONTEXTRECORD));

            DumpStack(pDD, pddb, ulEIP, szBuf, f, 
               STK_DMPALL | STK_DMPSHOW);
            break;
         }
         break;
      }
   }

   if (f != stderr) {
      fclose(f);
   }
   return 0;
}

int DumpModule(PDD pDD, const PDDBUF pddb, ULONG ulEIP, PCHAR pch, FILE * f)
{
   if (DdiLineInfo(pDD->cLib, pDD->pLib, ulEIP, pch, DDI_DMPMODULE)) {
      if (f) {
         ddPrintf(pDD->fl, f, " %s", pch);
      }
      return 1;
   }
   ulEIP = pddb->ulAddr;
   return 0;
}

int DumpReg(PDD pDD, const PDDBUF pddb, ULONG ulEIP, PCHAR pch, FILE * f)
{
   int     i = 0;
   APIRET  rc = 0;
   DDBUF   ddb;

   ulEIP = pddb->ulAddr;

   XC_READREG(pddb->ulTid);

   if (rc) {
      ddRcError(rc, ddb.lCmd, ddb.lValue, pDD, f);

   } else {
      i += sprintf(pch + i, "\nCS:EIP  %04X:%08X         %08X",
         ddb.usCS , ddb.ulEIP, ulEIP);
   
      i += sprintf(pch + i, "\nEAX %08X EBX %08X ECX %08X EDX %08X  "
         "DS %04X ES %04X",
         ddb.ulEAX, ddb.ulEBX, ddb.ulECX, ddb.ulEDX, ddb.usDS, ddb.usES);
   
      i += sprintf(pch + i, "\nEBP %08X ESP %08X EDI %08X ESI %08X  "
         "SS %04X FS %04X GS %04X",
         ddb.ulEBP, ddb.ulESP, ddb.ulEDI, ddb.ulESI, ddb.usSS, ddb.usFS, ddb.usGS );
   
      if (f) {
         ddPrintf(pDD->fl, f, "%s", pch);
      }
   
      i = 0;
/*
         pDDImport(pddb.pid, pch);
   
         ddPrintf(pDD->fl, f, "%s", pch);
*/
   } 
   return 0;
}

int DumpStack(PDD pDD, const PDDBUF pddb, ULONG ulEIP, PCHAR pch, FILE * f, ULONG fl)
{

   APIRET   rc = FALSE;
   PCHAR    pStack;
   PCHAR    pStackEnd;
   PULONG   pEIP;
   PUSHORT  pusEIP;
   ULONG    i;
   int      iThread;
   DDBUF    ddb;
   BOOL     fIsCaller;
#ifdef B32
   BOOL     f32 = FALSE;
#endif
   TID      tidShow;

   fl |= STK_DMPONCE;

   tidShow = 0;

   for (iThread = 0; iThread < pDD->cThrd; iThread++) {
      if (pDD->pThrd[iThread].tid == pddb->ulTid) {
         break;
      }
   }
   if (iThread != pDD->cThrd) {
      XC_READREG(pddb->ulTid);

      if (rc) {
         ddRcError(rc, ddb.lCmd, ddb.lValue, pDD, f);

      } else {
         rc = (APIRET) DdiLineInfo(pDD->cLib, pDD->pLib, ulEIP, pch,
            DDI_DMPINFO);
         if (rc == TRUE) {
            ddPrintf(pDD->fl, f, "\n!%08x %s", ulEIP, pch);
         }
         if ((fl & STK_DMPONCE && rc == FALSE) || fl & STK_DMPALL) {
            i = pDD->pThrd[iThread].ulESP - ddb.ulESP;
            if (i > 0 && i < 1024 * 1024) {
               pStack = malloc((unsigned) i);
               pStackEnd = pStack + i;
               rc = 0;
               while (i) {
                  XC_READMEMBUF(ddb.ulESP, (ULONG) pStack, i);
                  if (!rc) {
                     if (ddb.lCmd == DBG_N_SUCCESS) {

                        if (fl & STK_DMPSHOW) {
                           if (DosCreateThread(&tidShow,
                               thrDDShow, 0, 0, STACKSIZE)) {
                              tidShow = 0;
                           } 
                        } 
   
                        pEIP = (PULONG) pStack;
                        while (pEIP < (PULONG) pStackEnd) {
                           if (DdiLineInfo(pDD->cLib, pDD->pLib, *pEIP, pch,
                               DDI_DMPINFO)) {
                              fIsCaller = IsCaller(pDD, pddb, *pEIP, f);
                              if (fl & STK_DMPONCE) {
                                 ddPrintf(pDD->fl, f, "\n %08x %s", *pEIP, pch);
   //                              DdiSourceInfo(pch, pDD->pszSrc);
                                 if (fl & STK_DMPALL) {
                                    fl &= ~STK_DMPONCE;
                                 } else {
                                    break;
                                 }
                              }
                              if (fl == 1) {
                                 fl = 2;
                              } else {
                                 if (pDD->fl & EXEC_VERBOSE) {
                                    ddPrintf(pDD->fl, f, "\n%s%08x %s",
                                       fIsCaller ? "!" : "?", *pEIP, pch);
                                 } else {
                                    if (fIsCaller) {
                                       ddPrintf(pDD->fl, f, "\n %08x %s", *pEIP, pch);
   //                                    DdiSourceInfo(pch, pDD->pszSrc);
                                    }
                                 }
                              }
   
   #ifdef B32
                              f32 = TRUE;
   #endif
                           } else {
                              ;
                           }
   #ifdef B32
                           if (f32) {
                              pEIP++;
                           } else {
   #else
                           pusEIP = (PVOID) pEIP;
                           pusEIP++;
                           pEIP   = (PVOID)  pusEIP;
   #endif
   #ifdef B32
                           }
   #endif
                        }
                        if (pDD->fl & DDI_DDT) {
                           DumpStack2File(pDD, pddb, ddb.ulESP, pStack, i);
                        } 
                        pDD->cDumped++;
                        break;
   
                     } else {
                        i--;
                        ddb.ulESP++;
                     }
                  } else {
                     ddPrintf(pDD->fl, f, "\n Could not XC_READMEMBUF Rc = %i", rc);
                     rc = 0;
                     break;
                  }
   
               }
               if (!i) {
                  ddPrintf(pDD->fl, f, "\n Could not XC_READMEMBUF lCmd = %i", ddb.lCmd);
               }
               free(pStack);
            } else {
               ddPrintf(pDD->fl, f, "\n No Stack");
            }
         }
      } 


   } else {
      ddPrintf(pDD->fl, f, "\n Unknown threadID");
   }

//   pDD->fDumping = FALSE;
   if (tidShow) {
      DosKillThread(tidShow);
   } 

   return (int) rc;
}

VOID APIENTRY thrDDShow(ULONG ul)
{
   HPOINTER ahptr[4];
   int      i;
   BOOL     fTrue = TRUE;

   ul       = 0;
   i        = 0;
   ahptr[0] = WinQuerySysPointer(HWND_DESKTOP, SPTR_SIZENWSE, FALSE);
   ahptr[1] = WinQuerySysPointer(HWND_DESKTOP, SPTR_SIZEWE,   FALSE);
   ahptr[2] = WinQuerySysPointer(HWND_DESKTOP, SPTR_SIZENESW, FALSE);
   ahptr[3] = WinQuerySysPointer(HWND_DESKTOP, SPTR_SIZENS,   FALSE);

   while (fTrue) {
      WinSetPointer(HWND_DESKTOP, ahptr[i++]);
      if (i == 4) {
         i = 0;
      } 
      DosSleep(200); 
   } 

   return;
}

int DumpStack2File(PDD pDD, const PDDBUF pddb, ULONG ulESP, PBYTE pb, int cb)
{
   FILE * f;
   int    i,j, c;
   PULONG pul;
   ULONG  esp;
   CHAR   szBuf[512];
   PSZ    psz = "\n                    0 1 2 3  4 5 6 7  8 9 A B  C D E F"
                " 0123456789ABCDEF";
   ULONG  csz;
   char   asz[4][256];
   int    hit;
   PCHAR  pch;
   int    ich;


   if (pDD->cDumped) {
      return 0;
   } 

   f = fopen(pDD->pszStack, "a");
   if (f) {
      DumpModule(pDD, pddb, pddb->ulAddr, szBuf, NULL);
      fprintf(f, "%s", szBuf);
      DumpReg(pDD, pddb, pddb->ulAddr, szBuf, NULL);
      fprintf(f, "\n%s", szBuf);

      fprintf(f,
         "\n\n%08X %8i  bytes deep\n%s",
         cb, cb, psz);


//      i = DumpFirst(f, pb, ulESP, pDD);
//      esp += i;
      c = i = 0;
      esp   = ulESP & 0xFFFFFFF0;
      esp  += 16;

      i    += esp - ulESP;
      c    += 16;

      fprintf(f, "\n%08lX %08lx ", esp, c);

      csz = 0;
      while (i < cb) {

         pul = (PULONG) &pb[i];
         if (*pul) {
            fprintf(f, " %08X", *pul);
         } else {
            fprintf(f, "         ");
         }
         esp += 4;
         i   += 4;

         if (!(esp & 0x0000000F)) {
            fprintf(f, " ");
            i -= 16;
            for (j = 0; j < 16; j++) {
               if (pb[i]) {
                  if (pb[i] < 32 || pb[i] > 126) {
                     fprintf(f, ".");
                  } else {
                     fprintf(f, "%c", pb[i]);
                  }

               } else {
                  fprintf(f, " ");
               }
               i++;
            }
            for (j = 0; j < csz; j++) {
               fprintf(f, "\n  %.*s%s", j * 2, "        ", asz[j]);
            }

            csz = 0;

            if (!(esp & 0x000000FF)) {
               fprintf(f, "\n%s", psz);
            }
            c  += 16;
            fprintf(f, "\n%08lX %08lx ", esp, c);
         }
      }

      fprintf(f, "\n\n->\n");
      hit = 0;
      pch = malloc(4096);
      ich = 0;
      DumpAsso(pDD, pb, cb, szBuf, f, 0, &hit, ulESP, pch, &ich);
      free(pch);

      fclose(f);
   }
   return 0;
}

int isstring(PBYTE pb, int i, int cb)
{
   int c = 0;

   while (i < cb) {
      while (isprint(pb[i]) || isspace(pb[i])) {
         i++;
         c++;
      }
      if (pb[i]) {
         c = 0;
      } 
      break;
   } 
   return c;
}

int DumpAsso(PDD pDD, PBYTE pb, int cb, PSZ psz, FILE * f, 
   int level, int * phit, ULONG ulAddr, PCHAR pch, int * pich)
{
   int    i, c;
   PULONG pul;
   int    cch;
   BYTE   ab[80];
   APIRET rc = FALSE;
   DDBUF  ddb;
   PDDBUF pddb = &ddb;
   int    ich;

#define MAX_LEVEL 8

   //               fprintf(f, "\nlevel %i cb %i i %i", level, cb, i);
   ich = *pich;
   for (i = 0; i < cb; i += 4, ulAddr += 4) {
      pul    = (PULONG) &pb[i];
      cch    = 0;

      cch = isstring(pb, i, cb);
      if (cch > 2) {
         cch = cch > 80 ? 80 : cch;

         *pich += sprintf(pch + *pich, "\n%08X %08X %.*s\"%.*s\"",
            ulAddr, *pul, level * 2, "->->->->->->->->->", cch, &pb[i]);

         fprintf(f, "%s", pch);
         *pich = 0;

         cch    &= 0xFFFFFFFC;
         i      += cch;
         ulAddr += cch;
         *phit = cch;

         if (level) {
            return level;
         } 


      } else {
         cch = 0;
         if (*pul) {
            cch = DdiLineInfo(pDD->cLib, pDD->pLib, *pul, psz, DDI_DMPANYTHING);
         } 

         if (cch) {
            *pich += sprintf(pch + *pich, "\n%08X %08X %.*s %s",
               ulAddr, *pul, level * 2, "->->->->->->->->->", psz);

            *phit = 4;
   
            if (level < MAX_LEVEL && *psz != ' ') {
               for (c = 80; c; c--) {
                  ddb.ulPid = pDD->pid;
                  XC_READMEMBUF(*pul, ab, c);
                  if (!rc && ddb.lCmd == DBG_N_SUCCESS) {
                     break;
                  } 
               } 
               if (c) {
                  DumpAsso(pDD, ab, c, psz, f, level + 1, phit, *pul, pch, pich);

               } 
            } 
         } else {
            *pich = ich;
            if (level) {
               return level;
            }
         }
      }
   }
   return level;
}

BOOL IsCaller(PDD pDD, const PDDBUF pddb, ULONG ulEIP, FILE * f)
{
   #define  CALL_PTR 0x9A
   #define  CALL_REL 0xE8
   #define  CALL_REG 0xFF
   #define  CALL_MEM 0xFF
   #define  MAX_CALLEE  7

   USHORT   c;

   CHAR     pCallee[MAX_CALLEE] = {0, 0, 0, 0, 0, 0, 0};
   PCHAR    pulEIPCallee;
   DDBUF    ddb;
   APIRET   rc;

   pulEIPCallee = (PCHAR) ulEIP;
   pulEIPCallee -= MAX_CALLEE;

   for (c = MAX_CALLEE; c; c--) {
      XC_READMEMBUF((ULONG) pulEIPCallee + MAX_CALLEE - c,
         (ULONG) &pCallee[MAX_CALLEE - c], c);
      if (!rc && ddb.lCmd == DBG_N_SUCCESS) {
         break;
      }
   } 
   if (pDD->fl & EXEC_VERBOSE && c) {
      ddPrintf(pDD->fl, f,  "\n%02X%02X%02X%02X%02X%02X%02X",
         pCallee[0], pCallee[1], pCallee[2], pCallee[3],
         pCallee[4], pCallee[5], pCallee[6]);
   }
   if (c >= 5 && pCallee[2] == CALL_REL) {
      return TRUE;
   }
   if (c >= 6 && pCallee[1] == CALL_MEM) {
      return TRUE;
   }
   if (c == 7 && pCallee[0] == CALL_PTR) {
      return TRUE;
   }
   if (c >= 3 && pCallee[4] == CALL_REG) {
      return TRUE;
   }
   return FALSE;

}


PSZ ExplainException(PCHAR                  psz,
                     PEXCEPTIONREPORTRECORD pXReport,
                     PCONTEXTRECORD         pXContext)
{
   int i = 0;


   if (pXReport) {
      i += sprintf(psz + i, "%s ", szException(pXReport->ExceptionNum));

      switch (pXReport->ExceptionNum) {
      case XCPT_ACCESS_VIOLATION:
         if (pXReport->ExceptionInfo[0] == (ULONG) XCPT_DATA_UNKNOWN  ) {
            i += sprintf(psz + i, "DATA_UNKNOWN ");
         } else {
            if ((int)pXReport->ExceptionInfo[0] & XCPT_UNKNOWN_ACCESS) {
               i += sprintf(psz + i, "UNKNOWN_ACCESS ");
            }
            if ((int) pXReport->ExceptionInfo[0] & XCPT_READ_ACCESS   ) {
               i += sprintf(psz + i, "READ_ACCESS ");
            }
            if ((int) pXReport->ExceptionInfo[0] & XCPT_WRITE_ACCESS  ) {
               i += sprintf(psz + i, "WRITE_ACCESS ");
            }
            if ((int) pXReport->ExceptionInfo[0] & XCPT_EXECUTE_ACCESS) {
               i += sprintf(psz + i, "EXECUTE_ACCESS ");
            }
            if ((int) pXReport->ExceptionInfo[0] & XCPT_SPACE_ACCESS  ) {
               i += sprintf(psz + i, "SPACE_ACCESS ");
            }
            if ((int) pXReport->ExceptionInfo[0] & XCPT_LIMIT_ACCESS  ) {
               i += sprintf(psz + i, "LIMIT_ACCESS ");
            }
         }

         if (pXReport->ExceptionInfo[1] == (ULONG) XCPT_DATA_UNKNOWN  ) {
            if (pXReport->ExceptionInfo[0] != (ULONG) XCPT_DATA_UNKNOWN  ) {
               i += sprintf(psz + i, "DATA_UNKNOWN ");
            }
         } else {
            if ((int) pXReport->ExceptionInfo[1] & XCPT_UNKNOWN_ACCESS) {
               if ((int) pXReport->ExceptionInfo[0] & ~XCPT_UNKNOWN_ACCESS) {
                  i += sprintf(psz + i, "UNKNOWN_ACCESS ");
               }
            }
            if ((int) pXReport->ExceptionInfo[1] & XCPT_READ_ACCESS   ) {
               if ((int) pXReport->ExceptionInfo[0] & ~XCPT_READ_ACCESS   ) {
                  i += sprintf(psz + i, "READ_ACCESS ");
               }
            }
            if ((int) pXReport->ExceptionInfo[1] & XCPT_WRITE_ACCESS  ) {
               if ((int) pXReport->ExceptionInfo[0] & ~XCPT_WRITE_ACCESS  ) {
                  i += sprintf(psz + i, "WRITE_ACCESS ");
               }
            }
            if ((int) pXReport->ExceptionInfo[1] & XCPT_EXECUTE_ACCESS) {
               if ((int) pXReport->ExceptionInfo[0] & ~XCPT_EXECUTE_ACCESS) {
                  i += sprintf(psz + i, "EXECUTE_ACCESS ");
               }
            }
            if ((int) pXReport->ExceptionInfo[1] & XCPT_SPACE_ACCESS  ) {
               if ((int) pXReport->ExceptionInfo[0] & ~XCPT_SPACE_ACCESS  ) {
                  i += sprintf(psz + i, "SPACE_ACCESS ");
               }
            }
            if ((int) pXReport->ExceptionInfo[1] & XCPT_LIMIT_ACCESS  ) {
               if ((int) pXReport->ExceptionInfo[0] & ~XCPT_LIMIT_ACCESS  ) {
                  i += sprintf(psz + i, "LIMIT_ACCESS ");
               }
            }
            if (pXReport->ExceptionInfo[1] == (ULONG) XCPT_DATA_UNKNOWN  ) {
               if (pXReport->ExceptionInfo[0] == ~XCPT_DATA_UNKNOWN  ) {
                  i += sprintf(psz + i, "DATA_UNKNOWN ");
               }
            }
         }
         if (i) {
            i--;
            psz[i] = 0;
         }

         break;

      case XCPT_ASYNC_PROCESS_TERMINATE:
         i += sprintf(psz + i, "TerminatorTid %u ", pXReport->ExceptionInfo[0]);
         break;

      case XCPT_SIGNAL:
         switch (pXReport->ExceptionInfo[0]) {
         case XCPT_SIGNAL_INTR    :
            i += sprintf(psz + i, "       XCPT_SIGNAL_INTR ");
            break;
         case XCPT_SIGNAL_KILLPROC:
            i += sprintf(psz + i, "       XCPT_SIGNAL_KILLPROC ");
            break;
         case XCPT_SIGNAL_BREAK   :
            i += sprintf(psz + i, "       XCPT_SIGNAL_BREAK ");
            break;
         default:
            i += sprintf(psz + i, "       SignalNum %u ",
               pXReport->ExceptionInfo[0]);
            break;
         }

         break;

      default :
         break;
      }

   }
   if (pXContext) {
      i = i;
   }
   return psz;
}

PSZ szMte(USHORT cLib, PLIB aLib, HMODULE mte)
{
   int i;

   for (i = 0; i < cLib; i++) {
      if (aLib[i].mte == mte) {
         return aLib->sz;
      }
   }
   return "Invalid mte";
}

PCHAR szAsciiTimeStamp(void)
{
   long        lTime;
   struct tm * ptm;
   PSZ         psz;
   PCHAR       pch;

   time(&lTime);
   ptm = localtime(&lTime);
   pch = psz = asctime(ptm);
   while (*pch && *pch != '\n') {
      pch ++;
   }
   *pch = 0;
   return psz;
}


