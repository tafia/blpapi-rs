use crate::{datetime::Datetime, name::Name, try_, Error};
use blpapi_sys::*;
use std::{
    ffi::{CStr, CString},
    marker::PhantomData,
    os::raw::c_int,
    ptr,
};

/// An element
pub struct Element {
    pub(crate) ptr: *mut blpapi_Element_t,
}

impl Element {
    unsafe fn opt(res: c_int, ptr: *mut blpapi_Element_t) -> Option<Self> {
        if res == 0 {
            Some(Element { ptr })
        } else {
            log::warn!("cannot find element: '{}'", res);
            None
        }
    }

    /// name
    pub fn string_name(&self) -> String {
        self.name().to_string_lossy().into_owned()
    }

    /// name
    pub fn name(&self) -> Name {
        let name = unsafe { blpapi_Element_name(self.ptr) };
        Name(name)
    }

    /// Has element
    pub fn has_element(&self, name: &str) -> bool {
        let name = CString::new(name).unwrap();
        let named = ptr::null();
        unsafe { blpapi_Element_hasElement(self.ptr, name.as_ptr(), named) != 0 }
    }

    /// Has element
    pub fn has_named_element(&self, named: &Name) -> bool {
        let name = ptr::null();
        unsafe { blpapi_Element_hasElement(self.ptr, name, named.0) != 0 }
    }

    /// Number of values
    pub fn num_values(&self) -> usize {
        unsafe { blpapi_Element_numValues(self.ptr) }
    }

    /// Number of elements
    pub fn num_elements(&self) -> usize {
        unsafe { blpapi_Element_numElements(self.ptr) }
    }

    /// Get element from its name
    pub fn get_element(&self, name: &str) -> Option<Element> {
        unsafe {
            let mut element = ptr::null_mut();
            let name = CString::new(name).unwrap();
            let res = blpapi_Element_getElement(
                self.ptr,
                &mut element as *mut _,
                name.as_ptr(),
                ptr::null(),
            );
            Element::opt(res, element)
        }
    }

    /// Get element from its name
    pub fn get_named_element(&self, named_element: &Name) -> Option<Element> {
        unsafe {
            let mut element = ptr::null_mut();
            let res = blpapi_Element_getElement(
                self.ptr,
                &mut element as *mut _,
                ptr::null(),
                named_element.0,
            );
            Element::opt(res, element)
        }
    }

    /// Get element at index
    pub fn get_element_at(&self, index: usize) -> Option<Element> {
        unsafe {
            let mut element = ptr::null_mut();
            let res = blpapi_Element_getElementAt(self.ptr, &mut element as *mut _, index);
            Element::opt(res, element)
        }
    }

    /// Append a new element
    pub fn append_element(&mut self) -> Result<Element, Error> {
        unsafe {
            let mut ptr = ptr::null_mut();
            try_(blpapi_Element_appendElement(self.ptr, &mut ptr as *mut _))?;
            Ok(Element { ptr })
        }
    }

    /// Append a new element with `value`
    pub fn append<V: SetValue>(&mut self, value: V) -> Result<(), Error> {
        value.append_to(self)
    }

    /// Get value at given index
    pub fn get_at<V: GetValue>(&self, index: usize) -> Option<V> {
        V::get_at(self, index)
    }

    /// Set value at given index
    pub fn set_at<V: SetValue>(&mut self, index: usize, value: V) -> Result<(), Error> {
        value.set_at(self, index)
    }

    /// Set value from element defined by name
    pub fn set<V: SetValue>(&mut self, name: &str, value: V) -> Result<(), Error> {
        value.set(self, name)
    }

    /// Set value from named element
    pub fn set_named<V: SetValue>(&mut self, name: &Name, value: V) -> Result<(), Error> {
        value.set_named(self, name)
    }

    /// Get an element value
    pub fn element_value<V: GetValue>(&self, element: &str) -> Option<V> {
        self.get_element(element)?.value()
    }

    /// Get current element value (index at 0)
    pub fn value<V: GetValue>(&self) -> Option<V> {
        self.get_at(0)
    }

    /// Get an iterator over the values
    pub fn values<V: GetValue>(&self) -> Values<V> {
        Values {
            len: self.num_values(),
            element: self,
            i: 0,
            _phantom: PhantomData,
        }
    }

    /// Get an iterator over the elements
    pub fn elements(&self) -> Elements {
        Elements {
            len: self.num_elements(),
            element: self,
            i: 0,
        }
    }
}

/// A trait to represent an Element value
pub trait GetValue: Sized {
    /// Get value from elements by index
    fn get_at(element: &Element, index: usize) -> Option<Self>;
}

