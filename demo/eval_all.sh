#!/bin/bash

cd build 
target="data/"
for f in "$target"/*
do
    echo $(basename $f)
    ./eval ../data/$(basename $f) 0
    # break;

done