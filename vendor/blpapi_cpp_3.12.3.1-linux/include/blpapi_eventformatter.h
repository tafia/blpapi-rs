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
// blpapi_eventformatter.h                                            -*-C++-*-
#ifndef INCLUDED_BLPAPI_EVENTFORMATTER
#define INCLUDED_BLPAPI_EVENTFORMATTER

//@PURPOSE: Add messages to an Event for publishing
//
//@CLASSES:
// blpapi::EventFormatter: A Mechanism to add information to an Event.
//
//@DESCRIPTION: This component adds messages to an Event which can be
// later published.

#ifndef INCLUDED_BLPAPI_CALL
#include <blpapi_call.h>
#endif

#ifndef INCLUDED_BLPAPI_DEFS
#include <blpapi_defs.h>
#endif

#ifndef INCLUDED_BLPAPI_EVENT
#include <blpapi_event.h>
#endif

#ifndef INCLUDED_BLPAPI_TOPIC
#include <blpapi_topic.h>
#endif

#ifndef INCLUDED_BLPAPI_TYPES
#include <blpapi_types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

BLPAPI_EXPORT
blpapi_EventFormatter_t *blpapi_EventFormatter_create(blpapi_Event_t *event);

BLPAPI_EXPORT
void blpapi_EventFormatter_destroy(blpapi_EventFormatter_t *victim);

BLPAPI_EXPORT
int blpapi_EventFormatter_appendMessage(blpapi_EventFormatter_t *formatter,
                                        const char              *typeString,
                                        blpapi_Name_t           *typeName,
                                        const blpapi_Topic_t    *topic);

BLPAPI_EXPORT
int blpapi_EventFormatter_appendMessageSeq(
                                      blpapi_EventFormatter_t *formatter,
                                      const char              *typeString,
                                      blpapi_Name_t           *typeName,
                                      const blpapi_Topic_t    *topic,
                                      unsigned int             sequenceNumber,
                                      unsigned                 int);

BLPAPI_EXPORT
int blpapi_EventFormatter_appendResponse(blpapi_EventFormatter_t *formatter,
                                         const char              *typeString,
                                         blpapi_Name_t           *typeName);

BLPAPI_EXPORT
int blpapi_EventFormatter_appendRecapMessage(
                                      blpapi_EventFormatter_t      *formatter,
                                      const blpapi_Topic_t         *topic,
                                      const blpapi_CorrelationId_t *cid);

BLPAPI_EXPORT
int blpapi_EventFormatter_appendRecapMessageSeq(
                                 blpapi_EventFormatter_t      *formatter,
                                 const blpapi_Topic_t         *topic,
                                 const blpapi_CorrelationId_t *cid,
                                 unsigned int                  sequenceNumber,
                                 unsigned                      int);

BLPAPI_EXPORT
int blpapi_EventFormatter_appendFragmentedRecapMessage(
                                   blpapi_EventFormatter_t      *formatter,
                                   const char                   *typeString,
                                   blpapi_Name_t                *typeName,
                                   const blpapi_Topic_t         *topic,
                                   const blpapi_CorrelationId_t *cid,
                                   int                           fragmentType);

BLPAPI_EXPORT
int blpapi_EventFormatter_appendFragmentedRecapMessageSeq(
                                 blpapi_EventFormatter_t      *formatter,
                                 const char                   *typeString,
                                 blpapi_Name_t                *typeName,
                                 const blpapi_Topic_t         *topic,
                                 int                           fragmentType,
                                 unsigned int                  sequenceNumber);

BLPAPI_EXPORT
int blpapi_EventFormatter_setValueBool(blpapi_EventFormatter_t *formatter,
                                       const char              *typeString,
                                       const blpapi_Name_t     *typeName,
                                       blpapi_Bool_t            value);


BLPAPI_EXPORT
int blpapi_EventFormatter_setValueChar(blpapi_EventFormatter_t *formatter,
                                       const char              *typeString,
                                       const blpapi_Name_t     *typeName,
                                       char                     value);

BLPAPI_EXPORT
int blpapi_EventFormatter_setValueInt32(blpapi_EventFormatter_t *formatter,
                                        const char              *typeString,
                                        const blpapi_Name_t     *typeName,
                                        blpapi_Int32_t           value);

BLPAPI_EXPORT
int blpapi_EventFormatter_setValueInt64(blpapi_EventFormatter_t *formatter,
                                        const char              *typeString,
                                        const blpapi_Name_t     *typeName,
                                        blpapi_Int64_t           value);

BLPAPI_EXPORT
int blpapi_EventFormatter_setValueFloat32(blpapi_EventFormatter_t *formatter,
                                          const char              *typeString,
                                          const blpapi_Name_t     *typeName,
                                          blpapi_Float32_t         value);

BLPAPI_EXPORT
int blpapi_EventFormatter_setValueFloat64(blpapi_EventFormatter_t *formatter,
                                          const char              *typeString,
                                          const blpapi_Name_t     *typeName,
                                          blpapi_Float64_t         value);

BLPAPI_EXPORT
int blpapi_EventFormatter_setValueDatetime(
                                          blpapi_EventFormatter_t *formatter,
                                          const char              *typeString,
                                          const blpapi_Name_t     *typeName,
                                          const blpapi_Datetime_t *value);

BLPAPI_EXPORT
int blpapi_EventFormatter_setValueHighPrecisionDatetime(
                              blpapi_EventFormatter_t              *formatter,
                              const char                           *typeString,
                              const blpapi_Name_t                  *typeName,
                              const blpapi_HighPrecisionDatetime_t *value);

BLPAPI_EXPORT
int blpapi_EventFormatter_setValueString(blpapi_EventFormatter_t *formatter,
                                         const char              *typeString,
                                         const blpapi_Name_t     *typeName,
                                         const char              *value);

BLPAPI_EXPORT
int blpapi_EventFormatter_setValueFromName(
                                          blpapi_EventFormatter_t *formatter,
                                          const char              *typeString,
                                          const blpapi_Name_t     *typeName,
                                          const blpapi_Name_t     *value);

BLPAPI_EXPORT
int blpapi_EventFormatter_setValueNull(blpapi_EventFormatter_t *formatter,
                                       const char              *typeString,
                                       const blpapi_Name_t     *typeName);

