# blpapi

A rust wrapper for Bloomberg blpapi.

This is a Work In Progress, I do not plan on getting to parity with the C++ API. On the other hand, I very welcome any contribution!

Tested on Windows only (DesktopApi). Compiles on Linux.
Tested version: 3.12.3.1

## Installation

1. Install C/C++ BLPAPI. (Download and extract the file from https://www.bloomberg.com/professional/support/api-library/)
2. Set `BLPAPI_LIB` environment variable
    a. On windows: *<Extract path>\lib*
    b. On linux: *<Extract path>/Linux*

## Examples

```sh
# Cargo.toml
[dependencies]
blpapi = { version = "0.0.1", features = [ "derive", "dates" ] }
```

### Reference data

```rust
use blpapi::{RefData, session::SessionSync};

// use the derive feature to automatically convert field names into bloomberg fields
#[derive(Default, RefData)]
struct EquityData {
    ticker: String,
    crncy: String,
    market_status: Option<String>,
}

let mut session = SessionSync::new().unwrap();
let securities: &[&str] = &[ /* list of security tickers */ ];

let maybe_equities = session.ref_data::<_, EquityData>(securities);
```

### Historical data

```rust
use blpapi::{RefData, session::{SessionSync, HistOptions}};

// use the **derive** feature to automatically convert field names into bloomberg fields
#[derive(Default, RefData)]
struct Price {
    px_last: f64,
}

let mut session = SessionSync::new().unwrap();
let securities: &[&str] = &[ /* list of security tickers */ ];

let options = HistOptions::new("20190101", "20191231");
let prices = session.hist_data::<_, Price>(securities, options);
```
