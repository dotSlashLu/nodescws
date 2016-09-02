#include "nodescws2.h"
#include <iostream>

using v8::Local;
using v8::FunctionTemplate;

Nan::Persistent<v8::Function> NodeScws::constructor;

// NodeScws::NodeScws(const v8::Local<v8::Object> conf) : Config(conf)
// {
// }

NodeScws::NodeScws(const Local<v8::Object> config) : Config(config)
{
}

NodeScws::~NodeScws()
{
}

void NodeScws::Init(v8::Local<v8::Object> module)
{
        Nan::HandleScope scope;

        Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
        tpl->SetClassName(Nan::New("scws").ToLocalChecked());
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        // proto
        Nan::SetPrototypeMethod(tpl, "segment", ScwsSegment);
        Nan::SetPrototypeMethod(tpl, "destroy", ScwsDestroy);
        Nan::SetPrototypeMethod(tpl, "getConfig", ScwsGetConfig);

        constructor.Reset(tpl->GetFunction());
        module->Set(Nan::New("exports").ToLocalChecked(), tpl->GetFunction());
}

void NodeScws::New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
        if (info.IsConstructCall()) {
                // Invoked as constructor: `new NodeScws(...)`
                Local<v8::Object> Config = info[0]->IsUndefined() ?
                                Nan::New<v8::Object>() :
                                info[0]->ToObject();
                NodeScws* Scws = new NodeScws(Config);
                // auto val = Nan::To<v8::Number>(Scws->Config->Get(Nan::New<v8::String>("a").ToLocalChecked())).ToLocalChecked();
                Scws->Wrap(info.This());
                info.GetReturnValue().Set(info.This());
        }
        // makes no sense to invoke like a function
        else {
                auto msg = "Direct function call not supported, "
                        "use constructor call.";
                Nan::ThrowError(msg);
                return;
        }
}

void NodeScws::ScwsInit(const Nan::FunctionCallbackInfo<v8::Value>& info)
{
        info.GetReturnValue().Set(Nan::New("init").ToLocalChecked());
}

void NodeScws::ScwsSegment(const Nan::FunctionCallbackInfo<v8::Value>& info)
{
        info.GetReturnValue().Set(Nan::New("segment").ToLocalChecked());
}

void NodeScws::ScwsDestroy(const Nan::FunctionCallbackInfo<v8::Value>& info)
{
        info.GetReturnValue().Set(Nan::New("destroy").ToLocalChecked());
}

void NodeScws::ScwsGetConfig(const Nan::FunctionCallbackInfo<v8::Value>& info)
{
        NodeScws* scws = ObjectWrap::Unwrap<NodeScws>(info.This());
        info.GetReturnValue().Set(Nan::New(scws->Config));
}
