#!/bin/bash

SCRIPTS_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

SHOULD_SHUTDOWN=false

if [[ $# == 1 && $1 == "--shutdown" ]]
then
    SHOULD_SHUTDOWN=true
fi

if [ ! -f "$SCRIPTS_DIR/.ml.pid" ]
then
    echo "Mighty looper pid file doesnt exist." >&2
    echo "Process of mighty_looper.py has to be killed manually." >&2
else
    kill $(cat "$SCRIPTS_DIR/.ml.pid")
    if [ $? -eq 0 ]
    then
        rm "$SCRIPTS_DIR/.ml.pid"
        echo "Mighty Looper stopped"
        if [[ $SHOULD_SHUTDOWN == true ]]
        then
            echo "Shutting down..."
            shutdown -h now
        fi
    else
        echo "Unable to stop Mighty Looper" >&2
    fi
fi
