def a = 10
def b = 15
def l = [1,2,3,4]
def s = "Hello, world!"
def t = "abcd"
def z = nil

def SingleStatement = fn (x)
	x + b + 2
end

def IfStatement = fn (x)
	if (x < 5)
		def z = 4
		z + x + test(3)
	else
		1
	end
end

def ReturnsArgument = fn (x, y)
	y
end

def ThreeArguments = fn (x, y, z)
	x * y - z
end

def InnerFunction = fn ()
	def c = 12
	def Inner = fn ()
		c
	end	
	Inner()
end

def HigherOrderFunction = fn (f)
	f(2)
end

def Double = fn(x)
	x * 2
end

def SumWithFun = fn(x, y, f)
	f(x) + f(y)
end

def Factorial = fn(x)
	if (x == 0)
		1
	else
		x * Factorial(x - 1)
	end
end

def GreaterThan3 = fn (x)
	x > 3
end

def ListCountdown = fn (x)
	if (x > 0)
		Cons(ListCountdown(x - 1), x)
	else
		nil
	end
end

def mm = Map(Double, l)

Assert("Types: int equality", 1, 1)
Assert("Types: float equality", 4.5, 4.5)
Assert("Types: bool equality 1", true, true)
Assert("Types: bool equality 2", false, false)
Assert("Types: string literal equality", "Hello, world!", "Hello, world!")
Assert("Types: string variable equality", s, s)
Assert("Types: string literal and variable equality", s, "Hello, world!")
Assert("Types: nil equality", nil, nil)
Assert("Types: empty list is nil", nil, [])
Assert("Types: empty list equality", [], [])
Assert("Types: short list equality", [2], [2])
Assert("Types: multi-item list equality", [1,2,3,4,5], [1,2,3,4,5])
Assert("Types: list of variables equality", [a, b], [10, 15])
Assert("Types: nested list equality", [1,2,[3,[4],5,6,7],8,[9,10],11], [1,2,[3,[4],5,6,7],8,[9,10],11])
Assert("Types: int type code", Type(1), 1)
Assert("Types: float type code", Type(4.5), 2)
Assert("Types: bool type code", Type(true), 3)
Assert("Types: char type code", Type('a'), 5)
Assert("Types: nil type code", Type(nil), 0)
Assert("Types: function type code", Type(Double), 6)
Assert("Types: type code of a variable", Type(a), 1)
Assert("Types: integer type check", Integer(10), true)
Assert("Types: integer type check to fail", Integer(false), false)
Assert("Types: float type check", Float(10.1), true)
Assert("Types: float type check to fail", Float(12), false)
Assert("Types: char type check", Char('z'), true)
Assert("Types: char type check to fail", Char(12), false)
Assert("Types: boolean type check", Bool(false), true)
Assert("Types: boolean type check to fail", Bool(88), false)
Assert("Types: named function type check", Function(Double), true)
Assert("Types: anonymous function type check", Function(fn (x) x end), true)

Assert("Operators: integer addition", 2 + 3, 5)
Assert("Operators: integer multiplication", 2 * 3, 6)
Assert("Operators: true or true", true || true, true)
Assert("Operators: true or false", true || false, true)
Assert("Operators: false or true", false || true, true)
Assert("Operators: false or false", false || false, false)
Assert("Operators: true and true", true && true, true)
Assert("Operators: true and false", true && false, false)
Assert("Operators: false and true", false && true, false)
Assert("Operators: false and false", false && false, false)
Assert("Operators: modulus", 11 % 3, 2)
Assert("Operators: not true", !true, false)
Assert("Operators: not false", !false, true)
Assert("Operators: not with equality", !1 == 2, true)
Assert("Operators: not with equality 2", !1 == 1, false)
Assert("Operators: unary minus", -10, -10)
Assert("Operators: subtraction", 0-10, -10)
Assert("Operators: precedence", 10 * 2 - 3, 17)
Assert("Operators: complex precedence", 12 + 3 * 2 / 5 % 2, 13)
Assert("Operators: unary minus with precedence", -10 * 2 - 3, -23)
Assert("Operators: list append", [1,2,3] ++ [4,5,6], [1,2,3,4,5,6])
Assert("Operators: nested list append", [1,2,3,[4,5]] ++ [4,5,[5,5,5],6], [1,2,3,[4,5],4,5,[5,5,5],6])
Assert("Operators: string append", "this" ++ " is " ++ "a test", "this is a test")
Assert("Operators: simple range", 1..5, [1,2,3,4,5])
Assert("Operators: precedence of range vs append", 1..5 ++ 6..10 ++ 11..15, [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15])

