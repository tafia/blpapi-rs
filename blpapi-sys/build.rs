fn main() {
    let lib_dir = std::env::var("BLPAPI_LIB").expect(r#"Error while building blpapi-sys.
    
    Cannot find 'BLPAPI_LIB' environment variable.
    
    You can download blpapi binaries from bloomberg at:
    https://www.bloomberg.com/professional/support/api-library/
    
    Once extracted, the BLPAPI_LIB environment variable should point to the corresponding lib dir:
    - windows: <EXTRACT_PATH>\lib
    - linux: <EXTRACT_PATH>/Linux"#);

    println!("cargo:rustc-link-search={}", lib_dir);
    println!("cargo:rustc-link-lib=blpapi3_64");
}
