use blpapi::{event::EventType, session_options::SessionOptions, Error};

pub fn main() -> Result<(), Error> {
    let mut session = SessionOptions::new()
        .with_server_host("localhost")?
        .with_server_port(3189)?
        .sync();

    let service = session
        .get_service("/blp/refdata")
        .expect("Can't find refdata service");
    let mut request = service.create_request("ReferenceDataRequest")?;

    // append securities
    request.append("securities", "IBM US Equity".to_owned())?;
    request.append("securities", "MSFT US Equity".to_owned())?;

    // append fields
    request.append("fields", "PX_LAST".to_owned())?;
    request.append("fields", "DS002".to_owned())?;

    // send request and get it's newly created id
    let _id = session.send(request, None)?;

    let timeout = None;
    let mut event_type = EventType::Unknown;
    while event_type != EventType::Response {
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
