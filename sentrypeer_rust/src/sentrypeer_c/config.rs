/* automatically generated by rust-bindgen 0.69.4 */

pub type pthread_t = ::std::os::raw::c_ulong;
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct dht_infohash {
    pub d: [u8; 20usize],
}
#[test]
fn bindgen_test_layout_dht_infohash() {
    const UNINIT: ::std::mem::MaybeUninit<dht_infohash> = ::std::mem::MaybeUninit::uninit();
    let ptr = UNINIT.as_ptr();
    assert_eq!(
        ::std::mem::size_of::<dht_infohash>(),
        20usize,
        concat!("Size of: ", stringify!(dht_infohash))
    );
    assert_eq!(
        ::std::mem::align_of::<dht_infohash>(),
        1usize,
        concat!("Alignment of ", stringify!(dht_infohash))
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).d) as usize - ptr as usize },
        0usize,
        concat!(
            "Offset of field: ",
            stringify!(dht_infohash),
            "::",
            stringify!(d)
        )
    );
}
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct dht_op_token {
    _unused: [u8; 0],
}
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct dht_runner {
    _unused: [u8; 0],
}
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct sentrypeer_config {
    pub api_mode: bool,
    pub bgp_agent_mode: bool,
    pub debug_mode: bool,
    pub json_log_mode: bool,
    pub p2p_dht_mode: bool,
    pub sip_agent_mode: bool,
    pub sip_mode: bool,
    pub sip_responsive_mode: bool,
    pub syslog_mode: bool,
    pub verbose_mode: bool,
    pub webhook_mode: bool,
    pub webhook_url: *mut ::std::os::raw::c_char,
    pub oauth2_mode: bool,
    pub oauth2_client_id: *mut ::std::os::raw::c_char,
    pub oauth2_client_secret: *mut ::std::os::raw::c_char,
    pub oauth2_access_token: *mut ::std::os::raw::c_char,
    pub db_file: *mut ::std::os::raw::c_char,
    pub json_log_file: *mut ::std::os::raw::c_char,
    pub node_id: *mut ::std::os::raw::c_char,
    pub p2p_bootstrap_node: *mut ::std::os::raw::c_char,
    pub sip_daemon_thread: pthread_t,
    pub http_daemon: *mut MHD_Daemon,
    pub dht_node: *mut dht_runner,
    pub dht_info_hash: *mut dht_infohash,
    pub dht_op_token: *mut dht_op_token,
}
#[test]
fn bindgen_test_layout_sentrypeer_config() {
    const UNINIT: ::std::mem::MaybeUninit<sentrypeer_config> = ::std::mem::MaybeUninit::uninit();
    let ptr = UNINIT.as_ptr();
    assert_eq!(
        ::std::mem::size_of::<sentrypeer_config>(),
        128usize,
        concat!("Size of: ", stringify!(sentrypeer_config))
    );
    assert_eq!(
        ::std::mem::align_of::<sentrypeer_config>(),
        8usize,
        concat!("Alignment of ", stringify!(sentrypeer_config))
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).api_mode) as usize - ptr as usize },
        0usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(api_mode)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).bgp_agent_mode) as usize - ptr as usize },
        1usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(bgp_agent_mode)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).debug_mode) as usize - ptr as usize },
        2usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(debug_mode)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).json_log_mode) as usize - ptr as usize },
        3usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(json_log_mode)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).p2p_dht_mode) as usize - ptr as usize },
        4usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(p2p_dht_mode)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).sip_agent_mode) as usize - ptr as usize },
        5usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(sip_agent_mode)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).sip_mode) as usize - ptr as usize },
        6usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(sip_mode)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).sip_responsive_mode) as usize - ptr as usize },
        7usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(sip_responsive_mode)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).syslog_mode) as usize - ptr as usize },
        8usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(syslog_mode)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).verbose_mode) as usize - ptr as usize },
        9usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(verbose_mode)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).webhook_mode) as usize - ptr as usize },
        10usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(webhook_mode)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).webhook_url) as usize - ptr as usize },
        16usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(webhook_url)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).oauth2_mode) as usize - ptr as usize },
        24usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(oauth2_mode)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).oauth2_client_id) as usize - ptr as usize },
        32usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(oauth2_client_id)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).oauth2_client_secret) as usize - ptr as usize },
        40usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(oauth2_client_secret)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).oauth2_access_token) as usize - ptr as usize },
        48usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(oauth2_access_token)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).db_file) as usize - ptr as usize },
        56usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(db_file)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).json_log_file) as usize - ptr as usize },
        64usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(json_log_file)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).node_id) as usize - ptr as usize },
        72usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(node_id)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).p2p_bootstrap_node) as usize - ptr as usize },
        80usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(p2p_bootstrap_node)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).sip_daemon_thread) as usize - ptr as usize },
        88usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(sip_daemon_thread)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).http_daemon) as usize - ptr as usize },
        96usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(http_daemon)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).dht_node) as usize - ptr as usize },
        104usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(dht_node)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).dht_info_hash) as usize - ptr as usize },
        112usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(dht_info_hash)
        )
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).dht_op_token) as usize - ptr as usize },
        120usize,
        concat!(
            "Offset of field: ",
            stringify!(sentrypeer_config),
            "::",
            stringify!(dht_op_token)
        )
    );
}
extern "C" {
    pub fn sentrypeer_config_new() -> *mut sentrypeer_config;
}
extern "C" {
    pub fn sentrypeer_config_destroy(self_ptr: *mut *mut sentrypeer_config);
}
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct MHD_Daemon {
    pub _address: u8,
}
