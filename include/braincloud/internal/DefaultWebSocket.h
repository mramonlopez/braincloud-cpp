#ifndef _DEFAULTWEBSOCKER_H_
#define _DEFAULTWEBSOCKER_H_

#include "braincloud/internal/IWebSocket.h"

#include <libwebsockets.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

namespace BrainCloud
{
    class DefaultWebSocket;

	class DefaultWebSocket : public IWebSocket
	{
	public:
        static int libWebsocketsCallback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len);

		virtual ~DefaultWebSocket();

		virtual bool isValid();

		virtual void send(const std::string& message);
		virtual std::string recv();

		virtual void close();

	protected:
		friend class IWebSocket;

		DefaultWebSocket(const std::string& address, int port, const std::map<std::string, std::string>& headers);

	private:
        void onClose();
        void onError(const char* msg);
        void onConnect();
        void onRecv(const char* buffer, int len);
        bool onProcessHeaders(unsigned char** ppBuffer, unsigned char* pEnd);
        void processSendQueue();

        struct lws_context* _pLwsContext;
        struct lws* _pLws;

        bool _isValid;
        bool _isConnecting;
        
        std::thread _updateThread;
        std::mutex _mutex;
        std::mutex _recvMutex;
        std::condition_variable _connectionCondition;
        std::condition_variable _recvCondition;
        std::map<std::string, std::string> _authHeaders;
        std::vector<std::string> _sendQueue;
        std::vector<std::string> _recvQueue;
	};
};

#endif /* _DEFAULTTCPSOCKER_H_ */
