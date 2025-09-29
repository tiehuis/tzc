const Node = opaque {};
const Operand = opaque {};
const Error = opaque {};

fn genPrefixExpr(pe: Node.PrefixExpr) Error!Operand.RK {
    const nr: Operand.RK = blk: switch (pe.base) {
        else => break :blk .{},
    };
    return nr;
}
