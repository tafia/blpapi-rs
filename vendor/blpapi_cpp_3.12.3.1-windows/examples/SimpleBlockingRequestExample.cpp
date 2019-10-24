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
#include <blpapi_exception.h>

#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <string.h>

using namespace BloombergLP;
using namespace blpapi;
namespace {
    const Name LAST_PRICE("LAST_PRICE");
}

class MyEventHandler :public EventHandler {
    // Process events using callback

    public:
    bool processEvent(const Event &event, Session *session) {
        try {
            if (event.eventType() == Event::SUBSCRIPTION_DATA) {
                MessageIterator msgIter(event);
                while (msgIter.next()) {
                    Message msg = msgIter.message();
                    if (msg.hasElement(LAST_PRICE)) {
                        Element field = msg.getElement(LAST_PRICE);
                        std::cout << field.name() << " = "
                            << field.getValueAsString() << std::endl;
                    }
                }
            }
            return true;
        } catch (Exception &e) {
            std::cerr << "Library Exception!!! " << e.description()
                << std::endl;
        } catch (...) {
            std::cerr << "Unknown Exception!!!" << std::endl;
        }
        return false;
    }
};

class SimpleBlockingRequestExample {

    CorrelationId d_cid;
    EventQueue d_eventQueue;
    std::string         d_host;
    int                 d_port;

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
            if (!std::strcmp(argv[i],"-p") &&  i + 1 < argc) {
                d_port = std::atoi(argv[++i]);
                continue;
            }
            printUsage();
            return false;
        }
        return true;
    }

public:
    SimpleBlockingRequestExample(): d_cid((int)1) {
    }

    void run(int argc, char **argv) {
        d_host = "localhost";
        d_port = 8194;
        if (!parseCommandLine(argc, argv)) return;

        SessionOptions sessionOptions;
        sessionOptions.setServerHost(d_host.c_str());
        sessionOptions.setServerPort(d_port);

        std::cout << "Connecting to " <<  d_host << ":" << d_port << std::endl;
        Session session(sessionOptions, new MyEventHandler());
        if (!session.start()) {
            std::cerr << "Failed to start session." << std::endl;
            return;
        }
        if (!session.openService("//blp/mktdata")) {
            std::cerr << "Failed to open //blp/mktdata" << std::endl;
            return;
        }
        if (!session.openService("//blp/refdata")) {
            std::cerr <<"Failed to open //blp/refdata" << std::endl;
            return;
        }

        std::cout << "Subscribing to IBM US Equity" << std::endl;
        SubscriptionList subscriptions;
        subscriptions.add("IBM US Equity", "LAST_PRICE", "", d_cid);
        session.subscribe(subscriptions);

        std::cout << "Requesting reference data IBM US Equity" << std::endl;
        Service refDataService = session.getService("//blp/refdata");
        Request request = refDataService.createRequest("ReferenceDataRequest");
        request.append("securities", "IBM US Equity");
        request.append("fields", "DS002");

        CorrelationId cid(this);
        session.sendRequest(request, cid, &d_eventQueue);
        while (true) {
            Event event = d_eventQueue.nextEvent();
            MessageIterator msgIter(event);
            while (msgIter.next()) {
                Message msg = msgIter.message();
                msg.print(std::cout);
            }
            if (event.eventType() == Event::RESPONSE) {
                break;
            }
        }
        // wait for enter key to exit application
        std::cout << "Press ENTER to quit" << std::endl;
        char dummy[2];
        std::cin.getline(dummy, 2);
    }

};

int main(int argc, char **argv) {
    std::cout << "SimpleBlockingRequestExample" << std::endl;
    SimpleBlockingRequestExample example;
    try {
        example.run(argc, argv);
    } catch (Exception &e) {
        std::cerr << "Library Exception!!! " << e.description() << std::endl;
    }

    return 0;
}
