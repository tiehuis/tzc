const std = @import("std");

pub const keywords: std.StaticStringMap(u32) = .initComptime(.{
    .{ "and", .kw_and },
});
