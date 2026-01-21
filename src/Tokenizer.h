typedef enum TokenTag {
    token_invalid,
    token_invalid_periodasterisks,
    token_identifier,
    token_string_literal,
    token_multiline_string_literal_line,
    token_char_literal,
    token_eof,
    token_builtin,
    token_bang,
    token_pipe,
    token_pipe_pipe,
    token_pipe_equal,
    token_equal,
    token_equal_equal,
    token_equal_angle_bracket_right,
    token_bang_equal,
    token_l_paren,
    token_r_paren,
    token_semicolon,
    token_percent,
    token_percent_equal,
    token_l_brace,
    token_r_brace,
    token_l_bracket,
    token_r_bracket,
    token_period,
    token_period_asterisk,
    token_ellipsis2,
    token_ellipsis3,
    token_caret,
    token_caret_equal,
    token_plus,
    token_plus_plus,
    token_plus_equal,
    token_plus_percent,
    token_plus_percent_equal,
    token_plus_pipe,
    token_plus_pipe_equal,
    token_minus,
    token_minus_equal,
    token_minus_percent,
    token_minus_percent_equal,
    token_minus_pipe,
    token_minus_pipe_equal,
    token_asterisk,
    token_asterisk_equal,
    token_asterisk_asterisk,
    token_asterisk_percent,
    token_asterisk_percent_equal,
    token_asterisk_pipe,
    token_asterisk_pipe_equal,
    token_arrow,
    token_colon,
    token_slash,
    token_slash_equal,
    token_comma,
    token_ampersand,
    token_ampersand_equal,
    token_question_mark,
    token_angle_bracket_left,
    token_angle_bracket_left_equal,
    token_angle_bracket_angle_bracket_left,
    token_angle_bracket_angle_bracket_left_equal,
    token_angle_bracket_angle_bracket_left_pipe,
    token_angle_bracket_angle_bracket_left_pipe_equal,
    token_angle_bracket_right,
    token_angle_bracket_right_equal,
    token_angle_bracket_angle_bracket_right,
    token_angle_bracket_angle_bracket_right_equal,
    token_tilde,
    token_number_literal,
    token_doc_comment,
    token_container_doc_comment,
    token_keyword_addrspace,
    token_keyword_align,
    token_keyword_allowzero,
    token_keyword_and,
    token_keyword_anyframe,
    token_keyword_anytype,
    token_keyword_asm,
    token_keyword_break,
    token_keyword_callconv,
    token_keyword_catch,
    token_keyword_comptime,
    token_keyword_const,
    token_keyword_continue,
    token_keyword_defer,
    token_keyword_else,
    token_keyword_enum,
    token_keyword_errdefer,
    token_keyword_error,
    token_keyword_export,
    token_keyword_extern,
    token_keyword_fn,
    token_keyword_for,
    token_keyword_if,
    token_keyword_inline,
    token_keyword_noalias,
    token_keyword_noinline,
    token_keyword_nosuspend,
    token_keyword_opaque,
    token_keyword_or,
    token_keyword_orelse,
    token_keyword_packed,
    token_keyword_pub,
    token_keyword_resume,
    token_keyword_return,
    token_keyword_linksection,
    token_keyword_struct,
    token_keyword_suspend,
    token_keyword_switch,
    token_keyword_test,
    token_keyword_threadlocal,
    token_keyword_try,
    token_keyword_union,
    token_keyword_unreachable,
    token_keyword_var,
    token_keyword_volatile,
    token_keyword_while,
} TokenTag;

