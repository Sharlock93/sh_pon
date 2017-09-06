#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

enum sh_token_type {
        sh_open_paran, // (
        sh_close_paran, // )
        sh_open_brak, // [
        sh_close_brak, // ]
        sh_open_brace, // {
        sh_close_brace, // }
        sh_single_quote, // '
        sh_double_quote, // "
        sh_colon, // :
        sh_asterisk, // *
        sh_semicolon, // ;

        sh_comment,
        sh_identifier,
        sh_unknown,
        sh_end_of_stream
};

struct stream_data {
        char *cur;
        int read_so_far;
        int stream_length;
};

struct sh_token {
        sh_token_type type;
        char *pos;
        int length;
};

struct sh_token_node {
        sh_token token;
        sh_token_node *next;
};

struct sh_expr {
        sh_token_node *tok_head;
        sh_token_node *tok_tail;
        int token_number;
};

struct struct_types_seen {
        char *name;
        struct_types_seen *next;
};


struct types_seen {
        char *name;
        types_seen *next;
};



types_seen *first_type = nullptr;
struct_types_seen *first_struct_type = nullptr;
FILE *sh_generated = nullptr;

bool type_seen_already(sh_token *struct_token) {
        bool result = false;
        for(types_seen *temp = first_type; temp; temp = temp->next) {
                if(strncmp(temp->name,  struct_token->pos, strlen(temp->name)) == 0) return true;
        }
        return result;
}
//
// bool sh_strcmp(char *str1, char *str2) {
//         while(str1[0] != '\0' && str2[0] != '\0' && (str1[0] == str2[0])){
//                 ++str1;
//                 ++str2;
//         }
//
//         if(str1[0] == '\0')
// }
//
void add_tok_to_expr(sh_expr *expr, sh_token tok) {
        sh_token_node *new_node = (sh_token_node *) malloc(sizeof(sh_token_node));
        new_node->token = tok;
        new_node->next = NULL;

        if(expr->tok_head == NULL && expr->tok_tail == NULL) {
                expr->tok_head = expr->tok_tail = new_node;
        } else {
                if(expr->tok_tail->next == NULL) {
                        expr->tok_tail->next = new_node;
                        expr->tok_tail = new_node;
                }
        }
}

bool struct_type_seen_already(sh_token *struct_token) {
        bool result = false;
        for(struct_types_seen *temp = first_struct_type; temp; temp = temp->next) {
                if(strncmp(struct_token->pos, temp->name, strlen(temp->name)) == 0) return true;
        }
        return result;
}

void add_new_type(sh_token *token) {
        types_seen *n = (types_seen *) malloc(sizeof(types_seen));
        n->name = (char *) malloc(token->length+1);
        memcpy(n->name, token->pos, token->length);
        n->name[token->length] = 0;
        n->next = first_type;
        first_type = n;
}

void add_new_struct_type(sh_token *token) {
        struct_types_seen *n = (struct_types_seen *) malloc(sizeof(struct_types_seen));
        n->name = (char *) malloc(token->length+1);
        memcpy(n->name, token->pos, token->length);
        n->name[token->length] = 0;
        n->next = first_struct_type;
        first_struct_type = n;
}

char* sh_readfile(char *file_name, stream_data *stream) {
        FILE *file = fopen(file_name, "r");
        char *content = nullptr;
        if(file) {
                fseek(file, 0, SEEK_END);
                int size = ftell(file);
                stream->stream_length = size;
                fseek(file, 0, SEEK_SET);
                content = (char*) malloc(sizeof(char)*( size + 1 ));
                fread(content, size, 1, file);
                content[size] = 0;
                fclose(file);
        }

        return content;
}

int is_end_of_stream(stream_data *stream) {
        int result = ((stream->cur[0] == '\0') ||
                        (stream->read_so_far >= stream->stream_length));
        return result;
}

void move_stream_one(stream_data *stream) {
        assert(stream->read_so_far < stream->stream_length);
        ++stream->cur;
        ++stream->read_so_far;
}

int is_at_whitespace(stream_data *stream) {
        char c = *stream->cur;
        int result = ( (c == ' ')  ||
                        (c == '\t') ||
                        (c == '\n') ||
                        (c == '\r') );

        return result;
}

int is_alpha(char c) {
        if(( c >= 'a' && c <= 'z' ) || (c >= 'A' && c <= 'Z')) return 1;
        return 0;
}

int is_number(char c) {
        if(c >= '0' && c <= '9') return 1;
        return 0;
}

