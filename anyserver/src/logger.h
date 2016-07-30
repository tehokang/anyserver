#ifndef __ANY_LOGGER_H__
#define __ANY_LOGGER_H__

#include <stdio.h>
#include <string>
#include <string.h>
#include <pthread.h>

using namespace std;

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
#define LOG_DEBUG(...) Logger::debug \
        (__SHORT_FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);
#define LOG_INFO(...)  Logger::info \
        (__SHORT_FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);
#define LOG_WARNING(...) Logger::warning \
        (__SHORT_FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);
#define LOG_ERROR(...) Logger::error \
        (__SHORT_FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);
#define LOG_KEY(...) Logger::debug \
        (__SHORT_FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);

#define REMOTE_DEBUG LOG_DEBUG
#define REMOTE_INFO LOG_INFO
#define REMOTE_WARNING LOG_WARNING
#define REMOTE_ERROR LOG_ERROR

#endif

#else
#define LOG_DEBUG(...)
#define LOG_INFO(...)
#define LOG_WARNING(...)
#define LOG_ERROR(...)
#define LOG_KEY(...)

#define REMOTE_DEBUG(...)
#define REMOTE_INFO(...)
#define REMOTE_WARNING(...)
#define REMOTE_ERROR(...)
#endif


namespace anyserver
{
/**
    @defgroup Logging
    @brief A module to control to logging without dependency of platform.
    @ingroup AnyServer
    @{
*/

/**
 * @brief class to manage logging
 *
 * @note ANSI Colored logger \n
 * 3X : foreground color \n
 * 4X : background color \n
 * X = {0:black,  1:red,  2:green,
 *         3:yellow, 4:blue, 5:magenta
 *        6:cyan,   7:white }
 *
 */
class Logger
{
public:
    enum LOG_TYPE
    {
        DEBUG,
        INFO,
        ERROR,
        WARNING,
    };

    static void setLogLevel(bool info, bool debug, bool warn, bool error, \
                        bool filewrite = false, unsigned int filesize = 5 * 1024 * 1024, \
                        string rootpath = "/mnt/hd2/", string log_filename_prefix="noname");

    static void info(const string filename, const unsigned int linenumber, \
                        const string funcname, const string format, ...);

    static void debug(const string filename, const unsigned int linenumber, \
                        const string funcname, const string format, ...);

    static void warning(const string filename, const unsigned int linenumber, \
                        const string funcname, const string format, ...);

    static void error(const string filename, const unsigned int linenumber, \
                        const string funcname, const string format, ...);

protected:
    /**
     * @warning DO NOT CREATE Logger INSTANCE
     **/
    Logger();
    ~Logger();

    static string getDate();
    static string getLogTypeString(LOG_TYPE logtype);

    static void __log__(LOG_TYPE logtype, FILE *out,
            const string msg_prefix, const string msg);

    static void __save_logfile__(const string msg_prefix, const string msg);

    static pthread_mutex_t m_filelogging_mutex;
    static bool m_info;
    static bool m_debug;
    static bool m_warning;
    static bool m_error;
    static bool m_filewrite;
    static unsigned int m_filesize;

    enum
    {
        MAX_PREFIX_LEN = 64,
        MAX_MSG_LEN = 4*1024,
    };
    static FILE *m_log_fp;
    static string m_log_filename;
    static string m_log_filename_prefix;
    static string m_log_rootpath;
    static bool m_use_date;
};

/** @} */ // End of doxygen group

}

#endif
