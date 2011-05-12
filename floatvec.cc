/* This code is PUBLIC DOMAIN, and is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND. See the accompanying
* LICENSE file.
*/

#include <v8.h>
#include <node.h>

#include <stdlib.h>
#include <strings.h>

using namespace node;
using namespace v8;

#include "floatvec.h"

FloatVec::~FloatVec()
{
  if (vec) {
    //fprintf(stderr, "floatvec: free vec @%p\n", vec);
    free(vec);
  }
}

static Persistent<FunctionTemplate> s_ct;

Handle<Value>
FloatVec::New(const Arguments& args)
{
  HandleScope scope;
  FloatVec* hw = new FloatVec();

  // If there is an integer argument, then use that as initial length.
  if (args.Length() > 0) {
    if (args[0]->IsInt32()) {
      int32_t len = args[0]->Int32Value();
      //fprintf(stderr, "floatvec: initial length %d\n", len);
      if (len < 0) {
        return ThrowException(Exception::TypeError(String::New("Bad argument")));
      }
      hw->extend(len);
    } else if (args[0]->IsString()) {
      //fprintf(stderr, "floatvec: new from string\n");
      if (hw->setString(Local<String>::Cast(args[0])) < 0) {
        return ThrowException(Exception::TypeError(String::New("Invalid FloatVec string")));
      }
    } else {
      return ThrowException(Exception::TypeError(String::New("Bad argument")));
    }
  }

  hw->Wrap(args.This());
  return args.This();
}

Handle<Value>
FloatVec::GetLength(Local<String> property, const AccessorInfo& info)
{
  HandleScope scope;
  FloatVec* hw = ObjectWrap::Unwrap<FloatVec>(info.This());
  return scope.Close(Integer::New(hw->length));
}

Handle<Value>
FloatVec::GetJSON(Local<String> property, const AccessorInfo& info)
{
  FloatVec* hw = ObjectWrap::Unwrap<FloatVec>(info.This());

  HandleScope scope;
  return scope.Close(hw->toString(true));
}

float
FloatVec::get(uint32_t idx)
{
  return vec[idx];
}

float
FloatVec::set(uint32_t idx, float value)
{
  if (idx < length || value) {
    extend(idx+1);
    //fprintf(stderr, "floatvec: set(%d,%d)\n", idx, value);
    vec[idx] = value;
  }
  return value;
}

int
FloatVec::setString(Local<String> str) {
  int len = str->Utf8Length();
  char *data = (char *) malloc(len+1);
  str->WriteUtf8(data, len+1);

  //fprintf(stderr, "floatvec: setString %s\n", data);
  const char *start = data, *p, *q;
  if (strncmp(start, "FloatVec[", 9) == 0) { start += 9; }

  int i = 0;
  for (p = start; p; ++i, p = index(p+1, ',')) {}
  //fprintf(stderr, "floatvec: setString len %d\n", i);
  extend(i);

  float v;
  for (i = 0, p = start; (q = index(p, ',')); ++i, p = q+1) {
    if (sscanf(p, "%f", &v) < 1) { return -1; }
    set(i, v);
  }
  if (sscanf(p, "%f", &v) < 1) { return -1; }
  set(i, v);

  free(data);
  return length;
}

Handle<Value>
FloatVec::ToString(const Arguments& args)
{
  FloatVec* hw = ObjectWrap::Unwrap<FloatVec>(args.This());

  HandleScope scope;
  return scope.Close(hw->toString());
}

Handle<Value>
FloatVec::toString(bool json)
{
  Local<String> sep = String::NewSymbol(",");
  Local<String> rep = json ? String::New("FloatVec[") : String::New("");

  char buffer[16];
  for (uint32_t i = 0; i < length; ++i) {
    float val = vec[i];
    if (i > 0) { rep = String::Concat(rep, sep); }
    sprintf(buffer, "%g", val);
    rep = String::Concat(rep, String::New(buffer));
  }
  if (json) { rep = String::Concat(rep, String::New("]")); }
  return rep;
}

void
FloatVec::extend(uint32_t len) {
  if (len < length) {
    return;
  } else {
    if (vec) {
      vec = (float *) realloc(vec, len * sizeof(float));
      //fprintf(stderr, "floatvec: realloc %d ints @%p\n", len, vec);
      int32_t prev_len = length;
      bzero(vec + prev_len, (len - prev_len) * sizeof(float));
    } else {
      vec = (float *) calloc(len, sizeof(float));
      //fprintf(stderr, "floatvec: calloc %d ints @%p\n", len, vec);
    }

    //fprintf(stderr, "floatvec: extend %d -> %d\n", length, word_len*32);
    length = len;
  }
}

