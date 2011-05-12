var vows = require("vows"), assert = require('assert');
var vec = require("../build/default/vec");

var suite = vows.describe("BitVec");

suite.addBatch({
  'a bitvec': {
    topic: function() {
      return new vec.BitVec();
    },

    'is initially empty': function(bitvec) {
      assert.equal(bitvec.length, 0);
    },

    'and returns false for all gets': function(bitvec) {
      assert.isFalse(bitvec[0]);
      assert.isFalse(bitvec[1000]);
      assert.isUndefined(bitvec[-1]);
    },

    'on set': {
      topic: function(bitvec) {
        return bitvec[120] = true;
      },

      'returns true': function(val) {
        assert.isTrue(val);
      },

      'and': {
        topic: function(val, bitvec) { return bitvec; },

        'has length >= index': function(bitvec) {
          assert.isTrue(bitvec.length >= 120);
        },

        'intervening values are still false': function(bitvec) {
          assert.isFalse(bitvec[0]);
          assert.isFalse(bitvec[119]);
        }
      }
    }
  }
});


suite.addBatch({
  'a bitvec initialized with a length': {
    topic: function() {
      return new vec.BitVec(100);
    },

    'has length >= initialized value': function(bitvec) {
      assert.isTrue(bitvec.length >= 100);
    },

    'and returns false for all gets': function(bitvec) {
      assert.isFalse(bitvec[0]);
      assert.isFalse(bitvec[1000]);
      assert.isUndefined(bitvec[-1]);
    }
  }
});

suite.addBatch({
  'a bitvec initialized with sequence': {
    topic: function() {
      var v = new vec.BitVec(20);
      for (var i = 0; i < 20; ++i) { v[i] = !(i%2); }
      return v;
    },

    'is represented base 64': function(v) {
      assert.equal(v.toString(64), "/lll1");
    },

    'is represented base 16': function(v) {
      assert.equal(v.toString(16), "0x55555");
    },

    'is represented base 8': function(v) {
      assert.equal(v.toString(8), "05252521");
    },

    'is represented base 2': function(v) {
      assert.equal(v.toString(2), "0b10101010101010101010");
    },

    'is represented as JSON': function(v) {
      assert.equal(v.JSON, "BitVec[/lll1]");
    },

    'iterated with forEach': {
      topic: function (v) {
        var top = this;
        v.forEach(function (val, i) {
          top.callback(null, val, i);
        });
      },

      'returns all values': function(err, val, i) {
        assert.equal(val, !(i%2));
      }
    },

    'iterated with forEachTrue': {
      topic: function (v) {
        var top = this;
        v.forEachTrue(function (i) {
          top.callback(null, i);
        });
      },

      'returns all true values': function(err, i) {
        assert.isTrue(!(i%2));
      }
    },

    'can be mapped': {
      topic: function (v) {
        return v.map(function (x) { return !x; });
      },

      'to a reduced array': function(result) {
        for (var i = 0; i < 20; ++i) {
          assert.equal(result[i], !!(i%2));
        }
      }
    },

    'can be reduced': {
      topic: function (vec) {
        return vec.reduce(0, function (r, x) { return r | x; });
      },

      'to compute the sum': function (result) {
        assert.equal(result, true);
      }
    }/*,

    'has JSON': {
      topic: function(vec) {
        return vec.JSON;
      },

      'that begins with BitVec[': function(json) {
        assert.equal(json.substr(0,7), "BitVec[");
      },

      'that reads back': {
        topic: function(json, v) {
          return new vec.BitVec(json);
        },

        'without change': function (restore) {
          var topics = this.topics;
          console.warn("bitvec: topics " + util.inspect(topics));
          assert.isTrue(true);
        }
      }
    }
     */
  }
});

(function (strings) {
  for (var str in strings) {
    var batch = {};
    batch['a bitvec initialized with ' + str] = {
      topic: function() {
        return new vec.BitVec(str);
      },

      'returns the string for toString()': function(bitvec) {
        assert.equal(bitvec.toString(strings[str]), str);
      },

      'returns a framed string for JSON': function(bitvec) {
        var json = bitvec.JSON;
        assert.equal(bitvec.JSON.substr(0,7), "BitVec[");
        assert.equal(bitvec.JSON.substr(-1), "]");
      }
    }
    suite.addBatch(batch);
  }
})({
  "/1000AFG": 64,
  "/jba87uygb890jhg+/kjAHJGGJHGgsh": 64,
  "0b111001001000100": 2,
  "07774543425": 8,
  "0x765120aff876876786": 16
});

suite.export(module);