Assert("Funcall: simple", Factorial(5), 120)
Assert("Funcall: nested", ThreeArguments(InnerFunction(), HigherOrderFunction(SingleStatement), 12), 216)
Assert("Funcall: function returns new list", ListCountdown(5), [5,4,3,2,1])
Assert("Funcall: higher order with literals", SumWithFun(2, 3, Double), 10)
Assert("Funcall: higher order with variables", SumWithFun(a, b, Double), 50)
Assert("Funcall: higher order with fn", SumWithFun(a, b, fn (x) 2 * x end), 50)

Assert("Library: Cons equals literal list", Cons([1,2,3], 4), [4,1,2,3])
Assert("Library: nested Cons", Cons(Cons([0,1], [2,3,4]), [5,[6,7,8]]), [[5,[6,7,8]],[2,3,4],0,1])
Assert("Library: Nth element is a list", Nth([1,2,[3,4,5,6,7],8,9,10], 2), [3,4,5,6,7])
Assert("Library: simple Map", Map(Double, [1,2,3]), [2,4,6])
Assert("Library: simple Filter", Filter(GreaterThan3, [1,2,3,2,4,5,6]), [4,5,6])
Assert("Library: cons onto result of Map", Cons(Map(Double, [1,2,3]), 0), [0,2,4,6])
Assert("Library: triple nested Cons", Cons(Cons(Cons([1,2,3], 0), 5), 6), [6,5,0,1,2,3])
Assert("Library: Map over concatenated lists", Map(Double, [1,2,3] ++ [4,5,6]), [2,4,6,8,10,12])
Assert("Library: Concatenate and Cons", [1,2,3,[4,5]] ++ [4,5,[5,5,5],6] ++ [1,2] ++ Cons([], 7), [1,2,3,[4,5],4,5,[5,5,5],6,1,2,7])
Assert("Library: simple Length", Length(l), 4)
Assert("Library: test empty list", Empty([]), true)
Assert("Library: test non-empty list", Empty([1]), false)
Assert("Library: test non-empty list of different types", Empty([true, 12.4, 10]), false)
Assert("Library: simple Length test", Length([1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17]), 17)
Assert("Library: length of concatenated list", Length([1,2,3,4] ++ [1,2,3,4]), 8)
Assert("Library: simple Head test", Head([1,2,3]), 1)
Assert("Library: head of the tail of a list", Head(Tail([1,2,3])), 2)
Assert("Library: head of the tail of the tail of a list", Head(Tail(Tail([1,2,3]))), 3)
Assert("Library: simple Find", Find(11, [4,6,10,2,4,11]), 11)
Assert("Library: failing Find", Find(12, [4,6,10,2,4,11]), nil)
Assert("Library: length of multiple list concatenations", Length([9,10] ++ [11,12] ++ [13,14,15] ++ [16] ++ [17]), 9)
Assert("Library: sum of list of int", Sum([1,2,3]), 6)
Assert("Library: simple Nth", Nth([44,12,66,87], 2), 66)
Assert("Library: Length of nested list", Length([1,2,[3,4,5,6,7],6]), 4)
Assert("Library: Length of list extracted by Nth", Length(Nth([1,2,[3,4,5,6,7],6], 2)), 5)
Assert("Library: Length of the empty list", Length([]), 0)
Assert("Library: combined list functions 1", Sum(Map(Double, Filter(fn (x) 10 % x == 0 end, Cons(Cons([1, 2], 3), 4)))), 6)
Assert("Library: FoldL with fn", FoldL(fn (x, y) x + y end, 0, [1,2,3,4,5]), 15)
Assert("Library: FoldR with fn", FoldR(fn (x, y) x + y end, 0, [1,2,3,4,5]), 15)
Assert("Library: Any with pass in middle of list", Any(fn (x) x == 2 end, [1,2,3,4]), true)
Assert("Library: Any with pass at start of list", Any(fn (x) x == 2 end, [2,1,3,4]), true)
Assert("Library: Any with pass at end of list", Any(fn (x) x == 2 end, [1,3,4,2]), true)
Assert("Library: All passes", All(fn (x) x * x > 4 end, [3,4,5,6]), true)
Assert("Library: All fails", All(fn (x) x * x > 4 end, [2,3,4,5,6]), false)
Assert("Library: Take nothing", Take(0, [1,2,3]), [])
Assert("Library: Take 1", Take(1, [1,2,3]), [1])
Assert("Library: Take 2", Take(2, [1,2,3]), [1,2])
Assert("Library: Take all", Take(3, [1,2,3]), [1,2,3])
Assert("Library: Take more than list has", Take(4, [1,2,3]), [1,2,3])
Assert("Library: Drop nothing", Drop(0, [1,2,3,4,5]), [1,2,3,4,5])
Assert("Library: Drop 1", Drop(1, [1,2,3,4,5]), [2,3,4,5])
Assert("Library: Drop all but 1", Drop(4, [1,2,3,4,5]), [5])
Assert("Library: Drop all", Drop(5, [1,2,3,4,5]), [])
Assert("Library: Drop more than list has", Drop(6, [1,2,3,4,5]), [])
Assert("Library: Reverse a list", Reverse([1,2,3]), [3,2,1])
Assert("Library: Reverse a two item list", Reverse([1,2]), [2,1])
Assert("Library: Reverse a one item list", Reverse([1]), [1])
Assert("Library: Reverse a list twice", Reverse(Reverse([1,2,3])), [1,2,3])
Assert("Library: Reverse a list three times", Reverse(Reverse(Reverse([1,2,3]))), [3,2,1])
Assert("Library: Reverse the empty list", Reverse([]), [])
Assert("Library: Reverse a string", Reverse("hello"), "olleh")
Assert("Library: simple TakeWhile", TakeWhile(fn (x) x < 3 end, [1,2,3,4,5]), [1,2])
Assert("Library: TakeWhile takes nothing", TakeWhile(fn (x) x < 0 end, [1,2,3,4,5]), nil)
Assert("Library: TakeWhile takes everything", TakeWhile(fn (x) x < 6 end, [1,2,3,4,5]), [1,2,3,4,5])
Assert("Library: DropWhile drops nothing", DropWhile(fn (x) x < 0 end, [1,2,3,4,5]), [1,2,3,4,5])
Assert("Library: simple DropWhile", DropWhile(fn (x) x < 3 end, [1,2,3,4,5]), [3,4,5])
Assert("Library: DropWhile drops all but one", DropWhile(fn (x) x < 5 end, [1,2,3,4,5]), [5])
Assert("Library: DropWhile drops everything", DropWhile(fn (x) x < 6 end, [1,2,3,4,5]), [])
Assert("Library: simple Sum", Sum([1,2,3,4,5]), 15)
Assert("Library: Sum of one element list", Sum([1]), 1)
Assert("Library: Sum of empty list", Sum([]), 0)
Assert("Library: simple Product", Product([1,2,3,4,5]), 120)
Assert("Library: Product of one element list", Product([5]), 5)
Assert("Library: Product of empty list", Product([]), 0)
Assert("Library: Sort", Sort([7,2,4,5,1,3,8,6,9]), [1,2,3,4,5,6,7,8,9])
Assert("Library: Sort empty list", Sort([]), [])
Assert("Library: simple Min", Min([7,2,4,5,1,3,8,6,9]), 1)
Assert("Library: Min of one element list", Min([7]), 7)
Assert("Library: Min of empty list", Min([]), [])
Assert("Library: simple Max", Max([7,2,4,5,1,3,8,6]), 8)
Assert("Library: Max is last item", Max([7,2,4,5,1,3,8,6,9]), 9)
Assert("Library: Max is first item", Max([7,2,4,5,1,3,6]), 7)
Assert("Library: Max of empty list", Max([]), [])
Assert("Library: simple Last", Last([1,2,3,4,5]), 5)
Assert("Library: Last of one element list", Last([1]), 1)
Assert("Library: Last of empty list", Last([]), nil)
Assert("Library: Odd is true", Odd(1), true)
Assert("Library: Odd if false", Odd(2), false)
Assert("Library: Even is false", Even(1), false)
Assert("Library: Even is true", Even(2), true)
Assert("Library: string variable length", Length(s), 13)
Assert("Library: string literal length", Length("foo"), 3)
Assert("Library: Sort a string", Sort("qwertyuiopasdfghjklzxcvbnm"), "abcdefghijklmnopqrstuvwxyz")
Assert("Library: string length", Length(s), 13)
Assert("Library: string Reverse", Reverse("foo"), "oof")
Assert("Library: take from string", Take(4, s), "Hell")
Assert("Library: drop from string", Drop(4, s), "o, world!")
Assert("Library: Head of string is a char", Head(s), 'H')
Assert("Library: Tail of string", Tail(s), "ello, world!")
Assert("Library: Zip", Zip([1,2,3], [4,5,6]), [[1,4],[2,5],[3,6]])
Assert("Library: Zip with short first list", Zip([1,2,3], [4,5,6,7]), [[1,4],[2,5],[3,6]])
Assert("Library: Zip with short second list", Zip([1,2,3,4], [4,5,6]), [[1,4],[2,5],[3,6]])