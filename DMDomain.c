/*
Copyright (c) 2012 Unixmedia S.r.l. <info@unixmedia.it>
Copyright (c) 2012 Franco (nextime) Lanza <franco@unixmedia.it>

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

   BYTE ControlStatus=DOMAIN_STATUS_HOME;
   BYTE ControlIdx=0u;
   BYTE ControlStartIdx=0u;
   BYTE DomainSaveStatus=DOMAIN_STATUS_HOME;
   BYTE DomainStatus=DOMAIN_CYCLE_CONTROL;
   BYTE MatchStatus=TRUE;
   BYTE MatchIdx=0u;
   BYTE MatchIdxSave=0u;
   BYTE flag=0u;
   BYTE specialflag=DOMAIN_NULL;
   BYTE SubStatus=SUB_STATUS_NONE;
   BYTE ControlSpecialIdx;
   BYTE MatchSpecialIdx;

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

      if((ControlIdx >= ControlLen || ControlStatus==DOMAIN_STATUS_END)
         && DomainStatus==DOMAIN_CYCLE_CONTROL)
      {
         DomainStatus=DOMAIN_CYCLE_MATCH;  
         DomainSaveStatus=ControlStatus;
         ControlStatus=DOMAIN_STATUS_END;
         #if defined(__GNUC__) && defined(DEBUG)
         printf("DMDomainMatch: CONTROL STRING IS OVER AT CYCLE START %d (%c)\n",
                ControlIdx, (unsigned int)*(ControlStr+ControlIdx));
         #endif

      }
      #if defined(__18CXX)
      if(strlen(MatchStr) <= MatchIdx)
      #else
      if(strlen((const char *)MatchStr) <= MatchIdx)
      #endif
      {
         #if defined(__GNUC__) && defined(DEBUG)
         printf("DMDomainMatch: MATCH STRING IS OVER AT CYCLE START %d (%c) %d %d\n", 
                ControlIdx, (unsigned int)*(ControlStr+ControlIdx), 
                (int)strlen((const char *)MatchStr), MatchIdx);
         #endif
         if(ControlStatus==DOMAIN_STATUS_END
            && (
              (DomainStatus==TRUE && ControlIdx==ControlStartIdx)
             || (DomainSaveStatus==DOMAIN_STATUS_NEGATE_PARSE && ControlIdx!=ControlStartIdx)
             || DomainSaveStatus==DOMAIN_STATUS_STAR
             || DomainSaveStatus==DOMAIN_STATUS_HOME
           )
         )
         {
            return TRUE;
         }
         if (ControlStatus==DOMAIN_STATUS_END 
             && DomainStatus==DOMAIN_CYCLE_MATCH)
            return MatchStatus;
         if(*(ControlStr+ControlIdx)==DOMAIN_SEPARATOR)
            return FALSE;
      }


      if(DomainStatus==DOMAIN_CYCLE_CONTROL)
      {
         if(MatchStatus==FALSE &&
            DomainSaveStatus==DOMAIN_STATUS_STAR)
         {
            #if defined(__GNUC__) && defined(DEBUG)
            printf("DMDomainMatch Control: Continue for STAR %d (%c)\n", 
                   ControlIdx, (unsigned int)*(ControlStr+ControlIdx));
            #endif 
            SubStatus=STAR_STATUS_CONTINUE;
         } else if ( MatchStatus==TRUE )
         {
            SubStatus=SUB_STATUS_NONE;
         }
         // execute the Control parse part
         switch(*(ControlStr+ControlIdx))
         {
            case DOMAIN_NULL:
               switch(ControlStatus)
               {
                  case DOMAIN_STATUS_HOME:
                  case DOMAIN_STATUS_STAR:
                  case DOMAIN_STATUS_LIST_STAR_STOP:
                  case DOMAIN_STATUS_EQUAL:
                  case DOMAIN_STATUS_NEGATE_PARSE:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: DOMAIN_STATUS_END %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx));
                     #endif 
                     DomainStatus=DOMAIN_CYCLE_MATCH;

                     DomainSaveStatus=ControlStatus;
                     ControlStatus=DOMAIN_STATUS_END;
                     break;

                  case DOMAIN_STATUS_LIST_STOP:
                  case DOMAIN_STATUS_LIST_NEGATE_STOP:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: DOMAIN_STATUS_END LIST %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx));
                     #endif
                     if(MatchStatus==DOMAIN_STATUS_LIST_TRUE)
                     {
                        if(ControlStatus==DOMAIN_STATUS_LIST_STOP)
                           ControlStartIdx=ControlIdx;
                        else
                           ControlStartIdx++;
                     }
                     DomainStatus=DOMAIN_CYCLE_MATCH;
                     DomainSaveStatus=ControlStatus;
                     ControlStatus=DOMAIN_STATUS_END;
                     break;

                  default:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: NULL DEFAULT %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx));
                     #endif

                     return FALSE;
               }
               break;

            case DOMAIN_SEPARATOR:
              switch(ControlStatus)
              {
                  case DOMAIN_STATUS_STAR:
                  case DOMAIN_STATUS_LIST_STOP:
                  case DOMAIN_STATUS_LIST_NEGATE_STOP:
                  case DOMAIN_STATUS_LIST_STAR_STOP:
                  case DOMAIN_STATUS_EQUAL:
                  case DOMAIN_STATUS_NEGATE_PARSE:
                    #if defined(__GNUC__) && defined(DEBUG)
                    printf("DMDomainMatch Control: DOMAIN_STATUS_SEPARATOR %d (%c)\n", 
                           ControlIdx, (unsigned int)*(ControlStr+ControlIdx));
                    #endif
                    DomainStatus=DOMAIN_CYCLE_MATCH;
                    DomainSaveStatus=ControlStatus;
                    ControlStatus=DOMAIN_STATUS_SEPARATOR;
                    break;

                  default:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: SEPARATOR DEFAULT %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx));
                     #endif

                     return FALSE;
              }
              break;

            case DOMAIN_STAR:
               switch(ControlStatus)
               {
                  case DOMAIN_STATUS_HOME:
                  case DOMAIN_STATUS_SEPARATOR:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: DOMAIN_STATUS_STAR %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx));
                     #endif

                     ControlStatus=DOMAIN_STATUS_STAR;
                     break;

                  case DOMAIN_STATUS_LIST_START:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: DOMAIN_STATUS_LIST_STAR %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx));
                     #endif

                     ControlStatus=DOMAIN_STATUS_LIST_STAR;
                     break;

                  default:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: STAR DEFAULT %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx));
                     #endif

                     return FALSE;
               }
               break;

            case DOMAIN_NEGATION:
               switch(ControlStatus)
               {
                  case DOMAIN_STATUS_HOME:
                  case DOMAIN_STATUS_SEPARATOR:
                    ControlStatus=DOMAIN_STATUS_NEGATE;
                    ControlStartIdx=ControlIdx+1u;
                    #if defined(__GNUC__) && defined(DEBUG)
                    printf("DMDomainMatch Control: DOMAIN_STATUS_NEGATE %d (%c)\n", 
                           ControlIdx, (unsigned int)*(ControlStr+ControlIdx));
                    #endif

                    break;

                  default:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: NEGATION DEFAULT %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx));
                     #endif

                     return FALSE;
               }
               break;
            case DOMAIN_LIST_START:
               switch(ControlStatus)
               {
                  case DOMAIN_STATUS_HOME:
                  case DOMAIN_STATUS_SEPARATOR:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: DOMAIN_STATUS_LIST_START %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx));
                     #endif

                     ControlStartIdx=ControlIdx+1u;
                     ControlStatus=DOMAIN_STATUS_LIST_START;
                     break;

                  case DOMAIN_STATUS_NEGATE:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: DOMAIN_STATUS_LIST_NEGATE_START %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx));
                     #endif

                     ControlStartIdx=ControlIdx+1u;
                     ControlStatus=DOMAIN_STATUS_LIST_NEGATE_START;
                     break;


                  default:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: LIST_START DEFAULT %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx));
                     #endif

                     return FALSE;
               }
               MatchStatus=FALSE;
               SubStatus=LIST_STATUS_START;
               break;

            case DOMAIN_LIST_STOP:
               switch(ControlStatus)
               {

                  case DOMAIN_STATUS_LIST_PARSE:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: DOMAIN_STATUS_LIST_STOP %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx));
                     #endif

                    ControlStatus=DOMAIN_STATUS_LIST_STOP;
                    break;
                  case DOMAIN_STATUS_LIST_STAR:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: DOMAIN_STATUS_LIST_STAR_STOP %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx));
                     #endif

                     ControlStatus=DOMAIN_STATUS_LIST_STAR_STOP;
                     break;
                  case DOMAIN_STATUS_LIST_NEGATE_PARSE:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: DOMAIN_STATUS_LIST_NEGATE_STOP %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx));
                     #endif

                     ControlStatus=DOMAIN_STATUS_LIST_NEGATE_STOP;
                     break;

                  default:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: LIST_STOP_DEFAULT %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx));
                     #endif

                     return FALSE;

               }
               break;

            case DOMAIN_LIST_SEPARATOR:
               switch(ControlStatus)
               {

                  case DOMAIN_STATUS_LIST_PARSE:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: DOMAIN_STATUS_LIST_SEPARATOR %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx));
                     #endif
                     ControlStatus=DOMAIN_STATUS_LIST_SEPARATOR;
                     if(MatchStatus==DOMAIN_STATUS_LIST_TRUE)
                     {
                        #if defined(__GNUC__) && defined(DEBUG)
                        printf("DMDomainMatch Control: DOMAIN_STATUS_LIST ALREADY TRUE AT SEPARATOR\n");
                        #endif
                        SubStatus=SUB_STATUS_NONE;
                     } else {
                        DomainSaveStatus=ControlStatus;
                        DomainStatus=DOMAIN_CYCLE_MATCH;
                     }
                     break;
                  case DOMAIN_STATUS_LIST_NEGATE_PARSE:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: DOMAIN_STATUS_LIST_NEGATE_SEPARATOR %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx));
                     #endif

                     ControlStatus=DOMAIN_STATUS_LIST_NEGATE_SEPARATOR;
                     if(MatchStatus==DOMAIN_STATUS_LIST_TRUE)
                     {
                        #if defined(__GNUC__) && defined(DEBUG)
                        printf("DMDomainMatch Control: DOMAIN_STATUS_NEGATE_LIST ALREADY TRUE AT SEPARATOR\n");
                        #endif
                        SubStatus=SUB_STATUS_NONE;
                     } else {
                        DomainSaveStatus=ControlStatus;
                        DomainStatus=DOMAIN_CYCLE_MATCH;
                     }
                     break;

                  default:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: LIST_SEPARATOR DEFAULT %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx)  );
                     #endif

                     return FALSE;
               }
               break;
            
            default:
               switch(ControlStatus)
               {
                  case DOMAIN_STATUS_HOME:
                  case DOMAIN_STATUS_SEPARATOR:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: DOMAIN_STATUS_EQUAL %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx)  );
                     #endif
                     ControlStatus=DOMAIN_STATUS_EQUAL;
                     ControlStartIdx=ControlIdx;
                     break;
                  case DOMAIN_STATUS_LIST_START:
                  case DOMAIN_STATUS_LIST_SEPARATOR:

                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: DOMAIN_STATUS_LIST_PARSE %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx)  );
                     #endif
                     ControlStatus=DOMAIN_STATUS_LIST_PARSE;
                     break;
                  case DOMAIN_STATUS_NEGATE:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: DOMAIN_STATUS_NEGATE_PARSE %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx)  );
                     #endif
                     ControlStatus=DOMAIN_STATUS_NEGATE_PARSE;
                     break;
                  case DOMAIN_STATUS_LIST_NEGATE_START:
                  case DOMAIN_STATUS_LIST_NEGATE_SEPARATOR:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: DOMAIN_STATUS_LIST_NEGATE_PARSE %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx)  );
                     #endif
                     ControlStatus=DOMAIN_STATUS_LIST_NEGATE_PARSE;
                     break;
         
                  case DOMAIN_STATUS_LIST_NEGATE_PARSE:
                  case DOMAIN_STATUS_NEGATE_PARSE:
                  case DOMAIN_STATUS_LIST_PARSE:
                  case DOMAIN_STATUS_EQUAL:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: CHAR PARSE %d (%c)\n", 
                            ControlIdx, (unsigned int)*(ControlStr+ControlIdx) );
                     #endif
                     break;


                  default:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Control: DEFAULT DEFAULT %d (%c)\n", ControlIdx,
                            (unsigned int)*(ControlStr+ControlIdx));
                     #endif
                     
                     return FALSE;

               }
         }

      } else {
         if(DomainStatus==DOMAIN_CYCLE_MATCH) 
         {
            #if defined(__GNUC__) && defined(DEBUG)
            printf("ENTERING MATCH CYCLE %d\n", DomainSaveStatus);
            #endif
            flag=FALSE;
            MatchStatus=FALSE;
            if(DomainSaveStatus==DOMAIN_STATUS_NEGATE_PARSE
               || DomainSaveStatus==DOMAIN_STATUS_LIST_NEGATE_SEPARATOR
               || DomainSaveStatus==DOMAIN_STATUS_LIST_NEGATE_STOP)
               DomainStatus=FALSE;
            else
               DomainStatus=TRUE;

            if(SubStatus==STAR_STATUS_CONTINUE)
            {
               #if defined(__GNUC__) && defined(DEBUG)
               printf("SAVING ControlStartIdx cause of STAR CONTINUE STATUS\n");
               #endif
               switch(DomainSaveStatus)
               {
                  case DOMAIN_STATUS_LIST_NEGATE_STOP:
                  case DOMAIN_STATUS_LIST_NEGATE_SEPARATOR:
                    #if defined(__GNUC__) && defined(DEBUG)
                    printf("NEGATION LIST PRECEEDED BY A STAR\n");
                    #endif
                  case DOMAIN_STATUS_NEGATE_PARSE:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("NEGATION PRECEEDED BY A STAR: SPECIAL CASE\n");
                     #endif
                     /*
                        - Count how many levels remain after the negation
                          on the control string
                        - count how many levels we have in the match string
                          to be parsed
                        - check if the negation string appears in the
                          difference between the control and the match remaining
                          levels, assuming the first is more than the second.
                     
                        at this point, if the result is found, return FALSE,
                        if the result isn't found, continue as usual. 
                     */

                     // Count how many levels there are in the Match string
                     // staring from one as we are at the next level compared
                     // comparing to the control 
                     SubStatus=1;
                     MatchSpecialIdx=MatchIdx;
                     while(MatchSpecialIdx<DOMAIN_MAXLEN)
                     {
                        if(*(MatchStr+MatchSpecialIdx)==DOMAIN_SEPARATOR)
                        {
                           SubStatus++;
                        } else if(*(MatchStr+MatchSpecialIdx)==DOMAIN_NULL)
                           break;
                        MatchSpecialIdx++;
                     }

                     // Substract the Control levels remaining after the negation
                     ControlSpecialIdx=ControlIdx;
                     specialflag=SubStatus;
                     while(ControlSpecialIdx<ControlLen)
                     {
                        if(!SubStatus)
                           break;
                        if(*(ControlStr+ControlSpecialIdx)==DOMAIN_SEPARATOR)
                        {
                           SubStatus--;
                        }
                        ControlSpecialIdx++;
                     }

                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("check negation for %d  levels \n", SubStatus);
                     #endif

                     if(specialflag==SubStatus && 
                        (DomainSaveStatus==DOMAIN_STATUS_NEGATE_PARSE ||
                         DomainSaveStatus==DOMAIN_STATUS_LIST_NEGATE_STOP))
                     {
                        // a special case in the special case.
                        // The negation is at the last level in the control string.
                        #if defined(__GNUC__) && defined(DEBUG)
                        printf("NEGATION IS IN THE LAST CONTROL LEVEL\n");
                        #endif

                        DomainSaveStatus=DOMAIN_STATUS_STAR_NEGATION_END;
                     }

                     // match all levels we have to check for negation
                     ControlSpecialIdx=ControlStartIdx;
                     MatchSpecialIdx=MatchIdx;
                     MatchStatus=TRUE;
                     specialflag=SubStatus;
                     while(SubStatus)
                     {
                        #if defined(__GNUC__) && defined(DEBUG)
                        printf("level %d: %c - %c\n", SubStatus,
                           (unsigned int)*(ControlStr+ControlSpecialIdx),
                           (unsigned int)*(MatchStr+MatchSpecialIdx));
                        #endif

                        if(*(ControlStr+ControlSpecialIdx)==DOMAIN_SEPARATOR
                           || *(ControlStr+ControlSpecialIdx)==DOMAIN_NULL
                           || ControlSpecialIdx>=ControlLen
                           || *(ControlStr+ControlSpecialIdx)==DOMAIN_LIST_SEPARATOR
                           || *(ControlStr+ControlSpecialIdx)==DOMAIN_LIST_STOP
                           || *(MatchStr+MatchSpecialIdx)==DOMAIN_SEPARATOR
                           || *(MatchStr+MatchSpecialIdx)==DOMAIN_NULL
                           || MatchSpecialIdx>=DOMAIN_MAXLEN)
                        {
                           #if defined(__GNUC__) && defined(DEBUG)
                           printf("level separator or terminator found\n");
                           #endif
                           if(MatchStatus
                                 && (
                                       (
                                          (*(MatchStr+MatchSpecialIdx)==DOMAIN_SEPARATOR
                                           || *(MatchStr+MatchSpecialIdx)==DOMAIN_NULL
                                           || MatchSpecialIdx>DOMAIN_MAXLEN)
                                       && (*(ControlStr+ControlSpecialIdx)==DOMAIN_SEPARATOR
                                           || *(ControlStr+ControlSpecialIdx)==DOMAIN_LIST_SEPARATOR
                                           || *(ControlStr+ControlSpecialIdx)==DOMAIN_LIST_STOP))
                                    ||
                                       (SubStatus<=1 &&
                                          (*(ControlStr+ControlSpecialIdx)==DOMAIN_NULL
                                          || ControlSpecialIdx>=ControlLen)
                                       )

                                    )
                              )
                           {
                              #if defined(__GNUC__) && defined(DEBUG)
                                 printf("NEGATION MATCH FOUND!\n");
                              #endif
                              return FALSE;
                           } 
                           else
                           {
                              if((*(ControlStr+ControlSpecialIdx)==DOMAIN_NULL
                                    || ControlSpecialIdx>=ControlLen)
                                 && SubStatus<=1)
                              {
                                 #if defined(__GNUC__) && defined(DEBUG)
                                    printf("NEGATION MATCH NOT FOUND AND CONTROL END!\n");
                                 #endif
                                 return TRUE;
                              }
                              MatchStatus=TRUE;
                           }
                           ControlSpecialIdx=ControlStartIdx;
                           SubStatus--;
                           if(*(MatchStr+MatchSpecialIdx)==DOMAIN_SEPARATOR)
                              MatchSpecialIdx++;
                           else
                           {
                              // Move Match cursor to the next level
                              while(TRUE)
                              {
                                 if(*(MatchStr+MatchSpecialIdx)==DOMAIN_SEPARATOR)
                                 {
                                    MatchSpecialIdx++;
                                    break;
                                 } else if(*(MatchStr+MatchSpecialIdx)==DOMAIN_NULL
                                            || MatchSpecialIdx>=DOMAIN_MAXLEN)
                                 {
                                    break;
                                 }
                                 MatchSpecialIdx++;
                              }
                           }
                        } else {
                           if(*(MatchStr+MatchSpecialIdx)!=*(ControlStr+ControlSpecialIdx)
                              && MatchStatus==TRUE)
                              MatchStatus=FALSE;
                           ControlSpecialIdx++;
                           MatchSpecialIdx++;
                        }
                        if(!SubStatus 
                           && (DomainSaveStatus==DOMAIN_STATUS_LIST_NEGATE_SEPARATOR
                              || DomainSaveStatus==DOMAIN_STATUS_LIST_NEGATE_STOP
                              )
                          )
                        {
                           #if defined(__GNUC__) && defined(DEBUG)
                           printf("Manage NEGATION LIST\n");
                           #endif
                           ControlSpecialIdx=ControlStartIdx;
                           while(TRUE)
                           {
                              if(*(ControlStr+ControlSpecialIdx)==DOMAIN_LIST_SEPARATOR)
                              {
                                 #if defined(__GNUC__) && defined(DEBUG)
                                 printf("LIST SEPARATOR, continue with negation list\n");
                                 #endif
                                 SubStatus=specialflag;
                                 MatchSpecialIdx=MatchIdx;
                                 ControlSpecialIdx++;
                                 break;
                              }
                              if(*(ControlStr+ControlSpecialIdx)==DOMAIN_LIST_STOP)
                              {
                                 #if defined(__GNUC__) && defined(DEBUG)
                                 printf("LIST STOP, done negation list\n");
                                 #endif
                                 ControlSpecialIdx++;
                                 DomainSaveStatus=DOMAIN_STATUS_LIST_NEGATE_STOP;
                                 break;
                              }
                              ControlSpecialIdx++;
                           }
                           //MatchSpecialIdx=MatchIdx;

                           if(ControlSpecialIdx>=ControlLen || *(ControlStr+ControlSpecialIdx)==DOMAIN_NULL)
                              DomainSaveStatus=DOMAIN_STATUS_STAR_NEGATION_END;
                           //else if(*(ControlStr+ControlSpecialIdx)==DOMAIN_SEPARATOR)
                           //   ControlSpecialIdx++;
                           ControlStartIdx=ControlSpecialIdx;
                           ControlIdx=ControlSpecialIdx;
                           // NEXTIME

                        } 
                     }
                     if(DomainSaveStatus==DOMAIN_STATUS_STAR_NEGATION_END)
                     {
                        #if defined(__GNUC__) && defined(DEBUG)
                        printf("CONTROL LAST LEVEL AND CONTROL END\n");
                        #endif
                        return TRUE;
                     }

                     // If we are here the special case is over,
                     // and we don't have a match.
                     // So, we must continue to the next level!

                     // Move MatchIdx to the right level
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("SPECIAL CASE IS OVER, MOVING INDICES TO GO BACK IN NORMAL MODE (%c)\n",
                              (unsigned int)*(ControlStr+ControlSpecialIdx));
                     #endif
                     while(TRUE)
                     {
                        if(MatchSpecialIdx==0
                           || *(MatchStr+MatchSpecialIdx)==DOMAIN_SEPARATOR)
                           break;
                        MatchSpecialIdx--;
                     }
                     MatchIdx=MatchSpecialIdx;

                     SubStatus=SUB_STATUS_NONE;

                     specialflag=ControlSpecialIdx;
                     MatchStatus=FALSE;
                     ControlStatus=DOMAIN_STATUS_SEPARATOR;
                     break;

                  default:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("NOT A SPECIAL CASE %d %d %d %d\n", ControlIdx, MatchIdx, ControlStartIdx, MatchSpecialIdx);
                     #endif
                     // Isn't a special case!
                     MatchStatus=FALSE;
                     specialflag=ControlStartIdx;
                     MatchIdxSave=MatchIdx;
                     //MatchIdx=MatchSpecialIdx;
               } 
            } //else if(SubStatus==LIST_STATUS_START)
            if(SubStatus==LIST_STATUS_START)
            {
               #if defined(__GNUC__) && defined(DEBUG)
               printf("SAVING MatchIdx cause of LIST_START STATUS\n");
               #endif
               //SubStatus=MatchIdx;
               MatchIdxSave=MatchIdx;
               
            }
         } 
         // execute the Match parse part
         switch(*(MatchStr+MatchIdx))
         {
            case DOMAIN_SEPARATOR:
               if((ControlStatus==DOMAIN_STATUS_SEPARATOR && DomainStatus==TRUE)
                  || (DomainSaveStatus==DOMAIN_STATUS_NEGATE_PARSE && ControlIdx!=ControlStartIdx)
                  || (DomainSaveStatus==DOMAIN_STATUS_STAR)
                 )
               {
                  #if defined(__GNUC__) && defined(DEBUG)
                  printf("DMDomainMatch Match: TRUE SEPARATOR %d %d %d \n", DomainStatus,
                         ControlStatus, DomainSaveStatus);
                  #endif
                  if(DomainSaveStatus==DOMAIN_STATUS_STAR)
                     MatchStatus=FALSE;
                  else
                  {
                     if(ControlStartIdx!=ControlIdx && DomainSaveStatus==DOMAIN_STATUS_EQUAL)
                     {
                        return FALSE;
                     }
                     MatchStatus=TRUE;
                  }
                  MatchIdx++;
                  DomainStatus=DOMAIN_CYCLE_CONTROL;
                  break;
               } else if(DomainStatus==TRUE
                      && ((DomainSaveStatus==DOMAIN_STATUS_LIST_SEPARATOR
                           && ControlIdx==ControlStartIdx)
                        || ( DomainSaveStatus==DOMAIN_STATUS_LIST_STOP
                           && ControlIdx==ControlStartIdx+1u))) 
               {
                  #if defined(__GNUC__) && defined(DEBUG)
                  printf("DMDomainMatch Match: LIST SEPARATOR/STOP TRUE %d %d %d %d\n", DomainStatus,
                         ControlIdx, ControlStartIdx, DomainSaveStatus);
                  #endif
                  if(*(ControlStr+ControlIdx)==DOMAIN_NULL  
                     || ControlIdx>=DOMAIN_MAXLEN) // XXX Uhmmm isn't there a better way?
                     return FALSE;
                  MatchStatus=DOMAIN_STATUS_LIST_TRUE;
                  DomainStatus=DOMAIN_CYCLE_CONTROL;
                  break;
               } else if((DomainSaveStatus==DOMAIN_STATUS_LIST_NEGATE_SEPARATOR
                            && (DomainStatus==TRUE || ControlIdx!=ControlStartIdx))
                         || (DomainSaveStatus==DOMAIN_STATUS_LIST_NEGATE_STOP
                            && (DomainStatus==TRUE || ControlIdx!=ControlStartIdx+1u))
                        )
               {
                  #if defined(__GNUC__) && defined(DEBUG)
                  printf("DMDomainMatch Match: SEPARATOR LIST SEPARATOR/STOP TRUE %d %d %d %d\n", DomainStatus,
                         ControlIdx, ControlStartIdx, DomainSaveStatus);
                  #endif
                  if(*(ControlStr+ControlIdx)==DOMAIN_NULL 
                     || ControlIdx>=DOMAIN_MAXLEN) // XXX Uhmmm isn't there a better way?
                     return FALSE;
                  DomainStatus=DOMAIN_CYCLE_CONTROL;
                  if(DomainSaveStatus!=DOMAIN_STATUS_LIST_NEGATE_STOP)
                     MatchIdx=MatchIdxSave;
                  else
                     MatchIdx++;
                  ControlStartIdx++;
                  break;

               } else if(ControlStatus==DOMAIN_STATUS_LIST_SEPARATOR)
               {
                  #if defined(__GNUC__) && defined(DEBUG)
                  printf("DMDomainMatch Match: SEPARATOR LIST SEPARATOR FALSE %d %d %d %d\n", DomainStatus,
                         ControlIdx, ControlStartIdx, DomainSaveStatus);
                  #endif
                  MatchIdx=MatchIdxSave;
                  DomainStatus=DOMAIN_CYCLE_CONTROL;
                  ControlStartIdx=ControlIdx+1u;
                  break;

               } else if(SubStatus==STAR_STATUS_CONTINUE)
               {
                  #if defined(__GNUC__) && defined(DEBUG)
                  printf("DMDomainMatch Match: FALSE SEPARATOR BUT STAR %d %d %d \n", DomainStatus,
                         ControlStatus, DomainSaveStatus);
                  #endif
                  MatchStatus=FALSE;
                  ControlStartIdx=specialflag;
                  if(DomainSaveStatus==DOMAIN_STATUS_NEGATE_PARSE)
                     DomainStatus=FALSE;
                  else
                     DomainStatus=TRUE;
               
                  break;
               
               }
               #if defined(__GNUC__) && defined(DEBUG)
               printf("DMDomainMatch Match: FALSE SEPARATOR %d %d %d %d %d %d %d\n", DomainStatus,
                   ControlStatus, DomainSaveStatus, SubStatus, ControlIdx, ControlStartIdx, MatchIdx);
               #endif
               return FALSE;


            case DOMAIN_NULL:
               #if defined(__GNUC__) && defined(DEBUG)
               printf("DMDomainMatch Match: NULL %d %d %d %d %d %d %d\n", 
                      DomainStatus, ControlStatus, DomainSaveStatus, SubStatus,
                      ControlIdx, ControlStartIdx, MatchIdx);
               #endif

               if(ControlStatus==DOMAIN_STATUS_END 
                 && (
                      (DomainStatus==TRUE && ControlIdx==ControlStartIdx)
                      || (DomainSaveStatus==DOMAIN_STATUS_NEGATE_PARSE && ControlIdx!=ControlStartIdx)
                      || DomainSaveStatus==DOMAIN_STATUS_STAR
                      || DomainSaveStatus==DOMAIN_STATUS_HOME
                    )  
                 )
               {
                  #if defined(__GNUC__) && defined(DEBUG)
                  printf("DMDomainMatch Match: NULL TRUE %d %d %d %d\n", DomainStatus, 
                         ControlIdx, ControlStartIdx, DomainSaveStatus);
                  #endif
                  return TRUE;
               } else if(DomainStatus==TRUE 
                      && ((DomainSaveStatus==DOMAIN_STATUS_LIST_SEPARATOR
                           && ControlIdx==ControlStartIdx)
                        || ( DomainSaveStatus==DOMAIN_STATUS_LIST_STOP
                           && ControlIdx==ControlStartIdx+1u)))
               { 
                  #if defined(__GNUC__) && defined(DEBUG)
                  printf("DMDomainMatch Match: NULL LIST SEPARATOR/STOP TRUE %d %d %d %d\n", DomainStatus,
                         ControlIdx, ControlStartIdx, DomainSaveStatus);
                  #endif
                  if(*(ControlStr+ControlIdx)==DOMAIN_NULL 
                     || ControlIdx>=DOMAIN_MAXLEN) // XXX Uhmmm isn't there a better way?
                     return TRUE;
                  MatchStatus=DOMAIN_STATUS_LIST_TRUE;
                  DomainStatus=DOMAIN_CYCLE_CONTROL;
                  break;
               } else if((DomainSaveStatus==DOMAIN_STATUS_LIST_NEGATE_SEPARATOR
                            && (DomainStatus==TRUE || ControlIdx!=ControlStartIdx))
                         || (DomainSaveStatus==DOMAIN_STATUS_LIST_NEGATE_STOP
                            && (DomainStatus==TRUE || ControlIdx!=ControlStartIdx+1u))
                        )
               {
                  #if defined(__GNUC__) && defined(DEBUG)
                  printf("DMDomainMatch Match: NULL LIST NEGATE SEPARATOR TRUE %d %d %d %d %d\n",
                         ControlStartIdx, ControlIdx, DomainSaveStatus, DomainStatus, MatchIdx);
                  #endif
                  if((*(ControlStr+ControlIdx)==DOMAIN_NULL || ControlIdx>=DOMAIN_MAXLEN) 
                     && DomainSaveStatus==DOMAIN_STATUS_LIST_NEGATE_STOP)
                  {
                    return TRUE;
                  }
                  //MatchStatus=DOMAIN_STATUS_LIST_TRUE;
                  DomainStatus=DOMAIN_CYCLE_CONTROL;
                  ControlStartIdx++;
                  if(DomainSaveStatus!=DOMAIN_STATUS_LIST_NEGATE_STOP)
                     MatchIdx=MatchIdxSave;
                  break;

               } else if(ControlStatus==DOMAIN_STATUS_LIST_SEPARATOR)
               {
                  #if defined(__GNUC__) && defined(DEBUG)
                  printf("DMDomainMatch Match: NULL LIST SEPARATOR FALSE %d %d %d %d\n", DomainStatus,
                         ControlIdx, ControlStartIdx, DomainSaveStatus);
                  #endif
                  MatchIdx=MatchIdxSave;
                  DomainStatus=DOMAIN_CYCLE_CONTROL;
                  ControlStartIdx=ControlIdx+1u;
                  break;
               }

               return FALSE;

            case DOMAIN_STAR:
            case DOMAIN_NEGATION:
            case DOMAIN_LIST_START:
            case DOMAIN_LIST_SEPARATOR:
            case DOMAIN_LIST_STOP:
               #if defined(__GNUC__) && defined(DEBUG)
               printf("DMDomainMatch Match: INVALID CHAR %d\n", DomainStatus);
               #endif

               return FALSE;

            default:
               switch(DomainSaveStatus)
               {
                  case DOMAIN_STATUS_LIST_STOP:
                  case DOMAIN_STATUS_LIST_SEPARATOR:
                  case DOMAIN_STATUS_EQUAL: 
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Match: DEFAULT  STATUS_EQUAL %d (%c) - %d (%c)\n", MatchIdx, 
                            (unsigned int)*(MatchStr+MatchIdx), 
                            ControlStartIdx, (unsigned int)*(ControlStr+ControlStartIdx));
                     #endif
                     if(*(MatchStr+MatchIdx)!=*(ControlStr+ControlStartIdx)) 
                         DomainStatus=FALSE;
                        //MatchStatus=FALSE;
                     ControlStartIdx++;
                     if(MatchIdx>=DOMAIN_MAXLEN-1)
                     {
                        if(DomainStatus==TRUE)
                           MatchStatus=TRUE;
                     }
                     break;

                  case DOMAIN_STATUS_LIST_NEGATE_STOP:
                  case DOMAIN_STATUS_LIST_NEGATE_SEPARATOR:
                  case DOMAIN_STATUS_NEGATE_PARSE:
                     if( ControlStartIdx>2u 
                           && DomainSaveStatus!=DOMAIN_STATUS_NEGATE_PARSE
                           && (*(ControlStr+ControlStartIdx-2u)==DOMAIN_LIST_SEPARATOR
                               || *(ControlStr+ControlStartIdx-2u)==DOMAIN_LIST_SEPARATOR)
                           && flag==FALSE)
                     {
                        ControlStartIdx--;
                     }
                     if(DomainSaveStatus!=DOMAIN_STATUS_NEGATE_PARSE)
                        flag=TRUE;
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Match: DEFAULT  STATUS_NEGATE_PARSE %d (%c) - %d (%c)\n", 
                            MatchIdx, (unsigned int)*(MatchStr+MatchIdx),  
                            ControlStartIdx, (unsigned int)*(ControlStr+ControlStartIdx));
                     #endif
                     if(*(MatchStr+MatchIdx)!=*(ControlStr+ControlStartIdx))
                     {
                        DomainStatus=TRUE;
                        if(ControlStartIdx>=ControlLen)
                          MatchStatus=TRUE;
                     }
                     ControlStartIdx++;
                     break;

                  case DOMAIN_STATUS_STAR:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Match: DEFAULT  STATUS_STAR %d (%c)- %d (%c)\n",
                            MatchIdx, (unsigned int)*(MatchStr+MatchIdx),  
                            ControlStartIdx, (unsigned int)*(ControlStr+ControlStartIdx));
                     #endif
                     DomainStatus=TRUE;
                     MatchStatus=FALSE;
                     if(ControlStatus==DOMAIN_STATUS_END)
                        return TRUE;
                     ControlStartIdx++;
                     break;

                  case DOMAIN_STATUS_LIST_STAR_STOP:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Match: DEFAULT LIST_STAR_STOP %d (%c)- %d (%c)\n",
                            MatchIdx, (unsigned int)*(MatchStr+MatchIdx),  
                            ControlStartIdx, (unsigned int)*(ControlStr+ControlStartIdx));
                     #endif
                     if(ControlStatus==DOMAIN_STATUS_END)
                        return TRUE;
                     MatchStatus=TRUE;
                     break;

                  /*
                  case DOMAIN_STATUS_LIST_NEGATE_STOP:

                  */
                  default:
                     #if defined(__GNUC__) && defined(DEBUG)
                     printf("DMDomainMatch Match: DEFAULT DEFAULT %d (%c) - %d (%c)\n",
                             MatchIdx, (unsigned int)*(MatchStr+MatchIdx),  
                             ControlStartIdx, (unsigned int)*(ControlStr+ControlStartIdx));

                     #endif

                     return FALSE;

               }

         }
      }
      if(DomainStatus==DOMAIN_CYCLE_CONTROL)
         ControlIdx++;
      else if(DomainStatus!=DOMAIN_CYCLE_MATCH)
         MatchIdx++;
   }
   #if defined(__GNUC__) && defined(DEBUG)
   printf("DMDomainMatch RETURN OUTSIDE MAIN LOOP\n");
   #endif
   return MatchStatus;

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

