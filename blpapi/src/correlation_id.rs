use blpapi_sys::*;
use std::os::raw::c_uint;

const DEFAULT_CLASS_ID: c_uint = 0;

/// A Correlation Id
pub struct CorrelationId(pub(crate) blpapi_CorrelationId_t);

impl CorrelationId {
    pub fn new_u64(value: u64) -> Self {
        //TODO: Check!

        let size = std::mem::size_of::<blpapi_CorrelationId_t>() as c_uint;
        let value_type = BLPAPI_CORRELATION_TYPE_INT;
        let class_id = DEFAULT_CLASS_ID;
        let reserved = 0;
        let _bitfield_1 =
            blpapi_CorrelationId_t_::new_bitfield_1(size, value_type, class_id, reserved);
        let value = blpapi_CorrelationId_t___bindgen_ty_1 { intValue: value };

        let inner = blpapi_CorrelationId_t_ { value, _bitfield_1 };
        CorrelationId(inner)
    }
}

#[test]
fn correlation_u64() {
    let id = CorrelationId::new_u64(1);
    assert_eq!(unsafe { id.0.value.intValue }, 1);
}
