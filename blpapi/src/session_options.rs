use crate::{session::SessionSync, Error};
use blpapi_sys::*;
use std::ffi::{CStr, CString};
use std::os::raw::c_int;

/// A SessionOptions
///
/// Behaves like a `Session` builder
///
/// # Example
///
/// ```
/// use blpapi::session_options::SessionOptions;
///
/// let session = SessionOptions::new()
///     .with_server_host("localhost").unwrap()
///     .with_server_port(8194).unwrap()
///     .sync();
/// ```
pub struct SessionOptions(pub(crate) *mut blpapi_SessionOptions_t);

impl SessionOptions {
    /// Create a new SessionOptions
    pub fn new() -> Self {
        unsafe { SessionOptions(blpapi_SessionOptions_create()) }
    }

    /// Get client mode
    pub fn client_mode(&self) -> Result<ClientMode, Error> {
        let mode = unsafe { blpapi_SessionOptions_clientMode(self.0) };
        Error::check(mode)?;
        match mode as u32 {
            BLPAPI_CLIENTMODE_AUTO => Ok(ClientMode::Auto),
            BLPAPI_CLIENTMODE_DAPI => Ok(ClientMode::DApi),
            BLPAPI_CLIENTMODE_SAPI => Ok(ClientMode::SApi),
            BLPAPI_CLIENTMODE_COMPAT_33X => Ok(ClientMode::Compat33X),
            _ => Err(Error::Generic(mode)),
        }
    }

    /// Set client mode
    pub fn set_client_mode(&mut self, mode: ClientMode) {
        let mode = match mode {
            ClientMode::Auto => BLPAPI_CLIENTMODE_AUTO,
            ClientMode::DApi => BLPAPI_CLIENTMODE_DAPI,
            ClientMode::SApi => BLPAPI_CLIENTMODE_SAPI,
            ClientMode::Compat33X => BLPAPI_CLIENTMODE_COMPAT_33X,
        };
        unsafe {
            blpapi_SessionOptions_setClientMode(self.0, mode as c_int);
        }
    }

    /// Get server host
    pub fn server_host(&self) -> String {
        let chost = unsafe { CStr::from_ptr(blpapi_SessionOptions_serverHost(self.0)) };
        chost.to_string_lossy().into_owned()
    }

    /// Set server host
    pub fn with_server_host(self, host: &str) -> Result<Self, Error> {
        let chost = CString::new(host).unwrap();
        let res = unsafe { blpapi_SessionOptions_setServerHost(self.0, chost.as_ptr()) };
        Error::check(res)?;
        Ok(self)
    }

    /// Get server port
    pub fn server_port(&self) -> u16 {
        unsafe { blpapi_SessionOptions_serverPort(self.0) as u16 }
    }

    /// Set server port
    pub fn with_server_port(self, port: u16) -> Result<Self, Error> {
        let res = unsafe { blpapi_SessionOptions_setServerPort(self.0, port) };
        Error::check(res)?;
        Ok(self)
    }

    /// Build a session, transfer ownership
    pub fn sync(self) -> SessionSync {
        SessionSync::from_options(self)
    }
}

impl Drop for SessionOptions {
    fn drop(&mut self) {
        unsafe { blpapi_SessionOptions_destroy(self.0) }
    }
}

impl Clone for SessionOptions {
    fn clone(&self) -> Self {
        let cloned = SessionOptions::new();
        unsafe {
            blpapi_SessionOptions_copy(self.0, cloned.0);
        }
        cloned
    }
}

/// ClientMode
#[derive(Debug, Clone, Copy)]
pub enum ClientMode {
    /// Automatic
    Auto,
    /// Desktop API
    DApi,
    /// Server API
    SApi,
    /// Compat 33X
    Compat33X,
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_server_host() {
        let host = "localhost";
        let options = SessionOptions::new().with_server_host(host).unwrap();
        assert_eq!(host, options.server_host());
        let session = options.sync();
    }
}
