use crate::{request::Request, Error};
use blpapi_sys::*;
use std::ffi::CStr;

/// A `Service`
/// created from a `Session::get_service`
pub struct Service(pub(crate) *mut blpapi_Service_t);

impl Service {
    /// Get service name
    pub fn name(&self) -> String {
        let name = unsafe { CStr::from_ptr(blpapi_Service_name(self.0)) };
        name.to_string_lossy().into_owned()
    }

    /// Create a new request
    pub fn create_request(&self, operation: &str) -> Result<Request, Error> {
        Request::new(self, operation)
    }
}

impl Drop for Service {
    fn drop(&mut self) {
        unsafe { blpapi_Service_release(self.0) }
    }
}
