#!/bin/sh
# When we run kill 0 from a subshell, we will kill the entire process group of this script
trap "kill 0" SIGINT

silent_mode=$(mktemp);
verbose_mode=$(mktemp);

# Normally, we'd use the word ..silent, but in this case it's not defined yet until we include bootstrap.fs, so we hard code its effect
#printf "..silent\n" > $tmpff1
printf '6 DUP @ 1 1 LSHIFT OR SWAP !\n' > $silent_mode

printf "\n..verbose\n" > $verbose_mode
cat $@
echo Prepending the files: $@
echo _______________________
# The trap at the start of the file is used to kill cat from within the right side of this pipe
cat $silent_mode $@ $verbose_mode /dev/stdin | ( $N./fex; kill 0)
