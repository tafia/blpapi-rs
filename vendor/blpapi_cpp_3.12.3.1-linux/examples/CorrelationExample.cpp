/* Copyright 2012. Bloomberg Finance L.P.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:  The above
 * copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#include <blpapi_session.h>
#include <blpapi_eventdispatcher.h>

#include <blpapi_event.h>
#include <blpapi_message.h>
#include <blpapi_element.h>
#include <blpapi_name.h>
#include <blpapi_request.h>
#include <blpapi_subscriptionlist.h>
#include <blpapi_defs.h>

#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <cassert>

using namespace BloombergLP;
using namespace blpapi;

/**
 * An example to demonstrate use of CorrelationID.
 */
class CorrelationExample {
    /**
    * A helper class to simulate a GUI window.
    */
    class Window {
        std::string  d_name;

    public:
        Window(const char * name): d_name(name) {
        }

        void displaySecurityInfo(Message &msg) {
            std::cout << this->d_name << ": ";
            msg.print(std::cout);
        }
    };

    std::string         d_host;
    int                 d_port;
    Window              d_secInfoWindow;
    CorrelationId       d_cid;

    void printUsage()
    {
        std::cout << "Usage:" << std::endl
            << "    Retrieve reference data " << std::endl
            << "        [-ip        <ipAddress  = localhost>" << std::endl
            << "        [-p         <tcpPort    = 8194>" << std::endl;
    }

    bool parseCommandLine(int argc, char **argv)
    {
        for (int i = 1; i < argc; ++i) {
            if (!std::strcmp(argv[i],"-ip") && i + 1 < argc) {
                d_host = argv[++i];
                continue;
            }
            if (!std::strcmp(argv[i],"-p") && i + 1 < argc) {
                d_port = std::atoi(argv[++i]);
                continue;
            }
            printUsage();
            return false;
        }
        return true;
    }

    bool startSession(Session &session){
        if (!session.start()) {
            std::cerr << "Failed to connect!" << std::endl;
            return false;
        }
        if (!session.openService("//blp/refdata")) {
            std::cerr << "Failed to open //blp/refdata" << std::endl;
            session.stop();
            return false;
        }

        return true;
    }

    public:

    CorrelationExample(): d_secInfoWindow("SecurityInfo"), d_cid(&d_secInfoWindow) {
        d_host = "localhost";
        d_port = 8194;
    }

    ~CorrelationExample() {
    }

    void run(int argc, char **argv) {

        if (!parseCommandLine(argc, argv)) return;

        SessionOptions sessionOptions;
        sessionOptions.setServerHost(d_host.c_str());
        sessionOptions.setServerPort(d_port);

        std::cout << "Connecting to " + d_host + ":" << d_port << std::endl;
        Session session(sessionOptions);
        if (!startSession(session)) return;

        Service refDataService = session.getService("//blp/refdata");
        Request request = refDataService.createRequest("ReferenceDataRequest");
        request.append("securities", "IBM US Equity");
        request.append("fields", "PX_LAST");
        request.append("fields", "DS002");

        session.sendRequest(request, d_cid);

        while (true) {
            Event event = session.nextEvent();
            MessageIterator msgIter(event);
            while (msgIter.next()) {
                Message msg = msgIter.message();
                if (event.eventType() == Event::RESPONSE ||
                    event.eventType() == Event::PARTIAL_RESPONSE) {
                    ((Window *)msg.correlationId().asPointer())->
                            displaySecurityInfo(msg);
                }
            }
            if (event.eventType() == Event::RESPONSE) {
                // received final response
                break;
            }
        }
    }
};

int
main(int argc, char **argv)
{
    std::cout << "CorrelationExample" << std::endl;
    CorrelationExample example;
    try {
        example.run(argc, argv);
    } catch (Exception &e) {
        std::cerr << "Library Exception!!! " << e.description() << std::endl;
    }
    // wait for enter key to exit application
    std::cout << "Press ENTER to quit" << std::endl;
    char dummy[2];
    std::cin.getline(dummy, 2);
    return 0;
}