sh_token next_token(stream_data *stream) {
        while(is_at_whitespace(stream)) move_stream_one(stream);

        if(is_end_of_stream(stream)) {

                sh_token t = {sh_end_of_stream, stream->cur, 0};
                return t;
        }

        sh_token token = {};
        token.length = 1;
        token.pos = stream->cur;
        move_stream_one(stream);
        switch(*token.pos) {
                case '[': token.type = sh_open_brak; break;
                case ']': token.type = sh_close_brak; break;
                case '(': token.type = sh_open_paran; break;
                case ')': token.type = sh_close_paran; break;
                case '{': token.type = sh_open_brace; break;
                case '}': token.type = sh_close_brace; break;
                case ';': token.type = sh_semicolon; break;
                case ':': token.type = sh_colon; break;
                case '*': token.type = sh_asterisk; break;
                case '\'': {
                        token.type = sh_single_quote;
                        ++token.pos;

                        //empty single quote
                        if(stream->cur[0] == '\'') {
                                token.length = 0;
                        }

                        //escape seqence
                        if(stream->cur[0] == '\\') {
                                token.length = 2;
                                move_stream_one(stream);
                                move_stream_one(stream);
                        }

                        assert(stream->cur[0] == '\'');
                        move_stream_one(stream);
                } break;
                case '"': {
                        token.type = sh_double_quote;
                        token.pos++;
                        while(stream->cur[0] != '"' &&
                                        stream->cur[0] != 0)
                        {
                                if(stream->cur[0] == '\\' && stream->cur[1] != 0) {
                                        move_stream_one(stream);
                                }

                                move_stream_one(stream);
                        }

                        token.length = (int) (stream->cur - token.pos);
                        if(stream->cur[0] == '"') move_stream_one(stream);
                } break; 
                default:
                        if(is_alpha(token.pos[0]) || token.pos[0] == '_') {
                                token.type = sh_identifier;
                                while(is_alpha(stream->cur[0]) ||
                                                is_number(stream->cur[0]) ||
                                                stream->cur[0] == '_')
                                {
                                        move_stream_one(stream);
                                }

                                token.length = stream->cur - token.pos;
                        } else if(( token.pos[0] == '/' && token.pos[1] && token.pos[1] == '/' ) ||
                                        (token.pos[0] == '/' && token.pos[1] && token.pos[1] == '*'))
                        {
                                token.type = sh_comment;
                                if(token.pos[1] == '*') {
                                        while(stream->cur[0]) {
                                                move_stream_one(stream);
                                                if(stream->cur[0] && stream->cur[0] == '*' && stream->cur[1] && stream->cur[1] == '/') break;
                                        }

                                        token.length = stream->cur - token.pos;
                                        move_stream_one(stream);
                                        move_stream_one(stream);
                                }

                                if(token.pos[1] == '/') {
                                        while(stream->cur[0] && stream->cur[0] != '\n') move_stream_one(stream);
                                        token.length = stream->cur - token.pos;
                                        // move_stream_one(stream);
                                }
                        }

                        break;
        }

        return token;
}

bool token_equals(char *token_text, char *matched) {
        while(token_text[0] != 0 && matched[0] != 0) {
                if(token_text[0] != matched[0]) {
                        return false;
                }
                ++token_text;
                ++matched;
        }

        if(matched[0] == 0) return true;
        return false;
}

void unread_token(stream_data *stream, sh_token *tok) {
        stream->cur -= tok->length;   
        stream->read_so_far -= tok->length;
}

bool sh_expect_token(stream_data *stream, sh_token_type type, char *token_text = nullptr) {
        sh_token tok = next_token(stream);
        bool result = tok.type == type;
        if(type == sh_identifier && token_text != NULL) {
                result = result && token_equals(tok.pos, token_text);
        }

        if(!result) {
                unread_token(stream, &tok);
        }

        return result;
}

sh_token_type peak_next_token(stream_data *stream) {
        sh_token tok = next_token(stream);
        unread_token(stream, &tok);
        return tok.type;
}

void collect_to_semi_colon(stream_data *stream, sh_expr *expr) {
        int stop = 0;
        while(stop == 0) { 
                sh_token tok = next_token(stream);
                if(tok.type == sh_end_of_stream) {
                        // printf("we done fucked up");
                        break;
                }

                if(tok.type == sh_semicolon || tok.type == sh_colon) {
                        stop = 1;
                }

                add_tok_to_expr(expr, tok);
                expr->token_number++;
        }
}

void output_expr(sh_expr *expr) {
        sh_token_node *head = expr->tok_head;
        do {
                sh_token *t = &head->token;
                printf("%.*s ", t->length, t->pos);
                head = head->next;
        }while(head); 

        printf("\n");
}


