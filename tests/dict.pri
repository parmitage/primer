use dict
use test

def d = []
def d1 = dict:add(d, "one", 1)
def d2 = dict:add(d1, "two", 2)
def d3 = dict:add(d2, "three", 3)

test:assert("structure", d3, [["three",3],["two",2],["one",1]])
test:assert("lookup", dict:get(d3, "one"), 1)
test:assert("lookup fails", dict:get(d3, "four"), nil)
test:assert("update first entry", dict:update(d3, "one", 5), [["three",3],["two",2],["one",5]])
test:assert("update middle entry", dict:update(d3, "two", 5), [["three",3],["two",5],["one",1]])
test:assert("update last entry", dict:update(d3, "three", 5), [["three",5],["two",2],["one",1]])
test:assert("update not present", dict:update(d3, "four", 5), [["three",3],["two",2],["one",1]])
test:assert("remove first entry", dict:remove(d3, "one"), [["three",3],["two",2]])
test:assert("remove middle entry", dict:remove(d3, "two"), [["three",3],["one",1]])
test:assert("remove last entry", dict:remove(d3, "three"), [["two",2],["one",1]])
test:assert("remove not present", dict:remove(d3, "four"), [["three",3],["two",2],["one",1]])