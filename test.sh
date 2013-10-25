#!/bin/bash 
##################################################################
# Copyright (c) 2012 Unixmedia S.r.l. <info@unixmedia.it>
# Copyright (c) 2012 Franco (nextime) Lanza <franco@unixmedia.it>
#
# Domotika System Domain unit tester [http://trac.unixmedia.it]
#
# This file is part of DMDomain.
#
# DMDomain is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
######################################################################

ERED='\e[1;31m'
NO_COLOR='\e[0m'
EGREEN='\e[1;32m'
EYELLOW='\e[1;33m'
EBLUE='\e[1;34m'
EMAGENTA='\e[1;35m'
ECYAN='\e[1;36m'
EWHITE='\e[1;37m'
DEBUG="n"

echo
echo -e "$EWHITE\t\t DOMOTIKA DOMAIN TESTER \t\t$NO_COLOR\n"


if [ -f Makefile ] ; then
   make clean
   make debug
   echo
fi

if [ \! -f ./dmdomain ] ; then
   echo "dmdomain binary  not found"
   exit 1
fi

TCOLS=$(stty -a | tr -s ';' '\n' | grep "column" | sed s/'[^[:digit:]]'//g)
PBMIN=0     # progressbar min value
PBMAX=100   # progressbar max value
PBSYM="="   # symbol used for building the progressbar
PBPERCLEN=7 # length of % string,  i.e.  "[100%] " => 7 chars

# create the progressbar
function GetPBLine()
{
    PBVAL=$1 # current progressbar value (function param)
    RVAL=$2
    PBUCOLS=$(($TCOLS-$PBPERCLEN-7)) # usable columns
    PBNUM=$((($PBVAL*$PBUCOLS)/($PBMAX-$PBMIN)))
    for((j=0; j<$PBNUM; j++)); do
        PBLINE="$PBLINE$PBSYM"
    done
    PBPERC=$(printf "[%3d%%] " $PBVAL)  # i.e. "[ 12%] "
    echo -e "${EBLUE}$PBPERC$PBLINE> $2${NO_COLOR}\r"
    echo -ne "\033[K$3 $4 $5 $6\r"
    echo -ne "\033[1A\r"
}
 

c=0
o=0
e=0
p=0
RES=""
FILE=$1
if [ x"$1" = x"verbose" ] ; then
   FILE=$2
fi
if [ x"$1" = x"debug" ] ; then
   FILE=$2
   DEBUG="y"
fi
if [ x"$FILE" = x"" ] ; then
   FILE="tests/*.list"
else
   if [ \! -f $FILE ] ; then
      echo "File $FILE doesn't exists"
      exit 0
   fi
fi

echo -ne "$(GetPBLine $p $c 'Starting...')\r"
tot=`cat $FILE | grep -v "^#"  |  grep -v "^$" | wc -l`
for i in `cat $FILE | grep -v "^#" | grep -v "^$" `
do
   c=$[ c+1 ]
   uno=`echo $i | awk -F ':' '{print $1}'`
   due=`echo $i | awk -F ':' '{print $2}'`
   tre=`echo $i | awk -F ':' '{print $3}'`
   echo -ne "$(GetPBLine $p $c 'RUNNING TEST: ./dmdomain' \'${uno}\' \'${due}\')\r"
   totprova=$(./dmdomain "${uno}" "${due}")
   prova=`echo -en "$totprova" | tail -n 1`


   if [ "$DEBUG" = "y" ] ; then
      echo -en "\n\n DEBUG: \n $totprova \n ------------- \n"
   fi
   if [ ${prova} != ${tre} ] ; then
      RES="${RES}\n${ERED}ERROR: ${EYELLOW}$uno ${ECYAN}$due${EMAGENTA} -> $prova${EWHITE} (need: $tre)${NO_COLOR}| ./dmdomain '${uno}' '${due}'"
      e=$[ e+1 ]
   else
      o=$[ o+1 ]
      if [ "$1" = "verbose" ] || [ "$1" = "debug" ] ; then
         RES="${RES}\n${EGREEN}OK: ${EYELLOW}$uno ${ECYAN}$due${EMAGENTA} -> $prova${EWHITE} (need: $tre)${NO_COLOR}| ./dmdomain '${uno}' '${due}'"
      fi
   fi
   if [ "$DEBUG" = "y" ] ; then
      echo -e "$RES"
      RES=""
   fi
   p=$[ c*100/tot ]
   echo -ne "$(GetPBLine $p $c 'RUNNING TEST: ./dmdomain' ${uno} ${due})\r"
done
echo -ne "$(GetPBLine 100 $c 'Done.')\r"
echo
echo -ne ${RES}
echo 
echo "------------------------------------"
echo "FINAL REPORT:"
echo 
pok=$[ o*100/tot ]
per=$[ 100-pok ]
echo -e " TESTS:$ECYAN $c$NO_COLOR - OK:$EGREEN $o ($pok%)$NO_COLOR ERRORS:$ERED $e ($per%)$NO_COLOR"
echo 
echo "------------------------------------"