const char* TokenTag_name(TokenTag tag)
{
    switch (tag) {
    case token_invalid:
        return "invalid";
    case token_invalid_periodasterisks:
        return "invalid_periodasterisks";
    case token_identifier:
        return "identifier";
    case token_string_literal:
        return "string_literal";
    case token_multiline_string_literal_line:
        return "multiline_string_literal_line";
    case token_char_literal:
        return "char_literal";
    case token_eof:
        return "eof";
    case token_builtin:
        return "builtin";
    case token_bang:
        return "bang";
    case token_pipe:
        return "pipe";
    case token_pipe_pipe:
        return "pipe_pipe";
    case token_pipe_equal:
        return "pipe_equal";
    case token_equal:
        return "equal";
    case token_equal_equal:
        return "equal_equal";
    case token_equal_angle_bracket_right:
        return "equal_angle_bracket_right";
    case token_bang_equal:
        return "bang_equal";
    case token_l_paren:
        return "l_paren";
    case token_r_paren:
        return "r_paren";
    case token_semicolon:
        return "semicolon";
    case token_percent:
        return "percent";
    case token_percent_equal:
        return "percent_equal";
    case token_l_brace:
        return "l_brace";
    case token_r_brace:
        return "r_brace";
    case token_l_bracket:
        return "l_bracket";
    case token_r_bracket:
        return "r_bracket";
    case token_period:
        return "period";
    case token_period_asterisk:
        return "period_asterisk";
    case token_ellipsis2:
        return "ellipsis2";
    case token_ellipsis3:
        return "ellipsis3";
    case token_caret:
        return "caret";
    case token_caret_equal:
        return "caret_equal";
    case token_plus:
        return "plus";
    case token_plus_plus:
        return "plus_plus";
    case token_plus_equal:
        return "plus_equal";
    case token_plus_percent:
        return "plus_percent";
    case token_plus_percent_equal:
        return "plus_percent_equal";
    case token_plus_pipe:
        return "plus_pipe";
    case token_plus_pipe_equal:
        return "plus_pipe_equal";
    case token_minus:
        return "minus";
    case token_minus_equal:
        return "minus_equal";
    case token_minus_percent:
        return "minus_percent";
    case token_minus_percent_equal:
        return "minus_percent_equal";
    case token_minus_pipe:
        return "minus_pipe";
    case token_minus_pipe_equal:
        return "minus_pipe_equal";
    case token_asterisk:
        return "asterisk";
    case token_asterisk_equal:
        return "asterisk_equal";
    case token_asterisk_asterisk:
        return "asterisk_asterisk";
    case token_asterisk_percent:
        return "asterisk_percent";
    case token_asterisk_percent_equal:
        return "asterisk_percent_equal";
    case token_asterisk_pipe:
        return "asterisk_pipe";
    case token_asterisk_pipe_equal:
        return "asterisk_pipe_equal";
    case token_arrow:
        return "arrow";
    case token_colon:
        return "colon";
    case token_slash:
        return "slash";
    case token_slash_equal:
        return "slash_equal";
    case token_comma:
        return "comma";
    case token_ampersand:
        return "ampersand";
    case token_ampersand_equal:
        return "ampersand_equal";
    case token_question_mark:
        return "question_mark";
    case token_angle_bracket_left:
        return "angle_bracket_left";
    case token_angle_bracket_left_equal:
        return "angle_bracket_left_equal";
    case token_angle_bracket_angle_bracket_left:
        return "angle_bracket_angle_bracket_left";
    case token_angle_bracket_angle_bracket_left_equal:
        return "angle_bracket_angle_bracket_left_equal";
    case token_angle_bracket_angle_bracket_left_pipe:
        return "angle_bracket_angle_bracket_left_pipe";
    case token_angle_bracket_angle_bracket_left_pipe_equal:
        return "angle_bracket_angle_bracket_left_pipe_equal";
    case token_angle_bracket_right:
        return "angle_bracket_right";
    case token_angle_bracket_right_equal:
        return "angle_bracket_right_equal";
    case token_angle_bracket_angle_bracket_right:
        return "angle_bracket_angle_bracket_right";
    case token_angle_bracket_angle_bracket_right_equal:
        return "angle_bracket_angle_bracket_right_equal";
    case token_tilde:
        return "tilde";
    case token_number_literal:
        return "number_literal";
    case token_doc_comment:
        return "doc_comment";
    case token_container_doc_comment:
        return "container_doc_comment";
    case token_keyword_addrspace:
        return "keyword_addrspace";
    case token_keyword_align:
        return "keyword_align";
    case token_keyword_allowzero:
        return "keyword_allowzero";
    case token_keyword_and:
        return "keyword_and";
    case token_keyword_anyframe:
        return "keyword_anyframe";
    case token_keyword_anytype:
        return "keyword_anytype";
    case token_keyword_asm:
        return "keyword_asm";
    case token_keyword_break:
        return "keyword_break";
    case token_keyword_callconv:
        return "keyword_callconv";
    case token_keyword_catch:
        return "keyword_catch";
    case token_keyword_comptime:
        return "keyword_comptime";
    case token_keyword_const:
        return "keyword_const";
    case token_keyword_continue:
        return "keyword_continue";
    case token_keyword_defer:
        return "keyword_defer";
    case token_keyword_else:
        return "keyword_else";
    case token_keyword_enum:
        return "keyword_enum";
    case token_keyword_errdefer:
        return "keyword_errdefer";
    case token_keyword_error:
        return "keyword_error";
    case token_keyword_export:
        return "keyword_export";
    case token_keyword_extern:
        return "keyword_extern";
    case token_keyword_fn:
        return "keyword_fn";
    case token_keyword_for:
        return "keyword_for";
    case token_keyword_if:
        return "keyword_if";
    case token_keyword_inline:
        return "keyword_inline";
    case token_keyword_noalias:
        return "keyword_noalias";
    case token_keyword_noinline:
        return "keyword_noinline";
    case token_keyword_nosuspend:
        return "keyword_nosuspend";
    case token_keyword_opaque:
        return "keyword_opaque";
    case token_keyword_or:
        return "keyword_or";
    case token_keyword_orelse:
        return "keyword_orelse";
    case token_keyword_packed:
        return "keyword_packed";
    case token_keyword_pub:
        return "keyword_pub";
    case token_keyword_resume:
        return "keyword_resume";
    case token_keyword_return:
        return "keyword_return";
    case token_keyword_linksection:
        return "keyword_linksection";
    case token_keyword_struct:
        return "keyword_struct";
    case token_keyword_suspend:
        return "keyword_suspend";
    case token_keyword_switch:
        return "keyword_switch";
    case token_keyword_test:
        return "keyword_test";
    case token_keyword_threadlocal:
        return "keyword_threadlocal";
    case token_keyword_try:
        return "keyword_try";
    case token_keyword_union:
        return "keyword_union";
    case token_keyword_unreachable:
        return "keyword_unreachable";
    case token_keyword_var:
        return "keyword_var";
    case token_keyword_volatile:
        return "keyword_volatile";
    case token_keyword_while:
        return "keyword_while";
    }
}

