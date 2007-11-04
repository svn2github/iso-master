for FILE in *.mo
do
    mkdir -p `basename $FILE .mo`/LC_MESSAGES
    echo `basename $FILE .mo`/LC_MESSAGES/isomaster.mo
    cp $FILE `basename $FILE .mo`/LC_MESSAGES/isomaster.mo
done
