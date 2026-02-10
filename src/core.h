// Array defines a typed-contiguous array.
#define DEFINE_ARRAY(Type) DEFINE_ARRAY_NAMED(Type, Type)
#define DEFINE_ARRAY_NAMED(Type, TypeName)                                    \
typedef struct TypeName##Array {                                              \
    Type *data;                                                               \
    uint32_t len;                                                             \
    uint32_t cap;                                                             \
} TypeName##Array;                                                            \
                                                                              \
static void TypeName##Array_init(TypeName##Array *a)                          \
{                                                                             \
    a->len = 0;                                                               \
    a->cap = 8;                                                               \
    a->data = std_malloc(sizeof(Type) * a->cap);                              \
    if (!a->data) std_panic("oom");                                           \
}                                                                             \
static uint32_t TypeName##Array_append(TypeName##Array *a, Type tag)          \
{                                                                             \
    if (a->len + 1 >= a->cap) {                                               \
        a->cap *= 2;                                                          \
        Type *n = std_realloc(a->data, sizeof(Type) * a->cap);                \
        if (!n) std_panic("oom");                                             \
        a->data = n;                                                          \
    }                                                                         \
    uint32_t id = a->len++;                                                   \
    a->data[id] = tag;                                                        \
    return id;                                                                \
}                                                                             \
__attribute__((unused))                                                       \
static void TypeName##Array_appendMany(TypeName##Array *a, Type *tags, size_t tags_len)   \
{                                                                             \
    while (a->len + tags_len >= a->cap) {                                     \
        a->cap *= 2;                                                          \
        Type *n = std_realloc(a->data, sizeof(Type) * a->cap);                \
        if (!n) std_panic("oom");                                             \
        a->data = n;                                                          \
    }                                                                         \
    for (size_t i = 0; i < tags_len; i++) {                                   \
        a->data[a->len++] = tags[i];                                          \
    }                                                                         \
}

// Buffer contains a null-terminated string along with its length.
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

static bool Buffer_eql(Buffer b, const char *s)
{
    for (size_t i = 0; i < b.len; i++) {
        int d = s[i] - b.data[i];
        if (d != 0) return false;
    }
    return s[b.len] == 0;
}

static bool Buffer_eqlBuffer(Buffer b, Buffer c)
{
    if (b.len != c.len) return false;
    for (size_t i = 0; i < b.len; i++) {
        int d = c.data[i] - b.data[i];
        if (d != 0) return false;
    }
    return true;
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