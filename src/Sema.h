DEFINE_ARRAY_NAMED(char, Char);

static Buffer Sema_resolveTypeName(Buffer b)
{
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

static Buffer Sema_evalTypeName0(Node *n)
{
    switch (n->tag) {
        case node_type_expr:
        {
            Buffer name = Sema_evalTypeName0(n->data.type_expr.type_expr);
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

                        assume(p.ptr->tag == node_ptr_type_start);
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
                        std_panic("unsupported");
                }
            }
            return name;
        }

        case node_error_union_expr:
            return Sema_evalTypeName0(n->data.error_union_expr.suffix_expr);
        case node_suffix_expr:
            return Sema_evalTypeName0(n->data.suffix_expr.expr);
        case node_primary_type_expr:
            switch (n->data.primary_type_expr.tag) {
                case node_primary_type_identifier:
                    return Sema_resolveTypeName(n->data.primary_type_expr.data.raw);
                default:
                    std_panic("unsupported primary type expr tag");
            }
        case node_unary_expr:
            assume(n->data.unary_expr.ops_len == 0);
            return Sema_evalTypeName0(n->data.unary_expr.expr);
        default:
            std_panic("unsupported tag: %s\n", NodeTag_name(n->tag));
    }
}

// Given a type expression, returns a c-compatible representation.
static Buffer Sema_evalTypeName(Node *n)
{
    return Sema_evalTypeName0(n);
}