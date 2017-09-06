void dumpstruct(void *dumped_strct, struct_meta_info *meta_info, int meta_info_count) {

        for(int i = 0; i < meta_info_count; ++i) {
                char *m = ( (char *)dumped_strct ) + meta_info[i].offset;

                switch (meta_info[i].type) {
                        case type_int:
                                printf("val for %s is %d\n", meta_info[i].name, *(int *)m);
                                break;
                                
                }
        }
}

