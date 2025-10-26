extern fn printf([*c]const c_char, ...) c_int;

pub fn main() c_int {
    var i: c_int = 0;
    while (i < 10) : (i += 1) {
        _ = printf("Hello %d!\n", i);
    }
}
