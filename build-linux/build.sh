#!/bin/bash

#
# Copyright (C) YuqiaoZhang(HanetakaChou)
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

set -euo pipefail

# configure
if test \( $# -ne 1 \); then
    echo "usage: build.sh configuration"
    echo ""
    echo "configuration:"
    echo "  debug"
    echo "  release"
    echo ""
    exit 1
fi

if test \( \( -n "$1" \) -a \( "$1" = "debug" \) \); then 
    CONFIGURATION_NDK_DEBUG="NDK_DEBUG:=1"
    CONFIGURATION_APP_DEBUG="APP_DEBUG:=true"
    CONFIGURATION_DIRECTORY="debug"
elif test \( \( -n "$1" \) -a \( "$1" = "release" \) \); then
    CONFIGURATION_NDK_DEBUG="NDK_DEBUG:=0"
    CONFIGURATION_APP_DEBUG="APP_DEBUG:=false"
    CONFIGURATION_DIRECTORY="release"
else
    echo "The configuration \"$1\" is not supported!"
    echo ""
    echo "configuration:"
    echo "  debug"
    echo "  release"
    echo ""
    exit 1
fi

PROJECT_DIRECTORY="$(cd "$(dirname "$0")" 1>/dev/null 2>/dev/null && pwd)"  

OBJECT_DIRECTORY="${PROJECT_DIRECTORY}/obj/${CONFIGURATION_DIRECTORY}"
BINARY_DIRECTORY="${PROJECT_DIRECTORY}/bin/${CONFIGURATION_DIRECTORY}"

NDK_BUILD_PATH="${PROJECT_DIRECTORY}/../thirdparty/NDK-Build-Linux/ndk-build"

JOBS="$(nproc)"

NDK_BUILD_ARGUMENT=(
  "-j${JOBS}"
  "NDK_MODULE_PATH:=${PROJECT_DIRECTORY}/.."
  "NDK_PROJECT_PATH:=null"
  "NDK_OUT:=${OBJECT_DIRECTORY}"
  "NDK_LIBS_OUT:=${BINARY_DIRECTORY}"
  "${CONFIGURATION_NDK_DEBUG}"
  "NDK_APPLICATION_MK:=Application.mk"
  "APP_BUILD_SCRIPT:=Linux.mk"
  "APP_ABI:=x86_64"
  "APP_PLATFORM:=android-28"
)

MAKE_ARGUMENT=(
  "-j${JOBS}"
  "APP_PROJECT_PATH:=${PROJECT_DIRECTORY}"
  "APP_ABI:=x86_64"
  "${CONFIGURATION_APP_DEBUG}"
)

cd "${PROJECT_DIRECTORY}"

if "${NDK_BUILD_PATH}" -C "${PROJECT_DIRECTORY}" "APP_MODULES:=flatc protoc" "${NDK_BUILD_ARGUMENT[@]}"; then
    echo "ndk-build flatc & protoc passed"
else
    echo "ndk-build flatc & protoc failed"
    exit 1
fi

if make -C "${PROJECT_DIRECTORY}/../thirdparty/tensorflow/build-flat" -f "flat.mk" "${MAKE_ARGUMENT[@]}"; then
    echo "build tensorflow flat passed"
else
    echo "build tensorflow flat failed"
    exit 1
fi

if make -C "${PROJECT_DIRECTORY}/../thirdparty/tensorflow/build-proto" -f "proto.mk" "${MAKE_ARGUMENT[@]}"; then
    echo "build tensorflow proto passed"
else
    echo "build tensorflow proto failed"
    exit 1
fi

if make -C "${PROJECT_DIRECTORY}/../thirdparty/mediapipe/build-flat" -f "flat.mk" "${MAKE_ARGUMENT[@]}"; then
    echo "build mediapipe flat passed"
else
    echo "build mediapipe flat failed"
    exit 1
fi

if make -C "${PROJECT_DIRECTORY}/../thirdparty/mediapipe/build-proto" -f "proto.mk" "${MAKE_ARGUMENT[@]}"; then
    echo "build mediapipe proto passed"
else
    echo "build mediapipe proto failed"
    exit 1
fi

if make -C "${PROJECT_DIRECTORY}/../build-mediapipe-model" -f "mediapipe-model.mk" "${MAKE_ARGUMENT[@]}"; then
    echo "build mediapipe model passed"
else
    echo "build mediapipe model failed"
    exit 1
fi

if "${NDK_BUILD_PATH}" -C "${PROJECT_DIRECTORY}" "APP_MODULES:=OpenCL tensorflow-lite-c mediapipe Demo-MediaPipe" "${NDK_BUILD_ARGUMENT[@]}" ; then
    echo "ndk-build tensorflow-lite-c & mediapipe & Demo-MediaPipe passed"
else
    echo "ndk-build tensorflow-lite-c & mediapipe & Demo-MediaPipe failed"
    exit 1
fi
