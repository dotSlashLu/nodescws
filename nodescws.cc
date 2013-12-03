#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif
#include <node.h>
#include "nodescws.h"
#include <string>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "libscws/scws.h"

#define RESMEMSTEP 500
#define MAXDIRLEN 60

using namespace v8;

Nodescws::Nodescws(){};
Nodescws::~Nodescws(){};

int debug = 0;

void scws_debug(int level, char *msg, ...)
{
        if (debug)
        {
                va_list msg_args;
                va_start(msg_args, msg);
                switch (level)
                {
                        case NODESCWS_MSG_ERR:
                                printf("[scws ERROR] ");
                                break;
                        case NODESCWS_MSG_WARNING:
                                printf("[scws WARNING] ");
                                break;
                        case NODESCWS_MSG_LOG:
                                printf("[scws LOG] ");
                                break;
                }
                vprintf(msg, msg_args);
                va_end(msg_args);
        }
}

void Nodescws::Init(Handle<Object> target) 
{
        // Prepare constructor template
        Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
        tpl->SetClassName(String::NewSymbol("scws"));
        tpl->InstanceTemplate()->SetInternalFieldCount(1);
        // Prototype
        tpl->PrototypeTemplate()->Set(String::NewSymbol("segment"),
                FunctionTemplate::New(Segment)->GetFunction());  
        tpl->PrototypeTemplate()->Set(String::NewSymbol("destroy"),
                FunctionTemplate::New(Destroy)->GetFunction());  

        Persistent<Function> constructor = Persistent<Function>::New(tpl->GetFunction());
        target->Set(String::NewSymbol("init"), constructor);
}

Handle<Value> Nodescws::New(const Arguments& args) 
{
        HandleScope scope;

        Nodescws *scws = new Nodescws();
        
        if (!args[0]->IsObject()) {
                ThrowException(Exception::TypeError(String::New("[scws ERROR] init argument should be an object to specify configurations")));
                return scope.Close(Undefined());
        }
        // init scws
        scws_t ret = scws_new();
        v8::Local<v8::Object> Settings = args[0]->ToObject();

        // setup debug mode
        Local<Boolean> Debug = Settings->Get(String::New("debug"))->ToBoolean();
        if (Debug->BooleanValue())
                debug = 1;

        // setup charset
        std::string Charset(*v8::String::Utf8Value(Settings->Get(String::New("charset"))));
        char *charset = (char *)Charset.c_str();
        if (strcmp(charset, "undefined") == 0) {
                scws_debug(NODESCWS_MSG_WARNING, "Charset not specified\n");
                charset = "utf8";
        }
        else if (strcmp(charset, "utf8") != 0 && strcmp(charset, "gbk") != 0)
                charset = "utf8";
        scws_debug(NODESCWS_MSG_LOG, "Setting charset: %s\n", charset);
        scws_set_charset(ret, charset);

        // setup dict
        std::string Dicts(*v8::String::Utf8Value(Settings->Get(String::New("dicts"))));
        char *dicts = (char *)Dicts.c_str();
        if (strcmp(dicts, "undefined") == 0) {
                scws_debug(NODESCWS_MSG_WARNING, "Dict not specified, loading from the default path\n");
                if(scws_add_dict(ret, "./node_modules/scws/dicts/dict.utf8.xdb", SCWS_XDICT_XDB) == -1)
                        scws_debug(NODESCWS_MSG_ERR, "Default dict not loaded\n");
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
                                scws_debug(NODESCWS_MSG_LOG, "Setting dict: %s\n", dict);
                                if (scws_add_dict(ret, dict, dict_mode) == -1)
                                        scws_debug(NODESCWS_MSG_ERR, "Failed to load dict %s\n", dict);
                                free(dict);
                        }
                }
                else {
                        if (strstr(dicts, ".txt") != NULL)
                                dict_mode = SCWS_XDICT_TXT;
                        else
                                dict_mode = SCWS_XDICT_XDB;
                        scws_debug(NODESCWS_MSG_LOG, "Setting dict: %s\n", dicts);
                        if (scws_add_dict(ret, dicts, dict_mode) == -1) {
                                scws_debug(NODESCWS_MSG_ERR, "Failed to load dict %s\n", dicts);
                        }
                }
        }

        // set rules
        std::string Rule(*v8::String::Utf8Value(Settings->Get(String::New("rule"))));
        char *rule = (char *)Rule.c_str();
        if (strcmp(rule, "undefined") == 0) {
                scws_debug(NODESCWS_MSG_WARNING, "Rule not specified, loading from the default path\n");
                scws_set_rule(ret, "./node_modules/scws/rules/rules.utf8.ini");
        }
        else {
                scws_set_rule(ret, rule);
                scws_debug(NODESCWS_MSG_LOG, "Setting specified rule %s\n", rule);
        }

        // set ignore punctuation
        Local<Boolean> IgnorePunct = Settings->Get(String::New("ignorePunct"))->ToBoolean();
        if (IgnorePunct->BooleanValue())
                scws_set_ignore(ret, 1);
        
        Local<Boolean> ApplyStopWord = Settings->Get(String::New("applyStopWord"))->ToBoolean();
        if (IgnorePunct->BooleanValue())
                scws_set_stopword(ret, 1);
        else
                scws_set_stopword(ret, 0);

        std::string Multi(*v8::String::Utf8Value(Settings->Get(String::New("multi"))));
        char *multi = (char *)Multi.c_str();
        if (strcmp(multi, "undefined") == 0)
                scws_debug(NODESCWS_MSG_WARNING, "Multi mode not set, using default\n");
        else {
                int multi_mode = SCWS_MULTI_SHORT;
                if (strcmp(multi, "short") == 0)
                        multi_mode = SCWS_MULTI_SHORT;
                else if(strcmp(multi, "duality") == 0)
                        multi_mode = SCWS_MULTI_DUALITY;
                else if(strcmp(multi, "zmain") == 0)
                        multi_mode = SCWS_MULTI_ZMAIN;
                else if(strcmp(multi, "zall") == 0)
                        multi_mode = SCWS_MULTI_ZALL;
                scws_debug(NODESCWS_MSG_LOG, "Setting multi mode %s\n", multi);
                scws_set_multi(ret, multi_mode);
        }

        // scws->InstanceTemplate()->SetInternalField(0, ret);
        scws->instance_ = ret;
        scws->Wrap(args.This());

        return args.This();
}


