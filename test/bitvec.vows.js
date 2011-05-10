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
      assert.isFalse(bitvec.get(0));
      assert.isFalse(bitvec.get(1000));
      assert.isFalse(bitvec.get(-1));
    },

    'on set': {
      topic: function(bitvec) {
        return bitvec.set(120, true);
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
          assert.isFalse(bitvec.get(0));
          assert.isFalse(bitvec.get(119));
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
      assert.isFalse(bitvec.get(0));
      assert.isFalse(bitvec.get(1000));
      assert.isFalse(bitvec.get(-1));
    }
  }
});

suite.addBatch({
  'a bitvec initialized with sequence': {
    topic: function() {
      var v = new vec.BitVec(20);
      for (var i = 0; i < 20; ++i) { v.set(i, !(i%2)); }
      return v;
    },

    'is properly initialized': function(v) {
      assert.equal(v.toString().substr(0,4), "lll1");
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

["1000AFG", "jba87uygb890jhg+/kjAHJGGJHGgsh"].forEach(function (str) {
  var batch = {};
  batch['a bitvec initialized with a ' + str] = {
    topic: function() {
      return new vec.BitVec(str);
    },

    'has length >= 6 * string length': function(bitvec) {
      assert.isTrue(bitvec.length >= str.length*6);
    },

    'returns the string for toString()': function(bitvec) {
      assert.equal(bitvec.toString().substr(0,str.length), str);
    }
  }
  suite.addBatch(batch);
});

suite.export(module);
