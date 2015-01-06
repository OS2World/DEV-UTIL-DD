//#define INCL_DOSFILEMGR
#define INCL_DOSMODULEMGR
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "ddexec.h"
#include "ddi.h"

int   ParseArgs    (int * pcArg, int argc, char * argv[], PSZ pszExe, PSZ pszMap,
                    PULONG peip, PULONG plFlag, PDDEXEC pDDExec);

void  CrashTest    (ULONG ul);
void  CrashTest2   (ULONG ul);
void  Usage        (PSZ psz, PSZ pszFlag);


int main(int argc, char *argv[]);

int main(int argc, char *argv[])
{
   APIRET rc;
   CHAR   szMap[256];
   PSZ    pszArgs = szMap;
   ULONG  lFlag;
   ULONG  ulEIP;
   CHAR   sz   [1024];
   LIB    lib;
   int    cArg;
   int    cLib = 0;
   int    cBytes;
   ULONG  cSource;
   ULONG  cFunction;
   ULONG  cLine;
   ULONG  lddFlag = EXEC_TRACED | EXEC_WAIT | EXEC_FOREGRD | EXEC_INHERIT;
   PID    pid;
   TID    tid;
   DDEXEC DDExec;
//   ULONG  Void;


   setvbuf(stdout, NULL, _IONBF, 0);
   cArg          = 1;
   lFlag         = 0;
   cSource       = 0;
   cFunction     = 0;
   cLine         = 0;
   cBytes        = 0;
   lib.sz[0]     = 0;
   DDExec.pszDDD = NULL;

   while (ParseArgs(&cArg, argc, argv, lib.sz, szMap, &ulEIP, &lFlag, &DDExec)) {
      if (lFlag & DDI_DDI) {
         if (lFlag & DDI_BUILD) {
            if (DdiBuild(lib.sz, szMap, &lib.pDdi, lFlag)) {

               strcpy(sz, lib.sz);
               if (lFlag & DDI_WRITEDDI) {
                  Extention(sz, ".ddI");
               } 
               DdiWrite(sz, lib.pDdi);
               DdiFree(lib.pDdi);
            }
         }
         if (lFlag & DDI_SHOW) {
            DdiRead(1, &lib, DDI_READASYNC);
            DdiRead(1, &lib, DDI_READSYNC);
            if (lib.pDdi) {
               cLib++;
               printf("\n     %-32s", lib.sz);
               cBytes += DdiShow(lib.pDdi, lFlag, &cSource, &cFunction,
                  &cLine);
               DdiFree(lib.pDdi);
            }
         }
         if (lFlag & DDI_MAP) {
            DdiRead(1, &lib, DDI_READASYNC);
            DdiLineInfo(1, &lib, ulEIP, sz, DDI_MAP);
            if (*sz) {
               printf("\nEIP %08lX (%lu) maps to %s\n%s", 
                  ulEIP, ulEIP, lib.sz, sz);
            }
            DdiFree(lib.pDdi);
         }
         if (lFlag & DDI_MOVEDEBUGINFO) {
            if (!DdiHeader(DDI_MOVEDEBUGINFO, &lib, sz)) {
               printf("\n%s", sz);
            }
         }
         if (lFlag & DDI_ADDDEBUGINFO) {
            if (!DdiHeader(DDI_ADDDEBUGINFO, &lib, sz)) {
               printf("\n%s", sz);
            }
         }
         if (lFlag & DDI_DROPDEBUGINFO) {
            if (!DdiHeader(DDI_DROPDEBUGINFO, &lib, sz)) {
               printf("\n%s", sz);
            }
         }
         if (lFlag & DDI_CRASHTEST) {
            CrashTest(0);
         } 

      } else {
         if (lFlag & DDI_DDD) {
           DDExec.pszDDD = szMap;
         } 

         lddFlag |= (lFlag & 0xF0000000);

         sprintf(sz, "dd %s", lib.sz);
         rc = ddExec(sz, lib.sz, pszArgs, &pid, &tid, &DDExec, lddFlag);
         if (rc) {
            printf("\nCould not execute %s, rc = %u.", lib.sz, rc);
         }
      }

      lib.sz[0] = 0;
      lib.pDdi  = NULL;
   }

   if (lFlag & DDI_DDI && lFlag & DDI_SHOW && cLib > 1) {
      printf("\n\n     %-32s%5i Bytes", "Totals", cBytes);
      printf("\n%-5i Module%s",            cLib,      cLib      == 1 ? "" : "s");
      printf("\n%-5i Source%s",            cSource,   cSource   == 1 ? "" : "s");
      printf("\n%-5i Function%s",          cFunction, cFunction == 1 ? "" : "s");
      printf("\n%-5i Statement%s %s",      cLine,     cLine     == 1 ? "" : "s",
         cLine > 0 ? "or more" : "");
   }

#ifdef DUMP_ALLOC
   printf("\n");
   _dump_allocated(79);
#endif

   return 0;
}

#include "ddT.c"


