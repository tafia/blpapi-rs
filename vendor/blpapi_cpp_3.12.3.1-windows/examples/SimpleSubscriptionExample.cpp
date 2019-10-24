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
#include <blpapi_defs.h>
#include <blpapi_correlationid.h>
#include <blpapi_element.h>
#include <blpapi_event.h>
#include <blpapi_exception.h>
#include <blpapi_message.h>
#include <blpapi_session.h>
#include <blpapi_subscriptionlist.h>

#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>

using namespace BloombergLP;
using namespace blpapi;

class SimpleSubscriptionExample
{
    std::string         d_host;
    int                 d_port;
    int                 d_maxEvents;
    int                 d_eventCount;

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

        std::cout << "Connecting to " <<  d_host << ":" << d_port
                  << std::endl;
        Session session(sessionOptions);
        if (!session.start()) {
            std::cerr <<"Failed to start session." << std::endl;
            return;
        }
        if (!session.openService("//blp/mktdata")) {
            std::cerr <<"Failed to open //blp/mktdata" << std::endl;
            return;
        }

        const char *security1 = "IBM US Equity";
        const char *security2 = "/cusip/912810RE0@BGN";
            // this CUSIP identifies US Treasury Bill 'T 3 5/8 02/15/44 Govt'

        SubscriptionList subscriptions;
        subscriptions.add(
                security1,
                "LAST_PRICE,BID,ASK",
                "",
                CorrelationId((char *)security1));
        subscriptions.add(
                security2,
                "LAST_PRICE,BID,ASK,BID_YIELD,ASK_YIELD",
                "",
                CorrelationId((char *)security2));
        session.subscribe(subscriptions);

        while (true) {
            Event event = session.nextEvent();
            MessageIterator msgIter(event);
            while (msgIter.next()) {
                Message msg = msgIter.message();
                if (event.eventType() == Event::SUBSCRIPTION_STATUS ||
                    event.eventType() == Event::SUBSCRIPTION_DATA) {
                    const char *topic = (char *)msg.correlationId().asPointer();
                    std::cout << topic << " - ";
                }
                msg.print(std::cout) << std::endl;
            }
            if (event.eventType() == Event::SUBSCRIPTION_DATA) {
                if (++d_eventCount >= d_maxEvents) break;
            }
        }
    }
};

int main(int argc, char **argv)
{
    std::cout << "SimpleSubscriptionExample" << std::endl;
    SimpleSubscriptionExample example;
    try {
        example.run(argc, argv);
    }
    catch (Exception &e) {
        std::cerr << "Library Exception!!! " << e.description()
                      << std::endl;
    }

    // wait for enter key to exit application
    std::cout << "Press ENTER to quit" << std::endl;
    char dummy[2];
    std::cin.getline(dummy, 2);
    return 0;
}
