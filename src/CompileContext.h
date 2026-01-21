// A generic compiler context intended to be available everywhere within the compiler.
//
// Right now, this context includes:
// - string interner
// - type interner/registry

typedef struct CompileContext CompileContext;

// String Interner.
//
// This is implemented as a simplistic hash-based linear scan.

typedef uint32_t sInternId;
typedef uint32_t tInternId;

#define ty_invalid_id (~(uint32_t)0)

typedef struct {
    uint64_t hash;
    Buffer buffer;
} sInternEntry;

DEFINE_ARRAY(sInternEntry);

typedef struct {
    sInternEntryArray entries;
} sIntern;

// Type Interner.

// assumed to contain less than 256 entries
typedef enum {
    // primitives
    ty_anyopaque,
    ty_bool,
    ty_u8,
    ty_i8,
    ty_u16,
    ty_i16,
    ty_u32,
    ty_i32,
    ty_usize,
    ty_isize,
    ty_u64,
    ty_i64,
    ty_u128,
    ty_i128,
    ty_c_char,
    ty_c_short,
    ty_c_ushort,
    ty_c_int,
    ty_c_uint,
    ty_c_long,
    ty_c_ulong,
    ty_c_longlong,
    ty_c_ulonglong,
    ty_f16,
    ty_f32,
    ty_f64,
    ty_f80,
    ty_c_longdouble,
    ty_f128,
    // complex
    ty_ptr_one,
    ty_ptr_two,
    ty_int,
    ty_complex,
} tTypeTag;

const char* tTypeTag_Name(tTypeTag tag)
{
    switch (tag) {
    case ty_anyopaque:
        return "void";
    case ty_bool:
        return "bool";
    case ty_u8:
        return "u8";
    case ty_u16:
        return "u16";
    case ty_u32:
        return "u32";
    case ty_u64:
        return "u64";
    case ty_u128:
        return "u128";
    case ty_i8:
        return "i8";
    case ty_i16:
        return "i16";
    case ty_i32:
        return "i32";
    case ty_i64:
        return "i64";
    case ty_i128:
        return "i128";
    case ty_isize:
        return "isize";
    case ty_usize:
        return "usize";
    case ty_c_char:
        return "c_char";
    case ty_c_short:
        return "c_short";
    case ty_c_ushort:
        return "c_ushort";
    case ty_c_int:
        return "c_int";
    case ty_c_uint:
        return "c_uint";
    case ty_c_long:
        return "c_long";
    case ty_c_ulong:
        return "c_ulong";
    case ty_c_longlong:
        return "c_longlong";
    case ty_c_ulonglong:
        return "c_ulonglong";
    case ty_c_longdouble:
        return "c_longdouble";
    case ty_f16:
        return "f16";
    case ty_f32:
        return "f32";
    case ty_f64:
        return "f64";
    case ty_f80:
        return "f80";
    case ty_f128:
        return "f128";
    case ty_ptr_one:
        return "*";
    case ty_ptr_two:
        return "**";
    case ty_int:
        return "<generic-bit-width-int>";
    case ty_complex:
        return "<complex-type>";
    }
}

typedef enum {
    pointer_modifier_const      = 0x01,
    pointer_modifier_volatile   = 0x02,
    pointer_modifier_allowzero  = 0x04,
} tTypePointerModifiers;

typedef union {
    struct { uint16_t bits; bool is_signed; } int_;
    struct { tInternId child; tTypePointerModifiers modifiers; } ptr;
} tTypeData;

typedef struct {
    tTypeTag tag;               // 8 bits
    tTypeData data;             // remaining
} tType;

typedef enum {
    class_int,
    class_float,
    class_ptr,
    class_other,
} tTypeInfoClass;

typedef struct {
    tTypeInfoClass class;
    uint16_t bits;
    bool is_signed;
} tTypeInfo;

