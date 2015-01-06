
#define INCL_DOSSESMGR
#define INCL_DOSMISC
#define INCL_DOSQUEUES
#define INCL_DOSPROCESS
#define INCL_DOSERRORS
#define INCL_DOSNDDIPES

#include <os2.h>

#include <string.h>
#include <stdio.h>
#include <io.h>

#include "ddexec.h"
#include "dd.h"

APIRET RedirPgm(PDDEXEC pDDExec, PCHAR szExe, PCHAR szInput,
   ULONG lFlags);


APIRET ddExec(PSZ pszTitle, PSZ pszExe, PSZ pszArgs, PPID pPid, PTID pTid,
   PDDEXEC pDDExec, ULONG lFlags)
{

#define CMD_FILE          0
#define NOT_WINDOWABLE    1
#define EXE_WINDOWABLE    2
#define PM_WINDOWABLE     3

   APIRET      rc = 0;
   ULONG       flFlag;
   RESULTCODES ResultCodes;
   ULONG       sid;
   PCHAR       pch;
   PHQUEUE     phQueue;
   ULONG       ulExecFlags;
   CHAR        szInput    [MAXPATH];
   CHAR        szExe      [MAXPATH];
   CHAR        szArgs     [MAXPATH];
   STARTDATA   SessionData;

   if (pDDExec) {
      phQueue = (PHQUEUE) &pDDExec->hQueue;
      *phQueue = 0;
   } else {
      phQueue = NULL;
   }
//   phQueue  = pVoid;
   *pPid    = 0;
   *pTid    = 0;

//   if (*phQueue) {
//      ddExec(pszTitle, pszExe, pszArgs, pPid, pTid, pVoid, lFlags);
//   }

   strcpy(szExe, pszExe);

   if (!strpbrk(szExe, "\\:")) {
      flFlag = SEARCH_IGNORENETERRS |
               SEARCH_CUR_DIRECTORY |
               SEARCH_ENVIRONMENT,
      strcpy(szArgs, "PATH");
   } else {
      pch = strrchr(szExe, '\\');
      if (pch) {
         pch++;
         strncpy(szArgs, szExe, (unsigned) (pch - szExe));
         szArgs[pch - szExe] = 0;
      } else {
         pch = strrchr(szExe, ':');
         pch++;
         strncpy(szArgs, szExe, (unsigned) (pch - szExe));
         szArgs[pch - szExe] = 0;
      }
      strcpy(szExe, pch);
      flFlag = 0;
   }
   if (!strstr(szExe, ".")) {
      strcat(szExe, ".EXE");
   }

   rc = DosSearchPath(flFlag, szArgs, szExe, szInput, MAXPATH);
   if (rc) {
      return rc;
   }
   strcpy(szExe, szInput);

 
   if (lFlags & EXEC_DETACHED ||
       lFlags & EXEC_REDIR    ||
       lFlags & EXEC_SILENT) {
      strcpy(szInput, szExe);
      strcpy(&szInput[strlen(szInput) + 1], pszArgs);

      if (lFlags & EXEC_REDIR) {
         rc = RedirPgm(pDDExec, szExe, szInput, lFlags);
         if (rc) {
            return rc;
         }
      }
      if (lFlags & EXEC_TRACED) {
         ulExecFlags  = EXEC_TRACE; //ASYNCRESULTDB;

      } else {
         if (lFlags & EXEC_DETACHED) {
            ulExecFlags  = EXEC_BACKGROUND;
         } else {
            ulExecFlags  = EXEC_ASYNCRESULT;
         }

      } 

      rc = DosExecPgm(szInput, MAXPATH,
         ulExecFlags, szInput, NULL, &ResultCodes, szExe);
      if (!rc) {
         *pPid = ResultCodes.codeTerminate;
      }

   } else {

      SessionData.PgmTitle    = pszTitle;

      DosQAppType((PSZ) szExe, &flFlag);

      flFlag &= 3;

      switch (flFlag) {
      case NOT_WINDOWABLE:
      case EXE_WINDOWABLE:
      case PM_WINDOWABLE:
        SessionData.PgmName     = szExe;
        SessionData.SessionType = (USHORT) flFlag;
        SessionData.PgmInputs   = pszArgs;
        break;
       
      case CMD_FILE:
        SessionData.PgmName     = 0;   /* Use CMD.EXE             */
        SessionData.SessionType = EXE_WINDOWABLE;

        strcpy(szInput, "/C ");        /* Auto Close after ending */
        strcat(szInput, szExe);
        strcat(szInput, " ");
        strcat(szInput, pszArgs);
        SessionData.PgmInputs   = szInput;
        break;

      default:
        SessionData.PgmName     = "";
        SessionData.SessionType = EXE_WINDOWABLE;

        strcpy(szInput, "/C ");
        strcat(szInput, pszArgs);
        SessionData.PgmInputs   = szInput;
        break;
      }

      SessionData.Length      = 50;

      if (lFlags & EXEC_RELATED) {
         SessionData.Related     = 1;   /* 1 related  */
      } else {
         SessionData.Related     = 0;   /* 0 independent */
      }

      if (lFlags & EXEC_FULLSCR) {
         SessionData.SessionType = 1;
      }

      if (lFlags & EXEC_FOREGRD) {
         SessionData.FgBg        = 0;      /* 0 foreground        */
      } else {
         SessionData.FgBg        = 1;      /* 1 background        */
      }

      if (lFlags & EXEC_TRACED) {
         SessionData.TraceOpt    = 1;      /* 1 trace             */
         SessionData.Related     = 1;      /* 1 related           */
//         lFlags |= EXEC_TERMQ;
         SessionData.FgBg        = 0;      /* 0 foreground        */
      } else {
         SessionData.TraceOpt    = 0;      /* 0 no trace          */
      }

      if (lFlags & EXEC_REDIR) {
         if (flFlag == PM_WINDOWABLE) {
            return ERROR_NOT_SUPPORTED;
         }
         if (SessionData.PgmInputs  == pszArgs) {
            strcpy(szInput, pszArgs);
            SessionData.PgmInputs  = szInput;
         }
         rc = RedirPgm(pDDExec, szExe, szInput, lFlags);
         if (rc) {
            return rc;
         }
         SessionData.InheritOpt  = 1;   // 0 inherit Shell - 1 calling process
         SessionData.Related     = 1;   // 1 related

      } else {
         SessionData.InheritOpt  = 0;   // 0 inherit Shell - 1 calling process
      }

      if (lFlags & EXEC_INHERIT) {
         SessionData.InheritOpt  = 1;   // 0 inherit Shell - 1 calling process
      } else {
         SessionData.InheritOpt  = 0;   // 0 inherit Shell - 1 calling process
      }

      SessionData.Environment = NULL;
      SessionData.IconFile    = NULL;   /* pszIncon                              */

      if (lFlags & EXEC_TERMQ) {

         PTIB         pTib;
         PPIB         pPib;
         CHAR        szTermQ[32];


         DosGetInfoBlocks(&pTib, &pPib);
         sprintf(szTermQ, "\\QUEUES\\EXECPROG\\%li", pPib->pib_ulpid);

         SessionData.TermQ       = szTermQ;

         SessionData.Related     = 1;      /* 1 related                              */

         if (!pDDExec) {
            rc = ERROR_MON_BUFFER_TOO_SMALL;
            return rc;
         }
         rc = DosCreateQueue(phQueue, 0, szTermQ);

      } else {
         SessionData.TermQ       = NULL;   /* pszTermQueue                          */
      }

      SessionData.PgmHandle   = 0;
      SessionData.PgmControl  = 0;
      SessionData.InitXPos    = 0;
      SessionData.InitYPos    = 0;
      SessionData.InitXSize   = 0;
      SessionData.InitYSize   = 0;

      if (!rc) {
         rc = DosStartSession((PSTARTDATA) &SessionData, &sid, pPid);
         if (rc == ERROR_SMG_START_IN_BACKGROUND && !SessionData.FgBg) {
            rc = 0;
         }
      }

   }
   if (lFlags & EXEC_REDIR) {
      rc = RedirPgm(pDDExec, NULL, NULL, lFlags);
   } 

   if (!rc && (lFlags & EXEC_TRACED)) {
      rc = DDbug(*pPid, pDDExec, pTid, lFlags, szExe);
   }


   return rc;
}

