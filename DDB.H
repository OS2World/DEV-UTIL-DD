

typedef struct _DDBUF {
   ULONG     ulPid;
   ULONG     ulTid;
   LONG      lCmd;
   LONG      lValue;
   ULONG     ulAddr;
   ULONG     ulBuffer;
   ULONG     ulLen;
   ULONG     ulIndex;
   ULONG     ulMTE;
   ULONG     ulEAX;
   ULONG     ulECX;
   ULONG     ulEDX;
   ULONG     ulEBX;
   ULONG     ulESP;
   ULONG     ulEBP;
   ULONG     ulESI;
   ULONG     ulEDI;
   ULONG     ulEFlags;
   ULONG     ulEIP;
   ULONG     ulCSLim;
   ULONG     ulCSBase;
   UCHAR     ucCSAcc;
   UCHAR     ucCSAtr;
   USHORT    usCS;
   ULONG     ulDSLim;
   ULONG     ulDSBase;
   UCHAR     ucDSAcc;
   UCHAR     ucDSAtr;
   USHORT    usDS;
   ULONG     ulESLim;
   ULONG     ulESBase;
   UCHAR     ucESAcc;
   UCHAR     ucESAtr;
   USHORT    usES;
   ULONG     ulFSLim;
   ULONG     ulFSBase;
   UCHAR     ucFSAcc;
   UCHAR     ucFSAtr;
   USHORT    usFS;
   ULONG     ulGSLim;
   ULONG     ulGSBase;
   UCHAR     ucGSAcc;
   UCHAR     ucGSAtr;
   USHORT    usGS;
   ULONG     ulSSLim;
   ULONG     ulSSBase;
   UCHAR     ucSSAcc;
   UCHAR     ucSSAtr;
   USHORT    usSS;
} DDBUF;
typedef DDBUF *PDDBUF;


/*
 For a description of the parameters and returns for each DosDebug command, select a
 command:


 Command Number and Name      Description
 컴컴컴컴컴컴컴컴컴컴컴�      컴컴컴컴컴�

 0    DBG_C_Null              Null
 1    DBG_C_ReadMem           Read Word
 1    DBG_C_ReadMem_I         Read Word
 2    DBG_C_ReadMem_D         Read Word (same as 1)
 3    DBG_C_ReadReg           Read Register Set
 4    DBG_C_WriteMem          Write Word
 4    DBG_C_WriteMem_I        Write Word
 5    DBG_C_WriteMem_D        Write Word (same as 4)
 6    DBG_C_WriteReg          Write Register Set
 7    DBG_C_Go                Go
 8    DBG_C_Term              Terminate
 9    DBG_C_SStep             Single Step
 10   DBG_C_Stop              Stop
 11   DBG_C_Freeze            Freeze Thread
 12   DBG_C_Resume            Resume Thread
 13   DBG_C_NumToAddr         Object Number to Address
 14   DBG_C_ReadCoRegs        Read Coprocessor Registers
 15   DBG_C_WriteCoRegs       Write Coprocessor Registers
 16   Reserved                Reserved
 17   DBG_C_ThrdStat          Get Thread Status
 18   DBG_C_MapROAlias        Map Read-Only Alias
 19   DBG_C_MapRWAlias        Map Read-Write Alias
 20   DBG_C_UnMapAlias        Unmap Alias
 21   DBG_C_Connect           Connect to Debuggee
 22   DBG_C_ReadMemBuf        Read Memory Buffer
 23   DBG_C_WriteMemBuf       Write Memory Buffer
 24   DBG_C_SetWatch          Set Watchpoint
 25   DBG_C_ClearWatch        Clear Watchpoint
 26   DBG_C_RangeStep         Range Step
 27   DBG_C_Continue          Continue
 28   DBG_C_AddrToObject      Get Memory Object Information
 29   DBG_C_XchngOpcode       Exchange Opcode
 30   DBG_C_LinToSel          Translate Linear Address to
                                Segment:Offset
 31   DBG_C_SelToLin          Translate Segment:Offset to
                                Linear Address

 Not all fields must be defined for every DosDebug command.  The same field can have
 a different meaning in different DosDebug commands.  For each command, fields in the
 Debug Buffer structure that are not listed are not useful for that command, but may be
 modified by DosDebug as required.

 Error cases for commands are not listed.  The listed return values from commands are
 valid only if the DBG_N_Success notification is given.
*/

