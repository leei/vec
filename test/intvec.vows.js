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

    'returns zero for all gets': function(intvec) {
      assert.equal(intvec[0], 0);
      assert.equal(intvec[1000], 0);
    },

    'after set': {
      topic: function(intvec) {
        return intvec[120] = 12;
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
          assert.equal(intvec[120], 12);
        },

        'intervening values are still zero': function(intvec) {
          assert.equal(intvec[0], 0);
          assert.equal(intvec[119], 0);
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
      assert.equal(intvec[0], 0);
      assert.equal(intvec[1000], 0);
    }
  }
});

suite.addBatch({
  'an intvec initialized with sequence': {
    topic: function() {
      var v = new vec.IntVec(20);
      for (var i = 0; i < 20; ++i) { v[i] = i+1; }
      return v;
    },

    'is properly initialized': function(intvec) {
      assert.equal(intvec.toString(), "1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20");
    },

    'can be mapped': {
      topic: function (vec) {
        return vec.map(function (x) { return x-1; });
      },

      'to a reduced array': function(result) {
        for (var i = 0; i < 20; ++i) {
          assert.equal(result[i], i);
        }
      }
    },

    'can be reduced': {
      topic: function (vec) {
        return vec.reduce(0, function (r, x) { return r + x; });
      },

      'to compute the sum': function (result) {
        assert.equal(result, 210);
      }
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
      assert.equal(intvec[1000], 0);
    }
  };

  suite.addBatch(batch);
});

suite.addBatch({
  'an intvec with [2] = 3': {
    topic: function() {
      var v = new vec.IntVec();
      v[2] = 3;
      return v;
    },

    'has string 0,0,3': function(intvec) {
      assert.equal(intvec.toString(), "0,0,3");
    }
  }
});

suite.export(module);
