use crate::element::Element;

/// Error converted from `c_int`
#[derive(Debug)]
pub enum Error {
    InternalError,
    InvalidUser,
    NotLoggedIn,
    InvalidDisplay,
    EntitlementRefresh,
    InvalidAuthToken,
    ExpiredAuthToken,
    TokenInUse,
    /// Generic blpapi error return
    Generic(i32),
    /// Some element were not found
    NotFound(String),
    /// A securityError element was found
    Security {
        security: String,
        category: String,
        sub_category: Option<String>,
        message: String,
    },
    /// Timeout event
    TimeOut,
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

impl Error {
    /// Check if response is an error(!=0)
    pub fn check(res: i32) -> Result<(), Error> {
        if res == 0 {
            Ok(())
        } else {
            match res {
                100 => Err(Error::InternalError),
                101 => Err(Error::InvalidUser),
                102 => Err(Error::NotLoggedIn),
                103 => Err(Error::InvalidDisplay),
                105 => Err(Error::EntitlementRefresh),
                106 => Err(Error::InvalidAuthToken),
                107 => Err(Error::ExpiredAuthToken),
                108 => Err(Error::TokenInUse),
                _ => {
                    log::debug!("Unrecognized error code: {}", res);
                    Err(Error::Generic(res))
                }
            }
        }
    }

    /// Create a security error
    pub(crate) fn security(security: String, element: Element) -> Error {
        let category = element
            .get_element("category")
            .and_then(|e| e.get_at(0))
            .unwrap_or_else(|| String::new());
        let sub_category = element.get_element("subcategory").and_then(|e| e.get_at(0));
        let message = element
            .get_element("message")
            .and_then(|e| e.get_at(0))
            .unwrap_or_else(|| String::new());
        Error::Security {
            security,
            category,
            sub_category,
            message,
        }
    }
}
