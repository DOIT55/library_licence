#! /bin/bash

export IV_HOME=${PWD}
mkdir -p $IV_HOME/data
echo "HOST_NAME=TEST_HOST" > $IV_HOME/data/sysconfig