BLPAPI_EXPORT
int blpapi_EventFormatter_pushElement(blpapi_EventFormatter_t *formatter,
                                      const char              *typeString,
                                      const blpapi_Name_t     *typeName);

BLPAPI_EXPORT
int blpapi_EventFormatter_popElement(blpapi_EventFormatter_t *formatter);


BLPAPI_EXPORT
int blpapi_EventFormatter_appendValueBool(blpapi_EventFormatter_t *formatter,
                                          blpapi_Bool_t            value);

BLPAPI_EXPORT
int blpapi_EventFormatter_appendValueChar(blpapi_EventFormatter_t *formatter,
                                          char                     value);

BLPAPI_EXPORT
int blpapi_EventFormatter_appendValueInt32(blpapi_EventFormatter_t *formatter,
                                           blpapi_Int32_t           value);

BLPAPI_EXPORT
int blpapi_EventFormatter_appendValueInt64(blpapi_EventFormatter_t *formatter,
                                           blpapi_Int64_t           value);

BLPAPI_EXPORT
int blpapi_EventFormatter_appendValueFloat32(
                                           blpapi_EventFormatter_t *formatter,
                                           blpapi_Float32_t         value);

BLPAPI_EXPORT
int blpapi_EventFormatter_appendValueFloat64(
                                           blpapi_EventFormatter_t *formatter,
                                           blpapi_Float64_t         value);

BLPAPI_EXPORT
int blpapi_EventFormatter_appendValueDatetime(
                                           blpapi_EventFormatter_t *formatter,
                                           const blpapi_Datetime_t *value);

BLPAPI_EXPORT
int blpapi_EventFormatter_appendValueHighPrecisionDatetime(
                               blpapi_EventFormatter_t              *formatter,
                               const blpapi_HighPrecisionDatetime_t *value);

BLPAPI_EXPORT
int blpapi_EventFormatter_appendValueString(
                                           blpapi_EventFormatter_t *formatter,
                                           const char              *value);

BLPAPI_EXPORT
int blpapi_EventFormatter_appendValueFromName(
                                           blpapi_EventFormatter_t *formatter,
                                           const blpapi_Name_t     *value);

BLPAPI_EXPORT
int blpapi_EventFormatter_appendElement(blpapi_EventFormatter_t *formatter);

#ifdef __cplusplus
}

namespace BloombergLP {
namespace blpapi {
                         // ====================
                         // class EventFormatter
                         // ====================

class EventFormatter {
    // EventFormatter is used to populate 'Event's for publishing.
    //
    // An EventFormatter is created from an Event obtained from
    // createPublishEvent() on Service. Once the Message or Messages have been
    // appended to the Event using the EventFormatter the Event can be
    // published using publish() on the ProviderSession.
    //
    // EventFormatter objects cannot be copied or assigned so as to ensure
    // there is no ambiguity about what happens if two 'EventFormatter's are
    // both formatting the same 'Event'.
    //
    // The EventFormatter supports appending message of the same type multiple
    // time in the same 'Event'. However the 'EventFormatter' supports write
    // once only to each field. It is an error to call setElement() or
    // pushElement() for the same name more than once at a particular level of
    // the schema when creating a message.
    //
    // The EventFormatter supports appending recap messages with an
    // user-provided 'messageType'. If the user doesn't specify the
    // 'messageType' for the recap message, then the type is determined by the
    // service schema, with the 'FORCED_RECAP_TICK_TYPE' message used for
    // un-solicited recap messages, and 'RECAP_TICK_TYPE' used for solicited
    // recap messages.
    // In addition to that, services can send recaps in more than one tick
    // through the 'fragmentType' flag for recap messages.
    // Recap messages with 'fragmentType' other than 'Message::FRAGMENT_NONE'
    // must send message fragments in order. A recap message with
    // 'Message::FRAGMENT_START' may be followed by zero or more messages
    // with 'fragmentType' 'Message::FRAGMENT_INTERMEDIATE'.
    // Publishers must also publish a recap message with
    // 'Message::FRAGMENT_END' for each recap that was started with
    // 'Message::FRAGMENT_START'. Trying to publish recap messages with flags
    // in a different order is undefined behavior.

    blpapi_EventFormatter_t *d_handle;

  private:
    // NOT IMPLEMENTED
    EventFormatter& operator=(const EventFormatter&);
    EventFormatter(const EventFormatter&);
    EventFormatter();

  public:

    // CREATORS

    EventFormatter(Event& event);
        // Create an EventFormatter to create Messages in the
        // specified 'Event'. An Event may only be reference by one
        // EventFormatter at any time. Attempting to create a second
        // EventFormatter referencing the same Event will result in an
        // exception being thrown.

    ~EventFormatter();
        // Destroy this EventFormatter object.

    // MANIPULATORS

    void appendMessage(const char *messageType, const Topic& topic);
        // Append an (empty) message of the specified 'messageType'
        // that will be published under the specified 'topic' to the
        // Event referenced by this EventFormatter. After a message
        // has been appended its elements can be set using the various
        // setElement() methods.

    void appendMessage(const Name& messageType, const Topic& topic);
        // Append an (empty) message of the specified 'messageType'
        // that will be published under the specified 'topic' to the
        // Event referenced by this EventFormatter. After a message
        // has been appended its elements can be set using the various
        // setElement() methods.

    void appendMessage(const char   *messageType,
                       const Topic&  topic,
                       unsigned int  sequenceNumber);
        // Append an (empty) message of the specified 'messageType'
        // that will be published under the specified 'topic' with the
        // specified 'sequenceNumber' to the Event referenced by this
        // EventFormatter. It is expected that 'sequenceNumber' is
        // greater (unless the value wrapped) than the last value used in
        // any previous message on this 'topic', otherwise the behavior
        // is undefined.
        // After a message has been appended its elements
        // can be set using the various setElement() methods.

    void appendMessage(const Name&  messageType,
                       const Topic& topic,
                       unsigned int sequenceNumber);
        // Append an (empty) message of the specified 'messageType'
        // that will be published under the specified 'topic' with the
        // specified 'sequenceNumber' to the Event referenced by this
        // EventFormatter. It is expected that 'sequenceNumber' is
        // greater (unless the value wrapped) than the last value used in
        // any previous message on this 'topic', otherwise the behavior
        // is undefined.
        // After a message has been appended its elements
        // can be set using the various setElement() methods.

