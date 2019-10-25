#[allow(non_snake_case, non_camel_case_types, non_upper_case_globals)]
mod bindings;

pub use bindings::*;

#[test]
fn test_historical() {
    use std::ffi::CString;

    unsafe {
        let session_options = blpapi_SessionOptions_create();
        assert!(!session_options.is_null());

        let host = CString::new("localhost").unwrap();
        let res = blpapi_SessionOptions_setServerHost(session_options, host.as_ptr());
        assert_eq!(2, res, "{}", res);

        blpapi_SessionOptions_destroy(session_options);
    }
}
