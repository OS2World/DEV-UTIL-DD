
#define SZ_EXITED    "Success"
#define SZ_TRAPPED   "General protection"
#define SZ_KILLED    "Process killed"
#define SZ_TERMINATE "terminating"

APIRET  DDbug(PID pid, PDDEXEC pDDExec, PTID pTid, ULONG fl, PSZ pszExe);


