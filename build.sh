#!/bin/sh

set -x

SOURCE_DIR=`pwd`
BUILD_DIR=${BUILD_DIR:-./build}
INSTALL_DIR=${INSTALL_DIR:-../${BUILD_TYPE}}
CXX=${CXX:-g++}

mkdir -p $BUILD_DIR/ \
  && cd $BUILD_DIR/ \
  && cmake \
           -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
           -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
           -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
           $SOURCE_DIR \
  && make $*
