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
#include <time.h>

using namespace BloombergLP;
using namespace blpapi;

class SimpleIntradayBarExample
{
    std::string         d_host;
    int                 d_port;

    void printUsage()
    {
        std::cout << "Usage:" << std::endl
            << "    Retrieve reference data " << std::endl
            << "        [-ip        <ipAddress = localhost>" << std::endl
            << "        [-p         <tcpPort   = 8194>" << std::endl;
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

    int getPreviousTradingDate (int *year_p, int *month_p, int *day_p)
    {
        struct tm *tm_p;
        time_t currTime = time(0);

        while (currTime > 0) {
            currTime -= 86400; // GO back one day
            tm_p = localtime(&currTime);
            if (tm_p == NULL) {
                break;
            }

            // if not sunday / saturday, assign values & return
            if (tm_p->tm_wday != 0 && tm_p->tm_wday != 6 ) {// not Sun/Sat
                *year_p = tm_p->tm_year + 1900;
                *month_p = tm_p->tm_mon + 1;
                *day_p = tm_p->tm_mday;
                return (0) ;
            }
        }
        return (-1) ;
    }

  public:

    void run(int argc, char **argv)
    {
        d_host = "localhost";
        d_port = 8194;
        if (!parseCommandLine(argc, argv)) return;

        SessionOptions sessionOptions;
        sessionOptions.setServerHost(d_host.c_str());
        sessionOptions.setServerPort(d_port);

        std::cout << "Connecting to " <<  d_host << ":" << d_port << std::endl;
        Session session(sessionOptions);
        if (!session.start()) {
            std::cerr <<"Failed to start session." << std::endl;
            return;
        }
        if (!session.openService("//blp/refdata")) {
            std::cerr <<"Failed to open //blp/refdata" << std::endl;
            return;
        }

        Service refDataService = session.getService("//blp/refdata");
        Request request = refDataService.createRequest("IntradayBarRequest");
        request.set("security", "IBM US Equity");
        request.set("eventType", "TRADE");
        request.set("interval", 60); // bar interval in minutes

        int tradedOnYear  = 0;
        int tradedOnMonth = 0;
        int tradedOnDay   = 0;
        if (0 != getPreviousTradingDate (&tradedOnYear,
            &tradedOnMonth,
            &tradedOnDay) ) {
                std::cerr << "unable to get previous trading date" << std::endl;
                return;
        }

        Datetime starttime;
        starttime.setDate(tradedOnYear, tradedOnMonth, tradedOnDay);
        starttime.setTime(13, 30, 0, 0);
        request.set("startDateTime", starttime );

        Datetime endtime;
        endtime.setDate(tradedOnYear, tradedOnMonth, tradedOnDay);
        endtime.setTime(21, 30, 0, 0);
        request.set("endDateTime", endtime);

        std::cout << "Sending Request: " << request << std::endl;
        session.sendRequest(request);

        while (true) {
            Event event = session.nextEvent();
            MessageIterator msgIter(event);
            while (msgIter.next()) {
                Message msg = msgIter.message();
                std::cout << msg.messageType();
                msg.print(std::cout) << std::endl;
            }
            if (event.eventType() == Event::RESPONSE) {
                break;
            }
        }
    }
};

int main(int argc, char **argv)
{
    std::cout << "SimpleIntradayBarExample" << std::endl;
    SimpleIntradayBarExample example;
    try {
        example.run(argc, argv);
    }
    catch (Exception &e) {
        std::cerr << "Library Exception!!! " << e.description() << std::endl;
    }
    // wait for enter key to exit application
    std::cout << "Press ENTER to quit" << std::endl;
    char dummy[2];
    std::cin.getline(dummy, 2);
    return 0;
}
