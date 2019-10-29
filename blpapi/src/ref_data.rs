use crate::element::Element;

/// A trait to convert reference data element fields into a struct
pub trait RefData: Default {
    const FIELDS: &'static [&'static str];
    fn on_field(&mut self, field: &str, element: &Element);
}
