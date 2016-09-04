#pragma once

#include <nan.h>

using v8::Value;
using v8::Local;
using v8::Object;

using Nan::Persistent;
using Nan::FunctionCallbackInfo;

class NodeScws : public Nan::ObjectWrap {
        public:
        static void Init(Local<Object> module);

        private:
        explicit NodeScws(Local<Object>);
        ~NodeScws();

        static Nan::Persistent<v8::Function> constructor;
        static void New(const FunctionCallbackInfo<Value>& info);
        static void ScwsInit(const FunctionCallbackInfo<Value>& info);
        static void ScwsSegment(const FunctionCallbackInfo<Value>& info);
        static void ScwsDestroy(const FunctionCallbackInfo<Value>& info);
        static void ScwsGetConfig(const FunctionCallbackInfo<Value>& info);

        Persistent<Object> Config;
};
