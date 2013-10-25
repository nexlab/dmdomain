import struct
import _DMDomain
import copy


DOMAIN_TYPE_MATCH="\x00"
DOMAIN_TYPE_CONTROL="\x01"

def ValidateDomainString(DomainStr, DomainType, DomainLen):
   _DMDomain.ValidateDomainString(DomainStr, DomainType, struct.pack('<B', DomainLen))
   stend=DomainStr.find("\x00")
   if stend==-1:
      stend=len(DomainStr)
   if stend > 32:
      stend=32
   DomainStr=copy.deepcopy(DomainStr[0:stend])
   return DomainStr

def DMDomainMatch(ControlStr, MatchStr, ControlLen):
   return bool(struct.unpack('<B',
         _DMDomain.DMDomainMatch(ControlStr, MatchStr, struct.pack('<B',ControlLen)))[0])
