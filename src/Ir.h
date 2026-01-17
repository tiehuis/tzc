// IR. This represents a CFG (Control-Flow-Graph) of the AST, where each node consists
// of a block containing a set of instructions, with a terminator pointing to other blocks.
//
// No comptime evaluation happens here (but will happen using this IR as an input).

typedef uint32_t IrBlockId;
typedef uint32_t IrTempId;
typedef uint32_t IrTypeId;
typedef uint32_t IrVarId;
typedef Node* LazyType;

typedef struct {
    Buffer name; // optional
    LazyType type;
    bool is_varargs; // ugly, remove
} IrNamedType;

#define ir_invalid_id (~(uint32_t)0)

typedef enum {
    ir_val_tmp,
    ir_val_imm64,
    ir_val_sym,
} IrValueTag;

typedef union {
    IrTempId tmp;
    uint64_t val_imm64;
    Buffer sym;
} IrValueData;

typedef struct {
    IrValueTag tag;
    IrValueData data;
} IrValue;

typedef enum {
    // array
    ir_op_call,
    // i64
    ir_op_const_num,
    ir_op_const_char,
    // bytes
    ir_op_const_bytes,
    // arg
    ir_op_load_arg, // TODO: if we used actual names instead of id's may not need
    // tmp
    ir_op_copy,
    // var
    ir_op_load_var,
    ir_op_store_var,
    // unary
    ir_op_negate,
    ir_op_bw_not,
    ir_op_bw_and,
    ir_op_not,
    // binary
    ir_op_or,
    ir_op_and,
    ir_op_eq,
    ir_op_neq,
    ir_op_lt,
    ir_op_gt,
    ir_op_lte,
    ir_op_gte,
    ir_op_bit_and,
    ir_op_bit_xor,
    ir_op_shl,
    ir_op_shr,
    ir_op_add,
    ir_op_sub,
    ir_op_mul,
    ir_op_div,
    ir_op_mod,
    // void
    ir_op_unreachable,
    ir_op_invalid,
} IrOp;

static const char* IrOp_name(IrOp op)
{
    switch (op) {
        case ir_op_call:
            return "call";
        case ir_op_const_num:
            return "load_num";
        case ir_op_const_bytes:
            return "load_bytes";
        case ir_op_const_char:
            return "load_char";
        case ir_op_load_arg:
            return "load_arg";
        case ir_op_copy:
            return "copy";
        case ir_op_load_var:
            return "load_var";
        case ir_op_store_var:
            return "store_var";
        case ir_op_negate:
            return "negate";
        case ir_op_bw_not:
            return "bw_not";
        case ir_op_bw_and:
            return "bw_and";
        case ir_op_not:
            return "not";

        case ir_op_or:
            return "or";
        case ir_op_and:
            return "and";
        case ir_op_eq:
            return "eq";
        case ir_op_neq:
            return "neq";
        case ir_op_lt:
            return "lt";
        case ir_op_gt:
            return "gt";
        case ir_op_lte:
            return "lte";
        case ir_op_gte:
            return "gte";
        case ir_op_bit_and:
            return "bit_and";
        case ir_op_bit_xor:
            return "bit_xor";
        case ir_op_shl:
            return "shl";
        case ir_op_shr:
            return "shr";
        case ir_op_add:
            return "add";
        case ir_op_sub:
            return "sub";
        case ir_op_mul:
            return "mul";
        case ir_op_div:
            return "div";
        case ir_op_mod:
            return "mod";

        case ir_op_unreachable:
            return "unreachable";
        case ir_op_invalid:
            return "!invalid";
    }
}

