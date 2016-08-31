#include "obj.h"

Nan::Persistent<v8::Function> MyObject::constructor;

MyObject::MyObject(double value) : value_(value) {
}

MyObject::~MyObject() {
}

void MyObject::Init(v8::Local<v8::Object> module) {
        Nan::HandleScope scope;

        v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
        tpl->SetClassName(Nan::New("scws").ToLocalChecked());
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        // proto
        Nan::SetPrototypeMethod(tpl, "init", ScwsInit);
        Nan::SetPrototypeMethod(tpl, "segment", ScwsSegment);
        Nan::SetPrototypeMethod(tpl, "destroy", ScwsDestroy);

        constructor.Reset(tpl->GetFunction());
        module->Set(Nan::New("exports").ToLocalChecked(), tpl->GetFunction());
}

void MyObject::New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  if (info.IsConstructCall()) {
    // Invoked as constructor: `new MyObject(...)`
    double value = info[0]->IsUndefined() ? 0 : info[0]->NumberValue();
    MyObject* obj = new MyObject(value);
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    const char msg[] = "Direct function call not supported, please use constructor call.";
    Nan::ThrowError(msg);
    return;
    // Invoked as plain function `MyObject(...)`, turn into construct call.
    // const int argc = 1;
    // v8::Local<v8::Value> argv[argc] = { info[0] };
    // v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
    // info.GetReturnValue().Set(cons->NewInstance(argc, argv));
  }
}

void MyObject::ScwsInit(const Nan::FunctionCallbackInfo<v8::Value>& info)
{
        info.GetReturnValue().Set(Nan::New(std::string("init")).ToLocalChecked());
}

void MyObject::ScwsSegment(const Nan::FunctionCallbackInfo<v8::Value>& info)
{
        info.GetReturnValue().Set(Nan::New(std::string("segment")).ToLocalChecked());
}

void MyObject::ScwsDestroy(const Nan::FunctionCallbackInfo<v8::Value>& info)
{
        info.GetReturnValue().Set(Nan::New(std::string("destroy")).ToLocalChecked());
}
