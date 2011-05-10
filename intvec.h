/* This code is PUBLIC DOMAIN, and is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND. See the accompanying
* LICENSE file.
*/

#include <v8.h>
#include <node.h>

using namespace node;
using namespace v8;

class IntVec: ObjectWrap
{
private:
  uint32_t length;
  int32_t *vec;

public:

  static void Init(Handle<Object> target);

 IntVec() : length(0), vec(0) {}
  ~IntVec();

  // Prototype methods.
  static Handle<Value> New(const Arguments& args);
  static Handle<Value> ToString(const Arguments& args);

  static Handle<Value> Map(const Arguments& args);
  static Handle<Value> Reduce(const Arguments& args);

  // Getter
  static Handle<Value> GetLength(Local<String> property, const AccessorInfo& info);

  static Handle<Value> IndexGet(uint32_t idx, const AccessorInfo& info);
  static Handle<Value> IndexSet(uint32_t idx, Local<Value> val, const AccessorInfo& info);

  // Internal manipulators
  int32_t get(int32_t idx);
  int32_t set(uint32_t idx, int32_t v);
  void extend(uint32_t len);
  void setString(Local<String> str);
};