static IrOp IrOp_FromBinOp(BinOp op)
{
    switch (op) {
        case binop_or:
            return ir_op_or;
        case binop_and:
            return ir_op_and;
        case binop_eq:
            return ir_op_eq;
        case binop_neq:
            return ir_op_neq;
        case binop_lt:
            return ir_op_lt;
        case binop_gt:
            return ir_op_gt;
        case binop_lt_eq:
            return ir_op_lte;
        case binop_gt_eq:
            return ir_op_gte;
        case binop_bit_and:
            return ir_op_bit_and;
        case binop_bit_xor:
            return ir_op_bit_xor;
        case binop_shl:
            return ir_op_shl;
        case binop_shr:
            return ir_op_shr;
        case binop_add:
            return ir_op_add;
        case binop_sub:
            return ir_op_sub;
        case binop_mul:
            return ir_op_mul;
        case binop_div:
            return ir_op_div;
        case binop_mod:
            return ir_op_mod;
        default:
            return ir_op_invalid;
    }
}

typedef union {
    struct { IrValue fn; IrTempId args[16]; uint8_t args_len; } call;
    struct { IrTempId lhs; IrTempId rhs; } binary;
    struct { IrTempId lhs; } unary;
    struct { IrVarId id; IrTempId value; } var;
    struct { IrVarId id; Buffer name; } arg;
    Buffer bytes;
    int64_t i64;
} IrOpData;

typedef struct {
    int dst;
    int ty;
    IrOp op;
    IrOpData data;
} IrInst;

DEFINE_ARRAY(IrInst);

typedef enum {
    ir_term_jmp,
    ir_term_br,
    ir_term_ret,
    ir_term_next,
} IrTermTag;

typedef union {
    struct { IrBlockId target; } jmp;
    struct { IrTempId cond; IrBlockId t; IrBlockId f; } br;
    struct { IrTempId value; } ret;
} IrTermData;

typedef struct {
    IrTermTag tag;
    IrTermData data;
} IrTerm;

typedef struct {
    IrInstArray insts;
    IrTerm term;
} IrBlock;

void IrBlock_init(IrBlock *block)
{
    IrInstArray_init(&block->insts);
    block->term.tag = ir_term_next;
}

typedef struct {
    Buffer name;
    LazyType ty;
} IrVar;

DEFINE_ARRAY(IrNamedType);
DEFINE_ARRAY(IrVar);
DEFINE_ARRAY_NAMED(IrBlock*, IrBlock);

typedef struct {
    Buffer name;
    bool is_static;
    LazyType ret_ty;
    IrBlockArray blocks;
    IrNamedTypeArray call_args;
    IrVarArray vars;
    IrTempId next_temp;
    DeclModifiers modifiers;
} IrFunc;

void IrFunc_Init(IrFunc *func)
{
    IrBlockArray_init(&func->blocks);
    IrNamedTypeArray_init(&func->call_args);
    IrVarArray_init(&func->vars);
    func->next_temp = 0;
}
DEFINE_ARRAY_NAMED(IrFunc*, IrFunc);

typedef struct {
    IrFuncArray funcs;
} IrProgram;

void IrProgram_init(IrProgram *p)
{
    IrFuncArray_init(&p->funcs);
}

typedef struct {
    IrProgram p;
    size_t ir_count;

    IrFunc *func;   // active func
    IrBlock *block; // active block

    // stack for current control flow we are in (e.g. loop)
} Ir;

static IrTempId Ir_newTemp(Ir *ir)
{
    return ir->func->next_temp++;
}

static IrBlockId Ir_newBlock(Ir *ir)
{
    IrBlock *b = std_malloc(sizeof(IrBlock));
    if (!b) std_panic("oom\n");
    IrBlock_init(b);
    uint32_t id = ir->func->blocks.len;
    IrBlockArray_append(&ir->func->blocks, b);
    return id;
}

static void Ir_setBlock(Ir *ir, IrBlockId id)
{
    ir->block = ir->func->blocks.data[id];
}

// terminate a block, and add it to the current function
// leaves the current active block as NULL. Use Ir_newBlock
// to construct and set a new block to active.
static void Ir_terminateBlock(Ir *ir, IrTerm term)
{
    ir->block->term = term;
    ir->block = NULL;
}

