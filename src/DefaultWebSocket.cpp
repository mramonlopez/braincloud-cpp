#include "braincloud/internal/DefaultWebSocket.h"

#include <algorithm>
#include <iostream>
#include <cctype>

#define MAX_PAYLOAD 2048

namespace BrainCloud
{
    static struct lws_protocols protocols[] = {
        {
            "brainCloud",
            &DefaultWebSocket::libWebsocketsCallback,
            0,
            MAX_PAYLOAD,
            0, NULL, 0
        },
        { NULL, NULL, 0, 0 } /* terminator */
    };

    static const struct lws_extension exts[] = {
        {
            "permessage-deflate",
            lws_extension_callback_pm_deflate,
            "permessage-deflate; client_no_context_takeover"
        },
        {
            "deflate-frame",
            lws_extension_callback_pm_deflate,
            "deflate_frame"
        },
        { NULL, NULL, NULL /* terminator */ }
    };

    IWebSocket* IWebSocket::create(const std::string& address, int port, const std::map<std::string, std::string>& headers)
    {
        return new DefaultWebSocket(address, port, headers);
    }

    DefaultWebSocket::~DefaultWebSocket()
    {
        close();
    }

    DefaultWebSocket::DefaultWebSocket(const std::string& uri, int port, const std::map<std::string, std::string>& headers)
        : _isValid(false)
        , _pLwsContext(NULL)
        , _pLws(NULL)
        , _isConnecting(true)
        , _authHeaders(headers)
    {
        std::string uriCopy = uri;
        
        // Split address
        std::string protocol = uriCopy.substr(0, std::min<size_t>(uriCopy.size(), uriCopy.find_first_of(':')));
        size_t protocolSize = protocol.size() + 3;
        size_t argsPos = std::min<size_t>(uriCopy.size(), uriCopy.find_first_of('?', protocolSize));
        if (uriCopy[argsPos - 1] != '/')
        {
            uriCopy.insert(uriCopy.begin() + argsPos, '/');
        }
        size_t slashPos = std::min<size_t>(uriCopy.size(), uriCopy.find_first_of('/', protocolSize));
        std::string path = uriCopy.substr(slashPos);
        if (path.empty())
        {
            path = "/";
        }
        std::string addr = uriCopy.substr(protocolSize, slashPos - protocolSize);
        std::string host = addr;
        std::string origin = host + ":" + std::to_string(port);

        // Validate
        std::string protocolCaps = protocol;
        std::transform(protocol.begin(), protocol.end(), protocolCaps.begin(), [](unsigned char c){ return std::toupper(c); });
        if (protocolCaps != "WS" && protocolCaps != "WSS")
        {
            std::cout << "Invalid websocket protocol: " << protocol << std::endl;
            return;
        }
        bool useSSL = protocolCaps == "WSS";

        // Create context
        {
            struct lws_context_creation_info info;
            memset(&info, 0, sizeof info);

            info.port = CONTEXT_PORT_NO_LISTEN;
            info.protocols = protocols;
            info.gid = -1;
            info.uid = -1;
            info.extensions = exts;
            info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT |
                           LWS_SERVER_OPTION_VALIDATE_UTF8;

            _pLwsContext = lws_create_context(&info);
            if (!_pLwsContext)
            {
                std::cout << "Failed to create websocket context" << std::endl;
                return;
            }
        }

        // Start update thread
        _updateThread = std::thread([this]()
        {
            _mutex.lock();
            while (_isConnecting || _isValid)
            {
                _mutex.unlock();
                lws_callback_on_writable_all_protocol(_pLwsContext, &protocols[0]);
                lws_service(_pLwsContext, 100);
                _mutex.lock();
            }
            _mutex.unlock();
        });

        // Create connection
        {
            struct lws_client_connect_info connectInfo;
            memset(&connectInfo, 0, sizeof(connectInfo));

            connectInfo.context = _pLwsContext;
            connectInfo.address = addr.c_str();
            connectInfo.port = port;
            if (useSSL)
            {
                connectInfo.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK; //iUseSSL;
            }
            else
            {
                connectInfo.ssl_connection = 0;
            }
            connectInfo.path = path.c_str();
            connectInfo.host = host.c_str();
            connectInfo.origin = origin.c_str();
            connectInfo.protocol = protocols[0].name;
            connectInfo.ietf_version_or_minus_one = -1;
            connectInfo.userdata = this;

            _pLws = lws_client_connect_via_info(&connectInfo);
            if (!_pLws)
            {
                std::cout << "Failed to create websocket context" << std::endl;
                return;
            }
        }
    }

