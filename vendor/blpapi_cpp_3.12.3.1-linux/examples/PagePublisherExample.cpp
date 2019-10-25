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

#include <blpapi_element.h>
#include <blpapi_event.h>
#include <blpapi_eventformatter.h>
#include <blpapi_message.h>
#include <blpapi_name.h>
#include <blpapi_providersession.h>
#include <blpapi_topiclist.h>
#include <blpapi_topic.h>
#include <blpapi_identity.h>

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>

#include "BlpThreadUtil.h"

using namespace BloombergLP;
using namespace blpapi;

namespace {

Name AUTHORIZATION_SUCCESS("AuthorizationSuccess");
Name RESOLUTION_SUCCESS("ResolutionSuccess");
Name PERMISSION_REQUEST("PermissionRequest");
Name SESSION_TERMINATED("SessionTerminated");
Name TOKEN("token");
Name TOKEN_SUCCESS("TokenGenerationSuccess");
Name TOKEN_FAILURE("TokenGenerationFailure");
Name TOPICS("topics");
Name TOPIC_CREATED("TopicCreated");
Name TOPIC_RECAP("TopicRecap");
Name TOPIC_SUBSCRIBED("TopicSubscribed");
Name TOPIC_UNSUBSCRIBED("TopicUnsubscribed");

const std::string AUTH_USER       = "AuthenticationType=OS_LOGON";
const std::string AUTH_APP_PREFIX = "AuthenticationMode=APPLICATION_ONLY;ApplicationAuthenticationType=APPNAME_AND_KEY;ApplicationName=";
const std::string AUTH_USER_APP_PREFIX = "AuthenticationMode=USER_AND_APPLICATION;AuthenticationType=OS_LOGON;ApplicationAuthenticationType=APPNAME_AND_KEY;ApplicationName=";
const std::string AUTH_DIR_PREFIX = "AuthenticationType=DIRECTORY_SERVICE;DirSvcPropertyName=";
const char* AUTH_OPTION_NONE      = "none";
const char* AUTH_OPTION_USER      = "user";
const char* AUTH_OPTION_APP       = "app=";
const char* AUTH_OPTION_USER_APP  = "userapp=";
const char* AUTH_OPTION_DIR       = "dir=";

bool g_running = true;

Mutex g_lock;

enum AuthorizationStatus {
    WAITING,
    AUTHORIZED,
    FAILED
};

std::map<CorrelationId, AuthorizationStatus> g_authorizationStatus;

class MyStream {
    const std::string d_id;
    volatile bool d_isInitialPaintSent;

    Topic d_topic;
    bool  d_isSubscribed;

  public:
    MyStream()
        : d_id(""),
          d_isInitialPaintSent(false)
    {}

    MyStream(std::string const& id)
        : d_id(id),
          d_isInitialPaintSent(false)
    {}

    std::string const& getId()
    {
        return d_id;
    }

    bool isInitialPaintSent() const
    {
        return d_isInitialPaintSent;
    }

    void setIsInitialPaintSent(bool value)
    {
        d_isInitialPaintSent = value;
    }
    void setTopic(Topic topic) {
        d_topic = topic;
    }

    void setSubscribedState(bool isSubscribed) {
        d_isSubscribed = isSubscribed;
    }

    Topic& topic() {
        return d_topic;
    }

    bool isAvailable() {
        return d_topic.isValid() && d_isSubscribed;
    }

};

typedef std::map<std::string, MyStream*> MyStreams;

MyStreams g_streams;
int       g_availableTopicCount;
Mutex     g_mutex;

void printMessages(const Event& event)
{
    MessageIterator iter(event);
    while (iter.next()) {
        Message msg = iter.message();
        msg.print(std::cout);
    }
}

} // namespace {

class MyEventHandler : public ProviderEventHandler
{
    const std::string d_serviceName;

public:
    MyEventHandler(const std::string& serviceName)
    : d_serviceName(serviceName)
    {}

    bool processEvent(const Event& event, ProviderSession* session);
};

