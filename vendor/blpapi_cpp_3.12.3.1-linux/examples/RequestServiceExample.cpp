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
#include <blpapi_eventdispatcher.h>
#include <blpapi_eventformatter.h>
#include <blpapi_message.h>
#include <blpapi_name.h>
#include <blpapi_providersession.h>
#include <blpapi_session.h>
#include <blpapi_topiclist.h>
#include <blpapi_topic.h>
#include <blpapi_identity.h>

#include <ctime>
#include <sys/timeb.h>
#include <iostream>
#include <iterator>
#include <map>
#include <string>

using namespace BloombergLP;
using namespace blpapi;

namespace {

Name AUTHORIZATION_SUCCESS("AuthorizationSuccess");
Name RESOLUTION_SUCCESS("ResolutionSuccess");
Name SESSION_TERMINATED("SessionTerminated");
Name TOKEN("token");
Name TOKEN_SUCCESS("TokenGenerationSuccess");
Name TOKEN_FAILURE("TokenGenerationFailure");

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

double getTimestamp()
{
    timeb curTime;
    ftime(&curTime);
    return curTime.time + ((double)curTime.millitm)/1000;
}

} // namespace {

class MyProviderEventHandler : public ProviderEventHandler
{
    const std::string       d_serviceName;

public:
    MyProviderEventHandler(const std::string &serviceName)
    : d_serviceName(serviceName)
    {}

    bool processEvent(const Event& event, ProviderSession* session);
};

