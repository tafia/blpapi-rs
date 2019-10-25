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
#include <blpapi_element.h>
#include <blpapi_event.h>
#include <blpapi_eventdispatcher.h>
#include <blpapi_exception.h>
#include <blpapi_logging.h>
#include <blpapi_message.h>
#include <blpapi_name.h>
#include <blpapi_request.h>
#include <blpapi_session.h>
#include <blpapi_subscriptionlist.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <stdlib.h>
#include <string.h>

using namespace BloombergLP;
using namespace blpapi;
namespace {
    const Name SECURITY_DATA("securityData");
    const Name SECURITY("security");
    const Name FIELD_DATA("fieldData");
    const Name RESPONSE_ERROR("responseError");
    const Name SECURITY_ERROR("securityError");
    const Name FIELD_EXCEPTIONS("fieldExceptions");
    const Name FIELD_ID("fieldId");
    const Name ERROR_INFO("errorInfo");
    const Name CATEGORY("category");
    const Name MESSAGE("message");
    const Name REASON("reason");
    const Name SESSION_TERMINATED("SessionTerminated");
    const Name SESSION_STARTUP_FAILURE("SessionStartupFailure");
};

extern "C" {
    void loggingCallback(blpapi_UInt64_t    threadId,
                         int                severity,
                         blpapi_Datetime_t  timestamp,
                         const char        *category,
                         const char        *message);
}

void loggingCallback(blpapi_UInt64_t    threadId,
                     int                severity,
                     blpapi_Datetime_t  timestamp,
                     const char        *category,
                     const char        *message)
{
    std::string severityString;
    switch(severity) {
    // The following cases will not happen if callback registered at OFF
    case blpapi_Logging_SEVERITY_FATAL:
    {
        severityString = "FATAL";
    } break;
    // The following cases will not happen if callback registered at FATAL
    case blpapi_Logging_SEVERITY_ERROR:
    {
        severityString = "ERROR";
    } break;
    // The following cases will not happen if callback registered at ERROR
    case blpapi_Logging_SEVERITY_WARN:
    {
        severityString = "WARN";
    } break;
    // The following cases will not happen if callback registered at WARN
    case blpapi_Logging_SEVERITY_INFO:
    {
        severityString = "INFO";
    } break;
    // The following cases will not happen if callback registered at INFO
    case blpapi_Logging_SEVERITY_DEBUG:
    {
        severityString = "DEBUG";
    } break;
    // The following case will not happen if callback registered at DEBUG
    case blpapi_Logging_SEVERITY_TRACE:
    {
        severityString = "TRACE";
    } break;

    };
    std::stringstream sstream;
    sstream << category <<" [" << severityString << "] Thread ID = "
            << threadId << ": " << message << std::endl;
    std::cout << sstream.str() << std::endl;;
}

class RefDataExample
{
    std::string              d_host;
    int                      d_port;
    std::vector<std::string> d_securities;
    std::vector<std::string> d_fields;

