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
    if (sizeof(Node) != 64) std_panic("sizeof(Node) != 64: = %zu\n", sizeof(Node));

    if (argc < 2) {
        std_printf("tzc [-no-emit-bin|-tokens|-ast] -o <file> -lib <zig_lib_dir> <input>\n");
        std_exit(1);
    }

    Buffer source = Buffer_empty();
    const char *out_filename = NULL;
    const char *lib_dir = NULL;
    bool emit_tokens = false;
    bool emit_ast = false;
    bool no_emit_bin = false;
    bool report = false;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            if (source.len != 0) std_panic("multiple files provided\n");
            source = Buffer_fromFile(argv[i]);
        } else if (strequal(argv[i], "-lib")) {
            if (++i >= argc) std_panic("missing parameter for -lib\n");
            lib_dir = argv[i];
        } else if (strequal(argv[i], "-o")) {
            if (++i >= argc) std_panic("missing parameter for -o\n");
            out_filename = argv[i];
        } else if (strequal(argv[i], "-tokens")) {
            emit_tokens = true;
        } else if (strequal(argv[i], "-ast")) {
            emit_ast = true;
        } else if (strequal(argv[i], "-report")) {
            report = true;
        } else if (strequal(argv[i], "-no-emit-bin")) {
            no_emit_bin = true;
        } else {
            std_panic("unknown option '%s'\n", argv[i]);
        }
    }
    if (!no_emit_bin && !lib_dir) std_panic("-lib <zig_lib_dir> is required\n");
    if (!no_emit_bin && !out_filename) std_panic("-o <file> is required\n"); // just append .c to input file

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
            std_printf("|%u: %s: "PRIb"\n", i, TokenTag_name(token.tag), Buffer(Buffer_slice(source, token.loc.start, token.loc.end)));
        }
        return 0;
    }

    Parser p;
    Parser_init(&p, source, tokens.data, tokens.len);
    Node *root = Parser_parse(&p);

    if (report) {
        std_printf("tokens: size=%2.fKiB, count=%zu\n", (float) tokens.len * sizeof(Token) / 1024, tokens.len);
        std_printf(" nodes: size=%2.fKiB\n", (float) p.nodes_count * sizeof(Node) / 1024);
    }

    if (emit_ast) {
        AstRenderer r;
        AstRenderer_init(&r, &p);
        AstRenderer_render(&r, root);
        return 0;
    }

    if (!no_emit_bin) {
        CodeGen cg;
        CodeGen_init(&cg, &p, out_filename, lib_dir);
        CodeGen_gen(&cg, root);
    }
}
