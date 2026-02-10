extern fn printf([*c]const c_char, ...) c_int;

fn tryAdd(a: u8, b: u8) u8 {
    return if (a == 0) 0 else a + b;
}

pub fn main() c_int {
    _ = printf("%d\n", tryAdd(2, 3));
    return 0;
}
