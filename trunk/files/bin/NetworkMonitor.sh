#!/bin/sh 
##网络状态检测


readonly url="www.baidu.com"
readonly wName="wireless"
#wireless中ApCliSsid=aAP
readonly wAplic0DefName="aAP"
readonly wanPort="eth0.2"
readonly apPort="apcli0"
readonly networklog="/tmp/network.log"
readonly maxsize=$((1024*1024))
#网络连接模式 0:未知 1:WiFi桥接 2:网线
wClientMode=0
networkPort=4

#检测网络是否联通
NetworkConnectedCheck(){
	/bin/ping -c 5 $url > /dev/null 
	if [ $? -eq 0 ];then
		return 0 #检测网络正常
	else
		return 1 #检测网络连接异常
	fi
}

#检测无线配置文件是否配置
WirelessCfgCheck(){
	if [ -e /etc/config/wireless ];then
		ssid=`/sbin/uci get wireless.ap.ApCliSsid`
		#命令执行成功
		if [ $? -eq 0 ];then
			#WiFi无线没有没被配置
			if [ $ssid = $wAplic0DefName ];then
				return 0
			else #无线被配置
				return 2
			fi
		else
			return 1
		fi
	else
		return 1	#返回失败
	fi
}

#检测桥接是否连接到上级路由
WirelessApPortCheck(){
	ipaddr=`/sbin/ifconfig $apPort | grep "inet addr" | awk '{ print $2}' | awk -F: '{print $2}' `
	 #WAN口没有获取到IP
	if [ ! -n "$ipaddr" ];then
		return 1
	else
		return 0
	fi
}

#扫描周围的WIFI，并匹配是否有指定的WIFI名称
WirelessScanSid(){
	ssid=$1
	/usr/sbin/iwpriv ra0 set SiteSurvey=1
	sleep 2
	msg=`/usr/sbin/iwpriv ra0 get_site_survey | grep $ssid`
	if [ ! -n "$msg" ];then
		return 1
	else
		return 0
	fi
}

#监测WAN网线连接是否正常
NetworkWanMoniter(){
	date="`date '+%Y-%m-%d %H:%M:%S'`"
	WanPortStatus=`/sbin/swconfig dev switch0 port ${networkPort} show | grep "link:up"`
	#没有连接网线
	if [ ! -n "$WanPortStatus" ];then
		return 1
	else
		ipaddr=`/sbin/ifconfig $wanPort | grep "inet addr" | awk '{ print $2}' | awk -F: '{print $2}' `
		 #WAN口没有获取到IP
		if [ ! -n "$ipaddr" ];then
			echo "[${date}]NetworkWanMoniter restart" >>$networklog
			/etc/init.d/network restart
			return 2
		else
			#echo "[${date}] NetworkWanMoniter is connected" >>$networklog
			return 0
		fi
	fi
} 


#apcli0监测
WirelessApMonitor(){
	date="`date '+%Y-%m-%d %H:%M:%S'`"
	WirelessCfgCheck
	#如果被配置
	if [ $? -eq 2 ];then
		WirelessApPortCheck
		#没有获取到IP地址
		if [ $? -eq 1 ];then
			ssid=`/sbin/uci get wireless.ap.ApCliSsid`
			if [ $? -eq 0 ];then
				cnt=0
				while [ $cnt -lt 5 ]
				do
					cnt=`expr $cnt+1`
					WirelessScanSid $ssid
					if [ $? -eq 0 ];then
						echo "[${date}] WirelessApMonitor restart" >>$networklog
						#重启WIFI 
						/sbin/wifi
						break;
					fi
				done
			fi
		fi
	fi
}

#日志文件大小检测
NetworkLogFileCheck(){
	if [ -e $networklog ];then
		filesize=`ls -l $networklog | awk '{ print $5 }'`
		if [ $filesize -gt $maxsize ];then
			rm -rf $networklog
			touch $networklog
		fi
	fi
}

touch $networklog
date="`date '+%Y-%m-%d %H:%M:%S'`"
echo "[${date}] NetworkMonitor Start!!!!" >>$networklog

while true
do 
	#循环检测网络状态
	NetworkWanMoniter
	WirelessApMonitor
	NetworkLogFileCheck
	sleep 300 #间隔10分钟检测一次
done
