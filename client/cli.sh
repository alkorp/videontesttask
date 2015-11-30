#!/bin/bash

srv_ctrl=${1:-/var/tmp/srv_fifo}
cli_in="$(mktemp -u)"
cli_out="$(mktemp -u)"

CTRL_FIFO_SETUP_FAILED=10

clean_up() {
    rm -f "$cli_out" "$cli_in"
}

connect() {
    if [ ! -p "$srv_ctrl" ]; then 
        echo "No FIFO at $srv_ctrl"; exit $CTRL_FIFO_SETUP_FAILED
    fi
    #create client input pipe
    if [ -e "$cli_in" ] || ! mkfifo "$cli_in"; then 
        echo "$cli_in creation failed"; exit $CTRL_FIFO_SETUP_FAILED
    fi
    #create client output pipe
    if [ -e "$cli_out" ] || ! mkfifo "$cli_out"; then 
        echo "$cli_out creation failed"; exit $CTRL_FIFO_SETUP_FAILED
    fi

    exec {srv_in}> "$srv_ctrl"
    #echo "Request connection to $cli_in, $cli_out from $srv_ctrl"
    echo "$cli_in $cli_out" >&$srv_in
    {srv_in}>&-

    exec {in}< "$cli_in"
    exec {out}> "$cli_out"
}

LED_update() {
    #read LED state
    echo "get-led-state" >&$out
    read -u $in reply
    set -- $reply
    [[ $1 == "OK" ]] && LED_state=$2 || led_state="unknown"
    #read LED color
    echo "get-led-color" >&$out
    read -u $in reply
    set -- $reply
    [[ $1 == "OK" ]] && LED_color=$2 || led_state="unknown"
    #read LED rate
    echo "get-led-rate" >&$out
    read -u $in reply
    set -- $reply
    [[ $1 == "OK" ]] && LED_rate=$2 || led_state="unknown"
}

LED_print() {
    LED_update
    echo -n "LED status is ${LED_state:-"unknown"}, "
    echo "color is ${LED_color:-"unknown"} and rate is ${LED_rate:-"unknown"}"
}

do_cmd() {
    echo "$1 $2" >&$out
    read -u $in reply
    set -- $reply
    echo $1
    LED_print
    [ $1 = "OK" ] && return 0 || return 1;
}

main_menu=(
    "Switch LED On or Off" 'menu onoff_menu[@] "Switch LED"'
    "Select color" 'menu color_menu[@] "Select color"'
    "Select rate" '
        echo -n "Enter new rate[0-5]: "
        read rate
        do_cmd set-led-rate $rate
        '
    "Quit" "return 0")
onoff_menu=(
    "On" "do_cmd set-led-state on"
    "Off" "do_cmd set-led-state off"
    "Back" "return 0")
color_menu=(
    "Red" "do_cmd set-led-color red"
    "Green" "do_cmd set-led-color green"
    "Blue" "do_cmd set-led-color blue"
    "Back" "return 0")

menu () {
    PS3='Please enter your choice: '
    local -a arr=( "${!1}" )
    local -a opts
    local -a funcs
    local sz=${#arr[*]};
    for ((i = 0; i < $sz/2; i++))
    do
        opts[$i]=${arr[2*$i]}
        funcs[$i]=${arr[2*$i+1]}
    done
    while :
    do
        echo; echo "_____$2_____"
        select opt in "${opts[@]}"
        do
            eval "${funcs[$REPLY-1]}"
            break
        done
    done
}

trap "clean_up; exit 1" SIGINT SIGHUP SIGTERM

echo connecting to $srv_ctrl
connect

echo "Connected"
LED_print

menu main_menu[@] "Main menu"

clean_up
exit 0

