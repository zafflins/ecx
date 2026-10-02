#pragma once

#include <zl/zlstd.h>

#define ZL_LOG_INFO 0
#define ZL_LOG_WARN 1
#define ZL_LOG_ERROR 2
#define ZL_LOG_FATAL 3

ZL_API void Log(i32 level, char* message);
ZL_API const char* GetLog(i32 level);
