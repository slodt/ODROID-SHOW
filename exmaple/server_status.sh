#!/bin/bash

flag=0
serialPort="/dev/ttyUSB0"

trap "flag=1" SIGINT SIGKILL SIGTERM

./port_open &
subppid=$!

sleep 0.1
echo -ne "\ec\e[1s\e[3r" > $serialPort
sleep 0.1

function cpu_state {
	cpuFreqM=$(echo "scale=0; " `cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq` "/1000" | bc)
	cpuTempM=$(echo "scale=1; " `cat /sys/class/thermal/thermal_zone0/temp` "/1000" | bc)
}

while true
do
	if [ $flag -ne 0 ] ; then
		#echo -ne "\ec\e[2s\e[1r" > $serialPort
		#sleep 0.1
		kill $subppid
		exit
	fi
		echo -ne "\e[H\e[35mTime : \e[36m" > $serialPort
		date +"%T" > $serialPort
		sleep 0.1
		echo -ne "\eE\eM\e[32mcore0:\e[31m" > $serialPort
		sleep 0.1
		mpstat -P 0 | grep -A1 "usr" | grep -v "usr" | awk '{print""$4"% "}' > $serialPort
		sleep 0.1
		echo -ne "\eM\e[32mcore1:\e[31m" > $serialPort
		sleep 0.1
		mpstat -P 1 | grep -A1 "usr" | grep -v "usr" | awk '{print ""$4"%   "}' > $serialPort
		sleep 0.1
		echo -ne "\eM\e[32mcore2:\e[31m" > $serialPort
		sleep 0.1
		mpstat -P 2 | grep -A1 "usr" | grep -v "usr" | awk '{print ""$4"% "}' > $serialPort
		sleep 0.1
		echo -ne "\r\e[33m" > $serialPort
		sleep 0.1
		df -h | grep "sda" | awk '{print ""$1"\n\rSize  : "$2"\n\rUsed  : "$3"\n\rAvail : "$4"\n\rUse   : "$5""}' > $serialPort
		echo -ne "$sdaInfo" > $serialPort
		sleep 0.1
		echo -ne "\r\e[33m" > $serialPort
		sleep 0.1
		df -h | grep "sdb" | awk '{print ""$1"\n\rSize  : "$2"\n\rUsed  : "$3"\n\rAvail : "$4"\n\rUse   : "$5""}' > $serialPort
		echo -ne "$sdaInfo" > $serialPort
		sleep 0.1
		echo -ne "\r\e[33m" > $serialPort
		sleep 0.1
		df -h | grep "sdc" | awk '{print ""$1"\n\rSize  : "$2"\n\rUsed  : "$3"\n\rAvail : "$4"\n\rUse   : "$5""}' > $serialPort
		echo -ne "$sdaInfo" > $serialPort
		sleep 0.1
		echo -ne "\r\e[33m" > $serialPort
		sleep 0.1
		df -h | grep "sdd" | awk '{print ""$1"\n\rSize  : "$2"\n\rUsed  : "$3"\n\rAvail : "$4"\n\rUse   : "$5""}' > $serialPort
		echo -ne "$sdaInfo" > $serialPort
		sleep 0.1
		cpu_state
		sleep 0.1
		echo -ne "\r" > $serialPort
		echo -ne "\e[33mCPU Freq: \e[37m"$cpuFreqM"MHz   \eE" > $serialPort
		echo -ne "\e[33mCPU Temp: \e[37m$cpuTempM\e   " > $serialPort
		sleep 0.1
done