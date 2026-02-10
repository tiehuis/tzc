extern fn printf([*c]const c_char, ...) c_int;

pub fn main() c_int {
    for (0..10) |i| {
        _ = printf("Hello %d!\n", i);
    }
    return 0;
}