static void Ir_termJmp(Ir *ir, IrBlockId to)
{
    IrTerm term = {
        .tag = ir_term_jmp,
        .data = { .jmp = { .target = to } },
    };
    Ir_terminateBlock(ir, term);
}

static void Ir_termRet(Ir *ir, IrTempId value)
{
    IrTerm term = {
        .tag = ir_term_ret,
        .data = { .ret = { .value = value } },
    };
    Ir_terminateBlock(ir, term);
}

static void Ir_termBr(Ir *ir, IrTempId cond, IrBlockId t, IrBlockId f)
{
    IrTerm term = {
        .tag = ir_term_br,
        .data = { .br = { .cond = cond, .t = t, .f = f }},
    };
    Ir_terminateBlock(ir, term);
}

static void Ir_init(Ir *ir)
{
    IrProgram_init(&ir->p);
    ir->ir_count = 0;
    ir->func = NULL;
    ir->block = NULL;
}

// append an instruction to the current block, returning the dst temp id
static IrTempId Ir_appendInst(Ir *ir, IrInst inst)
{
    ir->ir_count += 1;
    IrInstArray_append(&ir->block->insts, inst);
    return inst.dst;
}
static IrVarId Ir_appendVar(Ir *ir, IrVar var)
{
    IrVarArray_append(&ir->func->vars, var);
    return ir->func->vars.len - 1;
}

static void Ir_emitStoreVar(Ir *ir, IrVarId var, IrTempId value)
{
    IrInst inst = {
        .op = ir_op_store_var,
        .dst = ir_invalid_id,
        .ty = 0,
        .data = { .var = { .id = var, .value = value } },
    };
    Ir_appendInst(ir, inst);
}

static IrTempId Ir_emitLoadVar(Ir *ir, IrVarId var)
{
    IrInst inst = {
        .op = ir_op_load_var,
        .dst = Ir_newTemp(ir),
        .ty = 0,
        .data = { .var = { .id = var, .value = ir_invalid_id } },
    };
    return Ir_appendInst(ir, inst);
}

static IrTempId Ir_lowerExpr(Ir *ir, Node *expr);
static void Ir_lowerBlock(Ir *ir, NodeDataBlock block);
static void Ir_lowerStatementExpr(Ir *ir, Node *statement_or_expr);
static IrVarId Ir_InternVar(Ir *ir, Buffer name);

static IrTempId Ir_lowerPrimaryTypeExpr(Ir *ir, NodeDataPrimaryTypeExpr primary_type_expr)
{
    IrInst inst;
    inst.dst = Ir_newTemp(ir);
    inst.ty = 0;

    switch (primary_type_expr.tag) {
        case node_primary_type_number_literal:
            inst.op = ir_op_const_num;
            inst.data.i64 = Buffer_toInt(primary_type_expr.data.raw, 10);
            break;

        case node_primary_type_identifier:
            inst.op = ir_op_load_var;
            inst.data.var.id = Ir_InternVar(ir, primary_type_expr.data.raw);
            break;

        case node_primary_type_char_literal:
            inst.op = ir_op_const_char;
            assume(primary_type_expr.data.raw.len == 0);
            inst.data.i64 = primary_type_expr.data.raw.data[0];
            break;

        case node_primary_type_string_literal:
            inst.op = ir_op_const_bytes;
            inst.data.bytes = primary_type_expr.data.raw;
            break;

        case node_primary_type_unreachable:
            inst.op = ir_op_unreachable;
            break;

        case node_primary_type_builtin:
        case node_primary_type_container_decl:
        case node_primary_type_dot_identifier:
        case node_primary_type_dot_initlist:
        case node_primary_type_error_set_decl:
        case node_primary_type_grouped_expr:
        case node_primary_type_labeled_type_expr:
        case node_primary_type_if_type_expr:
        case node_primary_type_comptime_type_expr:
        case node_primary_type_error:
        case node_primary_type_anyframe:
            std_panic("unsupported tag: %s\n", NodePrimaryTypeTag_name(primary_type_expr.tag));

        default:
            assume(false);
    }

    return Ir_appendInst(ir, inst);
}