static tTypeInfo tType_info(tType type)
{
    switch (type.tag) {
        case ty_u8:
            return (tTypeInfo){ .class = class_int, .bits = 8, .is_signed = false };
        case ty_u16:
            return (tTypeInfo){ .class = class_int, .bits = 16, .is_signed = false };
        case ty_u32:
            return (tTypeInfo){ .class = class_int, .bits = 32, .is_signed = false };
        case ty_u64:
            return (tTypeInfo){ .class = class_int, .bits = 64, .is_signed = false };
        case ty_u128:
            return (tTypeInfo){ .class = class_int, .bits = 128, .is_signed = false };
        case ty_i8:
            return (tTypeInfo){ .class = class_int, .bits = 8, .is_signed = true };
        case ty_i16:
            return (tTypeInfo){ .class = class_int, .bits = 16, .is_signed = true };
        case ty_i32:
            return (tTypeInfo){ .class = class_int, .bits = 32, .is_signed = true };
        case ty_i64:
            return (tTypeInfo){ .class = class_int, .bits = 64, .is_signed = true };
        case ty_i128:
            return (tTypeInfo){ .class = class_int, .bits = 128, .is_signed = true };
        case ty_isize:
            return (tTypeInfo){ .class = class_int, .bits = 64, .is_signed = true }; // platform-specific
        case ty_usize:
            return (tTypeInfo){ .class = class_int, .bits = 64, .is_signed = false }; // platform-specific
        case ty_c_char:
            return (tTypeInfo){ .class = class_int, .bits = 8, .is_signed = false }; // platform-specific
        case ty_c_short:
            return (tTypeInfo){ .class = class_int, .bits = 16, .is_signed = true }; // platform-specific
        case ty_c_ushort:
            return (tTypeInfo){ .class = class_int, .bits = 16, .is_signed = false }; // platform-specific
        case ty_c_int:
            return (tTypeInfo){ .class = class_int, .bits = 32, .is_signed = true }; // platform-specific
        case ty_c_uint:
            return (tTypeInfo){ .class = class_int, .bits = 32, .is_signed = false }; // platform-specific
        case ty_c_long:
            return (tTypeInfo){ .class = class_int, .bits = 64, .is_signed = true }; // platform-specific
        case ty_c_ulong:
            return (tTypeInfo){ .class = class_int, .bits = 64, .is_signed = false }; // platform-specific
        case ty_int:
            return (tTypeInfo){ .class = class_int, .bits = type.data.int_.bits, .is_signed = type.data.int_.is_signed };

        case ty_c_longdouble:
            return (tTypeInfo){ .class = class_float, .bits = 80, .is_signed = false }; // platform-specific
        case ty_f16:
            return (tTypeInfo){ .class = class_float, .bits = 16, .is_signed = false };
        case ty_f32:
            return (tTypeInfo){ .class = class_float, .bits = 32, .is_signed = false };
        case ty_f64:
            return (tTypeInfo){ .class = class_float, .bits = 64, .is_signed = false };
        case ty_f80:
            return (tTypeInfo){ .class = class_float, .bits = 80, .is_signed = false };
        case ty_f128:
            return (tTypeInfo){ .class = class_float, .bits = 128, .is_signed = false };

        case ty_ptr_one:
        case ty_ptr_two:
            return (tTypeInfo){ .class = class_ptr };

        default:
            return (tTypeInfo){ .class = class_other };
    }
}

static bool tType_eql(tType a, tType b)
{
    if (a.tag != b.tag) return false;
    switch (a.tag) {
        case ty_anyopaque:
        case ty_u8:
        case ty_u16:
        case ty_u32:
        case ty_u64:
        case ty_u128:
        case ty_i8:
        case ty_i16:
        case ty_i32:
        case ty_i64:
        case ty_i128:
        case ty_isize:
        case ty_usize:
        case ty_c_char:
        case ty_c_short:
        case ty_c_ushort:
        case ty_c_int:
        case ty_c_uint:
        case ty_c_long:
        case ty_c_ulong:
        case ty_c_longlong:
        case ty_c_ulonglong:
        case ty_c_longdouble:
        case ty_f16:
        case ty_f32:
        case ty_f64:
        case ty_f80:
        case ty_f128:
            return true;
        case ty_ptr_one:
        case ty_ptr_two:
            return a.data.ptr.modifiers == b.data.ptr.modifiers && a.data.ptr.child == b.data.ptr.child;
        case ty_int:
            return a.data.int_.bits == b.data.int_.bits && a.data.int_.is_signed == b.data.int_.is_signed;
        default:
            assume(false);
    }
}

