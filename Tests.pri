##############################################################################
#
# This is the Primer test suite. The first section comprises data and
# functions for the test cases which follow.
#
##############################################################################

a: 10
b: 15
c: 66.78
d: false
l: [1,2,3,4]
s: "Hello, world!"

F1: fn (x) x + b + 2 end
F2: fn (x, y, z) x * y - z end

F3: fn ()
   Inner() where Inner: fn () 12 end
end

F4: fn (f)
  f(2)
end

Double: fn(x)
  x * 2
end

SumWithFun: fn(x, y, f)
  f(x) + f(y)
end

Factorial: fn(x)
  if x == 0 then 1
  else x * Factorial(x - 1)
end

GreaterThan3: fn (x)
  x > 3
end

ListCountdown: fn (x)
   Reverse(0..x)
end

MakeAdder: fn (y)
  fn (a) y + a end
end

Add1: MakeAdder(1)
Add2: MakeAdder(2)

### equality tests

Assert("int equality", 1, 1)
Assert("int variable equality", b, 15)
Assert("float equality", 4.5, 4.5)
Assert("float variable equality", c, 66.78)
Assert("bool equality true", true, true)
Assert("bool equality false", false, false)
Assert("bool variable equality", d, false)
Assert("string literal equality", "Hello, world!", "Hello, world!")
Assert("string variable equality", s, s)
Assert("string literal and variable equality", s, "Hello, world!")

### simple type checks

Assert("integer type check", IsInt(10), true)
Assert("integer type check to fail", IsInt(false), false)
Assert("float type check", IsFloat(10.1), true)
Assert("float type check to fail", IsFloat(12), false)
Assert("char type check", IsChar('z'), true)
Assert("char type check to fail", IsChar(12), false)
Assert("boolean type check", IsBool(false), true)
Assert("boolean type check to fail", IsBool(88), false)
Assert("list type check", IsList([1,2,3]), true)
Assert("list type check to fail", IsList(2), false)
Assert("empty list type check", IsList([]), true)
Assert("list variable type check", IsList(l), true)
Assert("string type check", IsString("hello"), true)
Assert("string type check to fail", IsString([1,2,3,4]), false)
Assert("string variable type check", IsString(s), true)
Assert("string variable type check to fail", IsString(l), false)

### type conversions

Assert("int as float", 10 as float, 10.0)
Assert("int as string", 123456 as string, "123456")
Assert("int as bool true", 1 as bool, true)
Assert("int as bool false", 0 as bool, false)
Assert("float as string", 123.45 as string, "123.45")
Assert("char as string", 'a' as string, "a")
Assert("char as int", 'b' as int, 98)
Assert("char as float", 'c' as float, 99.0)
Assert("bool as int", true as int, 1)
Assert("bool as float", false as float, 0.0)
Assert("bool as string", true as string, "true")
Assert("string as int", "123" as int, 123)
Assert("string as float", "4.56" as float, 4.56)
Assert("string as char", "c" as char, 'c')

### math and logic operators

Assert("integer addition", 2 + 3, 5)
Assert("integer multiplication", 2 * 3, 6)
Assert("true or true", true or true, true)
Assert("true or false", true or false, true)
Assert("false or true", false or true, true)
Assert("false or false", false or false, false)
Assert("true and true", true and true, true)
Assert("true and false", true and false, false)
Assert("false and true", false and true, false)
Assert("false and false", false and false, false)
Assert("modulus", 11 mod 3, 2)
Assert("not true", not true, false)
Assert("not false", not false, true)
Assert("not with equality", not 1 == 2, true)
Assert("not with equality 2", not 1 == 1, false)
Assert("unary minus", -10, -10)
Assert("subtraction", 0-10, -10)
Assert("precedence", 10 * 2 - 3, 17)
Assert("complex precedence", 12 + 3 * 2 / 5 mod 2, 13)
Assert("unary minus with precedence", -10 * 2 - 3, -23)
Assert("override precedence", 10 * (2 - 3), -10)
Assert("nested precedence override", 3 + (2 * ((9 - (8 / 2)) + 1)), 15)
Assert("greater than", 5 > 2, true)
Assert("greater than fails", 5 > 9, false)
Assert("less than", 2 < 5, true)
Assert("less than fails", 2 < 1, false)
Assert("less than or equal", 2 <= 5, true)
Assert("less than or equal equal", 2 <= 2, true)
Assert("less than or equal fails", 2 <= 1, false)
Assert("greater than or equal", 9 >= 5, true)
Assert("greater than or equal equal", 2 >= 2, true)
Assert("greater than or equal fails", 2 >= 11, false)

### list operators

