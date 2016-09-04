#include <nan.h>
#include "nodescws.h"

using v8::Local;
using v8::Object;

void Init(Local<Object> exports, Local<Object> module)
{
        NodeScws::Init(module);
}

NODE_MODULE(addon, Init)
