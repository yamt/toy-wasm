#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include "instance.h"
#include "report.h"
#include "type.h"
#include "xlog.h"

int
import_object_alloc(uint32_t nentries, struct import_object **resultp)
{
        struct import_object *im;

        im = zalloc(sizeof(*im));
        if (im == NULL) {
                return ENOMEM;
        }
        im->nentries = nentries;
        if (nentries > 0) {
                im->entries = zalloc(nentries * sizeof(*im->entries));
                if (im->entries == NULL) {
                        free(im);
                        return ENOMEM;
                }
        } else {
                im->entries = NULL;
        }
        *resultp = im;
        return 0;
}

/*
 * Note: an import_object created with this function is not
 * immutable because memory and table instances can grow.
 */
int
import_object_create_for_exports(struct instance *inst,
                                 const struct name *module_name,
                                 struct import_object **resultp)
{
        const struct module *m = inst->module;
        struct import_object *im;
        int ret;

        ret = import_object_alloc(m->nexports, &im);
        if (ret != 0) {
                return ret;
        }
        uint32_t i;
        for (i = 0; i < m->nexports; i++) {
                const struct export *ex = &m->exports[i];
                const struct exportdesc *d = &ex->desc;
                struct import_object_entry *e = &im->entries[i];
                switch (d->type) {
                case EXTERNTYPE_FUNC:
                        e->u.func = VEC_ELEM(inst->funcs, d->idx);
                        break;
                case EXTERNTYPE_TABLE:
                        e->u.table = VEC_ELEM(inst->tables, d->idx);
                        break;
                case EXTERNTYPE_MEMORY:
                        e->u.mem = VEC_ELEM(inst->mems, d->idx);
                        break;
                case EXTERNTYPE_GLOBAL:
                        e->u.global = VEC_ELEM(inst->globals, d->idx);
                        break;
                default:
                        assert(false);
                }
                e->type = d->type;
                e->module_name = module_name;
                e->name = &ex->name;
                xlog_trace("created an entry for %.*s:%.*s",
                           CSTR(e->module_name), CSTR(e->name));
        }
        im->next = NULL;
        *resultp = im;
        return 0;
}

void
import_object_destroy(struct import_object *im)
{
        if (im->dtor != NULL) {
                im->dtor(im);
        }
        free(im->entries);
        free(im);
}

int
import_object_find_entry(
        const struct import_object *impobj, const struct import *im,
        int (*check)(const struct import_object_entry *e, const void *arg),
        const void *checkarg, const struct import_object_entry **resultp,
        struct report *report)
{
        int result = ENOENT;
        size_t i;
        for (i = 0; i < impobj->nentries; i++) {
                const struct import_object_entry *e = &impobj->entries[i];
                if (!compare_name(e->module_name, &im->module_name) &&
                    !compare_name(e->name, &im->name)) {
                        if (e->type != im->desc.type) {
                                report_error(report,
                                             "Type mismatch for import "
                                             "%.*s:%.*s (%u != %u)",
                                             CSTR(&im->module_name),
                                             CSTR(&im->name),
                                             (unsigned int)e->type,
                                             (unsigned int)im->desc.type);
                                return EINVAL;
                        }
                        int ret = check(e, checkarg);
                        if (ret == 0) {
                                xlog_trace("Found an entry for import "
                                           "%.*s:%.*s",
                                           CSTR(&im->module_name),
                                           CSTR(&im->name));
                                *resultp = e;
                                return 0;
                        }
                        result = EINVAL;
                }
        }
        return result;
}
