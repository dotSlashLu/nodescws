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

Handle<Value> Segment(const Arguments& args) {
        HandleScope scope;

        if (args.Length() < 1) {
                ThrowException(Exception::TypeError(String::New("[scws ERROR] Wrong number of arguments")));
                return scope.Close(Undefined());
        }

        if (!args[0]->IsString()) {
                ThrowException(Exception::TypeError(String::New("[scws ERROR] Argument 1 should be the string to segment")));
                return scope.Close(Undefined());
        }


        /* 
         * setup scws
         */
        scws_t ret = scws_new();
        // get settings
        v8::Local<v8::Object> Settings = args[1]->ToObject();

        // setup charset
        std::string Charset(*v8::String::Utf8Value(Settings->Get(String::New("charset"))));
        char *charset = (char *)Charset.c_str();
        if (strcmp(charset, "undefined") == 0) {
                printf("[scws WARNING] Charset not specified\n");
                charset = "utf8";
        }
        else if (strcmp(charset, "utf8") != 0 && strcmp(charset, "gbk") != 0)
                charset = "utf8";
        printf("[scws LOG] Setting charset: %s\n", charset);
        scws_set_charset(ret, charset);

        // setup dict
        std::string Dicts(*v8::String::Utf8Value(Settings->Get(String::New("dicts"))));
        char *dicts = (char *)Dicts.c_str();
        if (strcmp(dicts, "undefined") == 0) {
                printf("[scws WARNING] Dict not specified, loading from the default path\n");
                if(scws_add_dict(ret, "./dicts/dict.utf8.xdb", SCWS_XDICT_XDB) == -1)
                        printf("[scws ERROR] Default dict not loaded\n");
        }
        else {
                int dict_mode;
                if (strchr(dicts, ':') != NULL) {
                        while (*dicts != '\0') {
                                char *dict = (char *)malloc(sizeof(char) * MAXDIRLEN);
                                int i = 0;
                                while (i < MAXDIRLEN && *dicts != ':' && *dicts != '\0')
                                        dict[i++] = *dicts++;
                                dict[i++] = '\0';
                                if (*dicts != '\0')
                                        dicts++; // skip the ':'

                                if (strstr(dict, ".txt") != NULL)
                                        dict_mode = SCWS_XDICT_TXT;
                                else
                                        dict_mode = SCWS_XDICT_XDB;
                                printf("[scws LOG] Setting dict: %s\n", dict);
                                if (scws_add_dict(ret, dict, dict_mode) == -1)
                                        printf("[scws ERROR] Failed to load dict %s\n", dict);
                                free(dict);
                        }
                }
                else {
                        if (strstr(dicts, ".txt") != NULL)
                                dict_mode = SCWS_XDICT_TXT;
                        else
                                dict_mode = SCWS_XDICT_XDB;
                        printf("[scws LOG] Setting dict: %s\n", dicts);
                        if (scws_add_dict(ret, dicts, dict_mode) == -1) {
                                printf("[scws ERROR] Failed to load dict %s\n", dicts);
                        }
                }
        }

        // set rules
        std::string Rule(*v8::String::Utf8Value(Settings->Get(String::New("rule"))));
        char *rule = (char *)Rule.c_str();
        if (strcmp(rule, "undefined") == 0) {
                printf("[scws WARNING] Rule not specified, loading from the default path\n");
                scws_set_rule(ret, "./rules/rules.utf8.ini");
        }
        else {
                scws_set_rule(ret, rule);
                printf("[scws LOG] Setting specified rule %s\n", rule);
        }

        // set ignore punctuation
        Local<Boolean> IgnorePunct = Settings->Get(String::New("ignorePunct"))->ToBoolean();
        if (IgnorePunct->BooleanValue())
                scws_set_ignore(ret, 1);

        
        std::string Text(*v8::String::Utf8Value(args[0]->ToString()));
        char *text = (char *)Text.c_str();
        scws_send_text(ret, text, strlen(text));

        scws_res_t res;
        int result_words_count = 0;
        long memsize = RESMEMSTEP * sizeof(scws_result);
        int memsteps = 1;
        scws_result *results_raw = (scws_result *)malloc(memsize);

        while ((res = scws_get_result(ret)) != NULL) {
                while (res != NULL) {
                        memcpy(&results_raw[result_words_count], res, sizeof(*res));
                        result_words_count++;
                        if (result_words_count >= RESMEMSTEP * memsteps) {
                                long new_size = RESMEMSTEP * (memsteps + 1) * sizeof(scws_result);
                                if((results_raw = (scws_result *)realloc(results_raw, new_size)) == NULL){
                                        printf("[scws ERROR] Failed to allocate memory for results\n");
                                        ENDSCWS();
                                        return scope.Close(Array::New(0));
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
                        FunctionTemplate::New(Segment)->GetFunction());
}

NODE_MODULE(scws, Init)
