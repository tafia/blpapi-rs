use blpapi::{event::EventType, session_options::SessionOptions, Error};

pub fn main() -> Result<(), Error> {
    let mut args = std::env::args();
    let host = args.nth(1).unwrap_or("127.0.0.1".to_owned());
    let port = args.next().unwrap_or("8194".to_owned()).parse().unwrap();

    println!("creating session");
    let mut session = SessionOptions::new()
        .with_server_host(&host)?
        .with_server_port(port)?
        .sync();

    // starting session
    println!("starting session");
    session.start()?;

    println!("getting ref data service");
    let service = session.get_service("//blp/refdata")?;
    let mut request = service.create_request("ReferenceDataRequest")?;

    // append securities
    request.append("securities", "IBM US Equity")?;
    request.append("securities", "MSFT US Equity")?;
    request.append("securities", "3333 HK Equity")?;

    // append fields
    request.append("fields", "PX_LAST")?;
    request.append("fields", "DS002")?;

    // send request and get it's newly created id
    println!("sending request");
    let _id = session.send(request, None)?;

    // event loop
    let timeout = None;
    let mut event_type = EventType::Unknown;
    println!("starting loop");
    while event_type != EventType::Response {
        println!("event_type: {:?}", event_type);
        let event = session.next_event(timeout)?;
        for message in event.messages() {
            println!(
                "Message {}, Topic: {}",
                message.type_string(),
                message.topic_name()
            );
        }
        event_type = event.event_type();
    }
    Ok(())
}
