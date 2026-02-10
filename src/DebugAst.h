typedef struct DebugAst {
    const Parser *p;
    int indent;
} DebugAst;

static void DebugAst_init(DebugAst *r, Parser *p)
{
    r->p = p;
    r->indent = 0;
}

__attribute__((format(printf, 2, 3)))
int DebugAst_p(DebugAst *r, const char *fmt, ...)
{
    std_printf("%*c", r->indent, ' ');
    va_list args;
    va_start(args, fmt);
    int result = std_vprintf(fmt, args);
    va_end(args);
    std_printf("\n");
    return result;
}

static void DebugAst_beginSection(DebugAst *r, const char *name)
{
    DebugAst_p(r, "|%s", name);
    r->indent++;
}
static void DebugAst_endSection(DebugAst *r)
{
    r->indent--;
}

static void DebugAst_render(DebugAst *r, Node *n)
{
    if (!n) return;

    switch (n->tag)
    {
        case node_container_members:
            DebugAst_beginSection(r, "node_container_members");
            for (uint32_t i = 0; i < n->data.container_members.decls_len; i++) {
                DebugAst_render(r, n->data.container_members.decls[i]);
            }
            for (uint32_t i = 0; i < n->data.container_members.fields_len; i++) {
                DebugAst_render(r, n->data.container_members.fields[i]);
            }
            DebugAst_endSection(r);
            break;

        case node_container_field:
            DebugAst_beginSection(r, "node_container_field");
            DebugAst_p(r, "name: "PRIb, Buffer(n->data.container_field.name));
            DebugAst_p(r, "is_comptime: %d", n->data.container_field.is_comptime);
            DebugAst_render(r, n->data.container_field.expr);
            DebugAst_render(r, n->data.container_field.type_expr);
            DebugAst_endSection(r);
            break;

        case node_test_decl:
            DebugAst_beginSection(r, "test_decl");
            DebugAst_p(r, "name: "PRIb, Buffer(n->data.test_decl.name));
            DebugAst_p(r, "is_ident: %d", n->data.test_decl.is_ident);
            DebugAst_render(r, n->data.test_decl.block);
            DebugAst_endSection(r);
            break;

        case node_comptime_decl:
            DebugAst_beginSection(r, "comptime_decl");
            DebugAst_render(r, n->data.comptime_decl.block);
            DebugAst_endSection(r);
            break;

        case node_var_decl_proto:
            DebugAst_beginSection(r, "var_decl_proto");
            DebugAst_p(r, "name: "PRIb, Buffer(n->data.var_decl_proto.name));
            DebugAst_p(r, "is_const: %d", n->data.var_decl_proto.is_const);
            DebugAst_render(r, n->data.var_decl_proto.type);
            DebugAst_endSection(r);
            break;

        case node_global_var_decl:
            DebugAst_beginSection(r, "global_var_decl");
            DebugAst_render(r, n->data.global_var_decl.var_decl_proto);
            DebugAst_render(r, n->data.global_var_decl.expr);
            DebugAst_endSection(r);
            break;

        case node_decl_fn:
            DebugAst_beginSection(r, "decl_fn");
            DebugAst_render(r, n->data.decl_fn.fn_proto);
            DebugAst_render(r, n->data.decl_fn.block);
            DebugAst_endSection(r);
            break;

        case node_decl_global_var_decl:
            DebugAst_beginSection(r, "decl_global_var_decl");
            DebugAst_render(r, n->data.decl_global_var_decl.global_var_decl);
            DebugAst_endSection(r);
            break;

        case node_block:
            DebugAst_beginSection(r, "block");
            for (uint32_t i = 0; i < n->data.block.statements_len; i++) {
                DebugAst_render(r, n->data.block.statements[i]);
            }
            DebugAst_endSection(r);
            break;

        case node_fn_proto:
            DebugAst_beginSection(r, "fn_proto");
            DebugAst_p(r, "name: "PRIb, Buffer(n->data.fn_proto.name));
            DebugAst_render(r, n->data.fn_proto.params);
            DebugAst_render(r, n->data.fn_proto.return_type);
            DebugAst_endSection(r);
            break;

        case node_fn_proto_extra:
            std_panic("unreachable");

        case node_param_decl_list:
            DebugAst_beginSection(r, "param_decl_list");
            for (uint32_t i = 0; i < n->data.param_decl_list.params_len; i++) {
                DebugAst_render(r, n->data.param_decl_list.params[i]);
            }
            DebugAst_endSection(r);
            break;

        case node_param_decl:
            DebugAst_beginSection(r, "param_decl");
            DebugAst_p(r, "name: "PRIb, Buffer(n->data.param_decl.identifier));
            DebugAst_p(r, "is_varargs: %d", n->data.param_decl.is_varargs);
            if (n->data.param_decl.modifier != token_invalid) {
                DebugAst_p(r, "modifier: %s", TokenTag_name(n->data.param_decl.modifier));
            }
            DebugAst_render(r, n->data.param_decl.type);
            DebugAst_endSection(r);
            break;

        case node_type_expr:
            DebugAst_beginSection(r, "type_expr");
            DebugAst_render(r, n->data.type_expr.type_expr);
            for (uint32_t i = 0; i < n->data.type_expr.prefix_type_ops_len; i++) {
                DebugAst_render(r, n->data.type_expr.prefix_type_ops[i]);
            }
            DebugAst_endSection(r);
            break;

        case node_error_union_expr:
            if (n->data.error_union_expr.error_type_expr == NULL) {
                DebugAst_render(r, n->data.error_union_expr.suffix_expr);
                return;
            }

            DebugAst_beginSection(r, "error_union_expr");
            DebugAst_render(r, n->data.error_union_expr.error_type_expr);
            DebugAst_render(r, n->data.error_union_expr.suffix_expr);
            DebugAst_endSection(r);
            break;

        case node_suffix_expr:
            if (n->data.suffix_expr.suffixes_len == 0) {
                DebugAst_render(r, n->data.suffix_expr.expr);
                return;
            }

            DebugAst_beginSection(r, "suffix_expr");
            DebugAst_render(r, n->data.suffix_expr.expr);
            for (uint32_t i = 0; i < n->data.suffix_expr.suffixes_len; i++) {
                DebugAst_render(r, n->data.suffix_expr.suffixes[i]);
            }
            DebugAst_endSection(r);
            break;

        case node_comptime_statement:
            DebugAst_beginSection(r, "comptime_statement");
            DebugAst_render(r, n->data.comptime_statement.comptime_statement);
            DebugAst_endSection(r);
            break;

        case node_nosuspend_statement:
            DebugAst_beginSection(r, "nosuspend_statement");
            DebugAst_render(r, n->data.nosuspend_statement.block_expr);
            DebugAst_endSection(r);
            break;

        case node_suspend_statement:
            DebugAst_beginSection(r, "suspend_statement");
            DebugAst_render(r, n->data.suspend_statement.block_expr);
            DebugAst_endSection(r);
            break;

        case node_defer_statement:
            DebugAst_beginSection(r, "defer_statement");
            DebugAst_render(r, n->data.defer_statement.block_expr);
            DebugAst_endSection(r);
            break;

        case node_errdefer_statement:
            DebugAst_beginSection(r, "errdefer_statement");
            DebugAst_p(r, "payload: "PRIb, Buffer(n->data.errdefer_statement.payload_name));
            DebugAst_render(r, n->data.errdefer_statement.block_expr);
            DebugAst_endSection(r);
            break;

        case node_unary_expr:
            if (n->data.unary_expr.ops_len == 0) {
                DebugAst_render(r, n->data.unary_expr.expr);
                return;
            }

            DebugAst_beginSection(r, "unary_expr");
            if (n->data.unary_expr.ops_len > 0) {
                DebugAst_beginSection(r, "ops");
                for (uint32_t i = 0; i < n->data.unary_expr.ops_len; i++) {
                    DebugAst_p(r, "%d", n->data.unary_expr.ops[i]);
                }
                DebugAst_endSection(r);
            }
            DebugAst_render(r, n->data.unary_expr.expr);
            DebugAst_endSection(r);
            break;

        case node_binary_expr:
            DebugAst_beginSection(r, "binary_expr");
            DebugAst_p(r, "op: %d", n->data.binary_expr.op);
            DebugAst_render(r, n->data.binary_expr.lhs);
            DebugAst_render(r, n->data.binary_expr.rhs);
            DebugAst_endSection(r);
            break;

        case node_comptime_expr:
            DebugAst_beginSection(r, "comptime_expr");
            DebugAst_render(r, n->data.comptime_expr);
            DebugAst_endSection(r);
            break;

        case node_nosuspend_expr:
            DebugAst_beginSection(r, "nosuspend_expr");
            DebugAst_render(r, n->data.nosuspend_expr);
            DebugAst_endSection(r);
            break;

        case node_resume_expr:
            DebugAst_beginSection(r, "resume_expr");
            DebugAst_render(r, n->data.resume_expr);
            DebugAst_endSection(r);
            break;

        case node_return_expr:
            DebugAst_beginSection(r, "return_expr");
            DebugAst_render(r, n->data.return_expr);
            DebugAst_endSection(r);
            break;

        case node_curly_suffix_expr:
            DebugAst_beginSection(r, "curly_suffix_expr");
            DebugAst_render(r, n->data.curly_suffix_expr.type);
            DebugAst_render(r, n->data.curly_suffix_expr.initlist);
            DebugAst_endSection(r);
            break;

        case node_primary_type_expr:
            DebugAst_beginSection(r, "primary_type_expr");
            switch (n->data.primary_type_expr.tag) {
                case node_primary_type_builtin:
                    DebugAst_p(r, "builtin: "PRIb, Buffer(n->data.primary_type_expr.data.raw));
                    DebugAst_render(r, n->data.primary_type_expr.data.node);
                    break;

                case node_primary_type_identifier:
                    DebugAst_p(r, "identifier: "PRIb, Buffer(n->data.primary_type_expr.data.raw));
                    break;

                case node_primary_type_char_literal:
                case node_primary_type_dot_identifier:
                case node_primary_type_number_literal:
                case node_primary_type_error:
                case node_primary_type_string_literal:
                    DebugAst_p(r, "literal: "PRIb, Buffer(n->data.primary_type_expr.data.raw));
                    break;

                case node_primary_type_container_decl:
                case node_primary_type_dot_initlist:
                case node_primary_type_error_set_decl:
                case node_primary_type_fn_proto:
                case node_primary_type_grouped_expr:
                case node_primary_type_labeled_type_expr:
                case node_primary_type_if_type_expr:
                case node_primary_type_comptime_type_expr:
                    DebugAst_render(r, n->data.primary_type_expr.data.node);
                    break;

                case node_primary_type_anyframe:
                    DebugAst_p(r, "anyframe");
                    break;

                case node_primary_type_unreachable:
                    DebugAst_p(r, "unreachable");
                    break;

                case node_primary_type_anytype:
                    DebugAst_p(r, "anytype");
                    break;
            }
            DebugAst_endSection(r);
            break;

        case node_top_level_decl:
            DebugAst_beginSection(r, "top_level_decl");
            DebugAst_p(r, "is_pub: %d", n->data.top_level_decl.is_pub);
            DebugAst_render(r, n->data.top_level_decl.decl);
            DebugAst_endSection(r);
            break;

        case node_for_item:
            DebugAst_beginSection(r, "for_item");
            DebugAst_render(r, n->data.for_item.for_start);
            DebugAst_render(r, n->data.for_item.for_end);
            DebugAst_p(r, "is_range: %d", n->data.for_item.is_range);
            DebugAst_endSection(r);
            break;

        case node_for_args:
            DebugAst_beginSection(r, "for_args");
            for (uint32_t i = 0; i < n->data.for_args.args_len; i++) {
                DebugAst_render(r, n->data.for_args.args[i]);
            }
            DebugAst_endSection(r);
            break;

        case node_field_init:
            DebugAst_beginSection(r, "field_init");
            DebugAst_p(r, "name: "PRIb, Buffer(n->data.field_init.name));
            DebugAst_render(r, n->data.field_init.expr);
            DebugAst_endSection(r);
            break;

        case node_struct_decl:
            DebugAst_beginSection(r, "struct_decl");
            DebugAst_endSection(r);
            break;

        case node_opaque_decl:
            DebugAst_beginSection(r, "opaque_decl");
            DebugAst_endSection(r);
            break;

        case node_enum_decl:
            DebugAst_beginSection(r, "enum_decl");
            DebugAst_endSection(r);
            break;

        case node_union_decl:
            DebugAst_beginSection(r, "union_decl");
            DebugAst_endSection(r);
            break;

        case node_switch_item:
            DebugAst_beginSection(r, "switch_item");
            DebugAst_endSection(r);
            break;

        case node_switch_case:
            DebugAst_beginSection(r, "switch_case");
            DebugAst_endSection(r);
            break;

        case node_labeled_block:
            DebugAst_beginSection(r, "labeled_block");
            DebugAst_endSection(r);
            break;

        case node_labeled_loop_expr:
            DebugAst_beginSection(r, "labeled_loop_expr");
            DebugAst_endSection(r);
            break;

        case node_labeled_switch_expr:
            DebugAst_beginSection(r, "labeled_switch_expr");
            DebugAst_endSection(r);
            break;

        case node_while_statement:
            DebugAst_beginSection(r, "while_statement");
            DebugAst_render(r, n->data.while_statement.condition);
            DebugAst_render(r, n->data.while_statement.block);
            if (n->data.while_statement.else_payload_name.len != 0) {
                DebugAst_p(r, "else_payload_name: "PRIb, Buffer(n->data.while_statement.else_payload_name));
            }
            DebugAst_render(r, n->data.while_statement.else_statement);
            DebugAst_endSection(r);
            DebugAst_endSection(r);
            break;

        case node_for_statement:
            DebugAst_beginSection(r, "for_statement");
            DebugAst_render(r, n->data.for_statement.condition);
            DebugAst_render(r, n->data.for_statement.block);
            DebugAst_render(r, n->data.for_statement.else_statement);
            DebugAst_endSection(r);
            break;

        case node_if_statement:
            DebugAst_beginSection(r, "if_statement");
            DebugAst_render(r, n->data.if_statement.condition);
            DebugAst_render(r, n->data.if_statement.block);
            if (n->data.if_statement.else_payload_name.len != 0) {
                DebugAst_p(r, "else_payload_name: "PRIb, Buffer(n->data.if_statement.else_payload_name));
            }
            DebugAst_render(r, n->data.if_statement.else_statement);
            DebugAst_endSection(r);
            break;

        case node_labeled_statement:
            if (n->data.labeled_statement.label.len == 0) {
                DebugAst_render(r, n->data.labeled_statement.statement);
                return;
            }

            DebugAst_beginSection(r, "labeled_statement");
            DebugAst_p(r, "label: "PRIb, Buffer(n->data.labeled_statement.label));
            DebugAst_render(r, n->data.labeled_statement.statement);
            DebugAst_endSection(r);
            break;

        case node_if_expr:
            DebugAst_beginSection(r, "if_expr");
            DebugAst_render(r, n->data.if_expr.condition);
            DebugAst_render(r, n->data.if_expr.expr);
            if (n->data.if_expr.else_payload_name.len != 0) {
                DebugAst_p(r, "else_payload_name: "PRIb, Buffer(n->data.if_expr.else_payload_name));
            }
            DebugAst_render(r, n->data.if_expr.else_payload_expr);
            DebugAst_endSection(r);
            break;

        case node_var_decl_statement:
            DebugAst_beginSection(r, "var_decl_statement");
            DebugAst_render(r, n->data.var_decl_statement.var_decl);
            DebugAst_render(r, n->data.var_decl_statement.expr);
            for (uint32_t i = 0; i < n->data.var_decl_statement.var_decl_additional_len; i++) {
                DebugAst_render(r, n->data.var_decl_statement.var_decl_additional[i]);
            }
            DebugAst_endSection(r);
            break;

        case node_single_assign_expr:
            DebugAst_beginSection(r, "single_assign_expr");
            DebugAst_p(r, "op: %s", TokenTag_name(n->data.single_assign_expr.assign_op));
            DebugAst_render(r, n->data.single_assign_expr.lhs);
            DebugAst_render(r, n->data.single_assign_expr.rhs);
            DebugAst_endSection(r);
            break;

        case node_multi_assign_expr:
            DebugAst_beginSection(r, "multi_assign_expr");
            DebugAst_endSection(r);
            break;

        case node_loop_expr:
            DebugAst_beginSection(r, "loop_expr");
            DebugAst_endSection(r);
            break;

        case node_continue_expr:
            DebugAst_beginSection(r, "continue_expr");
            DebugAst_endSection(r);
            break;

        case node_break_expr:
            DebugAst_beginSection(r, "break_expr");
            DebugAst_endSection(r);
            break;

        case node_while_expr:
            DebugAst_beginSection(r, "while_expr");
            DebugAst_endSection(r);
            break;

        case node_for_expr:
            DebugAst_beginSection(r, "loop_statement");
            DebugAst_endSection(r);
            break;

        case node_loop_statement:
            DebugAst_beginSection(r, "node_loop_statement");
            DebugAst_p(r, "is_inline: %d", n->data.loop_statement.is_inline);
            DebugAst_render(r, n->data.loop_statement.statement);
            DebugAst_endSection(r);
            break;

        case node_container_decl_auto:
            DebugAst_beginSection(r, "node_container_decl_auto");
            DebugAst_render(r, n->data.container_decl_auto.type);
            DebugAst_render(r, n->data.container_decl_auto.members);
            DebugAst_endSection(r);
            break;

        case node_prefix_type_op_optional:
            DebugAst_beginSection(r, "prefix_type_op_optional");
            DebugAst_endSection(r);
            break;

        case node_prefix_type_op_anyframe:
            DebugAst_beginSection(r, "prefix_type_op_anyframe");
            DebugAst_endSection(r);
            break;

        case node_prefix_type_op_slice:
            DebugAst_beginSection(r, "prefix_type_op_slice");
            DebugAst_render(r, n->data.prefix_type_slice.addrspace);
            DebugAst_render(r, n->data.prefix_type_slice.bytealign);
            DebugAst_p(r, "is_allowzero: %d", (n->data.prefix_type_slice.modifiers & pointer_modifier_allowzero) != 0);
            DebugAst_p(r, "is_const: %d", (n->data.prefix_type_slice.modifiers & pointer_modifier_const) != 0);
            DebugAst_p(r, "is_volatile: %d", (n->data.prefix_type_slice.modifiers & pointer_modifier_volatile) != 0);
            DebugAst_render(r, n->data.prefix_type_slice.slice);
            DebugAst_endSection(r);
            break;

        case node_prefix_type_op_ptr:
            DebugAst_beginSection(r, "prefix_type_op_ptr");
            DebugAst_render(r, n->data.prefix_type_ptr.addrspace);
            DebugAst_render(r, n->data.prefix_type_ptr.align);
            DebugAst_p(r, "is_allowzero: %d", (n->data.prefix_type_ptr.modifiers & pointer_modifier_allowzero) != 0);
            DebugAst_p(r, "is_const: %d", (n->data.prefix_type_ptr.modifiers & pointer_modifier_const) != 0);
            DebugAst_p(r, "is_volatile: %d", (n->data.prefix_type_ptr.modifiers & pointer_modifier_volatile) != 0);
            DebugAst_render(r, n->data.prefix_type_ptr.ptr);
            DebugAst_endSection(r);
            break;

        case node_prefix_type_op_array:
            DebugAst_beginSection(r, "prefix_type_op_array");
            DebugAst_render(r, n->data.prefix_type_array.array);
            DebugAst_endSection(r);
            break;

        case node_ptr_align_expr:
            DebugAst_beginSection(r, "ptr_align_expr");
            DebugAst_endSection(r);
            break;

        case node_array_type_start:
            DebugAst_beginSection(r, "array_type_start");
            DebugAst_endSection(r);
            break;

        case node_ptr_type_start:
            DebugAst_beginSection(r, "ptr_type_start");
            DebugAst_endSection(r);
            break;

        case node_slice_type_start:
            DebugAst_beginSection(r, "slice_type_start");
            DebugAst_endSection(r);
            break;

        case node_suffix_type_op_slice:
            DebugAst_beginSection(r, "suffix_slice");
            DebugAst_render(r, n->data.suffix_type_op_slice.start_expr);
            DebugAst_render(r, n->data.suffix_type_op_slice.end_expr);
            DebugAst_render(r, n->data.suffix_type_op_slice.sentinel_expr);
            DebugAst_endSection(r);
            break;

        case node_suffix_type_op_named_access:
            DebugAst_beginSection(r, "suffix_member");
            DebugAst_p(r, "."PRIb, Buffer(n->data.suffix_type_op_named_access.name));
            DebugAst_endSection(r);
            break;

        case node_suffix_type_op_deref:
            DebugAst_beginSection(r, "suffix_deref");
            DebugAst_p(r, ".*");
            DebugAst_endSection(r);
            break;

        case node_suffix_type_op_assert_maybe:
            DebugAst_beginSection(r, "suffix_type_op_assert_maybe");
            DebugAst_p(r, ".?");
            DebugAst_endSection(r);
            break;

        case node_fn_call_arguments:
            if (n->data.fn_call_arguments.exprs_len == 0) return;

            DebugAst_beginSection(r, "fn_call_arguments");
            for (uint32_t i = 0; i < n->data.fn_call_arguments.exprs_len; i++) {
                DebugAst_render(r, n->data.fn_call_arguments.exprs[i]);
            }
            DebugAst_endSection(r);
            break;

        case node_for_prefix:
            DebugAst_beginSection(r, "for_prefix");
            DebugAst_render(r, n->data.for_prefix.for_args);
            DebugAst_render(r, n->data.for_prefix.ptr_list_payload);
            DebugAst_endSection(r);
            break;

        case node_while_prefix:
            DebugAst_beginSection(r, "while_prefix");
            DebugAst_render(r, n->data.while_prefix.condition);
            DebugAst_render(r, n->data.while_prefix.ptr_payload);
            DebugAst_render(r, n->data.while_prefix.while_continue_expr);
            DebugAst_endSection(r);
            break;

        case node_if_prefix:
            DebugAst_beginSection(r, "if_prefix");
            DebugAst_render(r, n->data.if_prefix.condition);
            DebugAst_render(r, n->data.if_prefix.ptr_payload);
            DebugAst_endSection(r);
            break;

        case node_payload:
            DebugAst_beginSection(r, "payload");
            DebugAst_p(r, "name: %s"PRIb, n->data.payload.is_pointer ? "*" : "", Buffer(n->data.payload.name));
            DebugAst_endSection(r);
            break;

        case node_payload_index:
            DebugAst_beginSection(r, "payload_index");
            DebugAst_endSection(r);
            break;

        case node_payload_list:
            DebugAst_beginSection(r, "payload_list");
            for (uint32_t i = 0; i < n->data.payload_list.payloads_len; i++) {
                DebugAst_render(r, n->data.payload_list.payloads[i]);
            }
            DebugAst_endSection(r);
            break;

        case node_switch_prong:
            DebugAst_beginSection(r, "switch_prong");
            DebugAst_endSection(r);
            break;

        case node_for_type_expr:
            DebugAst_beginSection(r, "for_type_expr");
            DebugAst_endSection(r);
            break;

        case node_switch_prong_list:
            DebugAst_beginSection(r, "switch_prong_list");
            DebugAst_endSection(r);
            break;

        case node_container_decl:
            DebugAst_beginSection(r, "node_container_decl");
            DebugAst_p(r, "is_extern: %d", n->data.container_decl.is_extern);
            DebugAst_p(r, "is_packed: %d", n->data.container_decl.is_packed);
            DebugAst_render(r, n->data.container_decl.container_decl);
            DebugAst_endSection(r);
            break;

        case node_if_type_expr:
            DebugAst_beginSection(r, "node_if_type_expr");
            DebugAst_endSection(r);
            break;

        case node_while_type_expr:
            DebugAst_beginSection(r, "node_while_type_expr");
            DebugAst_endSection(r);
            break;

        case node_identifier_list:
            DebugAst_beginSection(r, "node_identifier_list");
            DebugAst_endSection(r);
            break;

        case node_switch_expr:
            DebugAst_beginSection(r, "node_switch_expr");
            DebugAst_endSection(r);
            break;

        case node_init_list_field:
            if (n->data.init_list_field.nodes_len == 0) return;

            DebugAst_beginSection(r, "node_init_list_field");
            for (uint32_t i = 0; i < n->data.init_list_field.nodes_len; i++) {
                DebugAst_render(r, n->data.init_list_field.nodes[i]);
            }
            DebugAst_endSection(r);
            break;

        case node_init_list_expr:
            DebugAst_beginSection(r, "node_init_list_expr");
            DebugAst_endSection(r);
            break;

        case node_init_list_empty:
            DebugAst_beginSection(r, "node_init_list_empty");
            DebugAst_endSection(r);
            break;

        case node_asm_input_list:
            DebugAst_beginSection(r, "node_asm_input_list");
            DebugAst_endSection(r);
            break;

        case node_asm_output_list:
            DebugAst_beginSection(r, "node_asm_output_list");
            DebugAst_endSection(r);
            break;

        case node_asm_input_item:
            DebugAst_beginSection(r, "node_asm_input_item");
            DebugAst_endSection(r);
            break;

        case node_asm_output_item:
            DebugAst_beginSection(r, "node_asm_output_item");
            DebugAst_endSection(r);
            break;

        case node_asm_input:
            DebugAst_beginSection(r, "node_asm_input");
            DebugAst_endSection(r);
            break;

        case node_asm_output:
            DebugAst_beginSection(r, "node_asm_output");
            DebugAst_endSection(r);
            break;

        case node_asm_expr:
            DebugAst_beginSection(r, "node_asm_expr");
            DebugAst_endSection(r);
            break;

        case node_type_or_name:
            DebugAst_beginSection(r, "node_type_or_name");
            DebugAst_endSection(r);
            break;

        case node_invalid:
            std_panic("unreachable");
    }
}