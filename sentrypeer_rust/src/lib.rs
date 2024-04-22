#[no_mangle]
pub extern fn display_rust() {
    println!("Greetings from Rust");
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_works() {
        display_rust();
    }
}
