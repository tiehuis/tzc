extern fn printf([*c]const c_char, ...) c_int;

pub fn main() c_int {
    const a: u8 = 1;
    const b: u16 = a + 1;
    const c: u32 = b + 1;
    const d: u64 = c + 1;
    const e: u128 = d + 1;
    _ = printf("%d\n", e);
    return 0;
}
