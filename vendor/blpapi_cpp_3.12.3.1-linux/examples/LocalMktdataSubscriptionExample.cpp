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
#include <blpapi_tlsoptions.h>

#include <ctime>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <fstream>

using namespace BloombergLP;
using namespace blpapi;

namespace {

Name TOKEN_SUCCESS("TokenGenerationSuccess");
Name TOKEN_FAILURE("TokenGenerationFailure");
Name AUTHORIZATION_SUCCESS("AuthorizationSuccess");
Name TOKEN("token");

const std::string AUTH_USER       = "AuthenticationType=OS_LOGON";
const std::string AUTH_APP_PREFIX = "AuthenticationMode=APPLICATION_ONLY;"
                                    "ApplicationAuthenticationType=APPNAME_AND_KEY;"
                                    "ApplicationName=";
const std::string AUTH_USER_APP_PREFIX = "AuthenticationMode=USER_AND_APPLICATION;"
                                         "AuthenticationType=OS_LOGON;"
                                         "ApplicationAuthenticationType=APPNAME_AND_KEY;"
                                         "ApplicationName=";
const std::string AUTH_USER_APP_MANUAL_PREFIX = "AuthenticationMode=USER_AND_APPLICATION;"
                                                "AuthenticationType=MANUAL;"
                                                "ApplicationAuthenticationType=APPNAME_AND_KEY;"
                                                "ApplicationName=";
const std::string AUTH_DIR_PREFIX = "AuthenticationType=DIRECTORY_SERVICE;"
                                    "DirSvcPropertyName=";

const char* AUTH_OPTION_NONE      = "none";
const char* AUTH_OPTION_USER      = "user";
const char* AUTH_OPTION_APP       = "app=";
const char* AUTH_OPTION_USER_APP  = "userapp=";
const char* AUTH_OPTION_DIR       = "dir=";
const char* AUTH_OPTION_MANUAL    = "manual=";

std::vector<std::string> splitBy(const std::string& str, char delim)
{
    std::string::size_type start = 0u, pos = 0u;
    std::vector<std::string> tokens;
    while ((pos = str.find(delim, start)) != std::string::npos) {
        tokens.push_back(str.substr(start, pos - start));
        start = pos + 1;
    }
    if (start != str.size()) {
        tokens.push_back(str.substr(start));
    }
    return tokens;
}

}

