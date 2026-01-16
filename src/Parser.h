// Parser_expect* functions try parse and otherwise std_panic if does not match.
// Parser_parse* calls will return NULL and roll-back to the initial token in the case of no match.
// On parse error, a single error is reported and the code will std_panic.

//#define TRACE

// TODO: can we just track the last p->index and compare against to check invalid cycles/loops
#define LOOP_MAX 3000

DEFINE_ARRAY_NAMED(Node*, Node)
DEFINE_ARRAY(TokenTag)
DEFINE_ARRAY(Buffer)

typedef struct Parser {
    uint32_t index;
    Buffer source;
    Token *tokens;
    uint32_t tokens_len;
    uint32_t nodes_count;
} Parser;

static void Parser_debugNode(Parser *p, Node *n, const char *prefix);

static void Parser_init(Parser *p, Buffer source, Token *tokens, uint32_t tokens_len)
{
    p->source = source;
    p->tokens = tokens;
    p->tokens_len = tokens_len;
    p->index = 0;
    p->nodes_count = 0;
}

#define Parser_fail(p_, ...) Parser_fail0(p_, __LINE__, "parse error: " __VA_ARGS__)
static void _Noreturn Parser_fail0(Parser *p, int line_no, const char *fmt, ...)
{
    std_printf("%d:", line_no);

    va_list args;
    va_start(args, fmt);
    std_vprintf(fmt, args);
    va_end(args);
    std_printf("\n");

    // context
    Token current = p->tokens[p->index];
    const char *s = p->source.data + current.loc.start;
    size_t offset = 0;
    while (s != p->source.data && *s != '\n') {
        s--;
        offset++;
    }
    if (*s == '\n') s++;
    const char *e = p->source.data + current.loc.end;
    while (*e != '\n' && *e != 0) e++;
    Buffer line = Buffer_slice(p->source, s - p->source.data, e - p->source.data);
    std_printf(PRIb"\n", Buffer(line));
    if (offset > 1) for (size_t i = 0; i < offset - 1; i++) std_printf(" ");
    std_printf("^\n");

    std_exit(1);
}

static bool Parser_peek(Parser *p, TokenTag tag)
{
    if (p->index >= p->tokens_len) return false;
    return p->tokens[p->index].tag == tag;
}

static bool Parser_eat(Parser *p, TokenTag tag)
{
    if (Parser_peek(p, tag)) {
        p->index++;
        return 1;
    }
    return 0;
}

#ifdef TRACE
#define Parser_trace(p) Parser_dump0(p, __func__)
#else
#define Parser_trace(p)
#endif

#define Parser_dump(p) Parser_dump0(p, __func__)
__attribute__((unused))
static void Parser_dump0(Parser *p, const char *function)
{
    Token t = p->tokens[p->index];
    Buffer token = Buffer_slice(p->source, t.loc.start, t.loc.end);
    std_printf("%d:%s:%s:"PRIb"\n", p->index, function, TokenTag_name(t.tag), Buffer(token));
}

#define Parser_expect(p, tag) Parser_expect0(p, __LINE__, __func__, tag)
static void Parser_expect0(Parser *p, int line_no, const char *function, TokenTag tag)
{
    Token found = p->tokens[p->index];
    if (!Parser_eat(p, tag)) Parser_fail0(p, line_no, "%s: expected tag %s found %s\n", function, TokenTag_name(tag), TokenTag_name(found.tag));
}

static TokenTag Parser_eatOneOf(Parser *p, TokenTag *tags, size_t tags_len)
{
    for (size_t i = 0; i < tags_len; i++) {
        if (Parser_eat(p, tags[i])) return tags[i];
    }
    return token_invalid;
}

static Buffer Parser_tokenSlice(Parser *p)
{
    Token t = p->tokens[p->index];
    return Buffer_slice(p->source, t.loc.start, t.loc.end);
}

static Buffer Parser_eatIdentifier(Parser *p)
{
    Buffer b = Parser_tokenSlice(p);
    if (Parser_eat(p, token_identifier)) return b;
    return Buffer_empty();
}

#define Parser_expectIdentifier(p) Parser_expectIdentifier0(p, __func__)
static Buffer Parser_expectIdentifier0(Parser *p, const char *function)
{
    Buffer b = Parser_tokenSlice(p);
    if (!Parser_eat(p, token_identifier)) Parser_fail(p, "%s: expected identifier\n", function);
    return b;
}

#define Parser_allocNode(p) Parser_allocNode0(p, __func__, __LINE__)
static Node* Parser_allocNode0(Parser *p, const char *function, int line)
{
    (void)p;
    (void)function;
    (void)line;

#ifdef TRACE
    std_printf("Node - %s:%d\n", function, line);
#endif

    p->nodes_count++;
    Node *n = std_malloc(sizeof(Node));
    if (!n) std_panic("oom\n");
    return n;
}

static Node* Parser_parseExpr(Parser *p);

// ExprList <- (Expr COMMA)* Expr?
static Node** Parser_parseExprList(Parser *p, uint32_t *exprs_len)
{
    Parser_trace(p);
    NodeArray exprs;
    NodeArray_init(&exprs);

    int c = 0;
    while (c++ < LOOP_MAX) {
        Node *expr = Parser_parseExpr(p);
        if (!expr) break;
        NodeArray_append(&exprs, expr);
        if (!Parser_eat(p, token_comma)) break;
    }
    if (c >= LOOP_MAX) Parser_fail(p, "infinite loop detected");

    *exprs_len = exprs.len;
    return exprs.data;
}

static Node* Parser_parseParamDecl(Parser *p);
// ParamDeclList <- (ParamDecl COMMA)* ParamDecl?
static Node* Parser_parseParamDeclList(Parser *p)
{
    Parser_trace(p);

    NodeArray a;
    NodeArray_init(&a);

    int c = 0;
    while (c++ < LOOP_MAX && !Parser_peek(p, token_r_paren)) {
        Node *param = Parser_parseParamDecl(p);
        if (!param) return NULL;
        NodeArray_append(&a, param);
        Parser_eat(p, token_comma);
    }
    if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");

    Node *n = Parser_allocNode(p);
    n->tag = node_param_decl_list;
    n->data.param_decl_list = (NodeDataParamDeclList){
        .params = a.data,
        .params_len = a.len
    };
    return n;
}

static Node* Parser_parseAsmInputItem(Parser *p);
// AsmInputList <- (AsmInputItem COMMA)* AsmInputItem?
static Node* Parser_parseAsmInputList(Parser *p)
{
    NodeArray a;
    NodeArray_init(&a);

    int c = 0;
    while (c++ < LOOP_MAX) {
        Node *item = Parser_parseAsmInputItem(p);
        if (!item) break;
        NodeArray_append(&a, item);
        if (!Parser_eat(p, token_comma)) break;
    }
    if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");

    Node *n = Parser_allocNode(p);
    n->tag = node_asm_input_list;
    n->data.asm_input_list = (NodeDataAsmInputList){
        .asm_inputs = a.data,
        .asm_inputs_len = a.len,
    };
    return n;
}

static Node* Parser_parseAsmOutputItem(Parser *p);
// AsmOutputList <- (AsmOutputItem COMMA)* AsmOutputItem?
static Node* Parser_parseAsmOutputList(Parser *p)
{
    NodeArray a;
    NodeArray_init(&a);

    int c = 0;
    while (c++ < LOOP_MAX) {
        Node *item = Parser_parseAsmOutputItem(p);
        if (!item) break;
        NodeArray_append(&a, item);
        if (!Parser_eat(p, token_comma)) break;
    }
    if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");

    Node *n = Parser_allocNode(p);
    n->tag = node_asm_output_list;
    n->data.asm_output_list = (NodeDataAsmOutputList){
        .asm_outputs = a.data,
        .asm_outputs_len = a.len,
    };
    return n;
}

static Node* Parser_parseSwitchProng(Parser *p);
// SwitchProngList <- (SwitchProng COMMA)* SwitchProng?
static Node* Parser_parseSwitchProngList(Parser *p)
{
    Parser_trace(p);
    NodeArray a;
    NodeArray_init(&a);

    int c = 0;
    while (c++ < LOOP_MAX) {
        Node *prong = Parser_parseSwitchProng(p);
        if (!prong) break;
        NodeArray_append(&a, prong);
        if (!Parser_eat(p, token_comma)) break;
    }
    if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");

    Node *n = Parser_allocNode(p);
    n->tag = node_switch_prong_list;
    n->data.switch_prong_list = (NodeDataSwitchProngList){
        .prongs = a.data,
        .prongs_len = a.len,
    };
    return n;
}

// IdentifierList <- (doc_comment? IDENTIFIER COMMA)* (doc_comment? IDENTIFIER)?
static Node* Parser_parseIdentifierList(Parser *p)
{
    Parser_trace(p);

    BufferArray a;
    BufferArray_init(&a);

    int c = 0;
    while (c++ < LOOP_MAX) {
        while (Parser_eat(p, token_doc_comment)) {}
        Buffer ident = Parser_eatIdentifier(p);
        if (ident.len == 0) break;
        BufferArray_append(&a, ident);
        if (!Parser_eat(p, token_comma)) break;
    }
    if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");

    Node *n = Parser_allocNode(p);
    n->tag = node_identifier_list;
    n->data.identifier_list = (NodeDataIdentifierList){
        .idents = a.data,
        .idents_len = a.len,
    };
    return n;
}

// ByteAlign <- KEYWORD_align LPAREN Expr RPAREN
static Node* Parser_parseByteAlign(Parser *p)
{
    Parser_trace(p);
    if (!Parser_eat(p, token_keyword_align)) return NULL;
    Parser_expect(p, token_l_paren);
    Node *n = Parser_parseExpr(p);
    Parser_expect(p, token_r_paren);
    return n;
}

// ContainerDeclType
//     <- KEYWORD_struct (LPAREN Expr RPAREN)?
//      / KEYWORD_opaque
//      / KEYWORD_enum (LPAREN Expr RPAREN)?
//      / KEYWORD_union (LPAREN (KEYWORD_enum (LPAREN Expr RPAREN)? / Expr) RPAREN)?
static Node* Parser_parseContainerDeclType(Parser *p)
{
    Parser_trace(p);
    if (Parser_eat(p, token_keyword_struct)) {
        Node *expr = NULL;
        if (Parser_eat(p, token_l_paren)) {
            expr = Parser_parseExpr(p);
            Parser_expect(p, token_r_paren);
        }
        Node *n = Parser_allocNode(p);
        n->tag = node_struct_decl;
        n->data.struct_decl = expr;
        return n;
    } else if (Parser_eat(p, token_keyword_opaque)) {
        Node *n = Parser_allocNode(p);
        n->tag = node_opaque_decl;
        return n;
    } else if (Parser_eat(p, token_keyword_enum)) {
        Node *expr = NULL;
        if (Parser_eat(p, token_l_paren)) {
            expr = Parser_parseExpr(p);
            Parser_expect(p, token_r_paren);
        }
        Node *n = Parser_allocNode(p);
        n->tag = node_enum_decl;
        n->data.enum_decl = expr;
        return n;
    } else if (Parser_eat(p, token_keyword_union)) {
        Node *expr = NULL;
        bool is_tagged = false;
        if (Parser_eat(p, token_l_paren)) {
            if (Parser_eat(p, token_keyword_enum)) {
                if (Parser_eat(p, token_l_paren)) {
                    is_tagged = true;
                    expr = Parser_parseExpr(p);
                    Parser_expect(p, token_r_paren);
                }
            } else {
                expr = Parser_parseExpr(p);
            }
            Parser_expect(p, token_r_paren);
        }
        Node *n = Parser_allocNode(p);
        n->tag = node_union_decl;
        n->data.union_decl = (NodeDataUnionDecl){
            .is_tagged = is_tagged,
            .expr = expr,
        };
        return n;
    }

    return NULL;
}

static Node* Parser_expectContainerMembers(Parser *p);
// ContainerDeclAuto <- ContainerDeclType LBRACE ContainerMembers RBRACE
static Node* Parser_parseContainerDeclAuto(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    Node *type = Parser_parseContainerDeclType(p);
    if (!type) goto fail;
    if (!Parser_eat(p, token_l_brace)) goto fail;
    Node *members = Parser_expectContainerMembers(p);
    if (!Parser_eat(p, token_r_brace)) goto fail;

    Node *n = Parser_allocNode(p);
    n->tag = node_container_decl_auto;
    n->data.container_decl_auto = (NodeDataContainerDeclAuto){
        .type = type,
        .members = members,
    };
    return n;

fail:
    p->index = index;
    return NULL;
}

