#!/bin/bash

failed=`grep failed startup.log | wc -l`
if [ $failed -ne "0" ]
then
    echo "failed!"
    cat startup.log
fi
