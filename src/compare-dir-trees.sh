#!/bin/bash

FIRST=$1
SECOND=$2

if [[ ! -d $FIRST ]] || [[ ! -d $SECOND ]]
then
  echo "Syntax: $0 dir1 dir2"
  exit
fi

#~ echo first $FIRST second $SECOND

for NAME in `cd $FIRST && find -noleaf`
do
  if [[ $NAME = "." ]]
  then
    continue
  fi

  if [[ -e $FIRST/$NAME ]] && [[ ! -e $SECOND/$NAME ]]
  then
    echo -n "$FIRST/$NAME doesn't exist in $SECOND continue? (Y/n) "
    read GOON
    if [[ $GOON = "n" ]]
    then
      exit
    fi
    continue
  fi
  
  if [[ -d $FIRST/$NAME ]] && [[ ! -d $SECOND/$NAME ]]
  then
    echo "$FIRST/$NAME is a directory but $SECOND/$NAME is not"
    exit
  elif [[ ! -d $FIRST/$NAME ]] && [[ ! -d $SECOND/$NAME ]]
  then
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
  else
    echo "Skipped $NAME"
  fi
  
done
