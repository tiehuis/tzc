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
    astp(r, "%s", name);
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

        case node_param_decl_list:
            AstRenderer_beginSection(r, "param_decl_list");
            AstRenderer_endSection(r);
            break;

        case node_param_decl:
            AstRenderer_beginSection(r, "param_decl");
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
            AstRenderer_beginSection(r, "error_union_expr");
            AstRenderer_render(r, n->data.error_union_expr.error_type_expr);
            AstRenderer_render(r, n->data.error_union_expr.suffix_expr);
            AstRenderer_endSection(r);
            break;

        case node_suffix_expr:
            AstRenderer_beginSection(r, "suffix_expr");
            AstRenderer_render(r, n->data.suffix_expr.expr);
            for (uint32_t i = 0; i < n->data.suffix_expr.suffixes_len; i++) {
                AstRenderer_render(r, n->data.suffix_expr.suffixes[i]);
            }
            AstRenderer_endSection(r);
            break;

        case node_comptime_statement:
            AstRenderer_beginSection(r, "comptime_statement");
            AstRenderer_endSection(r);
            break;

        case node_nosuspend_statement:
            AstRenderer_beginSection(r, "nosuspend_statement");
            AstRenderer_endSection(r);
            break;

        case node_suspend_statement:
            AstRenderer_beginSection(r, "suspend_statement");
            AstRenderer_endSection(r);
            break;

        case node_defer_statement:
            AstRenderer_beginSection(r, "defer_statement");
            AstRenderer_endSection(r);
            break;

        case node_errdefer_statement:
            AstRenderer_beginSection(r, "errdefer_statement");
            AstRenderer_endSection(r);
            break;

        case node_unary_expr:
            AstRenderer_beginSection(r, "unary_expr");
            if (n->data.unary_expr.ops_len > 0) {
                AstRenderer_beginSection(r, "ops");
                for (uint32_t i = 0; i < n->data.unary_expr.ops_len; i++) {
                    astp(r, "%d\n", n->data.unary_expr.ops[i]);
                }
                AstRenderer_endSection(r);
            }
            AstRenderer_render(r, n->data.unary_expr.expr);
            AstRenderer_endSection(r);
            break;

        case node_binary_expr:
            AstRenderer_beginSection(r, "binary_expr");
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
            astp(r, "payload_name:", n->data.if_statement.else_payload_name);
            AstRenderer_render(r, n->data.if_statement.else_statement);
            AstRenderer_endSection(r);
            break;

        case node_labeled_statement:
            AstRenderer_beginSection(r, "labeled_statement");
            AstRenderer_endSection(r);
            break;

        case node_if_expr:
            AstRenderer_beginSection(r, "if_expr");
            AstRenderer_endSection(r);
            break;

        case node_var_decl_statement:
            AstRenderer_beginSection(r, "var_decl_statement");
            AstRenderer_endSection(r);
            break;

        case node_single_assign_expr:
            AstRenderer_beginSection(r, "single_assign_expr");
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
            AstRenderer_beginSection(r, "prefix_type_op_optional");
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
            AstRenderer_endSection(r);
            break;

        case node_prefix_type_op_ptr:
            AstRenderer_beginSection(r, "prefix_type_op_ptr");
            AstRenderer_endSection(r);
            break;

        case node_prefix_type_op_array:
            AstRenderer_beginSection(r, "prefix_type_op_array");
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
            AstRenderer_beginSection(r, "suffix_type_op_slice");
            AstRenderer_endSection(r);
            break;

        case node_suffix_type_op_named_access:
            AstRenderer_beginSection(r, "suffix_type_op_named_access");
            AstRenderer_endSection(r);
            break;

        case node_suffix_type_op_deref:
            AstRenderer_beginSection(r, "suffix_type_op_deref");
            AstRenderer_endSection(r);
            break;

        case node_suffix_type_op_assert_maybe:
            AstRenderer_beginSection(r, "suffix_type_op_assert_maybe");
            AstRenderer_endSection(r);
            break;

        case node_fn_call_arguments:
            AstRenderer_beginSection(r, "fn_call_arguments");
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
            AstRenderer_endSection(r);
            break;

        case node_payload:
            AstRenderer_beginSection(r, "payload");
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
            AstRenderer_beginSection(r, "node_init_list_field");
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

        case node_invalid:
            std_panic("unreachable");
    }
}

#undef boolstring
#undef astp
