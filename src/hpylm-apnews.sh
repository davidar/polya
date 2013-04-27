#!/bin/sh
TEST_SIZE=963071
make >&2 && gunzip < ../data/apnews.txt.gz | ./hpylm $1 $TEST_SIZE
