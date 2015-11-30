#!/bin/bash
cmd="g++ -E -o __tmp.cpp ${1}.fe.cpp "
echo $cmd
ret=`$cmd`

cmd="./flowext __tmp.cpp ${1}.cpp"
echo $cmd
ret=`$cmd`