static IrTempId Ir_lowerTypeExpr(Ir *ir, NodeDataTypeExpr expr)
{
    NodeDataErrorUnionExpr error_union_expr = expr.type_expr->data.error_union_expr;
    assume(error_union_expr.error_type_expr == NULL);
    assume(error_union_expr.suffix_expr->tag == node_suffix_expr);
    NodeDataSuffixExpr suffix_expr = error_union_expr.suffix_expr->data.suffix_expr;
    assume(suffix_expr.expr->tag == node_primary_type_expr);

    // This is a bit of a hack
    if (suffix_expr.suffixes_len == 0) {
        return Ir_lowerPrimaryTypeExpr(ir, suffix_expr.expr->data.primary_type_expr);
    }

    IrTempId dst = Ir_newTemp(ir);

    for (uint32_t i = 0; i < suffix_expr.suffixes_len; i++) {
        Node *s = suffix_expr.suffixes[i];
        switch (s->tag) {
            case node_fn_call_arguments:
            {
                // assumes this is a base type
                IrValue value = {
                    .tag = ir_val_sym,
                    .data = { .sym = suffix_expr.expr->data.primary_type_expr.data.raw },
                };

                if (s->data.fn_call_arguments.exprs_len > 16) {
                    std_panic("call supports 16 arguments max\n");
                }

                IrInst call = {
                    .op = ir_op_call,
                    .dst = dst,
                    .ty = 0,
                    .data = { .call = { .fn = value, .args_len = s->data.fn_call_arguments.exprs_len } },
                };

                for (uint32_t i = 0; i < s->data.fn_call_arguments.exprs_len; i++) {
                    call.data.call.args[i] = Ir_lowerExpr(ir, s->data.fn_call_arguments.exprs[i]);
                }

                Ir_appendInst(ir, call);
            }
            break;

            default:
                assume(false);
        }
    }

    return dst;
}

static IrTempId Ir_lowerPrimaryExpr(Ir *ir, Node *expr)
{
    switch (expr->tag) {
        case node_if_expr:
        {
            NodeDataIfExpr if_expr = expr->data.if_expr;
            assume(if_expr.else_payload_name.len == 0);

            IrBlockId b_if = Ir_newBlock(ir);
            IrBlockId b_else = Ir_newBlock(ir);
            IrBlockId b_next = Ir_newBlock(ir);

            IrTempId dst = Ir_newTemp(ir);
            IrInst inst3 = {
                .op = ir_op_const_num,
                .ty = 0,
                .dst = dst,
                .data = { .i64 = 0 },
            };
            Ir_appendInst(ir, inst3);

            IrTempId cond = Ir_lowerExpr(ir, if_expr.condition);
            Ir_termBr(ir, cond, b_if, b_else);

            Ir_setBlock(ir, b_if);
            IrInst inst1 = {
                .op = ir_op_copy,
                .ty = 0,
                .dst = dst,
                .data = { .unary = { .lhs = Ir_lowerExpr(ir, if_expr.expr) } },
            };
            Ir_appendInst(ir, inst1);
            Ir_termJmp(ir, b_next);

            Ir_setBlock(ir, b_else);
            IrInst inst2 = {
                .op = ir_op_copy,
                .ty = 0,
                .dst = dst,
                .data = { .unary = { .lhs = Ir_lowerExpr(ir, if_expr.else_payload_expr) } },
            };
            Ir_appendInst(ir, inst2);
            Ir_termJmp(ir, b_next);

            Ir_setBlock(ir, b_next);
            return dst;
        }
        break;

        case node_return_expr:
        {
            IrTempId value = Ir_lowerExpr(ir, expr->data.return_expr);
            Ir_termRet(ir, value);
            Ir_setBlock(ir, Ir_newBlock(ir));   // TODO: Make on demand
            return value;
        }
        break;

        case node_type_expr:
        {
            return Ir_lowerTypeExpr(ir, expr->data.type_expr);
        }
        break;

        case node_asm_expr:
        case node_break_expr:
        case node_comptime_expr:
        case node_nosuspend_expr:
        case node_continue_expr:
        case node_resume_expr:
            std_panic("unimplemented tag: %s\n", NodeTag_name(expr->tag));

        default:
            std_panic("unsupported tag: %s\n", NodeTag_name(expr->tag));
    }
}

