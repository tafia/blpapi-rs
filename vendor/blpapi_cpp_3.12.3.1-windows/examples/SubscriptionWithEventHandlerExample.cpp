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
#include "BlpThreadUtil.h"

#include <blpapi_correlationid.h>
#include <blpapi_element.h>
#include <blpapi_event.h>
#include <blpapi_message.h>
#include <blpapi_name.h>
#include <blpapi_session.h>
#include <blpapi_subscriptionlist.h>

#include <cassert>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>

using namespace BloombergLP;
using namespace blpapi;

namespace {

Name SLOW_CONSUMER_WARNING("SlowConsumerWarning");
Name SLOW_CONSUMER_WARNING_CLEARED("SlowConsumerWarningCleared");
Name DATA_LOSS("DataLoss");
Name SUBSCRIPTION_TERMINATED("SubscriptionTerminated");
Name SOURCE("source");

inline
std::string getTopic(const CorrelationId& cid)
{
    // Return the topic used to create the specified correlation 'cid'.
    // The behaviour is undefined unless the specified cid was
    return *(reinterpret_cast<std::string*>(cid.asPointer()));
}

std::string getTopicsString(const SubscriptionList& list)
{
    std::ostringstream out;
    int count = 0;
    for (size_t i = 0; i < list.size(); ++i) {
        if (count++ != 0) {
            out << ", ";
        }
        out << getTopic(list.correlationIdAt(i));
    }
    return out.str();
}

const char * getSubscriptionTopicString(const SubscriptionList& list,
                                       const CorrelationId&    cid)
{
    for (size_t i = 0; i < list.size(); ++i) {
        if (list.correlationIdAt(i) == cid) {
            return list.topicStringAt(i);
        }
    }
    return 0;
}

size_t getTimeStamp(char *buffer, size_t bufSize)
{
    const char *format = "%Y/%m/%d %X";

    time_t now = time(0);
    tm _timeInfo;
#ifdef _WIN32
    tm *timeInfo = &_timeInfo;
    localtime_s(&_timeInfo, &now);
#else
    tm *timeInfo = localtime_r(&now, &_timeInfo);
#endif
    return strftime(buffer, bufSize, format, timeInfo);
}

}

class ConsoleOut
{
  private:
    std::ostringstream  d_buffer;
    Mutex              *d_consoleLock;
    std::ostream&       d_stream;

    // NOT IMPLEMENTED
    ConsoleOut(const ConsoleOut&);
    ConsoleOut& operator=(const ConsoleOut&);

  public:
    explicit ConsoleOut(Mutex         *consoleLock,
                        std::ostream&  stream = std::cout)
    : d_consoleLock(consoleLock)
    , d_stream(stream)
    {}

    ~ConsoleOut() {
        MutexGuard guard(d_consoleLock);
        d_stream << d_buffer.str();
        d_stream.flush();
    }

    template <typename T>
    std::ostream& operator<<(const T& value) {
        return d_buffer << value;
    }

    std::ostream& stream() {
        return d_buffer;
    }
};

struct SessionContext
{
    Mutex            d_consoleLock;
    Mutex            d_mutex;
    bool             d_isStopped;
    SubscriptionList d_subscriptions;

    SessionContext()
    : d_isStopped(false)
    {
    }
};

class SubscriptionEventHandler: public EventHandler
{
    bool                     d_isSlow;
    SubscriptionList         d_pendingSubscriptions;
    std::set<CorrelationId>  d_pendingUnsubscribe;
    SessionContext          *d_context_p;
    Mutex                   *d_consoleLock_p;

    bool processSubscriptionStatus(const Event &event, Session *session)
    {
        char timeBuffer[64];
        getTimeStamp(timeBuffer, sizeof(timeBuffer));

        SubscriptionList subscriptionList;
        ConsoleOut(d_consoleLock_p)
            << "\nProcessing SUBSCRIPTION_STATUS"
            << std::endl;

        MessageIterator msgIter(event);
        while (msgIter.next()) {
            Message msg = msgIter.message();
            CorrelationId cid = msg.correlationId();

            std::string topic = getTopic(cid);
            {
                ConsoleOut out(d_consoleLock_p);
                out << timeBuffer << ": " << topic
                    << std::endl;
                msg.print(out.stream(), 0, 4);
            }

            if (msg.messageType() == SUBSCRIPTION_TERMINATED
                && d_pendingUnsubscribe.erase(cid)) {
                // If this message was due to a previous unsubscribe
                const char *topicString = getSubscriptionTopicString(
                    d_context_p->d_subscriptions,
                    cid);
                assert(topicString);
                if (d_isSlow) {
                    ConsoleOut(d_consoleLock_p)
                        << "Deferring subscription for topic = " << topic
                        << " because session is slow." << std::endl;
                    d_pendingSubscriptions.add(topicString, cid);
                }
                else {
                    subscriptionList.add(topicString, cid);
                }
            }
        }

        if (0 != subscriptionList.size() && !d_context_p->d_isStopped) {
            session->subscribe(subscriptionList);
        }

        return true;
    }

