typedef struct IrRenderer {
    int indent;
    CompileContext *ctx;
} IrRenderer;

#define indent(r) std_printf("%*c", r->indent, ' ')
#define emit(fmt, ...) std_printf(fmt, ## __VA_ARGS__)
#define Buf(_b) Buffer(CompileContext_getString(r->ctx, (_b)))

static void IrRenderer_init(IrRenderer *r, CompileContext *ctx)
{
    r->indent = 0;
    r->ctx = ctx;
}

static void IrRenderer_renderInst(IrRenderer *r, IrInst inst)
{
    indent(r);

    tType ty = CompileContext_getType(r->ctx, inst.ty);
    emit("[%s] ", tTypeTag_Name(ty.tag)); // TODO: should accept tType and render that

    switch (inst.op) {
        case ir_op_call:
            emit("%s %d "PRIb, IrOp_name(inst.op), inst.dst, Buf(inst.data.call.fn.data.sym));
            for (uint8_t i = 0; i < inst.data.call.args_len; i++) {
                emit(" %d", inst.data.call.args[i]);
            }
            emit("\n");
            break;

        case ir_op_invalid:
        case ir_op_unreachable:
            emit("%s\n", IrOp_name(inst.op));
            break;

        case ir_op_negate:
        case ir_op_bw_not:
        case ir_op_bw_and:
        case ir_op_not:
        case ir_op_copy:
            emit("%s %d %d\n", IrOp_name(inst.op), inst.dst, inst.data.unary.lhs);
            break;

        case ir_op_const_num:
        case ir_op_const_char:
            emit("%s %d $%d\n", IrOp_name(inst.op), inst.dst, inst.data.i64);
            break;

        case ir_op_const_bytes:
            emit("%s %d `"PRIb"`\n", IrOp_name(inst.op), inst.dst, Buf(inst.data.bytes));
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
            emit("%s %d %d %d\n", IrOp_name(inst.op), inst.dst, inst.data.binary.lhs, inst.data.binary.rhs);
            break;

        case ir_op_store_var:
            emit("%s v%d %d\n", IrOp_name(inst.op), inst.data.var.id, inst.data.var.value);
            break;

        case ir_op_load_var:
            emit("%s %d v%d\n", IrOp_name(inst.op), inst.dst, inst.data.var.id);
            break;

        case ir_op_load_arg:
            emit("%s v%d "PRIb"\n", IrOp_name(inst.op), inst.data.arg.id, Buf(inst.data.arg.name));
            break;
    }
}

static void IrRenderer_renderTerm(IrRenderer *r, IrTerm term)
{
    indent(r);
    switch (term.tag) {
        case ir_term_br:
            emit("br %d b%d b%d\n", term.data.br.cond, term.data.br.t, term.data.br.f);
            break;

        case ir_term_jmp:
            emit("jmp b%d\n", term.data.jmp.target);
            break;

        case ir_term_ret:
            emit("ret %d\n", term.data.ret.value);
            break;

        case ir_term_next:
            emit("next\n");
    }
}

static void IrRenderer_renderBlock(IrRenderer *r, IrBlockId id, IrBlock *block)
{
    indent(r);
    emit("b%d:\n", id);
    r->indent++;
    for (uint32_t i = 0; i < block->insts.len; i++) {
        IrRenderer_renderInst(r, block->insts.data[i]);
    }
    IrRenderer_renderTerm(r, block->term);
    r->indent--;
}

static void IrRenderer_renderFunc(IrRenderer *r, IrFunc *func)
{
    emit(PRIb":\n", Buf(func->name));
    r->indent++;
    for (uint32_t i = 0; i < func->blocks.len; i++) {
        IrRenderer_renderBlock(r, i, func->blocks.data[i]);
    }
    emit("\n");
    r->indent--;
}

static void IrRenderer_render(IrRenderer *r, IrProgram *ir)
{
    for (uint32_t i = 0; i < ir->funcs.len; i++) {
        IrRenderer_renderFunc(r, ir->funcs.data[i]);
    }
}

#undef emit
#undef indent
#undef Buf