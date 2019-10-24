/* Copyright 2016. Bloomberg Finance L.P.
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
// blpapi_tlsoptions.h                                                -*-C++-*-
#ifndef INCLUDED_BLPAPI_TLSOPTIONS
#define INCLUDED_BLPAPI_TLSOPTIONS

//@PURPOSE: Maintain client credentials and trust material.
//
//@CLASSES:
//  blpapi::TlsOptions: user specified TLS options.
//
//@SEE_ALSO: blpapi_sessionoptions
//
//@DESCRIPTION: TlsOptions instances maintain client credentials and
// trust material used by a session to establish secure mutually authenticated
// connections to endpoints.
//
// The client credentials comprise an encrypted private key with a client
// certificate. The trust material comprises one or more certificates.
//
// TlsOptions objects are created using the methods
// 'TlsOptions::createFromBlobs' and 'TlsOptions::createFromFiles'; both accept
// the DER encoded client credentials in PKCS#12 format and the DER encoded
// trusted material in PKCS#7 format.
//
//
///Usage
///-----
// The following snippet shows to use the TlsOptions when creating a
// 'SessionOptions'.
//..
// blpapi::TlsOptions tlsOptionsFromFiles
//     = blpapi::TlsOptions::createFromFiles("client",
//                                           "mypassword",
//                                           "trusted");
// tlsOptionsFromFiles.setTlsHandshakeTimeoutMs(123456);
// SessionOptions sessionOptions1;
// sessionOptions1.setTlsOptions(tlsOptionsFromFiles);
//
// std::string credentials  = getCredentials();
// std::string password     = getPassword();
// std::string trustedCerts = getCerts();
// blpapi::TlsOptions tlsOptionsFromBlobs
//     = blpapi::TlsOptions::createFromBlobs(credentials.data(), 
//                                           credentials.size(),
//                                           password.c_str(),
//                                           trustedCerts.data(),
//                                           trustedCerts.size());
// tlsOptionsFromBlobs.setCrlFetchTimeoutMs(234567);
// SessionOptions sessionOptions2;
// sessionOptions2.setTlsOptions(tlsOptionsFromBlobs);
//..

#ifndef INCLUDED_BLPAPI_CALL
#include <blpapi_call.h>
#endif

#ifndef INCLUDED_BLPAPI_DEFS
#include <blpapi_defs.h>
#endif

#ifndef INCLUDED_BLPAPI_EXCEPTION
#include <blpapi_exception.h>
#endif

#ifndef INCLUDED_BLPAPI_TYPES
#include <blpapi_types.h>
#endif

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

BLPAPI_EXPORT
blpapi_TlsOptions_t *blpapi_TlsOptions_create(void);

BLPAPI_EXPORT
blpapi_TlsOptions_t *blpapi_TlsOptions_duplicate(
                                        const blpapi_TlsOptions_t *parameters);

BLPAPI_EXPORT
void blpapi_TlsOptions_copy(blpapi_TlsOptions_t       *lhs,
                            const blpapi_TlsOptions_t *rhs);

BLPAPI_EXPORT
void blpapi_TlsOptions_destroy(blpapi_TlsOptions_t *parameters);

BLPAPI_EXPORT
blpapi_TlsOptions_t *blpapi_TlsOptions_createFromFiles(
                                      const char *clientCredentialsFileName,
                                      const char *clientCredentialsPassword,
                                      const char *trustedCertificatesFileName);

BLPAPI_EXPORT
blpapi_TlsOptions_t *blpapi_TlsOptions_createFromBlobs(
                                 const char *clientCredentialsRawData,
                                 int         clientCredentialsRawDataLength,
                                 const char *clientCredentialsPassword,
                                 const char *trustedCertificatesRawData,
                                 int         trustedCertificatesRawDataLength);

BLPAPI_EXPORT
void blpapi_TlsOptions_setTlsHandshakeTimeoutMs(
                                   blpapi_TlsOptions_t *paramaters,
                                   int                  tlsHandshakeTimeoutMs);

BLPAPI_EXPORT
void blpapi_TlsOptions_setCrlFetchTimeoutMs(
                                       blpapi_TlsOptions_t *paramaters,
                                       int                  crlFetchTimeoutMs);


#ifdef __cplusplus
}

namespace BloombergLP {
namespace blpapi {

                         // ================
                         // class TlsOptions
                         // ================

class TlsOptions {
    // Contains the user specified TLS options.
    //
    // To enable SSL connections, create a TlsOptions object using the
    // methods 'TlsOptions::createFromBlobs' and 'TlsOptions::createFromFiles'.

    blpapi_TlsOptions_t *d_handle_p;

  public:

    TlsOptions();
        // Create a TlsOptions with all TLS options with no
        // certificate information.

    TlsOptions(const TlsOptions&);
        // Copy constructor

    ~TlsOptions();
        // Destroy this TlsOptions.

    // MANIPULATORS

    TlsOptions& operator=(const TlsOptions&);
        // Assign to this object the value of the specified 'rhs' object.

    static TlsOptions createFromFiles(const char *clientCredentialsFileName,
                                      const char *clientCredentialsPassword,
                                      const char *trustedCertificatesFileName);
        // Creates a TlsOptions using a DER encoded client credentials in
        // PKCS#12 format and DER encoded trust material in PKCS#7 format from
        // the specified files.

    static TlsOptions createFromBlobs(
                                 const char *clientCredentialsRawData,
                                 int         clientCredentialsRawDataLength,
                                 const char *clientCredentialsPassword,
                                 const char *trustedCertificatesRawData,
                                 int         trustedCertificatesRawDataLength);
        // Create a TlsOptions using DER encoded client credentials in PKCS#12
        // format and DER encoded trust material in PKCS#7 format from the
        // specified raw data.

    void setTlsHandshakeTimeoutMs(int tlsHandshakeTimeoutMs);
        // Set the TLS handshake timeout to the specified
        // 'tlsHandshakeTimeoutMs'. The default is 10,000 milliseconds.
        // The TLS handshake timeout will be set to the default if
        // the specified 'tlsHandshakeTimeoutMs' is not positive.

    void setCrlFetchTimeoutMs(int crlFetchTimeoutMs);
        // Set the CRL fetch timeout to the specified
        // 'crlFetchTimeoutMs'. The default is 20,000 milliseconds.
        // The TLS handshake timeout will be set to the default if
        // the specified 'crlFetchTimeoutMs' is not positive.

    // ACCESSORS

    blpapi_TlsOptions_t *handle() const;
        // Return the handle of the current TLS options.
};

// ============================================================================
//                      INLINE FUNCTION DEFINITIONS
// ============================================================================

                            // ----------------
                            // class TlsOptions
                            // ----------------
inline
TlsOptions::TlsOptions()
{
    d_handle_p = BLPAPI_CALL(blpapi_TlsOptions_create)();
}

inline
TlsOptions::TlsOptions(const TlsOptions& options)
{
    d_handle_p = BLPAPI_CALL(blpapi_TlsOptions_duplicate)(options.handle());
}

inline
TlsOptions::~TlsOptions()
{
    BLPAPI_CALL(blpapi_TlsOptions_destroy)(d_handle_p);
}

inline
TlsOptions& TlsOptions::operator=(const TlsOptions& rhs)
{
    BLPAPI_CALL(blpapi_TlsOptions_copy)(this->handle(), rhs.handle());
    return *this;
}

inline
TlsOptions TlsOptions::createFromFiles(const char *clientCredentialsFileName,
                                       const char *clientCredentialsPassword,
                                       const char *trustedCertificatesFileName)
{
    TlsOptions tlsOptions;
    tlsOptions.d_handle_p = BLPAPI_CALL(blpapi_TlsOptions_createFromFiles)(
                                                  clientCredentialsFileName,
                                                  clientCredentialsPassword,
                                                  trustedCertificatesFileName);
    return tlsOptions;
}

inline
TlsOptions TlsOptions::createFromBlobs(
                                 const char *clientCredentialsRawData,
                                 int         clientCredentialsRawDataLength,
                                 const char *clientCredentialsPassword,
                                 const char *trustedCertificatesRawData,
                                 int         trustedCertificatesRawDataLength)
{
    TlsOptions tlsOptions;
    tlsOptions.d_handle_p = BLPAPI_CALL(blpapi_TlsOptions_createFromBlobs)(
                                             clientCredentialsRawData,
                                             clientCredentialsRawDataLength,
                                             clientCredentialsPassword,
                                             trustedCertificatesRawData,
                                             trustedCertificatesRawDataLength);
    return tlsOptions;
}

inline
void TlsOptions::setTlsHandshakeTimeoutMs(int tlsHandshakeTimeoutMs)
{
    BLPAPI_CALL(blpapi_TlsOptions_setTlsHandshakeTimeoutMs)(
                                                        d_handle_p, 
                                                        tlsHandshakeTimeoutMs);
}

inline
void TlsOptions::setCrlFetchTimeoutMs(int crlFetchTimeoutMs)
{
    BLPAPI_CALL(blpapi_TlsOptions_setCrlFetchTimeoutMs)(d_handle_p,
                                                      crlFetchTimeoutMs);
}

inline
blpapi_TlsOptions_t *TlsOptions::handle() const
{
    return d_handle_p;
}

}  // close namespace blpapi
}  // close namespace BloombergLP

#endif // #ifdef __cplusplus
#endif // #ifndef INCLUDED_BLPAPI_TLSOPTIONS
