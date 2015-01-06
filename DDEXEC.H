
#define MSGSIZE   1024
#define MAXPATH 256

#define EXEC_NORMAL   0x00000000
#define EXEC_TRACED   0X00000001
#define EXEC_DETACHED 0X00000002
#define EXEC_RELATED  0X00000004
#define EXEC_FULLSCR  0X00000008
#define EXEC_FOREGRD  0x00000010
#define EXEC_TERMQ    0x00000020
#define EXEC_INHERIT  0x00000040
#define EXEC_REDIR    0x00000080
#define EXEC_RESTART  0x00000100
#define EXEC_WAIT     0x00000200

#define EXEC_DDT      0x80000000
#define EXEC_VERBOSE  0X40000000
#define EXEC_SILENT   0x20000000

#define SIZE_PIPE     512

typedef struct _DDEXEC {
   ULONG hQueue;
   PSZ   pszDDD;
   ULONG ahPipe     [12];
} DDEXEC;
typedef DDEXEC * PDDEXEC;

#define hPipeStdin    DDExec.ahPipe[0]
#define hPipeStdout   DDExec.ahPipe[1]
#define hPipeStderr   DDExec.ahPipe[2]
#define hStdin        DDExec.ahPipe[3]
#define hStdout       DDExec.ahPipe[4]
#define hStderr       DDExec.ahPipe[5]

#define hPipe1Read    DDExec.ahPipe[6]
#define hPipe1Write   DDExec.ahPipe[7]
#define hPipe2Read    DDExec.ahPipe[8]
#define hPipe2Write   DDExec.ahPipe[9]
#define hPipe3Read    DDExec.ahPipe[10]
#define hPipe3Write   DDExec.ahPipe[11]

#define phPipeStdin   pDDExec->ahPipe[0]
#define phPipeStdout  pDDExec->ahPipe[1]
#define phPipeStderr  pDDExec->ahPipe[2]
#define phStdin       pDDExec->ahPipe[3]
#define phStdout      pDDExec->ahPipe[4]
#define phStderr      pDDExec->ahPipe[5]
        
#define phPipe1Read   pDDExec->ahPipe[6]
#define phPipe1Write  pDDExec->ahPipe[7]
#define phPipe2Read   pDDExec->ahPipe[8]
#define phPipe2Write  pDDExec->ahPipe[9]
#define phPipe3Read   pDDExec->ahPipe[10]
#define phPipe3Write  pDDExec->ahPipe[11]

APIRET ExecPgmTitle(PCHAR PgmTitle, PCHAR PgmPathExe, PCHAR PgmParms,
   PPID pPid, PULONG pSel, PPID pTid, USHORT fHow);

APIRET ddExec(PSZ pszTitle, PSZ pszExe, PSZ pszArgs, PPID pPid, PTID pTid,
   PDDEXEC pDDExec, ULONG lFlags);

// Specifications on EXEC_REDIR

// When specifying EXEC_REDIR two HFILEs are returned on the PgmTitle pointer
// being the pipe to read the output from the started program (stdout & stderr)
// and the pipe to write input to the program (stdin)
// It is necessary to provide a PgmTitle with at least a strlen of 3 for
// ExecPgmTitle to return the pipes (rc ERROR_MON_BUFFER_TOO_SMALL is returned
// otherwise)
// Since PgmTitle get's overwritten by the call, be sure not to pass a
// global or static variable as the title (it will not be the same on return)
// Use temp storage on the stack and copy the static/global string to this
// variable. This way the PgmTitle will be reusable
// EXEC_REDIR uses an intermediary CMD.EXE to provide redirection from stdin,
// stdout and stderr. When you have more than 5 files open on the time
// off the call the function cannot perform redirection and returns
// ERROR_TOO_MANY_OPEN_FILES (see IBM OS/2 Extended Edition Version 1.2
// User Guide Volume 1: Base Operating System p.9-3)

// EXEC_REDIR is compatible with EXEC_DETACHED and EXEC_TERMQ (tested)
// and should work (not tested) with all other flags except of course EXEC_WAIT

// The pipes will still be open even when the program dies, so it is necessary
// to check when a program has dies. This is done by EXEC_TERMQ and DosReadQueue
// when not specifying EXEC_DETACH, when EXEC_DETACHing the programmer needs
// to DosCWait the EXEC_DETACHed pid.

// see example redir.c at U:\KB\PTRACE

// Note, when EXEC_REDIRing a C-program it's advisable to setbuf(stdout), NULL)
// to prevent the C-runtime library from buffering the stdout-handle