class LocalMktdataSubscriptionExample
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
    std::string              d_clientCredentials;
    std::string              d_clientCredentialsPassword;
    std::string              d_trustMaterial;
    bool                     d_readTlsData;

    bool                     d_manualToken;
    std::string              d_manualUserId;
    std::string              d_manualIPAddress;

    void printUsage()
    {
        std::cout <<
"Retrieve realtime data.\n"
"Usage:\n"
"\t[-ip   <ipAddress>]    server name or IP (default: localhost)\n"
"\t[-p    <tcpPort>]      server port (default: 8194)\n"
"\t[-s    <service>]      service name (default: //viper/mktdata))\n"
"\t[-t    <topic>]        topic name (default: /ticker/IBM Equity)\n"
"\t[-f    <field>]        field to subscribe to (default: empty)\n"
"\t[-o    <option>]       subscription options (default: empty)\n"
"\t[-me   <maxEvents>]    stop after this many events (default: INT_MAX)\n"
"\t[-auth <option>]       authentication option (default: user):\n"
"\t\tnone\n"
"\t\tuser                     as a user using OS logon information\n"
"\t\tdir=<property>           as a user using directory services\n"
"\t\tapp=<app>                as the specified application\n"
"\t\tuserapp=<app>            as user and application using logon information\n"
"\t\t                         for the user\n"
"\t\tmanual=<app>,<ip>,<user> as user and application, with manually provided\n"
"\t\t                         IP address and EMRS user\n"
"\n"
"TLS OPTIONS (specify all or none):\n"
"\t[-tls-client-credentials <file>]     name a PKCS#12 file to use as a source of client credentials\n"
"\t[-tls-client-credentials-password <pwd>]     specify password for accessing client credentials\n"
"\t[-tls-trust-material <file>]         name a PKCS#7 file to use as a source of trusted certificates\n"
"\t[-read-certificate-files]            (optional) read the TLS files and pass the blobs\n"
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
                d_manualToken = false;
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
                else if (!std::strncmp(argv[i], AUTH_OPTION_MANUAL, strlen(AUTH_OPTION_MANUAL))) {
                    std::vector<std::string> elems = splitBy(argv[i] + strlen(AUTH_OPTION_MANUAL), ',');
                    if (elems.size() != 3u) {
                        std::cerr << "Invalid auth option: " << argv[i] << '\n';
                        printUsage();
                        return false;
                    }
                    d_authOptions.clear();
                    d_authOptions.append(AUTH_USER_APP_MANUAL_PREFIX);
                    d_authOptions.append(elems[0]);

                    d_manualToken = true;
                    d_manualIPAddress.swap(elems[1]);
                    d_manualUserId.swap(elems[2]);
                }
                else {
                    printUsage();
                    return false;
                }
            } else if (!std::strcmp(argv[i], "-tls-client-credentials") && i + 1 < argc) {
                d_clientCredentials = argv[++i];
            } else if (!std::strcmp(argv[i], "-tls-client-credentials-password") && i + 1 < argc) {
                d_clientCredentialsPassword = argv[++i];
            } else if (!std::strcmp(argv[i], "-tls-trust-material") && i + 1 < argc) {
                d_trustMaterial = argv[++i];
            } else if (!std::strcmp(argv[i], "-read-certificate-files")) {
                d_readTlsData = true;
            } else {
                printUsage();

                std::cerr << "\nUnexpected option: '" << argv[i] << "'\n\n";
                return false;
            }
        }

        if (d_hosts.size() == 0) {
            d_hosts.push_back("localhost");
        }

        if (d_topics.size() == 0) {
            d_topics.push_back("/ticker/IBM Equity");
        }

        return true;
    }

   bool authorize(const Service &authService,
                  Identity *subscriptionIdentity,
                  Session *session,
                  const CorrelationId &cid)
    {
        EventQueue tokenEventQueue;
        if (!d_manualToken) {
            session->generateToken(CorrelationId(), &tokenEventQueue);
        } else {
            session->generateToken(d_manualUserId.c_str(),
                                   d_manualIPAddress.c_str(),
                                   CorrelationId(),
                                   &tokenEventQueue);
        }
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
    LocalMktdataSubscriptionExample()
        : d_port(8194)
        , d_maxEvents(INT_MAX)
        , d_eventCount(0)
        , d_service("//viper/mktdata")
        , d_authOptions(AUTH_USER)
        , d_readTlsData(false)
        , d_manualToken(false)
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

        if (d_clientCredentials.size()
                && d_trustMaterial.size()) {

            std::cout << "TlsOptions enabled" << std::endl;
            TlsOptions tlsOptions;
            if (d_readTlsData) {
                std::string clientCredentials;
                std::string trustMaterial;
                {
                    std::ifstream in(d_clientCredentials.c_str(),
                                     std::ios::in | std::ios::binary
                                        | std::ios::ate);
                    if (in) {
                        clientCredentials.resize(in.tellg());
                        in.seekg(0, std::ios::beg);
                        in.read(&clientCredentials[0], clientCredentials.size());
                    }
                    if (in.fail()) {
                        std::cerr << "Failed to read file from "
                                  << d_clientCredentials
                                  << std::endl;
                        return;
                    }

                    std::cout << "Read " << clientCredentials.size()
                              << " bytes from " << d_clientCredentials
                              << std::endl;
                }
                {
                    std::ifstream in(d_trustMaterial.c_str(),
                                     std::ios::in | std::ios::binary
                                        | std::ios::ate);
                    if (in) {
                        trustMaterial.resize(in.tellg());
                        in.seekg(0, std::ios::beg);
                        in.read(&trustMaterial[0], trustMaterial.size());
                    }
                    if (in.fail()) {
                        std::cerr << "Failed to read file from "
                                  << d_trustMaterial
                                  << std::endl;
                        return;
                    }

                    std::cout << "Read " << trustMaterial.size()
                              << " bytes from " << d_trustMaterial
                              << std::endl;
                }

                tlsOptions = TlsOptions::createFromBlobs(
                                        clientCredentials.data(),
                                        clientCredentials.size(),
                                        d_clientCredentialsPassword.c_str(),
                                        trustMaterial.data(),
                                        trustMaterial.size());
            } else {
                tlsOptions =
                    TlsOptions::createFromFiles(
                                        d_clientCredentials.c_str(),
                                        d_clientCredentialsPassword.c_str(),
                                        d_trustMaterial.c_str());
            }
            sessionOptions.setTlsOptions(tlsOptions);
        }


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

        SubscriptionList subscriptions;
        for (size_t i = 0; i < d_topics.size(); ++i) {
            std::string topic(d_service + d_topics[i]);
            subscriptions.add(topic.c_str(),
                              d_fields,
                              d_options,
                              CorrelationId((char*)d_topics[i].c_str()));
        }
        session.subscribe(subscriptions, subscriptionIdentity);

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
    std::cout << "LocalMktdataSubscriptionExample" << std::endl;
    LocalMktdataSubscriptionExample example;
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
