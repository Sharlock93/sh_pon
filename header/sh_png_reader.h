#define SKIP_BYTES(mem_pointer, byte_num) ( mem_pointer += byte_num );

void sh_load_png_mem(uint8 *mem, uint32 mem_size);
struct sh_png_sig  {
    uint8 sigs[8];
};

struct sh_png_chunk {
    uint32 len;
    uint8 head[4];
    uint8 *data;
    uint32 crc;
};

void sh_png_read_sig(uint8 *mem, sh_png_sig *sig) {
    *sig = *( (sh_png_sig *) mem );
}

void sh_read_mem_bige(uint8 *src, uint8 *into, uint32 byte_num) {
    while(byte_num-- > 0) {
        *( into + byte_num) = *src;
        ++src;
    }
}

uint32 sh_get_uint32be(uint8 *src) {
    uint32 result = 0;
    sh_read_mem_bige(src, (uint8 *) &result, sizeof(uint32));
    return result;
}

sh_png_chunk sh_png_read_chunk(uint8 *mem) {
    sh_png_chunk chnk = {};
    sh_read_mem_bige(mem, (uint8 *) &chnk.len, sizeof(uint32));

    SKIP_BYTES(mem, 4);

    *((uint32 *)&chnk.head) = *((uint32 *) mem);
    SKIP_BYTES(mem, 4);

    chnk.data = (uint8 *) malloc(sizeof(uint8)*chnk.len);
    sh_memcpy(chnk.data, mem, chnk.len);

    SKIP_BYTES(mem, chnk.len);
    
    *((uint32 *)&chnk.crc) = *((uint32 *) mem);
    
    return chnk;
}


struct sh_png_ihdr {
    uint32 x;
    uint32 y;
    uint8 bit_depth;
    uint8 color_type;
    uint8 compression_method;
    uint8 fitler_method;
    uint8 interlance_method;
};

struct sh_png_zlib {
    uint8 cmf;
    uint8 flags;
    uint8 *data;
    uint32 check_value;
};


sh_png_ihdr sh_png_read_head(sh_png_chunk *chunk) {
    sh_png_ihdr ihdr = {};
    
    ihdr.x = sh_get_uint32be(chunk->data);

    SKIP_BYTES(chunk->data, 4);
    ihdr.y = sh_get_uint32be(chunk->data);

    SKIP_BYTES(chunk->data, 4);
    ihdr.bit_depth = *((uint8 *) chunk->data);

    SKIP_BYTES(chunk->data, 1);
    ihdr.color_type = *((uint8 *) chunk->data);

    SKIP_BYTES(chunk->data, 1);
    ihdr.compression_method = *((uint8 *) chunk->data);;

    SKIP_BYTES(chunk->data, 1);
    ihdr.fitler_method = *((uint8 *) chunk->data);;

    SKIP_BYTES(chunk->data, 1);
    ihdr.interlance_method = *((uint8 *) chunk->data);;

    return ihdr;
}

sh_png_zlib sh_png_read_zlib_block(uint8 *stream, uint32 len) {
    sh_png_zlib zlib_block = {};
    zlib_block.cmf = *stream;
    stream++;
    zlib_block.flags = *stream;
    stream++;
    zlib_block.data = stream;
    SKIP_BYTES(stream, len - 2 );
    zlib_block.check_value = *((uint32 *) stream);
    return zlib_block;
}

struct sh_bits_buffer {
    uint8 *stream;
    uint32 buffer;
    uint8 bits_remains;
};


void sh_png_get_bits(sh_bits_buffer *buffer,  uint8 bits_needed) {
    uint8 extra_bits_required = ( buffer->bits_remains > bits_needed ) ? buffer->bits_remains - bits_needed : bits_needed - buffer->bits_remains ;
    uint8 num_bytes = (uint8) (extra_bits_required/8);
    uint8 bits_remaining = extra_bits_required%8;
    if(bits_remaining) num_bytes++;

    for(int i = 0; i < num_bytes; ++i) {
        uint32 byte = *buffer->stream++;
        buffer->buffer |= byte << (i * 8 + buffer->bits_remains);;
    }


    buffer->bits_remains += num_bytes*8;
}

