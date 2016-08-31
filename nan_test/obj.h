#pragma once

#include <nan.h>

class MyObject : public Nan::ObjectWrap {
        public:
        static void Init(v8::Local<v8::Object> exports);

        private:
        explicit MyObject(double value = 0);
        ~MyObject();

        static void Init(const Nan::FunctionCallBackInfo<v8::Value>& info);

}