#define DBG_C_Null           0
#define DBG_C_ReadMem        1
#define DBG_C_ReadMem_I      1
#define DBG_C_ReadMem_D      2
#define DBG_C_ReadReg        3
#define DBG_C_WriteMem       4
#define DBG_C_WriteMem_I     4
#define DBG_C_WriteMem_D     5
#define DBG_C_WriteReg       6
#define DBG_C_Go             7
#define DBG_C_Term           8
#define DBG_C_SStep          9
#define DBG_C_Stop           10
#define DBG_C_Freeze         11
#define DBG_C_Resume         12
#define DBG_C_NumToAddr      13
#define DBG_C_ReadCoRegs     14
#define DBG_C_WriteCoRegs    15
#define Reserved             16
#define DBG_C_ThrdStat       17
#define DBG_C_MapROAlias     18
#define DBG_C_MapRWAlias     19
#define DBG_C_UnMapAlias     20
#define DBG_C_Connect        21
#define DBG_C_ReadMemBuf     22
#define DBG_C_WriteMemBuf    23
#define DBG_C_SetWatch       24
#define DBG_C_ClearWatch     25
#define DBG_C_RangeStep      26
#define DBG_C_Continue       27
#define DBG_C_AddrToObject   28
#define DBG_C_XchngOpcode    29
#define DBG_C_LinToSel       30
#define DBG_C_SelToLin       31


#define DBG_O_OBJMTE       0x10000000

PSZ aszDebugCommand[] =
{
  "DBG_C_Null",
//  "DBG_C_ReadMem",
  "DBG_C_ReadMem_I",
  "DBG_C_ReadMem_D",
  "DBG_C_ReadReg",
//  "DBG_C_WriteMem",
  "DBG_C_WriteMem_I",
  "DBG_C_WriteMem_D",
  "DBG_C_WriteReg",
  "DBG_C_Go",
  "DBG_C_Term",
  "DBG_C_SStep",
  "DBG_C_Stop",
  "DBG_C_Freeze",
  "DBG_C_Resume",
  "DBG_C_NumToAddr",
  "DBG_C_ReadCoRegs",
  "DBG_C_WriteCoRegs",
  "Reserved",
  "DBG_C_ThrdStat",
  "DBG_C_MapROAlias",
  "DBG_C_MapRWAlias",
  "DBG_C_UnMapAlias",
  "DBG_C_Connect",
  "DBG_C_ReadMemBuf",
  "DBG_C_WriteMemBuf",
  "DBG_C_SetWatch",
  "DBG_C_ClearWatch",
  "DBG_C_RangeStep",
  "DBG_C_Continue",
  "DBG_C_AddrToObject",
  "DBG_C_XchngOpcode",
  "DBG_C_LinToSel",
  "DBG_C_SelToLin"
};

/*
PCHAR szDebugCommand(LONG lCmd)
{
   if (lCmd > -1 && lCmd < 32) {
      return aszDebugCommand[lCmd];
   } else {
      return "DBG_C_ Invalid";

   }
}
*/

PCHAR szDebugComm(LONG lCmd)
{

   if (lCmd > -1 && lCmd < 32) {
      return &aszDebugCommand[lCmd][6];
   } else {
      return "Invalid szDebugCommand";

   }
}

PSZ aszProcTerm [] = {
   "TC_EXIT",
   "TC_HARDERROR",
   "TC_TRAP",
   "TC_KILLPROCESS",
   "TC_EXCEPTION"
};

PCHAR szProcTerm(ULONG lCmd)
{
   return aszProcTerm[lCmd];
}




/*
 For a description of the data returned with each notification, select a notification:


 Notification
 Number and Name               Description
 컴컴컴컴컴컴컴�               컴컴컴컴컴�
    Successful command completion
    Error detected during command
    Process termination -
      DosExitList done
    Exception detected
    Module loaded
    Coprocessor not in use error
    Thread termination -
      not in DosExitList
    Async Stop detected
    New Process started
    Alias needs to be freed
    Watchpoint hit
    Thread creation
    Module freed
    Range Step detected

 Note:  References to "IP" in the data return descriptions refer to the instruction pointer
        address.  This is the 32-bit equivalent of the CS:EIP instruction pointer,
        regardless of the CS selector.  This is also known as a linearized instruction
        pointer.


 Some notifications (such as DBG_N_ModuleLoad and DBG_N_Watchpoint) may require
 multiple returns to the debugger.  These additional pending notifications will be returned
 before the process being debugged can execute any more user code, and will be
 returned on the Go, SStep, or Stop commands.

 Note that more notifications might be pending at any time, so a debugger should be
 ready to handle any notification at any time that a Go, SStep, or Stop command is
 issued.

 If DosDebug returns ERROR_INTERRUPT after a command, the next notification might
 have been lost.  If the process being debugged was executing code at that time (via a
 Go, SStep, or RangeStep command), it will be stopped automatically.  To prevent this,
 DosDebug should not be used by thread 1 while signals are being used, or the
 debugger should issue DosEnterMustComplete before issuing the command.
*/



