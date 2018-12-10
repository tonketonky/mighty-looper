#!/bin/bash

function help {
    echo "Usage examples:"
    echo "1) ./start-looper.sh --profile prod (runs app in prod profile - default)"
    echo "2) ./start-looper.sh --profile dev (runs app in dev profile)"
    echo "3) ./start-looper.sh (equivalent to first example)"
    echo ""
}

function set_prod_paths {
    # set env var for scripts dir
    export SCRIPTS_DIR="$LOOPER_HOME/scripts"
    # set env var for core dir
    export CORE_DIR="$LOOPER_HOME/core"
}

function set_dev_paths {
    # set env var for scripts dir
    export SCRIPTS_DIR="$LOOPER_HOME/mightyLooperScripts"
    # set env var for core dir
    export CORE_DIR="$LOOPER_HOME/mightyLooperCore"
}

# set environment variable for looper home dir
export LOOPER_HOME=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && cd .. && pwd )
# set environment variable for log dir
export LOG_DIR="$LOOPER_HOME/log"

# if num of args is 2 and first one is equal to "--profile"
if [[ $# == 2 && $1 == "--profile" ]]
    then
        case $2 in
            prod)
            # prod profile
            set_prod_paths
            ;;
            dev)
            # dev profile
            set_dev_paths
            ;;
            *)
            # unknown profile
            echo "Invalid profile name"
            help
            break
            ;;
        esac

elif [[ $# > 0 ]]
    # invalid number of args
    then
        echo "Invalid options..."
        help
        exit 1
else
    # no args, set paths for prod profile as default
    set_prod_paths
fi

# if doesn't exist create log dir
mkdir -p $LOG_DIR

# start mighty looper
python3 $SCRIPTS_DIR/mighty_looper.py >> $LOOPER_HOME/log/mighty-looper-console.log 2>> $LOOPER_HOME/log/mighty-looper-console.err&
echo "Mighty Looper started"

# save process id to hidden file
echo $! > $SCRIPTS_DIR/.ml.pid