Handle<Value> Nodescws::Segment(const v8::Arguments& args)
{
        HandleScope scope;
        
        if (!args[0]->IsString()) {
                ThrowException(Exception::TypeError(String::New("[scws ERROR] segment argument should be the string to segment")));
                return scope.Close(Undefined());
        }

        Nodescws *scws = node::ObjectWrap::Unwrap<Nodescws>(args.This());
        scws_t ret = scws->instance_;
        std::string Text(*v8::String::Utf8Value(args[0]->ToString()));
        char *text = (char *)Text.c_str();
        scws_send_text(ret, text, strlen(text));

        scws_res_t res;
        int result_words_count = 0;
        long memsize = RESMEMSTEP * sizeof(scws_result);
        int memsteps = 1;
        // scws_result *results_raw = (scws_result *)malloc(memsize);
        scws->result_raw_ = (scws_result *)malloc(memsize);

        while ((res = scws_get_result(ret)) != NULL) {
                while (res != NULL) {
                        memcpy(&scws->result_raw_[result_words_count], res, sizeof(*res));
                        result_words_count++;
                        if (result_words_count >= RESMEMSTEP * memsteps) {
                                long new_size = RESMEMSTEP * (memsteps + 1) * sizeof(scws_result);
                                if((scws->result_raw_ = (scws_result *)realloc(scws->result_raw_, new_size)) == NULL){
                                        scws_debug(NODESCWS_MSG_ERR, "Failed to allocate memory for results\n");
                                        free(scws->result_raw_);
                                        scws_free(ret);
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
                scws_result *cur = &scws->result_raw_[i];
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

        free(scws->result_raw_);
        return scope.Close(array);
}

Handle<Value> Nodescws::Destroy(const v8::Arguments& args)
{
        HandleScope scope;
        Nodescws *scws = node::ObjectWrap::Unwrap<Nodescws>(args.This());
        scws_free(scws->instance_);
        return scope.Close(Undefined());
}
