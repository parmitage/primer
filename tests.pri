def a = 10
def b = 15
def l = [1,2,3,4]
def s = "Hello, world!"
def z = nil

def singleStatement = fn (x)
	x + b + 2
end

def ifStatement = fn (x)
	if (x < 5)
		def z = 4
		z + x + test(3)
	else
		1
	end
end

def returnsArgument = fn (x, y)
	y
end

def threeArguments = fn (x, y, z)
	x * y - z
end

def innerFunction = fn ()
	def c = 12
	def inner = fn ()
		c
	end
	inner()
end

def higherOrderFunction = fn (f)
	f(2)
end

def double = fn(x)
	x * 2
end

def sumWithFun = fn(x, y, f)
	f(x) + f(y)
end

def factorial = fn(x)
	if (x == 0)
		1
	else
		x * factorial(x - 1)
	end
end

def greaterThan3 = fn (x)
	x > 3
end

def listCountdown = fn (x)
	if (x > 0)
		[x] ++ listCountdown(x - 1)
	else
		nil
	end
end



assert("Types: int equality", 1, 1)
assert("Types: float equality", 4.5, 4.5)
assert("Types: bool equality 1", true, true)
assert("Types: bool equality 2", false, false)
assert("Types: string literal equality", "Hello, world!", "Hello, world!")
assert("Types: string variable equality", s, s)
assert("Types: string literal and variable equality", s, "Hello, world!")
assert("Types: nil equality", nil, nil)
assert("Types: empty list is nil", nil, [])
assert("Types: empty list equality", [], [])
assert("Types: short list equality", [2], [2])
assert("Types: multi-item list equality", [1,2,3,4,5], [1,2,3,4,5])
assert("Types: list of variables equality", [a, b], [10, 15])
assert("Types: nested list equality", [1,2,[3,[4],5,6,7],8,[9,10],11], [1,2,[3,[4],5,6,7],8,[9,10],11])
assert("Types: int type code", type(1), 1)
assert("Types: float type code", type(4.5), 2)
assert("Types: bool type code", type(true), 3)
assert("Types: char type code", type('a'), 5)
assert("Types: nil type code", type(nil), 0)
assert("Types: function type code", type(double), 6)
assert("Types: type code of a variable", type(a), 1)
assert("Types: integer type check", isint(10), true)
assert("Types: integer type check to fail", isint(false), false)
assert("Types: float type check", isfloat(10.1), true)
assert("Types: float type check to fail", isfloat(12), false)
assert("Types: char type check", ischar('z'), true)
assert("Types: char type check to fail", ischar(12), false)
assert("Types: boolean type check", isbool(false), true)
assert("Types: boolean type check to fail", isbool(88), false)
assert("Types: named function type check", isfn(double), true)
assert("Types: anonymous function type check", isfn(fn (x) x end), true)

assert("Operators: integer addition", 2 + 3, 5)
assert("Operators: integer multiplication", 2 * 3, 6)
assert("Operators: true or true", true or true, true)
assert("Operators: true or false", true or false, true)
assert("Operators: false or true", false or true, true)
assert("Operators: false or false", false or false, false)
assert("Operators: true and true", true and true, true)
assert("Operators: true and false", true and false, false)
assert("Operators: false and true", false and true, false)
assert("Operators: false and false", false and false, false)
assert("Operators: modulus", 11 mod 3, 2)
assert("Operators: not true", not true, false)
assert("Operators: not false", not false, true)
assert("Operators: not with equality", not 1 == 2, true)
assert("Operators: not with equality 2", not 1 == 1, false)
assert("Operators: unary minus", -10, -10)
assert("Operators: subtraction", 0-10, -10)
assert("Operators: precedence", 10 * 2 - 3, 17)
assert("Operators: complex precedence", 12 + 3 * 2 / 5 mod 2, 13)
assert("Operators: unary minus with precedence", -10 * 2 - 3, -23)
assert("Operators: list append", [1,2,3] ++ [4,5,6], [1,2,3,4,5,6])
assert("Operators: nested list append", [1,2,3,[4,5]] ++ [4,5,[5,5,5],6], [1,2,3,[4,5],4,5,[5,5,5],6])
assert("Operators: string append", "this" ++ " is " ++ "a test", "this is a test")
assert("Operators: simple range", 1..5, [1,2,3,4,5])
assert("Operators: precedence of range vs append", 1..5 ++ 6..10 ++ 11..15, [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15])
assert("Operators: greater than", 5 > 2, true)
assert("Operators: greater than fails", 5 > 9, false)
assert("Operators: less than", 2 < 5, true)
assert("Operators: less than fails", 2 < 1, false)
assert("Operators: less than or equal", 2 <= 5, true)
assert("Operators: less than or equal equal", 2 <= 2, true)
assert("Operators: less than or equal fails", 2 <= 1, false)
assert("Operators: greater than or equal", 9 >= 5, true)
assert("Operators: greater than or equal equal", 2 >= 2, true)
assert("Operators: greater than or equal fails", 2 >= 11, false)

