typedef struct Node Node;

typedef struct {
    Node **decls;
    uint32_t decls_len;
    Node **fields;
    uint32_t fields_len;
} NodeDataContainerMembers;

typedef struct {
    Buffer name;
    Node *type_expr;
    Node *bytealign;
    Node *expr;
    bool is_comptime;
} NodeDataContainerField;

typedef struct {
    Buffer name;
    Node *block;
    bool is_ident;
} NodeDataTestDecl;

typedef struct {
    Node *block;
} NodeDataComptimeDecl;

typedef struct {
    Buffer name;
    Node *type;
    Node *bytealign;
    Node *addrspace;
    Node *linksection;
    bool is_const;
} NodeDataVarDeclProto;

typedef struct {
    Node *var_decl_proto;
    Node *expr;
} NodeDataGlobalVarDecl;

typedef enum {
    decl_modifier_export        = 0x01,
    decl_modifier_extern        = 0x02,
    decl_modifier_inline        = 0x04,
    decl_modifier_noinline      = 0x08,
    decl_modifier_threadlocal   = 0x10,
} DeclModifiers;

typedef struct {
    Node *fn_proto;
    Node *block;
    DeclModifiers modifiers;
    Buffer extern_name;
} NodeDataDeclFn;

typedef struct {
    Node *global_var_decl;
    DeclModifiers modifiers;
    Buffer extern_name;
} NodeDataDeclGlobalVarDecl;

typedef struct {
    Node **statements;
    uint32_t statements_len;
} NodeDataBlock;

typedef struct {
    Buffer name;
    Node *params;
    Node *return_type;
    Node *extra_data;
    bool is_return_type_error;
} NodeDataFnProto;

typedef struct {
    Node *bytealign;
    Node *addrspace;
    Node *linksection;
    Node *callconv;
} NodeDataFnProtoExtra;

typedef struct {
    Node **params;
    uint32_t params_len;
} NodeDataParamDeclList;

typedef struct {
    bool is_varargs;
    TokenTag modifier;
    Node *type;
    Buffer identifier;
} NodeDataParamDecl;

typedef struct {
    Node **prefix_type_ops;
    uint32_t prefix_type_ops_len;
    Node *type_expr;
} NodeDataTypeExpr;

typedef struct {
    Node *suffix_expr;
    Node *error_type_expr;
} NodeDataErrorUnionExpr;

typedef struct {
    Node *expr;
} NodeDataExpr;

typedef struct {
    Node *comptime_statement;
} NodeDataComptimeStatement;

typedef struct {
    Node *block_expr;
} NodeDataBlockExprStatement;

typedef struct {
    Buffer payload_name;
    Node *block_expr;
} NodeDataErrdeferStatement;

// Does not include assignment operators (as these have their own rules).
typedef enum BinOp {
    binop_or,
    binop_and,
    binop_eq,
    binop_neq,
    binop_lt,
    binop_gt,
    binop_lt_eq,
    binop_gt_eq,
    binop_bit_and,
    binop_bit_or,
    binop_bit_xor,
    binop_orelse,
    binop_catch,
    binop_shl,
    binop_shr,
    binop_shl_saturate,
    binop_add,
    binop_add_wrap,
    binop_add_saturate,
    binop_sub,
    binop_sub_wrap,
    binop_sub_saturate,
    binop_array_spread,
    binop_array_concat,
    binop_mul,
    binop_mul_wrap,
    binop_mul_saturate,
    binop_div,
    binop_mod,
    binop_error_set_merge,
    binop_invalid,
} BinOp;

typedef struct {
    TokenTag *ops;
    uint32_t ops_len;
    Node *expr;
} NodeDataUnaryExpr;

typedef struct {
    BinOp op;
    Node *lhs;
    Node *rhs;
} NodeDataBinaryExpr;

typedef struct {
    Node *type;
    Node *initlist;
} NodeDataCurlySuffixExpr;