// ArrayTypeStart <- LBRACKET Expr (COLON Expr)? RBRACKET
static Node* Parser_parseArrayTypeStart(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    if (!Parser_eat(p, token_l_bracket)) goto fail;
    Node *index_expr = Parser_parseExpr(p);
    if (!index) goto fail;
    Node *sentinel_expr = NULL;
    if (Parser_eat(p, token_colon)) {
        sentinel_expr = Parser_parseExpr(p);
        if (!sentinel_expr) goto fail;
    }
    if (!Parser_eat(p, token_r_bracket)) goto fail;

    Node *n = Parser_allocNode(p);
    n->tag = node_array_type_start;
    n->data.array_type_start = (NodeDataArrayTypeStart){
        .index = index_expr,
        .sentinel_expr = sentinel_expr,
    };
    return n;

fail:
    p->index = index;
    return NULL;
}

// PtrTypeStart
//     <- ASTERISK
//      / ASTERISK2
//      / LBRACKET ASTERISK (LETTERC / COLON Expr)? RBRACKET
static Node* Parser_parsePtrTypeStart(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    if (Parser_eat(p, token_asterisk)) {
        Node *n = Parser_allocNode(p);
        n->tag = node_ptr_type_start;
        n->data.ptr_type_start = (NodeDataPtrTypeStart){
            .type = node_ptr_type_single,
        };
        return n;
    } else if (Parser_eat(p, token_asterisk_asterisk)) {
        Node *n = Parser_allocNode(p);
        n->tag = node_ptr_type_start;
        n->data.ptr_type_start = (NodeDataPtrTypeStart){
            .type = node_ptr_type_double,
        };
        return n;
    }

    if (!Parser_eat(p, token_l_bracket)) goto fail;
    if (!Parser_eat(p, token_asterisk)) goto fail;

    NodePtrType type = node_ptr_type_multi;
    Node *sentinel_expr = NULL;

    if (Parser_eat(p, token_colon)) {
        type = node_ptr_type_sentinel;
        sentinel_expr = Parser_parseExpr(p);
        if (!sentinel_expr) Parser_fail(p, "expected expression for sentinel");
    } else {
        if (!Buffer_eql(Parser_tokenSlice(p), "c")) {
            p->index++;
            type = node_ptr_type_c;
        }
    }
    if (!Parser_eat(p, token_r_bracket)) goto fail;

    Node *n = Parser_allocNode(p);
    n->tag = node_ptr_type_start;
    n->data.ptr_type_start = (NodeDataPtrTypeStart){
        .type = type,
        .sentinel_expr = sentinel_expr,
    };
    return n;

fail:
    p->index = index;
    return NULL;
}

// SliceTypeStart <- LBRACKET (COLON Expr)? RBRACKET
static Node* Parser_parseSliceTypeStart(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    if (!Parser_eat(p, token_l_bracket)) goto fail;
    Node *sentinel_expr = NULL;
    if (Parser_eat(p, token_colon)) {
        sentinel_expr = Parser_parseExpr(p);
    }
    if (!Parser_eat(p, token_r_bracket)) goto fail;

    Node *n = Parser_allocNode(p);
    n->tag = node_slice_type_start;
    n->data.slice_type_start = (NodeDataSliceTypeStart){
        .sentinel_expr = sentinel_expr,
    };
    return n;

fail:
    p->index = index;
    return NULL;
}

// FnCallArguments <- LPAREN ExprList RPAREN
static Node* Parser_parseFnCallArguments(Parser *p)
{
    Parser_trace(p);
    if (!Parser_eat(p, token_l_paren)) return NULL;

    uint32_t exprs_len = 0;
    Node **expr_list = NULL;
    if (!Parser_eat(p, token_r_paren)) {
        exprs_len = 0;
        expr_list = Parser_parseExprList(p, &exprs_len);
        Parser_expect(p, token_r_paren);
    }

    Node *n = Parser_allocNode(p);
    n->tag = node_fn_call_arguments;
    n->data.fn_call_arguments = (NodeDataFnCallArguments){
        .exprs = expr_list,
        .exprs_len = exprs_len,
    };
    return n;
}

// SuffixOp
//     <- LBRACKET Expr (DOT2 (Expr? (COLON Expr)?)?)? RBRACKET
//      / DOT IDENTIFIER
//      / DOTASTERISK
//      / DOTQUESTIONMARK
static Node* Parser_parseSuffixOp(Parser *p)
{
    Parser_trace(p);
    if (Parser_eat(p, token_l_bracket)) {
        Node *range_start = Parser_parseExpr(p);
        Node *range_end = NULL;
        Node *sentinel = NULL;
        if (Parser_eat(p, token_ellipsis2)) {
            range_end = Parser_parseExpr(p);
            if (Parser_eat(p, token_colon)) {
                sentinel = Parser_parseExpr(p);
            }
        }
        Parser_expect(p, token_r_bracket);

        Node *n = Parser_allocNode(p);
        n->tag = node_suffix_type_op_slice;
        n->data.suffix_type_op_slice = (NodeDataSuffixTypeOpSlice){
            .start_expr = range_start,
            .end_expr = range_end,
            .sentinel_expr = sentinel,
        };
        return n;
    } else if (Parser_eat(p, token_period)) {
        // NOTE: grammar not updated but `.?` is not a singular token.
        if (Parser_eat(p, token_question_mark)) {
            Node *n = Parser_allocNode(p);
            n->tag = node_suffix_type_op_assert_maybe;
            return n;
        }

        Buffer name = Parser_expectIdentifier(p);
        Node *n = Parser_allocNode(p);
        n->tag = node_suffix_type_op_named_access;
        n->data.suffix_type_op_named_access = (NodeDataSuffixTypeOpNamedAccess){
            .name = name,
        };
        return n;
    } else if (Parser_eat(p, token_period_asterisk)) {
        Node *n = Parser_allocNode(p);
        n->tag = node_suffix_type_op_deref;
        return n;
    }

    return NULL;
}

// Align <- KEYWORD_align LPAREN Expr (COLON Expr COLON Expr)? RPAREN
static Node* Parser_parseAlign(Parser *p)
{
    Parser_trace(p);
    if (!Parser_eat(p, token_keyword_align)) return NULL;
    Parser_expect(p, token_l_paren);
    Node *byte_align = Parser_parseExpr(p);
    Node *bit_offset = NULL;
    Node *bit_backing_integer_size = NULL;
    if (Parser_eat(p, token_colon)) {
        bit_offset = Parser_parseExpr(p);
        if (!bit_offset) Parser_fail(p, "expected expression for bit_offset");
        Parser_expect(p, token_colon);
        bit_backing_integer_size = Parser_parseExpr(p);
        if (!bit_backing_integer_size) Parser_fail(p, "expected expression for bit_backing_integer_size");
    }
    Parser_expect(p, token_r_paren);

    Node *n = Parser_allocNode(p);
    n->tag = node_ptr_align_expr;
    n->data.ptr_align_expr = (NodeDataPtrAlignExpr){
        .byte_align = byte_align,
        .bit_offset = bit_offset,
        .bit_backing_integer_size = bit_backing_integer_size,
    };
    return n;
}

static Node* Parser_parseAddrSpace(Parser *p);
// PrefixTypeOp
//     <- QUESTIONMARK
//      / KEYWORD_anyframe MINUSRARROW
//      / SliceTypeStart (ByteAlign / AddrSpace / KEYWORD_const / KEYWORD_volatile / KEYWORD_allowzero)*
//      / PtrTypeStart (AddrSpace / Align / KEYWORD_const / KEYWORD_volatile / KEYWORD_allowzero)*
//      / ArrayTypeStart
static Node* Parser_parsePrefixTypeOp(Parser *p)
{
    Parser_trace(p);
    if (Parser_eat(p, token_question_mark)) {
        Node *n = Parser_allocNode(p);
        n->tag = node_prefix_type_op_optional;
        return n;
    } else if (Parser_eat(p, token_keyword_anyframe)) {
        Node *n = Parser_allocNode(p);
        n->tag = node_prefix_type_op_anyframe;
        return n;
    }

    Node *slice_type_start = Parser_parseSliceTypeStart(p);
    if (slice_type_start) {
        Node *bytealign = NULL;
        Node *addrspace = NULL;
        PointerModifiers modifiers = 0;

        int c = 0;
        while (c++ < LOOP_MAX) {
            bytealign = Parser_parseByteAlign(p);
            if (bytealign) {
                continue;
            }

            addrspace = Parser_parseAddrSpace(p);
            if (addrspace) {
                continue;
            }

            if (Parser_eat(p, token_keyword_const)) {
                modifiers |= pointer_modifier_const;
                continue;
            }

            if (Parser_eat(p, token_keyword_volatile)) {
                modifiers |= pointer_modifier_volatile;
                continue;
            }

            if (Parser_eat(p, token_keyword_allowzero)) {
                modifiers |= pointer_modifier_allowzero;
                continue;
            }

            break;
        }
        if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");

        Node *n = Parser_allocNode(p);
        n->tag = node_prefix_type_op_slice;
        n->data.prefix_type_slice = (NodeDataPrefixTypeSlice){
            .slice = slice_type_start,
            .bytealign = bytealign,
            .addrspace = addrspace,
            .modifiers = modifiers,
        };
        return n;
    }

    Node *ptr_type_start = Parser_parsePtrTypeStart(p);
    if (ptr_type_start) {
        Node *addrspace = NULL;
        Node *align = NULL;
        PointerModifiers modifiers = 0;

        int c = 0;
        while (c++ < LOOP_MAX) {
            addrspace = Parser_parseAddrSpace(p);
            if (addrspace) {
                continue;
            }

            align = Parser_parseAlign(p);
            if (align) {
                continue;
            }

            if (Parser_eat(p, token_keyword_const)) {
                modifiers |= pointer_modifier_const;
                continue;
            }

            if (Parser_eat(p, token_keyword_volatile)) {
                modifiers |= pointer_modifier_volatile;
                continue;
            }

            if (Parser_eat(p, token_keyword_allowzero)) {
                modifiers |= pointer_modifier_allowzero;
                continue;
            }

            break;
        }
        if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");

        Node *n = Parser_allocNode(p);
        n->tag = node_prefix_type_op_ptr;
        n->data.prefix_type_ptr = (NodeDataPrefixTypePtr){
            .ptr = ptr_type_start,
            .align = align,
            .addrspace = addrspace,
            .modifiers = modifiers,
        };
        return n;
    }

    Node *array = Parser_parseArrayTypeStart(p);
    if (array) {
        Node *n = Parser_allocNode(p);
        n->tag = node_prefix_type_op_array;
        n->data.prefix_type_array = (NodeDataPrefixTypeArray){
            .array = array,
        };
        return n;
    }

    return NULL;
}

// PrefixOp
//     <- EXCLAMATIONMARK
//      / MINUS
//      / TILDE
//      / MINUSPERCENT
//      / AMPERSAND
//      / KEYWORD_try
static TokenTag Parser_eatPrefixOp(Parser *p)
{
    Parser_trace(p);
    switch (p->tokens[p->index].tag) {
        case token_bang:
        case token_minus:
        case token_tilde:
        case token_minus_percent:
        case token_ampersand:
        case token_keyword_try:
            return p->tokens[p->index++].tag;
        default:
            return token_invalid;
    }
}

// AssignOp
//     <- ASTERISKEQUAL
//      / ASTERISKPIPEEQUAL
//      / SLASHEQUAL
//      / PERCENTEQUAL
//      / PLUSEQUAL
//      / PLUSPIPEEQUAL
//      / MINUSEQUAL
//      / MINUSPIPEEQUAL
//      / LARROW2EQUAL
//      / LARROW2PIPEEQUAL
//      / RARROW2EQUAL
//      / AMPERSANDEQUAL
//      / CARETEQUAL
//      / PIPEEQUAL
//      / ASTERISKPERCENTEQUAL
//      / PLUSPERCENTEQUAL
//      / MINUSPERCENTEQUAL
//      / EQUAL
static TokenTag Parser_eatAssignOp(Parser *p)
{
    Parser_trace(p);
    switch (p->tokens[p->index].tag) {
        case token_asterisk_equal:
        case token_asterisk_pipe_equal:
        case token_slash_equal:
        case token_percent_equal:
        case token_plus_equal:
        case token_plus_pipe_equal:
        case token_minus_equal:
        case token_minus_pipe_equal:
        case token_angle_bracket_angle_bracket_left_equal:
        case token_angle_bracket_angle_bracket_left_pipe_equal:
        case token_angle_bracket_angle_bracket_right_equal:
        case token_ampersand_equal:
        case token_caret_equal:
        case token_pipe_equal:
        case token_asterisk_percent_equal:
        case token_plus_percent_equal:
        case token_minus_percent_equal:
        case token_equal:
            return p->tokens[p->index++].tag;
        default:
            return token_invalid;
    }
}

