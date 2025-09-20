#include "os.h"

#include "Array.h"
#include "Buffer.h"
#include "Tokenizer.h"
#include "Parser.h"
#include "AstRenderer.h"
#include "CodeGen.h"

bool strequal(const char *a, const char *b)
{
    while (*a && *b) if (*a++ != *b++) return false;
    return *a == 0 && *b == 0;
}

DEFINE_ARRAY(Token)

int main(int argc, char **argv)
{
    if (argc < 2) {
        std_printf("tzc [-tokens|-ast|-o <output>] <input>\n");
        std_exit(1);
    }

    Buffer source = Buffer_empty();
    const char *output_filename = "tzc_output.c";
    bool emit_tokens = false;
    bool emit_ast = false;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            if (source.len != 0) std_panic("multiple files provided\n");
            source = Buffer_fromFile(argv[i]);
        } else if (strequal(argv[i], "-o")) {
            if (++i >= argc) std_panic("missing parameter for -o\n");
            output_filename = argv[i];
        } else if (strequal(argv[i], "-tokens")) {
            emit_tokens = true;
        } else if (strequal(argv[i], "-ast")) {
            emit_ast = true;
        } else {
            std_panic("unknown option '%s'\n", argv[i]);
        }
    }

    TokenArray tokens;
    TokenArray_init(&tokens);

    Tokenizer t;
    Tokenizer_init(&t, source);
    while (true) {
        Token token = Tokenizer_next(&t);
        TokenArray_append(&tokens, token);
        if (token.tag == token_eof || token.tag == token_invalid) break;
    }
    if (emit_tokens) {
        for (uint32_t i = 0; i < tokens.len; i++) {
            Token token = tokens.data[i];
            std_printf("|%u: %s: %s\n", i, TokenTag_name(token.tag), Buffer_staticZ(Buffer_slice(source, token.loc.start, token.loc.end)));
        }
        std_exit(0);
    }

    Parser p;
    Parser_init(&p, source, tokens.data, tokens.len);
    Node *root = Parser_parse(&p);

    if (emit_ast) {
        AstRenderer r;
        AstRenderer_init(&r, &p);
        AstRenderer_render(&r, root);
        std_exit(0);
    }

    CodeGen cg;
    CodeGen_init(&cg, output_filename);
    CodeGen_gen(&cg, root);
}
