#ifndef __POSIX_SIGNAL_INTERCEPTOR_H__
#define __POSIX_SIGNAL_INTERCEPTOR_H__

#include <map>
#include <vector>

namespace anyserver
{

class SignalHandlerIntf
{
public:
    virtual void HandleSignal(int) = 0;
    virtual ~SignalHandlerIntf() {}
};

template <typename TSignalHandler>
class SignalHandler : public SignalHandlerIntf
{
public:
    SignalHandler(TSignalHandler handler)
        : handler_(handler)
    {
    }

    virtual void HandleSignal(int signal_id) override
    {
        handler_(signal_id);
    }
private:
    TSignalHandler handler_;
};

class PosixSignalInterceptor
{
public:
    PosixSignalInterceptor();
    ~PosixSignalInterceptor();

    template <typename TSignalHandler>
    void HandleSignals(std::vector<int> signal_ids,
                     TSignalHandler handler)
    {
        for (std::vector<int>::const_iterator it = signal_ids.begin();
                it != signal_ids.end(); ++it)
        {
            HandleSignal(*it, new SignalHandler<TSignalHandler>(handler));
        }
    }

    void HandleSignal(int signal_id, SignalHandlerIntf* handler);
    void RaiseSignal(int signal_id);
private:
    typedef void (*PosixSignalHandler)(int);
    std::multimap<int, SignalHandlerIntf*> signal_handlers_;
    std::map<int, PosixSignalHandler> previous_handlers_;
};

}  // end of namespace
#endif