    void appendResponse(const char *opType);
        // Append an (empty) response message of the specified 'opType'
        // that will be sent in response to previously received
        // operation request. After a message has been appended its
        // elements can be set using the various setElement() methods.
        // Only one response can be appended.

    void appendResponse(const Name& opType);
        // Append an (empty) response message of the specified 'opType'
        // that will be sent in response to previously received
        // operation request. After a message has been appended its
        // elements can be set using the various setElement() methods.
        // Only one response can be appended.

    void appendRecapMessage(const Topic& topic, const CorrelationId *cid = 0);
        // DEPRECATED
        // Append an empty recap message with the default 'messageType' that
        // will be published under the specified 'topic' to the
        // event referenced by this 'EventFormatter'. Specify the optional
        // 'CorrelationId' pointer 'cid' if this recap message is added in
        // response to a 'TOPIC_RECAP' message. It is undefined behavior to
        // call this function with a non-null 'cid' for a TOPIC_RECAP message
        // with a different 'Topic' than that referenced by 'topic'. This
        // method will create a recap message with fragment type
        // 'FRAGMENT_NONE'. After a message has been appended its elements can
        // be set using the various setElement() methods. It is an error to
        // append a recap message to an Admin event. This function overload is
        // deprecated for non-null 'cid'. Utilize the overload
        // 'appendRecapMessage(const CorrelationId& cid,...)' to respond to
        // TOPIC_RECAP messages.

    void appendRecapMessage(const Topic&      topic,
                            Message::Fragment fragmentType);
        // Append an empty recap message with the default 'messageType' that
        // will be published under the specified 'topic' to the event
        // referenced by this 'EventFormatter'. The specified 'fragmentType'
        // determines what fragment flag to apply to this recap message.
        // Single-tick recap messages should have 'fragmentType'= Message::NONE
        // Multi-tick recaps can have either Message::START,
        // Message::INTERMEDIATE or Message::END as the 'fragmentType'. After a
        // message has been appended its elements can be set using the various
        // setElement() methods. It is an error to append a recap message to an
        // Admin event. Note that it is undefined behavior to send multi-tick
        // recaps with a fragment ordering other than one Message::START, and
        // zero or more Message::INTERMEDIATE followed by a Message::END
        // fragment. Refer to the component documentation for more information
        // on message fragments.

    void appendRecapMessage(
                      const Name&       messageType,
                      const Topic&      topic,
                      Message::Fragment fragmentType = Message::FRAGMENT_NONE);
        // Append an empty recap message of type 'messageType' that will be
        // published under the specified 'topic' to the event referenced by
        // this 'EventFormatter'. Specify the optional 'fragmentType' to create
        // a recap message with fragment type other than
        // 'Message::FRAGMENT_NONE', if 'fragmentType' is not provided, append
        // a recap message with fragment type 'FRAGMENT_NONE'. Single-tick
        // recap messages should have 'fragmentType'= Message::NONE. Multi-tick
        // recaps can have either Message::START, Message::INTERMEDIATE or
        // Message::END as the 'fragmentType'. After a message has been
        // appended its elements can be set using the various setElement()
        // methods. It is an error to append a recap message to an Admin event.
        // Note that it is undefined behavior to send multi-tick recaps with a
        // fragment ordering other than one Message::START, and zero or more
        // Message::INTERMEDIATE followed by a Message::END fragment. Refer to
        // the component documentation for more information on message
        // fragments.

    void appendRecapMessage(
                      const char*       messageType,
                      const Topic&      topic,
                      Message::Fragment fragmentType = Message::FRAGMENT_NONE);
        // Append an empty recap message of type 'messageType' that will be
        // published under the specified 'topic' to the event referenced by
        // this 'EventFormatter'. Specify the optional 'fragmentType' to create
        // a recap message with fragment type other than
        // 'Message::FRAGMENT_NONE', if 'fragmentType' is not provided, append
        // a recap message with fragment type 'FRAGMENT_NONE'. Single-tick
        // recap messages should have 'fragmentType'= Message::NONE.
        // Multi-tick recaps can have either Message::START,
        // Message::INTERMEDIATE or Message::END as the 'fragmentType'. After a
        // message has been appended its elements can be set using the various
        // setElement() methods. It is an error to append a recap message to an
        // Admin event. Note that it is undefined behavior to send multi-tick
        // recaps with a fragment ordering other than one Message::START, and
        // zero or more Message::INTERMEDIATE followed by a Message::END
        // fragment. Refer to the component documentation for more information
        // on message fragments.

    void appendRecapMessage(const Topic&         topic,
                            unsigned int         sequenceNumber,
                            const CorrelationId *cid = 0);
        // DEPRECATED
        // Append an empty recap message with the default 'messageType' that
        // will be published under the specified 'topic' with the specified
        // 'sequenceNumber' to the event referenced by this 'EventFormatter'.
        // Specify the optional CorrelationId pointer 'cid' if this recap
        // message is added in response to a TOPIC_RECAP message. It is
        // undefined behavior to call this function with a non-null 'cid' for a
        // TOPIC_RECAP message with a different 'Topic' than that referenced by
        // 'topic'. It is expected that 'sequenceNumber' is greater (unless the
        // value wrapped) than the last value used in any previous message on
        // this 'topic', otherwise the behavior is undefined. This method will
        // create a recap message with fragment type 'FRAGMENT_NONE'. After a
        // message has been appended its elements can be set using the various
        // setElement() methods. It is an error to append a recap message to an
        // Admin event. Note that this function overload is deprecated for
        // non-null 'cid'. Utilize the overload
        // 'appendRecapMessage(const CorrelationId& cid,...)' to respond to
        // TOPIC_RECAP messages. Specifying a 'sequenceNumber' for solicited
        // recap messages results in undefined behavior.

    void appendRecapMessage(const Name&  messageType,
                            const Topic& topic,
                            unsigned int sequenceNumber);
        // Append an empty recap message of type 'messageType' that will be
        // published under the specified 'topic' with the specified
        // 'sequenceNumber' to the event referenced by this 'EventFormatter'.
        // It is expected that 'sequenceNumber' is greater (unless the value
        // wrapped) than the last value used in any previous message on this
        // 'topic', otherwise the behavior is undefined. This method will
        // create a recap message with fragment type 'FRAGMENT_NONE'. After a
        // message has been appended its elements can be set using the various
        // setElement() methods. It is an error to append a recap message to
        // an Admin event.

