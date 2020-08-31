#!/bin/bash

TEST_NAME=@1
CONFIG=@2
CWD=$(pwd)

echo ${CWD}


./${TEST_NAME} ${CONFIG}