uint32 sh_read_bits(sh_bits_buffer *data, uint8 num_bits) {

    uint32 result = 0;

    if(num_bits > data->bits_remains) {
        sh_png_get_bits(data, num_bits); 
    }

    for(uint32 i = 0; i < num_bits; ++i) {
        result |= data->buffer & (1 << i);
    }

    data->buffer >>= num_bits;
    data->bits_remains -= num_bits;

    return result;
}

uint32 sh_rread_bits_no_move(sh_bits_buffer *data, uint8 num_bits) {
    uint32 result = 0;

    if(num_bits > data->bits_remains) {
        sh_png_get_bits(data, num_bits); 
    }

    for(uint32 i = 0; i < num_bits; ++i) {
        result <<= 1;
        uint32 bit_set = data->buffer & (1 << i);
        result |= (bit_set > 0) ? 1 : 0;
    }

    return result;
}


uint8 code_lengths_order[] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

uint32* sh_build_huffman_tree(uint8 *code_bit_lengths, uint32 size_of_array) {
    uint32 max_bit_length = 0;
    for(uint32 i = 0; i < size_of_array; ++i) {
        if( code_bit_lengths[i] > max_bit_length ) {
            max_bit_length = code_bit_lengths[i];
        }
    }

    uint32 *code_counts = (uint32 *) malloc(sizeof(uint32)*( max_bit_length + 1 ));
    sh_memset((uint8 *) code_counts, 0, sizeof(uint32)*( max_bit_length + 1 ));

    for(uint32 i = 0; i < size_of_array; ++i) {
        code_counts[code_bit_lengths[i]]++;
    }

    code_counts[0] = 0;

    uint32 *next_codes = (uint32 *) malloc(sizeof(uint32)*( max_bit_length + 1 ));
    sh_memset((uint8 *) next_codes, 0, sizeof(uint32)*( max_bit_length + 1 ));
    uint32 code = 0;
    for(uint32 i = 1; i <= max_bit_length; ++i ) {
        code = ( code + code_counts[i-1] ) << 1;
        if(code_counts[i]) {
            next_codes[i] = code;
        }
    }

    uint32 *assigned_codes = (uint32 *) malloc(sizeof(uint32)*size_of_array);
    sh_memset((uint8 *) assigned_codes, 0, sizeof(uint32)*size_of_array);
    for(uint32 i = 0; i < size_of_array; ++i ) {
        if(code_bit_lengths[i]) {
            assigned_codes[i] = next_codes[code_bit_lengths[i]]++;
        }
    }

    return assigned_codes;

}

uint32 sh_decode_huffman(sh_bits_buffer *stream_data, uint32 *symbols_codes, uint8 *symbols_bit_length, uint32 symbol_count) {

    for(uint32 i = 0; i < symbol_count; ++i)  {
        if(symbols_bit_length[i] == 0) continue;
        uint32 bits = sh_rread_bits_no_move(stream_data, symbols_bit_length[i]);
        if(symbols_codes[i] == bits) {
            stream_data->buffer >>= symbols_bit_length[i];
            stream_data->bits_remains -= symbols_bit_length[i];
            return i;
        }
    }

    return 0;
}

uint8 extra_bits_to_read[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1,
    2, 2, 2, 2,
    3, 3, 3, 3,  
    4, 4, 4, 4, 
    5, 5, 5, 5,
    0
};

uint32 base_to_add_to[] = {
    3, 4, 5, 6, 7, 8, 9, 10,
    11, 13, 15, 17,
    19, 23, 27, 31,
    35, 43, 51, 59,
    67, 83, 99, 115,
    131, 163, 195, 227,
    258
};

uint32 dist_bases[] = {
/*0*/ 1, 2, 3, 4,
/*1*/ 5, 7,
/*2*/ 9, 13,
/*3*/ 17, 25,
/*4*/ 33, 49,
/*5*/ 65, 97,
/*6*/ 129, 193,
/*7*/ 257, 385,
/*8*/ 513, 769,
/*9*/ 1025, 1537,
/*10*/ 2049, 3073,
/*11*/ 4097, 6145,
/*12*/ 8193, 12289,
/*13*/ 16385, 24577
};

uint32 dist_extra_bits[] = {
/*0*/ 0, 0, 0, 0,
/*1*/ 1, 1,
/*2*/ 2, 2,
/*3*/ 3, 3,
/*4*/ 4, 4,
/*5*/ 5, 5,
/*6*/ 6, 6,
/*7*/ 7, 7,
/*8*/ 8, 8,
/*9*/ 9, 9,
/*10*/ 10, 10,
/*11*/ 11, 11,
/*12*/ 12, 12,
/*13*/ 13, 13
};