/*
 * Get the value of the bit at [idx] of this vector.  Out of range values,
 * simply return zero.
 */
Handle<Value>
FloatVec::IndexGet(uint32_t idx, const AccessorInfo& info)
{
  FloatVec* hw = ObjectWrap::Unwrap<FloatVec>(info.This());

  float retval = (idx >= hw->length ? 0 : hw->get(idx));
  //fprintf(stderr, "intvec: IndexGet(%d) => %d\n", idx, retval);

  return Number::New(retval);
}

/*
 * Set the value of the bit at [idx] to this value. Out of range values,
 * extend the array.
 */
Handle<Value>
FloatVec::IndexSet(uint32_t idx, Local<Value> value, const AccessorInfo& info)
{
  if (idx < 0) {
    return ThrowException(Exception::TypeError(String::New("Bad argument")));
  }

  FloatVec* hw = ObjectWrap::Unwrap<FloatVec>(info.This());

  //fprintf(stderr, "intvec: IndexSet(%d, %d)\n", idx, value->Int32Value());

  hw->set(idx, value->NumberValue());
  return value;
}

Handle<Value>
FloatVec::ForEach(const Arguments& args)
{
  FloatVec* hw = ObjectWrap::Unwrap<FloatVec>(args.This());

  if (args.Length() < 1 || ! args[0]->IsFunction()) {
    return ThrowException(Exception::TypeError(String::New("Argument must be a function")));
  }

  Local<Function> cb = Local<Function>::Cast(args[0]);
  Handle<Object> global = Context::GetCurrent()->Global();

  Local<Value> argv[2];
  for (uint32_t i = 0; i < hw->length; ++i) {
    argv[0] = Number::New(hw->get(i));
    argv[1] = Int32::New(i);
    cb->Call(global, 2, argv);
  }

  HandleScope scope;
  return scope.Close(args.This());
}

Handle<Value>
FloatVec::Map(const Arguments& args)
{
  FloatVec* hw = ObjectWrap::Unwrap<FloatVec>(args.This());

  if (args.Length() < 1 || !args[0]->IsFunction()) {
    return ThrowException(Exception::TypeError(String::New("Argument must be a function")));
  }

  Local<Array> retval = Array::New(hw->length);
  Local<Function> cb = Local<Function>::Cast(args[0]);

  HandleScope scope;
  Handle<Object> global = Context::GetCurrent()->Global();

  Local<Value> argv[1];
  for (uint32_t i = 0; i < hw->length; ++i) {
    argv[0] = Number::New(hw->vec[i]);
    retval->Set(i, cb->Call(global, 1, argv));
  }

  return scope.Close(retval);
}

Handle<Value>
FloatVec::Reduce(const Arguments& args)
{
  FloatVec* hw = ObjectWrap::Unwrap<FloatVec>(args.This());

  if (args.Length() < 1) {
    return ThrowException(Exception::TypeError(String::New("Must provide a reduce argument")));
  } else if (args.Length() < 2 || !args[1]->IsFunction()) {
    return ThrowException(Exception::TypeError(String::New("Argument must be a function")));
  }

  Local<Function> cb = Local<Function>::Cast(args[1]);

  HandleScope scope;
  Handle<Object> global = Context::GetCurrent()->Global();

  Local<Value> argv[2];
  argv[0] = args[0];
  for (uint32_t i = 0; i < hw->length; ++i) {
    argv[1] = Number::New(hw->vec[i]);
    argv[0] = cb->Call(global, 2, argv);
  }

  return scope.Close(argv[0]);
}

void
FloatVec::Init(Handle<Object> target)
{
  HandleScope scope;

  Local<FunctionTemplate> t = FunctionTemplate::New(New);

  s_ct = Persistent<FunctionTemplate>::New(t);
  s_ct->InstanceTemplate()->SetInternalFieldCount(1);
  s_ct->SetClassName(String::NewSymbol("FloatVec"));

  NODE_SET_PROTOTYPE_METHOD(s_ct, "toString", ToString);

  NODE_SET_PROTOTYPE_METHOD(s_ct, "forEach", ForEach);
  NODE_SET_PROTOTYPE_METHOD(s_ct, "map", Map);
  NODE_SET_PROTOTYPE_METHOD(s_ct, "reduce", Reduce);

  s_ct->InstanceTemplate()->SetIndexedPropertyHandler(IndexGet, IndexSet);
  s_ct->InstanceTemplate()->SetAccessor(String::NewSymbol("length"), GetLength);
  s_ct->InstanceTemplate()->SetAccessor(String::NewSymbol("JSON"), GetJSON);

  target->Set(String::NewSymbol("FloatVec"), s_ct->GetFunction());
}
