#!/bin/bash

PATH=$PATH:`dirname $0`
export PATH

if [ -e "${RCVW_GPS_INPUT}" ]; then
  # Run a configuration for each parameter set in the environment
  ubxtool -h -v 5 | awk '$1 ~ /CFG-.*-/ { print $1 }' | while read PARAM; do
    VAR=`echo "${PARAM}" | tr '-' '_'`
    if [ ! -z "${VAR}" ]; then
      VAL=`eval echo \\\${${VAR}}`
      if [ ! -z "${VAL}" ]; then
        ubxtool -w 0.25 -z "${PARAM},${VAL}"
      fi
    fi
  done
fi

ubxtool -w 0.25 -p SAVE