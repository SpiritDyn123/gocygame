#!/bin/bash

rebuild=0
run=0
release=0

for arg in $@
do
	if [ "$arg" = "a" ] ; then
		rebuild=1
	fi
	if [ "$arg" = "run" ] ; then
		run=1
	fi
	if [ "$arg" = "r" ] ; then
		release=1
	fi
done

cd ./src
if [ $release -eq 1 ] ; then
	make RELEASE=1 clean all
elif [ $rebuild -eq 1 ] ; then
	make clean all
else
	make all
fi

cd ..

if [ $run -eq 1 ] ; then
	./startup c
fi
