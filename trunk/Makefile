TOPDIR=.
exclude_dirs= lib objs files discard template doc

MT7620_14_07_PATH_NEW=~/workSpace/openwrt/mt7620_14.07_new/package/gateway/files
MT7620_14_07_PATH=~/workSpace/openwrt/mt7620_14.07/package/gateway/files
MT7620_CC_PATH	= ~/workSpace/openwrt/openwrt_cc_mt7620/package/gateway/files
MT7688_PATH=~/workSpace/openwrt/openwrt_cc_mt7688/package/gateway/files


INSTALL_OBJS=$(MT7688_PATH)

VERSION:=$(shell $(TOPDIR)/version)

exe: create_output all  fszigbeegw
	
include $(TOPDIR)/include.mk

CFLAGS += -rdynamic -lpthread -lm -lrt -lcurl  -levent  -liwinfo -lnl-tiny -luci -lubox -linfrared -lsocketudp -ldl -lapr-1 -laprutil-1 -lmxml -lexpat
#-lcrypto -lssl

include $(TOPDIR)/config.mk

fszigbeegw:
	@echo "********************************************************" 
	@echo "Building $@..."
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $(FILES_DIR)/bin/$@ $(wildcard $(OBJS_DIR)/*.o) 
	@echo "Compile Successful"
	@echo "********************************************************" 

create_output:	
	@echo "********************************************************" 	
	@echo "Check existance of output folder" 	
	@-if test ! -d $(OBJS_DIR); then mkdir -p $(OBJS_DIR); fi
	@-if test ! -d $(LIBS_DIR); then mkdir -p $(LIBS_DIR); fi
	@-if test ! -d $(LIBPATH); then mkdir -p $(LIBPATH); fi	
	@echo "********************************************************" 
	
install:
	@echo "Install to files"
	@chmod 777 -R $(FILES_DIR)/bin/* $(FILES_DIR)/airkiss/*
	@cp -rf $(TOPDIR)/files/* $(INSTALL_OBJS)/

	
clean:
	@echo "********************************************************" 
	@for dir in $(SUBDIRS);\
	do $(MAKE) -C $$dir clean||exit 1;\
	done
	@$(RM) fszigbeegw $(OBJS_DIR)/* $(FILES_DIR)/bin/fszigbeegw  $(FILES_DIR)/lib/* $(LIBS_DIR)/*
	@echo "clean Successful"
	@echo "********************************************************" 
