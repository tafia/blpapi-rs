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
#include <string>
#include <stdlib.h>
#include <string.h>

using namespace BloombergLP;
using namespace blpapi;

namespace {
    const Name SECURITY_DATA("securityData");
    const Name SECURITY("security");
    const Name FIELD_DATA("fieldData");
    const Name FIELD_EXCEPTIONS("fieldExceptions");
    const Name FIELD_ID("fieldId");
    const Name ERROR_INFO("errorInfo");
}

class RefDataTableOverrideExample
{
    std::string         d_host;
    int                 d_port;

    void printUsage()
    {
        std::cout << "Usage:" << std::endl
            << "    Retrieve reference data " << std::endl
            << "        [-ip        <ipAddress  = localhost>" << std::endl
            << "        [-p         <tcpPort    = 8194>" << std::endl;
    }

    bool parseCommandLine(int argc, char **argv)
    {
        for (int i = 1; i < argc; ++i) {
            if (!std::strcmp(argv[i],"-ip") && i + 1 < argc) {
                d_host = argv[++i];
            } else if (!std::strcmp(argv[i],"-p") &&  i + 1 < argc) {
                d_port = std::atoi(argv[++i]);
            } else {
                printUsage();
                return false;
            }
        }
        return true;
    }

    void processMessage(Message &msg)
    {
        Element securityDataArray = msg.getElement(SECURITY_DATA);
        int numSecurities = securityDataArray.numValues();
        for (int i = 0; i < numSecurities; ++i) {
            Element securityData = securityDataArray.getValueAsElement(i);
            std::cout << securityData.getElementAsString(SECURITY)
                      << std::endl;
            const Element fieldData = securityData.getElement(FIELD_DATA);
            for (size_t j = 0; j < fieldData.numElements(); ++j) {
                Element field = fieldData.getElement(j);
                if (!field.isValid()) {
                    std::cout << field.name() <<  " is NULL." << std::endl;
                } else if (field.isArray()) {
                    // The following illustrates how to iterate over complex
                    // data returns.
                    for (int i = 0; i < field.numValues(); ++i) {
                        Element row = field.getValueAsElement(i);
                        std::cout << "Row " << i << ": " << row << std::endl;
                    }
                } else {
                    std::cout << field.name() << " = "
                        << field.getValueAsString() << std::endl;
                }
            }

            Element fieldExceptionArray =
                securityData.getElement(FIELD_EXCEPTIONS);
            for (size_t k = 0; k < fieldExceptionArray.numValues(); ++k) {
                Element fieldException =
                    fieldExceptionArray.getValueAsElement(k);
                std::cout <<
                    fieldException.getElement(ERROR_INFO).getElementAsString(
                    "category")
                    << ": " << fieldException.getElementAsString(FIELD_ID);
            }
            std::cout << std::endl;
        }
    }

  public:

    void run(int argc, char **argv)
    {
        d_host = "localhost";
        d_port = 8194;
        if (!parseCommandLine(argc, argv)) return;

        SessionOptions sessionOptions;
        sessionOptions.setServerHost(d_host.c_str());
        sessionOptions.setServerPort(d_port);

        std::cout << "Connecting to " <<  d_host << ":"
                  << d_port << std::endl;
        Session session(sessionOptions);
        if (!session.start()) {
            std::cerr <<"Failed to start session." << std::endl;
            return;
        }
        if (!session.openService("//blp/refdata")) {
            std::cerr <<"Failed to open //blp/refdata" << std::endl;
            return;
        }

        Service refDataService = session.getService("//blp/refdata");
        Request request = refDataService.createRequest("ReferenceDataRequest");

        // Add securities to request.

        request.append("securities", "CWHL 2006-20 1A1 Mtge");
        // ...

        // Add fields to request. Cash flow is a table (data set) field.

        request.append("fields", "MTG_CASH_FLOW");
        request.append("fields", "SETTLE_DT");

        // Add scalar overrides to request.

        Element overrides = request.getElement("overrides");
        Element override1 = overrides.appendElement();
        override1.setElement("fieldId", "ALLOW_DYNAMIC_CASHFLOW_CALCS");
        override1.setElement("value", "Y");
        Element override2 = overrides.appendElement();
        override2.setElement("fieldId", "LOSS_SEVERITY");
        override2.setElement("value", 31);

        // Add table overrides to request.

        Element tableOverrides = request.getElement("tableOverrides");
        Element tableOverride = tableOverrides.appendElement();
        tableOverride.setElement("fieldId", "DEFAULT_VECTOR");
        Element rows = tableOverride.getElement("row");

        // Layout of input table is specified by the definition of
        // 'DEFAULT_VECTOR'. Attributes are specified in the first rows.
        // Subsequent rows include rate, duration, and transition.

        Element row = rows.appendElement();
        Element cols = row.getElement("value");
        cols.appendValue("Anchor");  // Anchor type
        cols.appendValue("PROJ");    // PROJ = Projected
        row = rows.appendElement();
        cols = row.getElement("value");
        cols.appendValue("Type");    // Type of default
        cols.appendValue("CDR");     // CDR = Conditional Default Rate

        struct RateVector {
            float       rate;
            int         duration;
            const char *transition;
        } rateVectors[] = {
            { 1.0, 12, "S" },  // S = Step
            { 2.0, 12, "R" }   // R = Ramp
        };

        for (int i = 0; i < sizeof(rateVectors)/sizeof(rateVectors[0]); ++i)
        {
            const RateVector& rateVector = rateVectors[i];

            row = rows.appendElement();
            cols = row.getElement("value");
            cols.appendValue(rateVector.rate);
            cols.appendValue(rateVector.duration);
            cols.appendValue(rateVector.transition);
        }

        std::cout << "Sending Request: " << request << std::endl;
        CorrelationId cid(this);
        session.sendRequest(request, cid);

        while (true) {
            Event event = session.nextEvent();
            MessageIterator msgIter(event);
            while (msgIter.next()) {
                Message msg = msgIter.message();
                if (msg.correlationId() == cid) {
                    // Process the response generically.
                    processMessage(msg);
                }
            }
            if (event.eventType() == Event::RESPONSE) {
                break;
            }
        }
    }
};

int main(int argc, char **argv)
{
    std::cout << "RefDataTableOverrideExample" << std::endl;
    RefDataTableOverrideExample example;
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
