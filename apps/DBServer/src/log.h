#pragma once

#include <serverbench/log.hpp>
#define JSON_LOG_WRITER(data)	write_log(log_lvl_user_def, "%s\n", data);
#include <libant/utils/JsonLog.h>
