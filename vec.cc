/* This code is PUBLIC DOMAIN, and is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND. See the accompanying
* LICENSE file.
*/

#include <v8.h>
#include <node.h>

#include "bitvec.h"

using namespace node;
using namespace v8;

class IntVec: ObjectWrap
{
private:
  int length;
public:

  static Persistent<FunctionTemplate> s_ct;
  static void Init(Handle<Object> target)
  {
    HandleScope scope;

    Local<FunctionTemplate> t = FunctionTemplate::New(New);

    s_ct = Persistent<FunctionTemplate>::New(t);
    s_ct->InstanceTemplate()->SetInternalFieldCount(1);
    s_ct->SetClassName(String::NewSymbol("IntVec"));

    NODE_SET_PROTOTYPE_METHOD(s_ct, "hello", Hello);

    target->Set(String::NewSymbol("IntVec"),
                s_ct->GetFunction());
  }

  IntVec() : length(0)
  {
  }

  ~IntVec()
  {
  }

  static Handle<Value> New(const Arguments& args)
  {
    HandleScope scope;
    IntVec* hw = new IntVec();
    hw->Wrap(args.This());
    return args.This();
  }

  static Handle<Value> Hello(const Arguments& args)
  {
    HandleScope scope;
    //IntVec* hw = ObjectWrap::Unwrap<IntVec>(args.This());
    Local<String> result = String::New("Hello World");
    return scope.Close(result);
  }

  static Handle<Value> GetLength(const Arguments& args)
  {
    HandleScope scope;
    IntVec* hw = ObjectWrap::Unwrap<IntVec>(args.This());
    return scope.Close(Integer::New(hw->length));
  }

};

Persistent<FunctionTemplate> IntVec::s_ct;

extern "C" {
  static void init (Handle<Object> target)
  {
    BitVec::Init(target);
    IntVec::Init(target);
  }

  NODE_MODULE(vec, init);
}