typedef enum {
    node_primary_type_number_literal,
    node_primary_type_identifier,
    node_primary_type_builtin,
    node_primary_type_char_literal,
    node_primary_type_container_decl,
    node_primary_type_dot_identifier,
    node_primary_type_dot_initlist,
    node_primary_type_error_set_decl,
    node_primary_type_fn_proto,
    node_primary_type_grouped_expr,
    node_primary_type_labeled_type_expr,
    node_primary_type_if_type_expr,
    node_primary_type_comptime_type_expr,
    node_primary_type_error,
    node_primary_type_anyframe,
    node_primary_type_unreachable,
    node_primary_type_string_literal,
    node_primary_type_anytype,
} NodePrimaryTypeTag;

const char* NodePrimaryTypeTag_name(NodePrimaryTypeTag tag)
{
    switch (tag) {
        case node_primary_type_number_literal:
            return "node_primary_type_number_literal";
        case node_primary_type_identifier:
            return "node_primary_type_identifier";
        case node_primary_type_builtin:
            return "node_primary_type_builtin";
        case node_primary_type_char_literal:
            return "node_primary_type_container_decl";
        case node_primary_type_container_decl:
            return "node_primary_type_container_decl";
        case node_primary_type_dot_identifier:
            return "node_primary_type_dot_identifier";
        case node_primary_type_dot_initlist:
            return "node_primary_type_dot_initlist";
        case node_primary_type_error_set_decl:
            return "node_primary_type_error_set_decl";
        case node_primary_type_fn_proto:
            return "node_primary_type_fn_proto";
        case node_primary_type_grouped_expr:
            return "node_primary_type_grouped_expr";
        case node_primary_type_labeled_type_expr:
            return "node_primary_type_labeled_type_expr";
        case node_primary_type_if_type_expr:
            return "node_primary_type_if_type_expr";
        case node_primary_type_comptime_type_expr:
            return "node_primary_type_comptime_type_expr";
        case node_primary_type_error:
            return "node_primary_type_error";
        case node_primary_type_anyframe:
            return "node_primary_type_anyframe";
        case node_primary_type_unreachable:
            return "node_primary_type_unreachable";
        case node_primary_type_string_literal:
            return "node_primary_type_string_literal";
        case node_primary_type_anytype:
            return "node_primary_type_anytype";
    }
}

typedef struct {
    Buffer name;
    Node *args;
} NodePrimaryTypeDataBuiltin;

typedef union {
    Buffer raw;
    Node *node;
    NodePrimaryTypeDataBuiltin builtin;
} NodePrimaryTypeData;

typedef struct {
    NodePrimaryTypeTag tag;
    NodePrimaryTypeData data;
} NodeDataPrimaryTypeExpr;

typedef struct {
    Node *decl;
    bool is_pub;
} NodeDataTopLevelDecl;

typedef struct {
    Node *for_start;
    Node *for_end;
    bool is_range;
} NodeDataForItem;

typedef struct {
    Node **args;
    uint32_t args_len;
} NodeDataForArgs;

typedef struct {
    Buffer name;
    Node *expr;
} NodeDataFieldInit;

typedef struct {
    Node *expr;
    bool is_tagged;
} NodeDataUnionDecl;

typedef struct {
    Node *start;
    Node *end;
} NodeDataSwitchItem;

typedef struct {
    Node **cases;
    uint32_t cases_len;
    bool is_else;
} NodeDataSwitchCase;

typedef struct {
    Buffer label;
    Node *node;
} NodeDataLabeledTypeExpr;

typedef struct {
    Buffer label;
    Node *statement;
} NodeDataLabeledStatement;

typedef struct {
    Node *condition;
    Node *block;
    Buffer else_payload_name;
    Node *else_statement;
} NodeDataWhileStatement;

typedef struct {
    Node *condition;
    Node *block;
    Node *else_statement;
} NodeDataForStatement;

