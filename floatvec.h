/* This code is PUBLIC DOMAIN, and is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND. See the accompanying
* LICENSE file.
*/

#include <v8.h>
#include <node.h>

using namespace node;
using namespace v8;

class FloatVec: ObjectWrap
{
private:
  int32_t length;
  float *vec;

public:

  static void Init(Handle<Object> target);

 FloatVec() : length(0), vec(0) {}
  ~FloatVec();

  // Prototype methods.
  static Handle<Value> New(const Arguments& args);
  static Handle<Value> Get(const Arguments& args);
  static Handle<Value> Set(const Arguments& args);
  static Handle<Value> ToString(const Arguments& args);

  static Handle<Value> Map(const Arguments& args);
  static Handle<Value> Reduce(const Arguments& args);

  // Getter
  static Handle<Value> GetLength(Local<String> property, const AccessorInfo& info);

  // Internal manipulators
  float get(int idx);
  float set(int idx, float v);
  void extend(int32_t len);
  void setString(Local<String> str);
};
