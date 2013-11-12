#define BUILDING_NODE_EXTENSION
#include <node.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include "libscws/scws.h"

using namespace v8;
const char *V8StringToCString(v8::Handle<v8::Value>);

Handle<Value> Split(const Arguments& args) {
  HandleScope scope;

  /*
   *  scws(text, charset, dict, ignore_punct, multi);
   * */
  if (args.Length() < 1) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
    return scope.Close(Undefined());
  }

  if (!args[0]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Wrong arguments")));
    return scope.Close(Undefined());
  }

  // setup scws
  scws_t ret = scws_new();
  scws_set_charset(ret, "utf8");
  // int add_dict_ret = scws_add_dict(ret, "./dicts/dict.utf8.xdb", SCWS_XDICT_XDB ^ SCWS_XDICT_MEM);
  int add_dict_ret = scws_add_dict(ret, "./dicts/dict.utf8.xdb", SCWS_XDICT_XDB)/*&& scws_add_dict(ret, "./test-dict.txt", SCWS_XDICT_TXT)*/;
  scws_set_rule(ret, "./rules/rules.utf8.ini");
  if (add_dict_ret < 0)
    printf("add dict error\n");

  const char *const_text = V8StringToCString(args[0]);
  char *text = (char *)malloc(sizeof const_text * strlen(const_text));
  strcpy(text, const_text);

  // printf("segmenting: %s\tlength: %d\n", text, (int)strlen(text));
  scws_send_text(ret, text, strlen(text));
  scws_res_t res_count, res = scws_get_result(ret);

  int word_count = 0;
  while (res_count != NULL) {
    word_count++;
    res_count = res_count->next;
  }
  // printf("res count: %d\n", word_count);
  Handle<Array> array = Array::New(word_count + 1);
  int i = 0;
  while (res != NULL) {
    char *res_str = (char *)malloc(((int)res->len + 1) * sizeof(char));
    char *res_start = &text[(int)res->off];
    strncpy(res_str, res_start, (int)res->len);
    res_str[(int)res->len] = '\0';
    // printf("offset:%d length:%d word: %s\n", (int)res->off, (int)res->len, res_str);
    // printf("WORD: %.*s/%s (IDF = %4.2f)\n", res->len, text + res->off, res->attr, res->idf);
    Local<Object> obj = Object::New();
    obj->Set(String::NewSymbol("word"), String::New(res_str));
    obj->Set(String::NewSymbol("offset"), Number::New(res->off));
    obj->Set(String::NewSymbol("length"), Number::New(res->len));
    obj->Set(String::NewSymbol("attr"), String::New(res->attr));
    obj->Set(String::NewSymbol("idf"), Number::New(res->idf));
    array->Set(i++, obj);
    free(res_str);
    res_str = NULL;
    res = res->next;
  }
  scws_free_result(res);
  scws_free_result(res_count);
  scws_free(ret);

  return scope.Close(array);
}

void Init(Handle<Object> exports) {
  exports->Set(String::NewSymbol("segment"),
      FunctionTemplate::New(Split)->GetFunction());
}

const char *V8StringToCString(v8::Handle<v8::Value> value) {
  v8::String::Utf8Value value_str(value);
  std::string value_str_std = std::string(*value_str);
  return value_str_std.c_str();
}

NODE_MODULE(scws, Init)