    void appendRecapMessage(const char*  messageType,
                            const Topic& topic,
                            unsigned int sequenceNumber);
        // Append an empty recap message of type 'messageType' that will be
        // published under the specified 'topic' with the specified
        // 'sequenceNumber' to the event referenced by this 'EventFormatter'.
        // It is expected that 'sequenceNumber' is greater (unless the value
        // wrapped) than the last value used in any previous message on this
        // 'topic', otherwise the behavior is undefined. This method will
        // create a recap message with fragment type'FRAGMENT_NONE'. After a
        // message has been appended its elements can be set using the various
        // setElement() methods. It is an error to append a recap message to an
        // Admin event.

    void appendRecapMessage(const Topic&      topic,
                            Message::Fragment fragmentType,
                            unsigned int      sequenceNumber);
        // Append an empty recap message with the default 'messageType' that
        // will be published under the specified 'topic' with the specified
        // 'sequenceNumber' to the event referenced by this 'EventFormatter'.
        // It is expected that 'sequenceNumber' is greater (unless the value
        // wrapped) than the last value used in any previous message on this
        // 'topic', otherwise the behavior is undefined. The specified
        // 'fragmentType' determines what fragment flag to apply to this recap
        // message. Single-tick recap messages should have
        // 'fragmentType'= Message::NONE. Multi-tick recaps can have either
        // Message::START, Message::INTERMEDIATE or Message::END as the
        // 'fragmentType'. After a message has been appended its elements can
        // be set using the various setElement() methods. It is an error to
        // append a recap message to an Admin event. Note that it is undefined
        // behavior to send multi-tick recaps with a fragment ordering other
        // than one Message::START, and zero or more Message::INTERMEDIATE
        // followed by a Message::END fragment. Refer to the component
        // documentation for more information on message fragments.

    void appendRecapMessage(const Name&       messageType,
                            const Topic&      topic,
                            Message::Fragment fragmentType,
                            unsigned int      sequenceNumber);
        // Append an empty recap message of the specified 'messageType' that
        // will be published under the specified 'topic' with the specified
        // 'sequenceNumber' to the event referenced by this 'EventFormatter'.
        // It is expected that 'sequenceNumber' is greater (unless the value
        // wrapped) than the last value used in any previous message on this
        // 'topic', otherwise the behavior is undefined.The specified
        // 'fragmentType' determines what fragment flag to apply to this recap
        // message. Single-tick recap messages should have
        // 'fragmentType'= Message::NONE.  Multi-tick recaps can have either
        // Message::START, Message::INTERMEDIATE or Message::END as the
        // 'fragmentType'. After a message has been appended its elements can
        // be set using the various setElement() methods. It is an error to
        // append a recap message to an Admin event. Note that it is undefined
        // behavior to send multi-tick recaps with a fragment ordering other
        // than one Message::START, and zero or more Message::INTERMEDIATE
        // followed by a Message::END fragment. Refer to the component
        // documentation for more information on message fragments.

    void appendRecapMessage(const char*       messageType,
                            const Topic&      topic,
                            Message::Fragment fragmentType,
                            unsigned int      sequenceNumber);
        // Append an empty recap message of the specified 'messageType' that
        // will be published under the specified 'topic' with the specified
        // 'sequenceNumber' to the event referenced by this 'EventFormatter'.
        // It is expected that 'sequenceNumber' is greater (unless the value
        // wrapped) than the last value used in any previous message on this
        // 'topic', otherwise the behavior is undefined. The specified
        // 'fragmentType' determines what fragment flag to apply to this recap
        // message. Single-tick recap messages should have
        // 'fragmentType'= Message::NONE.  Multi-tick recaps can have either
        // Message::START, Message::INTERMEDIATE or Message::END as the
        // 'fragmentType'. After a message has been appended its elements can
        // be set using the various setElement() methods. It is an error to
        // append a recap message to an Admin event. Note that it is undefined
        // behavior to send multi-tick recaps with a fragment ordering other
        // than one Message::START, and zero or more Message::INTERMEDIATE
        // followed by a Message::END fragment. Refer to the component
        // documentation for more information on message fragments.

    void appendRecapMessage(
                   const CorrelationId& cid,
                   Message::Fragment    fragmentType = Message::FRAGMENT_NONE);
        // Append an empty recap message with the default 'messageType' that
        // will be published to the event referenced by this 'EventFormatter'.
        // The specified 'cid' identifies the TOPIC_RECAP message that this
        // recap message is in response to. Specify the optional 'fragmentType'
        // to create a recap message with fragment type other than
        // 'Message::FRAGMENT_NONE', if 'fragmentType' is not provided, append
        // a recap message with fragment type 'FRAGMENT_NONE'. Single-tick
        // recap messages should have 'fragmentType'= Message::NONE.
        // Multi-tick recaps can have either Message::START,
        // Message::INTERMEDIATE or Message::END as the 'fragmentType'. After a
        // message has been appended its elements can be set using the various
        // setElement() methods. It is an error to append a recap message to an
        // Admin event. Note that it is undefined behavior to send multi-tick
        // recaps with a fragment ordering other than one Message::START, and
        // zero or more Message::INTERMEDIATE followed by a Message::END
        // fragment. Refer to the component documentation for more information
        // on message fragments.

    void appendRecapMessage(
                   const Name&          messageType,
                   const CorrelationId& cid,
                   Message::Fragment    fragmentType = Message::FRAGMENT_NONE);
        // Append an empty recap message of type 'messageType' that will be
        // published to the event referenced by this 'EventFormatter'. The
        // specified 'cid' identifies the TOPIC_RECAP message that this recap
        // message is in response to. Specify the optional 'fragmentType' to
        // create a recap message with fragment type other than
        // 'Message::FRAGMENT_NONE', if 'fragmentType' is not provided, append
        // a recap message with fragment type 'FRAGMENT_NONE'. Single-tick
        // recap messages should have 'fragmentType'= Message::NONE.
        // Multi-tick recaps can have either Message::START,
        // Message::INTERMEDIATE or Message::END as the 'fragmentType'. After a
        // message has been appended its elements can be set using the various
        // setElement() methods. It is an error to append a recap message to an
        // Admin event. Note that it is undefined behavior to send multi-tick
        // recaps with a fragment ordering other than one Message::START, and
        // zero or more Message::INTERMEDIATE followed by a Message::END
        // fragment. Refer to the component documentation for more information
        // on message fragments.

