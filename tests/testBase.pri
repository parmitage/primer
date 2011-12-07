pragma GC_DISABLE;

using base;
using test;

##############################################################################
#
# This is the Primer test suite which should ultimately give 100% coverage of
# the interpreter.
#
##############################################################################

# test data

val a = 10;
val b = 15;
val c = 66.78;
val d = false;
val l = [1,2,3,4];
val s = "Hello, world!";

val f1 = fun x -> x + b + 2;
val f2 = fun x y z -> x * y - z;
val f3 = fun x -> fun y -> x * y;
val f4 = fun f -> f(2);
val double = fun x -> x * 2;
val sumWithFun = fun x y f -> f(x) + f(y);

val factorial = fun x ->
    if x == 0
    then 1
    else x * factorial(x - 1);

val greaterThan3 = fun x -> x > 3;
val listCountdown = fun x -> reverse(0..x);

val makeAdder = fun y -> fun a -> y + a;
val add1 = makeAdder(1);
val add2 = makeAdder(2);

### equality tests

assert("int equality", 1, 1);
assert("int variable equality", b, 15);
assert("float equality", 4.5, 4.5);
assert("float variable equality", c, 66.78);
assert("bool equality true", true, true);
assert("bool equality false", false, false);
assert("bool variable equality", d, false);
assert("string literal equality", "Hello, world!", "Hello, world!");
assert("string variable equality", s, s);
assert("string literal and variable equality", s, "Hello, world!");

### simple type checks

assert("integer type check", 10 is int, true);
assert("integer type check to fail", false is int, false);
assert("float type check", 10.1 is float, true);
assert("float type check to fail", 12 is float, false);
assert("char type check", 'z' is char, true);
assert("char type check to fail", 12 is char, false);
assert("boolean type check", false is bool, true);
assert("boolean type check to fail", 88 is bool, false);
assert("list type check", [1,2,3] is list, true);
assert("list type check to fail", 2 is list, false);
assert("empty list type check", [] is list, true);
assert("list variable type check", l is list, true);
#assert("string type check", "hello" is string, true);
#assert("string type check to fail", [1,2,3,4] is string, false);
#assert("string variable type check", s is string, true);
#assert("string variable type check to fail", l is string, false);
assert("function type check to pass", add1 is lambda, true);
assert("function type check to fail", add1 is int, false);

### type conversions

assert("int as float", 10 as float, 10.0);
assert("int as string", 123456 as string, "123456");
assert("int as bool true", 1 as bool, true);
assert("int as bool false", 0 as bool, false);
assert("float as string", 123.45 as string, "123.45");
assert("char as string", 'a' as string, "a");
assert("char as int", 'b' as int, 98);
assert("char as float", 'c' as float, 99.0);
assert("bool as int", true as int, 1);
assert("bool as float", false as float, 0.0);
assert("bool as string", true as string, "true");
assert("string as int", "123" as int, 123);
assert("string as float", "4.56" as float, 4.56);
assert("string as char", "c" as char, 'c');

### math and logic operators

assert("integer addition", 2 + 3, 5);
assert("integer multiplication", 2 * 3, 6);
assert("true or true", true or true, true);
assert("true or false", true or false, true);
assert("false or true", false or true, true);
assert("false or false", false or false, false);
assert("true and true", true and true, true);
assert("true and false", true and false, false);
assert("false and true", false and true, false);
assert("false and false", false and false, false);
assert("modulus", 11 mod 3, 2);
assert("not true", not true, false);
assert("not false", not false, true);
assert("not with equality", not 1 == 2, true);
assert("not with equality 2", not 1 == 1, false);
assert("unary minus", -10, -10);
assert("subtraction", 0-10, -10);
assert("precedence", 10 * 2 - 3, 17);
assert("complex precedence", 12 + 3 * 2 / 5 - 2, 11.2);
assert("unary minus with precedence", -10 * 2 - 3, -23);
assert("override precedence", 10 * (2 - 3), -10);
assert("greater than", 5 > 2, true);
assert("greater than fails", 5 > 9, false);
assert("less than", 2 < 5, true);
assert("less than fails", 2 < 1, false);
assert("less than or equal", 2 <= 5, true);
assert("less than or equal equal", 2 <= 2, true);
assert("less than or equal fails", 2 <= 1, false);
assert("greater than or equal", 9 >= 5, true);
assert("greater than or equal equal", 2 >= 2, true);
assert("greater than or equal fails", 2 >= 11, false);

### list operators

