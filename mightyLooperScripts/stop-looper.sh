#!/bin/bash

SCRIPTS_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

if [ ! -f "$SCRIPTS_DIR/.ml.pid" ]
    then
        echo "Mighty looper pid file doesnt exist." 
        echo "Process of mighty_looper.py has to be killed manually."
    else
        kill $(cat "$SCRIPTS_DIR/.ml.pid")
		if [ $? -eq 0 ]
            then
                rm "$SCRIPTS_DIR/.ml.pid"
                echo "Mighty Looper stopped"
            else
                echo "Unable to stop Mighty Looper"
    	fi
fi