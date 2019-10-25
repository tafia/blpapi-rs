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

using namespace BloombergLP;
using namespace blpapi;

namespace {

    const Name RESPONSE_ERROR("responseError");
    const Name SECURITY_DATA("securityData");
    const Name SECURITY("security");
    const Name EID_DATA("eidData");
    const Name AUTHORIZATION_SUCCESS("AuthorizationSuccess");
    const Name AUTHORIZATION_FAILURE("AuthorizationFailure");

    const char* REFRENCEDATA_REQUEST = "ReferenceDataRequest";
    const char* APIAUTH_SVC          = "//blp/apiauth";
    const char* REFDATA_SVC          = "//blp/refdata";

    void printEvent(const Event &event)
    {
        MessageIterator msgIter(event);
        while (msgIter.next()) {
            Message msg = msgIter.message();
            CorrelationId correlationId = msg.correlationId();
            if (correlationId.asInteger() != 0) {
                std::cout << "Correlator: " << correlationId.asInteger()
                    << std::endl;
            }
            msg.print(std::cout);
            std::cout << std::endl;
        }
    }

} // anonymous namespace

class SessionEventHandler: public  EventHandler
{
    std::vector<Identity> &d_identities;
    std::vector<int> &d_uuids;

    void processResponseEvent(const Event &event)
    {
        MessageIterator msgIter(event);
        while (msgIter.next()) {
            Message msg = msgIter.message();
            if (msg.hasElement(RESPONSE_ERROR)) {
                msg.print(std::cout) << std::endl;
                continue;
            }
            std::cout << "Response for User "
                      << msg.correlationId().asInteger() << ": "
                      << std::endl;
            msg.print(std::cout);
        }
    }

public :

    SessionEventHandler(std::vector<Identity> &identities,
        std::vector<int> &uuids) :
    d_identities(identities), d_uuids(uuids) {
    }

    bool processEvent(const Event &event, Session *session)
    {
        switch(event.eventType()) {
        case Event::SESSION_STATUS:
        case Event::SERVICE_STATUS:
        case Event::REQUEST_STATUS:
        case Event::AUTHORIZATION_STATUS:
            printEvent(event);
            break;

        case Event::RESPONSE:
        case Event::PARTIAL_RESPONSE:
            try {
                processResponseEvent(event);
            }
            catch (Exception &e) {
                std::cerr << "Library Exception!!! " << e.description()
                    << std::endl;
                return true;
            } catch (...) {
                std::cerr << "Unknown Exception!!!" << std::endl;
                return true;
            }
            break;
        }
        return true;
    }
};

class UserModeExample {

    std::string               d_host;
    int                       d_port;
    std::vector<std::string>  d_securities;
    std::vector<int>          d_uuids;
    std::vector<Identity>     d_identities;
    std::vector<std::string>  d_programAddresses;

    void printUsage()
    {
        std::cout
            << "Usage:" << '\n'
            << "    UserMode Example" << '\n'
            << "        [-s     <security   = IBM US Equity>]" << '\n'
            << "        [-c     <credential uuid:ipAddress" <<
            " eg:12345:10.20.30.40>]" << '\n'
            << "        [-ip    <ipAddress  = localhost>]" << '\n'
            << "        [-p     <tcpPort    = 8194>]" << '\n'
            << "Note:" << '\n'
            <<"Multiple securities and credentials can be" <<
            " specified." << std::endl;
    }

    void openServices(Session *session)
    {
        if (!session->openService(APIAUTH_SVC)) {
            std::cout << "Failed to open service: " << APIAUTH_SVC
                << std::endl;
            std::exit(-1);
        }

        if (!session->openService(REFDATA_SVC)) {
            std::cout << "Failed to open service: " << REFDATA_SVC
                << std::endl;
            std::exit(-2);
        }
    }

    bool authorizeUsers(EventQueue *authQueue, Session *session)
    {
        Service authService = session->getService(APIAUTH_SVC);
        bool is_any_user_authorized = false;

        // Authorize each of the users
        d_identities.reserve(d_uuids.size());
        for (size_t i = 0; i < d_uuids.size(); ++i) {
            d_identities.push_back(session->createIdentity());
            Request authRequest = authService.createAuthorizationRequest();
            authRequest.set("uuid", d_uuids[i]);
            authRequest.set("ipAddress", d_programAddresses[i].c_str());

            session->sendAuthorizationRequest(authRequest,
                                              &d_identities[i],
                                              CorrelationId(i),
                                              authQueue);

            Event event = authQueue->nextEvent();

            if (event.eventType() == Event::RESPONSE ||
                event.eventType() == Event::PARTIAL_RESPONSE ||
                event.eventType() == Event::REQUEST_STATUS ||
                event.eventType() == Event::AUTHORIZATION_STATUS) {

                MessageIterator msgIter(event);
                while (msgIter.next()) {
                    Message msg = msgIter.message();

                    if (msg.messageType() == AUTHORIZATION_SUCCESS) {
                        std::cout << d_uuids[msg.correlationId().asInteger()]
                            << " authorization success"
                            << std::endl;
                        is_any_user_authorized = true;
                    }
                    else if (msg.messageType() == AUTHORIZATION_FAILURE) {
                        std::cout << d_uuids[msg.correlationId().asInteger()]
                            << " authorization failed"
                            << std::endl;
                        std::cout << msg << std::endl;
                    }
                    else {
                        std::cout << msg << std::endl;
                    }
                }
            }
        }
        return is_any_user_authorized;
    }

