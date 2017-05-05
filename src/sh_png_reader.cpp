void sh_load_png_mem(uint8 *mem, uint32 mem_size) {
   
    uint8 *memory = mem;
    // sh_png_img image = {};
    sh_png_sig signature = {};
    sh_png_read_sig(memory, &signature);
    SKIP_BYTES(memory, 8);

    sh_png_chunk *chunks = (sh_png_chunk *) malloc(sizeof(sh_png_chunk)*2);

    int i = 0;
    sh_png_ihdr ihdr;
    uint8 *image = nullptr;

    for(;;) { 
        chunks[i] = sh_png_read_chunk(memory);
        
        if(sh_streq((char *)&chunks[i].head , "IHDR", 4)) {
            ihdr = sh_png_read_head(chunks + i); 
        }

        if(sh_streq((char *) &chunks[i].head, "IDAT", 4)) {
            sh_png_zlib zlib_block = sh_png_read_zlib_block(chunks[i].data, chunks[i].len);
            uint8 *data = sh_decompress_png_deflate(&zlib_block);
            image = sh_defilter_png(data, &ihdr);
        }

        if(sh_streq((char *) &chunks[i].head , "IEND", 4)) {
            break;
        }

        SKIP_BYTES(memory, chunks[i].len + 4 + 4 + 4);
        i++;
    }

}

