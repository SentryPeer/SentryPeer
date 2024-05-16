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

pub mod tls;

/// Return libc::EXIT_SUCCESS or libc::EXIT_FAILURE depending on the function argument
#[no_mangle]
pub extern "C" fn return_exit_status(success: bool) -> i32 {
    if success {
        libc::EXIT_SUCCESS
    } else {
        libc::EXIT_FAILURE
    }
}

/// The simplest function used to confirm that calling our Rust library from C is working
#[no_mangle]
pub extern "C" fn display_rust() {
    println!("Greetings from Rust");
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_works() {
        display_rust();
    }

    #[test]
    fn test_return_exit_status() {
        assert_eq!(return_exit_status(true), libc::EXIT_SUCCESS);
        assert_eq!(return_exit_status(false), libc::EXIT_FAILURE);
    }
    
    // #[test]
    // fn test_tls_listen() {
    //     tls::listen().unwrap();
    // }
}
