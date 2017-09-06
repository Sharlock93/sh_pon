#ifndef META_INFO_H
#define META_INFO_H
#include "sh_meta_types.h"

enum {
        meta_pointer = 0b01
};

struct struct_meta_info {
        int flags;
        member_type type;
        char *name;
        size_t offset;
};
#endif
