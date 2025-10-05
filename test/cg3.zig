extern fn printf([*c]const c_char, ...) c_int;

pub fn main() c_int {
    _ = printf("Hello %s!\n", "World");
}
