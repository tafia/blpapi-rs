use crate::{
    element::{Element, Value},
    name::Name,
    service::Service,
    try_, Error,
};
use blpapi_sys::*;
use std::ffi::CString;

/// A `Request`
/// Created from `Service::create_request`
///
/// A `Request` dereferences to an element
pub struct Request {
    pub(crate) ptr: *mut blpapi_Request_t,
    elements: *mut blpapi_Element_t,
}

impl Request {
    /// Create a new request from a `Service`
    pub fn new(service: &Service, operation: &str) -> Result<Self, Error> {
        let operation = CString::new(operation).unwrap();
        unsafe {
            let mut ptr = std::ptr::null_mut();
            let refptr = &mut ptr as *mut _;
            let res = blpapi_Service_createRequest(service.0, refptr, operation.as_ptr());
            try_(res)?;
            let elements = blpapi_Request_elements(ptr);
            Ok(Request { ptr, elements })
        }
    }

    /// Convert the request to an Element
    pub fn as_element(&self) -> &Element {
        unsafe { Element::from_raw(self.elements) }
    }

    /// Convert the request to an Element
    pub fn as_element_mut(&mut self) -> &mut Element {
        unsafe { Element::from_raw_mut(self.elements) }
    }

    /// Append a new value to the existing inner Element sequence defined by name
    pub fn append<V: Value>(&mut self, name: &str, value: V) -> Result<(), Error> {
        self.get_element_mut(name).ok_or(Error(0))?.append(value)
    }

    /// Append a new value to the existing inner Element sequence defined by name
    pub fn append_named<V: Value>(&mut self, name: &Name, value: V) -> Result<(), Error> {
        self.get_named_element_mut(name)
            .ok_or(Error(0))?
            .append(value)
    }
}

impl Drop for Request {
    fn drop(&mut self) {
        unsafe { blpapi_Request_destroy(self.ptr) }
    }
}

impl std::ops::Deref for Request {
    type Target = Element;
    fn deref(&self) -> &Element {
        self.as_element()
    }
}

impl std::ops::DerefMut for Request {
    fn deref_mut(&mut self) -> &mut Element {
        self.as_element_mut()
    }
}
