use blpapi::{session::SessionSync, Error, RefData};

#[derive(Debug, Default, RefData)]
struct Data {
    crncy: String,
    id_bb: String,
}

pub fn main() -> Result<(), Error> {
    //let mut args = std::env::args();
    //let host = args.nth(1).unwrap_or("127.0.0.1".to_owned());
    //let port = args.next().unwrap_or("8194".to_owned()).parse().unwrap();

    println!("creating session");
    let mut session = SessionSync::new()?;

    let securities = &["IBM US Equity", "MSFT US Equity", "3333 HK Equity"];

    let data = session.ref_data::<_, Data>(securities)?;
    println!("{:?}", data);

    Ok(())
}