typedef struct TokenLoc {
    uint32_t start;
    uint32_t end;
} TokenLoc;

typedef struct Token {
    TokenTag tag;
    TokenLoc loc;
} Token;

typedef struct Tokenizer {
    Buffer buffer;
    uint32_t index;
} Tokenizer;

typedef struct Keyword {
    const char *literal;
    TokenTag tag;
} Keyword;

static Keyword keywords[] = {
    { "addrspace", token_keyword_addrspace },
    { "align", token_keyword_align },
    { "allowzero", token_keyword_allowzero },
    { "and", token_keyword_and },
    { "anyframe", token_keyword_anyframe },
    { "anytype", token_keyword_anytype },
    { "asm", token_keyword_asm },
    { "break", token_keyword_break },
    { "callconv", token_keyword_callconv },
    { "catch", token_keyword_catch },
    { "comptime", token_keyword_comptime },
    { "const", token_keyword_const },
    { "continue", token_keyword_continue },
    { "defer", token_keyword_defer },
    { "else", token_keyword_else },
    { "enum", token_keyword_enum },
    { "errdefer", token_keyword_errdefer },
    { "error", token_keyword_error },
    { "export", token_keyword_export },
    { "extern", token_keyword_extern },
    { "fn", token_keyword_fn },
    { "for", token_keyword_for },
    { "if", token_keyword_if },
    { "inline", token_keyword_inline },
    { "noalias", token_keyword_noalias },
    { "noinline", token_keyword_noinline },
    { "nosuspend", token_keyword_nosuspend },
    { "opaque", token_keyword_opaque },
    { "or", token_keyword_or },
    { "orelse", token_keyword_orelse },
    { "packed", token_keyword_packed },
    { "pub", token_keyword_pub },
    { "resume", token_keyword_resume },
    { "return", token_keyword_return },
    { "linksection", token_keyword_linksection },
    { "struct", token_keyword_struct },
    { "suspend", token_keyword_suspend },
    { "switch", token_keyword_switch },
    { "test", token_keyword_test },
    { "threadlocal", token_keyword_threadlocal },
    { "try", token_keyword_try },
    { "union", token_keyword_union },
    { "unreachable", token_keyword_unreachable },
    { "var", token_keyword_var },
    { "volatile", token_keyword_volatile },
    { "while", token_keyword_while },
};

static int Tokenizer_getKeyword(Buffer b)
{
    size_t keywords_count = sizeof(keywords) / sizeof(Keyword);
    for (size_t i = 0; i < keywords_count; i++) {
        if (Buffer_eql(b, keywords[i].literal)) {
            return keywords[i].tag;
        }
    }
    return -1;
}