#define  DBG_N_SUCCESS        0
#define  DBG_N_ERROR         -1L
#define  TRC_C_SIG_ret       -2
#define  TRC_C_TBT_ret       -3
#define  TRC_C_BPT_ret       -4
#define  TRC_C_NMI_ret       -5
#define  DBG_N_ProcTerm      -6L

#define  DBG_N_Exception     -7L
#define  DBG_N_ModuleLoad    -8L
#define  DBG_N_CoError       -9L
#define  DBG_N_ThreadTerm   -10L

#define  DBG_N_AsyncStop    -11L
#define  DBG_N_NewProc      -12L
#define  DBG_N_AliasFree    -13L
#define  DBG_N_Watchpoint   -14L
#define  DBG_N_ThreadCreate -15L
#define  DBG_N_ModuleFree   -16L
#define  DBG_N_RangeStep    -17L



PSZ  aszDebugNotification[] =
{
   "DBG_N_SUCCESS     \0 Successful command completion",
//   "DBG_N_ERROR       \0 %s returns rc %08lX (%lu)",
   "DBG_N_ERROR       \0 ",
   "TRC_C_SIG_ret     \0 ?",
   "TRC_C_TBT_ret     \0 ?",
   "TRC_C_BPT_ret     \0 ?",
   "TRC_C_NMI_ret     \0 ?",
   "DBG_N_ProcTerm    \0 %-4i %-12s ExitList done",

   "DBG_N_Exception   \0\n           %s %08lX",
   "DBG_N_ModuleLoad  \0 %-4lu %s",
   "DBG_N_CoError     \0 Coprocessor not in use error",
   "DBG_N_ThreadTerm  \0 %-4i not in DosExitList",

   "DBG_N_AsyncStop   \0 Async Stop detected",
   "DBG_N_NewProc     \0 New Process started",
   "DBG_N_AliasFree   \0 Alias needs to be freed",
   "DBG_N_Watchpoint  \0 Watchpoint hit",
   "DBG_N_ThreadCreate\0 ",
   "DBG_N_ModuleFree  \0 %-4lu %s",
   "DBG_N_RangeStep   \0 Range Step detected"
};

PSZ szDebugNotification(LONG lCmd)
{
   if (lCmd < 1 && lCmd > -17) {
      return aszDebugNotification[-lCmd];
   } else {
      return "DBG_N_ Invalid";
   }
}

PSZ szDebugNotific(LONG lCmd)
{
   if (lCmd < 1 && lCmd > -17) {
      return &aszDebugNotification[-lCmd][19];
   } else {
      return "DBG_N_ Invalid";
   }
}


#define  DBG_X_PRE_FIRST_CHANCE 0x00000000
#define  DBG_X_FIRST_CHANCE     0x00000001
#define  DBG_X_LAST_CHANCE      0x00000002
#define  DBG_X_STACK_INVALID    0x00000003

PSZ  aszExceptValue[] =
{
   " PRE_FIRST_CHANCE",
   " FIRST_CHANCE",
   " LAST_CHANCE ",
   " STACK_INVALID"
};

PSZ szExceptValue(LONG lCmd)
{
   if (lCmd > -1 && lCmd < 4) {
      return aszExceptValue[lCmd];
   } else {
      return "DBG_X_INVALID";
   }
}

