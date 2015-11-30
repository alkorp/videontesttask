#!/bin/bash

ctrl_fifo=${1:-/var/tmp/srv_fifo}

CTRL_FIFO_SETUP_FAILED=10

clean_up() {
    echo "Exiting..."
    #close connections
    kill $(jobs -p) 2>/dev/null
    wait
    #remove temporaries
    rm -f "$ctrl_fifo"
}

echo Server prototype starting...

#[ -z "$1" ] || ctrl_fifo="$1"

if [[ -e $ctrl_fifo ]]; then
    if [[ -p $ctrl_fifo ]]; then
        echo "Warning: $ctrl_fifo already exists"
    else
        echo "$ctrl_fifo is not a FIFO"; exit $CTRL_FIFO_SETUP_FAILED
    fi
else
    if ! mkfifo $ctrl_fifo; then 
        echo "$ctrl_fifo creation failed"; exit $CTRL_FIFO_SETUP_FAILED
    fi
fi

trap "clean_up; exit 1" SIGINT SIGHUP SIGTERM

echo "Listening on $ctrl_fifo"

while true; do
    #while read cli_in cli_out; do
    while read -r line; do
        [[ "$line" == "exit" ]] && echo Stopping... && break 2
        set -- $line
        echo "Connecting to $1, $2"
        ./srv_disp.sh "$1" "$2" &
        jobs -p
    done <> "$ctrl_fifo"
done
clean_up
exit 0
