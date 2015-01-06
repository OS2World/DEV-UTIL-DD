typedef int ddTest (PSZ);
typedef ddTest *pddTest;

#define DDTEntry CHAR sz[80]; \
                 sprintf(sz, "%s %i <- %s", \
                    __FILE__, __LINE__, psz); \
                 {

#define DDTExit  } return 0;

#include "ddT1.c"

int ddT9(PSZ psz)
{
   PCHAR * ppch;
   PCHAR pch;

   pch  = psz;
   ppch = &pch;
   pch  = "*ppch /T9 strcat (*ppch, NULL);";
   strcat(*ppch, NULL);
   return 0;
}

int ddT8(PSZ psz)
{
   DDTEntry

   strcpy(sz, "DosExit(1, -1)");

   DosExit(1, -1);

   DDTExit

}

int ddT7(PSZ psz)
{
   DDTEntry

   HMODULE hmod;
   PSZ  pszI = NULL;

   DosLoadModule(sz, 80, "PMWIN", &hmod);
   DosLoadModule(sz, 80, "PMWIN", &hmod);
   DosLoadModule(sz, 80, "PMWIN", &hmod);

   strcpy(pszI, "strcpy(NULL, strcpy(NULL, strcpy(NULL,...");

   DDTExit
}

int ddT16(PSZ psz)
{
   DDTEntry

   HMODULE hmod;
   PSZ    pszI = NULL;
   APIRET rc;
   PFN    pfn;
 
   rc = DosLoadModule(sz, 80, "DDM16", &hmod);
   rc = DosQueryProcAddr(hmod, 0, "DDT16", &pfn);

   rc = pfn();

   strcpy(pszI, "strcpy(NULL, strcpy(NULL, strcpy(NULL,...");

   DDTExit
}


int ddT(PSZ psz)
{
   CHAR sz[80];
   pddTest addT[] = {
      NULL,
      ddT1,
      ddT2,
      ddT3,
      ddT4,
      ddT5,
      ddT6,
      ddT7,
      ddT8,
      ddT9,
      NULL, NULL, NULL, NULL, NULL, NULL,
      ddT16
   };
   int i;


   sprintf(sz, "%s %i <- %s", __FILE__, __LINE__, psz);

   sscanf(&psz[2], "%i", &i);
   addT[i](psz);

   return 0;
}