    bool parseCommandLine(int argc, char **argv)
    {
        int verbosityCount = 0;
        for (int i = 1; i < argc; ++i) {
            if (!std::strcmp(argv[i],"-s") && i + 1 < argc) {
                d_securities.push_back(argv[++i]);
            } else if (!std::strcmp(argv[i],"-f") && i + 1 < argc) {
                d_fields.push_back(argv[++i]);
            } else if (!std::strcmp(argv[i],"-ip") && i + 1 < argc) {
                d_host = argv[++i];
            } else if (!std::strcmp(argv[i],"-p") &&  i + 1 < argc) {
                d_port = std::atoi(argv[++i]);
            } else if (!std::strcmp(argv[i],"-v")) {
                ++verbosityCount;

            } else {
                printUsage();
                return false;
            }
        }
        if(verbosityCount) {
            registerCallback(verbosityCount);
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

    void printErrorInfo(const char *leadingStr, const Element &errorInfo)
    {
        std::cout << leadingStr
            << errorInfo.getElementAsString(CATEGORY)
            << " ("
            << errorInfo.getElementAsString(MESSAGE)
            << ")" << std::endl;
    }

    void printUsage()
    {
        std::cout << "Usage:" << std::endl
            << "    Retrieve reference data " << std::endl
            << "        [-s         <security   = IBM US Equity>" << std::endl
            << "        [-f         <field      = PX_LAST>" << std::endl
            << "        [-ip        <ipAddress  = localhost>" << std::endl
            << "        [-p         <tcpPort    = 8194>" << std::endl
            << "        [-v         increase verbosity"
            <<                    " (can be specified more than once)"
            << std::endl;
    }


    void registerCallback(int verbosityCount)
    {
        blpapi_Logging_Severity_t severity = blpapi_Logging_SEVERITY_OFF;
        switch(verbosityCount) {
          case 1: {
              severity = blpapi_Logging_SEVERITY_INFO;
          }break;
          case 2: {
              severity = blpapi_Logging_SEVERITY_DEBUG;
          }break;
          default: {
              severity = blpapi_Logging_SEVERITY_TRACE;
          }
        };
        blpapi_Logging_registerCallback(loggingCallback,
                                        severity);
    }

    void sendRefDataRequest(Session &session)
    {
        Service refDataService = session.getService("//blp/refdata");
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
        session.sendRequest(request);
    }
    // return true if processing is completed, false otherwise
    void processResponseEvent(Event event)
    {
        MessageIterator msgIter(event);
        while (msgIter.next()) {
            Message msg = msgIter.message();
            if (msg.asElement().hasElement(RESPONSE_ERROR)) {
                printErrorInfo("REQUEST FAILED: ",
                    msg.getElement(RESPONSE_ERROR));
                continue;
            }

            Element securities = msg.getElement(SECURITY_DATA);
            size_t numSecurities = securities.numValues();
            std::cout << "Processing " << (unsigned int)numSecurities
                      << " securities:"<< std::endl;
            for (size_t i = 0; i < numSecurities; ++i) {
                Element security = securities.getValueAsElement(i);
                std::string ticker = security.getElementAsString(SECURITY);
                std::cout << "\nTicker: " + ticker << std::endl;
                if (security.hasElement(SECURITY_ERROR)) {
                    printErrorInfo("\tSECURITY FAILED: ",
                        security.getElement(SECURITY_ERROR));
                    continue;
                }

                if (security.hasElement(FIELD_DATA)) {
                    const Element fields = security.getElement(FIELD_DATA);
                    if (fields.numElements() > 0) {
                        std::cout << "FIELD\t\tVALUE"<<std::endl;
                        std::cout << "-----\t\t-----"<< std::endl;
                        size_t numElements = fields.numElements();
                        for (size_t j = 0; j < numElements; ++j) {
                            Element field = fields.getElement(j);
                            std::cout << field.name() << "\t\t" <<
                                field.getValueAsString() << std::endl;
                        }
                    }
                }
                std::cout << std::endl;
                Element fieldExceptions = security.getElement(
                                                             FIELD_EXCEPTIONS);
                if (fieldExceptions.numValues() > 0) {
                    std::cout << "FIELD\t\tEXCEPTION" << std::endl;
                    std::cout << "-----\t\t---------" << std::endl;
                    for (size_t k = 0; k < fieldExceptions.numValues(); ++k) {
                        Element fieldException =
                            fieldExceptions.getValueAsElement(k);
                        Element errInfo = fieldException.getElement(
                                                                   ERROR_INFO);
                        std::cout
                                 << fieldException.getElementAsString(FIELD_ID)
                                 << "\t\t"
                                 << errInfo.getElementAsString(CATEGORY)
                                 << " ( "
                                 << errInfo.getElementAsString(MESSAGE)
                                 << ")"
                                 << std::endl;
                    }
                }
            }
        }
    }

    void eventLoop(Session &session)
    {
        bool done = false;
        while (!done) {
            Event event = session.nextEvent();
            if (event.eventType() == Event::PARTIAL_RESPONSE) {
                std::cout << "Processing Partial Response" << std::endl;
                processResponseEvent(event);
            }
            else if (event.eventType() == Event::RESPONSE) {
                std::cout << "Processing Response" << std::endl;
                processResponseEvent(event);
                done = true;
            } else {
                MessageIterator msgIter(event);
                while (msgIter.next()) {
                    Message msg = msgIter.message();
                    if (event.eventType() == Event::REQUEST_STATUS) {
                        std::cout << "REQUEST FAILED: " << msg.getElement(REASON) << std::endl;
                        done = true;
                    }
                    else if (event.eventType() == Event::SESSION_STATUS) {
                        if (msg.messageType() == SESSION_TERMINATED ||
                            msg.messageType() == SESSION_STARTUP_FAILURE) {
                            done = true;
                        }
                    }
                }
            }
        }
    }

public:
    RefDataExample()
    {
        d_host = "localhost";
        d_port = 8194;
    }

    ~RefDataExample()
    {
    }

    void run(int argc, char **argv)
    {
        if (!parseCommandLine(argc, argv)) return;

        SessionOptions sessionOptions;
        sessionOptions.setServerHost(d_host.c_str());
        sessionOptions.setServerPort(d_port);

        std::cout << "Connecting to " + d_host + ":" << d_port << std::endl;
        Session session(sessionOptions);
        if (!session.start()) {
            std::cout << "Failed to start session." << std::endl;
            return;
        }
        if (!session.openService("//blp/refdata")) {
            std::cout << "Failed to open //blp/refdata" << std::endl;
            return;
        }
        sendRefDataRequest(session);

        // wait for events from session.
        try {
            eventLoop(session);
        } catch (Exception &e) {
            std::cerr << "Library Exception !!!"
                      << e.description()
                      << std::endl;
        } catch (...) {
            std::cerr << "Unknown Exception !!!" << std::endl;
        }


        session.stop();
    }
};

int main(int argc, char **argv)
{
    std::cout << "RefDataExample" << std::endl;
    RefDataExample example;
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
