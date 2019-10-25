use blpapi_sys::*;
use std::ffi::{CStr, CString};

/// A `Name`
pub struct Name(pub(crate) *mut blpapi_Name_t);

impl Name {
    /// Create a new name
    pub fn new(s: &str) -> Self {
        let name = CString::new(s).unwrap().as_ptr();
        unsafe { Name(blpapi_Name_create(name)) }
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
        let s = CString::new(other.as_ref()).unwrap().as_ptr();
        unsafe { blpapi_Name_equalsStr(self.0, s) != 0 }
    }
}
