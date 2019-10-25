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

#include <ctime>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>

using namespace BloombergLP;
using namespace blpapi;

namespace {

Name AUTHORIZATION_SUCCESS("AuthorizationSuccess");
Name PERMISSION_REQUEST("PermissionRequest");
Name RESOLUTION_SUCCESS("ResolutionSuccess");
Name SESSION_TERMINATED("SessionTerminated");
Name TOKEN("token");
Name TOKEN_SUCCESS("TokenGenerationSuccess");
Name TOKEN_FAILURE("TokenGenerationFailure");
Name TOPICS("topics");
Name TOPIC_CREATED("TopicCreated");
Name TOPIC_SUBSCRIBED("TopicSubscribed");
Name TOPIC_UNSUBSCRIBED("TopicUnsubscribed");
Name TOPIC_RECAP("TopicRecap");

const std::string AUTH_USER       = "AuthenticationType=OS_LOGON";
const std::string AUTH_APP_PREFIX = "AuthenticationMode=APPLICATION_ONLY;"
    "ApplicationAuthenticationType=APPNAME_AND_KEY;ApplicationName=";
const std::string AUTH_USER_APP_PREFIX =
    "AuthenticationMode=USER_AND_APPLICATION;AuthenticationType=OS_LOGON;"
    "ApplicationAuthenticationType=APPNAME_AND_KEY;ApplicationName=";
const std::string AUTH_DIR_PREFIX =
    "AuthenticationType=DIRECTORY_SERVICE;DirSvcPropertyName=";
const char* AUTH_OPTION_NONE      = "none";
const char* AUTH_OPTION_USER      = "user";
const char* AUTH_OPTION_APP       = "app=";
const char* AUTH_OPTION_USER_APP  = "userapp=";
const char* AUTH_OPTION_DIR       = "dir=";


bool g_running = true;

class MyStream
{
    const std::string       d_id;
    const std::vector<Name> d_fields;
    int                     d_lastValue;
    int                     d_fieldsPublished;

    Topic                   d_topic;
    bool                    d_isSubscribed;

public:
    MyStream()
    : d_id("")
    , d_lastValue(0)
    , d_fieldsPublished(0)
    {}

    MyStream(const std::string& id, const std::vector<Name>& fields)
    : d_id(id)
    , d_fields(fields)
    , d_lastValue(0)
    , d_fieldsPublished(0)
    {}

    void setTopic(Topic topic) {
        d_topic = topic;
    }

    void setSubscribedState(bool isSubscribed) {
        d_isSubscribed = isSubscribed;
    }

    void fillData(EventFormatter&                eventFormatter,
                  const SchemaElementDefinition& elementDef)
    {
        for (int i = 0; i < d_fields.size(); ++i) {
            if (!elementDef.typeDefinition().hasElementDefinition(
                    d_fields[i])) {
                std::cerr << "Invalid field " << d_fields[i] << std::endl;
                continue;
            }
            SchemaElementDefinition fieldDef =
                elementDef.typeDefinition().getElementDefinition(d_fields[i]);

            switch (fieldDef.typeDefinition().datatype()) {
            case BLPAPI_DATATYPE_BOOL:
                eventFormatter.setElement(d_fields[i],
                                          bool((d_lastValue + i) % 2 == 0));
                break;
            case BLPAPI_DATATYPE_CHAR:
                eventFormatter.setElement(d_fields[i],
                                          char((d_lastValue + i) % 100 + 32));
                break;
            case BLPAPI_DATATYPE_INT32:
            case BLPAPI_DATATYPE_INT64:
                eventFormatter.setElement(d_fields[i], d_lastValue + i);
                break;
            case BLPAPI_DATATYPE_FLOAT32:
            case BLPAPI_DATATYPE_FLOAT64:
                eventFormatter.setElement(d_fields[i],
                                          (d_lastValue + i) * 1.1f);
                break;
            case BLPAPI_DATATYPE_STRING:
                {
                    std::ostringstream s;
                    s << "S" << (d_lastValue + i);
                    eventFormatter.setElement(d_fields[i], s.str().c_str());
                }
                break;
            case BLPAPI_DATATYPE_DATE:
            case BLPAPI_DATATYPE_TIME:
            case BLPAPI_DATATYPE_DATETIME:
                Datetime datetime;
                datetime.setDate(2011, 1, d_lastValue / 100 % 30 + 1);
                {
                    time_t now = time(0);
                    datetime.setTime(now / 3600 % 24,
                                     now % 3600 / 60,
                                     now % 60);
                }
                datetime.setMilliseconds(i);
                eventFormatter.setElement(d_fields[i], datetime);
                break;
            }
        }
    }