bool MyEventHandler::processEvent(const Event& event, ProviderSession* session)
{
    if (event.eventType() == Event::SESSION_STATUS) {
        printMessages(event);
        MessageIterator iter(event);
        while (iter.next()) {
            Message msg = iter.message();
            if (msg.messageType() == SESSION_TERMINATED) {
                g_running = false;
            }
        }
    }
    else if (event.eventType() == Event::TOPIC_STATUS) {
        TopicList topicList;
        MessageIterator iter(event);
        while (iter.next()) {
            Message msg = iter.message();
            std::cout << msg << std::endl;
            if (msg.messageType() == TOPIC_SUBSCRIBED) {
                std::string topicStr = msg.getElementAsString("topic");
                MutexGuard guard(&g_mutex);
                MyStreams::iterator it = g_streams.find(topicStr);
                if (it == g_streams.end()) {
                    // TopicList knows how to add an entry based on a
                    // TOPIC_SUBSCRIBED message.
                    topicList.add(msg);
                    it = (g_streams.insert(MyStreams::value_type(
                                     topicStr,
                                     new MyStream(topicStr)))).first;
                }
                it->second->setSubscribedState(true);
                if (it->second->isAvailable()) {
                    ++g_availableTopicCount;
                }
            }
            else if (msg.messageType() == TOPIC_UNSUBSCRIBED) {
                std::string topicStr = msg.getElementAsString("topic");
                MutexGuard guard(&g_mutex);
                MyStreams::iterator it = g_streams.find(topicStr);
                if (it == g_streams.end()) {
                    // we should never be coming here. TOPIC_UNSUBSCRIBED can
                    // not come before a TOPIC_SUBSCRIBED or TOPIC_CREATED
                    continue;
                }
                if (it->second->isAvailable()) {
                    --g_availableTopicCount;
                }
                it->second->setSubscribedState(false);
            }
            else if (msg.messageType() == TOPIC_CREATED) {
                std::string topicStr = msg.getElementAsString("topic");
                MutexGuard guard(&g_mutex);
                MyStreams::iterator it = g_streams.find(topicStr);
                if (it == g_streams.end()) {
                    it = (g_streams.insert(MyStreams::value_type(
                                     topicStr,
                                     new MyStream(topicStr)))).first;
                }
                try {
                    Topic topic = session->getTopic(msg);
                    it->second->setTopic(topic);
                } catch (blpapi::Exception &e) {
                    std::cerr
                        << "Exception in Session::getTopic(): "
                        << e.description()
                        << std::endl;
                    continue;
                }
                if (it->second->isAvailable()) {
                    ++g_availableTopicCount;
                }
            }
            else if (msg.messageType() == TOPIC_RECAP) {
                // Here we send a recap in response to a Recap request.
                try {
                    std::string topicStr = msg.getElementAsString("topic");
                    MyStreams::iterator it = g_streams.find(topicStr);
                    MutexGuard guard(&g_mutex);
                    if (it == g_streams.end() || !it->second->isAvailable()) {
                        continue;
                    }
                    Topic topic = session->getTopic(msg);
                    Service service = topic.service();
                    CorrelationId recapCid = msg.correlationId();

                    Event recapEvent = service.createPublishEvent();
                    EventFormatter eventFormatter(recapEvent);
                    eventFormatter.appendRecapMessage(topic, &recapCid);
                    eventFormatter.setElement("numRows", 25);
                    eventFormatter.setElement("numCols", 80);
                    eventFormatter.pushElement("rowUpdate");
                    for (int i = 1; i < 6; ++i) {
                        eventFormatter.appendElement();
                        eventFormatter.setElement("rowNum", i);
                        eventFormatter.pushElement("spanUpdate");
                        eventFormatter.appendElement();
                        eventFormatter.setElement("startCol", 1);
                        eventFormatter.setElement("length", 10);
                        eventFormatter.setElement("text", "RECAP");
                        eventFormatter.popElement();
                        eventFormatter.popElement();
                        eventFormatter.popElement();
                    }
                    eventFormatter.popElement();
                    guard.release()->unlock();
                    session->publish(recapEvent);
                } catch (blpapi::Exception &e) {
                    std::cerr
                        << "Exception in Session::getTopic(): "
                        << e.description()
                        << std::endl;
                    continue;
                }
            }
        }
        if (topicList.size()) {
            // createTopicsAsync will result in RESOLUTION_STATUS,
            // TOPIC_CREATED events.
            session->createTopicsAsync(topicList);
        }
    }
    else if (event.eventType() == Event::RESOLUTION_STATUS) {
        printMessages(event);
    }
    else if (event.eventType() == Event::REQUEST) {
        Service service = session->getService(d_serviceName.c_str());
        MessageIterator iter(event);
        while (iter.next()) {
            Message msg = iter.message();
            msg.print(std::cout);
            if (msg.messageType() == PERMISSION_REQUEST) {
                // This example always sends a 'PERMISSIONED' response.
                // See 'MktdataPublisherExample' on how to parse a Permission
                // request and send an appropriate 'PermissionResponse'.
                Event response = service.createResponseEvent(
                                                          msg.correlationId());
                int permission = 0; // ALLOWED: 0, DENIED: 1
                EventFormatter ef(response);
                ef.appendResponse("PermissionResponse");
                ef.pushElement("topicPermissions");
                // For each of the topics in the request, add an entry
                // to the response
                Element topicsElement = msg.getElement(TOPICS);
                for (size_t i = 0; i < topicsElement.numValues(); ++i) {
                    ef.appendElement();
                    ef.setElement("topic", topicsElement.getValueAsString(i));
                    ef.setElement("result", permission); //PERMISSIONED
                    ef.popElement();
                }
                ef.popElement();
                session->sendResponse(response);
            }
        }
    }
    else {
        MessageIterator iter(event);
        while (iter.next()) {
            Message msg = iter.message();
            MutexGuard guard(&g_lock);
            if (g_authorizationStatus.find(msg.correlationId()) !=
                    g_authorizationStatus.end()) {
                if (msg.messageType() == AUTHORIZATION_SUCCESS) {
                    g_authorizationStatus[msg.correlationId()] = AUTHORIZED;
                }
                else {
                    g_authorizationStatus[msg.correlationId()] = FAILED;
                }
            }
            msg.print(std::cout);
        }
    }

    return true;
}