typedef struct {
    Node *condition;
    Node *block;
    Buffer else_payload_name;
    Node *else_statement;
} NodeDataIfStatement;

typedef struct {
    Node *condition;
    Node *expr;
    Buffer else_payload_name;
    Node *else_payload_expr;
} NodeDataIfExpr;

typedef struct {
    Node *var_decl;
    Node **var_decl_additional;
    uint32_t var_decl_additional_len;
    Node *expr;
} NodeDataVarDeclStatement;

typedef struct {
    Node *lhs;
    TokenTag assign_op;
    Node *rhs;
} NodeDataSingleAssignExpr;

typedef struct {
    Node *lhs;
    Node **lhs_additional;
    uint32_t lhs_additional_len;
    Node *expr;
} NodeDataMultiAssignExpr;

typedef struct {
    Buffer label;
    Node *loop_expr;
} NodeDataLoopExpr;

typedef struct {
    Buffer label;
    Node *expr;
} NodeDataContinueExpr;

typedef struct {
    Buffer label;
    Node *expr;
} NodeDataBreakExpr;

typedef struct {
    Node *condition;
    Node *expr;
    Buffer else_payload_name;
    Node *else_expr;
} NodeDataWhileExpr;

typedef struct {
    Node *condition;
    Node *expr;
    Node *else_expr;
} NodeDataForExpr;

typedef struct {
    bool is_inline;
    Node *statement;
} NodeDataLoopStatement;

typedef struct {
    Node *type;
    Node *members;
} NodeDataContainerDeclAuto;

typedef struct {
    Node *byte_align;
    Node *bit_offset;
    Node *bit_backing_integer_size;
} NodeDataPtrAlignExpr;

typedef enum {
    pointer_modifier_const      = 0x01,
    pointer_modifier_volatile   = 0x02,
    pointer_modifier_allowzero  = 0x04,
} PointerModifiers;

typedef struct {
    Node *slice;
    Node *bytealign;
    Node *addrspace;
    PointerModifiers modifiers;
} NodeDataPrefixTypeSlice;

typedef struct {
    Node *ptr;
    Node *addrspace;
    Node *align;
    PointerModifiers modifiers;
} NodeDataPrefixTypePtr;

typedef struct {
    Node *array;
} NodeDataPrefixTypeArray;

typedef struct {
    Node *index;
    Node *sentinel_expr;
} NodeDataArrayTypeStart;

typedef struct {
    Node *sentinel_expr;
} NodeDataSliceTypeStart;

typedef enum {
   node_ptr_type_single,
   node_ptr_type_double,
   node_ptr_type_multi,
   node_ptr_type_c,
   node_ptr_type_sentinel,
} NodePtrType;

typedef struct {
    NodePtrType type;
    Node *sentinel_expr;
} NodeDataPtrTypeStart;

typedef struct {
    Node *start_expr;
    Node *end_expr;
    Node *sentinel_expr;
} NodeDataSuffixTypeOpSlice;

typedef struct {
    Buffer name;
} NodeDataSuffixTypeOpNamedAccess;

typedef struct {
    Node **exprs;
    uint32_t exprs_len;
} NodeDataFnCallArguments;

typedef struct {
    Node *expr;
    Node **suffixes;
    uint32_t suffixes_len;
} NodeDataSuffixExpr;

typedef struct {
    Node *for_args;
    Node *ptr_list_payload;
} NodeDataForPrefix;

typedef struct {
    Node *condition;
    Node *ptr_payload;
    Node *while_continue_expr;
} NodeDataWhilePrefix;

typedef struct {
    Node *condition;
    Node *ptr_payload;
} NodeDataIfPrefix;

typedef struct {
    Buffer name;
    bool is_pointer;
} NodeDataPayload;

typedef struct {
    Buffer name;
    bool is_pointer;
    Buffer name_index;
} NodeDataPayloadIndex;

