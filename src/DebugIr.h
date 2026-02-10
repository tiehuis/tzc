typedef struct DebugIr {
    int indent;
    Ctx *ctx;
} DebugIr;

static void DebugIr_init(DebugIr *r, Ctx *ctx)
{
    r->indent = 0;
    r->ctx = ctx;
}
static void DebugIr_indent(DebugIr *r)
{
    std_printf("%*c", r->indent, ' ');
}

__attribute__((format(printf, 2, 3)))
static int DebugIr_p(DebugIr *r, const char *fmt, ...)
{
    (void)r;

    va_list args;
    va_start(args, fmt);
    int result = std_vprintf(fmt, args);
    va_end(args);
    return result;
}

static void DebugIr_renderInst(DebugIr *r, IrFunc *func, IrInst inst)
{
    DebugIr_indent(r);
    DebugIr_indent(r);

    if (IrOp_hasDst(inst.op)) {
        IrTemp dst = func->temps.data[inst.dst];
        tType ty = Ctx_getType(r->ctx, dst.type);
        DebugIr_p(r, "[%s] ", tTypeTag_Name(ty.tag)); // TODO: should accept tType and render that
    }

    switch (inst.op) {
        case ir_op_call:
            DebugIr_p(r, "%s %d "PRIb, IrOp_name(inst.op), inst.dst, Ctx_Buffer(r->ctx, inst.data.call.fn.data.sym));
            for (uint8_t i = 0; i < inst.data.call.args_len; i++) {
                DebugIr_p(r, " %d", inst.data.call.args[i]);
            }
            DebugIr_p(r, "\n");
            break;

        case ir_op_invalid:
        case ir_op_unreachable:
            DebugIr_p(r, "%s\n", IrOp_name(inst.op));
            break;

        case ir_op_negate:
        case ir_op_bw_not:
        case ir_op_bw_and:
        case ir_op_not:
        case ir_op_copy:
            DebugIr_p(r, "%s %d %d\n", IrOp_name(inst.op), inst.dst, inst.data.unary.lhs);
            break;

        case ir_op_const_num:
        case ir_op_const_char:
            DebugIr_p(r, "%s %d $%lld\n", IrOp_name(inst.op), inst.dst, inst.data.i64);
            break;

        case ir_op_const_bytes:
            DebugIr_p(r, "%s %d `"PRIb"`\n", IrOp_name(inst.op), inst.dst, Ctx_Buffer(r->ctx, inst.data.bytes));
            break;

        case ir_op_or:
        case ir_op_and:
        case ir_op_eq:
        case ir_op_neq:
        case ir_op_lt:
        case ir_op_gt:
        case ir_op_lte:
        case ir_op_gte:
        case ir_op_bit_and:
        case ir_op_bit_xor:
        case ir_op_shl:
        case ir_op_shr:
        case ir_op_add:
        case ir_op_sub:
        case ir_op_mul:
        case ir_op_div:
        case ir_op_mod:
            DebugIr_p(r, "%s %d %d %d\n", IrOp_name(inst.op), inst.dst, inst.data.binary.lhs, inst.data.binary.rhs);
            break;

        case ir_op_store_var:
            DebugIr_p(r, "%s v%d %d\n", IrOp_name(inst.op), inst.data.var.id, inst.data.var.value);
            break;

        case ir_op_load_var:
            DebugIr_p(r, "%s %d v%d\n", IrOp_name(inst.op), inst.dst, inst.data.var.id);
            break;
    }
}

static void DebugIr_renderTerm(DebugIr *r, IrTerm term)
{
    DebugIr_indent(r);
    switch (term.tag) {
        case ir_term_br:
            DebugIr_p(r, "br %d b%d b%d\n", term.data.br.cond, term.data.br.t, term.data.br.f);
            break;

        case ir_term_jmp:
            DebugIr_p(r, "jmp b%d\n", term.data.jmp.target);
            break;

        case ir_term_ret:
            DebugIr_p(r, "ret %d\n", term.data.ret.value);
            break;

        case ir_term_next:
            DebugIr_p(r, "next\n");
    }
}

static void DebugIr_renderBlock(DebugIr *r, IrFunc *func, IrBlockId id, IrBlock *block)
{
    DebugIr_indent(r);
    DebugIr_p(r, "b%d:\n", id);
    r->indent++;
    for (uint32_t i = 0; i < block->insts.len; i++) {
        DebugIr_renderInst(r, func, block->insts.data[i]);
    }
    DebugIr_renderTerm(r, block->term);
    r->indent--;
}

static void DebugIr_renderFunc(DebugIr *r, IrFunc *func)
{
    DebugIr_p(r, PRIb":\n", Ctx_Buffer(r->ctx, func->name));
    r->indent++;
    for (uint32_t i = 0; i < func->blocks.len; i++) {
        DebugIr_renderBlock(r, func, i, func->blocks.data[i]);
    }
    DebugIr_p(r, "\n");
    r->indent--;
}

static void DebugIr_render(DebugIr *r, IrProgram *ir)
{
    for (uint32_t i = 0; i < ir->funcs.len; i++) {
        DebugIr_renderFunc(r, ir->funcs.data[i]);
    }
}