    void fillDataNull(EventFormatter&                eventFormatter,
                      const SchemaElementDefinition& elementDef)
    {
        for (int i = 0; i < d_fields.size(); ++i) {
            if (!elementDef.typeDefinition().hasElementDefinition(
                    d_fields[i])) {
                std::cerr << "Invalid field " << d_fields[i] << std::endl;
                continue;
            }
            SchemaElementDefinition fieldDef =
                elementDef.typeDefinition().getElementDefinition(d_fields[i]);

            if (fieldDef.typeDefinition().isSimpleType()) {
                // Publishing NULL value
                eventFormatter.setElementNull(d_fields[i]);
            }
        }
    }

    const std::string& getId() { return d_id; }

    void next() { d_lastValue += 1; }

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

enum AuthorizationStatus {
    WAITING,
    AUTHORIZED,
    FAILED
};

std::map<CorrelationId, AuthorizationStatus> g_authorizationStatus;

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
    const std::string       d_serviceName;
    Name                    d_messageType;
    const std::vector<Name> d_fields;
    const std::vector<int>  d_eids;
    int                     d_resolveSubServiceCode;

public:
    MyEventHandler(const std::string&       serviceName,
                   const Name&              messageType,
                   const std::vector<Name>& fields,
                   const std::vector<int>&  eids,
                   int                      resolveSubServiceCode)
    : d_serviceName(serviceName)
    , d_messageType(messageType)
    , d_fields(fields)
    , d_eids(eids)
    , d_resolveSubServiceCode(resolveSubServiceCode)
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
                                     new MyStream(topicStr, d_fields)))).first;
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
                                     new MyStream(topicStr, d_fields)))).first;
                }
                try {
                    Topic topic = session->getTopic(msg);
                    it->second->setTopic(topic);
                } catch (blpapi::Exception &e) {
                    std::cerr << "Exception while processing TOPIC_CREATED: "
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
                    SchemaElementDefinition elementDef =
                        service.getEventDefinition(d_messageType);
                    EventFormatter eventFormatter(recapEvent);
                    eventFormatter.appendRecapMessage(topic, &recapCid);
                    it->second->fillData(eventFormatter, elementDef);
                    guard.release()->unlock();
                    session->publish(recapEvent);
                } catch (blpapi::Exception &e) {
                    std::cerr << "Exception while processing TOPIC_RECAP: "
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
                // Similar to createPublishEvent. We assume just one
                // service - d_service. A responseEvent can only be
                // for single request so we can specify the
                // correlationId - which establishes context -
                // when we create the Event.
                Event response =
                    service.createResponseEvent(msg.correlationId());
                int permission = 1; // ALLOWED: 0, DENIED: 1
                EventFormatter ef(response);
                if (msg.hasElement("uuid")) {
                    int uuid = msg.getElementAsInt32("uuid");
                    permission = 0;
                }
                if (msg.hasElement("applicationId")) {
                    int applicationId = msg.getElementAsInt32("applicationId");
                    permission = 0;
                }

                // In appendResponse the string is the name of the
                // operation, the correlationId indicates
                // which request we are responding to.
                ef.appendResponse("PermissionResponse");
                ef.pushElement("topicPermissions");
                // For each of the topics in the request, add an entry
                // to the response
                Element topicsElement = msg.getElement(TOPICS);
                for (size_t i = 0; i < topicsElement.numValues(); ++i) {
                    ef.appendElement();
                    ef.setElement("topic", topicsElement.getValueAsString(i));
                    if (d_resolveSubServiceCode != INT_MIN) {
                        try {
                            ef.setElement("subServiceCode",
                                          d_resolveSubServiceCode);
                            std::cout << "Mapping topic "
                                      << topicsElement.getValueAsString(i)
                                      << " to subServiceCode "
                                      << d_resolveSubServiceCode
                                      << std::endl;
                        }
                        catch (blpapi::Exception &e) {
                            std::cerr << "subServiceCode could not be set."
                                      << " Resolving without subServiceCode"
                                      << std::endl;
                        }
                    }
                    ef.setElement("result", permission);
                        // ALLOWED: 0, DENIED: 1

                    if (permission == 1) {
                            // DENIED

                        ef.pushElement("reason");
                        ef.setElement("source", "My Publisher Name");
                        ef.setElement("category", "NOT_AUTHORIZED");
                            // or BAD_TOPIC, or custom

                        ef.setElement("subcategory", "Publisher Controlled");
                        ef.setElement("description",
                            "Permission denied by My Publisher Name");
                        ef.popElement();
                    }
                    else {
                        if (d_eids.size()) {
                            ef.pushElement("permissions");
                            ef.appendElement();
                            ef.setElement("permissionService",
                                          "//blp/blpperm");
                            ef.pushElement("eids");
                            for (std::vector<int>::const_iterator it
                                     = d_eids.begin();
                                 it != d_eids.end();
                                 ++it) {
                                ef.appendValue(*it);
                            }
                            ef.popElement();
                            ef.popElement();
                            ef.popElement();
                        }
                    }
                    ef.popElement();
                }
                ef.popElement();
                // Service is implicit in the Event. sendResponse has a
                // second parameter - partialResponse -
                // that defaults to false.
                session->sendResponse(response);
            }
        }
    }
    else {
        MessageIterator iter(event);
        while (iter.next()) {
            Message msg = iter.message();
            MutexGuard guard(&g_mutex);
            if (g_authorizationStatus.find(msg.correlationId())
                    != g_authorizationStatus.end()) {
                if (msg.messageType() == AUTHORIZATION_SUCCESS) {
                    g_authorizationStatus[msg.correlationId()] = AUTHORIZED;
                }
                else {
                    g_authorizationStatus[msg.correlationId()] = FAILED;
                }
            }
        }
        printMessages(event);
    }

    return true;
}

