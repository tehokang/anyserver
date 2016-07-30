#include "logger.h"
#include "macro.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <vector>
#include <chrono>
#include <ctime>

using namespace std;

namespace anyserver
{

#define BEGIN_OF_ANSI(out, logtype) \
    do { \
        switch ( logtype ) \
        { \
            case DEBUG: \
                fprintf(out, "\033[1;32;40m "); \
                break; \
            case INFO: \
                fprintf(out, "\033[1;32;40m "); \
                break; \
            case ERROR: \
                fprintf(out, "\033[1;31;40m " ); \
                break; \
            case WARNING: \
                fprintf(out, "\033[1;35;40m "); \
                break; \
        } \
    }while(0)

#define END_OF_ANSI(out) fprintf(out, "\033[0m");

bool Logger::m_info = true;
bool Logger::m_debug = true;
bool Logger::m_warning = true;
bool Logger::m_error = true;
bool Logger::m_filewrite = false;
unsigned int Logger::m_filesize = 0;

#if defined(__linux__)
pthread_mutex_t Logger::m_filelogging_mutex = PTHREAD_MUTEX_INITIALIZER;
#elif defined(__APPLE__)
pthread_mutex_t Logger::m_filelogging_mutex;// = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;
#else /* @todo : Window, FreeBSD */
pthread_mutex_t Logger::m_filelogging_mutex = PTHREAD_MUTEX_INITIALIZER
#endif

#define LOGGING_WITH_DATE true

FILE *Logger::m_log_fp = nullptr;
string Logger::m_log_filename;
string Logger::m_log_filename_prefix;
string Logger::m_log_rootpath;
bool Logger::m_use_date = LOGGING_WITH_DATE;

string Logger::getDate()
{
    char date[16] = {0,};
    time_t now_t = time(0);
    struct tm* t = localtime(&now_t);
    snprintf(date, sizeof(date), "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
    return string(date);
}

string Logger::getLogTypeString(LOG_TYPE logtype)
{
    switch ( logtype )
    {
        case DEBUG: return "DEBUG";
        case INFO: return "INFO";
        case ERROR: return "ERROR";
        case WARNING: return "WARN";
        default:
            break;
    }
    return "";
}

void Logger::setLogLevel(bool info, bool debug, bool warn, bool error, \
             bool filewrite, unsigned int filesize, string rootpath, string log_filename_prefix)
{
    m_info = info;
    m_debug = debug;
    m_warning = warn;
    m_error = error;
    m_filewrite = filewrite;
    m_filesize = filesize;
    m_log_rootpath = rootpath;
    m_log_filename_prefix = log_filename_prefix;
}

void Logger::__save_logfile__(const string msg_prefix, const string msg)
{
    pthread_mutex_lock( &m_filelogging_mutex );

    if ( nullptr != m_log_fp )
    {
        string message = string(msg_prefix + msg);
        fwrite(message.data(), 1, message.size(), m_log_fp);
        fflush(m_log_fp);

        struct stat st;
        if ( 0 == stat(m_log_filename.data(), &st) )
        {
            if ( m_filesize < st.st_size )
            {
                fclose(m_log_fp);
                m_log_fp = nullptr;
            }
        }
    }
    else
    {
        /* create a log file */
        char _suffix[64] = {'\0', };
        time_t timer;
        struct tm *t;
        timer = time(NULL);
        t = localtime(&timer);
        m_log_filename.assign(m_log_rootpath);
        snprintf(_suffix, sizeof(_suffix),
                    "/%s_%04d%02d%02d_%02d%02d%02d.log", \
                    m_log_filename_prefix.data(), \
                    t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, \
                    t->tm_hour, t->tm_min, t->tm_sec);
        m_log_filename.append(_suffix);
        m_log_fp = fopen(m_log_filename.data(), "w+");
        fprintf(stdout, "Created a log file [%s] \n", m_log_filename.data());
        if ( m_log_fp == nullptr )
        {
            fprintf(stderr, "Couldn't create archon logging file [%s] \n", m_log_filename.data());
        }
    }
    pthread_mutex_unlock( &m_filelogging_mutex );
}

void Logger::__log__(LOG_TYPE logtype, FILE *out, const string msg_prefix, const string msg)
{
    BEGIN_OF_ANSI( out, logtype );
    fprintf(out, "%s %s", msg_prefix.data(), msg.data());
    END_OF_ANSI(out);

    if ( m_filewrite == true )
        __save_logfile__(msg_prefix, msg);
}

#define LOGGING(enable, logtype, file, line, func, format) \
    do { \
        if ( enable ) \
        { \
            va_list arglist; \
            va_start(arglist, format); \
            char msg_prefix[MAX_PREFIX_LEN], msg[MAX_MSG_LEN]; \
            memset(msg_prefix, 0x00, sizeof(msg_prefix)); \
            memset(msg, 0x00, sizeof(msg)); \
            if ( m_use_date ) \
                snprintf(msg_prefix, sizeof(msg_prefix), "[%s] [%s][%s:%s:%d] ", \
                        getLogTypeString(logtype).data(), getDate().data(), \
                        filename.data(), funcname.data(), linenumber); \
            else \
                snprintf(msg_prefix, sizeof(msg_prefix), "[%s] [%s:%s:%d] ", \
                        getLogTypeString(logtype).data(), \
                        filename.data(), funcname.data(), linenumber); \
            vsnprintf(msg, sizeof(msg), format.data(), arglist); \
            __log__(logtype, stdout, msg_prefix, msg); \
            va_end(arglist); \
        } \
    } while(0)

void Logger::info(const string filename, const unsigned int linenumber, \
             const string funcname, const string format, ...)
{
    LOGGING(m_info, LOG_TYPE::INFO, filename, linenumber, funcname, format);
}

void Logger::debug(const string filename, const unsigned int linenumber, \
             const string funcname, const string format, ...)
{
    LOGGING(m_debug, LOG_TYPE::DEBUG, filename, linenumber, funcname, format);
}

void Logger::warning(const string filename, const unsigned int linenumber, \
             const string funcname, const string format, ...)
{
    LOGGING(m_warning, LOG_TYPE::WARNING, filename, linenumber, funcname, format);
}

void Logger::error(const string filename, const unsigned int linenumber, \
             const string funcname, const string format, ...)
{
    LOGGING(m_error, LOG_TYPE::ERROR, filename, linenumber, funcname, format);
}

}
