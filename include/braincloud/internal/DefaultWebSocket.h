#ifndef _DEFAULTWEBSOCKER_H_
#define _DEFAULTWEBSOCKER_H_

#include "braincloud/internal/IWebSocket.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

#if defined(WIN32)
extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; } 
#endif

#define ASIO_STANDALONE
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

typedef websocketpp::client<websocketpp::config::asio_tls_client> WebSocketPPClient;

namespace BrainCloud
{
    class DefaultWebSocket;
    
    class ConnectionHandler
    {
    public:
        typedef websocketpp::lib::shared_ptr<ConnectionHandler> ptr;

        ConnectionHandler(DefaultWebSocket* pWebSocket);

        void on_open(WebSocketPPClient* c, websocketpp::connection_hdl hdl);
        void on_fail(WebSocketPPClient* c, websocketpp::connection_hdl hdl);
        void on_message(
            websocketpp::connection_hdl hdl,
            WebSocketPPClient::message_ptr msg);

        bool getConnected();

        websocketpp::connection_hdl handle;

    private:
        bool _isCompleted = false;
        bool _isConnected = false;
        std::mutex _mutex;
        std::condition_variable _condition;
        DefaultWebSocket* _pWebSocket;
    };

	class DefaultWebSocket : public IWebSocket
	{
	public:
		virtual ~DefaultWebSocket();

		virtual bool isValid() const;

		virtual void send(const std::string& message);
		virtual std::string recv();

		virtual void close();

	protected:
		friend class IWebSocket;

		DefaultWebSocket(const std::string& address, int port);

	private:
        friend class ConnectionHandler;

        void queueMessage(const std::string& message);

        WebSocketPPClient _client;
        websocketpp::lib::shared_ptr<websocketpp::lib::thread> _thread;
        WebSocketPPClient::connection_ptr _connection;
        websocketpp::connection_hdl _handle;
        ConnectionHandler::ptr _pConnectionHandler;
        std::atomic<bool> _isValid;

        std::queue<std::string> _messageQueue;
        std::mutex _messageQueueMutex;
        std::condition_variable _messageQueueCondition;
	};
};

#endif /* _DEFAULTTCPSOCKER_H_ */
