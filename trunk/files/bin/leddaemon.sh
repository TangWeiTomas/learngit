#!/bin/sh

#
#本程序用于设置WAN与LAN相对应的LED灯
#

echo "wan lan Led daemon"
#swconfig dev <dev> [port <port>|vlan <vlan>] (help|set <key> <value>|get <key>|load <config>|show)

wan_led="/sys/class/leds/wr8305rt:wan/brightness"
lan_led="/sys/class/leds/wr8305rt:lan/brightness"

def_get_netdev_state() {
        local port=$1
		local bin="/sbin/swconfig"
		
		if [ ! -f $bin ];then
			return 2
		fi
		
        netstatestr=`$bin dev switch0 port $port get link`
		
        #echo $netstatestr
        local result=$(echo $netstatestr | grep "link:up")
        #echo $result
        if [ "$result" != "" ];then
                return 1
        fi
		
        return 0
}

def_set_netdev_led_state(){
	local device=$1
	local state=$2
	local curentst
	
	if [ $state -eq 2 ];then
		echo "state error"
		return 2
	fi
	
	if [ -f $device ];then
	
		curentst=`cat $device`
		#当状态改变的时候设置LED
		if [ $curentst != $state ];then
			#echo $device $curentst
			echo $2 > $device
		fi
		return 0
	fi
	
	return 1
}

while true
do
	#WAN口检测是否有网线插入
	def_get_netdev_state 4
	wan=$?
	#设置WAN口相对应的LED灯
	def_set_netdev_led_state $wan_led $wan
	
	def_get_netdev_state 3
	lan=$?
	def_set_netdev_led_state $lan_led $lan
	
	sleep 2
done

