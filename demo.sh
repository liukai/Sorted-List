#/bin/bash

set1=100
set2=101
k1=200
k2=201
k3=202
host=127.0.0.1
port=7999

echo set1=$set1, set2=$set2
echo k1=$k1, k2=$k2, k3=$k3

echo "<SIZE> <set1> [0]"
client $host $port "3 $set1"

echo "<ADD> <set1> <k1> <1>"
client $host $port "1 $set1 $k1 1"

echo "<GET> <set1> <k1> [1]"
client $host $port "4 $set1 $k1"
echo "<SIZE> <set1> [1]"
client $host $port "3 $set1"

echo "<ADD> <set1> <k1> <2>"
client $host $port "1 $set1 $k1 2"
echo "<GET> <set1> <k1> [2]"
client $host $port "4 $set1 $k1"

echo "<ADD> <set1> <k2> <3>"
client $host $port "1 $set1 $k2 3"
echo "<GETRANGE> <set1> <-1> <3> <3> [k2] [3] [-1]"
client $host $port "5 $set1 -1 3 3"

echo "<ADD> <set2> <k3> <1>"
client $host $port "1 $set2 $k3 1"
echo "<GETRANGE> <set1> <set2> <-1> <0> <INT_MAX> [k3] [1] [k1] [2] [k2] [3] [-1]"
client $host $port "5 $set1 $set2 -1 0 10000"

echo "<REM> <set1> <k2>"
client $host $port "2 $set1 $k2"
echo "<GETRANGE> <set1> <set2> <-1> <0> <INT_MAX> [k3] [1] [k1] [2] [-1]"
client $host $port "5 $set1 $set2 -1 0 10000"

echo "<GET> <set1> <k3> [1]"
client $host $port "4 $set2 $k3"