typedef struct {
    Node **payloads;
    uint32_t payloads_len;
} NodeDataPayloadList;

typedef struct {
    bool is_inline;
    Node *switch_case;
    Node *payload;
    Node *expr;
} NodeDataSwitchProng;

typedef struct {
    Node **prongs;
    uint32_t prongs_len;
} NodeDataSwitchProngList;

typedef struct {
    Node *for_prefix;
    Node *condition;
    Node *expr;
    Node *else_expr;
} NodeDataForTypeExpr;

typedef struct {
    bool is_extern;
    bool is_packed;
    Node *container_decl;
} NodeDataContainerDecl;

typedef struct {
    Node *if_prefix;
    Node *type_expr;
    Buffer else_payload_name;
    Node *else_payload_type_expr;
} NodeDataIfTypeExpr;

typedef struct {
    Node *while_prefix;
    Node *type_expr;
    Buffer else_payload_name;
    Node *else_payload_type_expr;
} NodeDataWhileTypeExpr;

typedef struct {
    Buffer *idents;
    uint32_t idents_len;
} NodeDataIdentifierList;

typedef struct {
    Node *expr;
    Node *switch_prong_list;
} NodeDataSwitchExpr;

typedef struct {
    Node **nodes;
    uint32_t nodes_len;
} NodeDataInitList;

typedef struct {
    Node **asm_inputs;
    uint32_t asm_inputs_len;
} NodeDataAsmInputList;

typedef struct {
    Buffer name;
    Buffer lit;
    Node *input_expr;
} NodeDataAsmInputItem;

typedef struct {
    Node **asm_outputs;
    uint32_t asm_outputs_len;
} NodeDataAsmOutputList;

typedef struct {
    Buffer name;
    Buffer lit;
    Node *output_expr;
} NodeDataAsmOutputItem;

typedef struct {
    Node *asm_output_list;
    Node *asm_input;
} NodeDataAsmOutput;

typedef struct {
    Node *asm_input_list;
    Node *clobbers;
} NodeDataAsmInput;

typedef struct {
    bool is_volatile;
    Node *expr;
    Node *asm_output;
} NodeDataAsmExpr;

typedef struct {
    Node *type;
    Buffer name;
    bool is_type;
} NodeDataTypeOrName;

