PROTO_DIR     = ../DigiMonProto/
PROTO_DST_DIR = $(CURDIR)/src/proto
TARGET        = libdbsvr.so

all: protos subobjs
	@rm ./bin/$(TARGET); cp src/$(TARGET) ./bin

protos:
	@-cd $(PROTO_DIR); \
	make SRC_FILES="SvrProtoID.proto ErrCode.proto Enum.proto CommonMsg.proto db.proto CSCoreMsg.proto Core.proto CenterServer.proto Battle.proto ConfigServer.proto Pay.proto ExOptions.proto" DST_DIR=$(PROTO_DST_DIR); \
	cd -

#-------------------------------------------
# recursively make into folders containing
# source files for SUBOBJS(if any)
#-------------------------------------------

subobjs:
	@make_all_sub_dir() \
	{ \
		for dir in `ls`; do \
			if [ -d $$dir ]; then \
				cd $$dir; \
				make_all_sub_dir; \
				if [ -f makefile -o -f Makefile ]; then \
					make all; \
				fi; \
				cd ..; \
			fi; \
		done; \
	}; \
	make_all_sub_dir

clean:
	@make_clean_sub_dir() \
	{ \
		for dir in `ls`; do \
			if [ -d $$dir ]; then \
				cd $$dir; \
				make_clean_sub_dir; \
				if [ -f makefile -o -f Makefile ]; then \
					make clean; \
				fi; \
				cd ..; \
			fi; \
		done; \
	}; \
	make_clean_sub_dir

rebuild: clean all

c: clean

r: rebuild