    int DefaultWebSocket::libWebsocketsCallback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len)
    {
        DefaultWebSocket *pWebSocket = (DefaultWebSocket *)lws_wsi_user(wsi);

        switch (reason)
        {
            case LWS_CALLBACK_CLOSED_CLIENT_HTTP:
            case LWS_CALLBACK_CLOSED:
            {
                if (!pWebSocket)
                {
                    return -1;
                }
                pWebSocket->onClose();
                break;
            }
            case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            {
                if (!pWebSocket)
                {
                    return -1;
                }
                pWebSocket->onError((const char*)in);
                break;
            }
            case LWS_CALLBACK_CLIENT_ESTABLISHED:
            {
                if (!pWebSocket)
                {
                    return -1;
                }
                pWebSocket->onConnect();
                break;
            }
            case LWS_CALLBACK_CLIENT_RECEIVE:
            {
                if (!pWebSocket)
                {
                    return -1;
                }
                pWebSocket->onRecv((const char*)in, (int)len);
                break;
            }
            case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
            {
                if (!pWebSocket)
                {
                    return -1;
                }

                unsigned char **p = (unsigned char **)in;
                unsigned char *end = (*p) + len;
                if (!pWebSocket->onProcessHeaders(p, end))
                {
                    return -1;
                }
                break;
            }
            case LWS_CALLBACK_CLIENT_WRITEABLE:
            {
                if (!pWebSocket)
                {
                    return -1;
                }
                pWebSocket->processSendQueue();
                break;
            }
            default:
                break;
        }

        return 0;
    }

    bool DefaultWebSocket::isValid()
    {
        std::unique_lock<std::mutex> lock(_mutex);

        if (_isConnecting)
        {
            _connectionCondition.wait(lock, [this]() { return !_isConnecting; });
        }

        return _isValid;
    }

    void DefaultWebSocket::send(const std::string& message)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _sendQueue.push_back(message);
    }

    void DefaultWebSocket::processSendQueue()
    {
        unsigned char buf[LWS_PRE + MAX_PAYLOAD];

        std::unique_lock<std::mutex> lock(_mutex);
        for (int i = 0; i < (int)_sendQueue.size(); ++i)
        {
            const std::string& message = _sendQueue[i];

            if (message.size() > MAX_PAYLOAD)
            {
                std::cout << "[error] Message size bigger than " << MAX_PAYLOAD << ". Message will not be sent." << std::endl;
                continue; // skip it
            }

            memcpy(buf + LWS_PRE, message.c_str(), message.size());
            lws_write(_pLws, buf + LWS_PRE, message.size(), LWS_WRITE_TEXT);
        }
        _sendQueue.clear();
    }

    std::string DefaultWebSocket::recv()
    {
        std::unique_lock<std::mutex> lock(_recvMutex);

        if (!_recvQueue.empty())
        {
            std::string message = _recvQueue.front();
            _recvQueue.erase(_recvQueue.begin());
            return message;
        }

        _recvCondition.wait(lock, [this]()
        {
            return !_isValid || !_recvQueue.empty();
        });

        if (!_recvQueue.empty())
        {
            std::string message = _recvQueue.front();
            _recvQueue.erase(_recvQueue.begin());
            return message;
        }

        return "";
    }

    void DefaultWebSocket::close()
    {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _isValid = false;
            _isConnecting = false;
            _connectionCondition.notify_all();
        }
        {
            std::unique_lock<std::mutex> lock(_recvMutex);
            _recvCondition.notify_all();
        }
        if (_updateThread.joinable())
        {
            _updateThread.join();
        }
    }

    void DefaultWebSocket::onClose()
    {
        std::cout << "WebSocket closed" << std::endl;

        std::unique_lock<std::mutex> lock(_mutex);
        _isValid = false;
        _isConnecting = false;
        _connectionCondition.notify_all();
    }

    void DefaultWebSocket::onError(const char* msg)
    {
        std::cout << "WebSocket error: " << msg << std::endl;

        std::unique_lock<std::mutex> lock(_mutex);
        _isValid = false;
        _isConnecting = false;
        _connectionCondition.notify_all();
    }

    void DefaultWebSocket::onConnect()
    {
        std::cout << "WebSocket Connected!" << std::endl;

        std::unique_lock<std::mutex> lock(_mutex);
        _isValid = true;
        _isConnecting = false;
        _connectionCondition.notify_all();
    }

    void DefaultWebSocket::onRecv(const char* buffer, int len)
    {
        std::cout << "WebSocket Connected!" << std::endl;

        std::unique_lock<std::mutex> lock(_recvMutex);
        std::string message(buffer, len);
        _recvQueue.push_back(message);
        _recvCondition.notify_all();
    }

    bool DefaultWebSocket::onProcessHeaders(unsigned char** ppBuffer, unsigned char* pEnd)
    {
        if (_authHeaders.empty())
        {
            return true;
        }

        std::string key;

        for (std::map<std::string, std::string>::iterator it = _authHeaders.begin(); it != _authHeaders.end(); ++it)
        {
            key = it->first;
            key += ":";
            const std::string& value = it->second;

            if (lws_add_http_header_by_name(_pLws, (const unsigned char *)key.c_str(), (const unsigned char *)value.c_str(), (int)value.size(), ppBuffer, pEnd))
            {
                return false;
            }
        }

        return true;
    }
};