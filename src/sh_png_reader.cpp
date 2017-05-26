#include "../header/sh_png_reader.h"

uint32 sh_png_paeth_defilter(int32 a, int32 b, int32 c) {
    int32 p = a + b - c;
    int32 pa = sh_abs( p - a );
    int32 pb = sh_abs( p - b );
    int32 pc = sh_abs( p - c );

    if(pa <= pb && pa <= pc) return a;
    if(pb <= pc) return b;
    return c;
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

void sh_png_read_sig(uint8 *mem, sh_png_sig *sig) {
    *sig = *( (sh_png_sig *) mem );
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

sh_png_ihdr sh_png_read_ihdr(sh_png_chunk *chunk) {
    sh_png_ihdr ihdr = {};
    uint8 *data = chunk->data;
    
    ihdr.x = sh_get_uint32be(chunk->data);

    SKIP_BYTES(data, 4);
    ihdr.y = sh_get_uint32be(data);

    SKIP_BYTES(data, 4);
    ihdr.bit_depth = *((uint8 *) data);

    SKIP_BYTES(data, 1);
    ihdr.color_type = *((uint8 *) data);

    SKIP_BYTES(data, 1);
    ihdr.compression_method = *((uint8 *) data);;

    SKIP_BYTES(data, 1);
    ihdr.fitler_method = *((uint8 *) data);;

    SKIP_BYTES(data, 1);
    ihdr.interlance_method = *((uint8 *) data);;

    return ihdr;
}

sh_zlib_block sh_read_zlib_block(uint8 *stream, uint32 len) {
    sh_zlib_block zlib_block = {};
    zlib_block.cmf = *stream;
    stream++;
    zlib_block.flags = *stream;
    stream++;
    zlib_block.data = stream;
    SKIP_BYTES(stream, len - 2 );
    zlib_block.check_value = *((uint32 *) stream);
    return zlib_block;
}

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

    free(code_counts);
    free(next_codes);

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

uint8* sh_decompress_png_deflate(sh_zlib_block *zip) {
    sh_bits_buffer init_data = {zip->data, 0, 0};

    uint8 final = sh_read_bits(&init_data, 1);
    uint8 type = sh_read_bits(&init_data, 2);
    
    uint32 hlit = sh_read_bits(&init_data, 5) + 257;
    uint32 hdist = sh_read_bits(&init_data, 5) + 1;
    uint32 hclen = sh_read_bits(&init_data, 4) + 4;

    uint8 code_lengths_code_length[19];
    sh_memset(code_lengths_code_length, 0, 19);
    
    for(uint8 i = 0; i < hclen; ++i) {
        code_lengths_code_length[code_lengths_order[i]] = sh_read_bits(&init_data, 3);
    }

    uint32 *codes = sh_build_huffman_tree(code_lengths_code_length, 19);
    
    uint32 total_codes = hlit + hdist;
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

    //@Note(sharo): 32 bit unsigned integer because length of codes could be as big as 15 bits
    uint32 *main_codes_huff_tree = sh_build_huffman_tree(main_code_lengths, hlit);

    //@Note(sharo): distance tree comes after the main literal/length tree
    uint32 *distance_code_huff_tree = sh_build_huffman_tree(main_code_lengths + 286, hdist); 
    uint8  *distance_code_length = main_code_lengths + 286;

    uint8 *decompressed_data = (uint8 *) malloc(sizeof(uint8)*(1024*1024));
    uint32 data_index = 0;
    uint32 stop = 0;
    while(!stop) {
        uint32 decoded_value = sh_decode_huffman(&init_data, main_codes_huff_tree, main_code_lengths, hlit);
        sh_assert(decoded_value < 286);
        sh_assert(data_index < 1024*1024);
        
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

            uint32 distance_code = sh_decode_huffman(&init_data, distance_code_huff_tree, distance_code_length, hdist);
            uint32 distance_base = dist_bases[distance_code];
            uint32 dist_base_add = 0;
            if(dist_extra_bits[distance_code]) {
                dist_base_add = sh_read_bits(&init_data, dist_extra_bits[distance_code]);
            }

            distance_base += dist_base_add;

            uint32 back_pointer_index = data_index - distance_base;
            while(length_to_repeat--)  {
                sh_assert(data_index < 1024*1024);
                decompressed_data[data_index++]  = decompressed_data[back_pointer_index++];
            }
        }
    }

    uint8 *decompressed_fit_data = (uint8 *) malloc(sizeof(uint8)*data_index);
    sh_memcpy(decompressed_fit_data, decompressed_data, data_index); 
    
    free(decompressed_data);
    free(main_code_lengths);

    return decompressed_fit_data;
}


uint8* sh_defilter_png(uint8 *decompressed_png, sh_png_ihdr *ihdr) {
    uint32 x = ihdr->x;
    uint32 y = ihdr->y;
    uint32 byte_per_pixel = 1;//( ihdr->bit_depth );
    uint32 stride = x*byte_per_pixel;
    uint8 *row = decompressed_png;
    uint8 *unfiltered = (uint8 *) malloc(x*y*byte_per_pixel);
    uint8 *current_working = unfiltered;

    for(uint32 i = 0; i < y; ++i) {
        //@Note(sharo): filter is (one byte, potentially) before the scanline
        uint8 filter = *row++; 
        current_working = unfiltered + i*stride;
        
        switch(filter) {
            case sh_no_filter: {
                for(uint32 j = 0; j < stride; ++j) {
                    current_working[j] = row[j];
                }
            } break;

            case sh_sub_filter: {
                for(uint32 j = 0; j < stride; ++j) {
                    uint8 a = 0;
                    if(j != 0) {
                        a = current_working[j-1];
                    }
                    uint8 value = row[j] + (a);
                    current_working[j] = value;
                }
            } break;
            
            case sh_up_filter: {
                uint8 *prev_scanline = ( current_working - stride );
                for(uint32 j = 0; j < stride; ++j) {
                    uint8 b = prev_scanline[j];
                    uint8 value = row[j] + (b);
                    current_working[j] = value;
                }
            } break;

            case sh_avg_filter: {
                uint8 *prev_scanline = ( current_working - stride );
                for(uint32 j = 0; j < stride; ++j) {
                    uint8 a = 0;
                    uint8 b = prev_scanline[j];
                    
                    if(j != 0) {
                        a = current_working[j-1];
                    }

                    uint8 value = row[j] + ((a + b) >> 1);
                    current_working[j] = value;
                }
            } break;

            case sh_paeth_filter: {
                uint8 *prev_scanline = ( current_working - stride );
                for(uint32 j = 0; j < stride; ++j) {
                    int a = 0;
                    int b = prev_scanline[j];
                    int c = 0;
                    
                    if(j != 0) {
                        a = current_working[j-1];
                        c = prev_scanline[j-1];
                    }
                    
                    uint8 value = ( row[j] + sh_png_paeth_defilter(a, b, c) ) & 255;
                    current_working[j] = value;
                }

            } break;
        } 

        row += (stride);
        // current_working += (stride);
    }

    return unfiltered;
}

uint8* sh_load_png_mem(uint8 *mem, uint32 mem_size, int32 *x, int32 *y) {
   
    uint8 *memory = mem;
    
    sh_png_sig signature = {};
    sh_png_read_sig(memory, &signature);
    SKIP_BYTES(memory, 8);

    int32 chunk_nums = 2;
    sh_png_chunk *chunks = (sh_png_chunk *) malloc(sizeof(sh_png_chunk)*chunk_nums);

    int i = 0;
    sh_png_ihdr ihdr = {};
    uint8 *image = nullptr;

    for(int32 i = 0; i < chunk_nums; ++i) { 
        chunks[i] = sh_png_read_chunk(memory);
        
        if(sh_streq((char *)&chunks[i].head , "IHDR", 4)) {
            ihdr = sh_png_read_ihdr(chunks + i); 
        }

        if(sh_streq((char *) &chunks[i].head, "IDAT", 4)) {
            sh_zlib_block zlib_block = sh_read_zlib_block(chunks[i].data, chunks[i].len);
            uint8 *data = sh_decompress_png_deflate(&zlib_block);
            image = sh_defilter_png(data, &ihdr);
        }

        if(sh_streq((char *) &chunks[i].head , "IEND", 4)) {
            break;
        }

        SKIP_BYTES(memory, chunks[i].len + 4 + 4 + 4);
        // i++;
    }

    while(--chunk_nums >= 0) {
        free(chunks[chunk_nums].data);
    }

    free(chunks);

    *x = ihdr.x;
    *y = ihdr.y;

    return image;
}

uint8* sh_convert_one_to_four_chnl(uint8 *mem, uint32 bytes, uint32 x, uint32 y) {
    uint8 *four_channel = (uint8 *) malloc(x*y*4);
    uint32 *work = (uint32 *)four_channel;

    for(uint32 i = 0; i < y; ++i) {
        uint8 *row = mem + i*x;

        for(uint32 j = 0; j < x; ++j) {
            uint8 value = row[j];
            work[j] = (value << 24) | (value << 16) | (value << 8) | value;
        }

        work += x;
    }

    return four_channel;
}
