#!/bin/bash

FIRST=$1
SECOND=$2

if [[ ! -d $FIRST ]] || [[ ! -d $SECOND ]]
then
  echo "Syntax: $0 dir1 dir2"
  exit
fi

#~ echo first $FIRST second $SECOND

for NAME in `cd $FIRST && find`
do
  if [[ $NAME = "." ]]
  then
    continue
  fi

  if [[ -e $FIRST/$NAME ]] && [[ ! -e $SECOND/$NAME ]]
  then
    echo "$FIRST/$NAME doesn't exist in $SECOND";
    exit
  fi
  
  if [[ -d $FIRST/$NAME ]] && [[ ! -d $SECOND/$NAME ]]
  then
    echo "$FIRST/$NAME is a directory but $SECOND/$NAME is not"
    exit
  else
    echo -n "Checking $NAME: "
    if [[ `md5sum $FIRST/$NAME | cut -f 1 -d ' '` != `md5sum $SECOND/$NAME | cut -f 1 -d ' '` ]]
    then
      echo -n "Files are not the same, continue? (y/n) "
      read GOON
      if [[ $GOON = "n" ]]
      then
        exit
      fi
    else
      echo "OK."
    fi
  fi
  
done
