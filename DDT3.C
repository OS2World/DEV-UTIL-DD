#include "ddT4.c"

int ddT3(PSZ psz)
{
   DDTEntry

   PSZ  pszI = "Recursive call";
   CHAR szI[4096];
                           
   ddT3(psz);

   szI[0] = *pszI;
   DDTExit
}

int ddT6(PSZ psz)
{
   DDTEntry

   PSZ    pszI = "Float Divide by Zero";
   double i    = 0;

   i = i / i;

   pszI = pszI;
   DDTExit
}