/// A trait to represent an Element value
pub trait SetValue: Sized {
    /// Set value from elements at index
    fn set_at(self, element: &mut Element, index: usize) -> Result<(), Error>;
    /// Set value from element at name
    fn set(self, element: &mut Element, name: &str) -> Result<(), Error>;
    /// Set value from element at name
    fn set_named(self, element: &mut Element, name: &Name) -> Result<(), Error>;
    /// Append a new value to a element
    ///
    /// Return an error if the element doesn't accept appending
    fn append_to(self, element: &mut Element) -> Result<(), Error> {
        Self::set_at(self, element, BLPAPI_ELEMENT_INDEX_END as usize)
    }
}

macro_rules! impl_value {
    ($ty:ty, $start:expr, $get_at:path, $set_at:path, $set:path) => {
        impl GetValue for $ty {
            fn get_at(element: &Element, index: usize) -> Option<Self> {
                unsafe {
                    let mut tmp = $start;
                    let res = $get_at(element.ptr, &mut tmp as *mut _, index);
                    if res == 0 {
                        Some(tmp)
                    } else {
                        None
                    }
                }
            }
        }
        impl SetValue for $ty {
            fn set_at(self, element: &mut Element, index: usize) -> Result<(), Error> {
                unsafe {
                    let res = $set_at(element.ptr, self, index);
                    try_(res)
                }
            }
            fn set(self, element: &mut Element, name: &str) -> Result<(), Error> {
                unsafe {
                    let named_element = ptr::null();
                    let name = CString::new(name).unwrap();
                    let res = $set(element.ptr, name.as_ptr(), named_element, self);
                    try_(res)
                }
            }
            fn set_named(self, element: &mut Element, named_element: &Name) -> Result<(), Error> {
                unsafe {
                    let name = ptr::null();
                    let res = $set(element.ptr, name, named_element.0, self);
                    try_(res)
                }
            }
        }
    };
    ($ty:ty, $get_at:path, $set_at:path, $set:path, $from_bbg: expr, $to_bbg: expr) => {
        impl GetValue for $ty {
            fn get_at(element: &Element, index: usize) -> Option<Self> {
                unsafe {
                    let tmp = ptr::null_mut();
                    let res = $get_at(element.ptr, tmp, index);
                    if res == 0 {
                        Some($from_bbg(*tmp))
                    } else {
                        None
                    }
                }
            }
        }
        impl SetValue for $ty {
            fn set_at(self, element: &mut Element, index: usize) -> Result<(), Error> {
                unsafe {
                    let res = $set_at(element.ptr, $to_bbg(self), index);
                    try_(res)
                }
            }
            fn set(self, element: &mut Element, name: &str) -> Result<(), Error> {
                unsafe {
                    let named_element = ptr::null();
                    let name = CString::new(name).unwrap();
                    let res = $set(element.ptr, name.as_ptr(), named_element, $to_bbg(self));
                    try_(res)
                }
            }
            fn set_named(self, element: &mut Element, named_element: &Name) -> Result<(), Error> {
                unsafe {
                    let name = ptr::null();
                    let res = $set(element.ptr, name, named_element.0, $to_bbg(self));
                    try_(res)
                }
            }
        }
    };
}

impl_value!(
    i64,
    0,
    blpapi_Element_getValueAsInt64,
    blpapi_Element_setValueInt64,
    blpapi_Element_setElementInt64
);
impl_value!(
    i32,
    0,
    blpapi_Element_getValueAsInt32,
    blpapi_Element_setValueInt32,
    blpapi_Element_setElementInt32
);
impl_value!(
    f64,
    0.,
    blpapi_Element_getValueAsFloat64,
    blpapi_Element_setValueFloat64,
    blpapi_Element_setElementFloat64
);
impl_value!(
    f32,
    0.,
    blpapi_Element_getValueAsFloat32,
    blpapi_Element_setValueFloat32,
    blpapi_Element_setElementFloat32
);
impl_value!(
    i8,
    0,
    blpapi_Element_getValueAsChar,
    blpapi_Element_setValueChar,
    blpapi_Element_setElementChar
);
impl_value!(
    bool,
    blpapi_Element_getValueAsBool,
    blpapi_Element_setValueBool,
    blpapi_Element_setElementBool,
    |bbg: blpapi_Bool_t| bbg != 0,
    |rust| if rust { 1 } else { 0 }
);
impl_value!(
    Name,
    blpapi_Element_getValueAsName,
    blpapi_Element_setValueFromName,
    blpapi_Element_setElementFromName,
    |bbg: *mut blpapi_Name_t| Name(bbg),
    |rust: Name| rust.0
);

impl GetValue for String {
    fn get_at(element: &Element, index: usize) -> Option<Self> {
        unsafe {
            let mut tmp = ptr::null();
            let res = blpapi_Element_getValueAsString(element.ptr, &mut tmp as *mut _, index);
            if res == 0 {
                Some(CStr::from_ptr(tmp).to_string_lossy().into_owned())
            } else {
                None
            }
        }
    }
}

