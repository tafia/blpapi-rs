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
// blpapi_requesttemplate.h                                           -*-C++-*-
#ifndef INCLUDED_BLPAPI_REQUESTTEMPLATE
#define INCLUDED_BLPAPI_REQUESTTEMPLATE

//@PURPOSE: Provide a handle for a request template.
//
//@CLASSES:
// blpapi::RequestTemplate: an opaque handle to track a request template
//
//@SEE_ALSO: blpapi::Session
//
//@DESCRIPTION: This component defines an opaque handle that represents a
// previously created request template. For example, a request template may be
// created for a snapshot using "Session::createSnapshot". This handle
// can be used for sending a request over a session using
// "Session::sendRequest()".

#ifndef INCLUDED_BLPAPI_CALL
#include <blpapi_call.h>
#endif

#ifndef INCLUDED_BLPAPI_DEFS
#include <blpapi_defs.h>
#endif

#ifndef INCLUDED_BLPAPI_TYPES
#include <blpapi_types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

BLPAPI_EXPORT
int blpapi_RequestTemplate_addRef(
    const blpapi_RequestTemplate_t *requestTemplate);

BLPAPI_EXPORT
int blpapi_RequestTemplate_release(
    const blpapi_RequestTemplate_t *requestTemplate);

#ifdef __cplusplus
}

#include <cstdlib>  // for std::abort

namespace BloombergLP {
namespace blpapi {
                         // =====================
                         // class RequestTemplate
                         // =====================

class RequestTemplate {
    // A handle to a single request template.
    //
    // The 'RequestTemplate' is a handle to a single underlying request
    // template. The underlying request template is reference counted and
    // destroyed when the last 'RequestTemplate' object which references it is
    // destroyed. 'RequestTemplate' objects are copyable, there is no
    // difference which instance of a 'RequestTemplate' is used for sending
    // requests. A copy of an 'RequestTemplate' contains a reference to the
    // same underlying request template as an original object.
    //
    // Request templates may accelerate request processing by caching
    // information required for a response (as it is done by the snapshot
    // service) and eliminating the need to create a new request object every
    // time.
    //
    // Request templates are obtained from a Session and should be always used
    // with the session that creates the template. When a session is
    // terminated, any request templates associated with that session become
    // invalid. Results of sending or canceling of invalid requests templates
    // is undefined.
    //
    // In order to send a request represented by a template, the
    // "Session::sendRequest" method should be called with the
    // 'RequestTemplate' object passed as the first argument.

    blpapi_RequestTemplate_t *d_impl;

  public:
    // CREATORS
    RequestTemplate(blpapi_RequestTemplate_t *handle = 0);
        // Construct a request template handle for the underlying request
        // template specified by optionally specified 'handle'.

    RequestTemplate(const RequestTemplate& original);
        // Construct a request template handle using underlying request
        // template from 'original'. This will add a reference to the
        // underlying request template.

    ~RequestTemplate();
        // Destroy this request template handle. This will also destroy the
        // underlying request template if there no other references to it.

    // MANIUPLATORS
    RequestTemplate& operator=(const RequestTemplate& rhs);
        // Copies the request template handle specified by 'rhs' into the
        // current request template handle. It will increase reference count
        // of the underlying request template of the 'rhs' and decrease
        // reference count (and possibly destroy) of the underlying request
        // template previously referenced by this object.

    // ACCESSORS
    const blpapi_RequestTemplate_t* impl() const;
        // Returns the internal implementation.

    blpapi_RequestTemplate_t* impl();
        // Returns the internal implementation.
};

// ============================================================================
//                      INLINE AND TEMPLATE FUNCTION IMPLEMENTATIONS
// ============================================================================

                            // ---------------------
                            // class RequestTemplate
                            // ---------------------
// CREATORS
inline
RequestTemplate::RequestTemplate(blpapi_RequestTemplate_t *handle)
: d_impl(handle)
{
}

inline
RequestTemplate::RequestTemplate(const RequestTemplate& original)
: d_impl(original.d_impl)
{
    if (d_impl) {
        BLPAPI_CALL(blpapi_RequestTemplate_addRef)(d_impl);
    }
}

inline
RequestTemplate::~RequestTemplate()
{
    if (d_impl) {
        if (BLPAPI_CALL_AVAILABLE(blpapi_RequestTemplate_release)) {
            BLPAPI_CALL_UNCHECKED(blpapi_RequestTemplate_release)(d_impl);
        } else {
            std::abort();
        }
    }
}

// MANIPULATORS
inline
RequestTemplate& RequestTemplate::operator=(const RequestTemplate& rhs)
{

    if (this == &rhs) {
        return *this;
    }

    if (d_impl == rhs.d_impl) {
        return *this;
    }

    if (d_impl) {
        BLPAPI_CALL(blpapi_RequestTemplate_release)(d_impl);
    }

    d_impl = rhs.d_impl;

    if (d_impl) {
        BLPAPI_CALL(blpapi_RequestTemplate_addRef)(d_impl);
    }

    return *this;
}

inline
const blpapi_RequestTemplate_t* RequestTemplate::impl() const
{
    return d_impl;
}

inline
blpapi_RequestTemplate_t* RequestTemplate::impl()
{
    return d_impl;
}

}  // close namespace blpapi
}  // close namespace BloombergLP

#endif // #ifdef __cplusplus
#endif // #ifndef INCLUDED_BLPAPI_REQUESTTEMPLATE
