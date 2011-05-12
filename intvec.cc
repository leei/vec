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

#include "intvec.h"

IntVec::~IntVec()
{
  if (vec) {
    //fprintf(stderr, "intvec: free vec @%p\n", vec);
    free(vec);
  }
}

static Persistent<FunctionTemplate> s_ct;

Handle<Value>
IntVec::New(const Arguments& args)
{
  HandleScope scope;
  IntVec* hw = new IntVec();

  // If there is an integer argument, then use that as initial length.
  if (args.Length() > 0) {
    if (args[0]->IsInt32()) {
      int32_t len = args[0]->Int32Value();
      //fprintf(stderr, "intvec: initial length %d\n", len);
      if (len < 0) {
        return ThrowException(Exception::TypeError(String::New("Bad argument")));
      }
      hw->extend(len);
    } else if (args[0]->IsString()) {
      hw->setString(Local<String>::Cast(args[0]));
    } else {
      return ThrowException(Exception::TypeError(String::New("Bad argument")));
    }
  }

  hw->Wrap(args.This());
  return args.This();
}

Handle<Value>
IntVec::GetLength(Local<String> property, const AccessorInfo& info)
{
  HandleScope scope;
  IntVec* hw = ObjectWrap::Unwrap<IntVec>(info.This());
  return scope.Close(Integer::New(hw->length));
}

Handle<Value>
IntVec::GetJSON(Local<String> property, const AccessorInfo& info)
{
  static Handle<String> prefix = String::New("IntVec:");
  IntVec* hw = ObjectWrap::Unwrap<IntVec>(info.This());
  Handle<Value> json = hw->toString();
  if (! json->IsString()) {
    return ThrowException(json);
  }

  HandleScope scope;
  return scope.Close(String::Concat(prefix, json->ToString()));
}

int32_t
IntVec::get(int idx)
{
  return vec[idx];
}

int32_t
IntVec::set(uint32_t idx, int32_t value)
{
  if (idx < length || value) {
    extend(idx+1);
    //fprintf(stderr, "intvec: set(%d,%d)\n", idx, value);
    vec[idx] = value;
  }
  return value;
}

void
IntVec::setString(Local<String> str) {
  int len = str->Utf8Length();
  char *data = (char *) malloc(len+1), *p, *q;
  str->WriteUtf8(data, len+1);

  int i = 0;
  for (p = data; p; ++i, p = index(p+1, ',')) {}
  //fprintf(stderr, "intvec: setString len %d\n", i);
  extend(i);

  int v;
  for (i = 0, p = data; (q = index(p, ',')); ++i, p = q+1) {
    sscanf(p, "%d", &v);
    set(i, v);
  }
  sscanf(p, "%d", &v);
  set(i, v);

  free(data);
}

Handle<Value>
IntVec::toString()
{
  Local<String> rep = String::New(""), sep = String::NewSymbol(",");

  char buffer[16];
  for (uint32_t i = 0; i < length; ++i) {
    int32_t val = vec[i];
    if (i > 0) { rep = String::Concat(rep, sep); }
    sprintf(buffer, "%d", val);
    rep = String::Concat(rep, String::New(buffer));
  }

  return rep;
}

Handle<Value>
IntVec::ToString(const Arguments& args)
{
  IntVec* hw = ObjectWrap::Unwrap<IntVec>(args.This());

  HandleScope scope;
  return scope.Close(hw->toString());
}

void
IntVec::extend(uint32_t len) {
  if (len < length) {
    return;
  } else {
    if (vec) {
      vec = (int32_t *) realloc(vec, len * sizeof(int32_t));
      //fprintf(stderr, "intvec: realloc %d ints @%p\n", len, vec);
      int32_t prev_len = length;
      bzero(vec + prev_len, (len - prev_len) * sizeof(int32_t));
    } else {
      vec = (int32_t *) calloc(len, sizeof(int32_t));
      //fprintf(stderr, "intvec: calloc %d ints @%p\n", len, vec);
    }

    //fprintf(stderr, "intvec: extend %d -> %d\n", length, word_len*32);
    length = len;
  }
}

/*
 * Get the value of the bit at [idx] of this vector.  Out of range values,
 * simply return false.
 */
Handle<Value>
IntVec::IndexGet(uint32_t idx, const AccessorInfo& info)
{
  IntVec* hw = ObjectWrap::Unwrap<IntVec>(info.This());

  int32_t retval = (idx >= hw->length ? 0 : hw->get(idx));
  //fprintf(stderr, "intvec: IndexGet(%d) => %d\n", idx, retval);

  return Integer::New(retval);
}

/*
 * Set the value of the bit at [idx] to this value. Out of range values,
 * extend the array.
 */
Handle<Value>
IntVec::IndexSet(uint32_t idx, Local<Value> value, const AccessorInfo& info)
{
  if (idx < 0) {
    return ThrowException(Exception::TypeError(String::New("Bad argument")));
  }

  IntVec* hw = ObjectWrap::Unwrap<IntVec>(info.This());

  //fprintf(stderr, "intvec: IndexSet(%d, %d)\n", idx, value->Int32Value());

  hw->set(idx, value->Int32Value());
  return value;
}

Handle<Value>
IntVec::Map(const Arguments& args)
{
  IntVec* hw = ObjectWrap::Unwrap<IntVec>(args.This());

  if (args.Length() < 1 || !args[0]->IsFunction()) {
    return ThrowException(Exception::TypeError(String::New("Argument must be a function")));
  }

  Local<Array> retval = Array::New(hw->length);
  Local<Function> cb = Local<Function>::Cast(args[0]);

  HandleScope scope;
  Handle<Object> global = Context::GetCurrent()->Global();

  Local<Value> argv[1];
  for (uint32_t i = 0; i < hw->length; ++i) {
    argv[0] = Int32::New(hw->vec[i]);
    retval->Set(i, cb->Call(global, 1, argv));
  }

  return scope.Close(retval);
}

Handle<Value>
IntVec::Reduce(const Arguments& args)
{
  IntVec* hw = ObjectWrap::Unwrap<IntVec>(args.This());

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
    argv[1] = Int32::New(hw->vec[i]);
    argv[0] = cb->Call(global, 2, argv);
  }

  return scope.Close(argv[0]);
}

void
IntVec::Init(Handle<Object> target)
{
  HandleScope scope;

  Local<FunctionTemplate> t = FunctionTemplate::New(New);

  s_ct = Persistent<FunctionTemplate>::New(t);
  s_ct->InstanceTemplate()->SetInternalFieldCount(1);
  s_ct->SetClassName(String::NewSymbol("IntVec"));

  NODE_SET_PROTOTYPE_METHOD(s_ct, "toString", ToString);

  NODE_SET_PROTOTYPE_METHOD(s_ct, "map", Map);
  NODE_SET_PROTOTYPE_METHOD(s_ct, "reduce", Reduce);

  s_ct->InstanceTemplate()->SetIndexedPropertyHandler(IndexGet, IndexSet);

  s_ct->InstanceTemplate()->SetAccessor(String::NewSymbol("length"), GetLength);
  s_ct->InstanceTemplate()->SetAccessor(String::NewSymbol("toJSON"), GetJSON);

  target->Set(String::NewSymbol("IntVec"), s_ct->GetFunction());
}
