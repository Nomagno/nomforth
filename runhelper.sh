#!/bin/sh

tmpff1=$(mktemp);
tmpff2=$(mktemp);
tmpff3=$(mktemp);

# Normally, we'd use the word ..silent, but in this case it's not defined yet until we include prims.fs, so we hard code its effect
#printf "..silent\n" > $tmpff1
printf '6 DUP @ 1 1 LSHIFT OR SWAP !\n' > $tmpff1

printf "\n..verbose\n" > $tmpff2
cat $@
echo Prepending the files: $@
echo  Note: you might have to press enter a couple of times after \'quit\' when using this wrapper.
echo "      In that case, the interpreter really has closed, but the input buffer hasn't."
echo _______________________
cat | cat $tmpff1 $@ $tmpff2 /dev/stdin $tmpff3 | ./fex;
