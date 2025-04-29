#!/bin/sh

silent_mode=$(mktemp);
verbose_mode=$(mktemp);

# Normally, we'd use the word ..silent, but in this case it's not defined yet until we include prims.fs, so we hard code its effect
#printf "..silent\n" > $tmpff1
printf '6 DUP @ 1 1 LSHIFT OR SWAP !\n' > $silent_mode

printf "\n..verbose\n" > $verbose_mode
cat $@
echo Prepending the files: $@
echo _______________________
cat $silent_mode $@ $verbose_mode /dev/stdin | $NOMPATH./fex;