assert("empty list equality", [], []);
assert("short list equality", [2], [2]);
assert("multi-item list equality", [1,2,3,4,5], [1,2,3,4,5]);
assert("list of variables equality", [a, b], [10, 15]);
assert("nested list equality", [1,2,[3,[4],5,6,7],8,[9,10],11], [1,2,[3,[4],5,6,7],8,[9,10],11]);
assert("lists with symbols", [0,a,b], [0,10,15]);
assert("list append", [1,2,3] ++ [4,5,6], [1,2,3,4,5,6]);
assert("nested list append", [1,2,3,[4,5]] ++ [4,5,[5,5,5],6], [1,2,3,[4,5],4,5,[5,5,5],6]);
assert("string append", "this" ++ " is " ++ "a test", "this is a test");
#assert("char append equals string", ('a'::'b'::'c'::[]) == "abc", true);
assert("simple range", 1..5, [1,2,3,4,5]);
assert("length of nested list", length([1,2,[3,4,5,6,7],6]), 4);
assert("length of multiple list concatenations", length([9,10] ++ [11,12] ++ [13,14,15] ++ [16] ++ [17]), 9);
assert("concatenated ranges", 0..5 ++ 6..10 ++ 11..15, [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]);

### function application

assert("simple function application", factorial(5), 120);
assert("nested function application", f2(f1(2), f4(f1), 12), 349);
assert("self nested function application", double(double(double(2))), 16);
assert("function returns new list", listCountdown(5), [5,4,3,2,1,0]);
assert("higher order with literals", sumWithFun(2, 3, double), 10);
assert("higher order with variables", sumWithFun(a, b, double), 50);
assert("higher order with fun", sumWithFun(a, b, fun x -> 2 * x), 50);
assert("closure one", add1(2), 3);
assert("closure two", add2(2), 4);

### base library