static IrTempId Ir_lowerUnaryExpr(Ir *ir, NodeDataUnaryExpr unary_expr)
{
    IrTempId inner = Ir_lowerPrimaryExpr(ir, unary_expr.expr);

    // passthrough if simple op
    if (unary_expr.ops_len == 0) return inner;

    IrInst inst = {
        .op = ir_op_invalid,
        .ty = 0,
    };

    // emit instructions during each loop
    for (uint32_t i = 0; i < unary_expr.ops_len; i++) {
        switch (unary_expr.ops[i]) {
            case token_minus:
                inst.op = ir_op_negate;
                break;
            case token_tilde:
                inst.op = ir_op_bw_not;
                break;
            case token_bang:
                inst.op = ir_op_not;
                break;
            case token_ampersand:
                inst.op = ir_op_bw_and;
                break;

            case token_minus_percent:
            case token_keyword_try:
                std_panic("unimplemented tag: %s\n", TokenTag_name(unary_expr.ops[i]));

            default:
                assume(false);
        }

        inst.dst = Ir_newTemp(ir);
        inst.data.unary.lhs = inner;
        inner = Ir_appendInst(ir, inst);
    }

    return inner;
}

static IrTempId Ir_lowerBinaryExpr(Ir *ir, NodeDataBinaryExpr binary_expr)
{
    IrInst inst = {
        .op = IrOp_FromBinOp(binary_expr.op),
        .dst = Ir_newTemp(ir),
        .ty = 0,
        .data = { .binary = {
            .lhs = Ir_lowerExpr(ir, binary_expr.lhs),
            .rhs = Ir_lowerExpr(ir, binary_expr.rhs),
        } },
    };

    return Ir_appendInst(ir, inst);
}

static IrTempId Ir_lowerExpr(Ir *ir, Node *expr)
{
    switch (expr->tag) {
        case node_unary_expr:
            return Ir_lowerUnaryExpr(ir, expr->data.unary_expr);

        case node_binary_expr:
            return Ir_lowerBinaryExpr(ir, expr->data.binary_expr);

        case node_if_prefix:
        {
            NodeDataIfPrefix if_prefix = expr->data.if_prefix;
            assume(if_prefix.ptr_payload == NULL);
            return Ir_lowerExpr(ir, if_prefix.condition);
        }
        break;

        default:
            std_panic("unsupported tag: %s\n", NodeTag_name(expr->tag));
    }
}

static IrVarId Ir_LookupVar(Ir *ir, Buffer name)
{
    for (uint32_t i = 0; i < ir->func->vars.len; i++) {
        Buffer a = ir->func->vars.data[i].name;
        if (name.len != a.len) goto next_loop;
        for (uint32_t j = 0; j < name.len; j++) {
            if (a.data[j] != name.data[j]) goto next_loop;
        }
        return i;
next_loop:;
    }
    return ir_invalid_id;
}
static IrVarId Ir_InternVar(Ir *ir, Buffer name)
{
    IrVarId id = Ir_LookupVar(ir, name);
    if (id == ir_invalid_id) {
        id = Ir_appendVar(ir, (IrVar){ .name = name, .ty = NULL });
    }
    return id;
}