    void appendRecapMessage(
                   const char*          messageType,
                   const CorrelationId& cid,
                   Message::Fragment    fragmentType = Message::FRAGMENT_NONE);
        // Append an empty recap message of type 'messageType' that will be
        // published to the event referenced by this 'EventFormatter'.The
        // specified 'cid' identifies the TOPIC_RECAP message that this recap
        // message is in response to. Specify the optional 'fragmentType' to
        // create a recap message with fragment type other than
        // 'Message::FRAGMENT_NONE', if 'fragmentType' is not provided, append
        // a recap message with fragment type 'FRAGMENT_NONE'. Single-tick
        // recap messages should have 'fragmentType'= Message::NONE.
        // Multi-tick recaps can have either Message::START,
        // Message::INTERMEDIATE or Message::END as the 'fragmentType'. After a
        // message has been appended its elements can be set using the various
        // setElement() methods. It is an error to append a recap message to an
        // Admin event. Note that it is undefined behavior to send multi-tick
        // recaps with a fragment ordering other than one Message::START, and
        // zero or more Message::INTERMEDIATE followed by a Message::END
        // fragment. Refer to the component documentation for more information
        // on message fragments.

    void setElement(const char *name, bool value);
        // Set the element with the specified 'name' to the specified
        // 'value' in the current message in the Event referenced by
        // this EventFormatter. If the 'name' is invalid for the
        // current message, if appendMessage() has never been called
        // or if the element identified by 'name' has already been set
        // an exception is thrown.

    void setElement(const char *name, char value);
        // Set the element with the specified 'name' to the specified
        // 'value' in the current message in the Event referenced by
        // this EventFormatter. If the 'name' is invalid for the
        // current message, if appendMessage() has never been called
        // or if the element identified by 'name' has already been set
        // an exception is thrown.

    void setElement(const char *name, Int32 value);
        // Set the element with the specified 'name' to the specified
        // 'value' in the current message in the Event referenced by
        // this EventFormatter. If the 'name' is invalid for the
        // current message, if appendMessage() has never been called
        // or if the element identified by 'name' has already been set
        // an exception is thrown.

    void setElement(const char *name, Int64 value);
        // Set the element with the specified 'name' to the specified
        // 'value' in the current message in the Event referenced by
        // this EventFormatter. If the 'name' is invalid for the
        // current message, if appendMessage() has never been called
        // or if the element identified by 'name' has already been set
        // an exception is thrown.

    void setElement(const char *name, Float32 value);
        // Set the element with the specified 'name' to the specified
        // 'value' in the current message in the Event referenced by
        // this EventFormatter. If the 'name' is invalid for the
        // current message, if appendMessage() has never been called
        // or if the element identified by 'name' has already been set
        // an exception is thrown.

    void setElement(const char *name, Float64 value);
        // Set the element with the specified 'name' to the specified
        // 'value' in the current message in the Event referenced by
        // this EventFormatter. If the 'name' is invalid for the
        // current message, if appendMessage() has never been called
        // or if the element identified by 'name' has already been set
        // an exception is thrown.

    void setElement(const char *name, const Datetime& value);
        // Set the element with the specified 'name' to the specified
        // 'value' in the current message in the Event referenced by
        // this EventFormatter. If the 'name' is invalid for the
        // current message, if appendMessage() has never been called
        // or if the element identified by 'name' has already been set
        // an exception is thrown.  This function sets the Datetime
        // value with millisecond resolution.

    void setElement(const char *name, const Datetime::HighPrecision& value);
        // Set the element with the specified 'name' to the specified
        // 'value' in the current message in the Event referenced by
        // this EventFormatter. If the 'name' is invalid for the
        // current message, if appendMessage() has never been called
        // or if the element identified by 'name' has already been set
        // an exception is thrown.  This function sets the Datetime
        // value with full precision.  Note that while Datetime supports
        // picosecond resolution, messages on the wire won't have the full
        // resolution.

    void setElement(const char *name, const char *value);
        // Set the element with the specified 'name' to the specified
        // 'value' in the current message in the Event referenced by
        // this EventFormatter. If the 'name' is invalid for the
        // current message, if appendMessage() has never been called
        // or if the element identified by 'name' has already been set
        // an exception is thrown. The behavior is undefined unless 'value'
        // is not 'NULL'.
        // Clients wishing to format and publish null values (e.g. for the
        // purpose of cache management) should *not* use this function; use
        // 'setElementNull' instead.

    void setElement(const char *name, const Name& value);
        // Set the element with the specified 'name' to the specified
        // 'value' in the current message in the Event referenced by
        // this EventFormatter. If the 'name' is invalid for the
        // current message, if appendMessage() has never been called
        // or if the element identified by 'name' has already been set
        // an exception is thrown.

    void setElementNull(const char *name);
        // Create a null element with the specified 'name'. Note that whether
        // or not fields containing null values are published to subscribers is
        // dependent upon details of the service and schema configuration. If
        // the 'name' is invalid for the current message, if appendMessage()
        // has never been called or if the element identified by 'name' has
        // already been set an exception is thrown.

    void setElement(const Name& name, bool value);
        // Set the element with the specified 'name' to the specified
        // 'value' in the current message in the Event referenced by
        // this EventFormatter. If the 'name' is invalid for the
        // current message, if appendMessage() has never been called
        // or if the element identified by 'name' has already been set
        // an exception is thrown.

    void setElement(const Name& name, char value);
        // Set the element with the specified 'name' to the specified
        // 'value' in the current message in the Event referenced by
        // this EventFormatter. If the 'name' is invalid for the
        // current message, if appendMessage() has never been called
        // or if the element identified by 'name' has already been set
        // an exception is thrown.

