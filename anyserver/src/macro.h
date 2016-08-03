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
