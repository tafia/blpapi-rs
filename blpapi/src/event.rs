use crate::message_iterator::MessageIterator;
use blpapi_sys::*;
use std::os::raw::c_int;

/// An event
pub struct Event(pub(crate) *mut blpapi_Event_t);

impl Event {
    /// Get event type
    pub fn event_type(&self) -> EventType {
        unsafe { blpapi_Event_eventType(self.0).into() }
    }

    /// Get an iterator over all messages of this event
    pub fn messages(&self) -> MessageIterator {
        MessageIterator::new(self)
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum EventType {
    Admin,
    SessionStatus,
    SubscriptionStatus,
    RequestStatus,
    Response,
    PartialResponse,
    SubscriptionData,
    ServiceStatus,
    Timeout,
    AuthorizationStatus,
    ResolutionStatus,
    TopicStatus,
    TokenStatus,
    Request,
    Unknown = -1,
}

impl From<c_int> for EventType {
    fn from(v: c_int) -> Self {
        match v as u32 {
            BLPAPI_EVENTTYPE_ADMIN => EventType::Admin,
            BLPAPI_EVENTTYPE_SESSION_STATUS => EventType::SessionStatus,
            BLPAPI_EVENTTYPE_SUBSCRIPTION_STATUS => EventType::SubscriptionStatus,
            BLPAPI_EVENTTYPE_REQUEST_STATUS => EventType::RequestStatus,
            BLPAPI_EVENTTYPE_RESPONSE => EventType::Response,
            BLPAPI_EVENTTYPE_PARTIAL_RESPONSE => EventType::PartialResponse,
            BLPAPI_EVENTTYPE_SUBSCRIPTION_DATA => EventType::SubscriptionData,
            BLPAPI_EVENTTYPE_SERVICE_STATUS => EventType::ServiceStatus,
            BLPAPI_EVENTTYPE_TIMEOUT => EventType::Timeout,
            BLPAPI_EVENTTYPE_AUTHORIZATION_STATUS => EventType::AuthorizationStatus,
            BLPAPI_EVENTTYPE_RESOLUTION_STATUS => EventType::ResolutionStatus,
            BLPAPI_EVENTTYPE_TOPIC_STATUS => EventType::TopicStatus,
            BLPAPI_EVENTTYPE_TOKEN_STATUS => EventType::TokenStatus,
            BLPAPI_EVENTTYPE_REQUEST => EventType::Request,
            _ => EventType::Unknown,
        }
    }
}
