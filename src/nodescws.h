#ifndef NODESCWS_H
#define NODESCWS_H

#define NODESCWS_MSG_ERR 1
#define NODESCWS_MSG_WARNING 2
#define NODESCWS_MSG_LOG 3

#include <nan.h>
#include "libscws/scws.h"

class Nodescws : public node::ObjectWrap {
        public:
        static void Init(v8::Handle<v8::Object> target);
        scws_t scws;
        scws_result *result_raw_;

        private:
        Nodescws();
        ~Nodescws();
        static NAN_METHOD(New);
        static NAN_METHOD(Segment);
        static NAN_METHOD(Destroy);
};

static void scws_log(int level, const char *msg, ...);
#endif
