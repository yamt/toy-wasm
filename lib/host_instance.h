/*
 * These APIs are used by implementations of host functions.
 */

#include <stddef.h>

#include "platform.h"
#include "type.h"

struct host_func {
        struct name name;
        const char *type;
        host_func_t func;
};

#define HOST_FUNC_DECL(NAME)                                                  \
        int NAME(struct exec_context *ctx, struct host_instance *hi,          \
                 const struct functype *ft, const struct cell *params,        \
                 struct cell *results)

#define HOST_FUNC_PREFIX(FUNC_PREFIX, NAME, TYPE)                             \
        {                                                                     \
                .name = NAME_FROM_CSTR_LITERAL(#NAME),                        \
                .type = TYPE,                                                 \
                .func = FUNC_PREFIX##NAME,                                    \
        }

#define HOST_FUNC(NAME, FUNC, TYPE)                                           \
        {                                                                     \
                .name = NAME_FROM_CSTR_LITERAL(NAME),                         \
                .type = TYPE,                                                 \
                .func = FUNC,                                                 \
        }

#define HOST_FUNC_CONVERT_PARAMS(FT, PARAMS)                                  \
        struct val *converted_params =                                        \
                calloc((FT)->parameter.ntypes, sizeof(*converted_params));    \
        if (converted_params == NULL) {                                       \
                return ENOMEM;                                                \
        }                                                                     \
        vals_from_cells(converted_params, (PARAMS), &(FT)->parameter)
#define HOST_FUNC_PARAM(FT, PARAMS, IDX, TYPE) converted_params[IDX].u.TYPE
#define HOST_FUNC_FREE_CONVERTED_PARAMS() free(converted_params)

#define HOST_FUNC_RESULT_SET(FT, RESULTS, IDX, TYPE, V)                       \
        do {                                                                  \
                struct val tmp;                                               \
                tmp.u.TYPE = V;                                               \
                uint32_t csz;                                                 \
                uint32_t cidx =                                               \
                        resulttype_cellidx(&(FT)->result, (IDX), &csz);       \
                val_to_cells(&tmp, &(RESULTS)[cidx], csz);                    \
        } while (0)

#if defined(TOYWASM_ENABLE_TRACING)
#define HOST_FUNC_TRACE                                                       \
        do {                                                                  \
                xlog_trace("host func %s called", __func__);                  \
                host_func_dump_params(ft, params);                            \
        } while (0)
#else
#define HOST_FUNC_TRACE                                                       \
        do {                                                                  \
        } while (0)
#endif

/*
 * memory and func_table are only relevant to host functions
 * following the C pointer conventions. (eg. wasip1)
 */
struct host_instance {
        struct meminst *memory;
        struct tableinst *func_table;
};

#define host_func_memory(hi) (hi)->memory
#define host_func_func_table(hi) (hi)->func_table

struct host_module {
        const struct name *module_name;
        const struct host_func *funcs;
        size_t nfuncs;
};

__BEGIN_EXTERN_C

struct import_object;
struct mem_context;
int import_object_create_for_host_funcs(struct mem_context *mctx,
                                        const struct host_module *modules,
                                        size_t n, struct host_instance *hi,
                                        struct import_object **impp);

void host_func_dump_params(const struct functype *ft,
                           const struct cell *params);
int host_func_check_align(struct exec_context *ctx, uint32_t wasmaddr,
                          size_t align);
int host_func_copyout(struct exec_context *ctx, struct meminst *mem,
                      const void *hostaddr, uint32_t wasmaddr, size_t len,
                      size_t align);
int host_func_copyin(struct exec_context *ctx, struct meminst *mem,
                     void *hostaddr, uint32_t wasmaddr, size_t len,
                     size_t align);
int host_func_getptr(struct exec_context *ctx, struct meminst *mem,
                     uint32_t ptr, uint32_t size, void **pp);
int host_func_getptr2(struct exec_context *ctx, struct meminst *mem,
                      uint32_t ptr, uint32_t size, void **pp, bool *movedp);
int host_func_trap(struct exec_context *ctx, const char *fmt, ...)
        __printflike(2, 3);
struct restart_info;
int schedule_call_from_hostfunc(struct exec_context *ctx,
                                struct restart_info *restart,
                                const struct funcinst *func);

__END_EXTERN_C
