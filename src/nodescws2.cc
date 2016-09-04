#include "nodescws2.h"

using v8::Local;
using v8::Object;
using v8::FunctionTemplate;

using Nan::Persistent;
using Nan::CopyablePersistentTraits;

Nan::Persistent<v8::Function> NodeScws::constructor;

NodeScws::NodeScws(Local<Object> config)
{
        Persistent<Object> conf(config);
        Config.Reset(conf);
}

NodeScws::~NodeScws()
{
        Config.Reset();
}

void NodeScws::Init(v8::Local<v8::Object> module)
{
        Nan::HandleScope scope;

        Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
        tpl->SetClassName(Nan::New("scws").ToLocalChecked());
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

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
                Nan::ThrowError("Direct function call not supported, "
                        "use constructor call.");
                return;
        }

        if (!info[0]->IsObject()) {
                Nan::ThrowTypeError("Config should be an object.");
                return;
        }

        Local<Object> conf = info[0].As<Object>();
        NodeScws* nscwsp = new NodeScws(conf);
        nscwsp->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
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
        NodeScws* scws = ObjectWrap::Unwrap<NodeScws>(info.Holder());
        Local<Object> conf = Nan::New(scws->Config);
        info.GetReturnValue().Set(conf);
}