typedef union NodeData {
    NodeDataContainerMembers container_members;
    NodeDataContainerField container_field;
    NodeDataTestDecl test_decl;
    NodeDataComptimeDecl comptime_decl;
    NodeDataVarDeclProto var_decl_proto;
    NodeDataGlobalVarDecl global_var_decl;
    NodeDataDeclFn decl_fn;
    NodeDataDeclGlobalVarDecl decl_global_var_decl;
    NodeDataBlock block;
    NodeDataFnProto fn_proto;
    NodeDataFnProtoExtra fn_proto_extra;
    NodeDataParamDeclList param_decl_list;
    NodeDataParamDecl param_decl;
    NodeDataTypeExpr type_expr;
    NodeDataErrorUnionExpr error_union_expr;
    NodeDataSuffixExpr suffix_expr;
    NodeDataComptimeStatement comptime_statement;
    NodeDataBlockExprStatement nosuspend_statement;
    NodeDataBlockExprStatement suspend_statement;
    NodeDataBlockExprStatement defer_statement;
    NodeDataErrdeferStatement errdefer_statement;
    NodeDataUnaryExpr unary_expr;
    NodeDataBinaryExpr binary_expr;
    Node* comptime_expr;
    Node* nosuspend_expr;
    Node* resume_expr;
    Node* return_expr;
    NodeDataCurlySuffixExpr curly_suffix_expr;
    NodeDataPrimaryTypeExpr primary_type_expr;
    NodeDataTopLevelDecl top_level_decl;
    NodeDataForItem for_item;
    NodeDataForArgs for_args;
    NodeDataFieldInit field_init;
    Node* struct_decl;
    Node* enum_decl;
    NodeDataUnionDecl union_decl;
    NodeDataSwitchItem switch_item;
    NodeDataSwitchCase switch_case;
    NodeDataLabeledTypeExpr labeled_type_expr;
    NodeDataWhileStatement while_statement;
    NodeDataForStatement for_statement;
    NodeDataIfStatement if_statement;
    NodeDataLabeledStatement labeled_statement;
    NodeDataIfExpr if_expr;
    NodeDataVarDeclStatement var_decl_statement;
    NodeDataSingleAssignExpr single_assign_expr;
    NodeDataMultiAssignExpr multi_assign_expr;
    NodeDataLoopExpr loop_expr;
    NodeDataContinueExpr continue_expr;
    NodeDataBreakExpr break_expr;
    NodeDataWhileExpr while_expr;
    NodeDataForExpr for_expr;
    NodeDataLoopStatement loop_statement;
    NodeDataContainerDeclAuto container_decl_auto;
    NodeDataPrefixTypeSlice prefix_type_slice;
    NodeDataPrefixTypePtr prefix_type_ptr;
    NodeDataPrefixTypeArray prefix_type_array;
    NodeDataPtrAlignExpr ptr_align_expr;
    NodeDataArrayTypeStart array_type_start;
    NodeDataSliceTypeStart slice_type_start;
    NodeDataPtrTypeStart ptr_type_start;
    NodeDataSuffixTypeOpNamedAccess suffix_type_op_named_access;
    NodeDataSuffixTypeOpSlice suffix_type_op_slice;
    NodeDataFnCallArguments fn_call_arguments;
    NodeDataForPrefix for_prefix;
    NodeDataWhilePrefix while_prefix;
    NodeDataIfPrefix if_prefix;
    NodeDataPayload payload;
    NodeDataPayloadIndex payload_index;
    NodeDataPayloadList payload_list;
    NodeDataSwitchProng switch_prong;
    NodeDataForTypeExpr for_type_expr;
    NodeDataSwitchProngList switch_prong_list;
    NodeDataContainerDecl container_decl;
    NodeDataIfTypeExpr if_type_expr;
    NodeDataWhileTypeExpr while_type_expr;
    NodeDataIdentifierList identifier_list;
    NodeDataSwitchExpr switch_expr;
    NodeDataInitList init_list_field;
    NodeDataInitList init_list_expr;
    NodeDataAsmInputList asm_input_list;
    NodeDataAsmOutputList asm_output_list;
    NodeDataAsmInputItem asm_input_item;
    NodeDataAsmOutputItem asm_output_item;
    NodeDataAsmInput asm_input;
    NodeDataAsmOutput asm_output;
    NodeDataAsmExpr asm_expr;
    NodeDataTypeOrName type_or_name;
} NodeData;

