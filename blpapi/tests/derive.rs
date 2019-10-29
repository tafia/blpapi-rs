use blpapi::RefData;

#[derive(Default, RefData)]
pub struct Equity {
    pub crncy: String,
}