assert("simple map", map(double, [1,2,3]), [2,4,6]);
assert("simple filter", filter(greaterThan3, [1,2,3,2,4,5,6]), [4,5,6]);
assert("map over concatenated lists", map(double, [1,2,3] ++ [4,5,6]), [2,4,6,8,10,12]);
assert("simple length", length(l), 4);
assert("test empty list", empty([]), true);
assert("test non-empty list", empty([1]), false);
assert("test non-empty list of different types", empty([true, 12.4, 10]), false);
assert("simple length test", length([1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17]), 17);
assert("length of concatenated list", length([1,2,3,4] ++ [1,2,3,4]), 8);
assert("simple head test", head([1,2,3]), 1);
assert("head of the tail of a list", head(tail([1,2,3])), 2);
assert("head of the tail of the tail of a list", head(tail(tail([1,2,3]))), 3);
assert("simple find", find(11, [4,6,10,2,4,11]), 11);
assert("failing find", find(12, [4,6,10,2,4,11]), false);
assert("replace", replace(2, 3, [1,2,3,2,4,5,2,7,7,4,2,3,4,2]), [1,3,3,3,4,5,3,7,7,4,3,3,4,3]);
assert("replace nothing", replace(0, 3, [1,2,3,2,4,5,2,7,7,4,2,3,4,2]), [1,2,3,2,4,5,2,7,7,4,2,3,4,2]);
assert("sum of list of int", sum([1,2,3]), 6);
assert("simple nth", [44,12,66,87] at 2, 66);
assert("nth with variable", (1..20) at a, 11);
assert("nth element is a list", [1,2,[3,4,5,6,7],8,9,10] at 2, [3,4,5,6,7]);
assert("length of list extracted by nth", length([1,2,[3,4,5,6,7],6] at 2), 5);
assert("length of the empty list", length([]), 0);
assert("combined list functions 1", sum(map(double, filter(fun x -> 10 mod x == 0, [1,2,3,4]))), 6);
assert("foldl with fun", foldl(fun x y -> x + y, 0, [1,2,3,4,5]), 15);
assert("foldr with fun", foldr(fun x y -> x + y, 0, [1,2,3,4,5]), 15);
assert("any with pass in middle of list", any(fun x -> x == 2, [1,2,3,4]), true);
assert("any with pass at start of list", any(fun x -> x == 2, [2,1,3,4]), true);
assert("any with pass at end of list", any(fun x -> x == 2, [1,3,4,2]), true);
assert("all passes", all(fun x -> (x * x) > 4, [3,4,5,6]), true);
assert("all fails", all(fun x -> x * x > 4, [2,3,4,5,6]), false);
assert("take nothing", take(0, [1,2,3]), []);
assert("take 1", take(1, [1,2,3]), [1]);
assert("take 2", take(2, [1,2,3]), [1,2]);
assert("take all", take(3, [1,2,3]), [1,2,3]);
assert("take more than list has", take(4, [1,2,3]), [1,2,3]);
assert("drop nothing", drop(0, [1,2,3,4,5]), [1,2,3,4,5]);
assert("drop 1", drop(1, [1,2,3,4,5]), [2,3,4,5]);
assert("drop all but 1", drop(4, [1,2,3,4,5]), [5]);
assert("drop all", drop(5, [1,2,3,4,5]), []);
assert("drop more than list has", drop(6, [1,2,3,4,5]), []);
assert("simple takewhile", takeWhile(fun x -> x < 3, [1,2,3,4,5]), [1,2]);
assert("takeWhile takes nothing", takeWhile(fun x -> x < 0, [1,2,3,4,5]), []);
assert("takeWhile takes everything", takeWhile(fun x -> x < 6, [1,2,3,4,5]), [1,2,3,4,5]);
assert("simple dropWhile", dropWhile(fun x -> x < 3, [1,2,3,4,5]), [3,4,5]);
assert("dropWhile drops nothing", dropWhile(fun x -> x < 0, [1,2,3,4,5]), [1,2,3,4,5]);
assert("dropWhile drops all but one", dropWhile(fun x -> x < 5, [1,2,3,4,5]), [5]);
assert("dropWhile drops everything", dropWhile(fun x -> x < 6, [1,2,3,4,5]), []);
assert("reverse a list", reverse([1,2,3]), [3,2,1]);
assert("reverse a two item list", reverse([1,2]), [2,1]);
assert("reverse a one item list", reverse([1]), [1]);
assert("reverse a list twice", reverse(reverse([1,2,3])), [1,2,3]);
assert("reverse a list three times", reverse(reverse(reverse([1,2,3]))), [3,2,1]);
assert("reverse the empty list", reverse([]), []);
assert("reverse a string", reverse("hello"), "olleh");
assert("simple sum", sum([1,2,3,4,5]), 15);
assert("sum of one element list", sum([1]), 1);
assert("sum of empty list", sum([]), 0);
assert("simple product", product([1,2,3,4,5]), 120);
assert("product of one element list", product([5]), 5);
assert("product of empty list", product([]), 0);
assert("sort", sort([7,2,4,5,1,3,8,6,9]), [1,2,3,4,5,6,7,8,9]);
assert("sort empty list", sort([]), []);
assert("simple min", min([7,2,4,5,1,3,8,6,9]), 1);
assert("min of one element list", min([7]), 7);
assert("min of empty list", min([]), []);
assert("simple max", max([7,2,4,5,1,3,8,6]), 8);
assert("max is last item", max([7,2,4,5,1,3,8,6,9]), 9);
assert("max is first item", max([7,2,4,5,1,3,6]), 7);
assert("max of empty list", max([]), []);
assert("simple last", last([1,2,3,4,5]), 5);
assert("last of one element list", last([1]), 1);
assert("last of empty list", last([]), []);
assert("odd is true", odd(1), true);
assert("odd if false", odd(2), false);
assert("even is false", even(1), false);
assert("even is true", even(2), true);
assert("string variable length", length(s), 13);
assert("string literal length", length("foo"), 3);
assert("string length", length(s), 13);
assert("string reverse", reverse("foo"), "oof");
assert("take from string", take(4, s), "Hell");
assert("drop from string", drop(4, s), "o, world!");
assert("head of string is a char", head(s), 'H');
assert("tail of string", tail(s), "ello, world!");
assert("sort a string", sort("qwertyuiopasdfghjklzxcvbnm"), "abcdefghijklmnopqrstuvwxyz");
assert("zip", zip([1,2,3], [4,5,6]), [[1,4],[2,5],[3,6]]);
assert("zip with short first list", zip([1,2,3], [4,5,6,7]), [[1,4],[2,5],[3,6]]);
assert("zip with short second list", zip([1,2,3,4], [4,5,6]), [[1,4],[2,5],[3,6]]);
assert("intersperse zero element list", intersperse('a', []), []);
assert("intersperse one element list", intersperse('a', [1]), [1]);
assert("intersperse two element list", intersperse('a', [1,2]), [1,'a',2]);
assert("intersperse integers", intersperse(0, [1,2,3,4,5]), [1,0,2,0,3,0,4,0,5]);
assert("intersperse string", intersperse(' ', "hello"), "h e l l o");