typedef struct {
    uint64_t hash;
    tType ty;
} tInternEntry;

DEFINE_ARRAY(tInternEntry);

typedef struct {
    tInternEntryArray entries;
} tIntern;

struct CompileContext {
    sIntern strings;
    tIntern types;
};

// fnv-1a
static uint64_t CompileContext_hashString(Buffer b)
{
    uint64_t h = 1469598103934665603ull;
    for (uint32_t i = 0; i < b.len; i++) {
        h ^= (uint64_t) b.data[i];
        h *= 1099511628211ull;
    }
    return h;
}

static sInternId CompileContext_putString(CompileContext *ctx, Buffer buffer)
{
    uint64_t hash = CompileContext_hashString(buffer);
    for (uint32_t i = 0 ; i < ctx->strings.entries.len; i++) {
        sInternEntry e = ctx->strings.entries.data[i];
        if (e.hash != hash) continue;
        if (Buffer_eqlBuffer(e.buffer, buffer)) return (sInternId)i;
    }

    sInternEntry en = { .buffer = buffer, .hash = hash };
    return (sInternId)sInternEntryArray_append(&ctx->strings.entries, en);
}

static Buffer CompileContext_getString(CompileContext *ctx, sInternId id)
{
    return ctx->strings.entries.data[id].buffer;
}

static uint64_t CompileContext_hashType(tType ty)
{
    uint64_t hash = ty.tag;
    switch (ty.tag) {
        case ty_anyopaque:
        case ty_bool:
        case ty_u8:
        case ty_u16:
        case ty_u32:
        case ty_u64:
        case ty_u128:
        case ty_i8:
        case ty_i16:
        case ty_i32:
        case ty_i64:
        case ty_i128:
        case ty_isize:
        case ty_usize:
        case ty_c_char:
        case ty_c_short:
        case ty_c_ushort:
        case ty_c_int:
        case ty_c_uint:
        case ty_c_long:
        case ty_c_ulong:
        case ty_c_longlong:
        case ty_c_ulonglong:
        case ty_c_longdouble:
        case ty_f16:
        case ty_f32:
        case ty_f64:
        case ty_f80:
        case ty_f128:
            break;

        case ty_ptr_one:
        case ty_ptr_two:
            hash |= ((uint64_t) ty.data.ptr.modifiers) << 8;
            hash |= ((uint64_t) ty.data.ptr.child) << 16;
            break;

        case ty_int:
            hash |= ((uint64_t) ty.data.int_.is_signed) << 8;
            hash |= ((uint64_t) ty.data.int_.bits) << 16;
            break;

        default:
            assume(false);
    }
    return hash;
}

static tInternId CompileContext_putType(CompileContext *ctx, tType ty)
{
    uint64_t hash = CompileContext_hashType(ty);
    for (uint32_t i = 0 ; i < ctx->types.entries.len; i++) {
        tInternEntry e = ctx->types.entries.data[i];
        if (e.hash != hash) continue;
        if (tType_eql(e.ty, ty)) return (tInternId)i;
    }

    tInternEntry en = { .ty = ty, .hash = hash };
    return (tInternId)tInternEntryArray_append(&ctx->types.entries, en);
}

static tType CompileContext_getType(CompileContext *ctx, tInternId id)
{
    return ctx->types.entries.data[id].ty;
}

// Compile context.

static void CompileContext_init(CompileContext *ctx)
{
    sInternEntryArray_init(&ctx->strings.entries);
    CompileContext_putString(ctx, Buffer_empty()); // reserve empty buffer as id 0

    tInternEntryArray_init(&ctx->types.entries);
}