    void setElement(const Name& name, Int32 value);
        // Set the element with the specified 'name' to the specified
        // 'value' in the current message in the Event referenced by
        // this EventFormatter. If the 'name' is invalid for the
        // current message, if appendMessage() has never been called
        // or if the element identified by 'name' has already been set
        // an exception is thrown.

    void setElement(const Name& name, Int64 value);
        // Set the element with the specified 'name' to the specified
        // 'value' in the current message in the Event referenced by
        // this EventFormatter. If the 'name' is invalid for the
        // current message, if appendMessage() has never been called
        // or if the element identified by 'name' has already been set
        // an exception is thrown.

    void setElement(const Name& name, Float32 value);
        // Set the element with the specified 'name' to the specified
        // 'value' in the current message in the Event referenced by
        // this EventFormatter. If the 'name' is invalid for the
        // current message, if appendMessage() has never been called
        // or if the element identified by 'name' has already been set
        // an exception is thrown.

    void setElement(const Name& name, Float64 value);
        // Set the element with the specified 'name' to the specified
        // 'value' in the current message in the Event referenced by
        // this EventFormatter. If the 'name' is invalid for the
        // current message, if appendMessage() has never been called
        // or if the element identified by 'name' has already been set
        // an exception is thrown.

    void setElement(const Name& name, const Datetime& value);
        // Set the element with the specified 'name' to the specified
        // 'value' in the current message in the Event referenced by
        // this EventFormatter. If the 'name' is invalid for the
        // current message, if appendMessage() has never been called
        // or if the element identified by 'name' has already been set
        // an exception is thrown.  This function sets the Datetime
        // value with millisecond resolution.

    void setElement(const Name& name, const Datetime::HighPrecision& value);
        // Set the element with the specified 'name' to the specified
        // 'value' in the current message in the Event referenced by
        // this EventFormatter. If the 'name' is invalid for the
        // current message, if appendMessage() has never been called
        // or if the element identified by 'name' has already been set
        // an exception is thrown.  This function sets the Datetime
        // value with full precision.  Note that while Datetime supports
        // picosecond resolution, messages on the wire won't have the full
        // resolution.


    void setElement(const Name& name, const char *value);
        // Set the element with the specified 'name' to the specified
        // 'value' in the current message in the Event referenced by
        // this EventFormatter. If the 'name' is invalid for the
        // current message, if appendMessage() has never been called
        // or if the element identified by 'name' has already been set
        // an exception is thrown. The behavior is undefined unless 'value'
        // is not 'NULL'.
        // Clients wishing to format and publish null values (e.g. for the
        // purpose of cache management) should *not* use this function; use
        // 'setElementNull' instead.

    void setElement(const Name& name, const Name& value);
        // Set the element with the specified 'name' to the specified
        // 'value' in the current message in the Event referenced by
        // this EventFormatter. If the 'name' is invalid for the
        // current message, if appendMessage() has never been called
        // or if the element identified by 'name' has already been set
        // an exception is thrown.

    void setElementNull(const Name& name);
        // Create a null element with the specified 'name'. Note that whether
        // or not fields containing null values are published to subscribers is
        // dependent upon details of the service and schema configuration. If
        // the 'name' is invalid for the current message, if appendMessage()
        // has never been called or if the element identified by 'name' has
        // already been set an exception is thrown.

    void pushElement(const char *name);
    void pushElement(const Name& name);
        // Changes the level at which this EventFormatter is operating
        // to the specified element 'name'. The element 'name' must
        // identify either a choice, a sequence or an array at the
        // current level of the schema or the behavior is
        // undefined.  If the 'name' is invalid for the current message, if
        // appendMessage() has never been called or if the element identified
        // by 'name' has already been set an exception is thrown. After this
        // returns the context of the EventFormatter is set to the element
        // 'name' in the schema and any calls to setElement() or pushElement()
        // are applied at that level. If 'name' represents an array of scalars
        // then appendValue() must be used to add values. If 'name' represents
        // an array of complex types then appendElement() creates the first
        // entry and set the context of the EventFormatter to that element.
        // Calling appendElement() again will create another entry.

    void popElement();
        // Undoes the most recent call to pushLevel() on this
        // EventFormatter and returns the context of the
        // EventFormatter to where it was before the call to
        // pushElement(). Once popElement() has been called it is
        // invalid to attempt to re-visit the same context.

    void appendValue(bool value);
    void appendValue(char value);
    void appendValue(Int32 value);
    void appendValue(Int64 value);
    void appendValue(Float32 value);
    void appendValue(Float64 value);
    void appendValue(const char *value);
    void appendValue(const Name& value);

    void appendValue(const Datetime& value);
    void appendValue(const Datetime::HighPrecision& value);

    void appendElement();
};

// ============================================================================
//                      INLINE AND TEMPLATE FUNCTION IMPLEMENTATIONS
// ============================================================================

                            // --------------------
                            // class EventFormatter
                            // --------------------

inline
EventFormatter::EventFormatter(Event& event)
{
    d_handle = blpapi_EventFormatter_create(event.impl());
}

inline
EventFormatter::~EventFormatter()
{
    blpapi_EventFormatter_destroy(d_handle);
}

inline
void EventFormatter::appendMessage(const char *messageType, const Topic& topic)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_appendMessage(
                                    d_handle,
                                    messageType, 0,
                                    topic.impl()));
}

inline
void EventFormatter::appendMessage(const Name& messageType, const Topic& topic)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_appendMessage(
                                    d_handle,
                                    0, messageType.impl(),
                                    topic.impl()));
}

inline
void EventFormatter::appendMessage(const char   *messageType,
                                   const Topic&  topic,
                                   unsigned int  sequenceNumber)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL_EVENTFORMATTER_APPENDMESSAGESEQ(
                                    d_handle,
                                    messageType,
                                    0,
                                    topic.impl(),
                                    sequenceNumber,
                                    0));
}

inline
void EventFormatter::appendMessage(const Name&  messageType,
                                   const Topic& topic,
                                   unsigned int sequenceNumber)
{
    ExceptionUtil::throwOnError(BLPAPI_CALL_EVENTFORMATTER_APPENDMESSAGESEQ(
                                    d_handle,
                                    0,
                                    messageType.impl(),
                                    topic.impl(),
                                    sequenceNumber,
                                    0));
}

