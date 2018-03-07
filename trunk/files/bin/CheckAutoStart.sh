#!/bin/sh
#打印出当前的jboss进程：grep jboss查询的jboss进程，grep -v "grep" 去掉grep进程 

funName=fszigbeegw
#cmds="/bin/fszigbeegw /dev/ttyS0"

while true
do
	#gwThread=`ps |grep $funName | grep -v "grep"` 
	#echo $gwThread

	#查询jboss进程个数：wc -l 返回行数
	count=`ps | grep -w ${funName} | grep -v "grep" | wc -l`
	echo "count = $count"

	if [ "${count}" -lt "1" ];then
		/usr/bin/logger -s "start ${funName}" -t "CheckAutoStart"
		/bin/${funName} /dev/ttyS0 > null &
	elif [ "${count}" -gt "1" ];then
		/usr/bin/logger -s "more than 1 ${funName},killall ${funName}" -t "CheckAutoStart"
		killall -9 ${funName}
		#/bin/${funName} /dev/ttyS0 > null &
	fi
	sleep 5
	#杀死僵尸进程
	NUM_STAT=`ps | grep -w ${funName} | grep T | grep -v "grep" | wc -l`
	if [ "${NUM_STAT}" -gt "0" ];then
		/usr/bin/logger -s "more than 1 ${funName},killall ${funName}" -t "CheckAutoStart"
		#killall -9 ${funName}
		#/bin/${funName} /dev/ttyS0 > null &
		reboot
	fi
	sleep 5
	#重启设备
	STAT=`ps | grep -w ${funName} | grep -v "grep" | awk '{print $3}'`
	if [ "${STAT}" -lt "1" ];then
		/usr/bin/logger -s "Reboot Device" -t "CheckAutoStart"
		reboot
	fi
	sleep 5
	#判断进程是否进入到了CLOSE_WAIT状态，
	close_wait=`netstat -tanp | grep ${funName} | grep CLOSE_WAIT | wc -l`
	echo "close_wait = $close_wait"
	if [ "${close_wait}" -gt "0" ];then 
		/usr/bin/logger -s "netstat CLOSE_WAIT " -t "CheckAutoStart"
		killall -9 ${funName}
		#/bin/${funName} /dev/ttyS0 > null &
	fi
	
	sleep 10
done


