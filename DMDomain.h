/*
Copyright (c) 2012 Unixmedia S.r.l. <info@unixmedia.it>
Copyright (c) 2012 Franco (nextime) Lanza <franco@unixmedia.it>

Domotika System Domain utils header [http://trac.unixmedia.it]

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

#ifndef __DMDOMAIN_H
#define __DMDOMAIN_H

#define DOMAIN_VALIDATION_MATCH
#define DOMAIN_VALIDATION_CONTROL
#define DOMAIN_VALIDATION_CONTROL_SAVE
#ifndef DOMAIN_MAXLEN
   #define DOMAIN_MAXLEN (32u)
#endif


#if defined(__GNUC__)
typedef unsigned char BYTE;
#endif

#if defined(DOMAIN_VALIDATION_MATCH) || defined(DOMAIN_VALIDATION_CONTROL) || defined(DOMAIN_VALIDATION_CONTROL_SAVE)
   #define DOMAIN_TYPE_MATCH (0x00)
   #define DOMAIN_TYPE_CONTROL (0x01)
   void ValidateDomainString(BYTE *DomainStr, BYTE DomainType, BYTE DomainLen);
#endif


BYTE DMDomainMatch(BYTE *ControlStr, BYTE *MatchStr, BYTE ControlLen);
#endif
