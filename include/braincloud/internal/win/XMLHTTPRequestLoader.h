//
//  XMLHTTPRequestLoader.h
//  GameClientLib
//

#ifndef _XMLHTTPREQUESTLOADER_H_
#define _XMLHTTPREQUESTLOADER_H_

#include "braincloud/internal/URLLoader.h"

#include <atomic>
#include <mutex>

#include <atlbase.h>
#include <comutil.h>
#include <msxml6.h>

namespace BrainCloud
{
    class XMLHTTPRequestLoader : public URLLoader
    {
    public:
        // Destructor
        virtual ~XMLHTTPRequestLoader();

        // Override the pure virtual methods.
        virtual void    close();
        virtual void    load(URLRequest const &);
        virtual void    load(URLRequest const * r) { if (r != NULL) load(*r); }
        virtual bool    isDone();

        virtual void setTimeout(int milliseconds) { _timeoutInterval = milliseconds; }

    protected:
        friend class URLLoader;

        XMLHTTPRequestLoader();

    private:
        static void* loadThread(void*);
        static void loadThreadXMLHTTPRequest(XMLHTTPRequestLoader*);

        static long _timeoutInterval;

        std::atomic<bool> _threadRunning;
        std::mutex _requestMutex;
        CComPtr<IXMLHTTPRequest> _request;
    };
}

#endif  // _XMLHTTPREQUESTLOADER_H_
