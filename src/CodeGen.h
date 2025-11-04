typedef struct {
    void *out;
    const char *output_filename;
    const char *zig_lib_dir;
    uint8_t indent;
    Parser *p;

    char *zig_h;
    long zig_h_len;
} CodeGen;

static void CodeGen_init(CodeGen *cg, Parser *p, const char *output_filename, const char *zig_lib_dir)
{
    cg->out = std_createFile(output_filename);
    cg->indent = 0;
    if (!cg->out) std_panic(NULL, "failed to fopen output");

    cg->zig_h = std_readFileRelative(zig_lib_dir, "zig.h", &cg->zig_h_len);
    if (!cg->zig_h) std_panic(NULL, "failed to open %s/zig.h", zig_lib_dir);

    cg->p = p;
}

#define CodeGen_unreachable(cg) CodeGen_fail0(cg, __func__, __LINE__, "unreachable")
#define CodeGen_fail(cg, message, ...) CodeGen_fail0(cg, __func__, __LINE__, message)
#define CodeGen_unsupportedTag(cg, tag) CodeGen_fail1(cg, __func__, __LINE__, tag)
static _Noreturn void CodeGen_fail0(CodeGen *cg, const char *function, int line, const char *message)
{
    (void) cg;
    std_printf("%d:lowering failed: %s: %s\n", line, function, message);
    std_exit(1);
}
static _Noreturn void CodeGen_fail1(CodeGen *cg, const char *function, int line, NodeTag tag)
{
    (void) cg;
    std_printf("%d:lowering failed: %s: unsupported node tag %s\n", line, function, NodeTag_name(tag));
    std_exit(1);
}

#define CodeGen_expect(cg, condition) CodeGen_expect0(cg, condition, __func__, __LINE__, #condition)
#define CodeGen_expect2(cg, condition, message) CodeGen_expect0(cg, condition, __func__, __LINE__, message)
static void CodeGen_expect0(CodeGen *cg, bool condition, const char *function, int line, const char *message)
{
    if (!condition) CodeGen_fail0(cg, function, line, message);
}

#define emit(fmt, ...) std_fprintf(cg->out, fmt, ## __VA_ARGS__)
#define indent() std_fprintf(cg->out, "%*c", 4 * cg->indent, ' ');

static size_t std_strlen(const char *s)
{
    size_t i = 0;
    while (*s++) i++;
    return i;
}

static Buffer Sema_resolveTypeName(CodeGen *cg, Buffer b)
{
    (void)cg;

    typedef struct {
        char *zig;
        char *c;
    } TypeMapping;

    // TODO: non-power-of-two types
    // TODO: name-mangling for reserved names
    static TypeMapping mappings[] = {
        { .zig = "u8", .c = "uint8_t" },
        { .zig = "u16", .c = "uint16_t" },
        { .zig = "u32", .c = "uint32_t" },
        { .zig = "u64", .c = "uint64_t" },
        { .zig = "i8", .c = "int8_t" },
        { .zig = "i16", .c = "int16_t" },
        { .zig = "i32", .c = "int32_t" },
        { .zig = "i64", .c = "int64_t" },
        { .zig = "i128", .c = "__int128" },
        { .zig = "u128", .c = "unsigned __int128" },
        { .zig = "isize", .c = "intptr_t" },
        { .zig = "usize", .c = "uintptr_t" },
        { .zig = "c_char", .c = "char" },
        { .zig = "c_short", .c = "short" },
        { .zig = "c_ushort", .c = "unsigned short" },
        { .zig = "c_int", .c = "int" },
        { .zig = "c_uint", .c = "unsigned int" },
        { .zig = "c_long", .c = "long" },
        { .zig = "c_ulong", .c = "unsigned long" },
        { .zig = "c_longlong", .c = "long long" },
        { .zig = "c_ulonglong", .c = "unsigned long long" },
        { .zig = "c_ulongdouble", .c = "long double" },
        { .zig = "f16", .c = "_Float16" },
        { .zig = "f32", .c = "float" },
        { .zig = "f64", .c = "double" },
        { .zig = "f80", .c = "long double" },
        { .zig = "f128", .c = "_Float128" },
        { .zig = "bool", .c = "bool" },
        { .zig = "anyopaque", .c = "void" },
    };

    for (uint32_t i = 0; i < sizeof(mappings)/sizeof(*mappings); i++) {
        if (!Buffer_eql(b, mappings[i].zig)) {
            return (Buffer){ .data = mappings[i].c, .len = std_strlen(mappings[i].c) };
        }
    }
    return b;
}

