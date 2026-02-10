static tInternId Sema_resolveBuiltinTypeId(Ctx *ctx, Buffer b)
{
    typedef struct {
        tTypeTag tag;
        char *name;
    } TypeMapping;

    static TypeMapping mappings[] = {
        { .name = "u8",    .tag = ty_u8 },
        { .name = "u16",   .tag = ty_u16 },
        { .name = "u32",   .tag = ty_u32 },
        { .name = "u64",   .tag = ty_u64 },
        { .name = "u128",  .tag = ty_u128 },
        { .name = "i8",    .tag = ty_i8 },
        { .name = "i16",   .tag = ty_i16 },
        { .name = "i32",   .tag = ty_i32 },
        { .name = "i64",   .tag = ty_i64 },
        { .name = "i128",  .tag = ty_i128 },
        { .name = "isize", .tag = ty_isize },
        { .name = "usize", .tag = ty_usize },

        { .name = "c_char",       .tag = ty_c_char },
        { .name = "c_short",      .tag = ty_c_short },
        { .name = "c_ushort",     .tag = ty_c_ushort },
        { .name = "c_int",        .tag = ty_c_int },
        { .name = "c_uint",       .tag = ty_c_uint },
        { .name = "c_long",       .tag = ty_c_long },
        { .name = "c_ulong",      .tag = ty_c_ulong },
        { .name = "c_longlong",   .tag = ty_c_longlong },
        { .name = "c_ulonglong",  .tag = ty_c_ulonglong },
        { .name = "c_longdouble",.tag = ty_c_longdouble },

        { .name = "f16",   .tag = ty_f16 },
        { .name = "f32",   .tag = ty_f32 },
        { .name = "f64",   .tag = ty_f64 },
        { .name = "f80",   .tag = ty_f80 },
        { .name = "f128",  .tag = ty_f128 },

        { .name = "bool",      .tag = ty_bool },
        { .name = "anyopaque", .tag = ty_anyopaque },
    };

    for (uint32_t i = 0; i < sizeof(mappings)/sizeof(*mappings); i++) {
        if (Buffer_eql(b, mappings[i].name)) {
            return Ctx_putType(ctx, (tType){ .tag = mappings[i].tag });
        }
    }

    return ty_invalid_id;
}

static Buffer Sema_evalSymbolName(Ctx *ctx, Node *n)
{
    switch (n->tag) {
        case node_type_expr:
            return Sema_evalSymbolName(ctx, n->data.type_expr.type_expr);
        case node_primary_type_expr:
            switch (n->data.primary_type_expr.tag) {
                case node_primary_type_identifier:
                    return n->data.primary_type_expr.data.raw;
                default:
                    std_panic("unsupported primary type expr tag");
            }
        case node_error_union_expr:
            return Sema_evalSymbolName(ctx, n->data.error_union_expr.suffix_expr);
        case node_suffix_expr:
            return Sema_evalSymbolName(ctx, n->data.suffix_expr.expr);
        case node_unary_expr:
            assume(n->data.unary_expr.ops_len == 0);
            return Sema_evalSymbolName(ctx, n->data.unary_expr.expr);
        default:
            std_panic("unsupported tag: %s\n", NodeTag_name(n->tag));
    }
}

// Given a Node*, returns a tTypeId.
static tInternId Sema_evalTypeName(Ctx *ctx, Node *n)
{
    switch (n->tag) {
        case node_type_expr:
        {
            tInternId id = Sema_evalTypeName(ctx, n->data.type_expr.type_expr);
            for (uint32_t i = 0; i < n->data.type_expr.prefix_type_ops_len; i++) {
                NodeTag tag = n->data.type_expr.prefix_type_ops[i]->tag;
                switch (tag) {
                    case node_prefix_type_op_ptr:
                    {
                        NodeDataPrefixTypePtr p = n->data.type_expr.prefix_type_ops[i]->data.prefix_type_ptr;
                        tType ty;
                        ty.data.ptr.modifiers = p.modifiers;
                        ty.data.ptr.child = id;
                        assume(p.ptr->tag == node_ptr_type_start);
                        switch (p.ptr->data.ptr_type_start.type)
                        {
                            // Ignore all the special types, just lower as-is.
                            case node_ptr_type_c:
                            case node_ptr_type_single:
                            case node_ptr_type_multi:
                            case node_ptr_type_sentinel:
                                ty.tag = ty_ptr_one;
                                break;

                            case node_ptr_type_double:
                                ty.tag = ty_ptr_two;
                                break;
                        }

                        return Ctx_putType(ctx, ty);
                    }

                    default:
                        std_panic("unsupported");
                }
            }
            return id;
        }

        case node_error_union_expr:
            return Sema_evalTypeName(ctx, n->data.error_union_expr.suffix_expr);
        case node_suffix_expr:
            return Sema_evalTypeName(ctx, n->data.suffix_expr.expr);
        case node_primary_type_expr:
            switch (n->data.primary_type_expr.tag) {
                case node_primary_type_identifier:
                {
                    tInternId id = Sema_resolveBuiltinTypeId(ctx, n->data.primary_type_expr.data.raw);
                    if (id == ty_invalid_id) {
                        std_panic("generic symbols not supported: '"PRIb"'",
                            Buffer(n->data.primary_type_expr.data.raw));
                    }
                    return id;
                }
                break;

                default:
                    std_panic("unsupported primary type expr tag");
            }
        case node_unary_expr:
            assume(n->data.unary_expr.ops_len == 0);
            return Sema_evalTypeName(ctx, n->data.unary_expr.expr);
        default:
            std_panic("unsupported tag: %s\n", NodeTag_name(n->tag));
    }
}
static tInternId Sema_peerResolveType(Ctx *ctx, tInternId a_id, tInternId b_id)
{
    if (a_id == b_id) return a_id;
    tType a = Ctx_getType(ctx, a_id);
    tType b = Ctx_getType(ctx, b_id);
    tTypeInfo a_info = tType_info(a);
    tTypeInfo b_info = tType_info(b);

    if (a_info.class == class_int && b_info.class == class_int) {
        // for now, choose one of the input types, but can generate types not either a or b
        return (a_info.bits > b_info.bits) ? a_id : b_id;
    }
    if (a_info.class == class_float && b_info.class == class_float) {
        return (a_info.bits > b_info.bits) ? a_id : b_id;
    }

    std_panic("peer resolution failed: %d <-> %d\n", a_info.class, b_info.class);
}