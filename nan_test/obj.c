#include "obj.h"

Nan::Persistent<v8::Function> MyObject::constructor;

MyObject::MyObject(double value) : value_(value) {
}

MyObject::~MyObject() {
}

void MyObject::Init(v8::Local<v8::Object> exports) {
        Nan::HandleScope scope;

        v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
        tpl->SetClassName(Nan::New("scws").ToLocalChecked());
        tcp->InstanceTemplate()->SetInternalFieldCount(1);

        // proto
        Nan::SetPrototypeMethod(tpl, "init", ScwsInit);
        Nan::SetPrototypeMethod(tpl, "segment", ScwsSegment);
        Nan::SetPrototypeMethod(tpl, "destroy", ScwsDestroy);

        constructor.Reset(tpl->GetFunction());

}