// ForItem <- Expr (DOT2 Expr?)?
static Node* Parser_parseForItem(Parser *p)
{
    Parser_trace(p);
    Node *for_start = Parser_parseExpr(p);
    if (!for_start) return NULL;

    Node *for_end = NULL;
    bool is_range = false;
    if (Parser_eat(p, token_ellipsis2)) {
        for_end = Parser_parseExpr(p);
        is_range = true;
    }

    Node *n = Parser_allocNode(p);
    n->tag = node_for_item;
    n->data.for_item = (NodeDataForItem){
        .for_start = for_start,
        .for_end = for_end,
        .is_range = is_range,
    };
    return n;
}

// ForArgumentsList <- ForItem (COMMA ForItem)* COMMA?
static Node* Parser_parseForArgumentsList(Parser *p)
{
    Parser_trace(p);
    NodeArray args;
    NodeArray_init(&args);

    int c = 0;
    while (c++ < LOOP_MAX) {
        Node *arg = Parser_parseForItem(p);
        if (!arg) break;
        Parser_eat(p, token_comma);
        NodeArray_append(&args, arg);
    }
    if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");

    Node *n = Parser_allocNode(p);
    n->tag = node_for_args;
    n->data.for_args = (NodeDataForArgs){
        .args = args.data,
        .args_len = args.len,
    };
    return n;
}

// SwitchItem <- Expr (DOT3 Expr)?
static Node* Parser_parseSwitchItem(Parser *p)
{
    Parser_trace(p);
    Node *start = Parser_parseExpr(p);
    if (!start) return NULL;
    Node *end = NULL;
    if (Parser_eat(p, token_ellipsis3)) {
        end = Parser_parseExpr(p);
        if (!end) Parser_fail(p, "expected expression after ...");
    }

    Node *n = Parser_allocNode(p);
    n->tag = node_switch_item;
    n->data.switch_item = (NodeDataSwitchItem){
        .start = start,
        .end = end,
    };
    return n;
}

// SwitchCase
//     <- SwitchItem (COMMA SwitchItem)* COMMA?
//      / KEYWORD_else
static Node* Parser_parseSwitchCase(Parser *p)
{
    Parser_trace(p);
    if (Parser_eat(p, token_keyword_else)) {
        Node *n = Parser_allocNode(p);
        n->tag = node_switch_case;
        n->data.switch_case = (NodeDataSwitchCase){
            .is_else = true,
        };
        return n;
    }

    NodeArray cases;
    NodeArray_init(&cases);

    int c = 0;
    while (c++ < LOOP_MAX && !Parser_peek(p, token_equal_angle_bracket_right)) {
        Node *item = Parser_parseSwitchItem(p);
        if (!item) break;
        Parser_eat(p, token_comma);
        NodeArray_append(&cases, item);
    }
    if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");

    Node *n = Parser_allocNode(p);
    n->tag = node_switch_case;
    n->data.switch_case = (NodeDataSwitchCase){
        .is_else = false,
        .cases = cases.data,
        .cases_len = cases.len,
    };
    return n;
}

static Node* Parser_parsePtrIndexPayload(Parser *p);
static Node* Parser_parseSingleAssignExpr(Parser *p);
// SwitchProng <- KEYWORD_inline? SwitchCase EQUALRARROW PtrIndexPayload? SingleAssignExpr
static Node* Parser_parseSwitchProng(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    bool is_inline = Parser_eat(p, token_keyword_inline);
    Node *sc = Parser_parseSwitchCase(p);
    if (!sc) goto fail;
    if (!Parser_eat(p, token_equal_angle_bracket_right)) goto fail;

    Node *payload = Parser_parsePtrIndexPayload(p);
    Node *expr = Parser_parseSingleAssignExpr(p);
    if (!expr) goto fail;

    Node *n = Parser_allocNode(p);
    n->tag = node_switch_prong;
    n->data.switch_prong = (NodeDataSwitchProng){
        .is_inline = is_inline,
        .switch_case = sc,
        .payload = payload,
        .expr = expr,
    };
    return n;

fail:
    p->index = index;
    return NULL;
}

// PtrListPayload <- PIPE ASTERISK? IDENTIFIER (COMMA ASTERISK? IDENTIFIER)* COMMA? PIPE
static Node* Parser_parsePtrListPayload(Parser *p)
{
    Parser_trace(p);
    NodeArray a;
    NodeArray_init(&a);

    Parser_expect(p, token_pipe);
    int c = 0;
    while (c++ < LOOP_MAX) {
        bool is_pointer = Parser_eat(p, token_asterisk);
        Buffer name = Parser_expectIdentifier(p);
        Node *n = Parser_allocNode(p);
        n->tag = node_payload;
        n->data.payload = (NodeDataPayload){
            .name = name,
            .is_pointer = is_pointer,
        };
        NodeArray_append(&a, n);
        if (!Parser_eat(p, token_comma)) break;
    }
    if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");
    Parser_eat(p, token_comma);
    Parser_expect(p, token_pipe);

    Node *n = Parser_allocNode(p);
    n->tag = node_payload_list;
    n->data.payload_list = (NodeDataPayloadList){
        .payloads = a.data,
        .payloads_len = a.len,
    };
    return n;
}

// PtrIndexPayload <- PIPE ASTERISK? IDENTIFIER (COMMA IDENTIFIER)? PIPE
static Node* Parser_parsePtrIndexPayload(Parser *p)
{
    Parser_trace(p);
    NodeArray a;
    NodeArray_init(&a);

    if (!Parser_eat(p, token_pipe)) return NULL;
    bool is_pointer = Parser_eat(p, token_asterisk);
    Buffer name = Parser_expectIdentifier(p);

    Buffer name_index = Buffer_empty();
    if (Parser_eat(p, token_comma)) {
        name_index = Parser_expectIdentifier(p);
    }
    Parser_expect(p, token_pipe);

    Node *n = Parser_allocNode(p);
    n->tag = node_payload;
    n->data.payload_index = (NodeDataPayloadIndex){
        .name = name,
        .is_pointer = is_pointer,
        .name_index = name_index,
    };
    return n;
}

// PtrPayload <- PIPE ASTERISK? IDENTIFIER PIPE
static Node* Parser_parsePtrPayload(Parser *p)
{
    Parser_trace(p);
    if (!Parser_eat(p, token_pipe)) return NULL;
    bool is_pointer = Parser_eat(p, token_asterisk);
    Buffer name = Parser_expectIdentifier(p);
    Parser_expect(p, token_pipe);

    Node *n = Parser_allocNode(p);
    n->tag = node_payload;
    n->data.payload = (NodeDataPayload){
        .name = name,
        .is_pointer = is_pointer,
    };
    return n;
}

// Payload <- PIPE IDENTIFIER PIPE
static Buffer Parser_parsePayload(Parser *p)
{
    Parser_trace(p);
    if (!Parser_eat(p, token_pipe)) return Buffer_empty();
    Buffer name = Parser_expectIdentifier(p);
    Parser_expect(p, token_pipe);
    return name;
}

// ForPrefix <- KEYWORD_for LPAREN ForArgumentsList RPAREN PtrListPayload
static Node* Parser_parseForPrefix(Parser *p)
{
    Parser_trace(p);
    if (!Parser_eat(p, token_keyword_for)) return NULL;
    Parser_expect(p, token_l_paren);
    Node *for_args = Parser_parseForArgumentsList(p);
    Parser_expect(p, token_r_paren);
    Node *ptr_list_payload = Parser_parsePtrListPayload(p);

    Node *n = Parser_allocNode(p);
    n->tag = node_for_prefix;
    n->data.for_prefix = (NodeDataForPrefix){
        .for_args = for_args,
        .ptr_list_payload = ptr_list_payload,
    };
    return n;
}

static Node* Parser_parseWhileContinueExpr(Parser *p);
// WhilePrefix <- KEYWORD_while LPAREN Expr RPAREN PtrPayload? WhileContinueExpr?
static Node* Parser_parseWhilePrefix(Parser *p)
{
    Parser_trace(p);
    if (!Parser_eat(p, token_keyword_while)) return NULL;
    Parser_expect(p, token_l_paren);
    Node *condition = Parser_parseExpr(p);
    Parser_expect(p, token_r_paren);
    Node *ptr_payload = Parser_parsePtrPayload(p);
    Node *while_continue_expr = Parser_parseWhileContinueExpr(p);

    Node *n = Parser_allocNode(p);
    n->tag = node_while_prefix;
    n->data.while_prefix = (NodeDataWhilePrefix){
        .condition = condition,
        .ptr_payload = ptr_payload,
        .while_continue_expr = while_continue_expr,
    };
    return n;
}

// IfPrefix <- KEYWORD_if LPAREN Expr RPAREN PtrPayload?
static Node* Parser_parseIfPrefix(Parser *p)
{
    Parser_trace(p);
    if (!Parser_eat(p, token_keyword_if)) return NULL;
    Parser_expect(p, token_l_paren);
    Node *condition = Parser_parseExpr(p);
    Parser_expect(p, token_r_paren);
    Node *ptr_payload = Parser_parsePtrPayload(p);

    Node *n = Parser_allocNode(p);
    n->tag = node_if_prefix;
    n->data.if_prefix = (NodeDataIfPrefix){
        .condition = condition,
        .ptr_payload = ptr_payload,
    };
    return n;
}

static Node* Parser_parseTypeExpr(Parser *p);
// ParamType
//     <- KEYWORD_anytype
//      / TypeExpr
static Node* Parser_parseParamType(Parser *p)
{
    Parser_trace(p);
    if (Parser_eat(p, token_keyword_anytype)) {
        Node *n = Parser_allocNode(p);
        n->tag = node_primary_type_expr;
        n->data.primary_type_expr.tag = node_primary_type_anytype;
        return n;
    }
    return Parser_parseTypeExpr(p);
}

// ParamDecl
//     <- doc_comment? (KEYWORD_noalias / KEYWORD_comptime)? (IDENTIFIER COLON)? ParamType
//      / DOT3
static Node* Parser_parseParamDecl(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    if (Parser_eat(p, token_ellipsis3)) {
        Node *n = Parser_allocNode(p);
        n->tag = node_param_decl;
        n->data.param_decl = (NodeDataParamDecl){
            .is_varargs = true,
        };
        return n;
    }

    while (Parser_eat(p, token_doc_comment)) {}
    TokenTag modifier = Parser_eatOneOf(p, (TokenTag[]){ token_keyword_noalias, token_keyword_comptime }, 2);
    // This may actually be the ParamType. Reset if no colon follows.
    Buffer identifier = Parser_eatIdentifier(p);
    if (identifier.len != 0 && !Parser_eat(p, token_colon)) {
        identifier = Buffer_empty();
        p->index--;
    }
    Node *type = Parser_parseParamType(p);
    if (!type) goto fail;

    Node *n = Parser_allocNode(p);
    n->tag = node_param_decl;
    n->data.param_decl = (NodeDataParamDecl){
        .is_varargs = false,
        .modifier = modifier,
        .identifier = identifier,
        .type = type,
    };
    return n;

fail:
    p->index = index;
    return NULL;
}

// CallConv <- KEYWORD_callconv LPAREN Expr RPAREN
static Node* Parser_parseCallConv(Parser *p)
{
    Parser_trace(p);
    if (!Parser_eat(p, token_keyword_callconv)) return NULL;
    Parser_expect(p, token_l_paren);
    Node *n = Parser_parseExpr(p);
    Parser_expect(p, token_r_paren);
    return n;
}

// AddrSpace <- KEYWORD_addrspace LPAREN Expr RPAREN
static Node* Parser_parseAddrSpace(Parser *p)
{
    Parser_trace(p);
    if (!Parser_eat(p, token_keyword_addrspace)) return NULL;
    Parser_expect(p, token_l_paren);
    Node *n = Parser_parseExpr(p);
    Parser_expect(p, token_r_paren);
    return n;
}

// LinkSection <- KEYWORD_linksection LPAREN Expr RPAREN
static Node* Parser_parseLinkSection(Parser *p)
{
    Parser_trace(p);
    if (!Parser_eat(p, token_keyword_linksection)) return NULL;
    Parser_expect(p, token_l_paren);
    Node *n = Parser_parseExpr(p);
    Parser_expect(p, token_r_paren);
    return n;
}

static Node* Parser_parseAssignExpr(Parser *p);
// WhileContinueExpr <- COLON LPAREN AssignExpr RPAREN
static Node* Parser_parseWhileContinueExpr(Parser *p)
{
    Parser_trace(p);
    if (!Parser_eat(p, token_colon)) return NULL;
    Parser_expect(p, token_l_paren);
    Node *expr = Parser_parseAssignExpr(p);
    Parser_expect(p, token_r_paren);
    return expr;
}

// FieldInit <- DOT IDENTIFIER EQUAL Expr
static Node* Parser_parseFieldInit(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    if (!Parser_eat(p, token_period)) return NULL;
    Buffer name = Parser_eatIdentifier(p);
    if (name.len == 0) goto fail;
    if (!Parser_eat(p, token_equal)) goto fail;
    Node *expr = Parser_parseExpr(p);
    if (!expr) Parser_fail(p, "expected expression");

    Node *n = Parser_allocNode(p);
    n->tag = node_field_init;
    n->data.field_init = (NodeDataFieldInit){
        .name = name,
        .expr = expr,
    };
    return n;

fail:
    p->index = index;
    return NULL;
}

