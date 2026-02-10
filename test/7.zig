extern fn printf([*c]const c_char, ...) c_int;

fn tryAdd(a: u8, b: u8) u8 {
    if (a == 0) {
        return 0;
    } else {
        return a + b;
    }
}

pub fn main() c_int {
    _ = printf("%d\n", tryAdd(2, 3));
    return 0;
}
