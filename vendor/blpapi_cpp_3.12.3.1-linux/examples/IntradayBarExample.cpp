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
#include <time.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>


using namespace BloombergLP;
using namespace blpapi;

namespace {
    const Name BAR_DATA("barData");
    const Name BAR_TICK_DATA("barTickData");
    const Name OPEN("open");
    const Name HIGH("high");
    const Name LOW("low");
    const Name CLOSE("close");
    const Name VOLUME("volume");
    const Name NUM_EVENTS("numEvents");
    const Name TIME("time");
    const Name RESPONSE_ERROR("responseError");
    const Name SESSION_TERMINATED("SessionTerminated");
    const Name CATEGORY("category");
    const Name MESSAGE("message");
};

class IntradayBarExample {

    std::string             d_host;
    int                     d_port;
    std::string             d_security;
    std::string             d_eventType;
    int                     d_barInterval;
    bool                    d_gapFillInitialBar;
    std::string             d_startDateTime;
    std::string             d_endDateTime;


    void printUsage()
    {
        std::cout <<"Usage:" << '\n'
            <<" Retrieve intraday bars" << '\n'
            <<"     [-s     <security   = IBM US Equity>" << '\n'
            <<"     [-e     <event      = TRADE>" << '\n'
            <<"     [-b     <barInterval= 60>" << '\n'
            <<"     [-sd    <startDateTime  = 2008-08-11T13:30:00>" << '\n'
            <<"     [-ed    <endDateTime    = 2008-08-12T13:30:00>" << '\n'
            <<"     [-g     <gapFillInitialBar = false>" << '\n'
            <<"     [-ip    <ipAddress = localhost>" << '\n'
            <<"     [-p     <tcpPort   = 8194>" << '\n'
            <<"1) All times are in GMT." << '\n'
            <<"2) Only one security can be specified." << '\n'
            <<"3) Only one event can be specified." << std::endl;
    }

    void printErrorInfo(const char *leadingStr, const Element &errorInfo)
    {
        std::cout
            << leadingStr
            << errorInfo.getElementAsString(CATEGORY)
            << " (" << errorInfo.getElementAsString(MESSAGE)
            << ")" << std::endl;
    }

    bool parseCommandLine(int argc, char **argv)
    {
        for (int i = 1; i < argc; ++i) {
            if (!std::strcmp(argv[i],"-s") && i + 1 < argc) {
                d_security = argv[++i];
            } else if (!std::strcmp(argv[i],"-ip") && i + 1 < argc) {
                d_host = argv[++i];
            } else if (!std::strcmp(argv[i],"-p") &&  i + 1 < argc) {
                d_port = std::atoi(argv[++i]);
            } else if (!std::strcmp(argv[i],"-e") &&  i + 1 < argc) {
                d_eventType = argv[++i];
            } else if (!std::strcmp(argv[i],"-b") &&  i + 1 < argc) {
                d_barInterval = std::atoi(argv[++i]);
            } else if (!std::strcmp(argv[i],"-g")) {
                d_gapFillInitialBar = true;
            } else if (!std::strcmp(argv[i],"-sd") && i + 1 < argc) {
                d_startDateTime = argv[++i];
            } else if (!std::strcmp(argv[i],"-ed") && i + 1 < argc) {
                d_endDateTime = argv[++i];
            } else {
                printUsage();
                return false;
            }
        }

        return true;
    }

    void processMessage(Message &msg) {
        Element data = msg.getElement(BAR_DATA).getElement(BAR_TICK_DATA);
        int numBars = data.numValues();
        std::cout <<"Response contains " << numBars << " bars" << std::endl;
        std::cout <<"Datetime\t\tOpen\t\tHigh\t\tLow\t\tClose" <<
            "\t\tNumEvents\tVolume" << std::endl;
        for (int i = 0; i < numBars; ++i) {
            Element bar = data.getValueAsElement(i);
            Datetime time = bar.getElementAsDatetime(TIME);
            assert(time.hasParts(DatetimeParts::DATE
                               | DatetimeParts::HOURS
                               | DatetimeParts::MINUTES));
            double open = bar.getElementAsFloat64(OPEN);
            double high = bar.getElementAsFloat64(HIGH);
            double low = bar.getElementAsFloat64(LOW);
            double close = bar.getElementAsFloat64(CLOSE);
            int numEvents = bar.getElementAsInt32(NUM_EVENTS);
            long long volume = bar.getElementAsInt64(VOLUME);

            std::cout.setf(std::ios::fixed, std::ios::floatfield);
            std::cout << time.month() << '/' << time.day() << '/' << time.year()
                << " " << time.hours() << ":" << time.minutes()
                <<  "\t\t" << std::showpoint
                << std::setprecision(3) << open << "\t\t"
                << high << "\t\t"
                << low <<  "\t\t"
                << close <<  "\t\t"
                << numEvents <<  "\t\t"
                << std::noshowpoint
                << volume << std::endl;
        }
    }