DEFINE_ARRAY_NAMED(char, Char);

static Buffer Sema_evalTypeName0(CodeGen *cg, Node *n)
{
    switch (n->tag) {
        case node_type_expr:
        {
            Buffer name = Sema_evalTypeName0(cg, n->data.type_expr.type_expr);
            for (uint32_t i = 0; i < n->data.type_expr.prefix_type_ops_len; i++) {
                NodeTag tag = n->data.type_expr.prefix_type_ops[i]->tag;
                switch (tag) {
                    case node_prefix_type_op_ptr:
                    {
                        CharArray s;
                        CharArray_init(&s);
                        NodeDataPrefixTypePtr p = n->data.type_expr.prefix_type_ops[i]->data.prefix_type_ptr;
                        if ((p.modifiers & pointer_modifier_const) != 0) {
                            CharArray_appendMany(&s, "const ", 6);
                        }
                        CharArray_appendMany(&s, name.data, name.len);

                        CodeGen_expect(cg, p.ptr->tag == node_ptr_type_start);
                        switch (p.ptr->data.ptr_type_start.type)
                        {
                            // Ignore all the special types, just lower as-is.
                            case node_ptr_type_c:
                            case node_ptr_type_single:
                            case node_ptr_type_multi:
                            case node_ptr_type_sentinel:
                                CharArray_append(&s, '*');
                                break;

                            case node_ptr_type_double:
                                CharArray_appendMany(&s, "**", 2);
                                break;
                        }
                        return (Buffer){ .data = s.data, .len = s.len };
                    }

                    default:
                        CodeGen_unsupportedTag(cg, tag);
                }
            }
            return name;
        }

        case node_error_union_expr:
            return Sema_evalTypeName0(cg, n->data.error_union_expr.suffix_expr);
        case node_suffix_expr:
            return Sema_evalTypeName0(cg, n->data.suffix_expr.expr);
        case node_primary_type_expr:
            switch (n->data.primary_type_expr.tag) {
                case node_primary_type_identifier:
                    return Sema_resolveTypeName(cg, n->data.primary_type_expr.data.raw);
                default:
                    CodeGen_fail(cg, "unsupported primary type expr tag");
            }
        default:
            CodeGen_unsupportedTag(cg, n->tag);
    }
}

// Given a type expression, returns a c-compatible representation.
static Buffer Sema_evalTypeName(CodeGen *cg, Node *n)
{
    // For now, traverse until we find a PrimaryTypeExpr.
    //
    // TypeExpr <- PrefixTypeOp* ErrorUnionExpr
    // ErrorUnionExpr <- SuffixExpr (EXCLAMATIONMARK TypeExpr)?
    // SuffixExpr
    //     <- PrimaryTypeExpr (SuffixOp / FnCallArguments)*
    //CodeGen_expect(cg, n->tag == node_type_expr);
    return Sema_evalTypeName0(cg, n);
}

static void CodeGen_genPrimaryTypeExpr(CodeGen *cg, NodeDataPrimaryTypeExpr primary_type_expr)
{
    switch (primary_type_expr.tag) {
        case node_primary_type_number_literal:
        case node_primary_type_identifier:
            emit(PRIb, Buffer(primary_type_expr.data.raw));
            break;

        case node_primary_type_char_literal:
            emit("'"PRIb"'", Buffer(primary_type_expr.data.raw));
            break;

        case node_primary_type_string_literal:
            emit(PRIb, Buffer(primary_type_expr.data.raw));
            break;

        case node_primary_type_unreachable:
            emit("__builtin_unreachable()\n");
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
            CodeGen_fail(cg, "unsupported primary type expr tag");

        default:
            CodeGen_unreachable(cg);
    }
}

