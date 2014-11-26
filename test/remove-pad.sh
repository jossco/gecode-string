#!/bin/bash

TMP=$(mktemp temp.XXXXXX)
sed -E "s/0//g" < $1 > $TMP
cat $TMP > $1
rm $TMP