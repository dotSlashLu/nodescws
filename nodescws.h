#ifndef NODESCWS_H
#define NODESCWS_H

#include <node.h>
#include "libscws/scws.h"

class Nodescws : public node::ObjectWrap {
        public:
        static void Init(v8::Handle<v8::Object> target);
        scws_t instance_; 

        private:
        Nodescws();
        ~Nodescws();

        static v8::Handle<v8::Value> New(const v8::Arguments& args);
        static v8::Handle<v8::Value> Segment(const v8::Arguments& args);
};
#endif
