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
    const Name FIELD_ID("id");
    const Name FIELD_MNEMONIC("mnemonic");
    const Name FIELD_DATA("fieldData");
    const Name FIELD_DESC("description");
    const Name FIELD_INFO("fieldInfo");
    const Name FIELD_ERROR("fieldError");
    const Name FIELD_MSG("message");
};

class SimpleFieldInfoExample
{

    int ID_LEN;
    int MNEMONIC_LEN;
    int DESC_LEN;
    std::string PADDING;
    std::string APIFLDS_SVC;
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
                continue;
            }
            if (!std::strcmp(argv[i],"-p") &&  i + 1 < argc) {
                d_port = std::atoi(argv[++i]);
                continue;
            }
            printUsage();
            return false;
        }
        return true;
    }

    std::string padString(std::string str, unsigned int width)
    {
        if (str.length() >= width || str.length() >= PADDING.length() ) return str;
        else return str + PADDING.substr(0, width-str.length());
    }

    void printField (const Element &field)
    {
        std::string  fldId = field.getElementAsString(FIELD_ID);
        if (field.hasElement(FIELD_INFO)) {
            Element fldInfo          = field.getElement (FIELD_INFO) ;
            std::string  fldMnemonic =
                fldInfo.getElementAsString(FIELD_MNEMONIC);
            std::string  fldDesc     =
                fldInfo.getElementAsString(FIELD_DESC);

            std::cout << padString(fldId, ID_LEN)
                << padString(fldMnemonic, MNEMONIC_LEN)
                << padString(fldDesc, DESC_LEN) << std::endl;
        }
        else {
            Element fldError = field.getElement(FIELD_ERROR) ;
            std::string  errorMsg = fldError.getElementAsString(FIELD_MSG) ;

            std::cout << std::endl << " ERROR: " << fldId << " - "
                << errorMsg << std::endl ;
        }
    }
    void printHeader ()
    {
        std::cout << padString("FIELD ID", ID_LEN) +
            padString("MNEMONIC", MNEMONIC_LEN) +
            padString("DESCRIPTION", DESC_LEN)
            << std::endl;
        std::cout << padString("-----------", ID_LEN) +
            padString("-----------", MNEMONIC_LEN) +
            padString("-----------", DESC_LEN)
            << std::endl;
    }


public:
    SimpleFieldInfoExample():
      PADDING("                                            "),
          APIFLDS_SVC("//blp/apiflds") {
              ID_LEN         = 13;
              MNEMONIC_LEN   = 36;
              DESC_LEN       = 40;
      }

      void run(int argc, char **argv)
      {
          d_host = "localhost";
          d_port = 8194;
          if (!parseCommandLine(argc, argv)) return;

          SessionOptions sessionOptions;
          sessionOptions.setServerHost(d_host.c_str());
          sessionOptions.setServerPort(d_port);

          std::cout << "Connecting to " <<  d_host << ":" << d_port << std::endl;
          Session session(sessionOptions);
          if (!session.start()) {
              std::cerr <<"Failed to start session." << std::endl;
              return;
          }
          if (!session.openService(APIFLDS_SVC.c_str())) {
              std::cerr <<"Failed to open " << APIFLDS_SVC << std::endl;
              return;
          }

          Service fieldInfoService = session.getService(APIFLDS_SVC.c_str());
          Request request = fieldInfoService.createRequest("FieldInfoRequest");
          request.append("id", "LAST_PRICE");
          request.append("id", "pq005");
          request.append("id", "zz0002");

          request.set("returnFieldDocumentation", true);

          std::cout << "Sending Request: " << request << std::endl;
          session.sendRequest(request, CorrelationId(this));

          while (true) {
              Event event = session.nextEvent();
              if (event.eventType() != Event::RESPONSE &&
                  event.eventType() != Event::PARTIAL_RESPONSE) {
                      continue;
              }

              MessageIterator msgIter(event);
              while (msgIter.next()) {
                  Message msg = msgIter.message();
                  Element fields = msg.getElement("fieldData");
                  int numElements = fields.numValues();
                  printHeader();
                  for (int i=0; i < numElements; i++) {
                      printField (fields.getValueAsElement(i));
                  }
                  std::cout << std::endl;
              }
              if (event.eventType() == Event::RESPONSE) {
                  break;
              }
          }
      }
};

int main(int argc, char **argv){
    SimpleFieldInfoExample example;
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
