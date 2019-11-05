use blpapi_sys::*;
use std::ffi::{CStr, CString};

lazy_static::lazy_static! {
    pub static ref SECURITY_DATA: Name = Name::new("securityData");
    pub static ref SECURITY_NAME: Name = Name::new("security");
    pub static ref FIELD_DATA: Name = Name::new("fieldData");
    pub static ref SECURITY_ERROR: Name = Name::new("securityError");
    pub static ref SECURITIES: Name = Name::new("securities");
    pub static ref FIELDS_NAME: Name = Name::new("fields");
    pub static ref SESSION_TERMINATED: Name = Name::new("SesssionTerminated");
    pub static ref SESSION_STARTUP_FAILURE: Name = Name::new("SessionStartupFailure");
}

/// A `Name`
pub struct Name(pub(crate) *mut blpapi_Name_t);

// As per bloomberg documentation:
// *Each Name instance refers to an entry in a global static table* thus Name is `Sync`
// https://bloomberg.github.io/blpapi-docs/dotnet/3.12/html/T_Bloomberglp_Blpapi_Name.htm
unsafe impl Sync for Name {}

impl Name {
    /// Create a new name
    pub fn new(s: &str) -> Self {
        let name = CString::new(s).unwrap();
        unsafe { Name(blpapi_Name_create(name.as_ptr())) }
    }

    /// Name length
    pub fn len(&self) -> usize {
        unsafe { blpapi_Name_length(self.0) }
    }
}

impl std::ops::Deref for Name {
    type Target = CStr;
    fn deref(&self) -> &Self::Target {
        unsafe {
            let ptr = blpapi_Name_string(self.0);
            let len = blpapi_Name_length(self.0);
            let slice = std::slice::from_raw_parts(ptr as *const u8, len + 1);
            CStr::from_bytes_with_nul_unchecked(slice)
        }
    }
}

impl Drop for Name {
    fn drop(&mut self) {
        unsafe { blpapi_Name_destroy(self.0) }
    }
}

impl Clone for Name {
    fn clone(&self) -> Self {
        unsafe { Name(blpapi_Name_duplicate(self.0)) }
    }
}

impl<S: AsRef<str>> PartialEq<S> for Name {
    fn eq(&self, other: &S) -> bool {
        let s = CString::new(other.as_ref()).unwrap();
        unsafe { blpapi_Name_equalsStr(self.0, s.as_ptr()) != 0 }
    }
}

impl PartialEq<Name> for Name {
    fn eq(&self, other: &Name) -> bool {
        self.0 == other.0 && self.len() == other.len()
    }
}

impl std::string::ToString for Name {
    fn to_string(&self) -> String {
        unsafe {
            let ptr = blpapi_Name_string(self.0);
            CStr::from_ptr(ptr).to_string_lossy().into_owned()
        }
    }
}

impl std::fmt::Debug for Name {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "Name: '{}'", self.to_string())
    }
}
