#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif
#include <nan.h>
#include "nodescws.h"

using v8::String;
using v8::FunctionTemplate;

using Nan::New;
using Nan::Set;
using Nan::GetFunction;

v8::Local<v8::Function> init_fn = GetFunction(
        New<FunctionTemplate>(Nodescws::Init)
).ToLocalChecked();

NAN_MODULE_INIT(InitAll) {
        Set(target, New<String>("init").ToLocalChecked(),
        init_fn);
}


NODE_MODULE(scws, InitAll)
