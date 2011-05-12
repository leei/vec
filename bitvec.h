/* This code is PUBLIC DOMAIN, and is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND. See the accompanying
* LICENSE file.
*/

#include <v8.h>
#include <node.h>

using namespace node;
using namespace v8;

class BitVec: ObjectWrap
{
 private:
  uint32_t length;   // Length of the vector in bits
  uint32_t word_len; // Length of the vector in uint32_t words
  uint32_t *vec;

 public:

  static void Init(Handle<Object> target);

  BitVec() : length(0), word_len(0), vec(0) {}
  ~BitVec();

  // Prototype methods.
  static Handle<Value> New(const Arguments& args);
  static Handle<Value> ToString(const Arguments& args);

  static Handle<Value> Map(const Arguments& args);
  static Handle<Value> Reduce(const Arguments& args);

  // Getters
  static Handle<Value> GetLength(Local<String> property, const AccessorInfo& info);
  static Handle<Value> GetJSON(Local<String> property, const AccessorInfo& info);

  // Index Getters
  static Handle<Value> IndexGet(uint32_t idx, const AccessorInfo& info);
  static Handle<Value> IndexSet(uint32_t idx, Local<Value> val, const AccessorInfo& info);

  // Internal manipulators
  uint32_t get(uint32_t idx);
  uint32_t set(uint32_t idx, bool v);
  void extend(uint32_t len);
  Handle<Value> setString(Local<String> str);
  Handle<Value> toString(uint32_t base);
};
