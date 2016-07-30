#ifndef __ANY_LOGGER_H__
#define __ANY_LOGGER_H__

#include <stdio.h>
#include <string>
#include <pthread.h>

using namespace std;

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
                        string rootpath = "/mnt/hd2/", string prefix="noname");

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
        MAX_FILENAME_LEN = 256,
        MAX_PREFIX_LEN = 64,
        MAX_MSG_LEN = 2048,
    };
    static FILE *m_log_fp;
    static char m_log_filename[MAX_FILENAME_LEN];
    static char m_log_rootpath[MAX_FILENAME_LEN];
    static char m_log_prefix[MAX_PREFIX_LEN];
    static bool m_use_date;
};

/** @} */ // End of doxygen group

}

#endif