    bool processSubscriptionDataEvent(const Event &event)
    {
        char timeBuffer[64];
        getTimeStamp(timeBuffer, sizeof(timeBuffer));

        ConsoleOut(d_consoleLock_p)
            << "\nProcessing SUBSCRIPTION_DATA" << std::endl;
        MessageIterator msgIter(event);
        while (msgIter.next()) {
            Message msg = msgIter.message();
            {
                ConsoleOut out(d_consoleLock_p);
                out << timeBuffer << ": " << getTopic(msg.correlationId())
                    << std::endl;
                msg.print(out.stream(), 0, 4);
            }
        }
        return true;
    }

    bool processAdminEvent(const Event &event, Session *session)
    {
        char timeBuffer[64];
        getTimeStamp(timeBuffer, sizeof(timeBuffer));

        ConsoleOut(d_consoleLock_p)
            << "\nProcessing ADMIN" << std::endl;
        std::vector<CorrelationId> cidsToCancel;
        bool previouslySlow = d_isSlow;
        MessageIterator msgIter(event);

        while (msgIter.next()) {
            Message msg = msgIter.message();
            // An admin event can have more than one messages.
            if (msg.messageType() == DATA_LOSS) {
                const CorrelationId& cid = msg.correlationId();
                {
                    ConsoleOut out(d_consoleLock_p);
                    out << timeBuffer << ": " << getTopic(msg.correlationId())
                        << std::endl;
                    msg.print(out.stream(), 0, 4);
                }

                if (msg.hasElement(SOURCE)) {
                    std::string sourceStr = msg.getElementAsString(SOURCE);
                    if (0 == sourceStr.compare("InProc")
                        && d_pendingUnsubscribe.find(cid) ==
                                                  d_pendingUnsubscribe.end()) {
                        // DataLoss was generated "InProc".
                        // This can only happen if applications are processing
                        // events slowly and hence are not able to keep-up with
                        // the incoming events.
                        cidsToCancel.push_back(cid);
                        d_pendingUnsubscribe.insert(cid);
                    }
                }
            }
            else {
                ConsoleOut(d_consoleLock_p)
                    << timeBuffer << ": "
                    << msg.messageType().string() << std::endl;
                if (msg.messageType() == SLOW_CONSUMER_WARNING) {
                    d_isSlow = true;
                }
                else if (msg.messageType() == SLOW_CONSUMER_WARNING_CLEARED) {
                    d_isSlow = false;
                }
            }
        }
        if (!d_context_p->d_isStopped) {
            if (0 != cidsToCancel.size()) {
                session->cancel(cidsToCancel);
            }
            else if (previouslySlow
                     && !d_isSlow
                     && d_pendingSubscriptions.size() > 0) {
                // Session was slow but is no longer slow. subscribe to any
                // topics for which we have previously received
                // SUBSCRIPTION_TERMINATED
                ConsoleOut(d_consoleLock_p)
                    << "Subscribing to topics - "
                    << getTopicsString(d_pendingSubscriptions)
                    << std::endl;
                session->subscribe(d_pendingSubscriptions);
                d_pendingSubscriptions.clear();
            }
        }

        return true;
    }

    bool processMiscEvents(const Event &event)
    {
        char timeBuffer[64];
        getTimeStamp(timeBuffer, sizeof(timeBuffer));

        MessageIterator msgIter(event);
        while (msgIter.next()) {
            Message msg = msgIter.message();
            ConsoleOut(d_consoleLock_p)
                << timeBuffer << ": "
                << msg.messageType().string() << std::endl;
        }
        return true;
    }

public:
    SubscriptionEventHandler(SessionContext *context)
    : d_isSlow(false)
    , d_context_p(context)
    , d_consoleLock_p(&context->d_consoleLock)
    {
    }

    bool processEvent(const Event &event, Session *session)
    {
        try {
            switch (event.eventType()) {
              case Event::SUBSCRIPTION_DATA: {
                return processSubscriptionDataEvent(event);
              } break;
              case Event::SUBSCRIPTION_STATUS: {
                MutexGuard guard(&d_context_p->d_mutex);
                return processSubscriptionStatus(event, session);
              } break;
              case Event::ADMIN: {
                MutexGuard guard(&d_context_p->d_mutex);
                return processAdminEvent(event, session);
              } break;
              default: {
                return processMiscEvents(event);
              }  break;
            }
        } catch (Exception &e) {
            ConsoleOut(d_consoleLock_p)
                << "Library Exception !!!"
                << e.description() << std::endl;
        }
        return false;
    }
};

class SubscriptionWithEventHandlerExample
{

