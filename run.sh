gcc -o fex -g \
        core/forth.c \
        primitives/prims.c \
        primitives/oa.c \
&& sh runhelper.sh forth_libs/bootstrap.fs forth_libs/utilities.fs $@
