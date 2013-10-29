# this is probably out of date
# use grunt instead
PCEJS_EMSCRIPTEN="yes"
PCEJS_EMFLAGS="-s TOTAL_MEMORY=268435456 -s ASM_JS=1 -O2"
PCEJS_CFLAGS="-Qunused-arguments -include build/include/pcedeps.h $PCEJS_EMFLAGS"
PCEJS_CONFIGURE="emconfigure ./configure"
PCEJS_PREFIX="$PWD/build/"
PCEJS_TARGET="macplus"