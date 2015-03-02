#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif
#include <nan.h>
#include "nodescws.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
        Nodescws::Init(exports);
}

NODE_MODULE(scws, InitAll)
