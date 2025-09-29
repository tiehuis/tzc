pub fn BoundedArrayAligned(
    comptime T: type,
    comptime buffer_capacity: usize,
) type {
    return struct {
        buffer: [buffer_capacity]T align(4) = undefined,
        len: usize = 0,

        pub fn slice(self: anytype) switch (@TypeOf(&self.buffer)) {
            *align(4) [buffer_capacity]T => []align(4) T,
            *align(4) const [buffer_capacity]T => []align(4) const T,
            else => unreachable,
        } {
            return self.buffer[0..self.len];
        }
    };
}