// void sh_struct_member_parse(stream_data *stream, sh_token struct_mbr_type, sh_token struct_name) {
//         bool member_end = 0;
//         if(!type_seen_already(&struct_mbr_type)) {
//                 add_new_type(&struct_mbr_type);
//         }
//         while(!member_end) {
//                 sh_token member_property = next_token(stream);
//                 bool is_member_pointer = false;
//                 int flags = 0;
//                 while(member_property.type == sh_asterisk)  {
//                         is_member_pointer = true;
//                         member_property = next_token(stream);
//                 }
//
//                 if(is_member_pointer) {
//                         flags |= 0b1;
//                 }
//
//                 if(member_property.type != sh_semicolon) {
//                         fprintf(sh_generated, "\t{%d, type_%.*s, \"%.*s\", offsetof(%.*s, %.*s)}",
//                                 flags,
//                                 struct_mbr_type.length, struct_mbr_type.pos,
//                                 member_property.length, member_property.pos,
//                                 struct_name.length, struct_name.pos,
//                                 member_property.length, member_property.pos
//                                 );
//                 }
//
//                 if(member_property.type == sh_semicolon)
//                         member_end = 1;
//         }
// }
//
void sh_parse_struct(stream_data *stream) {
        sh_token struct_name = next_token(stream);
        if(!struct_type_seen_already(&struct_name)) {
                add_new_struct_type(&struct_name);
        }

        fprintf(sh_generated, "struct_meta_info struct_%.*s[] = {\n", struct_name.length, struct_name.pos);
        if(sh_expect_token(stream, sh_open_brace)) {
                bool end_struct = 0;
                while(!end_struct) {
                        sh_token struct_member = next_token(stream);
                        if(struct_member.type == sh_comment) continue;
                        if(struct_member.type == sh_close_brace) {
                                end_struct = 1;
                        } else {
                                sh_struct_member_parse(stream, struct_member, struct_name);
                                fprintf(sh_generated, ",\n");
                        }
                }
        } else {
                printf("open brace was expected");
        }
        fprintf(sh_generated, "};\n");
}



void process_expr(sh_expr *expr, sh_token class_name) {
        sh_token tok = expr->tok_head->token;
        if(expr->token_number == 3 && tok.type != sh_comment  && tok.pos[0] != '#') {
                sh_token member_type = expr->tok_head->token;
                sh_token member_name = expr->tok_head->next->token;

                if(!type_seen_already(&member_type)) {
                        add_new_type(&member_type);
                }

                fprintf(sh_generated, "\t{%d, type_%.*s, \"%.*s\", offsetof(%.*s, %.*s)},\n",
                        0,
                        member_type.length, member_type.pos,
                        member_name.length, member_name.pos,
                        class_name.length, class_name.pos,
                        member_name.length, member_name.pos
                       );
        }
}

void sh_parse_class_members(sh_token class_name, stream_data *stream) {
        if(!struct_type_seen_already(&class_name)) {
                add_new_struct_type(&class_name);
        }

        fprintf(sh_generated, "struct_meta_info class_%.*s[] = {\n", class_name.length, class_name.pos);
        int stop = false;
        while(!stop) {
                sh_token_type next_tok_type = peak_next_token(stream);
                if( next_tok_type == sh_end_of_stream || next_tok_type == sh_close_brace) {
                        stop = true;
                        continue;
                }

                sh_expr exp = {};
                collect_to_semi_colon(stream, &exp);
                process_expr(&exp, class_name);
        }

        fprintf(sh_generated, "};\n");
}

void sh_parse_class(stream_data *stream) {
        sh_token prev_token = next_token(stream);
        sh_token cur_token = next_token(stream);
        while(cur_token.type != sh_open_brace ) {
                prev_token = cur_token;
                cur_token = next_token(stream);
        }

        if(cur_token.type == sh_open_brace) {

                sh_parse_class_members(prev_token, stream);
        }
}

void sh_parse_inspect(stream_data *stream) {
        if(sh_expect_token(stream, sh_identifier, "struct")) {
                sh_parse_class(stream);
        } else if(sh_expect_token(stream, sh_identifier, "class")) {
                sh_parse_class(stream);
        }
}

int main(int argc, char ** argv) {
        char *files_to_parse[] = {
                "header/grid_managment.h",
                "S://Code//libs//lib//include//Shar//headers/sh_circle.h",
                "S://Code//libs//lib//include//Shar//headers/shar.h"
        };

        int array_size = sizeof(files_to_parse)/sizeof(files_to_parse[0]);

        sh_generated = fopen("src/sh_generated.cpp", "w");
        for(int i = 0; i < array_size; ++i) {
                stream_data stream = {};
                char *file_data = sh_readfile(files_to_parse[i], &stream);    
                stream.cur = file_data;
                stream.read_so_far = 0;
                int parse = 1;
                 
                while(parse) {
                        sh_token tok = next_token(&stream);
                        if(tok.type == sh_end_of_stream) {
                                parse = 0;
                        }

                        if(tok.type == sh_identifier) {
                                if(token_equals(tok.pos, "sh_inspect")) {
                                        sh_parse_inspect(&stream);
                                }
                        }
                }

                free(file_data);
        }

        FILE *sh_types_h = fopen("header/sh_meta_types.h", "w");
        fprintf(sh_types_h, "#ifndef SH_META_TYPES_H\n");
        fprintf(sh_types_h, "#define SH_META_TYPES_H\n");
        fprintf(sh_types_h, "enum member_type {\n");
        for(types_seen *type = first_type; type; type = type->next) {
                fprintf(sh_types_h, "\ttype_%s,\n", type->name);
                //printf("type_%s\n", type->name);
        }
        fprintf(sh_types_h, "};\n");
         
        fprintf(sh_types_h, "#endif SH_META_TYPES_H\n");
        return 0;
}
