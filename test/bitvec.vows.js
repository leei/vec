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