// BlockLabel <- IDENTIFIER COLON
static Buffer Parser_parseBlockLabel(Parser *p)
{
    Parser_trace(p);
    if (p->tokens[p->index].tag == token_identifier && p->tokens[p->index + 1].tag == token_colon) {
        Buffer name = Parser_tokenSlice(p);
        p->index += 2;
        return name;
    }

    return Buffer_empty();
}

// BreakLabel <- COLON IDENTIFIER
static Buffer Parser_parseBreakLabel(Parser *p)
{
    Parser_trace(p);
    if (p->tokens[p->index].tag == token_colon && p->tokens[p->index + 1].tag == token_identifier) {
        p->index++;
        Buffer name = Parser_tokenSlice(p);
        p->index++;
        return name;
    }

    return Buffer_empty();
}

static Node* Parser_parseExpr(Parser *p);
// AsmClobbers <- COLON Expr
static Node* Parser_parseAsmClobbers(Parser *p)
{
    Parser_trace(p);
    if (!Parser_eat(p, token_colon)) return NULL;
    Node *n = Parser_parseExpr(p);
    if (!n) Parser_fail(p, "expected expression");
    return n;
}

// AsmInputItem <- LBRACKET IDENTIFIER RBRACKET STRINGLITERAL LPAREN Expr RPAREN
static Node* Parser_parseAsmInputItem(Parser *p)
{
    if (!Parser_eat(p, token_l_bracket)) return NULL;
    Buffer name = Parser_expectIdentifier(p);
    Parser_expect(p, token_r_bracket);
    Buffer lit = Parser_tokenSlice(p);
    if (!Parser_eat(p, token_string_literal)) Parser_fail(p, "expected string literal");
    Parser_expect(p, token_l_paren);
    Node *expr = Parser_parseExpr(p);
    Parser_expect(p, token_r_paren);

    Node *n = Parser_allocNode(p);
    n->tag = node_asm_input_item;
    n->data.asm_input_item = (NodeDataAsmInputItem){
        .name = name,
        .lit = lit,
        .input_expr = expr,
    };
    return n;
}

// AsmInput <- COLON AsmInputList AsmClobbers?
static Node* Parser_parseAsmInput(Parser *p)
{
    if (!Parser_eat(p, token_colon)) return NULL;
    Node *asm_input_list = Parser_parseAsmInputList(p);
    if (!asm_input_list) Parser_fail(p, "expected asm input list");
    Node *clobbers = Parser_parseAsmClobbers(p);

    Node *n = Parser_allocNode(p);
    n->tag = node_asm_input;
    n->data.asm_input = (NodeDataAsmInput){
        .asm_input_list = asm_input_list,
        .clobbers = clobbers,
    };
    return n;
}

// AsmOutputItem <- LBRACKET IDENTIFIER RBRACKET STRINGLITERAL LPAREN (MINUSRARROW TypeExpr / IDENTIFIER) RPAREN
static Node* Parser_parseAsmOutputItem(Parser *p)
{
    if (!Parser_eat(p, token_l_bracket)) return NULL;
    Buffer name = Parser_expectIdentifier(p);
    Parser_expect(p, token_r_bracket);
    Buffer lit = Parser_tokenSlice(p);
    if (!Parser_eat(p, token_string_literal)) Parser_fail(p, "expected string literal");
    Parser_expect(p, token_l_paren);

    Node *output_expr = Parser_allocNode(p);
    output_expr->tag = node_type_or_name;
    if (Parser_eat(p, token_arrow)) {
        Node *expr = Parser_parseTypeExpr(p);
        if (!expr) Parser_fail(p, "expected type expression");
        output_expr->data.type_or_name.is_type = true;
        output_expr->data.type_or_name.type = expr;
    } else {
        output_expr->data.type_or_name.is_type = false;
        output_expr->data.type_or_name.name = Parser_expectIdentifier(p);
    }
    Parser_expect(p, token_r_paren);

    Node *n = Parser_allocNode(p);
    n->tag = node_asm_output_item;
    n->data.asm_output_item = (NodeDataAsmOutputItem){
        .name = name,
        .lit = lit,
        .output_expr = output_expr,
    };
    return n;
}

// AsmOutput <- COLON AsmOutputList AsmInput?
static Node* Parser_parseAsmOutput(Parser *p)
{
    if (!Parser_eat(p, token_colon)) return NULL;
    Node *asm_output_list = Parser_parseAsmOutputList(p);
    if (!asm_output_list) Parser_fail(p, "expected asm output list");
    Node *asm_input = Parser_parseAsmInput(p);

    Node *n = Parser_allocNode(p);
    n->tag = node_asm_output;
    n->data.asm_output = (NodeDataAsmOutput){
        .asm_output_list = asm_output_list,
        .asm_input = asm_input,
    };
    return n;
}

// AsmExpr <- KEYWORD_asm KEYWORD_volatile? LPAREN Expr AsmOutput? RPAREN
static Node* Parser_parseAsmExpr(Parser *p)
{
    if (!Parser_eat(p, token_keyword_asm)) return NULL;
    bool is_volatile = Parser_eat(p, token_keyword_volatile);
    Parser_expect(p, token_l_paren);
    Node *expr = Parser_parseExpr(p);
    if (!expr) Parser_fail(p, "expected expression");
    Node *asm_output = Parser_parseAsmOutput(p);
    Parser_expect(p, token_r_paren);

    Node *n = Parser_allocNode(p);
    n->tag = node_asm_expr;
    n->data.asm_expr = (NodeDataAsmExpr){
        .is_volatile = is_volatile,
        .expr = expr,
        .asm_output = asm_output,
    };
    return n;
}

// SwitchExpr <- KEYWORD_switch LPAREN Expr RPAREN LBRACE SwitchProngList RBRACE
static Node* Parser_parseSwitchExpr(Parser *p)
{
    if (!Parser_eat(p, token_keyword_switch)) return NULL;
    Parser_expect(p, token_l_paren);
    Node *expr = Parser_parseExpr(p);
    Parser_expect(p, token_r_paren);
    Parser_expect(p, token_l_brace);
    Node *switch_prong_list = Parser_parseSwitchProngList(p);
    Parser_expect(p, token_r_brace);

    Node *n = Parser_allocNode(p);
    n->tag = node_switch_expr;
    n->data.switch_expr = (NodeDataSwitchExpr){
        .expr = expr,
        .switch_prong_list = switch_prong_list,
    };
    return n;
}

// WhileTypeExpr <- WhilePrefix TypeExpr (KEYWORD_else Payload? TypeExpr)?
static Node* Parser_parseWhileTypeExpr(Parser *p)
{
    uint32_t index = p->index;

    Node *while_prefix = Parser_parseWhilePrefix(p);
    if (!while_prefix) goto fail;

    Node *type_expr = Parser_parseTypeExpr(p);
    if (!type_expr) goto fail;

    Buffer else_payload_name = Buffer_empty();
    Node *else_payload_type_expr = NULL;
    if (Parser_eat(p, token_keyword_else)) {
        else_payload_name = Parser_parsePayload(p);
        else_payload_type_expr = Parser_parseTypeExpr(p);
    }

    Node *n = Parser_allocNode(p);
    n->tag = node_while_type_expr;
    n->data.while_type_expr = (NodeDataWhileTypeExpr){
        .while_prefix = while_prefix,
        .type_expr = type_expr,
        .else_payload_name = else_payload_name,
        .else_payload_type_expr = else_payload_type_expr,
    };
    return n;

fail:
    p->index = index;
    return NULL;
}

// ForTypeExpr <- ForPrefix TypeExpr (KEYWORD_else TypeExpr)?
static Node* Parser_parseForTypeExpr(Parser *p)
{
    Parser_trace(p);
    Node *for_prefix = Parser_parseForPrefix(p);
    if (!for_prefix) return NULL;

    Node *expr = Parser_parseTypeExpr(p);
    Node *else_expr = NULL;
    if (Parser_eat(p, token_keyword_else)) {
        else_expr = Parser_parseTypeExpr(p);
    }

    Node *n = Parser_allocNode(p);
    n->tag = node_for_type_expr;
    n->data.for_type_expr = (NodeDataForTypeExpr){
        .for_prefix = for_prefix,
        .expr = expr,
        .else_expr = else_expr,
    };
    return n;
}

// LoopTypeExpr <- KEYWORD_inline? (ForTypeExpr / WhileTypeExpr)
static Node* Parser_parseLoopTypeExpr(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    bool is_inline = Parser_eat(p, token_keyword_inline);
    (void)is_inline;
    if (Parser_peek(p, token_keyword_for)) {
        return Parser_parseForTypeExpr(p);  // TODO: inline
    } else if (Parser_peek(p, token_keyword_while)) {
        return Parser_parseWhileTypeExpr(p); // TODO: inline
    }

    p->index = index;
    return NULL;
}

static Node* Parser_parseBlock(Parser *p);
static Node* Parser_parseSwitchExpr(Parser *p);
// LabeledTypeExpr
//     <- BlockLabel Block
//      / BlockLabel? LoopTypeExpr
//      / BlockLabel? SwitchExpr
static Node* Parser_parseLabeledTypeExpr(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    Buffer label = Parser_parseBlockLabel(p);

    if (Parser_peek(p, token_l_brace)) {
        Node *block = Parser_parseBlock(p);
        if (!block) goto fail;
        Node *n = Parser_allocNode(p);
        n->tag = node_labeled_block;
        n->data.labeled_type_expr = (NodeDataLabeledTypeExpr){
            .label = label,
            .node = block,
        };
        return n;
    } else if (Parser_peek(p, token_keyword_inline)
        || Parser_peek(p, token_keyword_for)
        || Parser_peek(p, token_keyword_while)) {
        Node *loop_type_expr = Parser_parseLoopTypeExpr(p);
        if (loop_type_expr) goto fail;

        Node *n = Parser_allocNode(p);
        n->tag = node_labeled_loop_expr;
        n->data.labeled_type_expr = (NodeDataLabeledTypeExpr){
            .label = label,
            .node = loop_type_expr,
        };
        return n;
    } else if (Parser_peek(p, token_keyword_switch)) {
        Node *switch_expr = Parser_parseSwitchExpr(p);
        if (!switch_expr) goto fail;

        Node *n = Parser_allocNode(p);
        n->tag = node_labeled_switch_expr;
        n->data.labeled_type_expr = (NodeDataLabeledTypeExpr){
            .label = label,
            .node = switch_expr,
        };
        return n;
    }

fail:
    p->index = index;
    return NULL;
}

// IfTypeExpr <- IfPrefix TypeExpr (KEYWORD_else Payload? TypeExpr)?
static Node* Parser_parseIfTypeExpr(Parser *p)
{
    uint32_t index = p->index;

    Node *if_prefix = Parser_parseIfPrefix(p);
    if (!if_prefix) goto fail;

    Node *type_expr = Parser_parseTypeExpr(p);
    if (!type_expr) goto fail;

    Buffer else_payload_name = Buffer_empty();
    Node *else_payload_type_expr = NULL;
    if (Parser_eat(p, token_keyword_else)) {
        else_payload_name = Parser_parsePayload(p);
        else_payload_type_expr = Parser_parseTypeExpr(p);
    }

    Node *n = Parser_allocNode(p);
    n->tag = node_if_type_expr;
    n->data.if_type_expr = (NodeDataIfTypeExpr){
        .if_prefix = if_prefix,
        .type_expr = type_expr,
        .else_payload_name = else_payload_name,
        .else_payload_type_expr = else_payload_type_expr,
    };
    return n;;

fail:
    p->index = index;
    return NULL;
}

static Node* Parser_parseExpr(Parser *p);
// GroupedExpr <- LPAREN Expr RPAREN
static Node* Parser_parseGroupedExpr(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    if (!Parser_eat(p, token_l_paren)) goto fail;
    Node *n = Parser_parseExpr(p);
    if (!n) return NULL;
    Parser_expect(p, token_r_paren);
    return n;

fail:
    p->index = index;
    return NULL;
}

// ErrorSetDecl <- KEYWORD_error LBRACE IdentifierList RBRACE
static Node* Parser_parseErrorSetDecl(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    if (!Parser_eat(p, token_keyword_error)) goto fail;
    if (!Parser_eat(p, token_l_brace)) goto fail;
    Node *n = Parser_parseIdentifierList(p);
    if (!n) goto fail;
    Parser_expect(p, token_r_brace);
    return n;

fail:
    p->index = index;
    return NULL;
}