static Buffer Sema_evalTypeName(Node *n);
static void Ir_lowerAssignExpr(Ir *ir, NodeDataSingleAssignExpr e)
{
    Buffer name = Sema_evalTypeName(e.lhs);
    IrVarId id = Ir_InternVar(ir, name);

    switch (e.assign_op) {
        case token_equal:
        {
            IrTempId expr = Ir_lowerExpr(ir, e.rhs);
            Ir_emitStoreVar(ir, id, expr);
        }
        break;

        case token_plus_equal:
        {
            IrTempId dst = Ir_newTemp(ir);
            IrTempId lhs = Ir_emitLoadVar(ir, id);
            IrTempId rhs = Ir_lowerExpr(ir, e.rhs);
            IrInst add = {
                .op = ir_op_add,
                .dst = dst,
                .ty = 0,
                .data = { .binary = { .lhs = lhs, .rhs = rhs } },
            };
            Ir_emitStoreVar(ir, id, Ir_appendInst(ir, add));
        }
        break;

        default:
            assume(false);
    }
}

static void Ir_lowerVarDecl(Ir *ir, NodeDataVarDeclStatement vd)
{
    assume(vd.var_decl->tag == node_var_decl_proto);
    assume(vd.var_decl_additional_len == 0);

    IrVar var = {
        .name = vd.var_decl->data.var_decl_proto.name,
        .ty = vd.var_decl->data.var_decl_proto.type, // TODO: empty
    };
    IrVarId id = Ir_appendVar(ir, var);

    Ir_emitStoreVar(ir, id, Ir_lowerExpr(ir, vd.expr));
}

static void Ir_lowerLoop(Ir *ir, NodeDataLoopStatement loop)
{
    assume(loop.is_inline == false);
    switch (loop.statement->tag) {
        case node_while_statement:
        {
            NodeDataWhileStatement while_stmt = loop.statement->data.while_statement;
            NodeDataWhilePrefix while_prefix = while_stmt.condition->data.while_prefix;
            assume(while_prefix.ptr_payload == NULL);
            assume(while_stmt.else_statement == NULL);

            IrBlockId block_cond = Ir_newBlock(ir);
            IrBlockId block = Ir_newBlock(ir);
            IrBlockId cont_expr = Ir_newBlock(ir);
            IrBlockId next = Ir_newBlock(ir);

            Ir_setBlock(ir, block_cond);
            IrTempId cond = Ir_lowerExpr(ir, while_prefix.condition);
            Ir_termBr(ir, cond, block, next);

            Ir_setBlock(ir, block);
            Ir_lowerBlock(ir, while_stmt.block->data.block);
            Ir_termJmp(ir, cont_expr);

            Ir_setBlock(ir, cont_expr);
            Ir_lowerStatementExpr(ir, while_prefix.while_continue_expr);
            Ir_termJmp(ir, block_cond);

            Ir_setBlock(ir, Ir_newBlock(ir));
        }
        break;

        case node_for_statement:
        {
            NodeDataForStatement for_stmt = loop.statement->data.for_statement;
            NodeDataForPrefix for_prefix = for_stmt.condition->data.for_prefix;
            assume(for_stmt.else_statement == NULL);
            NodeDataForArgs for_args = for_prefix.for_args->data.for_args;
            assume(for_args.args_len == 1);

            IrTempId block_cond = Ir_newBlock(ir);
            IrTempId block = Ir_newBlock(ir);
            IrTempId cont_expr = Ir_newBlock(ir);
            IrTempId next = Ir_newBlock(ir);

            NodeDataPayloadList payloads = for_prefix.ptr_list_payload->data.payload_list;
            for (uint32_t i = 0; i < for_args.args_len; i++) {
                NodeDataForItem for_item = for_args.args[i]->data.for_item;
                assume(for_item.is_range);

                if (i < payloads.payloads_len) {
                    IrVarId id = Ir_InternVar(ir, payloads.payloads[i]->data.payload.name);
                    Ir_emitStoreVar(ir, id, Ir_lowerExpr(ir, for_item.for_start));

                    Ir_setBlock(ir, block_cond);
                    IrInst cond_inst = {
                        .op = ir_op_lt,
                        .ty = 0,
                        .dst = Ir_newTemp(ir),
                        .data = { .binary = { .lhs = Ir_emitLoadVar(ir, id), .rhs = Ir_lowerExpr(ir, for_item.for_end) } },
                    };
                    IrTempId cond = Ir_appendInst(ir, cond_inst);
                    Ir_termBr(ir, cond, block, next);

                    Ir_setBlock(ir, block);
                    Ir_lowerBlock(ir, for_stmt.block->data.block);
                    Ir_termJmp(ir, cont_expr);

                    Ir_setBlock(ir, cont_expr);
                    IrTempId dst = Ir_newTemp(ir);
                    IrTempId lhs = Ir_emitLoadVar(ir, id);
                    IrInst one = {
                        .op = ir_op_const_num,
                        .dst = Ir_newTemp(ir),
                        .ty = 0,
                        .data = { .i64 = 1 },
                    };
                    IrTempId rhs = Ir_appendInst(ir, one);
                    IrInst add = {
                        .op = ir_op_add,
                        .dst = dst,
                        .ty = 0,
                        .data = { .binary = { .lhs = lhs, .rhs = rhs } },
                    };
                    Ir_emitStoreVar(ir, id, Ir_appendInst(ir, add));
                    Ir_termJmp(ir, block_cond);
                } else {
                    Ir_setBlock(ir, block);
                    Ir_lowerBlock(ir, for_stmt.block->data.block);
                    Ir_termJmp(ir, block);
                }
            }

            Ir_setBlock(ir, Ir_newBlock(ir));
        }
        break;

        default:
            std_panic("unsupported tag: %s\n", NodeTag_name(loop.statement->tag));
    }
}