Assert("empty list equality", [], [])
Assert("short list equality", [2], [2])
Assert("multi-item list equality", [1,2,3,4,5], [1,2,3,4,5])
Assert("list of variables equality", [a, b], [10, 15])
Assert("nested list equality", [1,2,[3,[4],5,6,7],8,[9,10],11], [1,2,[3,[4],5,6,7],8,[9,10],11])
Assert("lists with symbols", [0,a,b], [0,10,15])
Assert("list append", [1,2,3] ++ [4,5,6], [1,2,3,4,5,6])
Assert("nested list append", [1,2,3,[4,5]] ++ [4,5,[5,5,5],6], [1,2,3,[4,5],4,5,[5,5,5],6])
Assert("string append", "this" ++ " is " ++ "a test", "this is a test")
Assert("char append equals string", ('a'::'b'::'c'::[]) == "abc", true)
Assert("simple range", 1..5, [1,2,3,4,5])
Assert("length of nested list", Length([1,2,[3,4,5,6,7],6]), 4)
Assert("length of multiple list concatenations", Length([9,10] ++ [11,12] ++ [13,14,15] ++ [16] ++ [17]), 9)
Assert("concatenated ranges", 0..5 ++ 6..10 ++ 11..15, [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15])

### function application

Assert("simple function application", Factorial(5), 120)
Assert("nested function application", F2(F3(), F4(F1), 12), 216)
Assert("self nested function application", Double(Double(Double(2))), 16)
Assert("function returns new list", ListCountdown(5), [5,4,3,2,1,0])
Assert("higher order with literals", SumWithFun(2, 3, Double), 10)
Assert("higher order with variables", SumWithFun(a, b, Double), 50)
Assert("higher order with fn", SumWithFun(a, b, fn (x) 2 * x end), 50)
Assert("closure one", Add1(2), 3)
Assert("closure two", Add2(2), 4)

### standard library

