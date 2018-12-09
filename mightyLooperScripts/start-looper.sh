#!/bin/bash

# get scripts dir
SCRIPTS_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

# set environment variable for looper home dir
export LOOPER_HOME="$SCRIPTS_DIR/.."

# if doesn't exist create log folder
mkdir -p $LOOPER_HOME/log

# start mighty looper
python3 $SCRIPTS_DIR/mighty_looper.py >> $LOOPER_HOME/log/mighty-looper-console.log 2>> $LOOPER_HOME/log/mighty-looper-console.err&
echo "Mighty Looper started"

# save process id to hidden file
echo $! > $SCRIPTS_DIR/.ml.pid

