var vows = require("vows"), assert = require('assert');
var vec = require("../build/default/vec");

var suite = vows.describe("FloatVec");

suite.addBatch({
  'a floatvec': {
    topic: function() {
      return new vec.FloatVec();
    },

    'is initially empty': function(v) {
      assert.equal(v.length, 0);
    },

    'and returns false for all gets': function(v) {
      assert.equal(v[0], 0);
      assert.equal(v[1000], 0);
    },

    'on set': {
      topic: function(v) {
        return v[120] = 12;
      },

      'returns the value': function(val) {
        assert.equal(val, 12);
      },

      'and': {
        topic: function(val, v) { return v; },

        'has length = index+1': function(v) {
          assert.equal(v.length, 121);
        },

        'returns the set value': function(v) {
          assert.equal(v[120], 12);
        },

        'intervening values are still zero': function(v) {
          assert.equal(v[0], 0);
          assert.equal(v[119], 0);
        }
      }
    }
  }
});

suite.addBatch({
  'a floatvec initialized with a length': {
    topic: function() {
      return new vec.FloatVec(100);
    },

    'has length >= initialized value': function(v) {
      assert.equal(v.length, 100);
    },

    'and returns false for all gets': function(v) {
      assert.equal(v[0], 0);
      assert.equal(v[1000], 0);
    }
  }
});

suite.addBatch({
  'a floatvec initialized with sequence': {
    topic: function() {
      var v = new vec.FloatVec(20);
      for (var i = 0; i < 20; ++i) { v[i] = i+1; }
      return v;
    },

    'is properly initialized': function(v) {
      assert.equal(v.toString(), "1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20");
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
  batch['a floatvec initialized with a "' + str + '"'] = {
    topic: function() {
      return new vec.FloatVec(str);
    },

    'returns the same string': function(v) {
      assert.equal(v.toString(), str);
    },

    'and returns zero for other gets': function(v) {
      assert.equal(v[1000], 0);
    }
  };

  suite.addBatch(batch);
});

suite.addBatch({
  'a floatvec with [2] = 3': {
    topic: function() {
      var v = new vec.FloatVec();
      v[2] = 3;
      return v;
    },

    'has string 0,0,3': function(v) {
      assert.equal(v.toString(), "0,0,3");
    }
  }
});

suite.export(module);
