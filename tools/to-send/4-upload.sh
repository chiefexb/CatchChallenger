#!/bin/bash

export TEMP_PATH="/mnt/world/CatchChallenger-temp/"
export CATCHCHALLENGER_SOURCE="/home/user/Desktop/CatchChallenger/"
export BASE_PWD=`pwd`

cd ${BASE_PWD}

export CATCHCHALLENGER_VERSION=`grep -F "CATCHCHALLENGER_VERSION" ${CATCHCHALLENGER_SOURCE}/general/base/GeneralVariable.h | grep -F "0." | sed -r "s/^.*([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+).*$/\1/g"`
echo ${CATCHCHALLENGER_VERSION}

echo "Upload version..."
source sub-script/upload-local.sh
cd ${BASE_PWD}
echo "Upload version... done"



 