static void CodeGen_genExpr(CodeGen *cg, Node *expr);
static void CodeGen_genTypeExpr(CodeGen *cg, NodeDataTypeExpr expr)
{
    CodeGen_expect2(cg, expr.prefix_type_ops_len == 0, "prefix type op not supported");

    CodeGen_expect(cg, expr.type_expr->tag == node_error_union_expr);
    NodeDataErrorUnionExpr error_union_expr = expr.type_expr->data.error_union_expr;
    CodeGen_expect2(cg, error_union_expr.error_type_expr == NULL, "error unions not supported");

    CodeGen_expect(cg, error_union_expr.suffix_expr->tag == node_suffix_expr);
    NodeDataSuffixExpr suffix_expr = error_union_expr.suffix_expr->data.suffix_expr;

    CodeGen_expect(cg, suffix_expr.expr->tag == node_primary_type_expr);
    CodeGen_genPrimaryTypeExpr(cg, suffix_expr.expr->data.primary_type_expr);

    for (uint32_t i = 0; i < suffix_expr.suffixes_len; i++) {
        Node *s = suffix_expr.suffixes[i];
        switch (s->tag) {
            case node_fn_call_arguments:
                emit("(");
                for (uint32_t i = 0; i < s->data.fn_call_arguments.exprs_len; i++) {
                    //Parser_debugNode(cg->p,s->data.fn_call_arguments.exprs[i], "try");
                    CodeGen_genExpr(cg, s->data.fn_call_arguments.exprs[i]);
                    if (i != s->data.fn_call_arguments.exprs_len - 1) emit(", ");
                }
                emit(")");
                break;
            default:
                CodeGen_fail(cg, "unimplemented suffix_expr: %s", NodeTag_name(suffix_expr.suffixes[i]->tag));
        }
    }
}

static void CodeGen_genExpr(CodeGen *cg, Node *expr);

static void CodeGen_genPrimaryExpr(CodeGen *cg, Node *expr)
{
    switch (expr->tag) {
        case node_asm_expr:
            CodeGen_fail(cg, "unimplemented asm_expr");

        case node_if_expr:
            CodeGen_fail(cg, "unimplemented if_expr");

        case node_break_expr:
            CodeGen_fail(cg, "unimplemented break_expr");

        case node_comptime_expr:
            CodeGen_fail(cg, "unimplemented comptime_expr");

        case node_nosuspend_expr:
            CodeGen_fail(cg, "unimplemented nosuspend_expr");

        case node_continue_expr:
            CodeGen_fail(cg, "unimplemented continue_expr");

        case node_resume_expr:
            CodeGen_fail(cg, "unimplemented resume_expr");

        case node_return_expr:
            indent();
            emit("return ");
            CodeGen_genExpr(cg, expr->data.return_expr);
            emit(";\n");
            break;

        case node_loop_expr:
            CodeGen_fail(cg, "unimplemented loop_expr");

        case node_block:
            CodeGen_fail(cg, "unimplemented block");

        case node_curly_suffix_expr:
            CodeGen_fail(cg, "unimplemented curly_suffix_expr");

        case node_type_expr:
            CodeGen_genTypeExpr(cg, expr->data.type_expr);
            break;

        default:
            CodeGen_unsupportedTag(cg, expr->tag);
    }
}

static void CodeGen_genUnaryExpr(CodeGen *cg, NodeDataUnaryExpr unary_expr)
{
    for (uint32_t i = 0; i < unary_expr.ops_len; i++) {
        TokenTag op = unary_expr.ops[i];
        switch (op) {
            case token_minus:
                emit("-");
                break;

            case token_tilde:
                emit("~");
                break;

            case token_bang:
                emit("!");
                break;

            case token_ampersand:
                emit("&");
                break;

            case token_minus_percent:
            case token_keyword_try:
                CodeGen_fail(cg, "unimplemented unary expr op: %s", TokenTag_name(op));

            default:
                CodeGen_unreachable(cg);
        }
    }

    CodeGen_genPrimaryExpr(cg, unary_expr.expr);
}

static void CodeGen_genBinaryExpr(CodeGen *cg, NodeDataBinaryExpr binary_expr)
{
    emit("(");
    CodeGen_genExpr(cg, binary_expr.lhs);
    emit(") ");

    switch (binary_expr.op) {
        case binop_or:
            emit("||");
            break;
        case binop_and:
            emit("&&");
            break;
        case binop_eq:
            emit("==");
            break;
        case binop_neq:
            emit("!=");
            break;
        case binop_lt:
            emit("<");
            break;
        case binop_gt:
            emit(">");
            break;
        case binop_lt_eq:
            emit(">=");
            break;
        case binop_gt_eq:
            emit("<=");
            break;
        case binop_bit_and:
            emit("&");
            break;
        case binop_bit_or:
            emit("|");
            break;
        case binop_bit_xor:
            emit("^");
            break;
        case binop_shl:
            emit("<<");
            break;
        case binop_shr:
            emit(">>");
            break;
        case binop_add:
            emit("+");
            break;
        case binop_sub:
            emit("-");
            break;
        case binop_mul:
            emit("*");
            break;
        case binop_div:
            emit("/");
            break;
        case binop_mod:
            emit("%%");
            break;
        case binop_array_concat:
        case binop_array_spread:
        case binop_add_wrap:
        case binop_sub_wrap:
        case binop_mul_wrap:
        case binop_shl_saturate:
        case binop_add_saturate:
        case binop_sub_saturate:
        case binop_mul_saturate:
        case binop_orelse:
        case binop_catch:
        case binop_error_set_merge:
        case binop_invalid:
            CodeGen_fail(cg, "unimplemented binop");
    }

    emit(" (");
    CodeGen_genExpr(cg, binary_expr.rhs);
    emit(")");
}