// ContainerDecl <- (KEYWORD_extern / KEYWORD_packed)? ContainerDeclAuto
static Node* Parser_parseContainerDecl(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    bool is_extern = Parser_eat(p, token_keyword_extern);
    bool is_packed = Parser_eat(p, token_keyword_packed);
    Node *container_decl = Parser_parseContainerDeclAuto(p);
    if (!container_decl) goto fail;

    Node *n = Parser_allocNode(p);
    n->tag = node_container_decl;
    n->data.container_decl = (NodeDataContainerDecl){
        .is_extern = is_extern,
        .is_packed = is_packed,
        .container_decl = container_decl,
    };
    return n;

fail:
    p->index = index;
    return NULL;
}

static Node* Parser_parseInitList(Parser *p);
static Node* Parser_expectFnProto(Parser *p);
// PrimaryTypeExpr
//     <- BUILTINIDENTIFIER FnCallArguments
//      / CHAR_LITERAL
//      / ContainerDecl
//      / DOT IDENTIFIER
//      / DOT InitList
//      / ErrorSetDecl
//      / FLOAT
//      / FnProto
//      / GroupedExpr
//      / LabeledTypeExpr
//      / IDENTIFIER
//      / IfTypeExpr
//      / INTEGER
//      / KEYWORD_comptime TypeExpr
//      / KEYWORD_error DOT IDENTIFIER
//      / KEYWORD_anyframe
//      / KEYWORD_unreachable
//      / STRINGLITERAL
static Node* Parser_parsePrimaryTypeExpr(Parser *p)
{
    Parser_trace(p);
    NodeDataPrimaryTypeExpr expr;
    Token tok = p->tokens[p->index];
    Buffer raw = Buffer_slice(p->source, tok.loc.start, tok.loc.end);

    if (Parser_eat(p, token_builtin)) {
        Node *args = Parser_parseFnCallArguments(p);
        expr.tag = node_primary_type_builtin;
        expr.data = (NodePrimaryTypeData){
            .builtin = (NodePrimaryTypeDataBuiltin){
                .name = raw,
                .args = args,
            },
        };
        goto done;
    }
    if (Parser_eat(p, token_char_literal)) {
        expr.tag = node_primary_type_char_literal;
        expr.data = (NodePrimaryTypeData){ .raw = raw };
        goto done;
    }
    Node *container_decl = Parser_parseContainerDecl(p);
    if (container_decl) {
        expr.tag = node_primary_type_container_decl;
        expr.data = (NodePrimaryTypeData){ .node = container_decl };
        goto done;
    }
    if (Parser_eat(p, token_period)) {
        raw = Parser_eatIdentifier(p);
        if (raw.len != 0) {
            expr.tag = node_primary_type_dot_identifier;
            expr.data = (NodePrimaryTypeData){ .raw = raw };
            goto done;
        }
        Node *initlist = Parser_parseInitList(p);
        if (!initlist) Parser_fail(p, "expected initlist");
        expr.tag = node_primary_type_dot_initlist;
        expr.data = (NodePrimaryTypeData){ .node = initlist };
        goto done;
    }
    Node *error_set_decl = Parser_parseErrorSetDecl(p);
    if (error_set_decl) {
        expr.tag = node_primary_type_error_set_decl;
        expr.data = (NodePrimaryTypeData){ .node = error_set_decl };
        goto done;
    }
    if (Parser_peek(p, token_keyword_fn)) {
        Node *fn_proto = Parser_expectFnProto(p);
        expr.tag = node_primary_type_fn_proto;
        expr.data = (NodePrimaryTypeData){ .node = fn_proto };
        goto done;
    }
    Node *grouped_expr = Parser_parseGroupedExpr(p);
    if (grouped_expr) {
        expr.tag = node_primary_type_grouped_expr;
        expr.data = (NodePrimaryTypeData){ .node = grouped_expr };
        goto done;
    }
    Node *labeled_type_expr = Parser_parseLabeledTypeExpr(p);
    if (labeled_type_expr) {
        expr.tag = node_primary_type_labeled_type_expr;
        expr.data = (NodePrimaryTypeData){ .node = labeled_type_expr };
        goto done;
    }
    if (Parser_eat(p, token_identifier)) {
        expr.tag = node_primary_type_identifier;
        expr.data = (NodePrimaryTypeData){ .raw = raw };
        goto done;
    }
    Node *if_type_expr = Parser_parseIfTypeExpr(p);
    if (if_type_expr) {
        expr.tag = node_primary_type_if_type_expr;
        expr.data = (NodePrimaryTypeData){ .node = if_type_expr };
        goto done;
    }
    if (Parser_eat(p, token_number_literal)) {
        expr.tag = node_primary_type_number_literal;
        expr.data = (NodePrimaryTypeData){ .raw = raw };
        goto done;
    }
    if (Parser_eat(p, token_keyword_comptime)) {
        Node *type_expr = Parser_parseTypeExpr(p);
        expr.tag = node_primary_type_comptime_type_expr;
        expr.data = (NodePrimaryTypeData){ .node = type_expr };
        goto done;
    }
    if (Parser_eat(p, token_keyword_error)) {
        Parser_expect(p, token_period);
        Buffer raw = Parser_expectIdentifier(p);
        expr.tag = node_primary_type_error;
        expr.data = (NodePrimaryTypeData){ .raw = raw };
        goto done;
    }
    if (Parser_eat(p, token_keyword_anyframe)) {
        expr.tag = node_primary_type_anyframe;
        goto done;
    }
    if (Parser_eat(p, token_keyword_unreachable)) {
        expr.tag = node_primary_type_unreachable;
        goto done;
    }
    if (Parser_eat(p, token_string_literal)) {
        expr.tag = node_primary_type_string_literal;
        expr.data = (NodePrimaryTypeData){ .raw = raw };
        goto done;
    }
    // TODO: could merge multiline literals.
    // We need to truncate the prefix of the raw data and merge the results here.
    if (Parser_peek(p, token_multiline_string_literal_line)) {
        while (Parser_eat(p, token_multiline_string_literal_line)) {}

        expr.tag = node_primary_type_string_literal;
        expr.data = (NodePrimaryTypeData){ .raw = raw };
        goto done;
    }

    return NULL;

    Node *n;
done:
    n = Parser_allocNode(p);
    n->tag = node_primary_type_expr;
    n->data.primary_type_expr = expr;
    return n;
}

// SuffixExpr
//     <- PrimaryTypeExpr (SuffixOp / FnCallArguments)*
static Node* Parser_parseSuffixExpr(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    Node *primary_type_expr = Parser_parsePrimaryTypeExpr(p);
    if (!primary_type_expr) goto fail;

    NodeArray a;
    NodeArray_init(&a);

    int c = 0;
    while (c++ < LOOP_MAX) {
        Node *suffix = Parser_parseSuffixOp(p);
        if (!suffix) suffix = Parser_parseFnCallArguments(p);
        if (!suffix) break;
        NodeArray_append(&a, suffix);
    }
    if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");

    Node *n = Parser_allocNode(p);
    n->tag = node_suffix_expr;
    n->data.suffix_expr = (NodeDataSuffixExpr){
        .expr = primary_type_expr,
        .suffixes = a.data,
        .suffixes_len = a.len,
    };
    return n;

fail:
    p->index = index;
    return NULL;
}

// ErrorUnionExpr <- SuffixExpr (EXCLAMATIONMARK TypeExpr)?
static Node* Parser_parseErrorUnionExpr(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    Node *suffix_expr = Parser_parseSuffixExpr(p);
    if (!suffix_expr) goto fail;

    Node *error_type_expr = NULL;
    if (Parser_eat(p, token_bang)) {
        error_type_expr = Parser_parseTypeExpr(p);
    }

    Node *n = Parser_allocNode(p);
    n->tag = node_error_union_expr;
    n->data.error_union_expr = (NodeDataErrorUnionExpr){
        .suffix_expr = suffix_expr,
        .error_type_expr = error_type_expr,
    };
    return n;

fail:
    p->index = index;
    return NULL;
}

// TypeExpr <- PrefixTypeOp* ErrorUnionExpr
static Node* Parser_parseTypeExpr(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    NodeArray a;
    NodeArray_init(&a);

    int c = 0;
    while (c++ < LOOP_MAX) {
        Node *prefix_type_op = Parser_parsePrefixTypeOp(p);
        if (!prefix_type_op) break;
        NodeArray_append(&a, prefix_type_op);
    }
    if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");

    Node *error_union_expr = Parser_parseErrorUnionExpr(p);
    if (!error_union_expr) goto fail;

    Node *n = Parser_allocNode(p);
    n->tag = node_type_expr;
    n->data.type_expr = (NodeDataTypeExpr){
        .prefix_type_ops = a.data,
        .prefix_type_ops_len = a.len,
        .type_expr = error_union_expr
    };
    return n;

fail:
    p->index = index;
    return NULL;
}

// InitList
//     <- LBRACE FieldInit (COMMA FieldInit)* COMMA? RBRACE
//      / LBRACE Expr (COMMA Expr)* COMMA? RBRACE
//      / LBRACE RBRACE
static Node* Parser_parseInitList(Parser *p)
{
    Parser_trace(p);
    if (!Parser_eat(p, token_l_brace)) return NULL;

    if (Parser_eat(p, token_r_brace)) {
        Node *n = Parser_allocNode(p);
        n->tag = node_init_list_empty;
        return n;
    }

    NodeArray a;
    NodeArray_init(&a);
    NodeTag tag = node_invalid;

    Node *field_init = Parser_parseFieldInit(p);
    if (field_init) {
        tag = node_init_list_field;
        NodeArray_append(&a, field_init);

        int c = 0;
        while (c++ < LOOP_MAX && Parser_eat(p, token_comma)) {
            Node *field_init = Parser_parseFieldInit(p);
            if (!field_init) break;
            NodeArray_append(&a, field_init);
        }
        if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");
    }

    Node *expr = Parser_parseExpr(p);
    if (expr) {
        tag = node_init_list_expr;
        NodeArray_append(&a, expr);

        int c = 0;
        while (c++ < LOOP_MAX && Parser_eat(p, token_comma)) {
            Node *expr = Parser_parseExpr(p);
            if (!expr) break;
            NodeArray_append(&a, expr);
        }
        if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");
    }

    if (tag == node_invalid) Parser_fail(p, "expected field_init or expr in init list");

    Parser_eat(p, token_comma);
    Parser_expect(p, token_r_brace);

    Node *n = Parser_allocNode(p);
    n->tag = tag;
    n->data.init_list_expr = (NodeDataInitList){
        .nodes = a.data,
        .nodes_len = a.len,
    };
    return n;
}

// CurlySuffixExpr <- TypeExpr InitList?
static Node* Parser_parseCurlySuffixExpr(Parser *p)
{
    Parser_trace(p);
    Node *type = Parser_parseTypeExpr(p);
    if (!type) return NULL;
    Node *initlist = Parser_parseInitList(p);
    if (!initlist) return type;

    Node *n = Parser_allocNode(p);
    n->tag = node_curly_suffix_expr;
    n->data.curly_suffix_expr = (NodeDataCurlySuffixExpr){
        .type = type,
        .initlist = initlist,
    };
    return n;
}

// WhileExpr <- WhilePrefix Expr (KEYWORD_else Payload? Expr)?
static Node* Parser_parseWhileExpr(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    Node *while_expr = Parser_parseWhilePrefix(p);
    if (!while_expr) goto fail;
    Node *expr = Parser_parseExpr(p);
    if (!expr) goto fail;
    Buffer else_payload_name = Buffer_empty();
    Node *else_expr = NULL;
    if (Parser_eat(p, token_keyword_else)) {
        else_payload_name = Parser_parsePayload(p);
        else_expr = Parser_parseExpr(p);
    }

    Node *n = Parser_allocNode(p);
    n->tag = node_while_expr;
    n->data.while_expr = (NodeDataWhileExpr){
        .condition = while_expr,
        .expr = expr,
        .else_payload_name = else_payload_name,
        .else_expr = else_expr,
    };
    return n;

fail:
    p->index = index;
    return NULL;
}

// ForExpr <- ForPrefix Expr (KEYWORD_else Expr)?
static Node* Parser_parseForExpr(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    Node *for_prefix = Parser_parseForPrefix(p);
    if (!for_prefix) goto fail;
    Node *expr = Parser_parseExpr(p);
    if (!expr) goto fail;
    Node *else_expr = NULL;
    if (Parser_eat(p, token_keyword_else)) {
        else_expr = Parser_parseExpr(p);
    }

    Node *n = Parser_allocNode(p);
    n->tag = node_for_expr;
    n->data.for_expr = (NodeDataForExpr){
        .condition = for_prefix,
        .expr = expr,
        .else_expr = else_expr,
    };
    return n;

fail:
    p->index = index;
    return NULL;
}

// LoopExpr <- KEYWORD_inline? (ForExpr / WhileExpr)
static Node* Parser_parseLoopExpr(Parser *p)
{
    Parser_trace(p);
    bool is_inline = Parser_eat(p, token_keyword_inline);
    (void)is_inline;    // TODO

    Node *for_expr = Parser_parseForExpr(p);
    if (for_expr) return for_expr;

    Node *while_expr = Parser_parseWhileExpr(p);
    if (while_expr) return while_expr;

    return NULL;
}

