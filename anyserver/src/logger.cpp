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
                fprintf(out, "\033[1;32;40m [DEBUG] "); \
                break; \
            case INFO: \
                fprintf(out, "\033[1;32;40m [INFO] "); \
                break; \
            case ERROR: \
                fprintf(out, "\033[1;31;40m [ERROR] " ); \
                break; \
            case WARNING: \
                fprintf(out, "\033[1;35;40m [WARN] "); \
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
pthread_mutex_t Logger::m_filelogging_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#elif defined(__APPLE__)
pthread_mutex_t Logger::m_filelogging_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;
#else /* @todo : Window, FreeBSD */

#endif

FILE *Logger::m_log_fp = NULL;
string Logger::m_log_filename(1024, '\0');
string Logger::m_log_rootpath(1024, '\0');

#define LOGGING_WITH_DATE

#ifdef LOGGING_WITH_DATE
#define _LOG_PREFIX_ "[%s][%s:%s:%d] "
#else
#define _LOG_PREFIX_ "[%s:%s:%d] "
#endif
string Logger::getDate()
{
    char date[16] = {0,};
    time_t now_t = time(0);
    struct tm* t = localtime(&now_t);
    snprintf(date, sizeof(date), "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
    return string(date);
}

void Logger::setLogLevel(bool info, bool debug, bool warn, bool error, \
             bool filewrite, unsigned int filesize, string rootpath)
{
    m_info = info;
    m_debug = debug;
    m_warning = warn;
    m_error = error;
    m_filewrite = filewrite;
    m_filesize = filesize;
    m_log_rootpath = rootpath;
}

void Logger::__save_logfile__(FILE *out, const string filename, const string funcname, \
             const unsigned int linenumber, const string format, va_list arglist)
{
    pthread_mutex_lock( &m_filelogging_mutex );

    string msg_header(256, '\0');
    string msg_body(2048, '\0');

#ifdef LOGGING_WITH_DATE
    snprintf((char*)msg_header.data(), msg_header.length(), _LOG_PREFIX_, getDate().data(), \
            filename.data(), funcname.data(), linenumber);
#else
    snprintf((char*)msg_header.data(), msg_header.length(), _LOG_PREFIX_, filename.data(), \
            funcname.data(), linenumber);
#endif

    vsnprintf((char*)msg_body.data(), msg_body.length(), format.data(), arglist);

    if ( NULL != m_log_fp )
    {
        string message = string(msg_header+msg_body);
        fwrite(message.data(), 1, message.size(), m_log_fp);
        fflush(m_log_fp);

        struct stat st;
        if ( 0 == stat(m_log_filename.data(), &st) )
        {
            if ( m_filesize < st.st_size )
            {
                fclose(m_log_fp);
                m_log_fp = NULL;
            }
        }
    }
    else
    {
        /* create a log file */
        char _suffix[512] = {'\0', };
        time_t timer;
        struct tm *t;
        timer = time(NULL);
        t = localtime(&timer);
        m_log_filename.assign(m_log_rootpath);
        snprintf(_suffix, sizeof(_suffix),
                    "/archon_%04d%02d%02d_%02d%02d%02d.log", \
                    t->tm_year + 1900, \
                    t->tm_mon + 1, \
                    t->tm_mday, \
                    t->tm_hour, \
                    t->tm_min, \
                    t->tm_sec);
        m_log_filename.append(_suffix);
        m_log_fp = fopen(m_log_filename.data(), "w+");
        if ( m_log_fp == nullptr )
        {
            fprintf(stderr, "Couldn't create archon logging file \n");
        }
    }
    pthread_mutex_unlock( &m_filelogging_mutex );
}

void Logger::__log__(LOG_TYPE logtype, FILE *out, const string filename, \
             const string funcname, const unsigned int linenumber, const string format, \
             va_list arglist)
{
    BEGIN_OF_ANSI( out, logtype );

#ifdef LOGGING_WITH_DATE
    fprintf(out, _LOG_PREFIX_, \
            getDate().data(), \
            filename.data(), \
            funcname.data(), \
            linenumber);
    vfprintf(out, format.data(), arglist);
#else
    fprintf(out, _LOG_PREFIX_, \
            filename.data(), \
            funcname.data(), \
            linenumber);
    vfprintf(out, format.data(), arglist);
#endif

    END_OF_ANSI(out);

    if ( m_filewrite == true )
        __save_logfile__(out, filename, funcname, linenumber, format, arglist);
}

void Logger::info(const string filename, const unsigned int linenumber, \
             const string funcname, const string format, ...)
{
    if ( m_info )
    {
        va_list arglist;
        va_start(arglist, format);

        __log__(LOG_TYPE::INFO, stdout, filename, funcname, linenumber, format, arglist);

        va_end(arglist);
    }
}

void Logger::debug(const string filename, const unsigned int linenumber, \
             const string funcname, const string format, ...)
{
    if ( m_debug )
    {
        va_list arglist;
        va_start(arglist, format);

        __log__(LOG_TYPE::DEBUG, stdout, filename, funcname, linenumber, format, arglist);

        va_end(arglist);
    }
}

void Logger::warning(const string filename, const unsigned int linenumber, \
             const string funcname, const string format, ...)
{
    if ( m_warning )
    {
        va_list arglist;
        va_start(arglist, format);

        __log__(LOG_TYPE::WARNING, stdout, filename, funcname, linenumber, format, arglist);

        va_end(arglist);
    }
}

void Logger::error(const string filename, const unsigned int linenumber, \
             const string funcname, const string format, ...)
{
    if ( m_error )
    {
        va_list arglist;
        va_start(arglist, format);

        __log__(LOG_TYPE::ERROR, stdout, filename, funcname, linenumber, format, arglist);

        va_end(arglist);
    }
}

}
