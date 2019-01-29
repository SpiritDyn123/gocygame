XX			= clang++
LIBS		=
INCLUDES	=
ifeq ($(RELEASE), 1)
	FLAGS		= -std=c++11 -Wall -Wsign-compare -Werror=return-type -DDEBUG -DRELEASE -DMYSQLPP_MYSQL_HEADERS_BURIED -g -fPIC -MMD
#	FLAGS		= -std=c++11 -Wall -Wsign-compare -Werror=return-type -DDEBUG -DRELEASE -DMYSQLPP_MYSQL_HEADERS_BURIED -O2 -g -fPIC -MMD
else
	FLAGS		= -std=c++11 -Wall -Wsign-compare -Werror=return-type -DDEBUG -DMYSQLPP_MYSQL_HEADERS_BURIED -g -fPIC -MMD
endif
SOURCE		= $(wildcard *.cc)
OBJS		= $(patsubst %.cc, %.o, $(SOURCE))

all: $(OBJS)

%.o: %.cc Makefile
	$(XX) $(FLAGS) $(INCLUDES) -c $< -o $@
	@-cp $@ ../$@
	@-mv -f $*.d .dep.$@

clean:
	rm -rf *.o .dep.*

rebuild: clean all

c: clean

r: rebuild

-include /dev/null $(wildcard .dep.*)
