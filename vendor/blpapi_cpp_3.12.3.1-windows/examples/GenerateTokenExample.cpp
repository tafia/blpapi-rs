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

#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>

using namespace BloombergLP;
using namespace blpapi;

namespace {
    const Name AUTHORIZATION_SUCCESS("AuthorizationSuccess");
    const Name AUTHORIZATION_FAILURE("AuthorizationFailure");
    const Name TOKEN_SUCCESS("TokenGenerationSuccess");
    const Name TOKEN_FAILURE("TokenGenerationFailure");
}

class GenerateTokenExample
{
    std::string                 d_host;
    int                         d_port;
    std::string                 d_DSProperty;
    bool                        d_useDS;
    std::vector<std::string>    d_securities;
    std::vector<std::string>    d_fields;

    Session            *d_session;
    Identity            d_identity;

    void printUsage()
    {
        std::cout << "Usage:" << std::endl
            << "    Generate a token for authorization " << std::endl
            << "        [-ip        <ipAddress  = localhost>" << std::endl
            << "        [-p         <tcpPort    = 8194>" << std::endl
            << "        [-s         <security   = IBM US Equity>" << std::endl
            << "        [-f         <field      = PX_LAST>" << std::endl
            << "        [-d         <dirSvcProperty = NULL>" << std::endl;
    }

    bool parseCommandLine(int argc, char **argv)
    {
        for (int i = 1; i < argc; ++i) {
            if (!std::strcmp(argv[i],"-ip") && i + 1 < argc)
                d_host = argv[++i];
            else if (!std::strcmp(argv[i],"-p") &&  i + 1 < argc)
                d_port = std::atoi(argv[++i]);
            else if (!std::strcmp(argv[i],"-s") && i + 1 < argc)
                d_securities.push_back(argv[++i]);
            else if (!std::strcmp(argv[i],"-f") && i + 1 < argc)
                d_fields.push_back(argv[++i]);
            else if (!std::strcmp(argv[i],"-d") &&  i + 1 < argc) {
                d_useDS = true;
                d_DSProperty = argv[++i];
            } else {
                printUsage();
                return false;
            }
        }
        // handle default arguments
        if (d_securities.size() == 0) {
            d_securities.push_back("IBM US Equity");
        }

        if (d_fields.size() == 0) {
            d_fields.push_back("PX_LAST");
        }
        return true;
    }

    void sendRequest()
    {
        Service refDataService = d_session->getService("//blp/refdata");
        Request request = refDataService.createRequest("ReferenceDataRequest");

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

        std::cout << "Sending Request: " << request << std::endl;
        d_session->sendRequest(request, d_identity);
    }

    bool processTokenStatus(const Event &event)
    {
        std::cout << "processTokenEvents" << std::endl;
        MessageIterator msgIter(event);
        while (msgIter.next()) {
            Message msg = msgIter.message();
            if (msg.messageType() == TOKEN_SUCCESS) {
                msg.print(std::cout);

                Service authService = d_session->getService("//blp/apiauth");
                Request authRequest = authService.createAuthorizationRequest();
                authRequest.set("token", msg.getElementAsString("token"));

                d_identity = d_session->createIdentity();
                d_session->sendAuthorizationRequest(authRequest, &d_identity,
                    CorrelationId(1));
            } else if (msg.messageType() == TOKEN_FAILURE) {
                msg.print(std::cout);
                return false;
            }
        }

        return true;
    }

    bool processEvent(const Event &event)
    {
        std::cout << "processEvent" << std::endl;
        MessageIterator msgIter(event);
        while (msgIter.next()) {
            Message msg = msgIter.message();
            if (msg.messageType() == AUTHORIZATION_SUCCESS) {
                std::cout << "Authorization SUCCESS" << std::endl;
                sendRequest();
            } else if (msg.messageType() == AUTHORIZATION_FAILURE) {
                std::cout << "Authorization FAILED" << std::endl;
                msg.print(std::cout);
                return false;
            } else {
                msg.print(std::cout);
                if (event.eventType() == Event::RESPONSE) {
                    std::cout << "Got Final Response" << std::endl;
                    return false;
                }
            }
        }
        return true;
    }

public:
    GenerateTokenExample()
        : d_host("localhost"), d_port(8194), d_useDS(false), d_session(0)
    {
    }

    ~GenerateTokenExample()
    {
        if (d_session) {
            d_session->stop();
            delete d_session;
        }
    }

    void run(int argc, char **argv)
    {
        if (!parseCommandLine(argc, argv)) return;

        SessionOptions sessionOptions;
        sessionOptions.setServerHost(d_host.c_str());
        sessionOptions.setServerPort(d_port);

        std::string authOptions = "AuthenticationType=OS_LOGON";
        if (d_useDS) {
            authOptions = "AuthenticationType=DIRECTORY_SERVICE;DirSvcPropertyName=";
            authOptions.append(d_DSProperty);
        }
        std::cout << "authOptions = " << authOptions << std::endl;
        sessionOptions.setAuthenticationOptions(authOptions.c_str());

        std::cout << "Connecting to " <<  d_host << ":" << d_port
            << std::endl;
        d_session = new Session(sessionOptions);
        if (!d_session->start()) {
            std::cerr <<"Failed to start session." << std::endl;
            return;
        }

        if (!d_session->openService("//blp/refdata")) {
            std::cerr << "Failed to open //blp/refdata" << std::endl;
            return;
        }
        if (!d_session->openService("//blp/apiauth")) {
            std::cerr << "Failed to open //blp/apiauth" << std::endl;
            return;
        }

        CorrelationId tokenReqId(99);
        d_session->generateToken(tokenReqId);

        while (true) {
            Event event = d_session->nextEvent();
            if (event.eventType() == Event::TOKEN_STATUS) {
                if (!processTokenStatus(event)) {
                    break;
                }
            } else {
                if (!processEvent(event)) {
                    break;
                }
            }
        }
    }
};

int main(int argc, char **argv)
{
    std::cout << "GenerateTokenExample" << std::endl;
    GenerateTokenExample example;
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
