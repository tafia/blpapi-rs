use std::fs;
use std::path::PathBuf;

const ENV_WARNING: &'static str = r#"Error while building blpapi-sys.

    Cannot find 'BLPAPI_LIB' environment variable.

    You can download blpapi binaries from bloomberg at:
    https://www.bloomberg.com/professional/support/api-library/

    Once extracted, the BLPAPI_LIB environment variable should point to the
    corresponding lib dir:

    - windows: <EXTRACT_PATH>\lib
    - linux: <EXTRACT_PATH>/Linux"
"#;

fn main() {
    // Use the bundled libs in `../vendor`
    let lib_dir = if cfg!(feature = "bundled") {
        let mut dir = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
        dir.pop();
        dir.push("vendor");

        // Select the correct folder for this OS based on the folder suffix.
        // E.g.
        //  Windows => ../vendor/xxx-windows/lib/
        //  Linux => ../vendor/xxx-linux/Linux/
        for entry in fs::read_dir(dir.as_path()).expect("Failed to read `vendor/` dir") {
            let entry = entry.expect("Failed to read entry in `vendor/` dir");
            let path = entry.path();
            if path.is_dir() {
                let dirname = path.file_name().unwrap_or_default().to_string_lossy();
                if cfg!(windows) && dirname.ends_with("windows") {
                    dir.push(path);
                    dir.push("lib");
                    break;
                } else if cfg!(unix) && dirname.ends_with("linux") {
                    dir.push(path);
                    dir.push("Linux");
                    break;
                }
            }
        }
        dir.into_os_string().into_string().unwrap()
    } else {
        //TODO: use pkg-config to search in system lib dirs instead of
        //only relying on env variable
        std::env::var("BLPAPI_LIB").expect(ENV_WARNING)
    };

    println!("cargo:rustc-link-search={}", lib_dir);
    println!("cargo:rustc-link-lib=blpapi3_64");
}