static void CodeGen_genExpr(CodeGen *cg, Node *expr)
{
    switch (expr->tag) {
        case node_unary_expr:
            CodeGen_genUnaryExpr(cg, expr->data.unary_expr);
            break;

        case node_binary_expr:
            CodeGen_genBinaryExpr(cg, expr->data.binary_expr);
            break;

        default:
            CodeGen_unsupportedTag(cg, expr->tag);
    }
}
static void CodeGen_genVarDecl(CodeGen *cg, NodeDataVarDeclStatement var_decl)
{
    CodeGen_expect2(cg, var_decl.var_decl_additional_len == 0, "multi-assign var_decl not supported");

    CodeGen_expect(cg, var_decl.var_decl->tag == node_var_decl_proto);
    NodeDataVarDeclProto var_proto = var_decl.var_decl->data.var_decl_proto;

    indent();
    emit(PRIb" "PRIb" =", Buffer(Sema_evalTypeName(cg, var_proto.type)), Buffer(var_proto.name));
    CodeGen_genExpr(cg, var_decl.expr);
    emit(";\n");
}

static void CodeGen_genBlock(CodeGen *cg, NodeDataBlock block);
static void CodeGen_genStatementExpr(CodeGen *cg, Node *statement_or_expr);

static void CodeGen_genLoopStatement(CodeGen *cg, NodeDataLoopStatement loop)
{
    CodeGen_expect2(cg, loop.is_inline == false, "inline loop's are unsupported (need comptime unrolling)");

    switch (loop.statement->tag) {
        case node_while_statement:
        {
            NodeDataWhileStatement while_stmt = loop.statement->data.while_statement;
            NodeDataWhilePrefix while_prefix = while_stmt.condition->data.while_prefix;
            CodeGen_expect(cg, while_prefix.ptr_payload == NULL);
            CodeGen_expect(cg, while_stmt.else_statement == NULL);

            indent();
            emit("while (");
            CodeGen_genExpr(cg, while_prefix.condition);
            emit(") {\n");
            indent();
            CodeGen_genBlock(cg, while_stmt.block->data.block);
            indent();
            CodeGen_genStatementExpr(cg, while_prefix.while_continue_expr);
            indent();
            emit("}\n");
        }
        break;

        case node_for_statement:
        {
            NodeDataForStatement for_stmt = loop.statement->data.for_statement;
            NodeDataForPrefix for_prefix = for_stmt.condition->data.for_prefix;
            CodeGen_expect(cg, for_stmt.else_statement == NULL);
            NodeDataForArgs for_args = for_prefix.for_args->data.for_args;
            CodeGen_expect(cg, for_args.args_len == 1);

            NodeDataPayloadList payloads = for_prefix.ptr_list_payload->data.payload_list;

            indent();
            emit("for (");
            for (uint32_t i = 0; i < for_args.args_len; i++) {
                NodeDataForItem for_item = for_args.args[i]->data.for_item;
                CodeGen_expect2(cg, for_item.is_range, "non-range for arguments not supported");

                if (i < payloads.payloads_len) {
                    NodeDataPayload payload = payloads.payloads[i]->data.payload;
                    emit("int "PRIb" = ", Buffer(payload.name));
                    CodeGen_genExpr(cg, for_item.for_start);
                    emit("; "PRIb" != ", Buffer(payload.name));
                    CodeGen_genExpr(cg, for_item.for_end);
                    emit("; "PRIb"++) {\n", Buffer(payload.name));
                } else {
                    emit(";;) {\n");
                }
            }

            indent();
            CodeGen_genBlock(cg, for_stmt.block->data.block);
            indent();
            emit("}\n");
        }
        break;

        default:
            CodeGen_unsupportedTag(cg, loop.statement->tag);
    }
}

