/* This code is PUBLIC DOMAIN, and is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND. See the accompanying
* LICENSE file.
*/

#include <v8.h>
#include <node.h>

#include "bitvec.h"
#include "intvec.h"

using namespace node;
using namespace v8;

extern "C" {
  static void init (Handle<Object> target)
  {
    BitVec::Init(target);
    IntVec::Init(target);
  }

  NODE_MODULE(vec, init);
}

