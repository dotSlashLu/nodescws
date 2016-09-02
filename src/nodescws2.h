#pragma once

#include <nan.h>

using Nan::FunctionCallbackInfo;
using v8::Local;

class NodeScws : public Nan::ObjectWrap {
        public:
        static void Init(v8::Local<v8::Object> module);

        private:
        // explicit NodeScws(const v8::Local<v8::Object>);
        explicit NodeScws(const v8::Local<v8::Object>);
        ~NodeScws();

        static Nan::Persistent<v8::Function> constructor;
        static void New(const FunctionCallbackInfo<v8::Value>& info);
        static void ScwsInit(const FunctionCallbackInfo<v8::Value>& info);
        static void ScwsSegment(const FunctionCallbackInfo<v8::Value>& info);
        static void ScwsDestroy(const FunctionCallbackInfo<v8::Value>& info);
        static void ScwsGetConfig(const FunctionCallbackInfo<v8::Value>& info);

        Local<v8::Object> Config;
};
