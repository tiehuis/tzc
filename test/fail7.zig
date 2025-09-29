test "op_call 1" {
    try @import("std").nothing(
        \\y()
        \\
    );
}
