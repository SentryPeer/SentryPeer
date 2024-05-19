/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 - 2024 Gavin Henry <ghenry@sentrypeer.org> */
/*
   _____            _              _____
  / ____|          | |            |  __ \
 | (___   ___ _ __ | |_ _ __ _   _| |__) |__  ___ _ __
  \___ \ / _ \ '_ \| __| '__| | | |  ___/ _ \/ _ \ '__|
  ____) |  __/ | | | |_| |  | |_| | |  |  __/  __/ |
 |_____/ \___|_| |_|\__|_|   \__, |_|   \___|\___|_|
                              __/ |
                             |___/
*/

use libc::c_char;
use std::ffi::CString;

mod tls;

/// The simplest function used to confirm that calling our Rust library from C is working
#[no_mangle]
pub extern "C" fn display_rust() {
    println!("Greetings from Rust");
}

/// Return libc::EXIT_SUCCESS or libc::EXIT_FAILURE depending on the function argument
#[no_mangle]
pub extern "C" fn return_exit_status(success: bool) -> i32 {
    if success {
        libc::EXIT_SUCCESS
    } else {
        libc::EXIT_FAILURE
    }
}

/// # Safety
///
/// Return a string
///
/// The caller is responsible for freeing the string. Generally, the caller
/// from the C FFI side.
#[no_mangle]
pub extern "C" fn return_string() -> *mut c_char {
    let string = CString::new("Greetings from Rust").unwrap();
    string.into_raw()
}

/// # Safety
///
/// Free the string allocated by into_raw from return_string
#[no_mangle]
pub unsafe extern "C" fn free_string(ptr_s: *mut c_char) {
    unsafe {
        if ptr_s.is_null() {
            return;
        }
        let _ = CString::from_raw(ptr_s);
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::ffi::CStr;

    #[test]
    fn it_works() {
        display_rust();
    }

    #[test]
    fn test_return_exit_status() {
        assert_eq!(return_exit_status(true), libc::EXIT_SUCCESS);
        assert_eq!(return_exit_status(false), libc::EXIT_FAILURE);
    }

    #[test]
    fn test_return_string() {
        let string = unsafe { CStr::from_ptr(return_string()).to_str().unwrap() };
        assert_eq!(string, "Greetings from Rust");
    }

    #[test]
    fn test_free_string() {
        let ptr = return_string();
        unsafe { free_string(ptr); }
    }

    // #[test]
    // fn test_tls_listen() {
    //     tls::listen().unwrap();
    // }
}