APIRET RedirPgm(PDDEXEC pDDExec, PCHAR szExe, PCHAR szInput, ULONG lFlags)
{
#define STDIN  0
#define STDOUT 1
#define STDERR 2
   APIRET   rc;
//   CHAR     sz1[256];
//   CHAR     sz2[256];
/*
   HFILE    hStdin  = STDIN;
   HFILE    hStdout = STDOUT;
   HFILE    hStderr = STDERR;
*/
   HFILE    Stdin;
   HFILE    Stdout;
   HFILE    Stderr;

   lFlags = 0;
   rc     = 0;
   if (!pDDExec) {
      rc = ERROR_MON_BUFFER_TOO_SMALL;
      return rc;
   }
   if (szExe) {
      phStdin  = 0xFFFF;
      phStdout = 0xFFFF;
      phStderr = 0xFFFF;
      Stdin    = STDIN;
      Stdout   = STDOUT;
      Stderr   = STDERR;
      rc = DosDupHandle(Stdin,  &phStdin);
      rc = DosDupHandle(Stdout, &phStdout);
      rc = DosDupHandle(Stderr, &phStderr);
      if (!(rc = DosCreatePipe(&phPipe1Read, &phPipe1Write, SIZE_PIPE)) &&
          !(rc = DosCreatePipe(&phPipe2Read, &phPipe2Write, SIZE_PIPE)) &&
          !(rc = DosCreatePipe(&phPipe3Read, &phPipe3Write, SIZE_PIPE))) {

         rc = DosClose(Stdin);
         rc = DosClose(Stdout);
         rc = DosClose(Stderr);

         rc = DosDupHandle(phPipe1Read,  &Stdin);
         rc = DosDupHandle(phPipe2Write, &Stdout);
         rc = DosDupHandle(phPipe3Write, &Stderr);

         rc = DosClose(phPipe1Read);
         rc = DosClose(phPipe2Write);
         rc = DosClose(phPipe3Write);

         phPipeStdin  = phPipe1Write;
         phPipeStdout = phPipe2Read;
         phPipeStderr = phPipe3Read;
      }
   } else {
      return rc;

   }
   return rc;
}

