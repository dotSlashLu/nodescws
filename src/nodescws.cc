#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdarg>

#include "nodescws.h"
#include "libscws/scws.h"

using v8::Local;
using v8::Object;
using v8::FunctionTemplate;

using Nan::Persistent;
using Nan::CopyablePersistentTraits;

static int debug = 0;

Nan::Persistent<v8::Function> NodeScws::constructor;

NodeScws::NodeScws(Local<Object> config)
{
        Persistent<Object> conf(config);
        Config.Reset(conf);
}

NodeScws::~NodeScws()
{
        Config.Reset();
        if (scws) scws_free(scws);
}

void NodeScws::Init(v8::Local<v8::Object> module)
{
        Nan::HandleScope scope;

        Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
        tpl->SetClassName(Nan::New("scws").ToLocalChecked());
        tpl->InstanceTemplate()->SetInternalFieldCount(3);

        // prototypes
        Nan::SetPrototypeMethod(tpl, "segment", ScwsSegment);
        Nan::SetPrototypeMethod(tpl, "destroy", ScwsDestroy);
        Nan::SetPrototypeMethod(tpl, "getConfig", ScwsGetConfig);

        constructor.Reset(tpl->GetFunction());
        module->Set(Nan::New("exports").ToLocalChecked(), tpl->GetFunction());
}

void NodeScws::New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
        // makes no sense to invoke like a function
        if (!info.IsConstructCall()) {
                Nan::ThrowError("[scws ERROR] Direct function call not supported, "
                        "use constructor call.");
                return;
        }

        if (!info[0]->IsObject()) {
                Nan::ThrowTypeError("[scws ERROR] Config should be an object.");
                return;
        }

        Local<Object> conf = info[0].As<Object>();
        NodeScws* nscwsp = new NodeScws(conf);
        nscwsp->ScwsInit();
        nscwsp->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
}

void NodeScws::ScwsInit()
{
        Local<Object> Settings = Nan::New<Object>(Config);

        // init scws
        scws_t scws = scws_new();

        // setup debug mode
        Local<Boolean> Debug = Settings->Get(Nan::New("debug").ToLocalChecked())->ToBoolean();
        if (Debug->BooleanValue()) {
                debug = 1;
                printf("debug: true\n");
        }

        // setup charset
        std::string Charset(*Nan::Utf8String(Settings->Get(Nan::New("charset").ToLocalChecked())));
        std::string charset = Charset.c_str();
        if (charset ==  "undefined") {
                Log(NODESCWS_MSG_WARNING, "Charset not specified\n");
                charset = "utf8";
        }
        else if (charset == "utf8" && charset == "gbk")
                charset = "utf8";
        Log(NODESCWS_MSG_LOG, "Setting charset: %s\n", charset.c_str());
        scws_set_charset(scws, charset.c_str());

        // setup dict
        std::string Dicts(*Nan::Utf8String(Settings->Get(Nan::New("dicts").ToLocalChecked())));
        char *dicts = (char *)Dicts.c_str();
        if (strcmp(dicts, "undefined") == 0) {
                Log(NODESCWS_MSG_WARNING, "Dict not specified, loading from the default path\n");
                if(scws_add_dict(scws, "./node_modules/scws/dicts/dict.utf8.xdb", SCWS_XDICT_XDB) == -1)
                        Log(NODESCWS_MSG_ERR, "Default dict not loaded\n");
        }
        else {
                int dict_mode;
                if (strchr(dicts, ':') != NULL) {
                        while (*dicts != '\0') {
                                char *dict = (char *)malloc(MAXDIRLEN);
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
                                Log(NODESCWS_MSG_LOG, "Setting dict: %s\n", dict);
                                if (scws_add_dict(scws, dict, dict_mode) == -1)
                                        Log(NODESCWS_MSG_ERR, "Failed to load dict %s\n", dict);
                                free(dict);
                        }
                }
                else {
                        if (strstr(dicts, ".txt") != NULL)
                                dict_mode = SCWS_XDICT_TXT;
                        else
                                dict_mode = SCWS_XDICT_XDB;
                        Log(NODESCWS_MSG_LOG, "Setting dict: %s\n", dicts);
                        if (scws_add_dict(scws, dicts, dict_mode) == -1) {
                                Log(NODESCWS_MSG_ERR, "Failed to load dict %s\n", dicts);
                        }
                }
        }

        // set rules
        std::string Rule(*Nan::Utf8String(GetObjectProp(Settings, "rule")));
        char *rule = (char *)Rule.c_str();
        if (strcmp(rule, "undefined") == 0) {
                Log(NODESCWS_MSG_WARNING, "Rule not specified, loading from the default path\n");
                scws_set_rule(scws, "./node_modules/scws/rules/rules.utf8.ini");
        }
        // json
        else if (strstr(rule, ".json") != NULL) {
                Log(NODESCWS_MSG_LOG, "Setting specified JSON rule %s\n", rule);
                scws_set_rule_json(scws, rule, SCWS_RULE_JSON_FILE);
        }
        // ini
        else if (strstr(rule, ".ini") > 0) {
                Log(NODESCWS_MSG_LOG, "Setting specified rule %s\n", rule);
                scws_set_rule(scws, rule);
        }
        // JSON string
        else {
                Log(NODESCWS_MSG_LOG, "Setting specified JSON string rule %s\n", rule);
                scws_set_rule_json(scws, rule, SCWS_RULE_JSON_STRING);
        }

        // set ignore punctuation
        auto IgnorePunct = GetObjectProp(Settings, "ignorePunct")->ToBoolean();
        if (IgnorePunct->BooleanValue())
                scws_set_ignore(scws, 1);

        auto ApplyStopWord = GetObjectProp(Settings, "applyStopWord")->ToBoolean();
        if (ApplyStopWord->BooleanValue()) {
                scws_set_stopword(scws, 1);
                Log(NODESCWS_MSG_LOG, "Set apply stop word\n");
        }
        else {
                scws_set_stopword(scws, 0);
                Log(NODESCWS_MSG_LOG, "Set not to apply stop word\n");
        }

        std::string Multi(*Nan::Utf8String(GetObjectProp(Settings, "multi")));
        char *multi = (char *)Multi.c_str();
        if (strcmp(multi, "undefined") == 0)
                Log(NODESCWS_MSG_WARNING, "Multi mode not set, using default\n");
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
                Log(NODESCWS_MSG_LOG, "Setting multi mode %s\n", multi);
                scws_set_multi(scws, multi_mode);
        }

        this->scws = scws;
}