class PagePublisherExample
{
    std::vector<std::string> d_hosts;
    int                      d_port;
    int                      d_priority;
    std::string              d_service;
    std::string              d_groupId;
    std::string              d_authOptions;

    void printUsage()
    {
        std::cout
            << "Publish on a topic. " << std::endl
            << "Usage:" << std::endl
            << "\t[-ip   <ipAddress>]  \tserver name or IP (default: localhost)" << std::endl
            << "\t[-p    <tcpPort>]    \tserver port (default: 8194)" << std::endl
            << "\t[-s    <service>]    \tservice name (default: //viper/page)" << std::endl
            << "\t[-g    <groupId>]    \tpublisher groupId (defaults to unique value)" << std::endl
            << "\t[-pri  <priority>]   \tset publisher priority level (default: 10)" << std::endl
            << "\t[-auth <option>]     \tauthentication option: user|none|app=<app>|userapp=<app>|dir=<property> (default: user)" << std::endl;
    }

    bool parseCommandLine(int argc, char **argv)
    {
        for (int i = 1; i < argc; ++i) {
            if (!std::strcmp(argv[i], "-ip") && i + 1 < argc)
                d_hosts.push_back(argv[++i]);
            else if (!std::strcmp(argv[i], "-p") &&  i + 1 < argc)
                d_port = std::atoi(argv[++i]);
            else if (!std::strcmp(argv[i], "-s") &&  i + 1 < argc)
                d_service = argv[++i];
            else if (!std::strcmp(argv[i],"-g") && i + 1 < argc)
                d_groupId = argv[++i];
            else if (!std::strcmp(argv[i],"-pri") && i + 1 < argc)
                d_priority = std::atoi(argv[++i]);
            else if (!std::strcmp(argv[i], "-auth") && i + 1 < argc) {
                ++ i;
                if (!std::strcmp(argv[i], AUTH_OPTION_NONE)) {
                    d_authOptions.clear();
                }
                else if (strncmp(argv[i], AUTH_OPTION_APP, strlen(AUTH_OPTION_APP)) == 0) {
                    d_authOptions.clear();
                    d_authOptions.append(AUTH_APP_PREFIX);
                    d_authOptions.append(argv[i] + strlen(AUTH_OPTION_APP));
                }
                else if (strncmp(argv[i], AUTH_OPTION_USER_APP, strlen(AUTH_OPTION_USER_APP)) == 0) {
                    d_authOptions.clear();
                    d_authOptions.append(AUTH_USER_APP_PREFIX);
                    d_authOptions.append(argv[i] + strlen(AUTH_OPTION_USER_APP));
                }
                else if (strncmp(argv[i], AUTH_OPTION_DIR, strlen(AUTH_OPTION_DIR)) == 0) {
                    d_authOptions.clear();
                    d_authOptions.append(AUTH_DIR_PREFIX);
                    d_authOptions.append(argv[i] + strlen(AUTH_OPTION_DIR));
                }
                else if (!std::strcmp(argv[i], AUTH_OPTION_USER)) {
                    d_authOptions.assign(AUTH_USER);
                }
                else {
                    printUsage();
                    return false;
                }
            }
            else {
                printUsage();
                return false;
            }
        }

        if (d_hosts.size() == 0) {
            d_hosts.push_back("localhost");
        }

        return true;
    }

public:

