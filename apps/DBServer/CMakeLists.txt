cmake_minimum_required(VERSION 2.8)
project(dbsvr)
include_directories(
	/usr/include/glib-2.0
	/usr/lib/x86_64-linux-gnu/glib-2.0/include
	../include/
	../include/libant
	../include/libant/utils
	../include/serverbench
	../thirdparty/include
	../common
)

link_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../lib
)

file(GLOB SRC_MAIN "src/*.cc")
file(GLOB SRC_HANDLER "src/handlers/*.cc")
file(GLOB SRC_PROTO "src/proto/*.cc")
file(GLOB SRC_TABLE "src/tables/*.cc")
file(GLOB SRC_CORE "src/core/*.cc")
file(GLOB SRC_CSV "src/CSV/*.cc")
file(GLOB SRC_COMMON "../common/Proto/*.cc")

add_compile_options(-std=c++11 -Wall -Wsign-compare -pipe -D_GNU_SOURCE -fPIC -D_REENTRANT -MMD -Wsign-compare -Werror=return-type -DMYSQLPP_MYSQL_HEADERS_BURIED -DUSE_SERVERBENCH)
add_compile_options(-D_PROJ_DIR_="${CMAKE_CURRENT_SOURCE_DIR}/")
#for rapidjson std::string
add_compile_options(-DRAPIDJSON_HAS_STDSTRING)
set(CMAKE_BUILD_TYPE "debug")
if(CMAKE_BUILD_TYPE MATCHES "debug")
	add_compile_options(-g -DDEBUG)
else()
	add_compile_options(-g -DDEBUG -DRELEASE)
endif()

add_library(${PROJECT_NAME} SHARED
	${SRC_MAIN}
	${SRC_HANDLER}
	${SRC_PROTO}
	${SRC_TABLE}
	${SRC_CORE}
	${SRC_CSV}
	${SRC_COMMON}
)

target_link_libraries( ${PROJECT_NAME}
	mysqlpp
	protobuf
	ant
	hiredis
	crypto
)
#set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX "")
#set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set(LIBRARY_OUTPUT_PATH ../bin)
