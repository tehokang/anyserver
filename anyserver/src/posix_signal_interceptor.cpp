#include "posix_signal_interceptor.h"

#include <cassert>
#include <csignal>
#include <cstdlib>

namespace anyserver
{

PosixSignalInterceptor* g_CurrentPosixSignalInterceptor = NULL;
void InterceptorSignalHandler(int signal_id)
{
    assert(g_CurrentPosixSignalInterceptor != NULL);
    g_CurrentPosixSignalInterceptor->RaiseSignal(signal_id);
}

PosixSignalInterceptor::PosixSignalInterceptor()
{
    assert(g_CurrentPosixSignalInterceptor == NULL);
    g_CurrentPosixSignalInterceptor = this;
}

PosixSignalInterceptor::~PosixSignalInterceptor()
{
    for (std::map<int, PosixSignalHandler>::iterator
    it = previous_handlers_.begin();
    it != previous_handlers_.end(); ++it)
    {
        std::signal(it->first, it->second);
    }

    typedef std::multimap<int, SignalHandlerIntf*>::iterator Iterator;
    for (Iterator it = signal_handlers_.begin(); it != signal_handlers_.end();
            ++it)
    {
        delete it->second;
    }

    assert(g_CurrentPosixSignalInterceptor == this);
    g_CurrentPosixSignalInterceptor = NULL;
}

void PosixSignalInterceptor::HandleSignal(int signal_id, SignalHandlerIntf* handler)
{
    if (previous_handlers_.find(signal_id) == previous_handlers_.end())
    {
        PosixSignalHandler previous_handler =
                std::signal(signal_id, &InterceptorSignalHandler);
        previous_handlers_.insert(std::make_pair(signal_id, previous_handler));
    }
    signal_handlers_.insert(std::make_pair(signal_id, handler));
}

void PosixSignalInterceptor::RaiseSignal(int signal_id)
{
    typedef std::multimap<int, SignalHandlerIntf*>::iterator Iterator;
    typedef std::pair<Iterator, Iterator> Range;
    Range range = signal_handlers_.equal_range(signal_id);
    for (Iterator it = range.first; it != range.second; ++it)
        it->second->HandleSignal(signal_id);
}

}  // end of namespace
