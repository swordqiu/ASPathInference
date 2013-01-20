#!/bin/bash

if [ $# -lt 1 ]; then
    echo "Usage: startInfer.sh <HOME>"
    exit -1
fi

HOME=$1

pushd $(dirname $(readlink -f "$BASH_SOURCE")) > /dev/null
ROOT_DIR=$(cd .. && pwd)
popd > /dev/null

function screen_it {
    NL=`echo -ne '\015'`
    SESSION=$(screen -ls | awk '/[0-9].'$1'/ { print $1 }')
    if [ ! -n "$SESSION" ]; then
        screen -d -m -S $1
        sleep  1.5
    fi
    screen -S $1 -p 0 -X stuff "$2$NL"
}

echo $ROOT_DIR

if [ -f $HOME/log/pid ]; then
    PID=`cat $HOME/log/pid`
    kill -9 $PID
    sleep 1
fi

screen_it aspath "cd /opt/data;$ROOT_DIR/script/startInfer.pl $1"