static Node* Parser_parseStatement(Parser *p);
// Block <- LBRACE Statement* RBRACE
static Node* Parser_parseBlock(Parser *p)
{
    Parser_trace(p);
    if (!Parser_eat(p, token_l_brace)) return NULL;

    NodeArray a;
    NodeArray_init(&a);

    int c = 0;
    while (c++ < LOOP_MAX && !Parser_peek(p, token_r_brace)) {
        Node *n = Parser_parseStatement(p);
        if (!n) break;
        NodeArray_append(&a, n);
    }
    if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");
    Parser_expect(p, token_r_brace);

    Node *n = Parser_allocNode(p);
    n->tag = node_block;
    n->data.block = (NodeDataBlock){
        .statements = a.data,
        .statements_len = a.len,
    };
    return n;
}

// IfExpr <- IfPrefix Expr (KEYWORD_else Payload? Expr)?
static Node* Parser_parseIfExpr(Parser *p)
{
    Parser_trace(p);
    Node *if_prefix = Parser_parseIfPrefix(p);
    if (!if_prefix) return NULL;
    Node *expr = Parser_parseExpr(p);
    if (!expr) Parser_fail(p, "expected expression");
    Buffer else_payload_name = Buffer_empty();
    Node *else_payload_expr = NULL;

    if (Parser_eat(p, token_keyword_else)) {
        else_payload_name = Parser_parsePayload(p);
        else_payload_expr = Parser_parseExpr(p);
        if (!else_payload_expr) Parser_fail(p, "expected expression");
    }

    Node *n = Parser_allocNode(p);
    n->tag = node_if_expr;
    n->data.if_expr = (NodeDataIfExpr){
        .condition = if_prefix,
        .expr = expr,
        .else_payload_name = else_payload_name,
        .else_payload_expr = else_payload_expr,
    };
    return n;
}

static Node* Parser_parseExpr(Parser *p);
// PrimaryExpr
//     <- AsmExpr
//      / IfExpr
//      / KEYWORD_break BreakLabel? Expr?
//      / KEYWORD_comptime Expr
//      / KEYWORD_nosuspend Expr
//      / KEYWORD_continue BreakLabel? Expr?
//      / KEYWORD_resume Expr
//      / KEYWORD_return Expr?
//      / BlockLabel? LoopExpr
//      / Block
//      / CurlySuffixExpr
static Node* Parser_parsePrimaryExpr(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    if (Parser_peek(p, token_keyword_asm)) {
        return Parser_parseAsmExpr(p);
    } else if (Parser_peek(p, token_keyword_if)) {
        return Parser_parseIfExpr(p);
    } else if (Parser_eat(p, token_keyword_break)) {
        Buffer label = Parser_parseBreakLabel(p);
        Node *expr = Parser_parseExpr(p);
        Node *n = Parser_allocNode(p);
        n->tag = node_break_expr;
        n->data.break_expr = (NodeDataBreakExpr){
            .label = label,
            .expr = expr,
        };
        return n;
    } else if (Parser_eat(p, token_keyword_comptime)) {
        Node *expr = Parser_parseExpr(p);
        if (!expr) Parser_fail(p, "expected expression after comptime keyword");
        Node *n = Parser_allocNode(p);
        n->tag = node_comptime_expr;
        n->data.comptime_expr = expr;
        return n;
    } else if (Parser_eat(p, token_keyword_nosuspend)) {
        Node *expr = Parser_parseExpr(p);
        if (!expr) Parser_fail(p, "expected expression after nosuspend keyword");
        Node *n = Parser_allocNode(p);
        n->data.comptime_expr = expr;
        n->tag = node_nosuspend_expr;
        return n;
    } else if (Parser_eat(p, token_keyword_continue)) {
        Buffer label = Parser_parseBreakLabel(p);
        Node *expr = Parser_parseExpr(p);
        Node *n = Parser_allocNode(p);
        n->tag = node_continue_expr;
        n->data.continue_expr = (NodeDataContinueExpr){
            .label = label,
            .expr = expr,
        };
        return n;
    } else if (Parser_eat(p, token_keyword_resume)) {
        Node *expr = Parser_parseExpr(p);
        if (!expr) Parser_fail(p, "expected expression after resume keyword");
        Node *n = Parser_allocNode(p);
        n->tag = node_resume_expr;
        n->data.resume_expr = expr;
        return n;
    } else if (Parser_eat(p, token_keyword_return)) {
        Node *expr = Parser_parseExpr(p);
        Node *n = Parser_allocNode(p);
        n->tag = node_return_expr;
        n->data.return_expr = expr;
        return n;
    }

    int c = 0;
    while (c++ < LOOP_MAX) {
        Buffer label = Parser_parseBlockLabel(p);
        Node *loop_expr = Parser_parseLoopExpr(p);
        if (!loop_expr) break;

        Node *n = Parser_allocNode(p);
        n->tag = node_loop_expr;
        n->data.loop_expr = (NodeDataLoopExpr) {
            .label = label,
            .loop_expr = loop_expr,
        };
        return n;
    }
    if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");

    p->index = index; // reset possible block label (may not be needed)

    if (Parser_peek(p, token_l_brace)) {
        return Parser_parseBlock(p);
    }

    return Parser_parseCurlySuffixExpr(p);
}

// PrefixExpr <- PrefixOp* PrimaryExpr
static Node* Parser_parsePrefixExpr(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    TokenTagArray a;
    TokenTagArray_init(&a);

    int c = 0;
    while (c++ < LOOP_MAX) {
        TokenTag prefixOp = Parser_eatPrefixOp(p);
        if (prefixOp == token_invalid) break;
        TokenTagArray_append(&a, prefixOp);
    }
    if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");

    Node *expr = Parser_parsePrimaryExpr(p);
    if (!expr) goto fail;

    Node *n = Parser_allocNode(p);
    n->tag = node_unary_expr;
    n->data.unary_expr = (NodeDataUnaryExpr){
        .ops = a.data,
        .ops_len = a.len,
        .expr = expr,
    };
    return n;

fail:
    p->index = index;
    return NULL;
}

// x() x[] x.y x.* x.?
// a!b
// x{}
// !x -x -%x ~x &x ?x
// * / % ** *% *| ||
// + - ++ +% -% +| -|
// << >> <<|
// & ^ | orelse catch
// == != < > <= >=
// and
// or
// = *= *%= *|= /= %= += +%= +|= -= -%= -|= <<= <<|= >>= &= ^= |=
static int Parser_binOpPrecedence(BinOp op)
{
    switch (op) {
        case binop_or:
            return 2;
        case binop_and:
            return 3;
        case binop_eq:
        case binop_neq:
        case binop_lt:
        case binop_gt:
        case binop_lt_eq:
        case binop_gt_eq:
            return 4;
        case binop_bit_and:
        case binop_bit_or:
        case binop_bit_xor:
        case binop_orelse:
        case binop_catch:
            return 5;
        case binop_shl:
        case binop_shr:
        case binop_shl_saturate:
            return 6;
        case binop_add:
        case binop_add_wrap:
        case binop_add_saturate:
        case binop_sub:
        case binop_sub_wrap:
        case binop_sub_saturate:
        case binop_array_concat:
            return 7;
        case binop_mul:
        case binop_mul_wrap:
        case binop_mul_saturate:
        case binop_div:
        case binop_mod:
        case binop_array_spread:
        case binop_error_set_merge:
            return 8;

        case binop_invalid:
            assume(false);
            return -1;
    }
}

// Peek a binary op (excluding assignment operators)
static BinOp Parser_peekBinOp(Parser *p)
{
    Parser_trace(p);
    switch (p->tokens[p->index].tag) {
        case token_keyword_or:
            return binop_or;
        case token_keyword_and:
            return binop_and;
        case token_equal_equal:
            return binop_eq;
        case token_bang_equal:
            return binop_neq;
        case token_angle_bracket_left:
            return binop_lt;
        case token_angle_bracket_right:
            return binop_gt;
        case token_angle_bracket_left_equal:
            return binop_lt_eq;
        case token_angle_bracket_right_equal:
            return binop_gt_eq;
        case token_ampersand:
            return binop_bit_and;
        case token_pipe:
            return binop_bit_or;
        case token_caret:
            return binop_bit_xor;
        case token_keyword_orelse:
            return binop_orelse;
        case token_keyword_catch:
            return binop_catch;
        case token_angle_bracket_angle_bracket_left:
            return binop_shl;
        case token_angle_bracket_angle_bracket_right:
            return binop_shr;
        case token_angle_bracket_angle_bracket_left_pipe:
            return binop_shl_saturate;
        case token_plus:
            return binop_add;
        case token_plus_percent:
            return binop_add_wrap;
        case token_plus_pipe:
            return binop_add_saturate;
        case token_minus:
            return binop_sub;
        case token_minus_percent:
            return binop_sub_wrap;
        case token_minus_pipe:
            return binop_sub_saturate;
        case token_asterisk_asterisk:
            return binop_array_spread;
        case token_plus_plus:
            return binop_array_concat;
        case token_asterisk:
            return binop_mul;
        case token_asterisk_percent:
            return binop_mul_wrap;
        case token_asterisk_pipe:
            return binop_mul_saturate;
        case token_slash:
            return binop_div;
        case token_percent:
            return binop_mod;
        case token_pipe_pipe:
            return binop_error_set_merge;
        default:
            return binop_invalid;
    }
}

// Expr <- BoolOrExpr
static Node* Parser_parseExpr0(Parser *p, int min_prec)
{
    Node *lhs = Parser_parsePrefixExpr(p);

    int c = 0;
    while (c++ < LOOP_MAX) {
        BinOp op = Parser_peekBinOp(p);
        if (op == binop_invalid) return lhs;
        int prec = Parser_binOpPrecedence(op);
        if (prec < min_prec) break;
        p->index++;

        Node *rhs = Parser_parseExpr0(p, prec + 1);
        Node *n = Parser_allocNode(p);
        n->tag = node_binary_expr;
        n->data.binary_expr = (NodeDataBinaryExpr){
            .op = op,
            .lhs = lhs,
            .rhs = rhs,
        };
        lhs = n;
    }
    if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");

    return lhs;
}
static Node* Parser_parseExpr(Parser *p)
{
    Parser_trace(p);
    return Parser_parseExpr0(p, 0);
}

// SingleAssignExpr <- Expr (AssignOp Expr)?
static Node* Parser_parseSingleAssignExpr(Parser *p)
{
    Parser_trace(p);
    Node *lhs = Parser_parseExpr(p);
    TokenTag assign_op = Parser_eatAssignOp(p);
    if (assign_op == token_invalid) return lhs;
    Node *rhs = Parser_parseExpr(p);
    if (!rhs) Parser_fail(p, "expected expression");

    Node *n = Parser_allocNode(p);
    n->tag = node_single_assign_expr;
    n->data.single_assign_expr = (NodeDataSingleAssignExpr){
        .lhs = lhs,
        .assign_op = assign_op,
        .rhs = rhs,
    };
    return n;
}

// AssignExpr <- Expr (AssignOp Expr / (COMMA Expr)+ EQUAL Expr)?
static Node* Parser_parseAssignExpr(Parser *p)
{
    Parser_trace(p);
    Node *lhs = Parser_parseExpr(p);
    TokenTag assign_op = Parser_eatAssignOp(p);
    if (assign_op != token_invalid) {
        Node *rhs = Parser_parseExpr(p);
        if (!rhs) Parser_fail(p, "expected expression");

        Node *n = Parser_allocNode(p);
        n->tag = node_single_assign_expr;
        n->data.single_assign_expr = (NodeDataSingleAssignExpr){
            .lhs = lhs,
            .assign_op = assign_op,
            .rhs = rhs,
        };
        return n;
    }
    if (!Parser_peek(p, token_comma)) return lhs;

    NodeArray a;
    NodeArray_init(&a);

    int c = 0;
    while (c++ < LOOP_MAX && Parser_eat(p, token_comma)) {
        Node *expr = Parser_parseExpr(p);
        if (!expr) Parser_fail(p, "expected expression");
        NodeArray_append(&a, expr);
    }
    if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");
    Parser_expect(p, token_equal);
    Node *rhs = Parser_parseExpr(p);
    if (!rhs) Parser_fail(p, "expected expression");
    Parser_expect(p, token_semicolon);

    Node *n = Parser_allocNode(p);
    n->tag = node_multi_assign_expr;
    n->data.multi_assign_expr = (NodeDataMultiAssignExpr){
        .lhs = lhs,
        .lhs_additional = a.data,
        .lhs_additional_len = a.len,
        .expr = rhs,
    };
    return n;
}

