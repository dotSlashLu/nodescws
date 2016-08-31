#include <nan.h>


void ScwsInit(const Nan::FunctionCallbackInfo<v8::Value>& info) {

  if (info.Length() < 2) {
    Nan::ThrowTypeError("Wrong number of arguments");
    return;
  }

  if (!info[0]->IsNumber() || !info[1]->IsNumber()) {
    Nan::ThrowTypeError("Wrong arguments");
    return;
  }

  double arg0 = info[0]->NumberValue();
  double arg1 = info[1]->NumberValue();
  v8::Local<v8::Number> num = Nan::New(arg0 + arg1);

  info.GetReturnValue().Set(num);
}

void CreateExport(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  v8::Local<v8::Object> _export = Nan::New<v8::Object>();
  _export->Set(Nan::New("init").ToLocalChecked(),
        Nan::New<v8::FunctionTemplate>(ScwsInit)->GetFunction());

  info.GetReturnValue().Set(_export);
}

void Init(v8::Local<v8::Object> exports,
        v8::Local<v8::Object> module) {
  module->Set(Nan::New("exports").ToLocalChecked(),
      Nan::New<v8::FunctionTemplate>(CreateExport)->GetFunction());
}

NODE_MODULE(addon, Init)
