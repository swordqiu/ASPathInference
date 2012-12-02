#!/bin/bash

function stop_service {
    SESSION=$(screen -ls | awk '/[0-9].'$1'/ { print $1 }')
    if [ -n "$SESSION" ]; then
        screen -X -S $SESSION quit
        return 1
    else
        return 0
    fi
}

stop_service aspath