typedef enum NodeTag {
    node_container_members,
    node_container_field,
    node_test_decl,
    node_comptime_decl,
    node_var_decl_proto,
    node_global_var_decl,
    node_decl_fn,
    node_decl_global_var_decl,
    node_block,
    node_fn_proto,
    node_fn_proto_extra,
    node_param_decl_list,
    node_param_decl,
    node_type_expr,
    node_error_union_expr,
    node_suffix_expr,
    node_comptime_statement,
    node_nosuspend_statement,
    node_suspend_statement,
    node_defer_statement,
    node_errdefer_statement,
    node_unary_expr,
    node_binary_expr,
    node_comptime_expr,
    node_nosuspend_expr,
    node_resume_expr,
    node_return_expr,
    node_curly_suffix_expr,
    node_primary_type_expr,
    node_top_level_decl,
    node_for_item,
    node_for_args,
    node_field_init,
    node_struct_decl,
    node_opaque_decl,
    node_enum_decl,
    node_union_decl,
    node_switch_item,
    node_switch_case,
    node_labeled_block,
    node_labeled_loop_expr,
    node_labeled_switch_expr,
    node_while_statement,
    node_for_statement,
    node_if_statement,
    node_labeled_statement,
    node_if_expr,
    node_var_decl_statement,
    node_single_assign_expr,
    node_multi_assign_expr,
    node_loop_expr,
    node_continue_expr,
    node_break_expr,
    node_while_expr,
    node_for_expr,
    node_loop_statement,
    node_container_decl_auto,
    node_prefix_type_op_optional,
    node_prefix_type_op_anyframe,
    node_prefix_type_op_slice,
    node_prefix_type_op_ptr,
    node_prefix_type_op_array,
    node_ptr_align_expr,
    node_array_type_start,
    node_ptr_type_start,
    node_slice_type_start,
    node_suffix_type_op_slice,
    node_suffix_type_op_named_access,
    node_suffix_type_op_deref,
    node_suffix_type_op_assert_maybe,
    node_fn_call_arguments,
    node_for_prefix,
    node_while_prefix,
    node_if_prefix,
    node_payload,
    node_payload_index,
    node_payload_list,
    node_switch_prong,
    node_for_type_expr,
    node_switch_prong_list,
    node_container_decl,
    node_if_type_expr,
    node_while_type_expr,
    node_identifier_list,
    node_switch_expr,
    node_init_list_field,
    node_init_list_expr,
    node_init_list_empty,
    node_asm_input_list,
    node_asm_output_list,
    node_asm_input_item,
    node_asm_output_item,
    node_asm_input,
    node_asm_output,
    node_asm_expr,
    node_type_or_name,
    node_invalid,
} NodeTag;

