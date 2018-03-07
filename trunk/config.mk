# Top level pattern, include by Makefile of child directory
# in which variable like TOPDIR, TARGET or LIB may be needed

CC= mipsel-openwrt-linux-gcc
MAKE=make

AR= mipsel-openwrt-linux-ar rcs

RM = -rm -rf

ifneq ($(DEBUG),)
	CFLAGS	+= -g -D DEBUG 
endif

#REV=$(shell svn info $(TOPDIR) |grep "Last Changed Rev: " |sed -e "s/Last Changed Rev: "//g)


CFLAGS += -DUSRTP=0x01 -D_GNU_SOURCE -D OPENWRT_TEST -D SYSLOG_TRACE 

#-DSVERSION="$(REV)"


#-DVERSION=\"$(shell git describe --tags)\"

#编译后的二进制文件位置
OBJS_DIR=$(TOPDIR)/objs
#自定义库安装位置
LIBS_DIR=$(TOPDIR)/lib/lib
FILES_DIR=$(TOPDIR)/files
LIBPATH=$(FILES_DIR)/lib



dirs:=$(shell find . -maxdepth 1 -type d)
dirs:=$(basename $(patsubst ./%,%,$(dirs)))
dirs:=$(filter-out $(exclude_dirs),$(dirs))
SUBDIRS := $(dirs)

SRCS=$(wildcard *.c)
OBJS=$(SRCS:%.c=%.o)
DEPENDS=$(SRCS:%.c=%.d)

all:subdirs $(OBJS)  $(LIB) $(TARGET)

$(LIB):$(OBJS) 
	@$(CC) -shared -fPIC  $(CFLAGS) -o $@ $(OBJS)
	@cp $@ $(LIBS_DIR)
	@cp $@ $(FILES_DIR)/lib/

subdirs:$(SUBDIRS)
	@for dir in $(SUBDIRS);\
	do $(MAKE) -C $$dir all||exit 1;\
	done
	
$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) 

$(OBJS):%.o:%.c
ifeq ($(LIB),)
	$(CC) $(CFLAGS) -c $< -o $@ 
	@cp $@ $(OBJS_DIR)/
else
	@$(CC) $(CFLAGS) -fPIC -c $< -o $@
endif

-include $(DEPENDS)

$(DEPENDS):%.d:%.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[:]*,\1.o $@:,g' < $@.$$$$ > $@; \
	rm $@.$$$$
	
clean:
	@for dir in $(SUBDIRS);\
	do $(MAKE) -C $$dir clean||exit 1;\
	done
	@$(RM) $(TARGET) $(LIB)  $(OBJS) $(DEPENDS) *.d.*
