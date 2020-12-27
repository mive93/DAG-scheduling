#!/bin/bash

cd build 
target="../data/"
for f in "$target"/*
do
    ./eval ../data/$(basename $f) 0
    # break;

done