inline
void EventFormatter::appendResponse(const char *opType)
{
    ExceptionUtil::throwOnError(
        blpapi_EventFormatter_appendResponse(
            d_handle,
            opType,
            0));
}

inline
void EventFormatter::appendResponse(const Name& opType)
{
    ExceptionUtil::throwOnError(
        blpapi_EventFormatter_appendResponse(
            d_handle,
            0,
            opType.impl()));
}

inline
void EventFormatter::appendRecapMessage(const Topic&         topic,
                                        const CorrelationId *cid)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_appendRecapMessage(
                                    d_handle,
                                    topic.impl(),
                                    cid ? &cid->impl() : 0));
}

inline
void EventFormatter::appendRecapMessage(const Topic&      topic,
                                        Message::Fragment fragmentType)
{
    ExceptionUtil::throwOnError(
        BLPAPI_CALL(blpapi_EventFormatter_appendFragmentedRecapMessage)(
            d_handle,
            0, // 'typeString'
            0, // 'typeName'
            topic.impl(), // 'topic'
            0, // 'cid'
            static_cast<int>(fragmentType)));
}

inline
void EventFormatter::appendRecapMessage(const Name&       messageType,
                                        const Topic&      topic,
                                        Message::Fragment fragmentType)
{
    ExceptionUtil::throwOnError(
        BLPAPI_CALL(blpapi_EventFormatter_appendFragmentedRecapMessage)(
            d_handle,
            0, // 'typeString'
            messageType.impl(), // 'typeName'
            topic.impl(), // 'topic'
            0, // 'cid'
            static_cast<int>(fragmentType)));
}

inline
void EventFormatter::appendRecapMessage(const char*       messageType,
                                        const Topic&      topic,
                                        Message::Fragment fragmentType)
{
    ExceptionUtil::throwOnError(
        BLPAPI_CALL(blpapi_EventFormatter_appendFragmentedRecapMessage)(
            d_handle,
            messageType, // 'typeString'
            0, // 'typeName'
            topic.impl(), // 'topic'
            0, // 'cid'
            static_cast<int>(fragmentType)));
}

inline
void EventFormatter::appendRecapMessage(const Topic&         topic,
                                        unsigned int         sequenceNumber,
                                        const CorrelationId *cid)
{
    ExceptionUtil::throwOnError(
        BLPAPI_CALL_EVENTFORMATTER_APPENDRECAPMESSAGESEQ(
            d_handle,
            topic.impl(),
            cid ? &cid->impl() : 0,
            sequenceNumber,
            0));

}

inline
void EventFormatter::appendRecapMessage(const Name&  messageType,
                                        const Topic& topic,
                                        unsigned int sequenceNumber)
{
    ExceptionUtil::throwOnError(
        BLPAPI_CALL(blpapi_EventFormatter_appendFragmentedRecapMessageSeq)(
            d_handle,
            0, // 'typeString'
            messageType.impl(), // 'typeName'
            topic.impl(), // 'topic'
            static_cast<int>(Message::FRAGMENT_NONE),
            sequenceNumber));
}

inline
void EventFormatter::appendRecapMessage(const char*  messageType,
                                        const Topic& topic,
                                        unsigned int sequenceNumber)
{
    ExceptionUtil::throwOnError(
        BLPAPI_CALL(blpapi_EventFormatter_appendFragmentedRecapMessageSeq)(
            d_handle,
            messageType, // 'typeString'
            0, // 'typeName'
            topic.impl(), // 'topic'
            static_cast<int>(Message::FRAGMENT_NONE),
            sequenceNumber));
}

inline
void EventFormatter::appendRecapMessage(const Topic&      topic,
                                        Message::Fragment fragmentType,
                                        unsigned int      sequenceNumber)
{
    ExceptionUtil::throwOnError(
        BLPAPI_CALL(blpapi_EventFormatter_appendFragmentedRecapMessageSeq)(
                d_handle,
                0, // 'typeString'
                0, // 'typeName'
                topic.impl(), // 'topic'
                static_cast<int>(fragmentType),
                sequenceNumber));
}

inline
void EventFormatter::appendRecapMessage(const Name&       messageType,
                                        const Topic&      topic,
                                        Message::Fragment fragmentType,
                                        unsigned int      sequenceNumber)
{
    ExceptionUtil::throwOnError(
        BLPAPI_CALL(blpapi_EventFormatter_appendFragmentedRecapMessageSeq)(
            d_handle,
            0, // 'typeString'
            messageType.impl(), // 'typeName'
            topic.impl(), // 'topic'
            static_cast<int>(fragmentType),
            sequenceNumber));
}

inline
void EventFormatter::appendRecapMessage(const char*       messageType,
                                        const Topic&      topic,
                                        Message::Fragment fragmentType,
                                        unsigned int      sequenceNumber)
{
    ExceptionUtil::throwOnError(
        BLPAPI_CALL(blpapi_EventFormatter_appendFragmentedRecapMessageSeq)(
            d_handle,
            messageType, // 'typeString'
            0, // 'typeName'
            topic.impl(), // 'topic'
            static_cast<int>(fragmentType),
            sequenceNumber));
}

inline
void EventFormatter::appendRecapMessage(const CorrelationId& cid,
                                        Message::Fragment    fragmentType)
{
    ExceptionUtil::throwOnError(
        BLPAPI_CALL(blpapi_EventFormatter_appendFragmentedRecapMessage)(
            d_handle,
            0, // 'typeString'
            0, // 'typeName'
            0, // 'topic'
            &cid.impl(), // 'cid'
            static_cast<int>(fragmentType)));
}

inline
void EventFormatter::appendRecapMessage(const Name&          messageType,
                                        const CorrelationId& cid,
                                        Message::Fragment    fragmentType)
{
    ExceptionUtil::throwOnError(
        BLPAPI_CALL(blpapi_EventFormatter_appendFragmentedRecapMessage)(
            d_handle,
            0, // 'typeString'
            messageType.impl(), // 'typeName'
            0, // 'topic'
            &cid.impl(), // 'cid'
            static_cast<int>(fragmentType)));
}

