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
    echo "  Debug"
    echo "  Release"
    echo ""
    exit 1
fi

if test \( \( -n "$1" \) -a \( "$1" = "Debug" \) \); then 
    XCODE_BUILD_CONFIGURATION="Debug"
elif test \( \( -n "$1" \) -a \( "$1" = "Release" \) \); then
    XCODE_BUILD_CONFIGURATION="Release"
else
    echo "The configuration \"$1\" is not supported!"
    echo ""
    echo "configuration:"
    echo "  Debug"
    echo "  Release"
    echo ""
    exit 1
fi

PROJECT_DIRECTORY="$(cd "$(dirname "$0")" 1>/dev/null 2>/dev/null && pwd)"  

XCODE_BUILD_ARGUMENT_BEFORE_SCHEME=(
  "-workspace"
  "${PROJECT_DIRECTORY}/Demo-MediaPipe.xcworkspace"
)

XCODE_BUILD_ARGUMENT_AFTER_SCHEME=(
  "-destination"
  "generic/platform=macOS"
  "-configuration"
  "${XCODE_BUILD_CONFIGURATION}"
  "-derivedDataPath"
  "${PROJECT_DIRECTORY}/Xcode/DerivedData"
  "build"
)

if xcodebuild "${XCODE_BUILD_ARGUMENT_BEFORE_SCHEME[@]}" -scheme flatc "${XCODE_BUILD_ARGUMENT_AFTER_SCHEME[@]}"; then
    echo "xcode build flatc passed"
else
    echo "xcode build flatc failed"
    exit 1
fi

if xcodebuild "${XCODE_BUILD_ARGUMENT_BEFORE_SCHEME[@]}" -scheme protoc "${XCODE_BUILD_ARGUMENT_AFTER_SCHEME[@]}"; then
    echo "xcode build protoc passed"
else
    echo "xcode build protoc failed"
    exit 1
fi

if xcodebuild "${XCODE_BUILD_ARGUMENT_BEFORE_SCHEME[@]}" -scheme tensorflow-build-flat "${XCODE_BUILD_ARGUMENT_AFTER_SCHEME[@]}"; then
    echo "xcode build tensorflow flat passed"
else
    echo "xcode build tensorflow flat failed"
    exit 1
fi

if xcodebuild "${XCODE_BUILD_ARGUMENT_BEFORE_SCHEME[@]}" -scheme tensorflow-build-proto "${XCODE_BUILD_ARGUMENT_AFTER_SCHEME[@]}"; then
    echo "xcode build tensorflow proto passed"
else
    echo "xcode build tensorflow proto failed"
    exit 1
fi

if xcodebuild "${XCODE_BUILD_ARGUMENT_BEFORE_SCHEME[@]}" -scheme tensorflow-lite-c "${XCODE_BUILD_ARGUMENT_AFTER_SCHEME[@]}"; then
    echo "xcode build tensorflow passed"
else
    echo "xcode build tensorflow failed"
    exit 1
fi

if xcodebuild "${XCODE_BUILD_ARGUMENT_BEFORE_SCHEME[@]}" -scheme mediapipe-build-flat "${XCODE_BUILD_ARGUMENT_AFTER_SCHEME[@]}"; then
    echo "xcode build mediapipe flat passed"
else
    echo "xcode build mediapipe flat failed"
    exit 1
fi

if xcodebuild "${XCODE_BUILD_ARGUMENT_BEFORE_SCHEME[@]}" -scheme mediapipe-build-proto "${XCODE_BUILD_ARGUMENT_AFTER_SCHEME[@]}"; then
    echo "xcode build mediapipe proto passed"
else
    echo "xcode build mediapipe proto failed"
    exit 1
fi

if xcodebuild "${XCODE_BUILD_ARGUMENT_BEFORE_SCHEME[@]}" -scheme mediapipe "${XCODE_BUILD_ARGUMENT_AFTER_SCHEME[@]}"; then
    echo "xcode build mediapipe passed"
else
    echo "xcode build mediapipe failed"
    exit 1
fi

if xcodebuild "${XCODE_BUILD_ARGUMENT_BEFORE_SCHEME[@]}" -scheme Demo-MediaPipe-Build-Model "${XCODE_BUILD_ARGUMENT_AFTER_SCHEME[@]}"; then
    echo "xcode build mediapipe model passed"
else
    echo "xcode build mediapipe model failed"
    exit 1
fi

if xcodebuild "${XCODE_BUILD_ARGUMENT_BEFORE_SCHEME[@]}" -scheme Demo-MediaPipe "${XCODE_BUILD_ARGUMENT_AFTER_SCHEME[@]}"; then
    echo "xcode build Demo-MediaPipe passed"
else
    echo "xcode build Demo-MediaPipe failed"
    exit 1
fi
