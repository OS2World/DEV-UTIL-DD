#pragma pack(4)
 
#ifndef DDI_H
#define DDI_H

#define DDI_BUILD         0X00000001
#define DDI_SHOW          0X00000002
#define DDI_DELMAP        0x00000004
#define DDI_WRITEDDI      0x00000008

#define DDI_READASYNC     0X00000010
#define DDI_READSYNC      0X00000020
#define DDI_READDIRECT    0X00000040
//#define DDI_SYNCHRONIZE   0X00000080

#define DDI_BUILDADDR     0x00000100
#define DDI_CHECK         0x00000200
#define DDI_CRASHTEST     0x00000400

#define DDI_DMPMODULE     0x00001000
#define DDI_DMPINFO       0x00002000
#define DDI_MAP           0X00004000
#define DDI_DMPANYTHING   0X00008000
#define DDI_DDD           0X00010000

#define DDI_DDI           0x01000000

#define DDI_MOVEDEBUGINFO 0x00100000
#define DDI_DROPDEBUGINFO 0x00200000
#define DDI_ADDDEBUGINFO  0x00400000

#define DDI_DDT           0x80000000
#define DDI_VERBOSE       0x40000000
#define DDI_SILENT        0x20000000


#define DDI_LIBNOINFO  ((PDDI) -1)
#define DDI_LIBREADING ((PDDI) -2)

#define MAXPATH       256
#define ALLOC_INC     512
#define ALLOC_INC_LIB  64

#define PAGE_SIZE 4096
#define ONE_MEG   1024*1024
/*
#define NEED_COMMIT(size, count)                   \
   (size * (count + 1)) & (PAGESIZE - 1) > PAGE_SIZE - size \
      ? TRUE                                       \
      : FALSE
*/
typedef struct _ADDR {
   ULONG i;
   ULONG lAddr;
   ULONG lSize;
   ULONG lFlags;
} ADDR;
typedef ADDR * PADDR;

typedef struct _LIB {
   HMODULE mte;
   CHAR    sz   [MAXPATH];
   ULONG   cObj;
   ULONG   cAddr;
   PADDR   pAddr;
   PVOID   pDdi;
} LIB;
typedef LIB * PLIB;

PCHAR  Extention     (PSZ pszFile, PSZ pszExtention);
PVOID  mrMalloc      (int size);
PVOID  mrAlloc       (int size);
PVOID  mrRealloc     (PVOID p, int size);
APIRET mrFree        (PVOID p);
BOOL   NEED_COMMIT   (int size, int inc);

int    DdiLineInfo   (USHORT cLib, PLIB pLib, ULONG eip, PSZ psz, int i);
int    DdiRead       (USHORT cLib, PLIB pLib, ULONG fl);
int    DdiHeader     (ULONG fl, PLIB pLib, PSZ psz);

#if !defined(DDI_C)

int    DdiBuild      (PSZ pszExe, PSZ pszMap, PVOID * ppDdi, ULONG fl);
int    DdiWrite      (PSZ pszExe, PVOID pDdi);
int    DdiFree       (PVOID pDdi);
int    DdiShow       (PVOID pDdi, ULONG fl, PULONG pcSource,
                     PULONG pcFunction, PULONG pcLine);
int    DdiSourceInfo (PSZ pszInfo, PSZ pszFile);

#else


#define SZ_DDI_Q     "\\QUEUES\\DD\\ID\\%i"
#define SZ_DDI_SEM   "\\SEM32\\DD\\ID\\%i\\%i\\%s"

#define DDI_MAGIC "MR"                                    // Yours truly
#define SET_MAGIC(cookie)   memcpy((cookie), DDI_MAGIC, 2)
#define QUERY_MAGIC(cookie) (!memcmp((cookie), DDI_MAGIC, 2))

const CHAR ICC_MAGIC[4] = { 'N', 'B', '0', '4' };
#define SET_ICC_MAGIC(cookie)     memcpy((cookie), ICC_MAGIC, 4)
#define QUERY_ICC_MAGIC(cookie) (!memcmp((cookie), ICC_MAGIC, 4))


typedef struct _BYVALUE {
   USHORT sel;
   ULONG  eip;
   ULONG  iName;
} BYVALUE;
typedef BYVALUE * PBYVALUE;

typedef struct _LINE {
   USHORT iLine;
   ULONG  eip;
} LINE;
typedef LINE * PLINE;

typedef struct _SOURCE {
   ULONG    iName;
   ULONG    iLineBegin;
   ULONG    iLineEnd;
} SOURCE;
typedef SOURCE * PSOURCE;

typedef struct _DDI {
   CHAR     szMagic1[2];
   ULONG    cch;
   ULONG    cByValue;
   PBYVALUE pByValue;
   ULONG    cLine;
   PLINE    pLine;
   ULONG    cSource;
   PSOURCE  pSource;
   ULONG    cchNameSpace;
   PCHAR    pchNameSpace;
   ULONG    cName;
   PSZ    * aszName;
   CHAR     szMagic2[2];
   LONG     lOffset;
} DDI;
typedef DDI * PDDI;

int    DdiBuild     (PSZ pszExe, PSZ pszMap, PDDI * ppDdi, ULONG fl);
int    DdiWrite     (PSZ pszExe, PDDI pDdi);
int    DdiFree      (PDDI pDdi);
int    DdiShow      (PDDI pDdi, ULONG fl, PULONG pcSource,
                     PULONG pcFunction, PULONG pcLine);

#endif

#endif


