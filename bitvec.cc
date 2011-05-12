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
      if (hw->setString(Local<String>::Cast(args[0])) < 0) {
        return ThrowException(Exception::TypeError(String::New("Invalid BitVec string")));
      }
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

Handle<Value>
BitVec::GetJSON(Local<String> property, const AccessorInfo& info)
{
  BitVec* hw = ObjectWrap::Unwrap<BitVec>(info.This());

  HandleScope scope;
  return scope.Close(hw->toString(64, true));
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

const char TRANS[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYX+/";

int
BitVec::setString(Local<String> str) {
  uint32_t len = str->Utf8Length();
  char *buf = (char *) malloc(len+1);
  str->WriteUtf8(buf, len+1);
  //fprintf(stderr, "bitvec: fromString '%s'\n", buf);

  const char *data = buf, *p;
  uint32_t i, j;

  if (strncmp(data, "BitVec[", 7) == 0) { data += 7; }
  //fprintf(stderr, "bitvec: fromString '%s'\n", data);

  uint32_t bpc = 6;
  if (data[0] == '/') {
    bpc = 6; p = data+1;
  } else if (data[0] == '0') {
    if (data[1] == 'x') {
      bpc = 4; p = data+2;
    } else if (data[1] == 'b') {
      bpc = 1; p = data+2;
    } else {
      bpc = 3; p = data+1;
    }
  } else {
    //fprintf(stderr, "bitvec: bad prefix '%s'\n", data);
    return -1;
  }

  extend((len - (p-data))*bpc);
  for (i = 0; p < data+len; ++p) {
    if (*p == ']') { break; }
    const char *t = index(TRANS, *p);
    if (! t || t-TRANS > 1<<bpc) {
      //fprintf(stderr, "bitvec: bad char '%c'\n", *p);
      return -1;
    }
    int val = t - TRANS;
    //fprintf(stderr, "bitvec: %d %c => %d\n", i, *p, val);
    for (j = 0; j < bpc; ++j) {
      //fprintf(stderr, "bitvec: %d %02x&%02x => %d\n", i+j, val, (1<<j), val & (1<<j));
      if (val & (1<<j)) { set(i+j, 1); }
    }
    i += bpc;
  }

  //fprintf(stderr, "bitvec: \"%s\" => length %d\n", data, length);
  return length;
}

Handle<Value>
BitVec::ToString(const Arguments& args)
{
  uint32_t base = 64;
  if (args.Length() >= 1) {
    if (args[0]->IsUint32()) {
      base = args[0]->Uint32Value();
    } else {
      return ThrowException(Exception::TypeError(String::New("Base must be 2, 8, 16 or 64")));
    }
  }

  BitVec* hw = ObjectWrap::Unwrap<BitVec>(args.This());
  Handle<Value> str = hw->toString(base);
  if (! str->IsString()) { return ThrowException(str); }

  HandleScope scope;
  return scope.Close(str);
}

Handle<Value>
BitVec::toString(uint32_t base, bool json)
{
  uint32_t bits = 6, mask = 077;
  const char *prefix = "";

  switch (base) {
  case 2:  bits = 1; prefix = "0b"; mask = 01; break;
  case 8:  bits = 3; prefix = "0";  mask = 07; break;
  case 16: bits = 4; prefix = "0x"; mask = 0xf; break;
  case 64: bits = 6; prefix = "/";  mask = 077; break;
  default:
    return Exception::TypeError(String::New("Base must be 2, 8, 16 or 64"));
  }
  //fprintf(stderr, "bitvec: base %d bits %d prefix %s\n", base, bits, prefix);

  uint32_t buflen = (length+bits)/bits + 2 + (json ? 8 : 0);
  //fprintf(stderr, "bitvec: length %d buflen %d\n", length, buflen);
  char *buf = (char *) malloc(buflen+1), *p;
  buf[0] = '\0';
  if (json) { strcpy(buf, "BitVec["); }
  strcat(buf, prefix); p = index(buf, 0);
  //fprintf(stderr, "bitvec: len %d start %s\n", p-buf, buf);
  for (uint32_t i = 0; i < length; i += bits, ++p) {
    uint32_t w0 = vec[i/32], shft0 = (i%32), mask0 = mask << shft0;
    uint32_t idx0 = (w0&mask0) >> shft0;
    if (shft0+bits <= 32) {
      //fprintf(stderr, "%d: w0 %x|%x idx %d char %c\n", i, w0, mask0, idx0, TRANS[idx0]);
      *p = TRANS[idx0];
    } else {
      uint32_t w1 = vec[i/32+1], shft1 = (32-shft0), mask1 = mask >> shft1;
      idx0 |= (w1&mask1) << shft1;
      //fprintf(stderr, "%d: w0 %x|%x w1 %x|%x idx %d char %c\n", i, w0, mask0, w1, mask1, idx0, TRANS[idx0]);
      *p = TRANS[idx0];
    }
  }
  if (json) { *p++ = ']'; }
  //*p = 0;
  //fprintf(stderr, "bitvec: p - buf = %d\n", p-buf);
  //fprintf(stderr, "bitvec: str = '%s'\n", buf);

  HandleScope scope;
  Local<String> ret = String::New(buf, p-buf);
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
BitVec::ForEach(const Arguments& args)
{
  BitVec* hw = ObjectWrap::Unwrap<BitVec>(args.This());

  if (args.Length() < 1 || ! args[0]->IsFunction()) {
    return ThrowException(Exception::TypeError(String::New("Argument must be a function")));
  }

  Local<Function> cb = Local<Function>::Cast(args[0]);
  Handle<Object> global = Context::GetCurrent()->Global();

  Local<Value> argv[2];
  for (uint32_t i = 0; i < hw->length; ++i) {
    argv[0] = *(hw->get(i) ? True() : False());
    argv[1] = Int32::New(i);
    cb->Call(global, 2, argv);
  }

  HandleScope scope;
  return scope.Close(args.This());
}

Handle<Value>
BitVec::ForEachTrue(const Arguments& args)
{
  BitVec* hw = ObjectWrap::Unwrap<BitVec>(args.This());

  if (args.Length() < 1 || ! args[0]->IsFunction()) {
    return ThrowException(Exception::TypeError(String::New("Argument must be a function")));
  }

  Local<Function> cb = Local<Function>::Cast(args[0]);
  Handle<Object> global = Context::GetCurrent()->Global();

  Local<Value> argv[1];
  for (uint32_t i = 0; i < hw->length; ++i) {
    if (hw->get(i)) {
      argv[0] = Integer::New(i);
      cb->Call(global, 1, argv);
    }
  }

  HandleScope scope;
  return scope.Close(args.This());
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
  NODE_SET_PROTOTYPE_METHOD(s_ct, "forEach", ForEach);
  NODE_SET_PROTOTYPE_METHOD(s_ct, "forEachTrue", ForEachTrue);

  s_ct->InstanceTemplate()->SetIndexedPropertyHandler(IndexGet, IndexSet);

  s_ct->InstanceTemplate()->SetAccessor(String::NewSymbol("length"), GetLength);
  s_ct->InstanceTemplate()->SetAccessor(String::NewSymbol("JSON"), GetJSON);

  target->Set(String::NewSymbol("BitVec"), s_ct->GetFunction());
}
