#/bin/bash

export LD_LIBRARY_PATH=$PWD:$LD_LIBRARY_PATH

clear

make clean
make mynums

if [ $? -eq 0 ]; then
    ./mynums
fi
