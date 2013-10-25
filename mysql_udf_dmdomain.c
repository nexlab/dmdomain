/* Copyright (c) 2012, Franco (nextime) Lanza (nextime@nexlab.it). All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifdef STANDARD
/* STANDARD is defined, don't use any mysql functions */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef __WIN__
typedef unsigned __int64 ulonglong;	/* Microsofts 64 bit types */
typedef __int64 longlong;
#else
typedef unsigned long long ulonglong;
typedef long long longlong;
#endif /*__WIN__*/
#else
#include <my_global.h>
#include <my_sys.h>
#if defined(MYSQL_SERVER)
#include <m_string.h>		/* To get strmov() */
#else
/* when compiled as standalone */
#include <string.h>
#define strmov(a,b) stpcpy(a,b)
#define bzero(a,b) memset(a,0,b)
#endif
#endif
#include <mysql.h>
#include <ctype.h>

#ifdef _WIN32
/* inet_aton needs winsock library */
#pragma comment(lib, "ws2_32")
#endif

#ifdef HAVE_DLOPEN

#include "DMDomain.h"



my_bool DMDOMAIN_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
longlong DMDOMAIN(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);
void DMDOMAIN_deinit(UDF_INIT *initid);

typedef struct _DMDOMAINS
{
   BYTE FirstStr[DOMAIN_MAXLEN];
   unsigned long firstlen;
   BYTE SecondStr[DOMAIN_MAXLEN];
   unsigned long secondlen;
   int Reversed;
   int FirstIdx;
   int SecondIdx;
} DOMAIN_STRUCT;

my_bool DMDOMAIN_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
   
   DOMAIN_STRUCT DomainData;

   if (args->arg_count != 2 
      || args->arg_type[0] != STRING_RESULT
      || args->arg_type[1] != STRING_RESULT
      || (!args->args[1] && !args->args[0])
      )
   {
    strcpy(message,"DMDOMAIN require 2 string arguments -> DMDOMAIN(matchdomain,controldomain)");
    return 1;
   }
   initid->maybe_null=1;
   initid->decimals=0;
   initid->max_length=1;

   if(args->args[1])
   {
      // FirstStr is the Control with wildcards
      // SecondStr is the Match
      DomainData.Reversed=0;
      DomainData.FirstIdx=1;
      DomainData.SecondIdx=0;
   } else {
      // SecondStr is the Control with wildcards
      // FirstStr is the Match
      DomainData.Reversed=1;
      DomainData.FirstIdx=0;
      DomainData.SecondIdx=1;
   }
      

   if( !(initid->ptr = (char *) malloc( sizeof(DomainData) ))) {
      strcpy(message,"Couldn't allocate memory!");
      return 1;
   }

   DomainData.firstlen=args->lengths[DomainData.FirstIdx];
   if(DomainData.firstlen>DOMAIN_MAXLEN)
      DomainData.firstlen=DOMAIN_MAXLEN;
   else if(DomainData.firstlen<DOMAIN_MAXLEN)
      DomainData.FirstStr[DomainData.firstlen]='\0';
   memcpy((char*)DomainData.FirstStr,args->args[DomainData.FirstIdx],(size_t)DomainData.firstlen);   
   if(DomainData.Reversed) {
      ValidateDomainString((BYTE *)DomainData.FirstStr,
         (BYTE)DOMAIN_TYPE_MATCH, (BYTE)DomainData.firstlen);
   } else {
      ValidateDomainString((BYTE *)DomainData.FirstStr, 
            (BYTE)DOMAIN_TYPE_CONTROL, (BYTE)DomainData.firstlen);
   }
   //strcpy(message, (const char*)DomainData.FirstStr);
   //return 1;
   memcpy((char*)initid->ptr, (char*)&DomainData, sizeof(DomainData));
   return 0;
}

void DMDOMAIN_deinit(UDF_INIT *initid __attribute__((unused)))
{
   if (initid->ptr)
      free(initid->ptr);
}

longlong DMDOMAIN(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{

   DOMAIN_STRUCT DomainData;

   memcpy((char*)&DomainData, (char*)initid->ptr, sizeof(DomainData));
   DomainData.secondlen=args->lengths[DomainData.SecondIdx];
   if(DomainData.secondlen>DOMAIN_MAXLEN)
      DomainData.secondlen=DOMAIN_MAXLEN;
   else if(DomainData.secondlen<DOMAIN_MAXLEN)
      DomainData.SecondStr[DomainData.secondlen]='\0';
   memcpy((char*)DomainData.SecondStr,
      args->args[DomainData.SecondIdx], (size_t)DomainData.secondlen);
   if(DomainData.Reversed) {
      ValidateDomainString((BYTE *)DomainData.SecondStr, 
         (BYTE)DOMAIN_TYPE_CONTROL, (BYTE)DomainData.secondlen);
      if(DMDomainMatch((BYTE *)DomainData.SecondStr, (BYTE *)DomainData.FirstStr,
         (BYTE)strlen((const char*)DomainData.SecondStr)))
         return 1;

   } else {
      ValidateDomainString((BYTE *)DomainData.SecondStr,
         (BYTE)DOMAIN_TYPE_MATCH, (BYTE)DomainData.secondlen);
      if(DMDomainMatch((BYTE *)DomainData.FirstStr, (BYTE *)DomainData.SecondStr, 
         (BYTE)strlen((const char*)DomainData.FirstStr)))
         return 1;
   }
   return 0;
}


#endif /* HAVE_DLOPEN */
