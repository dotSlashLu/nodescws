#define BUILDING_NODE_EXTENSION
#include <node.h>
#include "nodescws.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
        Nodescws::Init(exports);
}

NODE_MODULE(scws, InitAll)
