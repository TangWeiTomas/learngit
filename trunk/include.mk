INCLUDE += 	-I$(INCLUDE_DIR)/										\
			-I$(INCLUDE_DIR)/framework								\
			-I$(INCLUDE_DIR)/common									\
			-I$(INCLUDE_DIR)/zbSocDriver							\
			-I$(INCLUDE_DIR)/dbManager								\
			-I$(INCLUDE_DIR)/socketDriver							\
			-I$(INCLUDE_DIR)/libs/libinfrared						\
			-I$(INCLUDE_DIR)/utils									\
			-I$(INCLUDE_DIR)/libs/libsocketudp						\
			-I$(INCLUDE_DIR)/encrypt								\
			-I$(INCLUDE_DIR)/zbSocDevice							\
			-I$(INCLUDE_DIR)/gprs								\
			-I$(INCLUDE_DIR)/oss	
			
INCLUDE += 	-I$(TOPDIR)/lib/libcurl/include							\
			-I$(TOPDIR)/lib/libevent/include						\
			-I$(TOPDIR)/lib/libiwinfo/include						\
			-I$(TOPDIR)/lib/libiwinfo/include/iwinfo				\
			-I$(TOPDIR)/lib/libnl-tiny/include						\
			-I$(TOPDIR)/lib/libuci/include							\
			-I$(TOPDIR)/lib/libubox/include 						\
			-I$(TOPDIR)/lib/libopenssl/include/openssl 				\
			-I$(TOPDIR)/lib/libopenssl/include					\
			-I$(TOPDIR)/lib/libapr-1/include/apr-1/					\
			-I$(TOPDIR)/lib/libaprutil-1/include/apr-1/				\
			-I$(TOPDIR)/lib/libmxml/include/						
			
LDFLAGS +=  -L$(TOPDIR)/lib/libcurl/lib								\
			-L$(TOPDIR)/lib/libevent/lib							\
			-L$(TOPDIR)/lib/libiwinfo/lib							\
			-L$(TOPDIR)/lib/libnl-tiny/lib							\
			-L$(TOPDIR)/lib/libuci/lib								\
			-L$(TOPDIR)/lib/libubox/lib								\
			-L$(TOPDIR)/lib/libopenssl/lib							\
			-L$(TOPDIR)/lib/lib								\
			-L$(TOPDIR)/lib/libapr-1/lib							\
			-L$(TOPDIR)/lib/libaprutil-1/lib						\
			-L$(TOPDIR)/lib/libmxml/lib							