    void processResponseEvent(Event &event, Session &session) {
        MessageIterator msgIter(event);
        while (msgIter.next()) {
            Message msg = msgIter.message();
            if (msg.hasElement(RESPONSE_ERROR)) {
                printErrorInfo("REQUEST FAILED: ",
                    msg.getElement(RESPONSE_ERROR));
                continue;
            }
            processMessage(msg);
        }
    }

    void sendIntradayBarRequest(Session &session)
    {
        Service refDataService = session.getService("//blp/refdata");
        Request request = refDataService.createRequest("IntradayBarRequest");

        // only one security/eventType per request
        request.set("security", d_security.c_str());
        request.set("eventType", d_eventType.c_str());
        request.set("interval", d_barInterval);

        if (d_startDateTime.empty() || d_endDateTime.empty()) {
            Datetime startDateTime, endDateTime;
            if (0 == getTradingDateRange(&startDateTime, &endDateTime)) {
                request.set("startDateTime", startDateTime);
                request.set("endDateTime", endDateTime);
            }
        }
        else {
            if (!d_startDateTime.empty() && !d_endDateTime.empty()) {
                request.set("startDateTime", d_startDateTime.c_str());
                request.set("endDateTime", d_endDateTime.c_str());
            }
        }

        if (d_gapFillInitialBar) {
            request.set("gapFillInitialBar", d_gapFillInitialBar);
        }

        std::cout <<"Sending Request: " << request << std::endl;
        session.sendRequest(request);
    }

    void eventLoop(Session &session) {
        bool done = false;
        while (!done) {
            Event event = session.nextEvent();
            if (event.eventType() == Event::PARTIAL_RESPONSE) {
                std::cout <<"Processing Partial Response" << std::endl;
                processResponseEvent(event, session);
            }
            else if (event.eventType() == Event::RESPONSE) {
                std::cout <<"Processing Response" << std::endl;
                processResponseEvent(event, session);
                done = true;
            } else {
                MessageIterator msgIter(event);
                while (msgIter.next()) {
                    Message msg = msgIter.message();
                    if (event.eventType() == Event::SESSION_STATUS) {
                        if (msg.messageType() == SESSION_TERMINATED) {
                            done = true;
                        }
                    }
                }
            }
        }
    }

    int getTradingDateRange (Datetime *startDate_p, Datetime *endDate_p)
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
            if (tm_p->tm_wday == 0 || tm_p->tm_wday == 6 ) {// Sun/Sat
                continue ;
            }
            startDate_p->setDate(tm_p->tm_year + 1900,
                tm_p->tm_mon + 1,
                tm_p->tm_mday);
            startDate_p->setTime(13, 30, 0) ;

            //the next day is the end day
            currTime += 86400 ;
            tm_p = localtime(&currTime);
            if (tm_p == NULL) {
                break;
            }
            endDate_p->setDate(tm_p->tm_year + 1900,
                tm_p->tm_mon + 1,
                tm_p->tm_mday);
            endDate_p->setTime(13, 30, 0) ;

            return(0) ;
        }
        return (-1) ;
    }

public:

    IntradayBarExample() {
        d_host = "localhost";
        d_port = 8194;
        d_barInterval = 60;
        d_security = "IBM US Equity";
        d_eventType = "TRADE";
        d_gapFillInitialBar = false;
    }

    ~IntradayBarExample() {
    };

    void run(int argc, char **argv) {
        if (!parseCommandLine(argc, argv)) return;
        SessionOptions sessionOptions;
        sessionOptions.setServerHost(d_host.c_str());
        sessionOptions.setServerPort(d_port);

        std::cout <<"Connecting to " << d_host << ":" << d_port << std::endl;
        Session session(sessionOptions);
        if (!session.start()) {
            std::cerr << "Failed to start session." << std::endl;
            return;
        }
        if (!session.openService("//blp/refdata")) {
            std::cerr << "Failed to open //blp/refdata" << std::endl;
            return;
        }

        sendIntradayBarRequest(session);

        // wait for events from session.
        eventLoop(session);

        session.stop();
    }
};

int main(int argc, char **argv)
{
    std::cout << "IntradayBarExample" << std::endl;
    IntradayBarExample example;
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
