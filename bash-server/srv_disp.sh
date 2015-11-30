#!/bin/bash
cli_in=${1?"No client in FIFO specified"}
cli_out=${2?"No client out FIFO specified"}
cam_dev="./camera"

LED_state=
LED_color=
LED_rate=

LED_read() {
	echo "reading" >&1
	flock  200 || return 1
	read -u 200 LED_state LED_color LED_rate
} 200<$cam_dev

echo "Dispatching client on $cli_in, $cli_out (PID=$$)"

exec {out}>"$cli_in"
exec {in}<"$cli_out"

LED_read

while read -r line; do
	echo "Got $line"
    set -- $line
    case "$1" in
    	"echo") echo "$2" >&$out ;;
		"get-led-state")
			LED_read && echo "OK $LED_state" >&$out || echo "FAILED" >&$out;;
		"get-led-color")
			LED_read && echo "OK $LED_color" >&$out || echo "FAILED" >&$out;;
		"get-led-rate")
			LED_read && echo "OK $LED_rate" >&$out || echo "FAILED" >&$out;;
		"set-led-state")
			if [[ $2 == "on" -o $2 == "off" ]] &&
				flock $cam_dev -c "sed -i -e s_[[:alnum:]]*_$2_1 $cam_dev"
				then 
					echo "OK" >&$out
				else
					echo "FAILED" >&$out
			fi;;
		"set-led-color")
			if [[ $2 == "red" -o $2 == "green" -o $2 == "blue" ]] && 
				flock $cam_dev -c "sed -i -e s_[[:alnum:]]*_$2_2 $cam_dev"
				then 
					echo "OK" >&$out
				else
					echo "FAILED" >&$out
			fi;;
		"set-led-rate")
			if (( $2 >= 0 && $2 <= 5 )) &&
				flock $cam_dev -c "sed -i -e s_[[:alnum:]]*_$2_3 $cam_dev"
				then 
					echo "OK" >&$out
				else
					echo "FAILED" >&$out
			fi;;
		*) echo "FAILED" >&$out ;;
	esac
	echo "procd $LED_state"
done <&$in

echo "Disconnected $cli_in"

exit 0