static Node* Parser_parseVarDeclProto(Parser *p);
// VarDeclExprStatement
//     <- VarDeclProto (COMMA (VarDeclProto / Expr))* EQUAL Expr SEMICOLON
//      / Expr (AssignOp Expr / (COMMA (VarDeclProto / Expr))+ EQUAL Expr)? SEMICOLON
static Node* Parser_parseVarDeclExprStatement(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    NodeArray a;
    NodeArray_init(&a);

    if (Parser_peek(p, token_keyword_const) || Parser_peek(p, token_keyword_var)) {;
        Node *proto = Parser_parseVarDeclProto(p);
        if (!proto) goto fail;

        int c = 0;
        while (c++ < LOOP_MAX && Parser_eat(p, token_comma)) {
            Node *proto_or_expr = NULL;
            proto_or_expr = Parser_parseVarDeclProto(p);
            if (!proto_or_expr) proto_or_expr = Parser_parseExpr(p);
            if (!proto_or_expr) goto fail;
            NodeArray_append(&a, proto_or_expr);
        }
        if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");
        if (!Parser_eat(p, token_equal)) goto fail;
        Node *expr = Parser_parseExpr(p);
        if (!expr) goto fail;
        if (!Parser_eat(p, token_semicolon)) goto fail;

        Node *n = Parser_allocNode(p);
        n->tag = node_var_decl_statement;
        n->data.var_decl_statement = (NodeDataVarDeclStatement){
            .var_decl = proto,
            .var_decl_additional = a.data,
            .var_decl_additional_len = a.len,
            .expr = expr,
        };
        return n;
    }

    Node *lhs_expr = Parser_parseExpr(p);
    if (!lhs_expr) goto fail;
    if (Parser_eat(p, token_semicolon)) {
        return lhs_expr;
    }

    TokenTag assign_op = Parser_eatAssignOp(p);
    if (assign_op != token_invalid) {
        Node *rhs_expr = Parser_parseExpr(p);
        if (!Parser_eat(p, token_semicolon)) goto fail;
        Node *n = Parser_allocNode(p);
        n->tag = node_single_assign_expr;
        n->data.single_assign_expr = (NodeDataSingleAssignExpr){
            .lhs = lhs_expr,
            .assign_op = assign_op,
            .rhs = rhs_expr,
        };
        return n;
    }

    if (!Parser_peek(p, token_comma)) goto fail;

    int c = 0;
    while (c++ < LOOP_MAX && Parser_eat(p, token_comma)) {
        Node *proto_or_expr = NULL;
        proto_or_expr = Parser_parseVarDeclProto(p);
        if (!proto_or_expr) proto_or_expr = Parser_parseExpr(p);
        if (!proto_or_expr) goto fail;
        NodeArray_append(&a, proto_or_expr);
    }
    if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");
    if (!Parser_eat(p, token_equal)) goto fail;
    Node *expr = Parser_parseExpr(p);
    if (!expr) goto fail;
    if (!Parser_eat(p, token_semicolon)) goto fail;

    Node *n = Parser_allocNode(p);
    n->tag = node_var_decl_statement;
    n->data.var_decl_statement = (NodeDataVarDeclStatement){
        .var_decl = lhs_expr,
        .var_decl_additional = a.data,
        .var_decl_additional_len = a.len,
        .expr = expr,
    };
    return n;

fail:
    p->index = index;
    return NULL;
}

// BlockExpr <- BlockLabel? Block
static Node* Parser_parseBlockExpr(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    Buffer label = Parser_parseBlockLabel(p);
    Node *block = Parser_parseBlock(p);
    if (block == NULL) goto fail;
    if (label.len == 0) return block;

    Node *n = Parser_allocNode(p);
    n->tag = node_labeled_block;
    n->data.labeled_type_expr = (NodeDataLabeledTypeExpr){
        .label = label,
        .node = block,
    };
    return n;

fail:
    p->index = index;
    return NULL;
}

// BlockExprStatement
//     <- BlockExpr
//      / AssignExpr SEMICOLON
static Node* Parser_expectBlockExprStatement(Parser *p)
{
    Parser_trace(p);
    Node *block_expr = Parser_parseBlockExpr(p);
    if (block_expr) return block_expr;

    Node *assign_expr = Parser_parseAssignExpr(p);
    Parser_expect(p, token_semicolon);
    return assign_expr;
}

// WhileStatement
//     <- WhilePrefix BlockExpr ( KEYWORD_else Payload? Statement )?
//      / WhilePrefix AssignExpr ( SEMICOLON / KEYWORD_else Payload? Statement )
static Node* Parser_parseWhileStatement(Parser *p)
{
    Parser_trace(p);
    Node *while_prefix = Parser_parseWhilePrefix(p);
    Buffer else_payload_name = Buffer_empty();
    Node *else_statement = NULL;

    Node *block_expr = Parser_parseBlockExpr(p);
    if (block_expr) {
        if (Parser_eat(p, token_keyword_else)) {
            else_payload_name = Parser_parsePayload(p);
            else_statement = Parser_parseStatement(p);
            if (!else_statement) Parser_fail(p, "expected statement after while block");
        }
        Node *n = Parser_allocNode(p);
        n->tag = node_while_statement;
        n->data.while_statement = (NodeDataWhileStatement){
            .condition = while_prefix,
            .block = block_expr,
            .else_payload_name = else_payload_name,
            .else_statement = else_statement,
        };
        return n;
    }

    Node *assign_expr = Parser_parseAssignExpr(p);
    if (assign_expr) {
        if (!Parser_eat(p, token_semicolon)) {
            if (!Parser_eat(p, token_keyword_else)) {
                Parser_fail(p, "expected semicolon or else");
            }
            else_payload_name = Parser_parsePayload(p);
            else_statement = Parser_parseStatement(p);
            if (!else_statement) Parser_fail(p, "expected statement after while block");
        }
        Node *n = Parser_allocNode(p);
        n->tag = node_while_statement;
        n->data.while_statement = (NodeDataWhileStatement){
            .condition = while_prefix,
            .block = assign_expr,
            .else_payload_name = else_payload_name,
            .else_statement = else_statement,
        };
        return n;
    }

    Parser_fail(p, "expected block or assignment");
}

// ForStatement
//     <- ForPrefix BlockExpr ( KEYWORD_else Statement )?
//      / ForPrefix AssignExpr ( SEMICOLON / KEYWORD_else Statement )
static Node* Parser_parseForStatement(Parser *p)
{
    Parser_trace(p);
    Node *for_prefix = Parser_parseForPrefix(p);
    Node *else_statement = NULL;

    Node *block_expr = Parser_parseBlockExpr(p);
    if (block_expr) {
        if (Parser_eat(p, token_keyword_else)) {
            else_statement = Parser_parseStatement(p);
            if (!else_statement) Parser_fail(p, "expected statement after for block");
        }
        Node *n = Parser_allocNode(p);
        n->tag = node_for_statement;
        n->data.for_statement = (NodeDataForStatement){
            .condition = for_prefix,
            .block = block_expr,
            .else_statement = else_statement,
        };
        return n;
    }

    Node *assign_expr = Parser_parseAssignExpr(p);
    if (assign_expr) {
        if (!Parser_eat(p, token_semicolon)) {
            if (!Parser_eat(p, token_keyword_else)) {
                Parser_fail(p, "expected semicolon or else");
            }
            else_statement = Parser_parseStatement(p);
            if (!else_statement) Parser_fail(p, "expected statement after for block");
        }
        Node *n = Parser_allocNode(p);
        n->tag = node_for_statement;
        n->data.for_statement = (NodeDataForStatement){
            .condition = for_prefix,
            .block = assign_expr,
            .else_statement = else_statement,
        };
        return n;
    }

    Parser_fail(p, "expected block or assignment");
}

// LoopStatement <- KEYWORD_inline? (ForStatement / WhileStatement)
static Node* Parser_parseLoopStatement(Parser *p)
{
    Parser_trace(p);
    bool is_inline = Parser_eat(p, token_keyword_inline);
    Node *statement = NULL;
    if (Parser_peek(p, token_keyword_for)) {
        statement = Parser_parseForStatement(p);
    } else if (Parser_peek(p, token_keyword_while)) {
        statement = Parser_parseWhileStatement(p);
    } else {
        return NULL;
    }

    Node *n = Parser_allocNode(p);
    n->tag = node_loop_statement;
    n->data.loop_statement = (NodeDataLoopStatement){
        .is_inline = is_inline,
        .statement = statement,
    };
    return n;
}

// LabeledStatement <- BlockLabel? (Block / LoopStatement / SwitchExpr)
static Node* Parser_parseLabeledStatement(Parser *p)
{
    Parser_trace(p);
    Buffer label = Parser_parseBlockLabel(p);
    Node *block = NULL;

    if (Parser_peek(p, token_l_brace)) {
        block = Parser_parseBlock(p);
    } else if (Parser_peek(p, token_keyword_inline)
        || Parser_peek(p, token_keyword_for)
        || Parser_peek(p, token_keyword_while)) {
        block = Parser_parseLoopStatement(p);
    } else if (Parser_peek(p, token_keyword_switch)) {
        block = Parser_parseSwitchExpr(p);
    } else {
        return NULL;
    }

    Node *n = Parser_allocNode(p);
    n->tag = node_labeled_statement;
    n->data.labeled_statement = (NodeDataLabeledStatement){
        .label = label,
        .statement = block,
    };
    return n;
}

// IfStatement
//     <- IfPrefix BlockExpr ( KEYWORD_else Payload? Statement )?
//      / IfPrefix AssignExpr ( SEMICOLON / KEYWORD_else Payload? Statement )
static Node* Parser_expectIfStatement(Parser *p)
{
    Parser_trace(p);
    Node *if_prefix = Parser_parseIfPrefix(p);
    Buffer else_payload_name = Buffer_empty();
    Node *else_statement = NULL;

    Node *block_expr = Parser_parseBlockExpr(p);
    if (block_expr) {
        if (Parser_eat(p, token_keyword_else)) {
            else_payload_name = Parser_parsePayload(p);
            else_statement = Parser_parseStatement(p);
            if (!else_statement) Parser_fail(p, "expected statement after if block");
        }
        Node *n = Parser_allocNode(p);
        n->tag = node_if_statement;
        n->data.if_statement = (NodeDataIfStatement){
            .condition = if_prefix,
            .block = block_expr,
            .else_payload_name = else_payload_name,
            .else_statement = else_statement,
        };
        return n;
    }

    Node *assign_expr = Parser_parseAssignExpr(p);
    if (assign_expr) {
        if (!Parser_eat(p, token_semicolon)) {
            if (!Parser_eat(p, token_keyword_else)) {
                Parser_fail(p, "expected semicolon or else");
            }
            else_payload_name = Parser_parsePayload(p);
            else_statement = Parser_parseStatement(p);
            if (!else_statement) Parser_fail(p, "expected statement after if block");
        }
        Node *n = Parser_allocNode(p);
        n->tag = node_if_statement;
        n->data.if_statement = (NodeDataIfStatement){
            .condition = if_prefix,
            .block = assign_expr,
            .else_payload_name = else_payload_name,
            .else_statement = else_statement,
        };
        return n;
    }

    Parser_fail(p, "expected block or assignment");
}

// ComptimeStatement
//     <- BlockExpr
//      / VarDeclExprStatement
static Node* Parser_expectComptimeStatement(Parser *p)
{
    Parser_trace(p);
    Node *block_expr = Parser_parseBlockExpr(p);
    if (block_expr) return block_expr;

    Node *var_decl_expr = Parser_parseVarDeclExprStatement(p);
    if (var_decl_expr) return var_decl_expr;

    Parser_fail(p, "expected block expression or var/decl/expression");
}