    void sendRefDataRequest(Session *session)
    {
        Service service = session->getService(REFDATA_SVC);
        Request request = service.createRequest(REFRENCEDATA_REQUEST);

        // Add securities.
        Element securities = request.getElement("securities");
        for (size_t i = 0; i < d_securities.size(); ++i) {
            securities.appendValue(d_securities[i].c_str());
        }

        // Add fields
        Element fields = request.getElement("fields");
        fields.appendValue("PX_LAST");
        fields.appendValue("LAST_UPDATE");

        request.set("returnEids", true);

        for (size_t j = 0; j < d_identities.size(); ++j) {
            int uuid = d_uuids[j];
            std::cout << "Sending RefDataRequest for User "
                      << uuid << std::endl;
            session->sendRequest(request,
                                 d_identities[j],
                                 CorrelationId(uuid));
        }
    }

    bool parseCommandLine(int argc, char **argv)
    {
        for (int i = 1; i < argc; ++i) {
            if (!std::strcmp(argv[i],"-s") ) {
                if (++i >= argc) return false;
                d_securities.push_back(argv[i]);
            }
            else if (!std::strcmp(argv[i],"-c")) {
                if (++i >= argc) return false;

                std::string credential = argv[i];
                size_t idx = credential.find_first_of(':');
                if (idx == std::string::npos) return false;
                d_uuids.push_back(atoi(credential.substr(0,idx).c_str()));
                d_programAddresses.push_back(credential.substr(idx+1));
                continue;
            }
            else if (!std::strcmp(argv[i],"-ip")) {
                if (++i >= argc) return false;
                d_host = argv[i];
                continue;
            }
            else if (!std::strcmp(argv[i],"-p")) {
                if (++i >= argc) return false;
                d_port = std::atoi(argv[i]);
                continue;
            }
            else return false;
        }

        if (d_uuids.size() <= 0) {
            std::cout << "No uuids were specified" << std::endl;
            return false;
        }

        if (d_uuids.size() != d_programAddresses.size()) {
            std::cout << "Invalid number of program addresses provided"
                << std::endl;
            return false;
        }

        if (d_securities.size() <= 0) {
            d_securities.push_back("IBM US Equity");
        }

        return true;
    }



public:

    UserModeExample()
    : d_host("localhost")
    , d_port(8194)
    {
    }

    void run(int argc, char **argv) {
        if (!parseCommandLine(argc, argv)) {
            printUsage();
            return;
        }

        SessionOptions sessionOptions;
        sessionOptions.setServerHost(d_host.c_str());
        sessionOptions.setServerPort(d_port);

        std::cout << "Connecting to " + d_host + ":" << d_port << std::endl;

        SessionEventHandler eventHandler(d_identities, d_uuids);
        Session session(sessionOptions, &eventHandler);

        if (!session.start()) {
            std::cerr << "Failed to start session. Exiting..." << std::endl;
            std::exit(-1);
        }

        openServices(&session);

        EventQueue authQueue;

        // Authorize all the users that are interested in receiving data
        if (authorizeUsers(&authQueue, &session)) {
            // Make the various requests that we need to make
            sendRefDataRequest(&session);
        }

        // wait for enter key to exit application
        char dummy[2];
        std::cin.getline(dummy,2);

        {
            // Check if there were any authorization events received on the
            // 'authQueue'

            Event event;
            while (0 == authQueue.tryNextEvent(&event)) {
                printEvent(event);
            }
        }

        session.stop();
        std::cout << "Exiting...\n";
    }

};

int main(int argc, char **argv)
{
    std::cout << "UserModeExample" << std::endl;
    UserModeExample example;
    try {
        example.run(argc, argv);
    } catch (Exception &e) {
        std::cerr << "Library Exception!!! " << e.description()
                  << std::endl;
    } catch (...) {
        std::cerr << "Unknown Exception!!!" << std::endl;
    }

    // wait for enter key to exit application
    std::cout << "Press ENTER to quit" << std::endl;
    char dummy[2];
    std::cin.getline(dummy, 2);

    return 0;
}

