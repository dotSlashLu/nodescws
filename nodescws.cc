#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif
#include <node.h>
#include <string>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "libscws/scws.h"

#define RESMEMSTEP 500
#define MAXDIRLEN 60
#define ENDSCWS() free(results_raw);\
        scws_free(ret);
        
using namespace v8;
const char *V8StringToCString(v8::Local<v8::Value>);

Handle<Value> Split(const Arguments& args) {
        HandleScope scope;

        /*
         *  scws(text, charset, dict, ignore_punct, multi);
         * */
        if (args.Length() < 1) {
                ThrowException(Exception::TypeError(String::New("[scws] Wrong number of arguments")));
                return scope.Close(Undefined());
        }

        if (!args[0]->IsString()) {
                ThrowException(Exception::TypeError(String::New("[scws] Wrong arguments")));
                return scope.Close(Undefined());
        }

        /* 
         * setup scws
         */
        scws_t ret = scws_new();

        v8::String::Utf8Value charset_str(args[1]->ToString());
        std::string charset_str_std = std::string(*charset_str);
        char *charset = (char *)charset_str_std.c_str();
        if (strcmp(charset, "utf8") != 0 && strcmp(charset, "gbk") != 0)
                charset = "utf8";
        printf("charset: %s\n", charset);
        scws_set_charset(ret, charset);

        // setup dict
        v8::String::Utf8Value dicts_str(args[2]->ToString());
        std::string dicts_str_std = std::string(*dicts_str);
        char *dicts = (char *)dicts_str_std.c_str();
        int dict_mode;
        if (strchr(dicts, ':') != NULL) {
                while (*dicts != '\0') {
                        char *dict = (char *)malloc(sizeof(char) * MAXDIRLEN);
                        int i = 0;
                        while (i < MAXDIRLEN && (*dicts != ':'))
                                dict[i++] = *dicts++;
                        if (*dicts != '\0')
                                dicts++; // skip the ':'
                        if (strstr(dict, ".txt") != NULL) {
                                dict_mode = SCWS_XDICT_TXT;
                                printf("setting dict: txt mode\n");
                        }
                        else
                                dict_mode = SCWS_XDICT_XDB;
                        printf("setting dict: %s\n", dict);
                        scws_add_dict(ret, dict, dict_mode);
                        free(dict);
                }
        }
        else {
                if (strstr(dicts, ".txt") != NULL)
                        dict_mode = SCWS_XDICT_TXT;
                else
                        dict_mode = SCWS_XDICT_XDB;
                printf("setting dict: %s\n", dicts);
                scws_add_dict(ret, dicts, dict_mode);
        }

        int add_dict_ret = scws_add_dict(ret, "./dicts/dict.utf8.xdb", SCWS_XDICT_XDB);
        scws_set_rule(ret, "./rules/rules.utf8.ini");
        scws_set_ignore(ret, 1);
        if (add_dict_ret < 0)
                ThrowException(Exception::Error(String::New("[scws] Can't load dict")));

        v8::String::Utf8Value value_str(args[0]->ToString());
        std::string value_str_std = std::string(*value_str);
        char *text = (char *)value_str_std.c_str();
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
                sprintf(str, "%.*s", cur->len, text + cur->off);
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

const char *V8StringToCString(v8::Local<v8::Value> value) {
        HandleScope scope;
        v8::String::Utf8Value value_str(value->ToString());
        std::string value_str_std = std::string(*value_str);
        std::cout<<"\nread param:\n"<<value_str_std.c_str();
        return value_str_std.c_str();
}

NODE_MODULE(scws, Init)
