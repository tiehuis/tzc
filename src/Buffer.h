typedef struct Buffer {
    char *data;
    uint32_t len;
} Buffer;

static Buffer Buffer_empty(void)
{
    return (Buffer){ .data = (void*)0xaaaaaaaa, .len = 0 };
}

static Buffer Buffer_slice(Buffer parent, uint32_t start, uint32_t end)
{
    return (Buffer){ .data = parent.data + start, .len = end - start };
}

static int Buffer_eql(Buffer b, const char *s)
{
    for (size_t i = 0; i < b.len; i++) {
        int d = s[i] - b.data[i];
        if (d != 0) return d;
    }
    return s[b.len] != 0;
}

static const char* Buffer_staticZ(Buffer b)
{
    static char s[1024];
    for (size_t i = 0; i < b.len; i++) {
        s[i] = b.data[i];
    }
    s[b.len] = 0;
    return s;
}

static Buffer Buffer_fromFile(const char *filename)
{
    long fsize;
    char *source = std_readFile(filename, &fsize);
    if (!source) std_panic("failed to read file: %s", filename);

    return (Buffer){ .data = source, .len = fsize };
}