static void CodeGen_genStatementExpr(CodeGen *cg, Node *statement_or_expr)
{
    switch (statement_or_expr->tag) {
        case node_comptime_statement:
            CodeGen_fail(cg, "unimplemented comptime_statement");

        case node_nosuspend_statement:
            CodeGen_fail(cg, "unimplemented nosuspend_statement");

        case node_suspend_statement:
            CodeGen_fail(cg, "unimplemented suspend_statement");

        case node_defer_statement:
            CodeGen_fail(cg, "unimplemented defer_statement");

        case node_errdefer_statement:
            CodeGen_fail(cg, "unimplemented errdefer_statement");

        case node_if_statement:
        {
            NodeDataIfStatement if_stmt = statement_or_expr->data.if_statement;

            indent();
            emit("if (");
            CodeGen_expect(cg, if_stmt.condition->tag == node_if_prefix);
            NodeDataIfPrefix if_prefix = if_stmt.condition->data.if_prefix;
            CodeGen_expect2(cg, if_prefix.ptr_payload == NULL, "if payload unsupported");
            CodeGen_genExpr(cg, if_prefix.condition);
            emit(") {\n");
            CodeGen_expect2(cg, if_stmt.block->tag == node_block, "unlabelled blocks only are supported");
            cg->indent++;
            CodeGen_genBlock(cg, if_stmt.block->data.block);
            cg->indent--;

            if (if_stmt.else_statement != NULL) {
                indent();
                emit("} else {\n");
                cg->indent++;
                CodeGen_genStatementExpr(cg, if_stmt.else_statement);
                cg->indent--;
            }

            indent();
            emit("}\n");
        }
        break;

        case node_labeled_statement:
        {
            NodeDataLabeledStatement labeled_stmt = statement_or_expr->data.labeled_statement;
            CodeGen_expect(cg, labeled_stmt.label.len == 0);
            switch (labeled_stmt.statement->tag) {
                case node_loop_statement:
                    CodeGen_genLoopStatement(cg, labeled_stmt.statement->data.loop_statement);
                    break;
                case node_block:
                    CodeGen_genBlock(cg, labeled_stmt.statement->data.block);
                    break;
                case node_switch_expr:
                    CodeGen_unreachable(cg);
                default:
                    CodeGen_unsupportedTag(cg, labeled_stmt.statement->tag);
            }
        }
        break;

        case node_var_decl_statement:
            CodeGen_genVarDecl(cg, statement_or_expr->data.var_decl_statement);
            break;

        case node_single_assign_expr:
        {
            NodeDataSingleAssignExpr e = statement_or_expr->data.single_assign_expr;
            CodeGen_expect(cg, e.lhs->tag == node_unary_expr);

            Buffer name = Sema_evalTypeName(cg, e.lhs->data.unary_expr.expr);
            if (!Buffer_eql(name, "_")) {
                CodeGen_expect2(cg, e.assign_op == token_equal, "only standard assign supported for discard");
                indent();
                emit("(void)");
                CodeGen_genExpr(cg, e.rhs);
                emit(";\n");
                return;
            }

            indent();
            emit(PRIb" ", Buffer(name));
            switch (e.assign_op) {
                case token_equal:
                    emit("=");
                    break;
                case token_plus_equal:
                    emit("+=");
                    break;
                default:
                    CodeGen_unreachable(cg);
            }
            emit(" ");
            CodeGen_genExpr(cg, e.rhs);
            emit(";\n");
            return;
        }

        default:
            // TODO: statement expression here may still require ';'
            CodeGen_genExpr(cg, statement_or_expr);
    }
}

static void CodeGen_genFunctionProto(CodeGen *cg, NodeDataFnProto fn_proto, DeclModifiers modifiers, bool is_static)
{
    if ((modifiers & token_keyword_extern) != 0 && is_static) emit("static ");
    emit(PRIb" "PRIb, Buffer(Sema_evalTypeName(cg, fn_proto.return_type)), Buffer(fn_proto.name));

    if (!fn_proto.params) {
        emit("(void)");
    } else {
        CodeGen_expect(cg, fn_proto.params->tag == node_param_decl_list);
        NodeDataParamDeclList decllist = fn_proto.params->data.param_decl_list;
        emit("(");
        for (uint32_t i = 0; i < decllist.params_len; i++) {
            CodeGen_expect(cg, decllist.params[i]->tag == node_param_decl);
            NodeDataParamDecl decl = decllist.params[i]->data.param_decl;

            if (decl.is_varargs) {
                emit("...");
            } else {
                Buffer typename = Sema_evalTypeName(cg, decl.type);
                CodeGen_expect(cg, typename.len != 0);
                emit(PRIb, Buffer(typename));

                if (decl.identifier.len != 0) {
                    emit(" "PRIb, Buffer(decl.identifier));
                }
            }
            if (i != decllist.params_len - 1) emit(", ");
        }
        emit(")");
    }
}