int ParseArgs(int * pcArg, int argc, char * argv[], PSZ pszExe, PSZ pszMap,
              PULONG pulEIP, PULONG plFlag, PDDEXEC pDDExec)
{
   PCHAR  pch;
   int    i;
   PSZ    pszArgs = pszMap;
//   int   j;

   pszMap[0] = 0;
   pszExe[0] = 0;
//   *plFlag   = 0;

   for (i = *pcArg; argc > i && !pszExe[0]; i++) {
      switch (argv[i][0]) {
      case '-':
      case '/':
         strupr(argv[i]);
         if (*plFlag & DDI_DDI) {
            if (!strncmp(&argv[i][1], "B", 2)) {
               *plFlag |= DDI_BUILD;

            } else if (!strncmp(&argv[i][1], "BDDI", 2)) {
               *plFlag |= DDI_WRITEDDI;
               *plFlag |= DDI_BUILD;
   
            } else if (!strncmp(&argv[i][1], "MAPFILE:", 8)) {
               strcpy(pszMap, strupr(&argv[i][9]));


            } else if (!strncmp(&argv[i][1], "INFO", 5) ||
                       !strncmp(&argv[i][1], "I", 2)) {
   
               *plFlag |= DDI_SHOW;
               *plFlag |= DDI_VERBOSE;
   
            } else if (!strncmp(&argv[i][1], "CHECK", 6) ||
                       !strncmp(&argv[i][1], "C", 2)) {
   
               *plFlag |= DDI_SHOW;
               *plFlag |= DDI_CHECK;
   
   
            } else if (!strncmp(&argv[i][1], "DDEBUG", 6)) {
   
               *plFlag |= DDI_DROPDEBUGINFO;
   
            } else if (!strncmp(&argv[i][1], "MDEBUG", 6)) {

               *plFlag |= DDI_MOVEDEBUGINFO;

            } else if (!strncmp(&argv[i][1], "ADEBUG", 6)) {

               *plFlag |= DDI_ADDDEBUGINFO;

            } else if (!strncmp(&argv[i][1], "D", 2)) {
   
               *plFlag |= DDI_DELMAP;
   
            } else if (!strncmp(&argv[i][1], "T", 1)) {
   
               *plFlag |= DDI_CRASHTEST;

               ddT(argv[i]);

            } else if (!strncmp(&argv[i][1], "M:", 2)) {
               if (sscanf(&argv[i][3], "%lX", pulEIP) ||
                   sscanf(&argv[i][3], "%lu", pulEIP)) {
                  *plFlag |= DDI_MAP;
               } else {
                  Usage("Invalid ulEIP", argv[i]);
               }

            } else if (!strncmp(&argv[i][1], "?", 2)  ||
                       !strncmp(&argv[i][1], "H", 2)) {
               Usage(NULL, NULL);
   
            } else {
   
               Usage("Invalid flag", argv[i]);
            }

         } else {
            if (       !strncmp(&argv[i][1], "VERB", 5) ||
                       !strncmp(&argv[i][1], "V", 2)) {
   
               *plFlag |= EXEC_VERBOSE;
               *plFlag &= ~EXEC_SILENT;
/*
            } else if (!strncmp(&argv[i][1], "O", 1)) {
   
               *plFlag |= DDI_DDD;
               strcpy(pszMap, strupr(&argv[i][1]));
*/
            } else if (!strncmp(&argv[i][1], "T", 1) ) {
   
               *plFlag |= DDI_CRASHTEST;

               ddT(argv[i]);

            } else if (!strncmp(&argv[i][1], "F", 1)) {
               pDDExec->pszDDD = &argv[i][2];

            } else if (!strncmp(&argv[i][1], "S", 2)) {
   
               *plFlag |= EXEC_SILENT;
               *plFlag &= ~EXEC_VERBOSE;


            } else {
               Usage("Invalid flag", argv[i]);
            }
         } 
         break;

      case '?':
         Usage(NULL, NULL);
         break;

      default:
         if ((argv[i][0] == 'i' ||
             argv[i][0] == 'I') && !argv[i][1]) {

            *plFlag |= DDI_DDI;

         } else if ((argv[i][0] == 't' || argv[i][0] == 'T') &&
             !strstr(argv[i], ".") ) {

            if (!argv[i][1] && i + 1 < argc) {
               strcpy(pszExe, strupr(argv[++i]));
               *pszArgs = 0;
               i++;
               while (i < argc) {
                  strcat(pszArgs, argv[i++]);
                  strcat(pszArgs, " ");
               }
               if (*pszArgs) {
                  pszArgs[strlen(pszArgs) - 1] = 0;
               }

            } else {
   
               PID    pid;
               TID    tid;
//               LONG   l;
               CHAR   szExe  [20];
               CHAR   szArgs [80];
               CHAR   szTitle[80];
               BOOL   fDDD   = FALSE;
               DDEXEC DDExec;
   
               DDExec.pszDDD = NULL;
   
               if (argv[i][1] == 'T' || argv[i][1] == 't') {
                  fDDD = TRUE;
                  strcpy(szExe,    "cmd.exe");
                  strcpy(szArgs,   "/C copy dd.Exe ddd.exe");
                  sprintf(szTitle, "Desinfecting: %s %s", szExe, szArgs);
   
                  ddExec(szTitle, szExe, szArgs, &pid, &tid,
                     &DDExec, EXEC_TRACED | EXEC_WAIT | EXEC_FOREGRD |
                     EXEC_INHERIT | EXEC_DDT);
   
                  strcpy(szExe,    "dd.exe");
                  strcpy(szArgs,   "/I /b /MapFile:dd.map ddd");
                  sprintf(szTitle, "Desinfecting: %s %s", szExe, szArgs);
                  ddExec(szTitle, szExe, szArgs, &pid, &tid, 
                     &DDExec, EXEC_TRACED | EXEC_WAIT | EXEC_FOREGRD |
                     EXEC_INHERIT | EXEC_DDT);
               } 
   
               strcpy(szExe,    "dd.exe");
               DDExec.pszDDD = "ddT.ddD";
               if (fDDD) {
                  sprintf(szArgs, "ddd T dd /%s", &argv[i][1]);
               } else {
                  sprintf(szArgs, "/%s", argv[i]);
               } 
   
               sprintf(szTitle, "Desinfecting: %s %s", szExe, szArgs);
               ddExec(szTitle, szExe, szArgs, &pid, &tid, &DDExec,
                  EXEC_TRACED | EXEC_WAIT | EXEC_FOREGRD |
                  EXEC_INHERIT | EXEC_DDT);
   
               *pszExe = 0;
               *pcArg  = 0;

            } 
            *plFlag |= EXEC_DDT;

         } else {
            if (*plFlag & DDI_DDI) {
               strcpy(pszExe, strupr(argv[i]));
               pch = strstr(pszExe, ".EXE");
               if (!pch) {
                  if (!(pch = strstr(pszExe, ".DLL"))) {
                     if (!strstr(pszExe, ".")) {
                        strcat(pszExe, ".EXE");
                        pch = strstr(pszExe, ".");
                     }
                  }
               }
               if (!pszMap[0]) {
                  strcpy(pszMap, pszExe);
                  if (pch) {
                     strcpy(&pszMap[pch - pszExe], ".MAP");
                  } else {
                     strcat(pszMap, ".MAP");
                  } 
               }
               if (*plFlag == DDI_DDI) {
                  *plFlag |= DDI_CHECK;
                  *plFlag |= DDI_SHOW;
               }

            } else {
               strcpy(pszExe, strupr(argv[i++]));
               *pszArgs = 0;
               while (i < argc) {
                  strcat(pszArgs, argv[i++]);
                  strcat(pszArgs, " ");
               }
               if (*pszArgs) {
                  pszArgs[strlen(pszArgs) - 1] = 0;
               }
            } 
         } 
      }
   }

   if (!pszExe[0] && *pcArg == 1) {

      Usage(NULL, NULL);
   }
   *pcArg = i;

   return pszExe[0];
}

