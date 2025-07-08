export N=$NFPATH
gcc -o fex -g \
        $N/core/forth.c \
        $N/primitives/prims.c \
        $N/primitives/oa.c \
&& sh $N/runhelper.sh $N/forth_libs/bootstrap.fs $N/forth_libs/utilities.fs $@
