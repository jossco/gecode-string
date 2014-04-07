#!/bin/bash

MAXLEN=$1
shift
FILECOUNT=0
LSTFILE=$(mktemp ./out.XXXXXX)
TMPFILE=$(mktemp ./out.XXXXXX)
until [ -z "$1" ]
do
    let "FILECOUNT += 1"
    LSTFILENAME=$TMPFILENAME
    TMPFILENAME=$1
    echo
    rm $LSTFILE
    LSTFILE=$TMPFILE
    TMPFILE=$(mktemp ./out.XXXXXX)
    sed -E -e "s/\{.*\}//" < "$1" | sed -n -E -e "s/^.*\[ ([1-9]+)0* \].*$/\1/p" > $TMPFILE
    echo "$1:"
    SOLS=$(sed -n -e "/[0-9]/p" < "$TMPFILE" | wc -l | sed -n -E -e "s/ *([0-9]+).*/\1/p")
    echo "$SOLS reported"
    sort -n -u -o $TMPFILE $TMPFILE
    echo "$(wc -l $TMPFILE | sed -n -E -e "s/ *([0-9]+).*/\1/p") unique" 
    CORRECT=$(egrep  -e "^[1-3]*1[1-3]{6}$" "$TMPFILE" | egrep -c "^[1-3]*2[1-3]{5}$")
    echo "$CORRECT correct"
    echo "count by length:"
    for ((a=1; a <= MAXLEN ; a++))
    do
        COUNT=$(sed -E -n /^[0-9]{"$a"}$/p < "$TMPFILE" | wc -l | sed -n -E -e 's/ *([0-9]+).*/\1/p')
        if [ "$COUNT" -ne 0 ]
            then
            echo "$a: $COUNT"
        fi
    done
    if [ "$SOLS" -ne "$CORRECT" ]
        then
        echo '*********'
        echo "reported non-solutions:"
        sed -n -e "/[0-9]/p" < "$TMPFILE" | \
            sed -E -n -e '/(^[1-3]*1[1-3]{6}$)|(^[1-3]*2[1-3]{5}$)/!p'
    fi
    shift
    echo "========="
done
echo
if [ "$FILECOUNT" -gt 1 ]
    then
    echo "diff in dif.${LSTFILENAME}.${TMPFILENAME}"
    diff -y -W 40 $LSTFILE $TMPFILE > "dif.${LSTFILENAME}.${TMPFILENAME}"  
fi
rm $LSTFILE
rm $TMPFILE