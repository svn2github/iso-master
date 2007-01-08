#!/bin/bash

./extractstrings.sh

for NAME in *.po
do
  echo "$NAME ->" `basename $NAME .po`-updated.po
  msgmerge $NAME isomaster-new.pot > `basename $NAME .po`-updated.po
done