/*
      hPipe1Write = hPipeStdin;
      hPipe2Read  = hPipeStdout;
      hPipe3Read  = hPipeStderr;

      hPipeStdin  = 0xFFFF;
      hPipeStdout = 0xFFFF;
      hPipeStderr = 0xFFFF;
      rc = DosDupHandle(hPipe1Write,  &hPipeStdin);
      rc = DosDupHandle(hPipe2Read,   &hPipeStdout);
      rc = DosDupHandle(hPipe3Read,   &hPipeStderr);
      rc = DosClose(hPipe1Write);
      rc = DosClose(hPipe2Read);
      rc = DosClose(hPipe3Read);

      Stdin  = STDIN;
      Stdout = STDOUT;
      Stderr = STDERR;
      rc = DosClose(Stdin);
      rc = DosClose(Stdout);
      rc = DosClose(Stderr);
      rc = DosDupHandle(hStdin,  &Stdin);
      rc = DosDupHandle(hStderr, &Stdout);
      rc = DosDupHandle(hStderr, &Stderr);

      rc = DosClose(hStdin);
      rc = DosClose(hStdout);
      rc = DosClose(hStderr);

   } else {
      phStdin  = 0xFFFF;
      phStdout = 0xFFFF;
      phStderr = 0xFFFF;
      Stdin    = STDIN;
      Stdout   = STDOUT;
      Stderr   = STDERR;
      rc = DosDupHandle(Stdin,  &phStdin);
      rc = DosDupHandle(Stdout, &phStdout);
      rc = DosDupHandle(Stderr, &phStderr);
      if (!(rc = DosCreatePipe(&phPipe1Read, &phPipe1Write, SIZE_PIPE)) &&
          !(rc = DosCreatePipe(&phPipe2Read, &phPipe2Write, SIZE_PIPE)) &&
          !(rc = DosCreatePipe(&phPipe3Read, &phPipe3Write, SIZE_PIPE))) {

         rc = DosClose(Stdin);
         rc = DosClose(Stdout);
         rc = DosClose(Stderr);

         rc = DosDupHandle(phPipe1Read,  &Stdin);
         rc = DosDupHandle(phPipe2Write, &Stdout);
         rc = DosDupHandle(phPipe3Write, &Stderr);

         rc = DosClose(phPipe1Read);
         rc = DosClose(phPipe2Write);
         rc = DosClose(phPipe3Write);

         phPipeStdin  = phPipe1Write;
         phPipeStdout = phPipe2Read;
         phPipeStderr = phPipe3Read;

         if (lFlags & EXEC_SILENT) {
            Stdin  = -1;
            Stdout = -1;
            Stderr = -1;
            rc = DosDupHandle(STDIN,  &Stdin);
            rc = DosDupHandle(STDOUT, &Stdout);
            rc = DosDupHandle(STDERR, &Stderr);

            hStdin  = STDIN;
            hStdout = STDOUT;
            hStderr = STDERR;
            rc = DosDupHandle(hPipe1Read,  &hStdin);
            rc = DosDupHandle(hPipe2Write, &hStdout);
            rc = DosDupHandle(hPipe3Write, &hStderr);


         } else {
           if (phPipe1Read < 10 && phPipe2Write < 10) {
               rc = DosClose(phPipe3Read);
               rc = DosClose(phPipe3Write);
               if (lFlags & EXEC_DETACHED) {
                  strcpy(sz1, szExe);
                  strcpy(sz2, &szInput[strlen(szInput) + 1]);
                  strcpy(szExe, "C:\\OS2\\CMD.EXE");

                  strcpy(szInput, szExe);
                  sprintf(&szInput[strlen(szInput) + 1],
                     "0<&%i 1>&%i 2>&%i",
                     hPipe1Read, hPipe1Write, hPipe2Write);
   
               } else {
                  strcpy(sz1, szExe);
                  strcpy(sz2, szInput);
                  strcpy(szExe, "C:\\OS2\\CMD.EXE");
                  sprintf(szInput, "/C 0<&%i 2>&%i 1>&%i %s %s",
                     hPipe1Read, hPipe1Write, hPipe2Write, sz1, sz2);
               }
               printf("\n%s %s\n", szExe, szInput);
   
               hPipeStdout = hPipe2Read;
               hPipeStdin  = hPipe1Write;
   
            } else {
               rc = ERROR_TOO_MANY_OPEN_FILES;
            }

         } 
      }
   }

   return rc;
}
*/
