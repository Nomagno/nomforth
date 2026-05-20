# Replace PWD with the absolute location of the nomforth source to make this into a relocatable script
export NFPATH=$PWD/
export N=$NFPATH

# install package gcc-multilib
gcc -o fex -g -m32 -rdynamic -ldl \
        $N/core/forth.c \
        $N/primitives/prims.c \
        $N/primitives/oa.c \
        $N/forth_libs/foreign.c \
&& sh $N/runhelper.sh $N/core/bootstrap.fs $N/forth_libs/utilities.fs $@