assert("Funcall: simple", factorial(5), 120)
assert("Funcall: nested", threeArguments(innerFunction(), higherOrderFunction(singleStatement), 12), 216)
assert("Funcall: function returns new list", listCountdown(5), [5,4,3,2,1])
assert("Funcall: higher order with literals", sumWithFun(2, 3, double), 10)
assert("Funcall: higher order with variables", sumWithFun(a, b, double), 50)
assert("Funcall: higher order with fn", sumWithFun(a, b, fn (x) 2 * x end), 50)

assert("Library: cons equals literal list", cons([1,2,3], 4), [4,1,2,3])
assert("Library: nested cons", cons(cons([0,1], [2,3,4]), [5,[6,7,8]]), [[5,[6,7,8]],[2,3,4],0,1])
assert("Library: nth element is a list", nth([1,2,[3,4,5,6,7],8,9,10], 2), [3,4,5,6,7])
assert("Library: simple map", map(double, [1,2,3]), [2,4,6])
assert("Library: simple filter", filter(greaterThan3, [1,2,3,2,4,5,6]), [4,5,6])
assert("Library: cons onto result of map", cons(map(double, [1,2,3]), 0), [0,2,4,6])
assert("Library: triple nested cons", cons(cons(cons([1,2,3], 0), 5), 6), [6,5,0,1,2,3])
assert("Library: map over concatenated lists", map(double, [1,2,3] ++ [4,5,6]), [2,4,6,8,10,12])
assert("Library: concatenate and cons", [1,2,3,[4,5]] ++ [4,5,[5,5,5],6] ++ [1,2] ++ cons([], 7), [1,2,3,[4,5],4,5,[5,5,5],6,1,2,7])