    PagePublisherExample()
        : d_port(8194)
        , d_service("//viper/page")
        , d_authOptions(AUTH_USER)
        , d_priority(10)
    {
    }

    bool authorize(const Service &authService,
                   Identity *providerIdentity,
                   ProviderSession *session,
                   const CorrelationId &cid)
    {
        {
            MutexGuard guard(&g_lock);
            g_authorizationStatus[cid] = WAITING;
        }
        EventQueue tokenEventQueue;
        session->generateToken(CorrelationId(), &tokenEventQueue);
        std::string token;
        Event event = tokenEventQueue.nextEvent();
        if (event.eventType() == Event::TOKEN_STATUS ||
            event.eventType() == Event::REQUEST_STATUS) {
            MessageIterator iter(event);
            while (iter.next()) {
                Message msg = iter.message();
                {
                    MutexGuard guard(&g_lock);
                    msg.print(std::cout);
                }
                if (msg.messageType() == TOKEN_SUCCESS) {
                    token = msg.getElementAsString(TOKEN);
                }
                else if (msg.messageType() == TOKEN_FAILURE) {
                    break;
                }
            }
        }
        if (token.length() == 0) {
            MutexGuard guard(&g_lock);
            std::cout << "Failed to get token" << std::endl;
            return false;
        }

        Request authRequest = authService.createAuthorizationRequest();
        authRequest.set(TOKEN, token.c_str());

        session->sendAuthorizationRequest(
            authRequest,
            providerIdentity,
            cid);

        time_t startTime = time(0);
        const int WAIT_TIME_SECONDS = 10;
        while (true) {
            {
                MutexGuard guard(&g_lock);
                if (WAITING != g_authorizationStatus[cid]) {
                    return AUTHORIZED == g_authorizationStatus[cid];
                }
            }
            time_t endTime = time(0);
            if (endTime - startTime > WAIT_TIME_SECONDS) {
                return false;
            }
            SLEEP(1);
        }
    }

