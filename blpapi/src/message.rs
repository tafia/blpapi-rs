use crate::{correlation_id::CorrelationId, element::Element, event::Event, name::Name};
use blpapi_sys::*;
use std::ffi::CStr;
use std::marker::PhantomData;

/// A message
pub struct Message<'a> {
    pub(crate) ptr: *mut blpapi_Message_t,
    pub(crate) _phantom: PhantomData<&'a Event>,
    pub(crate) elements: *mut blpapi_Element_t,
}

impl<'a> Message<'a> {
    /// Get topic name
    pub fn topic_name(&self) -> String {
        unsafe {
            let name = blpapi_Message_topicName(self.ptr);
            CStr::from_ptr(name).to_string_lossy().into_owned()
        }
    }

    /// Get type string
    pub fn type_string(&self) -> String {
        unsafe {
            let name = blpapi_Message_typeString(self.ptr);
            CStr::from_ptr(name).to_string_lossy().into_owned()
        }
    }

    /// Get message type
    pub fn message_type(&self) -> Name {
        unsafe {
            let ptr = blpapi_Message_messageType(self.ptr);
            Name(ptr)
        }
    }

    /// Get number of correlation ids
    pub fn num_correlation_ids(&self) -> usize {
        unsafe { blpapi_Message_numCorrelationIds(self.ptr) as usize }
    }

    /// Get correlation id
    pub fn correlation_id(&self, index: usize) -> Option<CorrelationId> {
        if index > self.num_correlation_ids() {
            None
        } else {
            unsafe {
                let ptr = blpapi_Message_correlationId(self.ptr, index);
                Some(CorrelationId(ptr))
            }
        }
    }

    /// Get corresponding element
    pub fn element(&self) -> Element {
        Element { ptr: self.elements }
    }
}

//TODO:
//check if we must release it.
//from the doc, it appears that messages are reference counted (when cloned) and
//release just decrease the refcount ...
//
//impl<'a> Drop for Message<'a> {
//    fn drop(&mut self) {
//        unsafe { let _ = blpapi_Message_release(self.ptr); }
//    }
//}

//pub enum RecapType {
//None = BLPAPI_MESSAGE_RECAPTYPE_NONE,
//Solicited = BLPAPI_MESSAGE_RECAPTYPE_SOLICITED,
//Unsolicited = BLPAPI_MESSAGE_RECAPTYPE_UNSOLICITED }
//
//}
//
//impl From
