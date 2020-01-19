#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define TYPES          \
        sh_type(int)   \
        sh_type(float) \
        sh_type(char)  \
        sh_type(double)\
        sh_type(vec2)  \
        sh_type(void)  \


enum {
#define sh_type(type_name) sh_##type_name,
        TYPES
        list_size
#undef sh_type
};

char *types[] = {
 #define sh_type(type_name) #type_name,
        TYPES
#undef sh_type
};

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

int expect_in(sh_token_node *tok_head, sh_token_type type, char **types_text, int types_size) {
        sh_token tok = tok_head->token;
        // printf("token type: %.*s\n", tok.length, tok.pos);
        for(int i = 0; i < types_size; ++i) {
                if(strncmp(tok.pos, types_text[i], strlen(types_text[i])) == 0)  {
                        // printf("found a type: %s\n", types_text[i]);
                        return 1;
                }
        }

        return 0;
}

int expect(sh_token_node *tok_head, sh_token_type type) {
        // printf("token: %.*s\n", tok_head->token.length, tok_head->token.pos);
        return (tok_head->token.type == type);
}

int expect_func_paramteres(sh_token_node *tok_head) {
        return 1;
}

int parse_variable(sh_expr *expr, sh_token class_name) {
        sh_token_node *temp = expr->tok_head;

        // if(!expect_in(temp, sh_identifier, types, list_size)) {
        //         return 0;
        // }

        sh_token var_type = temp->token;

        temp = temp->next;
        int ptr_lvl = 0;
        do {
                if(expect(temp, sh_asterisk)) {
                        ++ptr_lvl;
                        temp = temp->next;
                } else {
                        break;
                }
        } while(temp->token.type == sh_asterisk);

        
        // temp = temp->next;
        if(!expect(temp, sh_identifier)) {
                return 0;
        }
        
        sh_token var_name = temp->token;

        temp = temp->next;
        if(!expect(temp, sh_semicolon)) {
                return 0;
        }

        int is_pointer = 0;
        if(ptr_lvl) is_pointer = 1;

        fprintf(sh_generated, "\t{%d, type_%.*s, \"%.*s\", offsetof(%.*s, %.*s)},\n",
                        is_pointer,
                        var_type.length, var_type.pos,
                        var_name.length, var_name.pos,
                        class_name.length, class_name.pos,
                        var_name.length, var_name.pos
               );

        if(!type_seen_already(&var_type)) {
                add_new_type(&var_type);
        }

        return 1;
}

int parse_func(sh_expr *expr) {
        sh_token_node *temp = expr->tok_head;
        if(!expect_in(temp, sh_identifier, types, list_size)) {
                // printf("function didn't start with a type\n");
                return 0;
        }

        temp = temp->next;
        if(!expect(temp, sh_identifier)) {
                // printf("function doesn't have a name\n");
                return 0;
        }

        temp = temp->next;

        if(!expect(temp, sh_open_paran)){
                // printf("function must have an open_paran after name\n");
                return 0;
        }  

        temp = temp->next;
        if(!expect_func_paramteres(temp)) {
                // printf("function must have parameters after open paran\n");
                return 0;
        }

        while(temp && temp->token.type != sh_close_paran)
                temp = temp->next;

        if(!expect(temp, sh_close_paran)) {
                // printf("function doesn't have a name\n");
                return 0;
        }

        temp = temp->next;
        if(!expect(temp, sh_semicolon)) {
                return 0;
        }

        return 1;
}

void process_expr(sh_expr *expr, sh_token class_name) {
        parse_variable(expr, class_name);
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
        sh_parse_class(stream);
}

int main(int argc, char ** argv) {
        char *files_to_parse[] = {
                "header/grid_managment.h",
                "W://code//libs//lib//include//Shar//headers/sh_circle.h",
                "W://code//libs//lib//include//Shar//headers/sh_line.h",
                "W://code//libs//lib//include//Shar//headers/shar.h"
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

        FILE* sh_types_h = fopen("header/sh_meta_types.h", "w");
        fprintf(sh_types_h, "#ifndef SH_META_TYPES_H\n");
        fprintf(sh_types_h, "#define SH_META_TYPES_H\n");
        fprintf(sh_types_h, "enum member_type {\n");
        for(types_seen *type = first_type; type; type = type->next) {
                fprintf(sh_types_h, "\ttype_%s,\n", type->name);
                //printf("type_%s\n", type->name);
        }
        fprintf(sh_types_h, "};\n");
        fprintf(sh_types_h, "\n\n");


	for(struct_types_seen *type = first_struct_type; type; type = type->next) {
		fprintf(sh_types_h, "#define DUMP_%s(t)", _strupr(type->name));
		fprintf(sh_types_h, "\t\t");
		fprintf(sh_types_h, "dump_object_log((void*)t, class_%s, array_count(class_%s), #t)\n", _strlwr(type->name), type->name);
		//printf("type_%s\n", type->name);
	}

         
        fprintf(sh_types_h, "#endif SH_META_TYPES_H\n");
	fclose(sh_types_h);
        return 0;
}
