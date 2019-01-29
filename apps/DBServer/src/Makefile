XX			= clang++
LIBS		= -lmysqlpp -lprotobuf -lhiredis -lant
LIB_PATH	=
INCLUDES	= -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/
ifeq ($(RELEASE), 1)
	FLAGS	= -std=c++11 -Wall -Wsign-compare -Werror=return-type -DDEBUG -DRELEASE -DMYSQLPP_MYSQL_HEADERS_BURIED -g -fPIC -MMD
else
	FLAGS	= -std=c++11 -Wall -Wsign-compare -Werror=return-type -DDEBUG -DMYSQLPP_MYSQL_HEADERS_BURIED -g -fPIC -MMD
endif
SOURCE		= $(wildcard *.cc)
OBJS		= $(patsubst %.cc, %.o, $(SOURCE))
TARGET		= libdbsvr.so

all: $(TARGET)

#$(TARGET): protos subobjs $(OBJS)
$(TARGET): subobjs $(OBJS)
	$(XX) $(FLAGS) -shared $(LIB_PATH) *.o $(LIBS) -o $(TARGET)

%.o: %.cc Makefile
	$(XX) $(FLAGS) $(INCLUDES) -c $< -o $@
	@-mv -f $*.d .dep.$@

-include /dev/null $(wildcard .dep.*)

#-------------------------------------------
# recursively make into folders containing
# source files for SUBOBJS(if any)
#-------------------------------------------

#protos:
#	@-cd proto_defs; \
#	make all; \
#	cd ..

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
	rm -rf *.o .dep.* $(TARGET)
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
