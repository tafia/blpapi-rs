#![cfg(feature = "derive")]

use blpapi::RefData;

#[derive(Default, RefData)]
pub struct Equity {
    pub crncy: String,
}

#[test]
fn derive_equity() {
    assert_eq!(Equity::FIELDS, &["CRNCY"]);
}