inline
void EventFormatter::appendRecapMessage(const char*          messageType,
                                        const CorrelationId& cid,
                                        Message::Fragment    fragmentType)
{
    ExceptionUtil::throwOnError(
        BLPAPI_CALL(blpapi_EventFormatter_appendFragmentedRecapMessage)(
            d_handle,
            messageType, // 'typeString'
            0, // 'typeName'
            0, // 'topic'
            &cid.impl(), // 'cid'
            static_cast<int>(fragmentType)));
}

inline
void EventFormatter::setElement(const char *name, bool value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_setValueBool(
                                    d_handle,
                                    name, 0,
                                    value));
}

inline
void EventFormatter::setElement(const char *name, char value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_setValueChar(
                                    d_handle,
                                    name, 0,
                                    value));
}

inline
void EventFormatter::setElement(const char *name, Int32 value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_setValueInt32(
                                    d_handle,
                                    name, 0,
                                    value));
}

inline
void EventFormatter::setElement(const char *name, Int64 value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_setValueInt64(
                                    d_handle,
                                    name, 0,
                                    value));
}

inline
void EventFormatter::setElement(const char *name, Float32 value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_setValueFloat32(
                                    d_handle,
                                    name, 0,
                                    value));
}

inline
void EventFormatter::setElement(const char *name, Float64 value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_setValueFloat64(
                                    d_handle,
                                    name, 0,
                                    value));
}

inline
void EventFormatter::setElement(const char *name, const Datetime& value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_setValueDatetime(
                                    d_handle,
                                    name, 0,
                                    &value.rawValue()));
}

inline
void EventFormatter::setElement(const char *name,
                                const Datetime::HighPrecision& value)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_EventFormatter_setValueHighPrecisionDatetime)(
                                              d_handle,
                                              name, 0,
                                              &value));
}

inline
void EventFormatter::setElement(const char *name, const char *value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_setValueString(
                                    d_handle,
                                    name, 0,
                                    value));
}

inline
void EventFormatter::setElement(const char *name, const Name& value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_setValueFromName(
                                    d_handle,
                                    name, 0,
                                    value.impl()));
}

inline
void EventFormatter::setElementNull(const char *name)
{
    ExceptionUtil::throwOnError(
        BLPAPI_CALL_EVENTFORMATTER_SETVALUENULL(d_handle,
                                                name,
                                                0));
}

inline
void EventFormatter::setElement(const Name& name, bool value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_setValueBool(
                                    d_handle,
                                    0, name.impl(),
                                    value));
}

inline
void EventFormatter::setElement(const Name& name, char value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_setValueChar(
                                    d_handle,
                                    0, name.impl(),
                                    value));
}

inline
void EventFormatter::setElement(const Name& name, Int32 value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_setValueInt32(
                                    d_handle,
                                    0, name.impl(),
                                    value));
}

inline
void EventFormatter::setElement(const Name& name, Int64 value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_setValueInt64(
                                    d_handle,
                                    0, name.impl(),
                                    value));
}

inline
void EventFormatter::setElement(const Name& name, Float32 value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_setValueFloat32(
                                    d_handle,
                                    0, name.impl(),
                                    value));
}
inline
void EventFormatter::setElement(const Name& name, Float64 value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_setValueFloat64(
                                    d_handle,
                                    0, name.impl(),
                                    value));
}

inline
void EventFormatter::setElement(const Name& name, const Datetime& value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_setValueDatetime(
                                    d_handle,
                                    0, name.impl(),
                                    &value.rawValue()));
}

inline
void EventFormatter::setElement(const Name& name,
                                const Datetime::HighPrecision& value)
{
    ExceptionUtil::throwOnError(
            BLPAPI_CALL(blpapi_EventFormatter_setValueHighPrecisionDatetime)(
                                              d_handle,
                                              0, name.impl(),
                                              &value));
}

inline
void EventFormatter::setElement(const Name& name, const char *value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_setValueString(
                                    d_handle,
                                    0, name.impl(),
                                    value));
}
inline
void EventFormatter::setElement(const Name& name, const Name& value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_setValueFromName(
                                    d_handle,
                                    0, name.impl(),
                                    value.impl()));
}

inline
void EventFormatter::setElementNull(const Name& name)
{
    ExceptionUtil::throwOnError(
        BLPAPI_CALL_EVENTFORMATTER_SETVALUENULL(d_handle,
                                                0,
                                                name.impl()));
}

inline
void EventFormatter::pushElement(const char *name)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_pushElement(
                                    d_handle, name, 0));
}

inline
void EventFormatter::pushElement(const Name& name)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_pushElement(
                                    d_handle, 0, name.impl()));
}

inline
void EventFormatter::popElement()
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_popElement(
                                    d_handle));
}

inline
void EventFormatter::appendValue(bool value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_appendValueBool(
                                    d_handle,
                                    value));
}

inline
void EventFormatter::appendValue(char value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_appendValueChar(
                                    d_handle,
                                    value));
}

inline
void EventFormatter::appendValue(Int32 value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_appendValueInt32(
                                    d_handle,
                                    value));
}

inline
void EventFormatter::appendValue(Int64 value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_appendValueInt64(
                                    d_handle,
                                    value));
}

inline
void EventFormatter::appendValue(Float32 value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_appendValueFloat32(
                                    d_handle,
                                    value));
}

inline
void EventFormatter::appendValue(Float64 value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_appendValueFloat64(
                                    d_handle,
                                    value));
}


inline
void EventFormatter::appendValue(const Datetime& value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_appendValueDatetime(
                                    d_handle,
                                    &value.rawValue()));

}

inline
void EventFormatter::appendValue(const Datetime::HighPrecision& value)
{
    ExceptionUtil::throwOnError(
           BLPAPI_CALL(blpapi_EventFormatter_appendValueHighPrecisionDatetime)(
                                              d_handle,
                                              &value));

}

inline
void EventFormatter::appendValue(const char *value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_appendValueString(
                                    d_handle,
                                    value));
}
inline
void EventFormatter::appendValue(const Name& value)
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_appendValueFromName(
                                    d_handle,
                                    value.impl()));
}

inline
void EventFormatter::appendElement()
{
    ExceptionUtil::throwOnError(blpapi_EventFormatter_appendElement(
                                   d_handle));
}


}  // close namespace blpapi
}  // close namespace BloombergLP

#endif // #ifdef __cplusplus
#endif // #ifndef INCLUDED_BLPAPI_EVENTFORMATTER