static void Tokenizer_init(Tokenizer *t, CompileContext *ctx, Buffer source)
{
    (void)ctx;

    t->buffer = source;
    t->index = (
        (unsigned char)t->buffer.data[0] == 0xef &&
        (unsigned char)t->buffer.data[1] == 0xbb &&
        (unsigned char)t->buffer.data[2] == 0xbf) ? 3 : 0; // UTF-8 BOM
}

static Token Tokenizer_next(Tokenizer *t)
{
    typedef enum TokenState {
        state_start,
        state_expect_newline,
        state_identifier,
        state_builtin,
        state_string_literal,
        state_string_literal_backslash,
        state_multiline_string_literal_line,
        state_char_literal,
        state_char_literal_backslash,
        state_backslash,
        state_equal,
        state_bang,
        state_pipe,
        state_minus,
        state_minus_percent,
        state_minus_pipe,
        state_asterisk,
        state_asterisk_percent,
        state_asterisk_pipe,
        state_slash,
        state_line_comment_start,
        state_line_comment,
        state_doc_comment_start,
        state_doc_comment,
        state_int,
        state_int_exponent,
        state_int_period,
        state_float,
        state_float_exponent,
        state_ampersand,
        state_caret,
        state_percent,
        state_plus,
        state_plus_percent,
        state_plus_pipe,
        state_angle_bracket_left,
        state_angle_bracket_angle_bracket_left,
        state_angle_bracket_angle_bracket_left_pipe,
        state_angle_bracket_right,
        state_angle_bracket_angle_bracket_right,
        state_period,
        state_period_2,
        state_period_asterisk,
        state_saw_at_sign,
        state_invalid,
    } TokenState;

    Token result = { .tag = token_invalid, .loc = { .start = t->index, .end = 0xaaaaaaaa } };
    TokenState state = state_start;

    #define CONTINUE(_state) do { state = (_state); goto switch_start; } while (0)
switch_start:
    switch (state) {
        case state_start:
            switch (t->buffer.data[t->index]) {
                case 0:
                    if (t->index == t->buffer.len) {
                        return (Token){
                            .tag = token_eof,
                            .loc = {
                                .start = t->index,
                                .end = t->index,
                            },
                        };
                    } else {
                        CONTINUE(state_invalid);
                    }

                case ' ':
                case '\n':
                case '\t':
                case '\r':
                    result.loc.start = ++t->index;
                    CONTINUE(state_start);

                case '"':
                    result.tag = token_string_literal;
                    CONTINUE(state_string_literal);

                case '\'':
                    result.tag = token_char_literal;
                    CONTINUE(state_char_literal);

                case 'a' ... 'z':
                case 'A' ... 'Z':
                case '_':
                    result.tag = token_identifier;
                    CONTINUE(state_identifier);

                case '@':
                    CONTINUE(state_saw_at_sign);

                case '=':
                    CONTINUE(state_equal);

                case '!':
                    CONTINUE(state_bang);

                case '|':
                    CONTINUE(state_pipe);

                case '(':
                    result.tag = token_l_paren;
                    t->index++;
                    break;

                case ')':
                    result.tag = token_r_paren;
                    t->index++;
                    break;

                case '[':
                    result.tag = token_l_bracket;
                    t->index++;
                    break;

                case ']':
                    result.tag = token_r_bracket;
                    t->index++;
                    break;

                case ';':
                    result.tag = token_semicolon;
                    t->index++;
                    break;

                case ',':
                    result.tag = token_comma;
                    t->index++;
                    break;

                case '?':
                    result.tag = token_question_mark;
                    t->index++;
                    break;

                case ':':
                    result.tag = token_colon;
                    t->index++;
                    break;

                case '%':
                    CONTINUE(state_percent);

                case '*':
                    CONTINUE(state_asterisk);

                case '+':
                    CONTINUE(state_plus);

                case '<':
                    CONTINUE(state_angle_bracket_left);

                case '>':
                    CONTINUE(state_angle_bracket_right);

                case '^':
                    CONTINUE(state_caret);

                case '\\':
                    result.tag = token_multiline_string_literal_line;
                    CONTINUE(state_backslash);

                case '{':
                    result.tag = token_l_brace;
                    t->index++;
                    break;

                case '}':
                    result.tag = token_r_brace;
                    t->index++;
                    break;

                case '~':
                    result.tag = token_tilde;
                    t->index++;
                    break;

                case '.':
                    CONTINUE(state_period);

                case '-':
                    CONTINUE(state_minus);

                case '/':
                    CONTINUE(state_slash);

                case '&':
                    CONTINUE(state_ampersand);

                case '0' ... '9':
                    result.tag = token_number_literal;
                    t->index++;
                    CONTINUE(state_int);

                default:
                    CONTINUE(state_invalid);
            }
            break;

        case state_expect_newline:
            switch (t->buffer.data[++t->index]) {
                case 0:
                    if (t->index == t->buffer.len) {
                        result.tag = token_invalid;
                    } else {
                        CONTINUE(state_invalid);
                    }
                    break;

                case '\n':
                    t->index++;
                    result.loc.start = t->index;
                    CONTINUE(state_start);

                default:
                    CONTINUE(state_invalid);
            }
            break;

        case state_invalid:
            switch (t->buffer.data[++t->index]) {
                case 0:
                    if (t->index == t->buffer.len) {
                        result.tag = token_invalid;
                    } else {
                        CONTINUE(state_invalid);
                    }
                    break;

                case '\n':
                    result.tag = token_invalid;
                    break;

                default:
                    CONTINUE(state_invalid);
            }
            break;

        case state_saw_at_sign:
            switch (t->buffer.data[++t->index]) {
                case 0:
                case '\n':
                    result.tag = token_invalid;
                    break;

                case '"':
                    result.tag = token_identifier;
                    CONTINUE(state_string_literal);

                case 'a' ... 'z':
                case 'A' ... 'Z':
                case '_':
                    result.tag = token_builtin;
                    CONTINUE(state_builtin);

                default:
                    CONTINUE(state_invalid);
            }
            break;

        case state_ampersand:
            switch (t->buffer.data[++t->index]) {
                case '=':
                    result.tag = token_ampersand_equal;
                    t->index++;
                    break;

                default:
                    result.tag = token_ampersand;
                    break;
            }
            break;

        case state_asterisk:
            switch (t->buffer.data[++t->index]) {
                case '=':
                    result.tag = token_asterisk_equal;
                    t->index++;
                    break;

                case '*':
                    result.tag = token_asterisk_asterisk;
                    t->index++;
                    break;

                case '%':
                    CONTINUE(state_asterisk_percent);

                case '|':
                    CONTINUE(state_asterisk_pipe);

                default:
                    result.tag = token_asterisk;
                    break;
            }
            break;

        case state_asterisk_percent:
            switch (t->buffer.data[++t->index]) {
                case '=':
                    result.tag = token_asterisk_percent_equal;
                    t->index++;
                    break;

                default:
                    result.tag = token_asterisk_percent;
                    break;
            }
            break;

        case state_asterisk_pipe:
            switch (t->buffer.data[++t->index]) {
                case '=':
                    result.tag = token_asterisk_pipe_equal;
                    t->index++;
                    break;

                default:
                    result.tag = token_asterisk_pipe;
                    break;
            }
            break;

        case state_percent:
            switch (t->buffer.data[++t->index]) {
                case '=':
                    result.tag = token_percent_equal;
                    t->index++;
                    break;

                default:
                    result.tag = token_percent;
                    break;
            }
            break;

        case state_plus:
            switch (t->buffer.data[++t->index]) {
                case '=':
                    result.tag = token_plus_equal;
                    t->index++;
                    break;

                case '+':
                    result.tag = token_plus_plus;
                    t->index++;
                    break;

                case '%':
                    CONTINUE(state_plus_percent);

                case '|':
                    CONTINUE(state_plus_pipe);

                default:
                    result.tag = token_plus;
                    break;
            }
            break;

        case state_plus_percent:
            switch (t->buffer.data[++t->index]) {
                case '=':
                    result.tag = token_plus_percent_equal;
                    t->index++;
                    break;

                default:
                    result.tag = token_plus_percent;
                    break;
            }
            break;

        case state_plus_pipe:
            switch (t->buffer.data[++t->index]) {
                case '=':
                    result.tag = token_plus_pipe_equal;
                    t->index++;
                    break;

                default:
                    result.tag = token_plus_pipe;
                    break;
            }
            break;

        case state_caret:
            switch (t->buffer.data[++t->index]) {
                case '=':
                    result.tag = token_caret_equal;
                    t->index++;
                    break;

                default:
                    result.tag = token_caret;
                    break;
            }
            break;

        case state_identifier:
            switch (t->buffer.data[++t->index]) {
                case 'a' ... 'z':
                case 'A' ... 'Z':
                case '_':
                case '0' ... '9':
                    CONTINUE(state_identifier);

                default:
                {
                    // use binary-search
                    Buffer ident = Buffer_slice(t->buffer, result.loc.start, t->index);
                    int kw = Tokenizer_getKeyword(ident);
                    if (kw >= 0) result.tag = (TokenTag)kw;
                }
            }
            break;

        case state_builtin:
            switch (t->buffer.data[++t->index]) {
                case 'a' ... 'z':
                case 'A' ... 'Z':
                case '_':
                case '0' ... '9':
                    CONTINUE(state_builtin);

                default:
                    break;
            }
            break;

        case state_backslash:
            switch (t->buffer.data[++t->index]) {
                case 0:
                case '\n':
                    result.tag = token_invalid;
                    break;

                case '\\':
                    CONTINUE(state_multiline_string_literal_line);

                default:
                    CONTINUE(state_invalid);
            }
            break;

        case state_string_literal:
            switch (t->buffer.data[++t->index]) {
                case 0:
                    if (t->index != t->buffer.len) {
                        CONTINUE(state_invalid);
                    } else {
                        result.tag = token_invalid;
                        break;
                    }

                case '\n':
                    result.tag = token_invalid;
                    break;

                case '\\':
                    CONTINUE(state_string_literal_backslash);

                case '"':
                    t->index++;
                    break;

                case 0x01 ... 0x09:
                case 0x0b ... 0x1f:
                case 0x7f:
                    CONTINUE(state_invalid);

                default:
                    CONTINUE(state_string_literal);
            }
            break;

        case state_string_literal_backslash:
            switch (t->buffer.data[++t->index]) {
                case 0:
                case '\n':
                    result.tag = token_invalid;
                    break;

                default:
                    CONTINUE(state_string_literal);
            }
            break;

        case state_char_literal:
            switch (t->buffer.data[++t->index]) {
                case 0:
                    if (t->index != t->buffer.len) {
                        CONTINUE(state_invalid);
                    } else {
                        result.tag = token_invalid;
                        break;
                    }

                case '\n':
                    result.tag = token_invalid;
                    break;

                case '\\':
                    CONTINUE(state_char_literal_backslash);

                case '\'':
                    t->index++;
                    break;

                case 0x01 ... 0x09:
                case 0x0b ... 0x1f:
                case 0x7f:
                    CONTINUE(state_invalid);

                default:
                    CONTINUE(state_char_literal);
            }
            break;

        case state_char_literal_backslash:
            switch (t->buffer.data[++t->index]) {
                case 0:
                    if (t->index != t->buffer.len) {
                        CONTINUE(state_invalid);
                    } else {
                        result.tag = token_invalid;
                        break;
                    }

                case '\n':
                    result.tag = token_invalid;
                    break;

                case 0x01 ... 0x09:
                case 0x0b ... 0x1f:
                case 0x7f:
                    CONTINUE(state_invalid);

                default:
                    CONTINUE(state_char_literal);
            }
            break;

        case state_multiline_string_literal_line:
            switch (t->buffer.data[++t->index]) {
                case 0:
                    if (t->index != t->buffer.len) {
                        CONTINUE(state_invalid);
                    }
                    break;

                case '\n':
                    break;

                case '\r':
                    if (t->buffer.data[t->index + 1] != '\n') {
                        CONTINUE(state_invalid);
                    }
                    break;

                case 0x01 ... 0x09:
                case 0x0b ... 0x0c:
                case 0x0e ... 0x1f:
                case 0x7f:
                    CONTINUE(state_invalid);

                default:
                    CONTINUE(state_multiline_string_literal_line);
            }
            break;

        case state_bang:
            switch (t->buffer.data[++t->index]) {
                case '=':
                    result.tag = token_bang_equal;
                    t->index++;
                    break;

                default:
                    result.tag = token_bang;
                    break;
            }
            break;

        case state_pipe:
            switch (t->buffer.data[++t->index]) {
                case '=':
                    result.tag = token_pipe_equal;
                    t->index++;
                    break;

                case '|':
                    result.tag = token_pipe_pipe;
                    t->index++;
                    break;

                default:
                    result.tag = token_pipe;
                    break;
            }
            break;

        case state_equal:
            switch (t->buffer.data[++t->index]) {
                case '=':
                    result.tag = token_equal_equal;
                    t->index++;
                    break;

                case '>':
                    result.tag = token_equal_angle_bracket_right;
                    t->index++;
                    break;

                default:
                    result.tag = token_equal;
                    break;
            }
            break;

        case state_minus:
            switch (t->buffer.data[++t->index]) {
                case '>':
                    result.tag = token_arrow;
                    t->index++;
                    break;

                case '=':
                    result.tag = token_minus_equal;
                    t->index++;
                    break;

                case '%':
                    CONTINUE(state_minus_percent);

                case '|':
                    CONTINUE(state_minus_pipe);

                default:
                    result.tag = token_minus;
                    break;
            }
            break;

        case state_minus_percent:
            switch (t->buffer.data[++t->index]) {
                case '=':
                    result.tag = token_minus_percent_equal;
                    t->index++;
                    break;

                default:
                    result.tag = token_minus_percent;
                    break;
            }
            break;

        case state_minus_pipe:
            switch (t->buffer.data[++t->index]) {
                case '=':
                    result.tag = token_minus_pipe_equal;
                    t->index++;
                    break;

                default:
                    result.tag = token_minus_pipe;
                    break;
            }
            break;

        case state_angle_bracket_left:
            switch (t->buffer.data[++t->index]) {
                case '<':
                    CONTINUE(state_angle_bracket_angle_bracket_left);

                case '=':
                    result.tag = token_angle_bracket_left_equal;
                    t->index++;
                    break;

                default:
                    result.tag = token_angle_bracket_left;
                    break;
            }
            break;

        case state_angle_bracket_angle_bracket_left:
            switch (t->buffer.data[++t->index]) {
                case '=':
                    result.tag = token_angle_bracket_angle_bracket_left_equal;
                    t->index++;
                    break;

                case '|':
                    CONTINUE(state_angle_bracket_angle_bracket_left_pipe);

                default:
                    result.tag = token_angle_bracket_angle_bracket_left;
                    break;
            }
            break;

        case state_angle_bracket_angle_bracket_left_pipe:
            switch (t->buffer.data[++t->index]) {
                case '=':
                    result.tag = token_angle_bracket_angle_bracket_left_pipe_equal;
                    t->index++;
                    break;

                default:
                    result.tag = token_angle_bracket_angle_bracket_left_pipe;
                    break;
            }
            break;

        case state_angle_bracket_right:
            switch (t->buffer.data[++t->index]) {
                case '>':
                    CONTINUE(state_angle_bracket_angle_bracket_right);

                case '=':
                    result.tag = token_angle_bracket_right_equal;
                    t->index++;
                    break;

                default:
                    result.tag = token_angle_bracket_right;
                    break;
            }
            break;

        case state_angle_bracket_angle_bracket_right:
            switch (t->buffer.data[++t->index]) {
                case '=':
                    result.tag = token_angle_bracket_angle_bracket_right_equal;
                    t->index++;
                    break;

                default:
                    result.tag = token_angle_bracket_angle_bracket_right;
                    break;
            }
            break;

        case state_period:
            switch (t->buffer.data[++t->index]) {
                case '.':
                    CONTINUE(state_period_2);

                case '*':
                    CONTINUE(state_period_asterisk);

                default:
                    result.tag = token_period;
                    break;
            }
            break;

        case state_period_2:
            switch (t->buffer.data[++t->index]) {
                case '.':
                    result.tag = token_ellipsis3;
                    t->index++;
                    break;

                default:
                    result.tag = token_ellipsis2;
                    break;
            }
            break;

        case state_period_asterisk:
            switch (t->buffer.data[++t->index]) {
                case '*':
                    result.tag = token_invalid_periodasterisks;
                    break;

                default:
                    result.tag = token_period_asterisk;
                    break;
            }
            break;

        case state_slash:
            switch (t->buffer.data[++t->index]) {
                case '/':
                    CONTINUE(state_line_comment_start);

                case '=':
                    result.tag = token_slash_equal;
                    t->index++;
                    break;

                default:
                    result.tag = token_slash;
                    break;
            }
            break;

        case state_line_comment_start:
            switch (t->buffer.data[++t->index]) {
                case 0:
                    if (t->index != t->buffer.len) {
                        CONTINUE(state_invalid);
                    } else {
                        return (Token){ .tag = token_eof, .loc = { .start = t->index, .end = t->index } };
                    }
                    break;

                case '!':
                    result.tag = token_container_doc_comment;
                    CONTINUE(state_doc_comment);

                case '\n':
                    result.loc.start = ++t->index;
                    CONTINUE(state_start);

                case '/':
                    CONTINUE(state_doc_comment_start);

                case '\r':
                    CONTINUE(state_expect_newline);

                case 0x01 ... 0x09:
                case 0x0b ... 0x0c:
                case 0x0e ... 0x1f:
                case 0x7f:
                    CONTINUE(state_invalid);

                default:
                    CONTINUE(state_line_comment);
            }
            break;

        case state_doc_comment_start:
            switch (t->buffer.data[++t->index]) {
                case 0:
                case '\n':
                    result.tag = token_doc_comment;
                    break;

                case '\r':
                    if (t->buffer.data[t->index + 1] != '\n') {
                        result.tag = token_doc_comment;
                    } else {
                        CONTINUE(state_invalid);
                    }
                    break;

                case '/':
                    CONTINUE(state_line_comment);

                case 0x01 ... 0x09:
                case 0x0b ... 0x0c:
                case 0x0e ... 0x1f:
                case 0x7f:
                    CONTINUE(state_invalid);

                default:
                    result.tag = token_doc_comment;
                    CONTINUE(state_doc_comment);
            }
            break;

        case state_line_comment:
            switch (t->buffer.data[++t->index]) {
                case 0:
                    if (t->index != t->buffer.len) {
                        CONTINUE(state_invalid);
                    } else {
                        return (Token){ .tag = token_eof, .loc = { .start = t->index, .end = t->index } };
                    }
                    break;

                case '\n':
                    result.loc.start = ++t->index;
                    CONTINUE(state_start);

                case '\r':
                    CONTINUE(state_expect_newline);

                case 0x01 ... 0x09:
                case 0x0b ... 0x0c:
                case 0x0e ... 0x1f:
                case 0x7f:
                    CONTINUE(state_invalid);

                default:
                    CONTINUE(state_line_comment);
            }
            break;

        case state_doc_comment:
            switch (t->buffer.data[++t->index]) {
                case 0:
                case '\n':
                    break;

                case '\r':
                    if (t->buffer.data[t->index + 1] != '\n') {
                        CONTINUE(state_invalid);
                    }
                    break;

                case 0x01 ... 0x09:
                case 0x0b ... 0x0c:
                case 0x0e ... 0x1f:
                case 0x7f:
                    CONTINUE(state_invalid);

                default:
                    CONTINUE(state_doc_comment);
            }
            break;

        case state_int:
            switch (t->buffer.data[t->index]) { // no increment
                case '.':
                    CONTINUE(state_int_period);

                case '_':
                case 'a' ... 'd':
                case 'f' ... 'o':
                case 'q' ... 'z':
                case 'A' ... 'D':
                case 'F' ... 'O':
                case 'Q' ... 'Z':
                case '0' ... '9':
                    t->index++;
                    CONTINUE(state_int);

                case 'e':
                case 'E':
                case 'p':
                case 'P':
                    CONTINUE(state_int_exponent);

                default:
                    break;
            }
            break;

        case state_int_exponent:
            switch (t->buffer.data[++t->index]) {
                case '-':
                case '+':
                    t->index++;
                    CONTINUE(state_float);

                default:
                    CONTINUE(state_int);
            }
            break;

        case state_int_period:
            switch (t->buffer.data[++t->index]) {
                case '_':
                case 'a' ... 'd':
                case 'f' ... 'o':
                case 'q' ... 'z':
                case 'A' ... 'D':
                case 'F' ... 'O':
                case 'Q' ... 'Z':
                case '0' ... '9':
                    t->index++;
                    CONTINUE(state_float);

                case 'e':
                case 'E':
                case 'p':
                case 'P':
                    CONTINUE(state_float_exponent);

                default:
                    t->index--;
            }
            break;

        case state_float:
            switch (t->buffer.data[t->index]) { // no increment
                case '_':
                case 'a' ... 'd':
                case 'f' ... 'o':
                case 'q' ... 'z':
                case 'A' ... 'D':
                case 'F' ... 'O':
                case 'Q' ... 'Z':
                case '0' ... '9':
                    t->index++;
                    CONTINUE(state_float);

                case 'e':
                case 'E':
                case 'p':
                case 'P':
                    CONTINUE(state_float_exponent);

                default:
                    break;
            }
            break;

        case state_float_exponent:
            switch (t->buffer.data[++t->index]) {
                case '-':
                case '+':
                    t->index++;
                    CONTINUE(state_float);

                default:
                    CONTINUE(state_float);
            }
            break;
    }
#undef CONTINUE

   result.loc.end = t->index;
   return result;
}