void NodeScws::ScwsSegment(const Nan::FunctionCallbackInfo<v8::Value>& info)
{
        if (!info[0]->IsString()) {
                Nan::ThrowTypeError("[scws ERROR] Argument "
                        "should be the string to segment");
                // return undefined
                info.GetReturnValue()
                        .Set(Nan::Undefined());
                return;
        }

        NodeScws *nscwsp = ObjectWrap::Unwrap<NodeScws>(info.Holder());
        scws_t scws = nscwsp->scws;
        // destroyed?
        if (!scws) {
                Nan::ThrowError("[scws ERROR] scws instance not found, "
                        "possibly already been destroy()ed.");
                // return undefined
                info.GetReturnValue()
                        .Set(Nan::Undefined());
                return;
        }

        // convert v8::String to char *
        std::string Text(*Nan::Utf8String(info[0]->ToString()));
        char *text = (char *)Text.c_str();
        scws_send_text(scws, text, strlen(text));

        scws_res_t res;
        int res_wc = 0;
        long memsize = RESMEMSTEP * sizeof(scws_result);
        int memsteps = 1;

        nscwsp->result_raw_ = (scws_result *)malloc(memsize);

        res = scws_get_result(scws);
        while (res != NULL) {
                while (res != NULL) {
                        memcpy(&nscwsp->result_raw_[res_wc], res, sizeof(*res));
                        res_wc++;
                        if (res_wc >= RESMEMSTEP * memsteps) {
                                long new_size = RESMEMSTEP * (memsteps + 1) * sizeof(scws_result);
                                if ((nscwsp->result_raw_ = (scws_result *)realloc(nscwsp->result_raw_, new_size)) == NULL) {
                                        Log(NODESCWS_MSG_ERR, "Failed to allocate memory for results\n");
                                        free(nscwsp->result_raw_);
                                        scws_free(scws);
                                        info.GetReturnValue().Set(Array::New(0));
                                }
                                memsteps++;
                        }
                        res = res->next;
                }
                scws_free_result(res);
                res = scws_get_result(scws);
        }

        Local<Array> resArr = Nan::New<Array>(res_wc);
        for (int i = 0; i < res_wc; i++) {
                scws_result *cur = &nscwsp->result_raw_[i];
                char *str = (char *)malloc(((int)cur->len + 1));
                sprintf(str, "%.*s", cur->len, text + cur->off);
                str[(int)cur->len] = '\0';

                Local<Object> obj = Nan::New<Object>();
                obj->Set(Nan::New<v8::String>("word").ToLocalChecked(), Nan::New(str).ToLocalChecked());
                free(str);
                obj->Set(Nan::New<v8::String>("offset").ToLocalChecked(), Nan::New<Number>(cur->off));
                obj->Set(Nan::New<v8::String>("length").ToLocalChecked(), Nan::New<Number>(cur->len));
                obj->Set(Nan::New<v8::String>("attr").ToLocalChecked(), Nan::New(cur->attr).ToLocalChecked());
                obj->Set(Nan::New<v8::String>("idf").ToLocalChecked(), Nan::New<Number>(cur->idf));
                resArr->Set(i, obj);
        }

        free(nscwsp->result_raw_);
        info.GetReturnValue().Set(resArr);
}

void NodeScws::ScwsDestroy(const Nan::FunctionCallbackInfo<v8::Value>& info)
{
        NodeScws* nscwsp = ObjectWrap::Unwrap<NodeScws>(info.Holder());

        // only free scws, other Local values will be GCed by v8
        if (nscwsp->scws)
                scws_free(nscwsp->scws);
        nscwsp->scws = NULL;

        info.GetReturnValue().Set(Nan::True());
}

void NodeScws::ScwsGetConfig(const Nan::FunctionCallbackInfo<v8::Value>& info)
{
        NodeScws* scws = ObjectWrap::Unwrap<NodeScws>(info.Holder());
        Local<Object> conf = Nan::New(scws->Config);
        info.GetReturnValue().Set(conf);
}

void NodeScws::Log(int level, const char *msg, ...)
{
        if (!debug) return;
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

Local<Value> NodeScws::GetObjectProp(Local<Object> object, const char *prop)
{
        auto propName = Nan::New(prop).ToLocalChecked();
        return object->Get(propName);
}