Assert("simple map", Map(Double, [1,2,3]), [2,4,6])
Assert("simple filter", Filter(GreaterThan3, [1,2,3,2,4,5,6]), [4,5,6])
Assert("map over concatenated lists", Map(Double, [1,2,3] ++ [4,5,6]), [2,4,6,8,10,12])
Assert("simple length", Length(l), 4)
Assert("test empty list", Empty([]), true)
Assert("test non-empty list", Empty([1]), false)
Assert("test non-empty list of different types", Empty([true, 12.4, 10]), false)
Assert("simple length test", Length([1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17]), 17)
Assert("length of concatenated list", Length([1,2,3,4] ++ [1,2,3,4]), 8)
Assert("simple head test", Head([1,2,3]), 1)
Assert("head of the tail of a list", Head(Tail([1,2,3])), 2)
Assert("head of the tail of the tail of a list", Head(Tail(Tail([1,2,3]))), 3)
Assert("simple find", Find(11, [4,6,10,2,4,11]), 11)
Assert("failing find", Find(12, [4,6,10,2,4,11]), false)
Assert("replace", Replace(2, 3, [1,2,3,2,4,5,2,7,7,4,2,3,4,2]), [1,3,3,3,4,5,3,7,7,4,3,3,4,3])
Assert("replace nothing", Replace(0, 3, [1,2,3,2,4,5,2,7,7,4,2,3,4,2]), [1,2,3,2,4,5,2,7,7,4,2,3,4,2])
Assert("sum of list of int", Sum([1,2,3]), 6)
Assert("simple nth", [44,12,66,87] at 2, 66)
Assert("nth with variable", (1..20) at a, 11)
Assert("nth element is a list", [1,2,[3,4,5,6,7],8,9,10] at 2, [3,4,5,6,7])
Assert("length of list extracted by nth", Length([1,2,[3,4,5,6,7],6] at 2), 5)
Assert("length of the empty list", Length([]), 0)
Assert("combined list functions 1", Sum(Map(Double, Filter(fn (x) 10 mod x == 0 end, [1,2,3,4]))), 6)
Assert("foldl with fn", FoldL(fn (x, y) x + y end, 0, [1,2,3,4,5]), 15)
Assert("foldr with fn", FoldR(fn (x, y) x + y end, 0, [1,2,3,4,5]), 15)
Assert("any with pass in middle of list", Any(fn (x) x == 2 end, [1,2,3,4]), true)
Assert("any with pass at start of list", Any(fn (x) x == 2 end, [2,1,3,4]), true)
Assert("any with pass at end of list", Any(fn (x) x == 2 end, [1,3,4,2]), true)
Assert("all passes", All(fn (x) (x * x) > 4 end, [3,4,5,6]), true)
Assert("all fails", All(fn (x) x * x > 4 end, [2,3,4,5,6]), false)
Assert("take nothing", Take(0, [1,2,3]), [])
Assert("take 1", Take(1, [1,2,3]), [1])
Assert("take 2", Take(2, [1,2,3]), [1,2])
Assert("take all", Take(3, [1,2,3]), [1,2,3])
Assert("take more than list has", Take(4, [1,2,3]), [1,2,3])
Assert("drop nothing", Drop(0, [1,2,3,4,5]), [1,2,3,4,5])
Assert("drop 1", Drop(1, [1,2,3,4,5]), [2,3,4,5])
Assert("drop all but 1", Drop(4, [1,2,3,4,5]), [5])
Assert("drop all", Drop(5, [1,2,3,4,5]), [])
Assert("drop more than list has", Drop(6, [1,2,3,4,5]), [])
Assert("reverse a list", Reverse([1,2,3]), [3,2,1])
Assert("reverse a two item list", Reverse([1,2]), [2,1])
Assert("reverse a one item list", Reverse([1]), [1])
Assert("reverse a list twice", Reverse(Reverse([1,2,3])), [1,2,3])
Assert("reverse a list three times", Reverse(Reverse(Reverse([1,2,3]))), [3,2,1])
Assert("reverse the empty list", Reverse([]), [])
Assert("reverse a string", Reverse("hello"), "olleh")
Assert("simple takewhile", TakeWhile(fn (x) x < 3 end, [1,2,3,4,5]), [1,2])
Assert("takewhile takes nothing", TakeWhile(fn (x) x < 0 end, [1,2,3,4,5]), [])
Assert("takewhile takes everything", TakeWhile(fn (x) x < 6 end, [1,2,3,4,5]), [1,2,3,4,5])
Assert("dropwhile drops nothing", DropWhile(fn (x) x < 0 end, [1,2,3,4,5]), [1,2,3,4,5])
Assert("simple dropwhile", DropWhile(fn (x) x < 3 end, [1,2,3,4,5]), [3,4,5])
Assert("dropwhile drops all but one", DropWhile(fn (x) x < 5 end, [1,2,3,4,5]), [5])
Assert("dropwhile drops everything", DropWhile(fn (x) x < 6 end, [1,2,3,4,5]), [])
Assert("simple sum", Sum([1,2,3,4,5]), 15)
Assert("sum of one element list", Sum([1]), 1)
Assert("sum of empty list", Sum([]), 0)
Assert("simple product", Product([1,2,3,4,5]), 120)
Assert("product of one element list", Product([5]), 5)
Assert("product of empty list", Product([]), 0)
Assert("sort", Sort([7,2,4,5,1,3,8,6,9]), [1,2,3,4,5,6,7,8,9])
Assert("sort empty list", Sort([]), [])
Assert("simple min", Min([7,2,4,5,1,3,8,6,9]), 1)
Assert("min of one element list", Min([7]), 7)
Assert("min of empty list", Min([]), [])
Assert("simple max", Max([7,2,4,5,1,3,8,6]), 8)
Assert("max is last item", Max([7,2,4,5,1,3,8,6,9]), 9)
Assert("max is first item", Max([7,2,4,5,1,3,6]), 7)
Assert("max of empty list", Max([]), [])
Assert("simple last", Last([1,2,3,4,5]), 5)
Assert("last of one element list", Last([1]), 1)
Assert("last of empty list", Last([]), [])
Assert("odd is true", Odd(1), true)
Assert("odd if false", Odd(2), false)
Assert("even is false", Even(1), false)
Assert("even is true", Even(2), true)
Assert("string variable length", Length(s), 13)
Assert("string literal length", Length("foo"), 3)
Assert("sort a string", Sort("qwertyuiopasdfghjklzxcvbnm"), "abcdefghijklmnopqrstuvwxyz")
Assert("string length", Length(s), 13)
Assert("string reverse", Reverse("foo"), "oof")
Assert("take from string", Take(4, s), "Hell")
Assert("drop from string", Drop(4, s), "o, world!")
Assert("head of string is a char", Head(s), 'H')
Assert("tail of string", Tail(s), "ello, world!")
Assert("zip", Zip([1,2,3], [4,5,6]), [[1,4],[2,5],[3,6]])
Assert("zip with short first list", Zip([1,2,3], [4,5,6,7]), [[1,4],[2,5],[3,6]])
Assert("zip with short second list", Zip([1,2,3,4], [4,5,6]), [[1,4],[2,5],[3,6]])
Assert("intersperse zero element list", Intersperse('a', []), [])
Assert("intersperse one element list", Intersperse('a', [1]), [1])
Assert("intersperse two element list", Intersperse('a', [1,2]), [1,'a',2])
Assert("intersperse integers", Intersperse(0, [1,2,3,4,5]), [1,0,2,0,3,0,4,0,5])
Assert("intersperse string", Intersperse(' ', "hello"), "h e l l o")