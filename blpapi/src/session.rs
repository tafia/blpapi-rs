use crate::{
    correlation_id::CorrelationId, event::Event, request::Request, service::Service,
    session_options::SessionOptions, try_, Error,
};
use blpapi_sys::*;
use std::{ffi::CString, ptr};

pub struct Session {
    ptr: *mut blpapi_Session_t,
    // keep a handle of the options (not sure if it should be droped or not)
    _options: SessionOptions,
    correlation_count: u64,
}

impl Session {
    /// Create a new session
    fn from_options(options: SessionOptions) -> Self {
        //TODO: check if null values are ok!
        let handler = None;
        let dispatcher = ptr::null_mut();
        let user_data = ptr::null_mut();
        let ptr = unsafe { blpapi_Session_create(options.0, handler, dispatcher, user_data) };
        Session {
            ptr,
            _options: options,
            correlation_count: 0,
        }
    }

    /// Start the session
    pub fn start(&mut self) -> Result<(), Error> {
        let res = unsafe { blpapi_Session_start(self.ptr) };
        try_(res)
    }

    /// Stop the session
    pub fn stop(&mut self) -> Result<(), Error> {
        let res = unsafe { blpapi_Session_stop(self.ptr) };
        try_(res)
    }

    /// Open service
    pub fn open_service(&mut self, service: &str) -> Result<(), Error> {
        let service = CString::new(service).unwrap().as_ptr();
        let res = unsafe { blpapi_Session_openService(self.ptr, service) };
        try_(res)
    }

    /// Get opened service
    pub fn get_service(&self, service: &str) -> Result<Service, Error> {
        let name = CString::new(service).unwrap().as_ptr();
        let mut service = ptr::null_mut();
        let res = unsafe { blpapi_Session_getService(self.ptr, &mut service as *mut _, name) };
        try_(res)?;
        Ok(Service(service))
    }

    /// Send request
    pub fn send(
        &mut self,
        request: Request,
        correlation_id: Option<CorrelationId>,
    ) -> Result<CorrelationId, Error> {
        let mut correlation_id = correlation_id.unwrap_or_else(|| self.new_correlation_id());
        let identity = ptr::null_mut();
        let event_queue = ptr::null_mut();
        let request_label = ptr::null_mut();
        let request_label_len = 0;
        unsafe {
            let res = blpapi_Session_sendRequest(
                self.ptr,
                request.ptr,
                &mut correlation_id.0 as *mut _,
                identity,
                event_queue,
                request_label,
                request_label_len,
            );
            try_(res)?;
            Ok(correlation_id)
        }
    }

    fn new_correlation_id(&mut self) -> CorrelationId {
        let id = CorrelationId::new_u64(self.correlation_count);
        self.correlation_count += 1;
        id
    }
}

impl Drop for Session {
    fn drop(&mut self) {
        unsafe { blpapi_Session_destroy(self.ptr) }
    }
}

/// A wrapper for session which only show sync fn
pub struct SessionSync(Session);

impl SessionSync {
    /// Create a new `SessionSync` from a `SessionOptions`
    pub fn from_options(options: SessionOptions) -> Self {
        SessionSync(Session::from_options(options))
    }

    /// Create a new `SessionSync` with default options
    pub fn new() -> Self {
        Self::from_options(SessionOptions::new())
    }

    /// Request for next event, optionally waiting timeout_ms if there is no event
    pub fn next_event(&mut self, timeout_ms: Option<u32>) -> Result<Event, Error> {
        let mut event = ptr::null_mut();
        let timeout = timeout_ms.unwrap_or(0);
        unsafe {
            let res = blpapi_Session_nextEvent(self.0.ptr, &mut event as *mut _, timeout);
            try_(res)?;
            Ok(Event(event))
        }
    }
}

impl std::ops::Deref for SessionSync {
    type Target = Session;
    fn deref(&self) -> &Session {
        &self.0
    }
}

impl std::ops::DerefMut for SessionSync {
    fn deref_mut(&mut self) -> &mut Session {
        &mut self.0
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn send_request() -> Result<(), Error> {
        let mut session = SessionOptions::new()
            .with_server_host("localhost")?
            .with_server_port(8194)?
            .sync();

        //session.start()?;
        //session.open_service("//blp/refdata")?;

        Ok(())
    }
}