static void Ir_lowerStatementExpr(Ir *ir, Node *statement_or_expr)
{
    switch (statement_or_expr->tag) {
        case node_comptime_statement:
            std_panic("unimplemented comptime_statement\n");

        case node_nosuspend_statement:
            std_panic("unimplemented nosuspend_statement\n");

        case node_suspend_statement:
            std_panic("unimplemented suspend_statement\n");

        case node_defer_statement:
            std_panic("unimplemented defer_statement\n");

        case node_errdefer_statement:
            std_panic("unimplemented errdefer_statement\n");

        case node_if_statement:
        {
            NodeDataIfStatement if_stmt = statement_or_expr->data.if_statement;
            assume(if_stmt.condition->tag == node_if_prefix);
            NodeDataIfPrefix if_prefix = if_stmt.condition->data.if_prefix;
            assume(if_prefix.ptr_payload == NULL);

            IrBlockId b_body = Ir_newBlock(ir);
            IrBlockId b_else = Ir_newBlock(ir);
            IrBlockId b_next = Ir_newBlock(ir);

            IrTempId cond = Ir_lowerExpr(ir, if_prefix.condition);
            Ir_termBr(ir, cond, b_body, b_else);

            Ir_setBlock(ir, b_body);
            Ir_lowerBlock(ir, if_stmt.block->data.block);
            Ir_termJmp(ir, b_next);

            Ir_setBlock(ir, b_else);
            if (if_stmt.else_statement != NULL) {
                Ir_lowerStatementExpr(ir, if_stmt.else_statement);
            }
            Ir_termJmp(ir, b_next);
            Ir_setBlock(ir, b_next);
        }
        break;

        case node_labeled_statement:
        {
            NodeDataLabeledStatement labeled_stmt = statement_or_expr->data.labeled_statement;
            assume(labeled_stmt.label.len == 0);
            switch (labeled_stmt.statement->tag) {
                case node_loop_statement:
                    Ir_lowerLoop(ir, labeled_stmt.statement->data.loop_statement);
                    break;
                case node_block:
                    // TODO: re-use the created block from above?
                    Ir_lowerBlock(ir, labeled_stmt.statement->data.block);
                    break;
                case node_switch_expr:
                    assume(false);
                default:
                    std_panic("unsupported tag: %s\n", NodeTag_name(labeled_stmt.statement->tag));
            }
        }
        break;

        case node_var_decl_statement:
            Ir_lowerVarDecl(ir, statement_or_expr->data.var_decl_statement);
            break;

        case node_single_assign_expr:
            Ir_lowerAssignExpr(ir, statement_or_expr->data.single_assign_expr);
            break;

        // simple, non-cfg-block creating instruction
        default:
            Ir_lowerExpr(ir, statement_or_expr);
            break;
    }
}

