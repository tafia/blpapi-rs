use blpapi_sys::*;
use std::ffi::CStr;

/// A `Name`
pub struct Name(pub(crate) *mut blpapi_Name_t);

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
