use crate::{
    element::{Element, SetValue},
    name::Name,
    service::Service,
    try_, Error,
};
use blpapi_sys::*;
use std::ffi::CString;
use std::os::raw::c_int;

pub struct Datetime(pub(crate) blpapi_Datetime_t);

impl Default for Datetime {
    fn default() -> Self {
        Datetime(blpapi_Datetime_t {
            parts: 0,
            hours: 0,
            minutes: 0,
            seconds: 0,
            milliSeconds: 0,
            month: 0,
            day: 0,
            year: 0,
            offset: 0,
        })
    }
}

pub enum DatetimeParts {
    Year,
    Month,
    Day,
    Offset,
    Hours,
    Minutes,
    Seconds,
    FracSeconds,
    Milliseconds,
    Date,
    Time,
    TimeFracSeconds,
    Unknown,
}

impl From<c_int> for DatetimeParts {
    fn from(e: c_int) -> Self {
        match e as u32 {
            BLPAPI_DATETIME_YEAR_PART => DatetimeParts::Year,
            BLPAPI_DATETIME_MONTH_PART => DatetimeParts::Month,
            BLPAPI_DATETIME_DAY_PART => DatetimeParts::Day,
            BLPAPI_DATETIME_OFFSET_PART => DatetimeParts::Offset,
            BLPAPI_DATETIME_HOURS_PART => DatetimeParts::Hours,
            BLPAPI_DATETIME_MINUTES_PART => DatetimeParts::Minutes,
            BLPAPI_DATETIME_SECONDS_PART => DatetimeParts::Seconds,
            BLPAPI_DATETIME_FRACSECONDS_PART => DatetimeParts::FracSeconds,
            BLPAPI_DATETIME_MILLISECONDS_PART => DatetimeParts::Milliseconds,
            BLPAPI_DATETIME_DATE_PART => DatetimeParts::Date,
            BLPAPI_DATETIME_TIME_PART => DatetimeParts::Time,
            BLPAPI_DATETIME_TIMEFRACSECONDS_PART => DatetimeParts::TimeFracSeconds,
            _ => DatetimeParts::Unknown,
        }
    }
}

/// Is Leap Year
pub fn is_leap_year(y: c_int) -> bool {
    y % 4 == 0 && (y <= 1752 || y % 100 != 0 || y % 400 == 0)
}