class MktdataPublisherExample
{
    std::vector<std::string> d_hosts;
    int                      d_port;
    int                      d_priority;
    std::string              d_service;
    std::vector<Name>        d_fields;
    std::string              d_messageType;
    std::vector<int>         d_eids;
    std::string              d_groupId;
    std::string              d_authOptions;
    int                      d_clearInterval;

    bool                     d_useSsc;
    int                      d_sscBegin;
    int                      d_sscEnd;
    int                      d_sscPriority;

    int                      d_resolveSubServiceCode;
    ProviderSession         *d_session_p;

    void printUsage()
    {
        std::cout
            << "Publish market data." << std::endl
            << "Usage:" << std::endl
            << "\t[-ip   <ipAddress>]  \tserver name or IP"
            << " (default: localhost)"
            << std::endl
            << "\t[-p    <tcpPort>]    \tserver port (default: 8194)"
            << std::endl
            << "\t[-s    <service>]    \tservice name"
            << " (default: //viper/mktdata)"
            << std::endl
            << "\t[-f    <field>]      \tfields (default: LAST_PRICE)"
            << std::endl
            << "\t[-m    <messageType>]\ttype of published event"
            << " (default: MarketDataEvents)" << std::endl
            << "\t[-e    <EID>]        \tpermission eid for all subscriptions"
            << std::endl
            << "\t[-g    <groupId>]    \tpublisher groupId"
            << " (defaults to unique value)" << std::endl
            << "\t[-pri  <priority>]   \tset publisher priority level"
            << " (default: 10)" << std::endl
            << "\t[-c    <event count>]\tnumber of events after which cache"
            << " will be cleared (default: 0 i.e cache never cleared)"
            << std::endl
            << "\t[-auth <option>]     \tauthentication option: user|none|"
            << "app=<app>|userapp=<app>|dir=<property> (default: user)"
            << std::endl
            << "\t[-ssc <option>]      \tactive sub-service code option:"
            << "<begin>,<end>,<priority> "
            << std::endl
            << "\t[-rssc <option>      \tsub-service code to be used in"
            << " resolves."
            << std::endl;
    }

