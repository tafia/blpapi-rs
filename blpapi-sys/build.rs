use std::{env, path::PathBuf};

fn main() {
    let project_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap())
        .canonicalize()
        .unwrap();
    let src = project_dir
        .parent()
        .unwrap()
        .join("vendor")
        .join("blpapi_cpp_3.12.3.1-windows");
    let out = PathBuf::from(env::var("OUT_DIR").unwrap());
    std::fs::copy(src.join("lib").join("blpapi3_64.dll"), out.join("blpapi3_64.dll")).unwrap();
    std::fs::copy(src.join("lib").join("blpapi3_64.lib"), out.join("blpapi3_64.lib")).unwrap();

    println!("cargo:rustc-link-search={}", out.display());
    println!("cargo:rustc-link-lib=blpapi3_64");
}