assert("Library: simple length", length(l), 4)
assert("Library: test empty list", empty([]), true)
assert("Library: test non-empty list", empty([1]), false)
assert("Library: test non-empty list of different types", empty([true, 12.4, 10]), false)
assert("Library: simple length test", length([1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17]), 17)
assert("Library: length of concatenated list", length([1,2,3,4] ++ [1,2,3,4]), 8)
assert("Library: simple head test", head([1,2,3]), 1)
assert("Library: head of the tail of a list", head(tail([1,2,3])), 2)
assert("Library: head of the tail of the tail of a list", head(tail(tail([1,2,3]))), 3)
assert("Library: simple find", find(11, [4,6,10,2,4,11]), 11)
assert("Library: failing find", find(12, [4,6,10,2,4,11]), nil)
assert("Library: length of multiple list concatenations", length([9,10] ++ [11,12] ++ [13,14,15] ++ [16] ++ [17]), 9)
assert("Library: sum of list of int", sum([1,2,3]), 6)
assert("Library: simple nth", nth([44,12,66,87], 2), 66)
assert("Library: length of nested list", length([1,2,[3,4,5,6,7],6]), 4)
assert("Library: length of list extracted by nth", length(nth([1,2,[3,4,5,6,7],6], 2)), 5)
assert("Library: length of the empty list", length([]), 0)
assert("Library: combined list functions 1", sum(map(double, filter(fn (x) 10 mod x == 0 end, cons(cons([1, 2], 3), 4)))), 6)
assert("Library: foldl with fn", foldl(fn (x, y) x + y end, 0, [1,2,3,4,5]), 15)
assert("Library: foldr with fn", foldr(fn (x, y) x + y end, 0, [1,2,3,4,5]), 15)
assert("Library: any with pass in middle of list", any(fn (x) x == 2 end, [1,2,3,4]), true)
assert("Library: any with pass at start of list", any(fn (x) x == 2 end, [2,1,3,4]), true)
assert("Library: any with pass at end of list", any(fn (x) x == 2 end, [1,3,4,2]), true)
assert("Library: all passes", all(fn (x) x * x > 4 end, [3,4,5,6]), true)
assert("Library: all fails", all(fn (x) x * x > 4 end, [2,3,4,5,6]), false)
assert("Library: take nothing", take(0, [1,2,3]), [])
assert("Library: take 1", take(1, [1,2,3]), [1])
assert("Library: take 2", take(2, [1,2,3]), [1,2])
assert("Library: take all", take(3, [1,2,3]), [1,2,3])
assert("Library: take more than list has", take(4, [1,2,3]), [1,2,3])
assert("Library: drop nothing", drop(0, [1,2,3,4,5]), [1,2,3,4,5])
assert("Library: drop 1", drop(1, [1,2,3,4,5]), [2,3,4,5])
assert("Library: drop all but 1", drop(4, [1,2,3,4,5]), [5])
assert("Library: drop all", drop(5, [1,2,3,4,5]), [])
assert("Library: drop more than list has", drop(6, [1,2,3,4,5]), [])
assert("Library: reverse a list", reverse([1,2,3]), [3,2,1])
assert("Library: reverse a two item list", reverse([1,2]), [2,1])
assert("Library: reverse a one item list", reverse([1]), [1])
assert("Library: reverse a list twice", reverse(reverse([1,2,3])), [1,2,3])
assert("Library: reverse a list three times", reverse(reverse(reverse([1,2,3]))), [3,2,1])
assert("Library: reverse the empty list", reverse([]), [])
assert("Library: reverse a string", reverse("hello"), "olleh")
assert("Library: simple takewhile", takewhile(fn (x) x < 3 end, [1,2,3,4,5]), [1,2])
assert("Library: takewhile takes nothing", takewhile(fn (x) x < 0 end, [1,2,3,4,5]), nil)
assert("Library: takewhile takes everything", takewhile(fn (x) x < 6 end, [1,2,3,4,5]), [1,2,3,4,5])
assert("Library: dropwhile drops nothing", dropwhile(fn (x) x < 0 end, [1,2,3,4,5]), [1,2,3,4,5])
assert("Library: simple dropwhile", dropwhile(fn (x) x < 3 end, [1,2,3,4,5]), [3,4,5])
assert("Library: dropwhile drops all but one", dropwhile(fn (x) x < 5 end, [1,2,3,4,5]), [5])
assert("Library: dropwhile drops everything", dropwhile(fn (x) x < 6 end, [1,2,3,4,5]), [])
assert("Library: simple sum", sum([1,2,3,4,5]), 15)
assert("Library: sum of one element list", sum([1]), 1)
assert("Library: sum of empty list", sum([]), 0)
assert("Library: simple product", product([1,2,3,4,5]), 120)
assert("Library: product of one element list", product([5]), 5)
assert("Library: product of empty list", product([]), 0)
assert("Library: sort", sort([7,2,4,5,1,3,8,6,9]), [1,2,3,4,5,6,7,8,9])
assert("Library: sort empty list", sort([]), [])
assert("Library: simple min", min([7,2,4,5,1,3,8,6,9]), 1)
assert("Library: min of one element list", min([7]), 7)
assert("Library: min of empty list", min([]), [])
assert("Library: simple max", max([7,2,4,5,1,3,8,6]), 8)
assert("Library: max is last item", max([7,2,4,5,1,3,8,6,9]), 9)
assert("Library: max is first item", max([7,2,4,5,1,3,6]), 7)
assert("Library: max of empty list", max([]), [])
assert("Library: simple last", last([1,2,3,4,5]), 5)
assert("Library: last of one element list", last([1]), 1)
assert("Library: last of empty list", last([]), nil)
assert("Library: odd is true", odd(1), true)
assert("Library: odd if false", odd(2), false)
assert("Library: even is false", even(1), false)
assert("Library: even is true", even(2), true)
assert("Library: string variable length", length(s), 13)
assert("Library: string literal length", length("foo"), 3)
assert("Library: sort a string", sort("qwertyuiopasdfghjklzxcvbnm"), "abcdefghijklmnopqrstuvwxyz")
assert("Library: string length", length(s), 13)
assert("Library: string reverse", reverse("foo"), "oof")
assert("Library: take from string", take(4, s), "Hell")
assert("Library: drop from string", drop(4, s), "o, world!")
assert("Library: head of string is a char", head(s), 'H')
assert("Library: tail of string", tail(s), "ello, world!")
assert("Library: zip", zip([1,2,3], [4,5,6]), [[1,4],[2,5],[3,6]])
assert("Library: zip with short first list", zip([1,2,3], [4,5,6,7]), [[1,4],[2,5],[3,6]])
assert("Library: zip with short second list", zip([1,2,3,4], [4,5,6]), [[1,4],[2,5],[3,6]])