    bool parseCommandLine(int argc, char **argv)
    {
        for (int i = 1; i < argc; ++i) {
            if (!std::strcmp(argv[i],"-ip") && i + 1 < argc)
                d_hosts.push_back(argv[++i]);
            else if (!std::strcmp(argv[i],"-p") && i + 1 < argc)
                d_port = std::atoi(argv[++i]);
            else if (!std::strcmp(argv[i],"-s") && i + 1 < argc)
                d_service = argv[++i];
            else if (!std::strcmp(argv[i],"-f") && i + 1 < argc)
                d_fields.push_back(Name(argv[++i]));
            else if (!std::strcmp(argv[i],"-m") && i + 1 < argc)
                d_messageType = argv[++i];
            else if (!std::strcmp(argv[i],"-e") && i + 1 < argc)
                d_eids.push_back(std::atoi(argv[++i]));
            else if (!std::strcmp(argv[i],"-g") && i + 1 < argc)
                d_groupId = argv[++i];
            else if (!std::strcmp(argv[i],"-pri") && i + 1 < argc)
                d_priority = std::atoi(argv[++i]);
            else if (!std::strcmp(argv[i],"-c") && i + 1 < argc)
                d_clearInterval = std::atoi(argv[++i]);
            else if (!std::strcmp(argv[i], "-auth") && i + 1 < argc) {
                ++ i;
                if (!std::strcmp(argv[i], AUTH_OPTION_NONE)) {
                    d_authOptions.clear();
                }
                else if (strncmp(argv[i],
                                 AUTH_OPTION_APP,
                                 strlen(AUTH_OPTION_APP)) == 0) {
                    d_authOptions.clear();
                    d_authOptions.append(AUTH_APP_PREFIX);
                    d_authOptions.append(argv[i] + strlen(AUTH_OPTION_APP));
                }
                else if (strncmp(argv[i],
                                 AUTH_OPTION_USER_APP,
                                 strlen(AUTH_OPTION_USER_APP)) == 0) {
                    d_authOptions.clear();
                    d_authOptions.append(AUTH_USER_APP_PREFIX);
                    d_authOptions.append(
                        argv[i] + strlen(AUTH_OPTION_USER_APP));
                }
                else if (strncmp(argv[i],
                                 AUTH_OPTION_DIR,
                                 strlen(AUTH_OPTION_DIR)) == 0) {
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
            else if (!std::strcmp(argv[i], "-ssc") && i + 1 < argc) {
                // subservice code entires
                d_useSsc = true;
                std::sscanf(argv[++i],
                            "%d,%d,%d",
                            &d_sscBegin,
                            &d_sscEnd,
                            &d_sscPriority);
            }
            else if (!std::strcmp(argv[i], "-rssc") && i + 1 < argc) {
                d_resolveSubServiceCode = std::atoi(argv[++i]);
            }
            else {
                printUsage();
                return false;
            }
        }

        if (d_hosts.size() == 0) {
            d_hosts.push_back("localhost");
        }
        if (!d_fields.size()) {
            d_fields.push_back(Name("LAST_PRICE"));
        }
        return true;
    }

public:
    MktdataPublisherExample()
        : d_port(8194)
        , d_service("//viper/mktdata")
        , d_messageType("MarketDataEvents")
        , d_authOptions(AUTH_USER)
        , d_priority(10)
        , d_clearInterval(0)
        , d_useSsc(false)
        , d_resolveSubServiceCode(INT_MIN)
    {
    }

    void activate() {
        if (d_useSsc) {
            std::cout << "Activating sub service code range "
                      << "[" << d_sscBegin << ", " << d_sscEnd
                      << "] @ priority " << d_sscPriority << std::endl;
            d_session_p->activateSubServiceCodeRange(d_service.c_str(),
                                                     d_sscBegin,
                                                     d_sscEnd,
                                                     d_sscPriority);
        }
    }
    void deactivate() {
        if (d_useSsc) {
            std::cout << "DeActivating sub service code range "
                      << "[" << d_sscBegin << ", " << d_sscEnd
                      << "] @ priority " << d_sscPriority << std::endl;

            d_session_p->deactivateSubServiceCodeRange(d_service.c_str(),
                                                       d_sscBegin,
                                                       d_sscEnd);
        }
    }

    bool authorize(const Service &authService,
                   Identity *providerIdentity,
                   ProviderSession *session,
                   const CorrelationId &cid)
    {
        {
            MutexGuard guard(&g_mutex);
            g_authorizationStatus[cid] = WAITING;
        }
        EventQueue tokenEventQueue;
        session->generateToken(CorrelationId(), &tokenEventQueue);
        std::string token;
        Event event = tokenEventQueue.nextEvent();
        if (event.eventType() == Event::TOKEN_STATUS) {
            MessageIterator iter(event);
            while (iter.next()) {
                Message msg = iter.message();
                msg.print(std::cout);
                if (msg.messageType() == TOKEN_SUCCESS) {
                    token = msg.getElementAsString(TOKEN);
                }
                else if (msg.messageType() == TOKEN_FAILURE) {
                    break;
                }
            }
        }
        if (token.length() == 0) {
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
                MutexGuard guard(&g_mutex);
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

        // NOTE: If running without a backup server, make many attempts to
        // connect/reconnect to give that host a chance to come back up (the
        // larger the number, the longer it will take for SessionStartupFailure
        // to come on startup, or SessionTerminated due to inability to fail
        // over).  We don't have to do that in a redundant configuration - it's
        // expected at least one server is up and reachable at any given time,
        // so only try to connect to each server once.
        sessionOptions.setNumStartAttempts(d_hosts.size() > 1? 1: 1000);

        std::cout << "Connecting to port " << d_port
                  << " on ";
        std::copy(d_hosts.begin(),
                  d_hosts.end(),
                  std::ostream_iterator<std::string>(std::cout, " "));
        std::cout << std::endl;

        Name PUBLISH_MESSAGE_TYPE(d_messageType.c_str());

        MyEventHandler myEventHandler(d_service,
                                      PUBLISH_MESSAGE_TYPE,
                                      d_fields,
                                      d_eids,
                                      d_resolveSubServiceCode);
        ProviderSession session(sessionOptions, &myEventHandler, 0);
        d_session_p = & session;
        if (!session.start()) {
            std::cerr << "Failed to start session." << std::endl;
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
        if (d_useSsc) {
            std::cout << "Adding active sub service code range "
                      << "[" << d_sscBegin << ", " << d_sscEnd
                      << "] @ priority " << d_sscPriority << std::endl;
            try {
                serviceOptions.addActiveSubServiceCodeRange(d_sscBegin,
                                                            d_sscEnd,
                                                            d_sscPriority);
            }
            catch (Exception& e) {
                std::cerr << "FAILED to add active sub service codes."
                          << " Exception "
                          << e.description() << std::endl;
            }
        }
        if (!session.registerService(d_service.c_str(),
                                     providerIdentity,
                                     serviceOptions)) {
            std::cerr << "Failed to register " << d_service << std::endl;
            return;
        }

        Service service = session.getService(d_service.c_str());
        SchemaElementDefinition elementDef
            = service.getEventDefinition(PUBLISH_MESSAGE_TYPE);
        int eventCount = 0;

        // Now we will start publishing
        int numPublished = 0;
        while (g_running) {
            Event event = service.createPublishEvent();
            {
                MutexGuard guard(&g_mutex);
                if (0 == g_availableTopicCount) {
                    guard.release()->unlock();
                    SLEEP(1);
                    continue;
                }

                bool publishNull = false;
                if (d_clearInterval > 0 && eventCount == d_clearInterval) {
                    eventCount = 0;
                    publishNull = true;
                }
                EventFormatter eventFormatter(event);
                for (MyStreams::iterator iter = g_streams.begin();
                    iter != g_streams.end(); ++iter) {
                    if (!iter->second->isAvailable()) {
                        continue;
                    }
                    eventFormatter.appendMessage(
                            PUBLISH_MESSAGE_TYPE,
                            iter->second->topic());
                    if (publishNull) {
                        iter->second->fillDataNull(eventFormatter, elementDef);
                    } else {
                        ++eventCount;
                        iter->second->next();
                        iter->second->fillData(eventFormatter, elementDef);
                    }
                }
            }

            printMessages(event);
            session.publish(event);
            SLEEP(1);
            if (++numPublished % 10 == 0) {
               deactivate();
               SLEEP(30);
               activate();
            }

        }
        session.stop();
    }
};

int main(int argc, char **argv)
{
    std::cout << "MktdataPublisherExample" << std::endl;
    MktdataPublisherExample example;
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
