#include "braincloud/internal/DefaultWebSocket.h"

#include <asio/ssl/context_base.hpp>

namespace BrainCloud
{
    IWebSocket* IWebSocket::create(const std::string& address, int port)
    {
        return new DefaultWebSocket(address, port);
    }
    
    ConnectionHandler::ConnectionHandler(DefaultWebSocket* pWebSocket)
        : _pWebSocket(pWebSocket)
    {
    }

    void ConnectionHandler::on_open(
        WebSocketPPClient* c, 
        websocketpp::connection_hdl hdl)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _isConnected = true;
        _isCompleted = true;
        handle = hdl;
        _condition.notify_all();
    }

    void ConnectionHandler::on_fail(
        WebSocketPPClient* c, 
        websocketpp::connection_hdl hdl)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _isConnected = false;
        _isCompleted = true;
        _condition.notify_all();
    }

    void ConnectionHandler::on_message(
        websocketpp::connection_hdl hdl,
        WebSocketPPClient::message_ptr msg)
    {
        // Extract message
        std::string message = msg->get_payload();
        // message = msg->get_payload();
        // if (msg->get_opcode() == websocketpp::frame::opcode::text)
        // {
        //     message = msg->get_payload();
        // }
        // else
        // {
        //     message = websocketpp::utility::to_hex(msg->get_payload()));
        // }

        _pWebSocket->queueMessage(message);
    }

    bool ConnectionHandler::getConnected()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _condition.wait(lock, [this]() { return _isCompleted; });
        return _isConnected;
    }

    DefaultWebSocket::~DefaultWebSocket()
    {
        if (_thread && _thread->joinable())
        {
            _thread->join();
        }
    }
    
    DefaultWebSocket::DefaultWebSocket(const std::string& address, int port)
    {
        _isValid = false;
        std::string uri = address + ":" + std::to_string(port);

        // The following code was inspired by the websocketpp library tutorial:
        // https://docs.websocketpp.org/md_tutorials_utility_client_utility_client.html
        
        //_client.clear_access_channels(websocketpp::log::alevel::all);
        //_client.clear_error_channels(websocketpp::log::elevel::all);

        _client.set_tls_init_handler([this](websocketpp::connection_hdl)
        {
            return websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::tlsv1);
        });

        _client.init_asio();
        _client.start_perpetual();

        _thread.reset(new websocketpp::lib::thread(
            &WebSocketPPClient::run, &_client));

        // Create connection
        websocketpp::lib::error_code ec;
        WebSocketPPClient::connection_ptr con;
        con = _client.get_connection(uri, ec);
        if (ec)
        {
            const std::string& err = ec.message();
            std::cout << 
                "WS initialization error: " << ec.message() << std::endl;
            return;
        }

        _pConnectionHandler = std::make_shared<ConnectionHandler>(this);

        con->set_open_handler(websocketpp::lib::bind(
            &ConnectionHandler::on_open,
            _pConnectionHandler,
            &_client,
            websocketpp::lib::placeholders::_1
        ));

        con->set_fail_handler(websocketpp::lib::bind(
            &ConnectionHandler::on_fail,
            _pConnectionHandler,
            &_client,
            websocketpp::lib::placeholders::_1
        ));

        con->set_message_handler(websocketpp::lib::bind(
            &ConnectionHandler::on_message,
            _pConnectionHandler,
            websocketpp::lib::placeholders::_1,
            websocketpp::lib::placeholders::_2
        ));

        _client.connect(con);

        _isValid = _pConnectionHandler->getConnected(); // Blocking call
        _handle = _pConnectionHandler->handle;
    }

    bool DefaultWebSocket::isValid() const
    {
        return _isValid;
    }

    void DefaultWebSocket::send(const std::string& message)
    {
        websocketpp::lib::error_code ec;
        _client.send(_handle, message, websocketpp::frame::opcode::text, ec);
    }

    void DefaultWebSocket::queueMessage(const std::string& message)
    {
        std::unique_lock<std::mutex> lock(_messageQueueMutex);
        _messageQueue.push(message);
        _messageQueueCondition.notify_one();
    }

    std::string DefaultWebSocket::recv()
    {
        std::unique_lock<std::mutex> lock(_messageQueueMutex);

        // If we already have a message in the queue, return that
        if (!_messageQueue.empty())
        {
            std::string message = _messageQueue.front();
            _messageQueue.pop();
            return message;
        }

        // Wait for a message to come in
        _messageQueueCondition.wait(lock, [this]()   
        {
            return _isValid && _messageQueue.empty();
        });
        
        // Return only if there's a message in the queue
        if (!_messageQueue.empty())
        {
            std::string message = _messageQueue.front();
            _messageQueue.pop();
            std::cout << "DefaultWebSocket::recv: " << message << std::endl;
            return message;
        }

        return "";
    }

    void DefaultWebSocket::close()
    {
        _isValid = false;

        websocketpp::lib::error_code ec;
        _client.close(_handle, websocketpp::close::status::going_away, "", ec);

        if (_pConnectionHandler) _pConnectionHandler.reset();
        if (_connection) _connection.reset();
        _handle.reset();
    }
};
