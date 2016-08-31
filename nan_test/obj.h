#pragma once

#include <nan.h>

class MyObject : public Nan::ObjectWrap {
        public:
        static void Init(v8::Local<v8::Object> module);

        private:
        explicit MyObject(double value = 0);
        ~MyObject();

        static void New(const Nan::FunctionCallbackInfo<v8::Value>& info);
        static void ScwsInit(const Nan::FunctionCallbackInfo<v8::Value>& info);
        static void ScwsSegment(const Nan::FunctionCallbackInfo<v8::Value>& info);
        static void ScwsDestroy(const Nan::FunctionCallbackInfo<v8::Value>& info);
        static Nan::Persistent<v8::Function> constructor;
        double value_;
};