bool MyProviderEventHandler::processEvent(
        const Event& event, ProviderSession* session)
{
    std::cout << std::endl << "Server received an event" << std::endl;
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
    else if (event.eventType() == Event::RESOLUTION_STATUS) {
        printMessages(event);
    }
    else if (event.eventType() == Event::REQUEST) {
        Service service = session->getService(d_serviceName.c_str());
        MessageIterator iter(event);
        while (iter.next()) {
            Message msg = iter.message();
            msg.print(std::cout);
            if (msg.messageType() == Name("ReferenceDataRequest")) {
                // Similar to createPublishEvent. We assume just one
                // service - d_service. A responseEvent can only be
                // for single request so we can specify the
                // correlationId - which establishes context -
                // when we create the Event.
                if (msg.hasElement("timestamp")) {
                    double requestTime = msg.getElementAsFloat64("timestamp");
                    double latency = getTimestamp() - requestTime;
                    std::cout << "Response latency = "
                              << latency << std::endl;
                }
                Event response = service.createResponseEvent(
                        msg.correlationId());
                EventFormatter ef(response);

                // In appendResponse the string is the name of the
                // operation, the correlationId indicates
                // which request we are responding to.
                ef.appendResponse("ReferenceDataRequest");
                Element securities = msg.getElement("securities");
                Element fields = msg.getElement("fields");
                ef.setElement("timestamp", getTimestamp());
                ef.pushElement("securityData");
                for (size_t i = 0; i < securities.numValues(); ++i) {
                    ef.appendElement();
                    ef.setElement("security", securities.getValueAsString(i));
                    ef.pushElement("fieldData");
                    for (size_t j = 0; j < fields.numValues(); ++j) {
                        ef.appendElement();
                        ef.setElement("fieldId", fields.getValueAsString(j));
                        ef.pushElement("data");
                        ef.setElement("doubleValue", getTimestamp());
                        ef.popElement();
                        ef.popElement();
                    }
                    ef.popElement();
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
            if (g_authorizationStatus.find(msg.correlationId()) != g_authorizationStatus.end()) {
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

class MyRequesterEventHandler : public EventHandler
{

public:
    MyRequesterEventHandler()
    {}

    bool processEvent(const Event& event, Session *session);
};

bool MyRequesterEventHandler::processEvent(
        const Event& event, Session *session)
{
    std::cout << std::endl << "Client received an event" << std::endl;
    MessageIterator iter(event);
    while (iter.next()) {
        Message msg = iter.message();
        MutexGuard guard(&g_mutex);
        msg.print(std::cout);
        if (g_authorizationStatus.find(msg.correlationId()) !=
                g_authorizationStatus.end()) {
            if (msg.messageType() == AUTHORIZATION_SUCCESS) {
                g_authorizationStatus[msg.correlationId()] = AUTHORIZED;
            }
            else {
                g_authorizationStatus[msg.correlationId()] = FAILED;
            }
        }
    }
    return true;
}

class RequestServiceExample
{
    enum Role {
        SERVER,
        CLIENT,
        BOTH
    };
    std::vector<std::string> d_hosts;
    int                      d_port;
    std::string              d_service;
    std::string              d_authOptions;
    Role                     d_role;

    std::vector<std::string> d_securities;
    std::vector<std::string> d_fields;

    void printUsage()
    {
        std::cout
            << "Usage:" << std::endl
            << "\t[-ip   <ipAddress>]  \tserver name or IP (default: localhost)" << std::endl
            << "\t[-p    <tcpPort>]    \tserver port (default: 8194)" << std::endl
            << "\t[-auth <option>]     \tauthentication option: user|none|app=<app>|userapp=<app>|dir=<property> (default: user)" << std::endl
            << "\t[-s    <security>]   \trequest security for client (default: IBM US Equity)" << std::endl
            << "\t[-f    <field>]      \trequest field for client (default: PX_LAST)" << std::endl
            << "\t[-r    <option>]     \tservice role option: server|client|both (default: both)" << std::endl;
    }

    bool parseCommandLine(int argc, char **argv)
    {
        for (int i = 1; i < argc; ++i) {
            if (!std::strcmp(argv[i],"-ip") && i + 1 < argc)
                d_hosts.push_back(argv[++i]);
            else if (!std::strcmp(argv[i],"-p") && i + 1 < argc)
                d_port = std::atoi(argv[++i]);
            else if (!std::strcmp(argv[i],"-s") && i + 1 < argc)
                d_securities.push_back(argv[++i]);
            else if (!std::strcmp(argv[i],"-f") && i + 1 < argc)
                d_fields.push_back(argv[++i]);
            else if (!std::strcmp(argv[i],"-r") && i + 1 < argc) {
                ++ i;
                if (!std::strcmp(argv[i], "server")) {
                    d_role = SERVER;
                }
                else if (!std::strcmp(argv[i], "client")) {
                    d_role = CLIENT;
                }
                else if (!std::strcmp(argv[i], "both")) {
                    d_role = BOTH;
                }
                else {
                    printUsage();
                    return false;
                }
            }
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
        if (d_securities.size() == 0) {
            d_securities.push_back("IBM US Equity");
        }

        if (d_fields.size() == 0) {
            d_fields.push_back("PX_LAST");
        }
        return true;
    }

public:

    RequestServiceExample()
        : d_port(8194)
        , d_service("//example/refdata")
        , d_authOptions(AUTH_USER)
        , d_role(BOTH)
    {
    }

    bool authorize(const Service &authService,
                   Identity *providerIdentity,
                   AbstractSession *session,
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


    void serverRun(ProviderSession *providerSession)
    {
        ProviderSession& session = *providerSession;
        std::cout << "Server is starting------" << std::endl;
        if (!session.start()) {
            std::cerr << "Failed to start server session." << std::endl;
            return;
        }

        Identity providerIdentity = session.createIdentity();
        if (!d_authOptions.empty()) {
            bool isAuthorized = false;
            const char* authServiceName = "//blp/apiauth";
            if (session.openService(authServiceName)) {
                Service authService = session.getService(authServiceName);
                isAuthorized = authorize(authService, &providerIdentity,
                        &session, CorrelationId((void *)"sauth"));
            }
            if (!isAuthorized) {
                std::cerr << "No authorization" << std::endl;
                return;
            }
        }

        if (!session.registerService(d_service.c_str(), providerIdentity)) {
            std::cerr <<"Failed to register " << d_service << std::endl;
            return;
        }
    }

    void clientRun(Session *requesterSession)
    {
        Session& session = *requesterSession;
        std::cout << "Client is starting------" << std::endl;
        if (!session.start()) {
            std::cerr <<"Failed to start client session." << std::endl;
            return;
        }

        Identity identity = session.createIdentity();
        if (!d_authOptions.empty()) {
            bool isAuthorized = false;
            const char* authServiceName = "//blp/apiauth";
            if (session.openService(authServiceName)) {
                Service authService = session.getService(authServiceName);
                isAuthorized = authorize(authService, &identity,
                        &session, CorrelationId((void *)"cauth"));
            }
            if (!isAuthorized) {
                std::cerr << "No authorization" << std::endl;
                return;
            }
        }

        if (!session.openService(d_service.c_str())) {
            std::cerr <<"Failed to open " << d_service << std::endl;
            return;
        }

        Service service = session.getService(d_service.c_str());
        Request request = service.createRequest("ReferenceDataRequest");

        // append securities to request
        // Add securities to request
        Element securities = request.getElement("securities");
        for (size_t i = 0; i < d_securities.size(); ++i) {
            securities.appendValue(d_securities[i].c_str());
        }

        // Add fields to request
        Element fields = request.getElement("fields");
        for (size_t i = 0; i < d_fields.size(); ++i) {
            fields.appendValue(d_fields[i].c_str());
        }

        request.set("timestamp", getTimestamp());

        {
            MutexGuard guard(&g_mutex);
            std::cout << "Sending Request: " << request << std::endl;
        }
        EventQueue eventQueue;
        session.sendRequest(request, identity,
                CorrelationId((void *)"AddRequest"), &eventQueue);

        while (true) {
            Event event = eventQueue.nextEvent();
            std::cout << std::endl << "Client received an event" << std::endl;
            MessageIterator msgIter(event);
            while (msgIter.next()) {
                Message msg = msgIter.message();
                MutexGuard guard(&g_mutex);
                if (event.eventType() == Event::RESPONSE) {
                    if (msg.hasElement("timestamp")) {
                        double responseTime = msg.getElementAsFloat64(
                                                "timestamp");
                        std::cout << "Response latency = "
                                  << getTimestamp() - responseTime
                                  << std::endl;
                    }
                }
                msg.print(std::cout) << std::endl;
            }
            if (event.eventType() == Event::RESPONSE) {
                break;
            }
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
        std::copy(d_hosts.begin(), d_hosts.end(), std::ostream_iterator<std::string>(std::cout, " "));
        std::cout << std::endl;

        MyProviderEventHandler providerEventHandler(d_service);
        ProviderSession providerSession(
                sessionOptions, &providerEventHandler, 0);

        MyRequesterEventHandler requesterEventHandler;
        Session requesterSession(sessionOptions, &requesterEventHandler, 0);

        if (d_role == SERVER || d_role == BOTH) {
            serverRun(&providerSession);
        }
        if (d_role == CLIENT || d_role == BOTH) {
            clientRun(&requesterSession);
        }

        // wait for enter key to exit application
        std::cout << "Press ENTER to quit" << std::endl;
        char dummy[2];
        std::cin.getline(dummy, 2);
        if (d_role == SERVER || d_role == BOTH) {
            providerSession.stop();
        }
        if (d_role == CLIENT || d_role == BOTH) {
            requesterSession.stop();
        }
    }
};

int main(int argc, char **argv)
{
    std::cout << "RequestServiceExample" << std::endl;
    RequestServiceExample example;
    try {
        example.run(argc, argv);
    } catch (Exception &e) {
        std::cerr << "Library Exception!!! " << e.description() << std::endl;
    }
    return 0;
}
