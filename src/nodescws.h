#pragma once

#include <nan.h>
#include "libscws/scws.h"

#define RESMEMSTEP 500
#define MAXDIRLEN 1024

#define NODESCWS_MSG_ERR 1
#define NODESCWS_MSG_WARNING 2
#define NODESCWS_MSG_LOG 3

using v8::Value;
using v8::Array;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Boolean;

using Nan::Persistent;
using Nan::FunctionCallbackInfo;

class NodeScws : public Nan::ObjectWrap {
        public:
        static void Init(Local<Object> module);

        private:
        explicit NodeScws(Local<Object>);
        ~NodeScws();

        static Nan::Persistent<v8::Function> constructor;
        // methods
        static void New(const FunctionCallbackInfo<Value>& info);
        static void ScwsSegment(const FunctionCallbackInfo<Value>& info);
        static void ScwsDestroy(const FunctionCallbackInfo<Value>& info);
        static void ScwsGetConfig(const FunctionCallbackInfo<Value>& info);

        static void Log(int level, const char *msg, ...);
        Local<Value> GetObjectProp(Local<Object>, const char *);
        void ScwsInit();

        Persistent<Object> Config;
        scws_t scws;
        scws_result *result_raw_;
};
