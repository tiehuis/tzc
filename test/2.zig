extern fn puts(s: [*c]const c_char) c_int;

pub fn main() c_int {
    _ = puts("Hello world");
    return 0;
}
