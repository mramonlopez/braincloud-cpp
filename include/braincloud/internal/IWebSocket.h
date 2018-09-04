#ifndef _IWEBSOCKET_H_
#define _IWEBSOCKET_H_

#include "ISocket.h"

namespace BrainCloud
{
    class IWebSocket : public ISocket
    {
    public:
        static IWebSocket* create(const std::string& address, int port);

        virtual ~IWebSocket() {}

    protected:
        IWebSocket() {}
    };
};

#endif /* _IWEBSOCKET_H_ */