PSZ szException(ULONG lXcpt)
{
   PSZ psz;

        if ( XCPT_GUARD_PAGE_VIOLATION    ==   lXcpt)
      psz = "XCPT_GUARD_PAGE_VIOLATION";
   else if ( XCPT_UNABLE_TO_GROW_STACK    ==   lXcpt)
      psz = "XCPT_UNABLE_TO_GROW_STACK";
   else if ( XCPT_DATATYPE_MISALIGNMENT   ==   lXcpt)
      psz = "XCPT_DATATYPE_MISALIGNMENT";
   else if ( XCPT_BREAKPOINT              ==   lXcpt)
      psz = "XCPT_BREAKPOINT";
   else if ( XCPT_SINGLE_STEP             ==   lXcpt)
      psz = "XCPT_SINGLE_STEP";
   else if ( XCPT_ACCESS_VIOLATION        ==   lXcpt)
      psz = "XCPT_ACCESS_VIOLATION";
   else if ( XCPT_ILLEGAL_INSTRUCTION     ==   lXcpt)
      psz = "XCPT_ILLEGAL_INSTRUCTION";
   else if ( XCPT_FLOAT_DENORMAL_OPERAND  ==   lXcpt)
      psz = "XCPT_FLOAT_DENORMAL_OPERAND";
   else if ( XCPT_FLOAT_DIVIDE_BY_ZERO    ==   lXcpt)
      psz = "XCPT_FLOAT_DIVIDE_BY_ZERO";
   else if ( XCPT_FLOAT_INEXACT_RESULT    ==   lXcpt)
      psz = "XCPT_FLOAT_INEXACT_RESULT";
   else if ( XCPT_FLOAT_INVALID_OPERATION ==   lXcpt)
      psz = "XCPT_FLOAT_INVALID_OPERATION";
   else if ( XCPT_FLOAT_OVERFLOW          ==   lXcpt)
      psz = "XCPT_FLOAT_OVERFLOW";
   else if ( XCPT_FLOAT_STACK_CHECK       ==   lXcpt)
      psz = "XCPT_FLOAT_STACK_CHECK";
   else if ( XCPT_FLOAT_UNDERFLOW         ==   lXcpt)
      psz = "XCPT_FLOAT_UNDERFLOW";
   else if ( XCPT_INTEGER_DIVIDE_BY_ZERO  ==   lXcpt)
      psz = "XCPT_INTEGER_DIVIDE_BY_ZERO";
   else if ( XCPT_INTEGER_OVERFLOW        ==   lXcpt)
      psz = "XCPT_INTEGER_OVERFLOW";
   else if ( XCPT_PRIVILEGED_INSTRUCTION  ==   lXcpt)
      psz = "XCPT_PRIVILEGED_INSTRUCTION";
   else if ( XCPT_IN_PAGE_ERROR           ==   lXcpt)
      psz = "XCPT_IN_PAGE_ERROR";
   else if ( XCPT_PROCESS_TERMINATE       ==   lXcpt)
      psz = "XCPT_PROCESS_TERMINATE";
   else if ( XCPT_ASYNC_PROCESS_TERMINATE ==   lXcpt)
      psz = "XCPT_ASYNC_PROCESS_TERMINATE";
   else if ( XCPT_NONCONTINUABLE_EXCEPTION==   lXcpt)
      psz = "XCPT_NONCONTINUABLE_EXCEPTION";
   else if ( XCPT_INVALID_DISPOSITION     ==   lXcpt)
      psz = "XCPT_INVALID_DISPOSITION";
   else if ( XCPT_INVALID_LOCK_SEQUENCE   ==   lXcpt)
      psz = "XCPT_INVALID_LOCK_SEQUENCE";
   else if ( XCPT_ARRAY_BOUNDS_EXCEEDED   ==   lXcpt)
      psz = "XCPT_ARRAY_BOUNDS_EXCEEDED";
   else if ( XCPT_B1NPX_ERRATA_02         ==   lXcpt)
      psz = "XCPT_B1NPX_ERRATA_02";
   else if ( XCPT_UNWIND                  ==   lXcpt)
      psz = "XCPT_UNWIND";
   else if ( XCPT_BAD_STACK               ==   lXcpt)
      psz = "XCPT_BAD_STACK";
   else if ( XCPT_INVALID_UNWIND_TARGET   ==   lXcpt)
      psz = "XCPT_INVALID_UNWIND_TARGET";
   else if ( XCPT_SIGNAL                  ==   lXcpt)
      psz = "XCPT_SIGNAL";
   else
      psz = "XCPT_UNKNOWN";

   return psz;
}




/*
    DosDebug returns the following values:

    0        NO_ERROR
    87       ERROR_INVALID_PARAMETER
    95       ERROR_INTERRUPT
    115      ERROR_PROTECTION_VIOLATION

*/


