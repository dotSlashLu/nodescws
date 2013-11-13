#define BUILDING_NODE_EXTENSION
#include <node.h>
#include <string>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "libscws/scws.h"

#define RESMEMSTEP 500
#define ENDSCWS() free(results_raw);\
        free(text);\
        scws_free(ret);

        
using namespace v8;
const char *V8StringToCString(v8::Handle<v8::Value>);

Handle<Value> Split(const Arguments& args) {
        HandleScope scope;

        /*
         *  scws(text, charset, dict, ignore_punct, multi);
         * */
        if (args.Length() < 1) {
                ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
                return scope.Close(Undefined());
        }

        if (!args[0]->IsString()) {
                ThrowException(Exception::TypeError(String::New("Wrong arguments")));
                return scope.Close(Undefined());
        }

        /* 
         * setup scws
         */
        scws_t ret = scws_new();
        scws_set_charset(ret, "utf8");
        int add_dict_ret = scws_add_dict(ret, "./dicts/dict.utf8.xdb", SCWS_XDICT_XDB);
        scws_set_rule(ret, "./rules/rules.utf8.ini");
        scws_set_ignore(ret, 1);
        if (add_dict_ret < 0)
                printf("add dict error\n");

        v8::String::Utf8Value value_str(args[0]);
        std::string value_str_std = std::string(*value_str);
        const char *const_text = value_str_std.c_str();
        char *text = (char *)malloc(sizeof const_text * strlen(const_text));
        strcpy(text, const_text);
        scws_send_text(ret, text, strlen(text));

        scws_res_t res;
        int result_words_count = 0;
        long memsize = RESMEMSTEP * sizeof(scws_result);
        int memsteps = 1;
        scws_result *results_raw = (scws_result *)malloc(memsize);

        while ((res = scws_get_result(ret)) != NULL) {
                // printf("word: %d\tlimit: %d\n", result_words_count, RESMEMSTEP * memsteps);
                while (res != NULL) {
                        memcpy(&results_raw[result_words_count], res, sizeof(*res));
                        result_words_count++;
                        if (result_words_count >= RESMEMSTEP * memsteps) {
                                long new_size = RESMEMSTEP * (memsteps + 1) * sizeof(scws_result);
                                if((results_raw = (scws_result *)realloc(results_raw, new_size)) == NULL){
                                        printf("failed to alloc memory to results");
                                        ENDSCWS();
                                }; 
                                memsteps++;
                        }
                        res = res->next;
                }
                scws_free_result(res);
        }

        
        Handle<Array> array = Array::New(result_words_count);
        for (int i = 0; i < result_words_count; i++) {
                scws_result *cur = &results_raw[i];
                char *str = (char *)malloc(((int)cur->len + 1) * sizeof(char));
                char *start = &text[(int)cur->off];
                strncpy(str, start, (int)cur->len);
                str[(int)cur->len] = '\0';

                Local<Object> obj = Object::New();
                obj->Set(String::NewSymbol("word"), String::New(str));
                free(str);
                obj->Set(String::NewSymbol("offset"), Number::New(cur->off));
                obj->Set(String::NewSymbol("length"), Number::New(cur->len));
                obj->Set(String::NewSymbol("attr"), String::New(cur->attr));
                obj->Set(String::NewSymbol("idf"), Number::New(cur->idf));
                array->Set(i++, obj);
        }
        ENDSCWS();

        return scope.Close(array);
}

void Init(Handle<Object> exports) {
        exports->Set(String::NewSymbol("segment"),
                        FunctionTemplate::New(Split)->GetFunction());
}

const char *V8StringToCString(v8::Handle<v8::Value> value) {
        v8::String::Utf8Value value_str(value);
        std::string value_str_std = std::string(*value_str);
        std::cout<<"\nread param:\n"<<value_str_std.c_str();
        return value_str_std.c_str();
}

NODE_MODULE(scws, Init)
