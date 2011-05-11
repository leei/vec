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

#include "bitvec.h"

BitVec::~BitVec()
{
  if (vec) { free(vec); }
}

static Persistent<FunctionTemplate> s_ct;

Handle<Value>
BitVec::New(const Arguments& args)
{
  HandleScope scope;
  BitVec* hw = new BitVec();

  // If there is an integer argument, then use that as initial length.
  if (args.Length() > 0) {
    if (args[0]->IsInt32()) {
      int32_t len = args[0]->Int32Value();
      //fprintf(stderr, "bitvec: initial length %d\n", len);
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
BitVec::GetLength(Local<String> property, const AccessorInfo& info)
{
  HandleScope scope;
  BitVec* hw = ObjectWrap::Unwrap<BitVec>(info.This());
  return scope.Close(Integer::New(hw->length));
}

uint32_t
BitVec::get(uint32_t idx)
{
  uint32_t word = idx/32, mask = (1) << (idx%32);
  return vec[word] & mask;
}

uint32_t
BitVec::set(uint32_t idx, bool value)
{
  if (idx < length || value) {
    uint32_t word = idx/32, mask = (1) << (idx%32);
    extend(idx+1);

    if (value) {
      vec[word] |= mask;
    } else {
      vec[word] &= ~mask;
    }
  }
  return value;
}

const char TRANS[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYX/+";

void
BitVec::setString(Local<String> str) {
  int len = str->Utf8Length();
  char *data = (char *) malloc(len+1);
  str->WriteUtf8(data, len+1);
  //fprintf(stderr, "bitvec: fromString '%s'\n", data);
  const char *p;
  int i, j;

  extend(len*6);
  for (i = 0, p = data; p < data+len; ++p) {
    const char *t = index(TRANS, *p);
    if (! t) {
      ThrowException(Exception::TypeError(String::New("Unrecognized bitvec char")));
      return;
    }
    int val = t - TRANS;
    //fprintf(stderr, "bitvec: %d %c => %d\n", i, *p, val);
    for (j = 0; j < 6; ++j) {
      //fprintf(stderr, "bitvec: %d %02x&%02x => %d\n", i+j, val, (1<<j), val & (1<<j));
      if (val & (1<<j)) { set(i+j, 1); }
    }
    i += 6;
  }
}

Handle<Value>
BitVec::ToString(const Arguments& args)
{
  BitVec* hw = ObjectWrap::Unwrap<BitVec>(args.This());

  uint32_t buflen = (hw->length+5)/6;
  char *buf = (char *) malloc(buflen+1), *p = buf;
  for (uint32_t i = 0; i < hw->length; i += 6, ++p) {
    uint32_t w0 = hw->vec[i/32], shft0 = (i%32), mask0 = (0x3f) << shft0;
    uint32_t idx0 = (w0&mask0) >> shft0;
    if (shft0+6 <= 32) {
      //fprintf(stderr, "%d: w0 %x|%x idx %d char %c\n", i, w0, mask0, idx0, TRANS[idx0]);
      *p = TRANS[idx0];
    } else {
      uint32_t w1 = hw->vec[i/32+1], shft1 = (32-shft0), mask1 = (0x3f) >> shft1;
      idx0 |= (w1&mask1) << shft1;
      //fprintf(stderr, "%d: w0 %x|%x w1 %x|%x idx %d char %c\n", i, w0, mask0, w1, mask1, idx0, TRANS[idx0]);
      *p = TRANS[idx0];
    }
  }
  *p = 0;
  //fprintf(stderr, "bitvec: str = '%s'\n", buf);

  HandleScope scope;
  Handle<Value> ret = scope.Close(String::New(buf, buflen));
  free(buf);
  return ret;
}

void
BitVec::extend(uint32_t len) {
  uint32_t new_word_len = (len+31)/32;
  if (new_word_len <= word_len) {
    return;
  }

  if (vec) {
    vec = (uint32_t *) realloc(vec, new_word_len * sizeof(uint32_t));
    bzero(vec + word_len, (new_word_len - word_len) * sizeof(uint32_t));
  } else {
    vec = (uint32_t *) calloc(new_word_len, sizeof(uint32_t));
  }

  //fprintf(stderr, "bitvec: extend %d -> %d\n", length, word_len*32);
  length = len;
  word_len = new_word_len;
}

Handle<Value>
BitVec::IndexGet(uint32_t idx, const AccessorInfo& info)
{
  BitVec* hw = ObjectWrap::Unwrap<BitVec>(info.This());

  uint32_t retval = (idx >= hw->length ? 0 : hw->get(idx));
  //fprintf(stderr, "intvec: IndexGet(%d) => %d\n", idx, retval);

  return retval ? True() : False();
}

/*
 * Set the value of the bit at [idx] to this value. Out of range values,
 * extend the array.
 */
Handle<Value>
BitVec::IndexSet(uint32_t idx, Local<Value> value, const AccessorInfo& info)
{
  if (idx < 0) {
    return ThrowException(Exception::TypeError(String::New("Bad argument")));
  }

  BitVec* hw = ObjectWrap::Unwrap<BitVec>(info.This());

  //fprintf(stderr, "intvec: IndexSet(%d, %d)\n", idx, value->Int32Value());

  hw->set(idx, value->BooleanValue());
  return value;
}

Handle<Value>
BitVec::Map(const Arguments& args)
{
  BitVec* hw = ObjectWrap::Unwrap<BitVec>(args.This());

  if (args.Length() < 1 || !args[0]->IsFunction()) {
    return ThrowException(Exception::TypeError(String::New("Argument must be a function")));
  }

  Local<Array> retval = Array::New(hw->length);
  Local<Function> cb = Local<Function>::Cast(args[0]);

  HandleScope scope;
  Handle<Object> global = Context::GetCurrent()->Global();

  Local<Value> argv[1];
  for (uint32_t i = 0; i < hw->length; ++i) {
    argv[0] = *(hw->get(i) ? True() : False());
    retval->Set(i, cb->Call(global, 1, argv));
  }

  return scope.Close(retval);
}

Handle<Value>
BitVec::Reduce(const Arguments& args)
{
  BitVec* hw = ObjectWrap::Unwrap<BitVec>(args.This());

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
    argv[1] = *(hw->get(i) ? True() : False());
    argv[0] = cb->Call(global, 2, argv);
  }

  return scope.Close(argv[0]);
}

void
BitVec::Init(Handle<Object> target)
{
  HandleScope scope;

  Local<FunctionTemplate> t = FunctionTemplate::New(New);

  s_ct = Persistent<FunctionTemplate>::New(t);
  s_ct->InstanceTemplate()->SetInternalFieldCount(1);
  s_ct->SetClassName(String::NewSymbol("BitVec"));

  NODE_SET_PROTOTYPE_METHOD(s_ct, "toString", ToString);

  NODE_SET_PROTOTYPE_METHOD(s_ct, "map", Map);
  NODE_SET_PROTOTYPE_METHOD(s_ct, "reduce", Reduce);

  s_ct->InstanceTemplate()->SetIndexedPropertyHandler(IndexGet, IndexSet);
  s_ct->InstanceTemplate()->SetAccessor(String::NewSymbol("length"), GetLength);

  target->Set(String::NewSymbol("BitVec"), s_ct->GetFunction());
}