static void CodeGen_genBlock(CodeGen *cg, NodeDataBlock block)
{
    for (uint32_t i = 0; i < block.statements_len; i++) {
        CodeGen_genStatementExpr(cg, block.statements[i]);
    }
}

static void CodeGen_genFunction(CodeGen *cg, NodeDataDeclFn fn, bool is_static)
{
    CodeGen_expect(cg, fn.fn_proto->tag == node_fn_proto);
    if (fn.block == NULL) return;

    CodeGen_genFunctionProto(cg, fn.fn_proto->data.fn_proto, fn.modifiers, is_static);
    emit("\n{\n");
    cg->indent++;

    CodeGen_expect(cg, fn.block->tag == node_block);
    CodeGen_genBlock(cg, fn.block->data.block);

    cg->indent--;
    emit("}\n\n");
}

static void CodeGen_genDecl(CodeGen *cg, NodeDataTopLevelDecl decl)
{
    switch (decl.decl->tag) {
        case node_decl_fn:
            CodeGen_genFunction(cg, decl.decl->data.decl_fn, !decl.is_pub);
            break;

        case node_decl_global_var_decl:
            CodeGen_fail(cg, "unimplemented node_decl_global_var_decl");

        default:
            CodeGen_unsupportedTag(cg, decl.decl->tag);
    }
}

static void CodeGen_genContainerMembers(CodeGen *cg, NodeDataContainerMembers members)
{
    CodeGen_expect2(cg, members.fields_len == 0, "top-level fields are not supported");
    for (uint32_t i = 0; i < members.decls_len; i++) {
        Node *n = members.decls[i];
        switch (n->tag) {
            case node_top_level_decl:
                CodeGen_genDecl(cg, n->data.top_level_decl);
                break;

            case node_test_decl:
                // tzc does not support running (or compiling) tests
                break;

            case node_comptime_decl:
                CodeGen_unreachable(cg);

            default:
                CodeGen_unsupportedTag(cg, n->tag);
        }
    }
}

//#define USE_ZIG_H

static void CodeGen_genPrologue(CodeGen *cg)
{
    emit("/* Generated by tzc */\n");
#ifdef USE_ZIG_H
    emit("\n");
    emit("#define ZIG_TARGET_MAX_INT_ALIGNMENT 16\n");
    emit("/* zig.h begin */\n");
    std_writeFile(cg->zig_h, cg->zig_h_len, 1, cg->out);
    emit("/* zig.h end */\n");
    emit("\n");
#else
    emit("#include <stddef.h>\n");
    emit("#include <stdbool.h>\n");
    emit("#include <stdint.h>\n");
    emit("\n");
#endif
}

// C must forward-decl functions unlike zig. Scan this as a first pass and
// for simplicity, forward declare every single function decl encountered.
void CodeGen_genForwardDecls(CodeGen *cg, NodeDataContainerMembers members)
{
    for (uint32_t i = 0; i < members.decls_len; i++) {
        if (members.decls[i]->tag == node_top_level_decl) {
            NodeDataTopLevelDecl top_level_decl = members.decls[i]->data.top_level_decl;
            if (top_level_decl.decl->tag == node_decl_fn) {
                NodeDataDeclFn fn = top_level_decl.decl->data.decl_fn;
                CodeGen_expect(cg, fn.fn_proto->tag == node_fn_proto);
                NodeDataFnProto fn_proto = fn.fn_proto->data.fn_proto;
                CodeGen_genFunctionProto(cg, fn_proto, fn.modifiers, !top_level_decl.is_pub);
                emit(";\n");
            }
        }
    }
    emit("\n");
}

static void CodeGen_gen(CodeGen *cg, Node *n)
{
    CodeGen_genPrologue(cg);
    CodeGen_expect(cg, n->tag == node_container_members);
    CodeGen_genForwardDecls(cg, n->data.container_members);
    CodeGen_genContainerMembers(cg, n->data.container_members);
}

#undef emit
#undef indent