// Statement
//     <- KEYWORD_comptime ComptimeStatement
//      / KEYWORD_nosuspend BlockExprStatement
//      / KEYWORD_suspend BlockExprStatement
//      / KEYWORD_defer BlockExprStatement
//      / KEYWORD_errdefer Payload? BlockExprStatement
//      / IfStatement
//      / LabeledStatement
//      / VarDeclExprStatement
static Node* Parser_parseStatement(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    if (Parser_eat(p, token_keyword_comptime)) {
        Node *comptime_statement = Parser_expectComptimeStatement(p);
        Node *n = Parser_allocNode(p);
        n->tag = node_comptime_statement;
        n->data.comptime_statement = (NodeDataComptimeStatement){
            .comptime_statement = comptime_statement,
        };
        return n;
    } else if (Parser_eat(p, token_keyword_nosuspend)) {
        Node *block_expr = Parser_expectBlockExprStatement(p);
        Node *n = Parser_allocNode(p);
        n->tag = node_nosuspend_statement;
        n->data.nosuspend_statement = (NodeDataBlockExprStatement){
            .block_expr = block_expr,
        };
        return n;
    } else if (Parser_eat(p, token_keyword_suspend)) {
        Node *block_expr = Parser_expectBlockExprStatement(p);
        Node *n = Parser_allocNode(p);
        n->tag = node_suspend_statement;
        n->data.suspend_statement = (NodeDataBlockExprStatement){
            .block_expr = block_expr,
        };
        return n;
    } else if (Parser_eat(p, token_keyword_defer)) {
        Node *block_expr = Parser_expectBlockExprStatement(p);
        Node *n = Parser_allocNode(p);
        n->tag = node_defer_statement;
        n->data.defer_statement = (NodeDataBlockExprStatement){
            .block_expr = block_expr,
        };
        return n;
    } else if (Parser_eat(p, token_keyword_errdefer)) {
        Buffer payload_name = Parser_parsePayload(p);
        Node *block_expr = Parser_expectBlockExprStatement(p);

        Node *n = Parser_allocNode(p);
        n->tag = node_errdefer_statement;
        n->data.errdefer_statement = (NodeDataErrdeferStatement){
            .payload_name = payload_name,
            .block_expr = block_expr,
        };
        return n;
    } else if (Parser_peek(p, token_keyword_if)) {
        return Parser_expectIfStatement(p);
    }

    Node *ls = Parser_parseLabeledStatement(p);
    if (ls) return ls;

    Node *vde = Parser_parseVarDeclExprStatement(p);
    if (vde) return vde;

    p->index = index;
    return NULL;
}

// ContainerField <- doc_comment? KEYWORD_comptime? !KEYWORD_fn (IDENTIFIER COLON)? TypeExpr ByteAlign? (EQUAL Expr)?
static Node* Parser_parseContainerField(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    while (Parser_eat(p, token_doc_comment)) {}
    bool is_comptime = Parser_eat(p, token_keyword_comptime);
    if (Parser_peek(p, token_keyword_fn)) goto fail;

    Buffer name = Parser_eatIdentifier(p);
    if (name.len != 0) {
        if (!Parser_eat(p, token_colon)) p->index--;    // reset, this is not a label
    }

    Node *type_expr = Parser_parseTypeExpr(p);
    if (!type_expr) goto fail;
    Node *bytealign = Parser_parseByteAlign(p);

    Node *expr = NULL;
    if (Parser_eat(p, token_equal)) {
        expr = Parser_parseExpr(p);
    }

    Node *n = Parser_allocNode(p);
    n->tag = node_container_field;
    n->data.container_field = (NodeDataContainerField){
        .name = name,
        .type_expr = type_expr,
        .bytealign = bytealign,
        .expr = expr,
        .is_comptime = is_comptime,
    };
    return n;

fail:
    p->index = index;
    return NULL;
}

static Node* Parser_parseVarDeclProto(Parser *p);
// GlobalVarDecl <- VarDeclProto (EQUAL Expr)? SEMICOLON
static Node* Parser_parseGlobalVarDecl(Parser *p)
{
    Parser_trace(p);
    Node *var_decl_proto = Parser_parseVarDeclProto(p);
    if (!var_decl_proto) return NULL;

    Node *expr = NULL;
    if (Parser_eat(p, token_equal)) {
        expr = Parser_parseExpr(p);
        if (!expr) Parser_fail(p, "failed to parse var decl expression");
    }
    Parser_expect(p, token_semicolon);

    Node *n = Parser_allocNode(p);
    n->tag = node_global_var_decl;
    n->data.global_var_decl = (NodeDataGlobalVarDecl){
        .var_decl_proto = var_decl_proto,
        .expr = expr,
    };
    return n;
}

// VarDeclProto <- (KEYWORD_const / KEYWORD_var) IDENTIFIER (COLON TypeExpr)? ByteAlign? AddrSpace? LinkSection?
static Node* Parser_parseVarDeclProto(Parser *p)
{
    Parser_trace(p);
    TokenTag tag = Parser_eatOneOf(p, (TokenTag[]){ token_keyword_const, token_keyword_var }, 2);
    if (tag == token_invalid) return NULL;
    Buffer name = Parser_expectIdentifier(p);

    Node *type = NULL;
    if (Parser_eat(p, token_colon)) {
        type = Parser_parseTypeExpr(p);
        if (!type) Parser_fail(p, "expected type after ';'");
    }

    Node *bytealign = Parser_parseByteAlign(p);
    Node *addrspace = Parser_parseAddrSpace(p);
    Node *linksection = Parser_parseLinkSection(p);

    Node *n = Parser_allocNode(p);
    n->tag = node_var_decl_proto;
    n->data.var_decl_proto = (NodeDataVarDeclProto){
        .is_const = tag == token_keyword_const,
        .name = name,
        .type = type,
        .bytealign = bytealign,
        .addrspace = addrspace,
        .linksection = linksection,
    };
    return n;
}

// FnProto <- KEYWORD_fn IDENTIFIER? LPAREN ParamDeclList RPAREN ByteAlign? AddrSpace? LinkSection? CallConv? EXCLAMATIONMARK? TypeExpr
static Node* Parser_expectFnProto(Parser *p)
{
    Parser_trace(p);
    Parser_expect(p, token_keyword_fn);
    Buffer name = Parser_eatIdentifier(p);
    Parser_expect(p, token_l_paren);
    Node *params = NULL;
    if (!Parser_peek(p, token_r_paren)) params = Parser_parseParamDeclList(p);
    Parser_expect(p, token_r_paren);
    Node *bytealign = Parser_parseByteAlign(p);
    Node *addrspace = Parser_parseAddrSpace(p);
    Node *linksection = Parser_parseLinkSection(p);
    Node *callconv = Parser_parseCallConv(p);
    bool is_return_type_error = Parser_eat(p, token_bang);
    Node *return_type = Parser_parseTypeExpr(p);
    if (!return_type) Parser_fail(p, "expected type expression");

    Node *extra = NULL;
    if (bytealign || addrspace || linksection || callconv) {
        extra = Parser_allocNode(p);
        extra->tag = node_fn_proto_extra;
        extra->data.fn_proto_extra = (NodeDataFnProtoExtra){
            .bytealign = bytealign,
            .addrspace = addrspace,
            .linksection = linksection,
            .callconv = callconv,
        };
    }

    Node *n = Parser_allocNode(p);
    n->tag = node_fn_proto;
    n->data.fn_proto = (NodeDataFnProto){
        .name = name,
        .params = params,
        .return_type = return_type,
        .extra_data = extra,
        .is_return_type_error = is_return_type_error,
    };
    return n;
}

// Decl
//     <- (KEYWORD_export / KEYWORD_extern STRINGLITERALSINGLE? / KEYWORD_inline / KEYWORD_noinline)? FnProto (SEMICOLON / Block)
//      / (KEYWORD_export / KEYWORD_extern STRINGLITERALSINGLE?)? KEYWORD_threadlocal? GlobalVarDecl
static Node* Parser_parseDecl(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    uint32_t modifiers = 0;
    Buffer extern_name = Buffer_empty();

    // can be stricter with this chain
    switch (p->tokens[p->index].tag) {
        case token_keyword_export:
            p->index++;
            modifiers |= decl_modifier_export;
            break;

        case token_keyword_extern:
            p->index++;
            modifiers |= decl_modifier_extern;
            extern_name = Parser_tokenSlice(p);
            if (!Parser_eat(p, token_string_literal)) extern_name = Buffer_empty();
            break;

        case token_keyword_inline:
            p->index++;
            modifiers |= decl_modifier_inline;
            break;

        case token_keyword_noinline:
            p->index++;
            modifiers |= decl_modifier_noinline;
            break;

        case token_keyword_threadlocal:
            p->index++;
            modifiers |= decl_modifier_threadlocal;
            break;

        default:
            break;
    }
    if (Parser_eat(p, token_keyword_threadlocal)) {
        modifiers |= decl_modifier_noinline;
    }

    if (Parser_peek(p, token_keyword_fn)) {
        Node *fn_proto = Parser_expectFnProto(p);
        Node *block = NULL;
        if (!Parser_eat(p, token_semicolon)) block = Parser_parseBlock(p);

        Node *n = Parser_allocNode(p);
        n->tag = node_decl_fn;
        n->data.decl_fn = (NodeDataDeclFn){
            .fn_proto = fn_proto,
            .block = block,
            .modifiers = modifiers,
            .extern_name = extern_name,
        };
        return n;
    } else {
        Node *global_var_decl = Parser_parseGlobalVarDecl(p);
        if (!global_var_decl) goto fail;

        Node *n = Parser_allocNode(p);
        n->tag = node_decl_global_var_decl;
        n->data.decl_global_var_decl = (NodeDataDeclGlobalVarDecl){
            .global_var_decl = global_var_decl,
            .modifiers = modifiers,
            .extern_name = extern_name,
        };
        return n;
    }

fail:
    p->index = index;
    return NULL;
}

// ComptimeDecl <- KEYWORD_comptime Block
static Node* Parser_expectComptimeDecl(Parser *p)
{
    Parser_trace(p);
    Parser_expect(p, token_keyword_comptime);
    Node *block = Parser_parseBlock(p);

    Node *n = Parser_allocNode(p);
    n->tag = node_comptime_decl;
    n->data.comptime_decl = (NodeDataComptimeDecl){
        .block = block,
    };
    return n;
}

// TestDecl <- KEYWORD_test (STRINGLITERALSINGLE / IDENTIFIER)? Block
static Node* Parser_expectTestDecl(Parser *p)
{
    Parser_trace(p);
    Parser_expect(p, token_keyword_test);

    Buffer name = Parser_tokenSlice(p);
    TokenTag tag = Parser_eatOneOf(p, (TokenTag[]){ token_string_literal, token_identifier }, 2);
    if (tag == token_invalid) name = Buffer_empty();
    Node *block = Parser_parseBlock(p);
    if (!block) Parser_fail(p, "expected block");

    Node *n = Parser_allocNode(p);
    n->tag = node_test_decl;
    n->data.test_decl = (NodeDataTestDecl){
        .name = name,
        .is_ident = tag == token_identifier,
        .block = block,
    };
    return n;
}

// ContainerDeclaration <- TestDecl / ComptimeDecl / doc_comment? KEYWORD_pub? Decl
static Node* Parser_parseContainerDeclaration(Parser *p)
{
    Parser_trace(p);
    uint32_t index = p->index;

    if (Parser_peek(p, token_keyword_test)) {
        return Parser_expectTestDecl(p);
    } else if (Parser_peek(p, token_keyword_comptime)) {
        return Parser_expectComptimeDecl(p);
    } else {
        while (Parser_eat(p, token_doc_comment)) {}
        bool is_pub = Parser_eat(p, token_keyword_pub);
        Node *decl = Parser_parseDecl(p);
        if (!decl) goto fail;

        Node *n = Parser_allocNode(p);
        n->tag = node_top_level_decl;
        n->data.top_level_decl = (NodeDataTopLevelDecl){
            .decl = decl,
            .is_pub = is_pub,
        };
        return n;
    }

fail:
    p->index = index;
    return NULL;
}

// ContainerMembers <- container_doc_comment? ContainerDeclaration* (ContainerField COMMA)* (ContainerField / ContainerDeclaration*)
static Node* Parser_expectContainerMembers(Parser *p)
{
    Parser_trace(p);

    // TODO: tokenizer should combine doc comment here, and not generate multiple
    while (Parser_eat(p, token_container_doc_comment)) {}

    NodeArray decls;
    NodeArray_init(&decls);

    NodeArray fields;
    NodeArray_init(&fields);

    int c = 0;
    while (c++ < LOOP_MAX && !Parser_peek(p, token_eof)) {
        Node *n = Parser_parseContainerDeclaration(p);
        if (!n) break;
        NodeArray_append(&decls, n);
    }
    if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");

    c = 0;
    while (c++ < LOOP_MAX && !Parser_peek(p, token_eof)) {
        Node *n = Parser_parseContainerField(p);
        if (!n) break;
        NodeArray_append(&fields, n);
        if (!Parser_eat(p, token_comma)) break;
    }
    if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");

    c = 0;
    while (c++ < LOOP_MAX && !Parser_peek(p, token_eof)) {
        Node *n = Parser_parseContainerDeclaration(p);
        if (!n) break;
        NodeArray_append(&decls, n);
    }
    if (c >= LOOP_MAX) Parser_fail(p, "infinite loop");

    Node *n = Parser_allocNode(p);
    n->tag = node_container_members;
    n->data.container_members = (NodeDataContainerMembers){
        .decls = decls.data,
        .decls_len = decls.len,
        .fields = fields.data,
        .fields_len = fields.len,
    };
    return n;
}

// Root <- skip ContainerMembers eof
static Node* Parser_expectRoot(Parser *p)
{
    Parser_trace(p);
    Node *n = Parser_expectContainerMembers(p);
    Parser_expect(p, token_eof);
    return n;
}

static Node* Parser_parse(Parser *p)
{
    return Parser_expectRoot(p);
}

#undef LOOP_MAX