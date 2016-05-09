#!/bin/bash

set -x
set -e

if [ "$build_version" == "" ]; then
  echo "Must set build_version environment variable"
  exit 1
fi

rm -rf artifacts
mkdir artifacts

pushd ../Android
export ANDROID_FOLDER="`pwd`"
popd

export NDK_ROOT="$ANDROID_FOLDER/android-ndk-r10d-mac"

# run the build for libcurl etc
#pushd "$ANDROID_FOLDER"
#./build_libcurl.sh
#popd

# compile braincloud
pushd ../GameClientLib
"$NDK_ROOT/ndk-build" clean
"$NDK_ROOT/ndk-build" 
popd

# and zip everything up
mkdir artifacts/brainCloud
mkdir -p artifacts/brainCloud/libs/armeabi
mkdir -p artifacts/brainCloud/libs/x86
#cp -r "$ANDROID_FOLDER/libs" artifacts/brainCloud
cp $ANDROID_FOLDER/cocos-curl/libs/armeabi/*.a artifacts/brainCloud/libs/armeabi
cp $ANDROID_FOLDER/cocos-curl/libs/x86/*.a artifacts/brainCloud/libs/x86

mkdir -p artifacts/brainCloud/thirdparty/curl/include
cp -r "$ANDROID_FOLDER/cocos-curl/include/curl" artifacts/brainCloud/thirdparty/curl/include
cp -r ../GameClientLib/lib/jsoncpp-1.0.0 artifacts/brainCloud/thirdparty

#cp -r ../GameClientLib/libs artifacts/brainCloud
cp ../GameClientLib/obj/local/armeabi/libbraincloud.a artifacts/brainCloud/libs/armeabi
cp ../GameClientLib/obj/local/x86/libbraincloud.a artifacts/brainCloud/libs/x86

mkdir artifacts/brainCloud/include
cp -r ../GameClientLib/src/ApiHeaders/* artifacts/brainCloud/include

mkdir artifacts/brainCloud/src
cp -r ../BrainCloudWrapper/* artifacts/brainCloud/src

mkdir artifacts/brainCloud/javasrc
cp -r ../Android/java/ artifacts/brainCloud/javasrc

#mkdir -p artifacts/brainCloud/thirdparty/curl-7.40.0/include
#cp -r "$ANDROID_FOLDER/curl-7.40.0/include/curl" artifacts/brainCloud/thirdparty/curl-7.40.0/include

cp ../../Common/docs/README.TXT artifacts/brainCloud

pushd artifacts
zip -r "brainCloudClient_Android_C++_${build_version}.zip" brainCloud

popd

