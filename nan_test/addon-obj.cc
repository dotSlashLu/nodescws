#include <nan.h>
#include "obj.h"

using v8::Local;
using v8::Object;

void Init(Local<Object> exports, Local<Object> module)
{
        MyObject::Init(module);
}

NODE_MODULE(addon, Init)