    const std::string         d_service;
    SessionOptions            d_sessionOptions;
    Session                  *d_session;
    SubscriptionEventHandler *d_eventHandler;
    std::vector<std::string>  d_topics;
    std::vector<std::string>  d_fields;
    std::vector<std::string>  d_options;
    SessionContext            d_context;

    bool createSession() {
        ConsoleOut(&d_context.d_consoleLock)
            << "Connecting to " << d_sessionOptions.serverHost()
            << ":" << d_sessionOptions.serverPort() << std::endl;

        d_eventHandler = new SubscriptionEventHandler(&d_context);
        d_session = new Session(d_sessionOptions, d_eventHandler);

        if (!d_session->start()) {
            ConsoleOut(&d_context.d_consoleLock)
                << "Failed to start session." << std::endl;
            return false;
        }

        ConsoleOut(&d_context.d_consoleLock)
            << "Connected successfully" << std::endl;

        if (!d_session->openService(d_service.c_str())) {
            ConsoleOut(&d_context.d_consoleLock)
                << "Failed to open mktdata service" << std::endl;
            d_session->stop();
            return false;
        }

        ConsoleOut(&d_context.d_consoleLock) << "Subscribing..." << std::endl;
        d_session->subscribe(d_context.d_subscriptions);
        return true;
    }


    bool parseCommandLine(int argc, char **argv)
    {
        for (int i = 1; i < argc; ++i) {
            if (!std::strcmp(argv[i],"-t") && i + 1 < argc)
                d_topics.push_back(argv[++i]);
            else if (!std::strcmp(argv[i],"-f") && i + 1 < argc)
                d_fields.push_back(argv[++i]);
            else if (!std::strcmp(argv[i],"-o") && i + 1 < argc)
                d_options.push_back(argv[++i]);
            else if (!std::strcmp(argv[i],"-ip") && i + 1 < argc)
                d_sessionOptions.setServerHost(argv[++i]);
            else if (!std::strcmp(argv[i],"-p") &&  i + 1 < argc)
                d_sessionOptions.setServerPort(std::atoi(argv[++i]));
            else if (!std::strcmp(argv[i],"-qsize") &&  i + 1 < argc)
                d_sessionOptions.setMaxEventQueueSize(std::atoi(argv[++i]));
            else {
                printUsage();
                return false;
            }
        }

        if (d_fields.size() == 0) {
            d_fields.push_back("LAST_PRICE");
        }

        if (d_topics.size() == 0) {
            d_topics.push_back("IBM US Equity");
        }

        for (size_t i = 0; i < d_topics.size(); ++i) {
            std::string topic(d_service);
            if (*d_topics[i].c_str() != '/')
                topic += '/';
            topic += d_topics[i];
            d_context.d_subscriptions.add(
                topic.c_str(),
                d_fields,
                d_options,
                CorrelationId(&d_topics[i]));
        }

        return true;
    }

    void printUsage()
    {
        const char *usage =
            "Usage:\n"
            "    Retrieve realtime data\n"
            "        [-t     <topic      = IBM US Equity>\n"
            "        [-f     <field      = LAST_PRICE>\n"
            "        [-o     <subscriptionOptions>\n"
            "        [-ip    <ipAddress  = localhost>\n"
            "        [-p     <tcpPort    = 8194>\n"
            "        [-qsize <queuesize  = 10000>\n";
        ConsoleOut(&d_context.d_consoleLock) << usage << std::endl;
    }

public:

    SubscriptionWithEventHandlerExample()
    : d_service("//blp/mktdata")
    , d_session(0)
    , d_eventHandler(0)
    {
        d_sessionOptions.setServerHost("localhost");
        d_sessionOptions.setServerPort(8194);
        d_sessionOptions.setMaxEventQueueSize(10000);
    }

    ~SubscriptionWithEventHandlerExample()
    {
        if (d_session) delete d_session;
        if (d_eventHandler) delete d_eventHandler;
    }

    void run(int argc, char **argv)
    {
        if (!parseCommandLine(argc, argv)) return;
        if (!createSession()) return;

        // wait for enter key to exit application
        ConsoleOut(&d_context.d_consoleLock)
            << "\nPress ENTER to quit" << std::endl;
        char dummy[2];
        std::cin.getline(dummy,2);
        {
            MutexGuard guard(&d_context.d_mutex);
            d_context.d_isStopped = true;
        }
        d_session->stop();
        ConsoleOut(&d_context.d_consoleLock) << "\nExiting..." << std::endl;
    }
};

int main(int argc, char **argv)
{
    std::cout << "SubscriptionWithEventHandlerExample" << std::endl;
    SubscriptionWithEventHandlerExample example;
    try {
        example.run(argc, argv);
    } catch (Exception &e) {
        std::cout << "Library Exception!!!" << e.description() << std::endl;
    }
    // wait for enter key to exit application
    std::cout << "Press ENTER to quit" << std::endl;
    char dummy[2];
    std::cin.getline(dummy,2);
    return 0;
}