typedef enum {
    sh_no_filter,
    sh_sub_filter,
    sh_up_filter,
    sh_avg_filter,
    sh_paeth_filter
} sh_png_filter;

uint8* sh_decompress_png_deflate(sh_png_zlib *zip) {
    sh_bits_buffer init_data = {zip->data, 0, 0};
    uint8 final = sh_read_bits(&init_data, 1);
    uint8 type = sh_read_bits(&init_data, 2);
    
    uint32 hlit = sh_read_bits(&init_data, 5) + 257;
    uint32 hdist = sh_read_bits(&init_data, 5) + 1;
    uint32 hclen = sh_read_bits(&init_data, 4) + 4;

    uint32 total_codes = hlit + hdist;

    uint8 code_lengths_code_length[19];
    sh_memset(code_lengths_code_length, 0, 19);
    
    for(uint8 i = 0; i < hclen; ++i) {
        code_lengths_code_length[code_lengths_order[i]] = sh_read_bits(&init_data, 3);
    }

    uint32 *codes = sh_build_huffman_tree(code_lengths_code_length, 19);
    

    uint8 *main_code_lengths = (uint8 *) malloc(total_codes);
    sh_memset(main_code_lengths, 0, total_codes);

    uint32 code_index = 0;
    while(code_index < total_codes) {
        uint32 decoded_value = sh_decode_huffman(&init_data, codes, code_lengths_code_length, 19);

        if(decoded_value < 16) {
            main_code_lengths[code_index++] = decoded_value;
            continue;
        }
    
        //@Note(sharo): taken from stb_image.h writen by Sean Barret
        uint32 repeat_count = 0;
        uint8 code_to_repeat = 0;
        switch(decoded_value) {
            case 16: {
                 repeat_count = sh_read_bits(&init_data, 2) + 3;
                 code_to_repeat = main_code_lengths[code_index - 1];
            } break;
            
            case 17: {
                repeat_count = sh_read_bits(&init_data, 3) + 3;
            } break;

            case 18: { 
                repeat_count = sh_read_bits(&init_data, 7) + 11;
            } break;
        }

        sh_memset(main_code_lengths + code_index, code_to_repeat, repeat_count);
        code_index += repeat_count;
    }

    uint32 *main_codes_huff_tree = sh_build_huffman_tree(main_code_lengths, hlit);
    uint32 *distance_code_huff_tree = sh_build_huffman_tree(main_code_lengths + 286, hdist);

    uint8 *decompressed_data = (uint8 *) malloc(sizeof(uint8)*(1024*1024));
    uint32 data_index = 0;
    uint32 stop = 0;
    while(!stop) {
        uint32 decoded_value = sh_decode_huffman(&init_data, main_codes_huff_tree, main_code_lengths, hlit);
        assert(decoded_value < 286);

        if(decoded_value == 256) {
            stop = 1;
            continue;
        }

        if(decoded_value < 256) {
            decompressed_data[data_index++] = decoded_value;
            continue;
        }

        if(decoded_value > 256 && decoded_value < 286 ) {
            uint32 index_of_length = decoded_value - 257;
            uint32 length_to_repeat = base_to_add_to[index_of_length];
            uint32 add_to_base = 0;
            if(extra_bits_to_read[index_of_length]) {
                add_to_base = sh_read_bits(&init_data, extra_bits_to_read[index_of_length]);
            }

            length_to_repeat += add_to_base;

            uint32 distance_code = sh_decode_huffman(&init_data, distance_code_huff_tree, main_code_lengths + 286, hdist);
            uint32 distance_base = dist_bases[distance_code];
            uint32 dist_base_add = 0;
            if(dist_extra_bits[distance_code]) {
                dist_base_add = sh_read_bits(&init_data, dist_extra_bits[distance_code]);
            }

            distance_base += dist_base_add;

            uint32 back_pointer_index = data_index - distance_base;
            while(length_to_repeat--)  {
                decompressed_data[data_index++]  = decompressed_data[back_pointer_index++];
            }
        }
    };
                
    return decompressed_data;
}
