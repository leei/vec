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

    'is properly initialized': function(v) {
      assert.equal(v.toString(), "/lll1");
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
    }
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
