use std::{
    env,
    fs::copy,
    path::{Path, PathBuf},
};

fn main() {
    let project_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap())
        .canonicalize()
        .unwrap();
    // let vendor: PathBuf = env::var("BLPAPI_LIB").expect("Cannot find BLPAPI_LIB variable.\
    // \
    // You can download blpapi binaries from bloomberg at:\
    // - windows: https://bloomberg.bintray.com/BLPAPI-Stable-Generic/blpapi_cpp_3.12.3.1-windows.zip \
    // - linux: https://bloomberg.bintray.com/BLPAPI-Stable-Generic/blpapi_cpp_3.12.3.1-linux.tar.gz \
    // - macos: https://bloomberg.bintray.com/BLPAPI-Experimental-Generic/blpapi_cpp_3.13.1.1-macos.tar.gz \
    // \
    // Once extracted, point to the corresponding lib dir:
    // - windows: <PATH>\\lib \
    // - linux: <PATH>/Linux").into();

    let vendor = project_dir.parent().unwrap().join("vendor");
    let out = PathBuf::from(env::var("OUT_DIR").unwrap());
    copy_libs(&vendor, &out);

    println!("cargo:rustc-link-search={}", out.display());
    println!("cargo:rustc-link-lib=blpapi3_64");
}

#[cfg(not(windows))]
fn copy_libs(vendor: &Path, out: &Path) {
    let lib_dir = vendor.join("blpapi_cpp_3.12.3.1-linux").join("Linux");
    copy(
        lib_dir.join("libblpapi3_64.so"),
        out.join("libblpapi3_64.so"),
    )
    .unwrap();
}

#[cfg(windows)]
fn copy_libs(vendor: &Path, out: &Path) {
    let lib_dir = vendor.join("blpapi_cpp_3.12.3.1-windows").join("lib");
    copy(lib_dir.join("blpapi3_64.dll"), out.join("blpapi3_64.dll")).unwrap();
    copy(lib_dir.join("blpapi3_64.lib"), out.join("blpapi3_64.lib")).unwrap();
}
