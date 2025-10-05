typedef struct AstRenderer {
    const Parser *p;
    int indent;
} AstRenderer;

static void AstRenderer_init(AstRenderer *r, Parser *p)
{
    r->p = p;
    r->indent = 0;
}

#define astp(r, fmt, ...) std_printf("%*c" fmt "\n", r->indent, ' ',  ## __VA_ARGS__)
#define boolstring(cond) ((cond) ? "true" : "false")

static void AstRenderer_beginSection(AstRenderer *r, const char *name)
{
    astp(r, "|%s", name);
    r->indent++;
}
static void AstRenderer_endSection(AstRenderer *r)
{
    r->indent--;
}

static void AstRenderer_render(AstRenderer *r, Node *n)
{
    if (!n) return;

    switch (n->tag)
    {
        case node_container_members:
            AstRenderer_beginSection(r, "node_container_members");
            for (uint32_t i = 0; i < n->data.container_members.decls_len; i++) {
                AstRenderer_render(r, n->data.container_members.decls[i]);
            }
            for (uint32_t i = 0; i < n->data.container_members.fields_len; i++) {
                AstRenderer_render(r, n->data.container_members.fields[i]);
            }
            AstRenderer_endSection(r);
            break;

        case node_container_field:
            AstRenderer_beginSection(r, "node_container_field");
            astp(r, "name: %s", Buffer_staticZ(n->data.container_field.name));
            astp(r, "is_comptime: %s", boolstring(n->data.container_field.is_comptime));
            AstRenderer_render(r, n->data.container_field.expr);
            AstRenderer_render(r, n->data.container_field.type_expr);
            AstRenderer_endSection(r);
            break;

        case node_test_decl:
            AstRenderer_beginSection(r, "test_decl");
            astp(r, "name: %s", Buffer_staticZ(n->data.test_decl.name));
            astp(r, "is_ident: %s", boolstring(n->data.test_decl.is_ident));
            AstRenderer_render(r, n->data.test_decl.block);
            AstRenderer_endSection(r);
            break;

        case node_comptime_decl:
            AstRenderer_beginSection(r, "comptime_decl");
            AstRenderer_render(r, n->data.comptime_decl.block);
            AstRenderer_endSection(r);
            break;

        case node_var_decl_proto:
            AstRenderer_beginSection(r, "var_decl_proto");
            astp(r, "name: %s", Buffer_staticZ(n->data.var_decl_proto.name));
            astp(r, "is_const: %s", boolstring(n->data.var_decl_proto.is_const));
            AstRenderer_render(r, n->data.var_decl_proto.type);
            AstRenderer_endSection(r);
            break;

        case node_global_var_decl:
            AstRenderer_beginSection(r, "global_var_decl");
            AstRenderer_render(r, n->data.global_var_decl.var_decl_proto);
            AstRenderer_render(r, n->data.global_var_decl.expr);
            AstRenderer_endSection(r);
            break;

        case node_decl_fn:
            AstRenderer_beginSection(r, "decl_fn");
            AstRenderer_render(r, n->data.decl_fn.fn_proto);
            AstRenderer_render(r, n->data.decl_fn.block);
            AstRenderer_endSection(r);
            break;

        case node_decl_global_var_decl:
            AstRenderer_beginSection(r, "decl_global_var_decl");
            AstRenderer_render(r, n->data.decl_global_var_decl.global_var_decl);
            AstRenderer_endSection(r);
            break;

        case node_block:
            AstRenderer_beginSection(r, "block");
            for (uint32_t i = 0; i < n->data.block.statements_len; i++) {
                AstRenderer_render(r, n->data.block.statements[i]);
            }
            AstRenderer_endSection(r);
            break;

        case node_fn_proto:
            AstRenderer_beginSection(r, "fn_proto");
            astp(r, "name: %s", Buffer_staticZ(n->data.fn_proto.name));
            AstRenderer_render(r, n->data.fn_proto.params);
            AstRenderer_render(r, n->data.fn_proto.return_type);
            AstRenderer_endSection(r);
            break;

        case node_fn_proto_extra:
            std_panic("unreachable");

        case node_param_decl_list:
            AstRenderer_beginSection(r, "param_decl_list");
            for (uint32_t i = 0; i < n->data.param_decl_list.params_len; i++) {
                AstRenderer_render(r, n->data.param_decl_list.params[i]);
            }
            AstRenderer_endSection(r);
            break;

        case node_param_decl:
            AstRenderer_beginSection(r, "param_decl");
            astp(r, "name: %s", Buffer_staticZ(n->data.param_decl.identifier));
            astp(r, "is_varargs: %s", boolstring(n->data.param_decl.is_varargs));
            if (n->data.param_decl.modifier != token_invalid) {
                astp(r, "modifier: %s", TokenTag_name(n->data.param_decl.modifier));
            }
            AstRenderer_render(r, n->data.param_decl.type);
            AstRenderer_endSection(r);
            break;

        case node_type_expr:
            AstRenderer_beginSection(r, "type_expr");
            AstRenderer_render(r, n->data.type_expr.type_expr);
            for (uint32_t i = 0; i < n->data.type_expr.prefix_type_ops_len; i++) {
                AstRenderer_render(r, n->data.type_expr.prefix_type_ops[i]);
            }
            AstRenderer_endSection(r);
            break;

        case node_error_union_expr:
            if (n->data.error_union_expr.error_type_expr == NULL) {
                AstRenderer_render(r, n->data.error_union_expr.suffix_expr);
                return;
            }

            AstRenderer_beginSection(r, "error_union_expr");
            AstRenderer_render(r, n->data.error_union_expr.error_type_expr);
            AstRenderer_render(r, n->data.error_union_expr.suffix_expr);
            AstRenderer_endSection(r);
            break;

        case node_suffix_expr:
            if (n->data.suffix_expr.suffixes_len == 0) {
                AstRenderer_render(r, n->data.suffix_expr.expr);
                return;
            }

            AstRenderer_beginSection(r, "suffix_expr");
            AstRenderer_render(r, n->data.suffix_expr.expr);
            for (uint32_t i = 0; i < n->data.suffix_expr.suffixes_len; i++) {
                AstRenderer_render(r, n->data.suffix_expr.suffixes[i]);
            }
            AstRenderer_endSection(r);
            break;

        case node_comptime_statement:
            AstRenderer_beginSection(r, "comptime_statement");
            AstRenderer_render(r, n->data.comptime_statement.comptime_statement);
            AstRenderer_endSection(r);
            break;

        case node_nosuspend_statement:
            AstRenderer_beginSection(r, "nosuspend_statement");
            AstRenderer_render(r, n->data.nosuspend_statement.block_expr);
            AstRenderer_endSection(r);
            break;

        case node_suspend_statement:
            AstRenderer_beginSection(r, "suspend_statement");
            AstRenderer_render(r, n->data.suspend_statement.block_expr);
            AstRenderer_endSection(r);
            break;

        case node_defer_statement:
            AstRenderer_beginSection(r, "defer_statement");
            AstRenderer_render(r, n->data.defer_statement.block_expr);
            AstRenderer_endSection(r);
            break;

        case node_errdefer_statement:
            AstRenderer_beginSection(r, "errdefer_statement");
            astp(r, "payload: %s", Buffer_staticZ(n->data.errdefer_statement.payload_name));
            AstRenderer_render(r, n->data.errdefer_statement.block_expr);
            AstRenderer_endSection(r);
            break;

        case node_unary_expr:
            if (n->data.unary_expr.ops_len == 0) {
                AstRenderer_render(r, n->data.unary_expr.expr);
                return;
            }

            AstRenderer_beginSection(r, "unary_expr");
            if (n->data.unary_expr.ops_len > 0) {
                AstRenderer_beginSection(r, "ops");
                for (uint32_t i = 0; i < n->data.unary_expr.ops_len; i++) {
                    astp(r, "%d", n->data.unary_expr.ops[i]);
                }
                AstRenderer_endSection(r);
            }
            AstRenderer_render(r, n->data.unary_expr.expr);
            AstRenderer_endSection(r);
            break;

        case node_binary_expr:
            AstRenderer_beginSection(r, "binary_expr");
            astp(r, "op: %d", n->data.binary_expr.op);
            AstRenderer_render(r, n->data.binary_expr.lhs);
            AstRenderer_render(r, n->data.binary_expr.rhs);
            AstRenderer_endSection(r);
            break;

        case node_comptime_expr:
            AstRenderer_beginSection(r, "comptime_expr");
            AstRenderer_render(r, n->data.comptime_expr);
            AstRenderer_endSection(r);
            break;

        case node_nosuspend_expr:
            AstRenderer_beginSection(r, "nosuspend_expr");
            AstRenderer_render(r, n->data.nosuspend_expr);
            AstRenderer_endSection(r);
            break;

        case node_resume_expr:
            AstRenderer_beginSection(r, "resume_expr");
            AstRenderer_render(r, n->data.resume_expr);
            AstRenderer_endSection(r);
            break;

        case node_return_expr:
            AstRenderer_beginSection(r, "return_expr");
            AstRenderer_render(r, n->data.return_expr);
            AstRenderer_endSection(r);
            break;

        case node_curly_suffix_expr:
            AstRenderer_beginSection(r, "curly_suffix_expr");
            AstRenderer_render(r, n->data.curly_suffix_expr.type);
            AstRenderer_render(r, n->data.curly_suffix_expr.initlist);
            AstRenderer_endSection(r);
            break;

        case node_primary_type_expr:
            AstRenderer_beginSection(r, "primary_type_expr");
            switch (n->data.primary_type_expr.tag) {
                case node_primary_type_builtin:
                    astp(r, "builtin: %s", Buffer_staticZ(n->data.primary_type_expr.data.raw));
                    AstRenderer_render(r, n->data.primary_type_expr.data.node);
                    break;

                case node_primary_type_identifier:
                    astp(r, "identifier: %s", Buffer_staticZ(n->data.primary_type_expr.data.raw));
                    break;

                case node_primary_type_char_literal:
                case node_primary_type_dot_identifier:
                case node_primary_type_number_literal:
                case node_primary_type_error:
                case node_primary_type_string_literal:
                    astp(r, "literal: %s", Buffer_staticZ(n->data.primary_type_expr.data.raw));
                    break;

                case node_primary_type_container_decl:
                case node_primary_type_dot_initlist:
                case node_primary_type_error_set_decl:
                case node_primary_type_fn_proto:
                case node_primary_type_grouped_expr:
                case node_primary_type_labeled_type_expr:
                case node_primary_type_if_type_expr:
                case node_primary_type_comptime_type_expr:
                    AstRenderer_render(r, n->data.primary_type_expr.data.node);
                    break;

                case node_primary_type_anyframe:
                    astp(r, "anyframe");
                    break;

                case node_primary_type_unreachable:
                    astp(r, "unreachable");
                    break;

                case node_primary_type_anytype:
                    astp(r, "anytype");
                    break;
            }
            AstRenderer_endSection(r);
            break;

        case node_top_level_decl:
            AstRenderer_beginSection(r, "top_level_decl");
            astp(r, "is_pub: %s", boolstring(n->data.top_level_decl.is_pub));
            AstRenderer_render(r, n->data.top_level_decl.decl);
            AstRenderer_endSection(r);
            break;

        case node_for_item:
            AstRenderer_beginSection(r, "for_item");
            AstRenderer_render(r, n->data.for_item.for_start);
            AstRenderer_render(r, n->data.for_item.for_end);
            astp(r, "is_range: %s", boolstring(n->data.for_item.is_range));
            AstRenderer_endSection(r);
            break;

        case node_for_args:
            AstRenderer_beginSection(r, "for_args");
            AstRenderer_endSection(r);
            break;

        case node_field_init:
            AstRenderer_beginSection(r, "field_init");
            astp(r, "name: %s", Buffer_staticZ(n->data.field_init.name));
            AstRenderer_render(r, n->data.field_init.expr);
            AstRenderer_endSection(r);
            break;

        case node_struct_decl:
            AstRenderer_beginSection(r, "struct_decl");
            AstRenderer_endSection(r);
            break;

        case node_opaque_decl:
            AstRenderer_beginSection(r, "opaque_decl");
            AstRenderer_endSection(r);
            break;

        case node_enum_decl:
            AstRenderer_beginSection(r, "enum_decl");
            AstRenderer_endSection(r);
            break;

        case node_union_decl:
            AstRenderer_beginSection(r, "union_decl");
            AstRenderer_endSection(r);
            break;

        case node_switch_item:
            AstRenderer_beginSection(r, "switch_item");
            AstRenderer_endSection(r);
            break;

        case node_switch_case:
            AstRenderer_beginSection(r, "switch_case");
            AstRenderer_endSection(r);
            break;

        case node_labeled_block:
            AstRenderer_beginSection(r, "labeled_block");
            AstRenderer_endSection(r);
            break;

        case node_labeled_loop_expr:
            AstRenderer_beginSection(r, "labeled_loop_expr");
            AstRenderer_endSection(r);
            break;

        case node_labeled_switch_expr:
            AstRenderer_beginSection(r, "labeled_switch_expr");
            AstRenderer_endSection(r);
            break;

        case node_while_statement:
            AstRenderer_beginSection(r, "while_statement");
            AstRenderer_endSection(r);
            break;

        case node_for_statement:
            AstRenderer_beginSection(r, "for_statement");
            AstRenderer_endSection(r);
            break;

        case node_if_statement:
            AstRenderer_beginSection(r, "if_statement");
            AstRenderer_render(r, n->data.if_statement.condition);
            AstRenderer_render(r, n->data.if_statement.block);
            if (n->data.if_statement.else_payload_name.len != 0) {
                astp(r, "payload_name:", n->data.if_statement.else_payload_name);
            }
            AstRenderer_render(r, n->data.if_statement.else_statement);
            AstRenderer_endSection(r);
            break;

        case node_labeled_statement:
            if (n->data.labeled_statement.label.len == 0) {
                AstRenderer_render(r, n->data.labeled_statement.statement);
                return;
            }

            AstRenderer_beginSection(r, "labeled_statement");
            astp(r, "label: %s", Buffer_staticZ(n->data.labeled_statement.label));
            AstRenderer_render(r, n->data.labeled_statement.statement);
            AstRenderer_endSection(r);
            break;

        case node_if_expr:
            AstRenderer_beginSection(r, "if_expr");
            AstRenderer_render(r, n->data.if_expr.condition);
            AstRenderer_render(r, n->data.if_expr.expr);
            if (n->data.if_expr.else_payload_name.len != 0) {
                astp(r, "else_payload_name: %s", Buffer_staticZ(n->data.if_expr.else_payload_name));
            }
            AstRenderer_render(r, n->data.if_expr.else_payload_expr);
            AstRenderer_endSection(r);
            break;

        case node_var_decl_statement:
            AstRenderer_beginSection(r, "var_decl_statement");
            AstRenderer_render(r, n->data.var_decl_statement.var_decl);
            AstRenderer_render(r, n->data.var_decl_statement.expr);
            for (uint32_t i = 0; i < n->data.var_decl_statement.var_decl_additional_len; i++) {
                AstRenderer_render(r, n->data.var_decl_statement.var_decl_additional[i]);
            }
            AstRenderer_endSection(r);
            break;

        case node_single_assign_expr:
            AstRenderer_beginSection(r, "single_assign_expr");
            astp(r, "op: %s", TokenTag_name(n->data.single_assign_expr.assign_op));
            AstRenderer_render(r, n->data.single_assign_expr.lhs);
            AstRenderer_render(r, n->data.single_assign_expr.rhs);
            AstRenderer_endSection(r);
            break;

        case node_multi_assign_expr:
            AstRenderer_beginSection(r, "multi_assign_expr");
            AstRenderer_endSection(r);
            break;

        case node_loop_expr:
            AstRenderer_beginSection(r, "loop_expr");
            AstRenderer_endSection(r);
            break;

        case node_continue_expr:
            AstRenderer_beginSection(r, "continue_expr");
            AstRenderer_endSection(r);
            break;

        case node_break_expr:
            AstRenderer_beginSection(r, "break_expr");
            AstRenderer_endSection(r);
            break;

        case node_while_expr:
            AstRenderer_beginSection(r, "while_expr");
            AstRenderer_endSection(r);
            break;

        case node_for_expr:
            AstRenderer_beginSection(r, "loop_statement");
            AstRenderer_endSection(r);
            break;

        case node_loop_statement:
            AstRenderer_beginSection(r, "container_decl_auto");
            AstRenderer_endSection(r);
            break;

        case node_container_decl_auto:
            AstRenderer_beginSection(r, "node_container_decl_auto");
            AstRenderer_render(r, n->data.container_decl_auto.type);
            AstRenderer_render(r, n->data.container_decl_auto.members);
            AstRenderer_endSection(r);
            break;

        case node_prefix_type_op_optional:
            AstRenderer_beginSection(r, "prefix_type_op_optional");
            AstRenderer_endSection(r);
            break;

        case node_prefix_type_op_anyframe:
            AstRenderer_beginSection(r, "prefix_type_op_anyframe");
            AstRenderer_endSection(r);
            break;

        case node_prefix_type_op_slice:
            AstRenderer_beginSection(r, "prefix_type_op_slice");
            AstRenderer_render(r, n->data.prefix_type_slice.addrspace);
            AstRenderer_render(r, n->data.prefix_type_slice.bytealign);
            astp(r, "is_allowzero: %s", boolstring((n->data.prefix_type_slice.modifiers & pointer_modifier_allowzero) != 0));
            astp(r, "is_const: %s", boolstring((n->data.prefix_type_slice.modifiers & pointer_modifier_const) != 0));
            astp(r, "is_volatile: %s", boolstring((n->data.prefix_type_slice.modifiers & pointer_modifier_volatile) != 0));
            AstRenderer_render(r, n->data.prefix_type_slice.slice);
            AstRenderer_endSection(r);
            break;

        case node_prefix_type_op_ptr:
            AstRenderer_beginSection(r, "prefix_type_op_ptr");
            AstRenderer_render(r, n->data.prefix_type_ptr.addrspace);
            AstRenderer_render(r, n->data.prefix_type_ptr.align);
            astp(r, "is_allowzero: %s", boolstring((n->data.prefix_type_ptr.modifiers & pointer_modifier_allowzero) != 0));
            astp(r, "is_const: %s", boolstring((n->data.prefix_type_ptr.modifiers & pointer_modifier_const) != 0));
            astp(r, "is_volatile: %s", boolstring((n->data.prefix_type_ptr.modifiers & pointer_modifier_volatile) != 0));
            AstRenderer_render(r, n->data.prefix_type_ptr.ptr);
            AstRenderer_endSection(r);
            break;

        case node_prefix_type_op_array:
            AstRenderer_beginSection(r, "prefix_type_op_array");
            AstRenderer_render(r, n->data.prefix_type_array.array);
            AstRenderer_endSection(r);
            break;

        case node_ptr_align_expr:
            AstRenderer_beginSection(r, "ptr_align_expr");
            AstRenderer_endSection(r);
            break;

        case node_array_type_start:
            AstRenderer_beginSection(r, "array_type_start");
            AstRenderer_endSection(r);
            break;

        case node_ptr_type_start:
            AstRenderer_beginSection(r, "ptr_type_start");
            AstRenderer_endSection(r);
            break;

        case node_slice_type_start:
            AstRenderer_beginSection(r, "slice_type_start");
            AstRenderer_endSection(r);
            break;

        case node_suffix_type_op_slice:
            AstRenderer_beginSection(r, "suffix_slice");
            AstRenderer_render(r, n->data.suffix_type_op_slice.start_expr);
            AstRenderer_render(r, n->data.suffix_type_op_slice.end_expr);
            AstRenderer_render(r, n->data.suffix_type_op_slice.sentinel_expr);
            AstRenderer_endSection(r);
            break;

        case node_suffix_type_op_named_access:
            AstRenderer_beginSection(r, "suffix_member");
            astp(r, ".%s", Buffer_staticZ(n->data.suffix_type_op_named_access.name));
            AstRenderer_endSection(r);
            break;

        case node_suffix_type_op_deref:
            AstRenderer_beginSection(r, "suffix_deref");
            astp(r, ".*");
            AstRenderer_endSection(r);
            break;

        case node_suffix_type_op_assert_maybe:
            AstRenderer_beginSection(r, "suffix_type_op_assert_maybe");
            astp(r, ".?");
            AstRenderer_endSection(r);
            break;

        case node_fn_call_arguments:
            if (n->data.fn_call_arguments.exprs_len == 0) return;

            AstRenderer_beginSection(r, "fn_call_arguments");
            for (uint32_t i = 0; i < n->data.fn_call_arguments.exprs_len; i++) {
                AstRenderer_render(r, n->data.fn_call_arguments.exprs[i]);
            }
            AstRenderer_endSection(r);
            break;

        case node_for_prefix:
            AstRenderer_beginSection(r, "for_prefix");
            AstRenderer_endSection(r);
            break;

        case node_while_prefix:
            AstRenderer_beginSection(r, "while_prefix");
            AstRenderer_endSection(r);
            break;

        case node_if_prefix:
            AstRenderer_beginSection(r, "if_prefix");
            AstRenderer_render(r, n->data.if_prefix.condition);
            AstRenderer_render(r, n->data.if_prefix.ptr_payload);
            AstRenderer_endSection(r);
            break;

        case node_payload:
            AstRenderer_beginSection(r, "payload");
            astp(r, "name: %s%s", n->data.payload.is_pointer ? "*" : "", Buffer_staticZ(n->data.payload.name));
            AstRenderer_endSection(r);
            break;

        case node_payload_index:
            AstRenderer_beginSection(r, "payload_index");
            AstRenderer_endSection(r);
            break;

        case node_payload_list:
            AstRenderer_beginSection(r, "payload_list");
            AstRenderer_endSection(r);
            break;

        case node_switch_prong:
            AstRenderer_beginSection(r, "switch_prong");
            AstRenderer_endSection(r);
            break;

        case node_for_type_expr:
            AstRenderer_beginSection(r, "for_type_expr");
            AstRenderer_endSection(r);
            break;

        case node_switch_prong_list:
            AstRenderer_beginSection(r, "switch_prong_list");
            AstRenderer_endSection(r);
            break;

        case node_container_decl:
            AstRenderer_beginSection(r, "node_container_decl");
            astp(r, "is_extern: %s", boolstring(n->data.container_decl.is_extern));
            astp(r, "is_packed: %s", boolstring(n->data.container_decl.is_packed));
            AstRenderer_render(r, n->data.container_decl.container_decl);
            AstRenderer_endSection(r);
            break;

        case node_if_type_expr:
            AstRenderer_beginSection(r, "node_if_type_expr");
            AstRenderer_endSection(r);
            break;

        case node_while_type_expr:
            AstRenderer_beginSection(r, "node_while_type_expr");
            AstRenderer_endSection(r);
            break;

        case node_identifier_list:
            AstRenderer_beginSection(r, "node_identifier_list");
            AstRenderer_endSection(r);
            break;

        case node_switch_expr:
            AstRenderer_beginSection(r, "node_switch_expr");
            AstRenderer_endSection(r);
            break;

        case node_init_list_field:
            if (n->data.init_list_field.nodes_len == 0) return;

            AstRenderer_beginSection(r, "node_init_list_field");
            for (uint32_t i = 0; i < n->data.init_list_field.nodes_len; i++) {
                AstRenderer_render(r, n->data.init_list_field.nodes[i]);
            }
            AstRenderer_endSection(r);
            break;

        case node_init_list_expr:
            AstRenderer_beginSection(r, "node_init_list_expr");
            AstRenderer_endSection(r);
            break;

        case node_init_list_empty:
            AstRenderer_beginSection(r, "node_init_list_empty");
            AstRenderer_endSection(r);
            break;

        case node_asm_input_list:
            AstRenderer_beginSection(r, "node_asm_input_list");
            AstRenderer_endSection(r);
            break;

        case node_asm_output_list:
            AstRenderer_beginSection(r, "node_asm_output_list");
            AstRenderer_endSection(r);
            break;

        case node_asm_input_item:
            AstRenderer_beginSection(r, "node_asm_input_item");
            AstRenderer_endSection(r);
            break;

        case node_asm_output_item:
            AstRenderer_beginSection(r, "node_asm_output_item");
            AstRenderer_endSection(r);
            break;

        case node_asm_input:
            AstRenderer_beginSection(r, "node_asm_input");
            AstRenderer_endSection(r);
            break;

        case node_asm_output:
            AstRenderer_beginSection(r, "node_asm_output");
            AstRenderer_endSection(r);
            break;

        case node_asm_expr:
            AstRenderer_beginSection(r, "node_asm_expr");
            AstRenderer_endSection(r);
            break;

        case node_type_or_name:
            AstRenderer_beginSection(r, "node_type_or_name");
            AstRenderer_endSection(r);
            break;

        case node_invalid:
            std_panic("unreachable");
    }
}

#undef boolstring
#undef astp

__attribute__((unused))
static void Parser_debugNode(Parser *p, Node *n, const char *prefix)
{
    std_printf("@%s\n", prefix);
    AstRenderer r;
    AstRenderer_init(&r, p);
    AstRenderer_render(&r, n);
}
