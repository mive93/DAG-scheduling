#!/bin/bash

cd build 
target="../data/thesis/single"
for f in "$target"/*
do
    # echo ./eval "$target"/$(basename $f) 0
    ./eval "$target"/$(basename $f) 0
    # break;

done