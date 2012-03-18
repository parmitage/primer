var equals = function (a, b) {
    if (typeof(a) != typeof(b))
        return false;

    if (typeof(a) != 'object')
        return a === b;

    if(!a || !b)
        return false;

    if(a.length == b.length) {
        for(var i = 0; i < a.length;i++) {
            if(typeof a[i] == 'object') {
                if(!equals(a[i], b[i]))
                    return false;
            }
            else if(a[i] != b[i])
                return false;
        }
        
        return true;
    }
    else
        return false;
};

var head = function (xs) {
    if (xs.length == 0)
        return [];
    return xs[0];
};

var cons = function (x, l) {
    var l2 = l.slice(0);
    l2.unshift(x);
    return l2;
};

var range = function (start, end) {
    var array = [];
    for (var i = start; i <= end; i += 1) {
        array.push(i);
    }    
    return array;
};

var charCode = function (c) {
    return c.charCodeAt(0) - "0".charCodeAt(0);
}