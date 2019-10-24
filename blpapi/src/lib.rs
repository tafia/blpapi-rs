pub mod correlation_id;
pub mod element;
pub mod event;
pub mod message;
pub mod message_iterator;
pub mod name;
pub mod request;
pub mod service;
pub mod session;
pub mod session_options;

/// Error converted from `c_int`
#[derive(Debug)]
pub struct Error(pub i32);

pub(crate) fn try_(res: i32) -> Result<(), Error> {
    if res == 0 {
        Ok(())
    } else {
        Err(Error(res))
    }
}

impl std::fmt::Display for Error {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> Result<(), std::fmt::Error> {
        write!(f, "{:?}", self)
    }
}

impl std::error::Error for Error {
    fn source(&self) -> Option<&(dyn std::error::Error + 'static)> {
        None
    }
}
