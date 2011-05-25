var util = require("util");
var vec = require("../build/default/vec");

var IntVec = vec.IntVec;
var size = 100000000;

for (var i = 0; i < 3; ++i) {
  (function (x) {
    (function (y) {
      (function (z) {
        (function (a) {
          (function (b) {
            console.warn("size = " +
                         4*(x.length + y.length + z.length + a.length + b.length)/1e6 + "MB");
          })(new IntVec(size));
        })(new IntVec(size));
      })(new IntVec(size));
    })(new IntVec(size));
  })(new IntVec(size));

  new Array(100);
  console.warn("memUsage = " + util.inspect(process.memoryUsage()));
}
