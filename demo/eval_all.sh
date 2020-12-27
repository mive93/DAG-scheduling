#!/bin/bash

cd build 
target="../data/"
for f in "$target"/*
do
    # echo ./eval ../data/$(basename $f) 0
    ./eval ../data/$(basename $f) 0
    # break;

done