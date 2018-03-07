## FeiLinker Zigbee Gateway
**FeiLinker**是无锡飞雪网络科技有限公司基于Openwrt推出的Zigbee网关解决方案  

## Documentation  

Please visit the [Wiki](https://git.oschina.net/Edward_Ou/fxgateway/wikis).

## Overview  

**FeiLinker**是无锡飞雪网络科技有限公司基于Openwrt推出的Zigbee网关解决方案。

## Installation  

配置编译环境(默认已经拥有交叉编译器)

    vim ~/.bashrc

添加

    STAGING_DIR=<openwrt Compile tools>/staging_dir

    OpenwrtComplie=<Openwrt Compile tools>/toolchain_mipsel_xxx_3.2/bin

    export PATH=$PATH:$OpenwrtComplie:$STAGING_DIR

下载代码

    $> git clone git@192.168.1.69:Products/FxGateway.git

代码编译

    $> cd <FxGateway path>/FxGateway/trunk && make

编译完成后会 <FxGateway path>/FxGateway/trunk/files/bin目标下面生成fszigbeegw程序

测试环境准备(将files中的文件拷贝至Openwrt网关中的相应目录中)
    
    files/bin --> /bin
	files/config -->/etc/config
	files/lib -->/lib

网关固件中还必须包含如下的库：
	 
	wireless-tools 
	libpthread 
	librt 
	libstdcpp 
	libpcap 
	libnl-tiny 
	libiwinfo
    libubox 
	libuci 
	libevent2 
	libcurl
以上的库可以在Openwrt编译的时候通过 make menuconfig 进行配置
	
## Usage  
1. PC与网关网络进行连接(有线连接或者无线连接)
2. 在PC上通过SSH工具连接到网关的终端界面,账号：root 密码:admin
3. 运行fszigbeegw

	fszigbeegw /dev/ttyS0 -d

## License  

None

## Reference Design
- [ZigBee Lighting Gateway](http://processors.wiki.ti.com/index.php/ZigBee_Lighting_Gateway)
- [ZigBee_Lighting_Gateway_SW](http://processors.wiki.ti.com/index.php/ZigBee_Lighting_Gateway_SW)

