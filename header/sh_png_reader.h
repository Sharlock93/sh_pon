/*@Todo(sharo):
 * support more than 8 bit depth
 * bounding check of the inputs, i.e: check the stream for falling of track
 * must be able to read multiple blocks 
 * use 16 bit integer for optimization, standard says code length cannot be bigger than 15 bits
 * take care of not knowing the size of the decompressed data before hand
 * fix the memory leaks, i.e: check where there is malloc and no free
*/

#define SKIP_BYTES(mem_pointer, byte_num) ( mem_pointer += byte_num );

struct sh_png_sig  {
    uint8 sigs[8];
};

struct sh_png_chunk {
    uint32 len;
    uint8 head[4];
    uint8 *data;
    uint32 crc;
};

struct sh_png_ihdr {
    uint32 x;
    uint32 y;
    uint8 bit_depth;
    uint8 color_type;
    uint8 compression_method;
    uint8 fitler_method;
    uint8 interlance_method;
};

struct sh_zlib_block {
    uint8 cmf;
    uint8 flags;
    uint8 *data;
    uint32 check_value;
};

struct sh_png_datastream {
    uint8 *data;
    uint32 size;
};

struct sh_bits_buffer {
    uint8 *stream;
    uint32 buffer;
    uint8 bits_remains;
};

typedef enum {
    sh_no_filter,
    sh_sub_filter,
    sh_up_filter,
    sh_avg_filter,
    sh_paeth_filter
} sh_png_filter;

uint8 code_lengths_order[] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

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


uint32 sh_png_paeth_defilter(int32 a, int32 b, int32 c);
void sh_read_mem_bige(uint8 *src, uint8 *into, uint32 byte_num);
uint32 sh_get_uint32be(uint8 *src);

void sh_png_get_bits(sh_bits_buffer *buffer,  uint8 bits_needed);
uint32 sh_read_bits(sh_bits_buffer *data, uint8 num_bits);
uint32 sh_rread_bits_no_move(sh_bits_buffer *data, uint8 num_bits);

void sh_png_read_sig(uint8 *mem, sh_png_sig *sig);
sh_png_chunk sh_png_read_chunk(uint8 *mem);
sh_png_ihdr sh_png_read_ihdr(sh_png_chunk *chunk);

sh_zlib_block sh_read_zlib_block(uint8 *stream, uint32 len);
uint32* sh_build_huffman_tree(uint8 *code_bit_lengths, uint32 size_of_array);
uint32 sh_decode_huffman(sh_bits_buffer *stream_data, uint32 *symbols_codes, uint8 *symbols_bit_length, uint32 symbol_count);
uint8* sh_decompress_png_deflate(sh_png_datastream *data);

uint8* sh_defilter_png(uint8 *decompressed_png, sh_png_ihdr *ihdr);
void sh_load_png_mem(uint8 *mem, uint32 mem_size);
uint8* sh_convert_one_to_four_chnl(uint8 *img, uint32 byte_size, uint32 x, uint32 y);
