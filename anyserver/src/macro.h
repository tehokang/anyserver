#ifndef __ANY_MACRO_H__
#define __ANY_MACRO_H__

#if defined(__clang__) || __cplusplus >= 201103L
#define OVERRIDE override
#else
#define OVERRIDE
#endif

#define SAFE_DELETE(x) do{if(x){delete x;x=nullptr;}}while(0)
#define SAFE_FREE(x) do{if(x){free(x);x=nullptr;}}while(0)

#include "logger.h"
#include <string.h>

#define __SHORT_FILE__ \
(strrchr(__FILE__,'/') \
? strrchr(__FILE__,'/')+1 \
: __FILE__ )

#if defined(CONFIG_DEBUG)
#if defined(USE_PLATFORM_LOGGER)
/* USE-CASE of HxLOG Humax octo LOGGER */
#define LOG_DEBUG PLATFORM_LOG_DEBUG
#define LOG_INFO PLATFORM_LOG_INFO
#define LOG_WARNING PLATFORM_LOG_WARNING
#define LOG_ERROR PLATFORM_LOG_ERROR
#define LOG_KEY PLATFORM_LOG_KEY

#define REMOTE_DEBUG PLATFORM_REMOTE_DEBUG
#define REMOTE_INFO PLATFORM_REMOTE_INFO
#define REMOTE_WARNING PLATFORM_REMOTE_WARNING
#define REMOTE_ERROR PLATFORM_REMOTE_ERROR
#else

/* USE-CASE of ARCHON LOGGER */
#define LOG_DEBUG(fmt, ...) Logger::debug \
        (__SHORT_FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);
#define LOG_INFO(fmt, ...)  Logger::info \
        (__SHORT_FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);
#define LOG_WARNING(fmt, ...) Logger::warning \
        (__SHORT_FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);
#define LOG_ERROR(fmt, ...) Logger::error \
        (__SHORT_FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);
#define LOG_KEY(fmt, ...) Logger::debug \
        (__SHORT_FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);

#define REMOTE_DEBUG LOG_DEBUG
#define REMOTE_INFO LOG_INFO
#define REMOTE_WARNING LOG_WARNING
#define REMOTE_ERROR LOG_ERROR

#endif

#else
#define LOG_DEBUG(fmt, ...)
#define LOG_INFO(fmt, ...)
#define LOG_WARNING(fmt, ...)
#define LOG_ERROR(fmt, ...)
#define LOG_KEY(fmt, ...)

#define REMOTE_DEBUG(fmt, ...)
#define REMOTE_INFO(fmt, ...)
#define REMOTE_WARNING(fmt, ...)
#define REMOTE_ERROR(fmt, ...)
#endif

/*
 * Return macro for each state
 */
#define RETURN_IF_NULL(x) \
    do{\
        if(x==nullptr){\
            LOG_ERROR("RETURN_IF_NULL \n");\
            return;\
        }\
    }while(0)

#define RETURN_IF_TRUE(x) \
    do{ \
        if(x) { \
            return;\
        }\
    }while(0)

#define RETURN_IF_FALSE(x) \
    do{\
        if(!(x)){\
            LOG_ERROR("RETURN_IF_FALSE \n");\
            return;\
        }\
    }while(0)

#define RETURN_NULL_IF_NULL(x) \
    do{\
        if(x==nullptr){\
            LOG_ERROR("RETURN_NULL_IF_NULL \n");\
            return nullptr;\
        }\
    }while(0)

#define RETURN_FALSE_IF_NULL(x) \
    do{\
        if(x==nullptr){\
            LOG_ERROR("RETURN_FALSE_IF_NULL \n");\
            return false;\
        }\
    }while(0)

#define RETURN_FALSE_IF_FALSE(x) \
    do{\
        if(!(x)){\
            LOG_ERROR("RETURN_FALSE_IF_FALSE \n");\
            return false;\
        }\
    }while(0)

#define RETURN_FALSE_IF_TRUE(x) \
    do{ \
        if(x) { \
            LOG_ERROR("RETURN_FALSE_IF_TRUE \n"); \
            return false;\
        }\
    }while(0)

#define RETURN_TRUE_IF_TRUE(x) \
    do{ \
        if(x) { \
            return true;\
        }\
    }while(0)

#define RETURN_NULL_IF_FALSE(x) \
    do{\
        if(!(x)){\
            LOG_ERROR("RETURN_NULL_IF_FALSE \n");\
            return nullptr;\
        }\
    }while(0)

#endif

/**
 *
 */
#define BASE_NOT_IMPLMENT_MUST_BE_OVERRIDED    false
