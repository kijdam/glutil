#!/bin/sh
#
#  Copyright (C) 2013 NixNodes
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# DO NOT EDIT/REMOVE THESE LINES
#@VERSION:0
#@REVISION:2
#@MACRO:imdb-sort|imdb sort:{m:exe} --silent -q imdb --imdblog "{m:q:imdb@file}" -execv `{m:spec1} {glroot}{dir} none {glroot} {siteroot} "{m:arg1} " {genre} {year} {actors} {director} {rated} {score}` 
#
##
BASE_DIR=/glftpd/site/_sorted
#
##########################################

[ ${#5} -gt 1 ] && BASE_DIR=`echo "${5}" | sed -r 's/^[ ]+//' | sed -r 's/[ ]+$//'`

R_PATH="${1}"
R_ARG=${2}
R_GLROOT="${3}"
R_SITEROOT="${4}"
R_GENRE="${6}"
R_YEAR="${7}"
R_ACTORS="${8}"
R_DIRECTOR="${9}"
R_RATING="${10}"
R_SCORE="${11}"

C_SITEROOT=`echo "${R_SITEROOT}" | sed -r 's/\//\\\\\//g'`


T_PATH=`echo "${R_PATH}" | sed -r "s/^${C_SITEROOT}//"`

[ -z "${T_PATH}" ] && echo "failed extracting base target path" && exit 0

! [ -d "${BASE_DIR}" ] && { 
	mkdir -p "${BASE_DIR}" && chmod 777 "${BASE_DIR}" || exit 0
}

BT_PATH=`dirname "${T_PATH}"`

[ -z "${BT_PATH}" ] || [ "${BT_PATH}" = "/" ] && echo "failed extracting directory part of base path - ${BT_PATH}" && exit 0

DT_PATH="${BASE_DIR}${BT_PATH}"

! [ -d "${DT_PATH}" ] && {
 	mkdir -p "${DT_PATH}" && chmod 777 "${DT_PATH}" || exit 0
}

B_PATH=`basename "${T_PATH}"`

C_GLROOT=`echo "${R_GLROOT}" | sed -r 's/\//\\\\\//g'`

CR_PATH=`echo "${R_PATH}" | sed -r "s/^${C_GLROOT}//"`

#[mode] [val]
proc_sort() {
	s_IFS=${IFS}
	IFS=","	
	for i in ${2}; do
		O_VAL=`echo "${i}" | sed -r 's/^[ ]+//' | sed -r 's/[ ]+$//' | sed -r 's/[^a-zA-Z0-9 .\_\-]//g'`
		[ -z "${O_VAL}" ] && continue
		C_PATH="${DT_PATH}/${1}/${O_VAL}"
		! [ -d "${C_PATH}" ] && {
			mkdir -p "${C_PATH}" && chmod 777 "${C_PATH}" || {
				IFS=${s_IFS}
				return 2
			}			
		}
		
		! [ -e "${C_PATH}/${B_PATH}" ] && { 
			echo "${C_PATH}/${B_PATH}"
			ln -s "${CR_PATH}" "${C_PATH}"
		}
	done
	IFS=${s_IFS}
}

[ -n "${R_GENRE}" ] && proc_sort Genre "${R_GENRE}"
[ -n "${R_YEAR}" ] && proc_sort Year ${R_YEAR}
[ -n "${R_ACTORS}" ] && proc_sort Actor "${R_ACTORS}"
[ -n "${R_DIRECTOR}" ] && proc_sort Director "${R_DIRECTOR}"
[ -n "${R_RATING}" ] && proc_sort Rating "${R_RATING}"
[ -n "${R_SCORE}" ] && proc_sort Score "${R_SCORE}"

chmod -R 777 "${BASE_DIR}"

exit 0