void CrashTest(ULONG ul)
{
   char  sz[1024];
   PVOID pVoid;

   pVoid = sz;
   sprintf(pVoid, "%lu %08lx", ul, ul);
   ul += 4;

   if (ul % 16) {
      CrashTest2(ul);
   } else {
      CrashTest(ul);
   }
   return;
}

void CrashTest2(ULONG ul)
{
   char  sz[1024];
   PVOID pVoid;

   pVoid = sz;
   sprintf(pVoid, "%lu %08lx", ul, ul);
   ul += 4;

   if (!(ul % 16)) {
      CrashTest2(ul);
   } else {
      CrashTest(ul);
   }
   return;
}


void Usage(PSZ psz, PSZ pszFlag)
{
   if (psz) {
      printf("\n%-10s %s", pszFlag, psz);
   } else {
      printf("\ndd DosDebugs at the sourcelevel");
      printf("\ndd I works on dd's sourcelevel-information");
      printf("\n");
   }

   printf("\nUsage    : dd   [/Flags]     <1[.exe]> [arg1] [arg2] [..]  DosDebug one");
   printf("\n           dd T [/Flags]     <1[.exe]> [arg1] [arg2] [..]  DosDebug & dump .ddT");
   printf("\n                [/F<file.ddD>]         Write trace to file.ddD");
   printf("\n                [/V[erbose]]           Verbose mode");
   printf("\n           dd Tn                       Desinfects dd.exe for 1 < n > 9");
   printf("\nOr       : dd I [/Flags]     <1[.exe]> [2.dll] [[/Flags] <?.dll>]");
   printf("\n                [/B]                   Build ddI");
   printf("\n                [/BDDI]                Build ddI and write .ddI");
   printf("\n                [/D]                   Delete .map after build");
   printf("\n                [/DDEBUG]              Delete DebugInfo from executable");
   printf("\n                [/MDEBUG]              Move DebugInfo to .deB");
   printf("\n                [/ADEBUG]              Add .deB to executable");
   printf("\n                [/C[heck]]             Check ddI");
   printf("\n                [/I[nfo]]              Display contents of ddI");
   printf("\n                [/MAPFILE:<file.map>]  Use file.map to build ddI");
   printf("\n                [/M:<[hex][dec]>]      Map EIP (hex/dec) in executable");

   exit(-1);
   return;
}
