var vows = require("vows"), assert = require('assert');
var vec = require("../build/default/vec");

var suite = vows.describe("IntVec");

suite.addBatch({
  'an intvec': {
    topic: function() {
      return new vec.IntVec();
    },

    'is initially empty': function(intvec) {
      assert.equal(intvec.length, 0);
    },

    'and returns false for all gets': function(intvec) {
      assert.equal(intvec.get(0), 0);
      assert.equal(intvec.get(1000), 0);
      assert.equal(intvec.get(-1), 0);
    },

    'on set': {
      topic: function(intvec) {
        return intvec.set(120, 12);
      },

      'returns the value': function(val) {
        assert.equal(val, 12);
      },

      'and': {
        topic: function(val, intvec) { return intvec; },

        'has length = index+1': function(intvec) {
          assert.equal(intvec.length, 121);
        },

        'returns the set value': function(intvec) {
          assert.equal(intvec.get(120), 12);
        },

        'intervening values are still zero': function(intvec) {
          assert.equal(intvec.get(0), 0);
          assert.equal(intvec.get(119), 0);
        }
      }
    }
  }
});

suite.addBatch({
  'an intvec initialized with a length': {
    topic: function() {
      return new vec.IntVec(100);
    },

    'has length >= initialized value': function(intvec) {
      assert.equal(intvec.length, 100);
    },

    'and returns false for all gets': function(intvec) {
      assert.equal(intvec.get(0), 0);
      assert.equal(intvec.get(1000), 0);
      assert.equal(intvec.get(-1), 0);
    }
  }
});

["1,2,3", "1", "-15,-17"].forEach(function (str) {
  var batch = {};
  batch['an intvec initialized with a "' + str + '"'] = {
    topic: function() {
      return new vec.IntVec(str);
    },

    'returns the same string': function(intvec) {
      assert.equal(intvec.toString(), str);
    },

    'and returns zero for other gets': function(intvec) {
      assert.equal(intvec.get(1000), 0);
      assert.equal(intvec.get(-1), 0);
    }
  };

  suite.addBatch(batch);
});

suite.addBatch({
  'an intvec with [2] = 3': {
    topic: function() {
      var v = new vec.IntVec();
      v.set(2, 3);
      return v;
    },

    'has string 0,0,3': function(intvec) {
      assert.equal(intvec.toString(), "0,0,3");
    }
  }
});

suite.export(module);