    void run(int argc, char **argv)
    {
        if (!parseCommandLine(argc, argv))
            return;

        SessionOptions sessionOptions;
        for (size_t i = 0; i < d_hosts.size(); ++i) {
            sessionOptions.setServerAddress(d_hosts[i].c_str(), d_port, i);
        }
        sessionOptions.setAuthenticationOptions(d_authOptions.c_str());
        sessionOptions.setAutoRestartOnDisconnection(true);
        sessionOptions.setNumStartAttempts(d_hosts.size());

        std::cout << "Connecting to port " << d_port
                  << " on ";
        std::copy(d_hosts.begin(),
                  d_hosts.end(),
                  std::ostream_iterator<std::string>(std::cout, " "));
        std::cout << std::endl;

        MyEventHandler myEventHandler(d_service);
        ProviderSession session(sessionOptions, &myEventHandler, 0);
        if (!session.start()) {
            std::cerr <<"Failed to start session." << std::endl;
            return;
        }

        Identity providerIdentity = session.createIdentity();
        if (!d_authOptions.empty()) {
            bool isAuthorized = false;
            const char* authServiceName = "//blp/apiauth";
            if (session.openService(authServiceName)) {
                Service authService = session.getService(authServiceName);
                isAuthorized = authorize(authService, &providerIdentity,
                        &session, CorrelationId((void *)"auth"));
            }
            if (!isAuthorized) {
                std::cerr << "No authorization" << std::endl;
                return;
            }
        }

        ServiceRegistrationOptions serviceOptions;
        serviceOptions.setGroupId(d_groupId.c_str(), d_groupId.size());
        serviceOptions.setServicePriority(d_priority);
        if (!session.registerService(d_service.c_str(), providerIdentity, serviceOptions)) {
            std::cerr <<"Failed to register " << d_service << std::endl;
            return;
        }

        Service service = session.getService(d_service.c_str());

        // Now we will start publishing
        int value=1;
        while (g_running) {
            Event event = service.createPublishEvent();
            {
                MutexGuard guard(&g_mutex);
                if (0 == g_availableTopicCount) {
                    guard.release()->unlock();
                    SLEEP(1);
                    continue;
                }

                EventFormatter eventFormatter(event);
                for (MyStreams::iterator iter = g_streams.begin();
                    iter != g_streams.end(); ++iter) {
                    if (!iter->second->isAvailable()) {
                        continue;
                    }
                    std::ostringstream os;
                    os << ++value;

                    if (!iter->second->isInitialPaintSent()) {
                        eventFormatter.appendRecapMessage(
                                                        iter->second->topic());
                        eventFormatter.setElement("numRows", 25);
                        eventFormatter.setElement("numCols", 80);
                        eventFormatter.pushElement("rowUpdate");
                        for (int i = 1; i < 6; ++i) {
                            eventFormatter.appendElement();
                            eventFormatter.setElement("rowNum", i);
                            eventFormatter.pushElement("spanUpdate");
                            eventFormatter.appendElement();
                            eventFormatter.setElement("startCol", 1);
                            eventFormatter.setElement("length", 10);
                            eventFormatter.setElement("text", "INITIAL");
                            eventFormatter.setElement("fgColor", "RED");
                            eventFormatter.popElement();
                            eventFormatter.popElement();
                            eventFormatter.popElement();
                        }
                        eventFormatter.popElement();
                        iter->second->setIsInitialPaintSent(true);
                    }

                    eventFormatter.appendMessage("RowUpdate",
                                                 iter->second->topic());
                    eventFormatter.setElement("rowNum", 1);
                    eventFormatter.pushElement("spanUpdate");
                    eventFormatter.appendElement();
                    Name START_COL("startCol");
                    eventFormatter.setElement(START_COL, 1);
                    eventFormatter.setElement("length", int(os.str().size()));
                    eventFormatter.setElement("text", os.str().c_str());
                    eventFormatter.popElement();
                    eventFormatter.popElement();
                }
            }

            printMessages(event);
            session.publish(event);
            SLEEP(10);
        }
        session.stop();
    }
};

int main(int argc, char **argv)
{
    std::cout << "PagePublisherExample" << std::endl;
    PagePublisherExample example;
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
