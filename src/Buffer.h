typedef struct Buffer {
    char *data;
    uint32_t len;
} Buffer;

// e.g. printf("Buffer: "PRIb, Buffer(b));
#define PRIb "%.*s"
#define Buffer(b) (b).len, (b).data

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

static int Buffer_eqlBuffer(Buffer b, Buffer c)
{
    if (b.len != c.len) return -1;
    for (size_t i = 0; i < b.len; i++) {
        int d = c.data[i] - b.data[i];
        if (d != 0) return d;
    }
    return 0;
}

static Buffer Buffer_fromFile(const char *filename)
{
    long fsize;
    char *source = std_readFile(filename, &fsize);
    if (!source) std_panic("failed to read file: %s", filename);

    return (Buffer){ .data = source, .len = fsize };
}

// TODO: correctly handle base and prefix
static int64_t Buffer_toInt(Buffer b, int base)
{
    int64_t v = 0;
    for (uint32_t i = 0; i < b.len; i++) {
        v *= base;
        v += b.data[i] - '0';
    }
    return v;
}