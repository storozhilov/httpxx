#!/bin/sh

echo ">>> Building HTTPXX"
scons
if [ $? -ne 0 ] ; then
	exit 1
fi

echo ">>> Running unit-tests"
LD_LIBRARY_PATH=lib ./test/test_units
if [ $? -ne 0 ] ; then
	exit 1
fi