// A NodeDataBlock differs from a cfg block in that it may produce multiple blocks. The AST concept
// of a block is simple a `{ ... }` listing, whilst the CFG definition is a sequence of instructions
// terminated by a control-flow operation.
//
// assumes the caller has created a new block, the reason this is done
// outside is that a caller may want to generate a prologue.
static void Ir_lowerBlock(Ir *ir, NodeDataBlock block)
{
    for (uint32_t i = 0; i < block.statements_len; i++) {
        Ir_lowerStatementExpr(ir, block.statements[i]);
    }
}

static IrFunc* Ir_lowerFunc(Ir *ir, NodeDataDeclFn fn, bool is_static)
{
    assume(fn.fn_proto->tag == node_fn_proto);
    NodeDataFnProto fn_proto = fn.fn_proto->data.fn_proto;

    IrFunc *func = std_malloc(sizeof(IrFunc));
    if (!func) std_panic("oom\n");
    IrFunc_Init(func);

    ir->func = func;
    ir->func->is_static = is_static;
    ir->func->modifiers = fn.modifiers;
    ir->func->name = fn_proto.name;
    ir->func->ret_ty = fn_proto.return_type;
    if (fn_proto.params) {
        assume(fn_proto.params->tag == node_param_decl_list);
        NodeDataParamDeclList decl_list = fn_proto.params->data.param_decl_list;
        for (uint32_t i = 0; i < decl_list.params_len; i++) {
            assume(decl_list.params[i]->tag == node_param_decl);
            NodeDataParamDecl decl = decl_list.params[i]->data.param_decl;

            IrNamedType ty;
            if (decl.is_varargs) {
                ty.is_varargs = true;
            } else {
                ty.name = decl.identifier;
                ty.type = decl.type;
                ty.is_varargs = false;
            }

            IrNamedTypeArray_append(&ir->func->call_args, ty);
        }
    }

    if (fn.block != NULL) {
        assume(fn.block->tag == node_block);
        Ir_setBlock(ir, Ir_newBlock(ir));

        for (uint32_t i = 0; i < ir->func->call_args.len; i++) {
            IrNamedType arg = ir->func->call_args.data[i];
            IrVar var = { .name = arg.name, .ty = arg.type };
            IrVarId var_id = Ir_appendVar(ir, var);
            IrInst inst = {
                .op = ir_op_load_arg,
                .dst =ir_invalid_id,
                .data = { .arg = { .id = var_id, .name = arg.name } },
            };
            Ir_appendInst(ir, inst);
        }

        Ir_lowerBlock(ir, fn.block->data.block);
    }
    return func;
}

static IrFunc* Ir_lowerDeclFn(Ir *ir, Node *decl)
{
    if (decl->tag == node_top_level_decl) {
        NodeDataTopLevelDecl *top_level_decl = &decl->data.top_level_decl;
        if (top_level_decl->decl->tag == node_decl_fn) {
            NodeDataDeclFn fn = top_level_decl->decl->data.decl_fn;
            return Ir_lowerFunc(ir, fn, !top_level_decl->is_pub);
        }
    }
    return NULL;
}

static IrProgram* Ir_lower(Ir *ir, Node *root)
{
    assume(root->tag == node_container_members);
    NodeDataContainerMembers *m = &root->data.container_members;

    for (uint32_t i = 0; i < m->decls_len; i++) {
        IrFunc *func = Ir_lowerDeclFn(ir, m->decls[i]);
        if (func) IrFuncArray_append(&ir->p.funcs, func);
    }

    return &ir->p;
}