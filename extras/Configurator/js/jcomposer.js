(function () {

// Extend code from underscorejs
var extend = function (obj) {
    for (var i = 1; i < arguments.length; ++i) {
        var source = arguments[i];
        for (var prop in source) {
            if (source[prop] !== undefined) {
                obj[prop] = source[prop];
            }
        }
    }
    return obj;
};

function jComposer(view, structure) {
    if (!(this instanceof arguments.callee)) {
        throw new Error("Constructor may not be called as a function");
    }
    if (!(view instanceof jDataView)) {
        view = new jDataView(view, undefined, undefined, true);
    }
    this.view = view;
    this.view.seek(0);
    this.structure = extend({}, jComposer.prototype.structure, structure);
}

function toInt(val) {
    if (typeof val === 'function') {
        val = val.call(this);
    }
    return val;
}

jComposer.prototype.structure = {
    uint8:   function (value) { this.view.setUint8(value);   },
    uint16:  function (value) { this.view.setUint16(value);  },
    uint32:  function (value) { this.view.setUint32(value);  },
    int8:    function (value) { this.view.setInt8(value);    },
    int16:   function (value) { this.view.setInt16(value);   },
    int32:   function (value) { this.view.setInt32(value);   },
    float32: function (value) { this.view.setFloat32(value); },
    float64: function (value) { this.view.setFloat64(value); },
    array:   function (value, type, length) {
        length = toInt.call(this, length);
        for (var i = 0; i < length; ++i) {
            this.compose([type], value[i]);
        }
    },
    seek: function (position, block) {
        position = toInt.call(this, position);
        if (block instanceof Function) {
            var old_position = this.view.tell();
            this.view.seek(position);
            var result = block.call(this);
            this.view.seek(old_position);
            return result;
        } else {
            return this.view.seek(position);
        }
    },
    tell: function () {
        return this.view.tell();
    },
    skip: function (offset) {
        offset = toInt.call(this, offset);
        this.view.seek(this.view.tell() + offset);
        return offset;
    }
};

jComposer.prototype.seek = jComposer.prototype.structure.seek;
jComposer.prototype.tell = jComposer.prototype.structure.tell;
jComposer.prototype.skip = jComposer.prototype.structure.skip;

jComposer.prototype.compose = function (structure, data) {
    dataType = this.structure;

    for(var i = 0; i < structure.length; i++) {
        dataType = dataType[structure[i]];
    }

    if (typeof data === 'number') {
        setter = structure in this.structure ? dataType : this.structure[dataType];
        setter.apply(this, [data]);
        return;
    }

    if (data instanceof Array) {
        setter = this.structure[dataType[0]];
        setter.apply(this, [data, dataType[1], dataType[2]]);
        return;
    }

    if (typeof data === 'object') {
        for (var key in data) {
            tempStructure = structure.slice(0);
            tempStructure.push(key);
            this.compose(tempStructure, data[key]);
        }
    }
};


var all = self;
all.jComposer = jComposer;

})();