__attribute__((unused))
static const char* NodeTag_name(NodeTag tag)
{
    switch (tag) {
        case node_container_members:
            return "node_container_members";
        case node_container_field:
            return "node_container_field";
        case node_test_decl:
            return "node_test_decl";
        case node_comptime_decl:
            return "node_comptime_decl";
        case node_var_decl_proto:
            return "node_var_decl_proto";
        case node_global_var_decl:
            return "node_global_var_decl";
        case node_decl_fn:
            return "node_decl_fn";
        case node_decl_global_var_decl:
            return "node_decl_global_var_decl";
        case node_block:
            return "node_block";
        case node_fn_proto:
            return "node_fn_proto";
        case node_fn_proto_extra:
            return "node_fn_proto_extra";
        case node_param_decl_list:
            return "node_param_decl_list";
        case node_param_decl:
            return "node_param_decl";
        case node_type_expr:
            return "node_type_expr";
        case node_error_union_expr:
            return "node_error_union_expr";
        case node_suffix_expr:
            return "node_suffix_expr";
        case node_comptime_statement:
            return "node_comptime_statement";
        case node_nosuspend_statement:
            return "node_nosuspend_statement";
        case node_suspend_statement:
            return "node_suspend_statement";
        case node_defer_statement:
            return "node_defer_statement";
        case node_errdefer_statement:
            return "node_errdefer_statement";
        case node_unary_expr:
            return "node_unary_expr";
        case node_binary_expr:
            return "node_binary_expr";
        case node_comptime_expr:
            return "node_comptime_expr";
        case node_nosuspend_expr:
            return "node_nosuspend_expr";
        case node_resume_expr:
            return "node_resume_expr";
        case node_return_expr:
            return "node_return_expr";
        case node_curly_suffix_expr:
            return "node_curly_suffix_expr";
        case node_primary_type_expr:
            return "node_primary_type_expr";
        case node_top_level_decl:
            return "node_top_level_decl";
        case node_for_item:
            return "node_for_item";
        case node_for_args:
            return "node_for_args";
        case node_field_init:
            return "node_field_init";
        case node_struct_decl:
            return "node_struct_decl";
        case node_opaque_decl:
            return "node_opaque_decl";
        case node_enum_decl:
            return "node_enum_decl";
        case node_union_decl:
            return "node_union_decl";
        case node_switch_item:
            return "node_switch_item";
        case node_switch_case:
            return "node_switch_case";
        case node_labeled_block:
            return "node_labeled_block";
        case node_labeled_loop_expr:
            return "node_labeled_loop_expr";
        case node_labeled_switch_expr:
            return "node_labeled_switch_expr";
        case node_while_statement:
            return "node_while_statement";
        case node_for_statement:
            return "node_for_statement";
        case node_if_statement:
            return "node_if_statement";
        case node_labeled_statement:
            return "node_labeled_statement";
        case node_if_expr:
            return "node_if_expr";
        case node_var_decl_statement:
            return "node_var_decl_statement";
        case node_single_assign_expr:
            return "node_single_assign_expr";
        case node_multi_assign_expr:
            return "node_multi_assign_expr";
        case node_loop_expr:
            return "node_loop_expr";
        case node_continue_expr:
            return "node_continue_expr";
        case node_break_expr:
            return "node_break_expr";
        case node_while_expr:
            return "node_while_expr";
        case node_for_expr:
            return "node_for_expr";
        case node_loop_statement:
            return "node_loop_statement";
        case node_container_decl_auto:
            return "node_container_decl_auto";
        case node_prefix_type_op_optional:
            return "node_prefix_type_op_optional";
        case node_prefix_type_op_anyframe:
            return "node_prefix_type_op_anyframe";
        case node_prefix_type_op_slice:
            return "node_prefix_type_op_slice";
        case node_prefix_type_op_ptr:
            return "node_prefix_type_op_ptr";
        case node_prefix_type_op_array:
            return "node_prefix_type_op_array";
        case node_ptr_align_expr:
            return "node_ptr_align_expr";
        case node_array_type_start:
            return "node_array_type_start";
        case node_ptr_type_start:
            return "node_ptr_type_start";
        case node_slice_type_start:
            return "node_slice_type_start";
        case node_suffix_type_op_slice:
            return "node_suffix_type_op_slice";
        case node_suffix_type_op_named_access:
            return "node_suffix_type_op_named_access";
        case node_suffix_type_op_deref:
            return "node_suffix_type_op_deref";
        case node_suffix_type_op_assert_maybe:
            return "node_suffix_type_op_assert_maybe";
        case node_fn_call_arguments:
            return "node_fn_call_arguments";
        case node_for_prefix:
            return "node_for_prefix";
        case node_while_prefix:
            return "node_while_prefix";
        case node_if_prefix:
            return "node_if_prefix";
        case node_payload:
            return "node_payload";
        case node_payload_index:
            return "node_payload_index";
        case node_payload_list:
            return "node_payload_list";
        case node_switch_prong:
            return "node_switch_prong";
        case node_for_type_expr:
            return "node_for_type_expr";
        case node_switch_prong_list:
            return "node_switch_prong_list";
        case node_container_decl:
            return "node_container_decl";
        case node_if_type_expr:
            return "node_if_type_expr";
        case node_while_type_expr:
            return "node_while_type_expr";
        case node_identifier_list:
            return "node_identifier_list";
        case node_switch_expr:
            return "node_switch_expr";
        case node_init_list_field:
            return "node_init_list_field";
        case node_init_list_expr:
            return "node_init_list_expr";
        case node_init_list_empty:
            return "node_init_list_empty";
        case node_asm_input_list:
            return "node_asm_input_list";
        case node_asm_output_list:
            return "node_asm_output_list";
        case node_asm_input_item:
            return "node_asm_input_item";
        case node_asm_output_item:
            return "node_asm_output_item";
        case node_asm_input:
            return "node_asm_input";
        case node_asm_output:
            return "node_asm_output";
        case node_asm_expr:
            return "node_asm_expr";
        case node_type_or_name:
            return "node_type_or_name";
        case node_invalid:
            return "node_invalid";
    }
}

struct Node {
    NodeTag tag;
    NodeData data;
};