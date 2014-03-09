/*
Copyright (c) 2012-2014 Unixmedia S.r.l. <info@unixmedia.it>
Copyright (c) 2012-2014 Franco (nextime) Lanza <franco@unixmedia.it>

Domotika System Domain utils [http://trac.unixmedia.it]

This file is part of DMDomain.

DMDomain is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __DMDOMAIN_C
#define __DMDOMAIN_C

#if defined(__GNUC__)
#include <string.h>
#include <stdio.h>

#define PRN_BOOL(VAR) (VAR ? "True" : "False")
#endif


#if defined(__18CXX)
   #include "DomotikaIncludes.h"
#endif

//#if defined(__GNUC__)
//typedef unsigned char BYTE;
//#endif


#include "DMDomain.h"


#ifndef FALSE
   #define FALSE 0
#endif

#ifndef TRUE
   #define TRUE  1U
#endif

#define DOMAIN_NEGATION (0x21) // !
#define DOMAIN_LIST_START (0x5b) // [
#define DOMAIN_LIST_STOP (0x5d) // ]
#define DOMAIN_LIST_SEPARATOR (0x7c) // |

#define DOMAIN_SEPARATOR (0x2e) // .
#define DOMAIN_NULL (0x00) // \0
#define DOMAIN_STAR (0x2a) // *

#define DOMAIN_STATUS_HOME (0x00)
#define DOMAIN_STATUS_STAR (0x01)
#define DOMAIN_STATUS_LIST_START (0x02)
#define DOMAIN_STATUS_LIST_STOP (0x03)
#define DOMAIN_STATUS_LIST_SEPARATOR (0x04)
#define DOMAIN_STATUS_LIST_PARSE (0x05)
#define DOMAIN_STATUS_EQUAL (0x06)
#define DOMAIN_STATUS_NEGATE (0x07)
#define DOMAIN_STATUS_NEGATE_PARSE (0x08)
#define DOMAIN_STATUS_SEPARATOR (0x09)
#define DOMAIN_STATUS_LIST_STAR (0x0a)
#define DOMAIN_STATUS_LIST_STAR_STOP (0x0b)
#define DOMAIN_STATUS_LIST_NEGATE_START (0x0c)
#define DOMAIN_STATUS_LIST_NEGATE_PARSE (0x0d)
#define DOMAIN_STATUS_LIST_NEGATE_STOP (0x0e)
#define DOMAIN_STATUS_LIST_NEGATE_SEPARATOR (0x0f)
#define DOMAIN_STATUS_END (0x10)
#define DOMAIN_STATUS_LIST_TRUE (0x11)
#define DOMAIN_STATUS_STAR_NEGATION_END (0x12)


#define DOMAIN_CYCLE_MATCH (0xfe)
#define DOMAIN_CYCLE_CONTROL (0xff)



#define SUB_STATUS_NONE (0xff)
#define STAR_STATUS_CONTINUE (0xfe)
#define LIST_STATUS_START (0x0fd)
#define LIST_STATUS_CONTINUE (0x0fc)


#if defined(DOMAIN_VALIDATION_MATCH) || defined(DOMAIN_VALIDATION_CONTROL) || defined(DOMAIN_VALIDATION_CONTROL_SAVE)


void ValidateDomainString(BYTE *DomainStr, 
                          BYTE DomainType, 
                          BYTE DomainLen)
{

   BYTE DomainStatus=DOMAIN_STATUS_HOME;
   BYTE DomainIdx=0u;

   #if defined(DOMAIN_VALIDATION_CONTROL) || defined(DOMAIN_VALIDATION_CONTROL_SAVE)
   BYTE InvalidChar=FALSE;
   #endif


   if(DomainLen > DOMAIN_MAXLEN)
      DomainLen=DOMAIN_MAXLEN;


   #if defined(DOMAIN_VALIDATION_CONTROL) || defined(DOMAIN_VALIDATION_CONTROL_SAVE)

   if(DomainType==DOMAIN_TYPE_CONTROL) 
   {
     #if defined(__GNUC__) && defined(DEBUG)
     printf("Control Validation: START\n");
     #endif

      while(DomainLen > DomainIdx)
      {
         switch(*(DomainStr+DomainIdx))
         {

            case DOMAIN_NULL:
               switch(DomainStatus)
               {
                  case DOMAIN_STATUS_HOME:
                  case DOMAIN_STATUS_STAR:
                  case DOMAIN_STATUS_LIST_STOP:
                  case DOMAIN_STATUS_EQUAL:
                  case DOMAIN_STATUS_NEGATE_PARSE:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("Control Validation: NULL TERMINATION\n");
                     #endif
                     return;

                  default:
                     InvalidChar=TRUE;

              }
              break;

            case DOMAIN_SEPARATOR:
              switch(DomainStatus)
              {
                  case DOMAIN_STATUS_STAR:
                  case DOMAIN_STATUS_LIST_STOP:
                  case DOMAIN_STATUS_EQUAL:
                  case DOMAIN_STATUS_NEGATE_PARSE:
                    if(DomainIdx < DomainLen-1u)
                    {
                       DomainStatus=DOMAIN_STATUS_SEPARATOR;
                       #if defined(__GNUC__) && defined(DEBUG)
                       printf("Control Validation: SEPARATOR FOUND\n");
                       #endif
                       break;
                    }
                    InvalidChar=TRUE;
                    break;

                  default:
                    InvalidChar=TRUE;
              }
              break;

            case DOMAIN_STAR:
               switch(DomainStatus)
               {
                  case DOMAIN_STATUS_HOME:
                  case DOMAIN_STATUS_SEPARATOR:
                     if(DomainIdx>=2u && *(DomainStr+DomainIdx-2u)==DOMAIN_STAR)
                     {
                        #if defined(__GNUC__) && defined(DEBUG)
                        printf("Control Validation: DOUBLE STAR SHIFTING\n");
                        #endif
                        *(DomainStr+DomainIdx-2u)=DomainIdx+1u;
                        while(*(DomainStr+DomainIdx-2u) < DomainLen 
                              && *(DomainStr+*(DomainStr+DomainIdx-2u))!=DOMAIN_NULL)
                        {
                           printf("Control Validation: DOUBLE STAR SHIFTING %d\n", 
                                  *(DomainStr+DomainIdx-2u));
                           *(DomainStr+*(DomainStr+DomainIdx-2u)-2u)=*(DomainStr+*(DomainStr+DomainIdx-2u));
                           *(DomainStr+DomainIdx-2u)+=1u;
                        }
                        #if defined(__GNUC__) && defined(DEBUG)
                        printf("Control Validation: DOUBLE STAR SHIFTING END AT %d\n", 
                               *(DomainStr+DomainIdx-2u));
                        #endif
                        *(DomainStr+*(DomainStr+DomainIdx-2u)-2u)=DOMAIN_NULL;
                        DomainLen-=2;
                        *(DomainStr+DomainIdx-2u)=DOMAIN_STAR;
                        DomainIdx-=2;
                        // SHIFT ARRAY!!  SE C'E' UN'ALTRA STAR
                     }
                     DomainStatus=DOMAIN_STATUS_STAR;
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("Control Validation: STAR FOUND\n");
                     #endif

                     break;

                  case DOMAIN_STATUS_LIST_START:
                     if(DomainIdx < DomainLen-1u)
                     {
                        #if defined(__GNUC__) && defined(DEBUG)
                        printf("Control Validation: LIST STAR FOUND\n");
                        #endif

                        DomainStatus=DOMAIN_STATUS_LIST_STAR;
                        break;
                     }
                     InvalidChar=TRUE;
                     break;
                  
                  default:
                     InvalidChar=TRUE;
               }
               break;

            case DOMAIN_NEGATION:
               switch(DomainStatus)
               {
                  case DOMAIN_STATUS_HOME:
                  case DOMAIN_STATUS_SEPARATOR:
                     if(DomainIdx < DomainLen-1u)
                     {
                        DomainStatus=DOMAIN_STATUS_NEGATE;
                        #if defined(__GNUC__) && defined(DEBUG)
                        printf("Control Validation: NEGATE FOUND\n");
                        #endif
                        break;
                     }
                     InvalidChar=TRUE;
                     break;

                  default:
                    InvalidChar=TRUE;
               }
               break;

            case DOMAIN_LIST_START:
               switch(DomainStatus)
               {
                  case DOMAIN_STATUS_HOME:
                  case DOMAIN_STATUS_SEPARATOR:
                     if(DomainIdx < DomainLen-2u)
                     {
                        #if defined(__GNUC__) && defined(DEBUG)
                        printf("Control Validation: LIST START FOUND\n");
                        #endif
                        DomainStatus=DOMAIN_STATUS_LIST_START;
                        break;
                     }
                     InvalidChar=TRUE;
                     break;
                  
                  case DOMAIN_STATUS_NEGATE:
                     if(DomainIdx < DomainLen-3u)
                     {
                        #if defined(__GNUC__) && defined(DEBUG)
                        printf("Control Validation: LIST NEGATE START FOUND\n");
                        #endif

                        DomainStatus=DOMAIN_STATUS_LIST_NEGATE_START;
                        break;
                     }
                     InvalidChar=TRUE;
                     break;

                  default:
                     InvalidChar=TRUE;
               }
               break;

            case DOMAIN_LIST_STOP:
               switch(DomainStatus)
               {

                  case DOMAIN_STATUS_LIST_PARSE:
                  case DOMAIN_STATUS_LIST_STAR:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("Control Validation: LIST STOP FOUND\n");
                     #endif

                     DomainStatus=DOMAIN_STATUS_LIST_STOP;
                     break;

                  default:
                    InvalidChar=TRUE;

               }
               break;

            case DOMAIN_LIST_SEPARATOR:
               switch(DomainStatus)
               {

                  case DOMAIN_STATUS_LIST_PARSE:
                     if(DomainIdx < DomainLen-2u)
                     {
                        #if defined(__GNUC__) && defined(DEBUG)
                        printf("Control Validation: LIST SEPARATOR FOUND\n");
                        #endif

                        DomainStatus=DOMAIN_STATUS_LIST_SEPARATOR;
                        break;
                     }
                     InvalidChar=TRUE;
                     break;

                  default:
                    InvalidChar=TRUE;
               }
               break;

             default:
               switch(DomainStatus)
               {
                  case DOMAIN_STATUS_HOME:
                  case DOMAIN_STATUS_SEPARATOR:
                     DomainStatus=DOMAIN_STATUS_EQUAL;
                     break;
                  case DOMAIN_STATUS_LIST_START:
                  case DOMAIN_STATUS_LIST_NEGATE_START:
                  case DOMAIN_STATUS_LIST_SEPARATOR:
                     if(DomainIdx < DomainLen-1u)
                     {
                        DomainStatus=DOMAIN_STATUS_LIST_PARSE;
                        break;
                     }
                     InvalidChar=TRUE;
                     break;
                  case DOMAIN_STATUS_NEGATE:
                     if(DomainIdx < DomainLen)
                     {
                        DomainStatus=DOMAIN_STATUS_NEGATE_PARSE;
                        break;
                     }
                     InvalidChar=TRUE;
                     break;
                  case DOMAIN_STATUS_EQUAL:
                     break;

                  case DOMAIN_STATUS_NEGATE_PARSE:
                     if(DomainIdx < DomainLen)
                        break;
                     InvalidChar=TRUE;
                     break;

                  case DOMAIN_STATUS_LIST_PARSE:
                  case DOMAIN_STATUS_LIST_NEGATE_PARSE:
                     if(DomainIdx < DomainLen-1u)
                        break;
                     InvalidChar=TRUE;
                     break;

                  default:
                     InvalidChar=TRUE;

               }
         }
         if(InvalidChar)
         {
            switch(DomainStatus)
            {

               case DOMAIN_STATUS_LIST_SEPARATOR:
                  #if defined(__GNUC__) && defined(DEBUG)
                  printf("Control Validation: END WITH LIST_SEPARATOR\n");
                  #endif

                  *(DomainStr+DomainIdx-1)=DOMAIN_LIST_STOP;
                  *(DomainStr+DomainIdx)=DOMAIN_NULL;
                  return;

               case DOMAIN_STATUS_LIST_STAR:
               case DOMAIN_STATUS_LIST_PARSE:
                  #if defined(__GNUC__) && defined(DEBUG)
                  printf("Control Validation: END WITH LIST_PARSE or STAR\n");
                  #endif

                  *(DomainStr+DomainIdx)=DOMAIN_LIST_STOP;
                  if(DomainIdx+1u < DomainLen)
                     *(DomainStr+DomainIdx+1u)=DOMAIN_NULL;
                  return;

               case DOMAIN_STATUS_LIST_START:
               case DOMAIN_STATUS_NEGATE:
                  #if defined(__GNUC__) && defined(DEBUG)
                  printf("Control Validation: END WITH LIST_START or NEGATE\n");
                  #endif

                  if(DomainIdx >= 2u)
                     *(DomainStr+DomainIdx-2u)=DOMAIN_NULL;
                  else
                     *(DomainStr)=DOMAIN_NULL;
                  return;

               case DOMAIN_STATUS_SEPARATOR:
                  #if defined(__GNUC__) && defined(DEBUG)
                  printf("Control Validation: END WITH SEPARATOR\n");
                  #endif

                  *(DomainStr+DomainIdx-1)=DOMAIN_NULL;
                  return;

               case DOMAIN_STATUS_LIST_NEGATE_START:
                  #if defined(__GNUC__) && defined(DEBUG)
                  printf("Control Validation: END WITH NEGATE_START\n");
                  #endif

                  if(DomainIdx >= 3u)
                     *(DomainStr+DomainIdx-3u)=DOMAIN_NULL;
                  else
                     *(DomainStr)=DOMAIN_NULL;
                  return;

               default:
                  #if defined(__GNUC__) && defined(DEBUG)
                  printf("Control Validation: END WITH INVALID CHAR\n");
                  #endif

                  // DOMAIN_STATUS_HOME
                  // DOMAIN_STATUS_LIST_STOP
                  // DOMAIN_STATUS_EQUAL
                  // DOMAIN_STATUS_NEGATE_PARSE
                  //
                  *(DomainStr+DomainIdx)=DOMAIN_NULL;
                  return;

            }
         }
         DomainIdx++;
      }
      #if defined(__GNUC__) && defined(DEBUG)
      printf("Control Validation: END WITH 32 CHAR\n");
      #endif
   }
   #endif
   #if (defined(DOMAIN_VALIDATION_CONTROL) || defined(DOMAIN_VALIDATION_CONTROL_SAVE)) && defined(DOMAIN_VALIDATION_MATCH)
   else
   #endif
   #if defined(DOMAIN_VALIDATION_MATCH)
   if(DomainType==DOMAIN_TYPE_MATCH) {
     #if defined(__GNUC__) && defined(DEBUG)
      printf("Match Validation: START\n");
      #endif

      while(DomainLen > DomainIdx)
      {

         switch(*(DomainStr+DomainIdx))
         {
            case DOMAIN_STAR:
            case DOMAIN_NEGATION:
            case DOMAIN_LIST_START:
            case DOMAIN_LIST_SEPARATOR:
            case DOMAIN_LIST_STOP:
            case DOMAIN_NULL:
               if(DomainStatus==DOMAIN_STATUS_SEPARATOR && DomainIdx > 0)
               {
                  *(DomainStr+DomainIdx-1u)=DOMAIN_NULL;
                  #if defined(__GNUC__) && defined(DEBUG)
                  printf("Match Validation: END WITH SEPARATOR\n");
                  #endif
               } else {
                  *(DomainStr+DomainIdx)=DOMAIN_NULL;
                  #if defined(__GNUC__) && defined(DEBUG)
                  printf("Match Validation: END WITHOUT SEPARATOR\n");
                  #endif
               }
               return;

            case DOMAIN_SEPARATOR:
               if(DomainIdx==0u)
               {
                  #if defined(__GNUC__) && defined(DEBUG)
                  printf("Match Validation: FIRST CHAR SEPARATOR\n");
                  #endif
                  *(DomainStr+DomainIdx)=DOMAIN_NULL;
                  return;
               } else if(DomainStatus==DOMAIN_STATUS_SEPARATOR) {
                  #if defined(__GNUC__) && defined(DEBUG)
                  printf("Match Validation: DOUBLE SEPARATOR\n");
                  #endif
                  *(DomainStr+DomainIdx-1u)=DOMAIN_NULL;
                  return;
               } else {
                  if(DomainIdx==DomainLen-1u)
                  {
                     *(DomainStr+DomainIdx)=DOMAIN_NULL;
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("Match Validation: END WITH SEPARATOR\n");
                     #endif
                     return;
                  }
                  #if defined(__GNUC__) && defined(DEBUG)
                  printf("Match Validation: SEPARATOR FOUND\n");
                  #endif
                  DomainStatus=DOMAIN_STATUS_SEPARATOR;
               }
               break;
            
            default:
               DomainStatus=DOMAIN_STATUS_EQUAL;
         }
         DomainIdx++;
      }
   }
   #endif
}
#endif

BYTE DMDomainMatch( BYTE *ControlStr, 
                    BYTE *MatchStr,
                    BYTE ControlLen)
{

   BYTE DomainStatus=DOMAIN_CYCLE_CONTROL;
   BYTE MatchIdx=0u;
   BYTE ControlIdx=0u;
   BYTE mstart=0u;
   BYTE cstart=0u;
   BYTE mlen=0u;
   BYTE clen=0u;
   BYTE ctype=DOMAIN_STATUS_HOME;

   if(ControlLen > DOMAIN_MAXLEN)
      ControlLen=DOMAIN_MAXLEN;

   #if defined(__GNUC__)
   if(ControlLen==0 || strlen((const char *)MatchStr)==0)
      return FALSE;
   #else
   if(ControlLen==0 || strlen(MatchStr)==0)
      return FALSE;
   #endif

   #if defined(__GNUC__) && defined(DEBUG)
   printf("DMDomainMatch: START -> '%.*s', '%.*s'\n", (unsigned int)DOMAIN_MAXLEN, 
          ControlStr, (unsigned int)DOMAIN_MAXLEN, MatchStr);
   #endif

   while(ControlLen > ControlIdx || DOMAIN_MAXLEN > MatchIdx)
   {
      #if defined(__18CXX)
      ClrWdt();
      #endif
      if(DomainStatus==DOMAIN_CYCLE_CONTROL)
      {
         switch(*(ControlStr+ControlIdx))
         {
            case DOMAIN_NEGATION:
               if(ControlIdx==0u || *(ControlStr+ControlIdx-1)==DOMAIN_SEPARATOR)
               {
                  ctype=DOMAIN_STATUS_NEGATE;
               } else {
                  ctype=DOMAIN_STATUS_END;
                  DomainStatus==DOMAIN_CYCLE_MATCH;
               }
               break;

            case DOMAIN_LIST_START:
               if(ControlIdx==0u || *(ControlStr+ControlIdx-1)==DOMAIN_SEPARATOR)
               {
                  ctype=DOMAIN_STATUS_LIST_START;
               } else if(ControlIdx>0u && *(ControlStr+ControlIdx-1)==DOMAIN_NEGATION) {
                  ctype=DOMAIN_STATUS_LIST_NEGATE_START;
               } else {
                  ctype=DOMAIN_STATUS_END;
               }
         
            default:
               clen++;
   
         }
         
         ControlIdx++;
      }
      else if(DomainStatus==DOMAIN_CYCLE_MATCH)
      {
         MatchIdx++;
      }
   }
   #if defined(__GNUC__) && defined(DEBUG)
   printf("DMDomainMatch RETURN OUTSIDE LOOP\n");
   #endif
   return FALSE;

}
 
#if defined(__GNUC__) 

int main(int argc, char *argv[])
{

   BYTE IsValidMatchStr;

   if(argc < 3)
   {
      printf("Not enough parameters: argc = %d\n", argc);
      printf("usage: %s <ControlStr> <MatchStr>\n", argv[0]);
      return 1;
   }
   #if defined(DOMAIN_VALIDATION_CONTROL) || defined(DOMAIN_VALIDATION_CONTROL_SAVE)
   //ValidateDomainString((BYTE *)argv[1], (BYTE)DOMAIN_TYPE_CONTROL, (BYTE)DOMAIN_MAXLEN);
   ValidateDomainString((BYTE *)argv[1], (BYTE)DOMAIN_TYPE_CONTROL, (BYTE)strlen((const char *)argv[1]));
   #endif

   #if defined(DOMAIN_VALIDATION_MATCH)
   //ValidateDomainString((BYTE *)argv[2], (BYTE)DOMAIN_TYPE_MATCH, (BYTE)DOMAIN_MAXLEN);
   ValidateDomainString((BYTE *)argv[2], (BYTE)DOMAIN_TYPE_MATCH, (BYTE)strlen((const char *)argv[2]));
   #endif

   IsValidMatchStr = DMDomainMatch((BYTE *)argv[1], // control
                                   (BYTE *)argv[2], // match
                                   (BYTE)strlen((const char*)argv[1])); //DOMAIN_MAXLEN); 

   printf( "%s\n", PRN_BOOL( IsValidMatchStr ) );
   return 0;
}

#endif // __GNUC__

#endif // __DMDOMAIN_C_

