/* Copyright 2015. Bloomberg Finance L.P.
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

#include <ctime>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>

using namespace BloombergLP;
using namespace blpapi;

namespace {

Name TOKEN_SUCCESS("TokenGenerationSuccess");
Name TOKEN_FAILURE("TokenGenerationFailure");
Name AUTHORIZATION_SUCCESS("AuthorizationSuccess");
Name TOKEN("token");

const std::string AUTH_USER       = "AuthenticationType=OS_LOGON";
const std::string AUTH_APP_PREFIX = "AuthenticationMode=APPLICATION_ONLY;ApplicationAuthenticationType=APPNAME_AND_KEY;ApplicationName=";
const std::string AUTH_USER_APP_PREFIX = "AuthenticationMode=USER_AND_APPLICATION;AuthenticationType=OS_LOGON;ApplicationAuthenticationType=APPNAME_AND_KEY;ApplicationName=";
const std::string AUTH_DIR_PREFIX = "AuthenticationType=DIRECTORY_SERVICE;DirSvcPropertyName=";
const char* AUTH_OPTION_NONE      = "none";
const char* AUTH_OPTION_USER      = "user";
const char* AUTH_OPTION_APP       = "app=";
const char* AUTH_OPTION_USER_APP  = "userapp=";
const char* AUTH_OPTION_DIR       = "dir=";

}

class SnapshotRequestTemplateExample
{
    std::vector<std::string> d_hosts;
    int                      d_port;
    int                      d_maxEvents;
    int                      d_eventCount;
    std::string              d_service;
    std::vector<std::string> d_topics;
    std::vector<std::string> d_fields;
    std::vector<std::string> d_options;
    std::string              d_authOptions;

    void printUsage()
    {
        std::cout
            << "Create a snapshot request template and send a request using the request template." << std::endl
            << "Usage:" << std::endl
            << "\t[-ip   <ipAddress>]\tserver name or IP (default: localhost)" << std::endl
            << "\t[-p    <tcpPort>]  \tserver port (default: 8194)" << std::endl
            << "\t[-s    <service>]  \tservice name (default: //viper/mktdata))" << std::endl
            << "\t[-t    <topic>]    \ttopic name (default: /ticker/IBM Equity)" << std::endl
            << "\t[-f    <field>]    \tfield to subscribe to (default: empty)" << std::endl
            << "\t[-o    <option>]   \tsubscription options (default: empty)" << std::endl
            << "\t[-me   <maxEvents>]\tstop after this many events (default: INT_MAX)" << std::endl
            << "\t[-auth <option>]   \tauthentication option: user|none|app=<app>|userapp=<app>|dir=<property> (default: user)" << std::endl;
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
            else if (!std::strcmp(argv[i],"-t") && i + 1 < argc)
                d_topics.push_back(argv[++i]);
            else if (!std::strcmp(argv[i],"-f") && i + 1 < argc)
                d_fields.push_back(argv[++i]);
            else if (!std::strcmp(argv[i],"-o") && i + 1 < argc)
                d_options.push_back(argv[++i]);
            else if (!std::strcmp(argv[i],"-me") && i + 1 < argc)
                d_maxEvents = std::atoi(argv[++i]);
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

        if (d_topics.size() == 0) {
            d_topics.push_back("/ticker/IBM Equity");
        }

        if (d_fields.size() == 0) {
            d_topics.push_back("LAST_PRICE");
        }

        return true;
    }

   bool authorize(const Service &authService,
                  Identity *subscriptionIdentity,
                  Session *session,
                  const CorrelationId &cid)
    {
        EventQueue tokenEventQueue;
        session->generateToken(CorrelationId(), &tokenEventQueue);
        std::string token;
        Event event = tokenEventQueue.nextEvent();
        MessageIterator iter(event);
        if (event.eventType() == Event::TOKEN_STATUS ||
            event.eventType() == Event::REQUEST_STATUS) {
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

        session->sendAuthorizationRequest(authRequest, subscriptionIdentity);

        time_t startTime = time(0);
        const int WAIT_TIME_SECONDS = 10;
        while (true) {
            Event event = session->nextEvent(WAIT_TIME_SECONDS * 1000);
            if (event.eventType() == Event::RESPONSE ||
                event.eventType() == Event::REQUEST_STATUS ||
                event.eventType() == Event::PARTIAL_RESPONSE)
            {
                MessageIterator msgIter(event);
                while (msgIter.next()) {
                    Message msg = msgIter.message();
                    msg.print(std::cout);
                    if (msg.messageType() == AUTHORIZATION_SUCCESS) {
                        return true;
                    }
                    else {
                        std::cout << "Authorization failed" << std::endl;
                        return false;
                    }
                }
            }
            time_t endTime = time(0);
            if (endTime - startTime > WAIT_TIME_SECONDS) {
                return false;
            }
        }
    }

public:
    SnapshotRequestTemplateExample()
        : d_port(8194)
        , d_maxEvents(INT_MAX)
        , d_eventCount(0)
        , d_service("//viper/mktdata")
        , d_authOptions(AUTH_USER)
    {
    }

    void run(int argc, char **argv)
    {
        if (!parseCommandLine(argc, argv))
            return;

        SessionOptions sessionOptions;
        for (size_t i = 0; i < d_hosts.size(); ++i) { // override default 'localhost:8194'
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
        for (size_t i = 0; i < sessionOptions.numServerAddresses(); ++i) {
            unsigned short port;
            const char *host;
            sessionOptions.getServerAddress(&host, &port, i);
            std::cout << (i? ", ": "") << host;
        }
        std::cout << std::endl;

        Session session(sessionOptions);
        if (!session.start()) {
            std::cerr <<"Failed to start session." << std::endl;
            return;
        }

        Identity subscriptionIdentity = session.createIdentity();
        if (!d_authOptions.empty()) {
            bool isAuthorized = false;
            const char* authServiceName = "//blp/apiauth";
            if (session.openService(authServiceName)) {
                Service authService = session.getService(authServiceName);
                isAuthorized = authorize(authService, &subscriptionIdentity,
                        &session, CorrelationId((void *)"auth"));
            }
            if (!isAuthorized) {
                std::cerr << "No authorization" << std::endl;
                return;
            }
        }

        std::string fieldsStr = "?fields=";
        for(int iField = 0; iField < d_fields.size(); iField++) {
            if(0 != iField) {
                fieldsStr+=",";
            }
            fieldsStr+=d_fields[iField];
        }

        // NOTE: resources used by a snapshot request template are
        // released only when 'RequestTemplateTerminated' message
        // is received or when the session is destroyed. In order
        // to release resources when request template is not needed
        // anymore, user should call the 'Session::cancel' and pass
        // the correlation id used when creating the request template.
        std::vector<RequestTemplate> snapshots;
        std::cout << "Creating snapshot request templates"  << std::endl;
        for (size_t iSnapshot = 0; iSnapshot < d_topics.size(); ++iSnapshot) {
            std::string subscriptionString(
                d_service + d_topics[iSnapshot] + fieldsStr);
            snapshots.push_back(
                session.createSnapshotRequestTemplate(
                    subscriptionString.c_str(), 
                    CorrelationId(iSnapshot),
                    subscriptionIdentity));
        }

        while (true) {
            Event event = session.nextEvent(1000);
            MessageIterator msgIter(event);
            while (msgIter.next()) {
                Message msg = msgIter.message();
                if (event.eventType() == Event::RESPONSE ||
                    event.eventType() == Event::PARTIAL_RESPONSE) {
                    int iSnapshot = msg.correlationId().asInteger();
                    std::cout << d_topics[iSnapshot] << " - ";
                }
                msg.print(std::cout) << std::endl;
            }
            if (event.eventType() == Event::RESPONSE) {
                if (++d_eventCount >= d_maxEvents) {
                    break;
                }
            }
            if (event.eventType() == Event::TIMEOUT) {
                std::cout << "Sending request using the request templates"  
                    << std::endl;
                for (size_t iSnapshot = 0;
                    iSnapshot < snapshots.size();
                    ++iSnapshot) {
                    session.sendRequest(snapshots[iSnapshot], 
                                        CorrelationId(iSnapshot));
                }
            }
        }
    }
};

int main(int argc, char **argv)
{
    std::cout << "SnapshotRequestTemplateExample" << std::endl;
    SnapshotRequestTemplateExample example;
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
