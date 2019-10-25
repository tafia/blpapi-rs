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

class SubscriptionCorrelationExample
{
    class GridWindow {
        std::string                  d_name;
        std::vector<std::string>    &d_securities;

    public :

        GridWindow(const char *name, std::vector<std::string> &securities)
            : d_name(name), d_securities(securities)
        {
        }

        void processSecurityUpdate(Message &msg, int row)
        {
            const std::string &topicname = d_securities[row];

            std::cout << d_name << ":" <<  row << ',' << topicname << std::endl;
        }
    };

    std::string              d_host;
    int                      d_port;
    int                      d_maxEvents;
    int                      d_eventCount;
    std::vector<std::string> d_securities;
    GridWindow               d_gridWindow;

    void printUsage()
    {
        std::cout << "Usage:" << std::endl
            << "    Retrieve realtime data " << std::endl
            << "        [-ip        <ipAddress  = localhost>" << std::endl
            << "        [-p         <tcpPort    = 8194>" << std::endl
            << "        [-me        <maxEvents  = MAX_INT>" << std::endl;
    }

    bool parseCommandLine(int argc, char **argv)
    {
        for (int i = 1; i < argc; ++i) {
            if (!std::strcmp(argv[i],"-ip") && i + 1 < argc)
                d_host = argv[++i];
            else if (!std::strcmp(argv[i],"-p") &&  i + 1 < argc)
                d_port = std::atoi(argv[++i]);
            else if (!std::strcmp(argv[i],"-me") && i + 1 < argc)
                d_maxEvents = std::atoi(argv[++i]);
            else {
                printUsage();
                return false;
            }
        }
        return true;
    }

public:

    SubscriptionCorrelationExample()
        : d_gridWindow("SecurityInfo", d_securities)
    {
        d_securities.push_back("IBM US Equity");
        d_securities.push_back("VOD LN Equity");
    }

    void run(int argc, char **argv)
    {
        d_host = "localhost";
        d_port = 8194;
        d_maxEvents = INT_MAX;
        d_eventCount = 0;

        if (!parseCommandLine(argc, argv)) return;

        SessionOptions sessionOptions;
        sessionOptions.setServerHost(d_host.c_str());
        sessionOptions.setServerPort(d_port);

        std::cout << "Connecting to "  <<  d_host  <<  ":"
            << d_port << std::endl;
        Session session(sessionOptions);
        if (!session.start()) {
            std::cerr <<"Failed to start session." << std::endl;
            return;
        }
        if (!session.openService("//blp/mktdata")) {
            std::cerr <<"Failed to open //blp/mktdata" << std::endl;
            return;
        }

        SubscriptionList subscriptions;
        for (size_t i = 0; i < d_securities.size(); ++i) {
            subscriptions.add(d_securities[i].c_str(),
                "LAST_PRICE",
                "",
                CorrelationId(i));
        }
        session.subscribe(subscriptions);

        while (true) {
            Event event = session.nextEvent();
            if (event.eventType() == Event::SUBSCRIPTION_DATA) {
                MessageIterator msgIter(event);
                while (msgIter.next()) {
                    Message msg = msgIter.message();
                    int row = (int)msg.correlationId().asInteger();
                    d_gridWindow.processSecurityUpdate(msg, row);
                }
                if (++d_eventCount >= d_maxEvents) break;
            }
        }
    }
};

int main(int argc, char **argv)
{
    std::cout << "SubscriptionCorrelationExample" << std::endl;
    SubscriptionCorrelationExample example;
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
