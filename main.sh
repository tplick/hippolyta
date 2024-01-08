#!/bin/bash
ulimit -s unlimited
while true
do
    nice -n 10   python go.py
    sleep 5
done

