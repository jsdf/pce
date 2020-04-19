#!/bin/bash

# initialises build environment from PCEJS_conf vars

export PCEJS_PREFIX=$(node -e "process.stdout.write(require('path').resolve('$PCEJS_conf_prefix'))")
export PCEJS_PACKAGEDIR=$(node -e "process.stdout.write(require('path').resolve('$PCEJS_conf_packagedir'))")
export PCEJS_TARGET=$PCEJS_conf_target
export PCEJS_OUTPUT_FORMAT=$PCEJS_conf_outputformat

if [[ -n $PCEJS_conf_emscripten ]]; then
  emflags=""
 
  if [[ -n $PCEJS_conf_asmjs ]]; then
    emflags+=" -s ASM_JS=1"
  fi

  if [[ -n $PCEJS_conf_wasm  ]]; then
    emflags+=" -s WASM=1'"
    emflags+=" --source-map-base http://127.0.0.1:8080/"
  elif [[  -n $PCEJS_conf_asmjs ]]; then
    emflags+=" -s WASM=0"
    emflags+=" -s MEM_INIT_METHOD=0"
  fi


  # emflags+=" -s VERBOSE=1"

  emflags+=" -s FORCE_FILESYSTEM=1"

  emflags+=" --js-library libjs/library_sdl.js"

  if [[ -n $PCEJS_conf_worker ]]; then
    emflags+=" -s PROXY_TO_WORKER=1"
  fi

  if [[ -n $PCEJS_conf_threaded ]]; then
    emflags+=" -s USE_PTHREADS=1"
    emflags+=" -s PTHREAD_POOL_SIZE=1"
    emflags+=" -s PTHREAD_HINT_NUM_CORES=2"
  fi

  if [[ -n $PCEJS_conf_memory ]]; then
    memory=$((PCEJS_conf_memory*1024*1024))
    emflags+=" -s TOTAL_MEMORY=${memory}"
  fi

  # emflags+=" -s OUTLINING_LIMIT=16000"
  if [[ -n $PCEJS_conf_outlining ]]; then
    emflags+=" -s OUTLINING_LIMIT=${PCEJS_conf_outlining}"
  fi

  if [[ -n $PCEJS_conf_exportfuncs ]]; then
    emflags+=" -s EXPORTED_FUNCTIONS=${PCEJS_conf_exportfuncs}"
  fi

  if [[ -n $PCEJS_conf_optlvl ]]; then
    emflags+=" -${PCEJS_conf_optlvl}"
  fi

  pcejs_make_cflags=""

  if [[ -n $PCEJS_conf_worker ]]; then
    pcejs_make_cflags+=" --proxy-to-worker"
  fi

  if [[ -n $PCEJS_conf_dbglvl ]]; then
    if [[ $PCEJS_conf_dbglvl == "p" ]]; then
      emflags+=" --profiling"
    else
      pcejs_make_cflags+=" -g${PCEJS_conf_dbglvl}"
    fi
  fi

  if [[ -n "$PCEJS_conf_emsdkpath" ]]; then
    # explicitly set
    export PCEJS_EMSDK_PATH="$PCEJS_conf_emsdkpath"
  else
    env_emsdk_emcc=$(which emcc)
    if [[ -n "$env_emsdk_emcc" ]]; then
      # we probably just want this to come from the env
      export PCEJS_EMSDK_PATH=$(dirname $env_emsdk_emcc)
    fi
  fi
  export PCEJS_EMSCRIPTEN="yes"
  export PCEJS_EMFLAGS="$emflags"
  export PCEJS_CFLAGS="-Qunused-arguments -include src/include/pcedeps.h $emflags"
  export PCEJS_CONFIGURE="${PCEJS_EMSDK_PATH}/emconfigure ./configure"
  export PCEJS_MAKE_CFLAGS="$pcejs_make_cflags"
else
  export PCEJS_CFLAGS="-I/usr/local/opt/emscripten/system/include/emscripten/"
  export PCEJS_CONFIGURE="./configure"
  export PCEJS_MAKE_CFLAGS=""
fi
