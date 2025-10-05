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
static void TypeName##Array_append(TypeName##Array *a, Type tag)              \
{                                                                             \
    if (a->len + 1 >= a->cap) {                                               \
        a->cap *= 2;                                                          \
        Type *n = std_realloc(a->data, sizeof(Type) * a->cap);                \
        if (!n) std_panic("oom");                                             \
        a->data = n;                                                          \
    }                                                                         \
    a->data[a->len++] = tag;                                                  \
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