impl<'a> SetValue for &'a str {
    fn set_at(self, element: &mut Element, index: usize) -> Result<(), Error> {
        let value = CString::new(self).unwrap();
        unsafe {
            let res = blpapi_Element_setValueString(element.ptr, value.as_ptr(), index);
            try_(res)
        }
    }
    fn set(self, element: &mut Element, name: &str) -> Result<(), Error> {
        let value = CString::new(self).unwrap();
        unsafe {
            let named_element = ptr::null();
            let name = CString::new(name).unwrap();
            let res = blpapi_Element_setElementString(
                element.ptr,
                name.as_ptr(),
                named_element,
                value.as_ptr(),
            );
            try_(res)
        }
    }
    fn set_named(self, element: &mut Element, named_element: &Name) -> Result<(), Error> {
        let value = CString::new(self).unwrap();
        unsafe {
            let name = ptr::null();
            let res =
                blpapi_Element_setElementString(element.ptr, name, named_element.0, value.as_ptr());
            try_(res)
        }
    }
}

impl GetValue for Datetime {
    fn get_at(element: &Element, index: usize) -> Option<Self> {
        unsafe {
            let mut tmp = Datetime::default();
            let res = blpapi_Element_getValueAsDatetime(element.ptr, &mut tmp.0, index);
            if res == 0 {
                Some(tmp)
            } else {
                None
            }
        }
    }
}

impl<T: GetValue> GetValue for Option<T> {
    fn get_at(element: &Element, index: usize) -> Option<Self> {
        T::get_at(element, index).map(Some)
    }
}

impl<T: GetValue> GetValue for Vec<T> {
    fn get_at(element: &Element, index: usize) -> Option<Self> {
        Some(element.values().skip(index).collect())
    }
}

impl GetValue for Element {
    fn get_at(element: &Element, index: usize) -> Option<Self> {
        unsafe {
            let mut ptr = ptr::null_mut();
            let res = blpapi_Element_getValueAsElement(element.ptr, &mut ptr as *mut _, index);
            if res == 0 {
                Some(Element { ptr })
            } else {
                None
            }
        }
    }
}

impl<T: GetValue + std::hash::Hash + Eq> GetValue for std::collections::HashSet<T> {
    fn get_at(element: &Element, index: usize) -> Option<Self> {
        Some(element.values().skip(index).collect())
    }
}

impl<'a> SetValue for &'a Datetime {
    fn set_at(self, element: &mut Element, index: usize) -> Result<(), Error> {
        unsafe {
            let res = blpapi_Element_setValueDatetime(element.ptr, &self.0 as *const _, index);
            try_(res)
        }
    }
    fn set(self, element: &mut Element, name: &str) -> Result<(), Error> {
        unsafe {
            let named_element = ptr::null();
            let name = CString::new(name).unwrap();
            let res = blpapi_Element_setElementDatetime(
                element.ptr,
                name.as_ptr(),
                named_element,
                &self.0 as *const _,
            );
            try_(res)
        }
    }
    fn set_named(self, element: &mut Element, named_element: &Name) -> Result<(), Error> {
        unsafe {
            let name = ptr::null();
            let res = blpapi_Element_setElementDatetime(
                element.ptr,
                name,
                named_element.0,
                &self.0 as *const _,
            );
            try_(res)
        }
    }
}

/// An iterator over values
pub struct Values<'a, V> {
    element: &'a Element,
    i: usize,
    len: usize,
    _phantom: PhantomData<V>,
}

impl<'a, V: GetValue> Iterator for Values<'a, V> {
    type Item = V;
    fn size_hint(&self) -> (usize, Option<usize>) {
        (self.len - self.i, Some(self.len - self.i))
    }
    fn next(&mut self) -> Option<V> {
        if self.i == self.len {
            return None;
        }
        let v = self.element.get_at(self.i);
        self.i += 1;
        v
    }
}

#[cfg(feature = "dates")]
impl GetValue for chrono::NaiveDate {
    fn get_at(element: &Element, index: usize) -> Option<Self> {
        element.get_at(index).map(|d: Datetime| {
            chrono::NaiveDate::from_ymd(d.0.year as i32, d.0.month as u32, d.0.day as u32)
        })
    }
}

/// An iterator over elements
pub struct Elements<'a> {
    element: &'a Element,
    i: usize,
    len: usize,
}

impl<'a> Iterator for Elements<'a> {
    type Item = Element;
    fn size_hint(&self) -> (usize, Option<usize>) {
        (self.len - self.i, Some(self.len - self.i))
    }
    fn next(&mut self) -> Option<Element> {
        if self.i == self.len {
            return None;
        }
        let v = self.element.get_element_at(self.i);
        self.i += 1;
        v
    }
}
