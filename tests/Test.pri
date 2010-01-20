def a = 10
def b = 15
def l = [1,2,3,4]
def z = nil

def test = lambda (x)
	x + b + 2;
end

def test2 = lambda (x)
	if (x < 5)
		def z = 4;
		z + x + test(3);
	else
		1;
	end
end

def test3 = lambda (x, y)
	y;
end

def test4 = lambda (x, y, z)
	x * y - z;
end

def test5 = lambda ()
	b;
end

def test6 = lambda ()
	def c = 12;
	def test6a = lambda ()
		c;
	end	
	test6a();
end

def test7 = lambda (fn)
	fn(2);
end

def test8 = lambda ()
	def fx = lambda (x) x; end;
	fx;
end

def Double = lambda(x)
	x * 2;
end

def SumWithFun = lambda(x, y, fn)
	fn(x) + fn(y);
end

def Factorial = lambda(x)
	if (x == 0)
		1;
	else
		x * Factorial(x - 1);
	end
end

def GreaterThan3 = lambda (x)
	x > 3;
end

def ListCountdown = lambda (x)
	if (x > 0)
		Cons(ListCountdown(x - 1), x);
	else
		nil;
	end
end

def mm = Map(Double, l)

Show([1,2,[3,4,5,6,7],8,9,10])
Show(Nth([1,2,[3,4,5,6,7],8,9,10], 2))
Show([1,2,[3,[4],5,6,7],8,[9,10],11])
Show([2])
Show(Cons(Cons([0,1], [2,3,4]), [5,[6,7,8]]))
Show(Cons([1,2,3], 4))
Show(Double)
Show(Map(Double, [1,2,3]))
Show(Filter(GreaterThan3, [1,2,3,2,4,5,6]))
Show(ListCountdown(5))
Show(Head([[1,2,3],[4,5,6],7,8,[9,10]]))
Show(Cons(Map(Double, [1,2,3]), 0))
Show(Cons(Cons(Cons([1,2,3], 0), 5), 6))
Show(Tail(Map(Double, Map(Double, [11,22,33]))))
Show([1,2,3] ++ [4,5,6])
Show(Map(Double, [1,2,3] ++ [4,5,6]))
Show([1,2,3] ++ [4,5,6])
Show([1,2,3,[4,5]] ++ [4,5,[5,5,5],6])
Show([1,2,3,[4,5]] ++ [4,5,[5,5,5],6] ++ [1,2] ++ Cons([], 7))

Assert(SumWithFun(2, 3, Double), 10)
Assert(SumWithFun(a, b, Double), 50)
Assert(Factorial(5), 120)
Assert(test4(test6(), test7(test), 12), 216)
Assert(Length(l), 4)
Assert(Length([1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17]), 17)
Assert(Length([1,2,3,4] ++ [1,2,3,4]), 8)
Assert(Head([1,2,3]), 1)
Assert(Head(Tail([1,2,3])), 2)
Assert(Head(Tail(Tail([1,2,3]))), 3)
Assert(Find(11, [4,6,10,2,4,11]), 11)
Assert(Find(12, [4,6,10,2,4,11]), nil)
Assert(SumWithFun(2, 3, lambda (x) 3 * x; end), 15)
Assert(Length([9,10] ++ [11,12] ++ [13,14,15] ++ [16] ++ [17]), 9)
Assert(true || true, true)
Assert(11 % 3, 2)
Assert(2, 2)
Assert(Sum([1,2,3]), 6)
Assert(Nth([44,12,66,87], 2), 66)
Assert(Length([1,2,[3,4,5,6,7],6]), 4)
Assert(Length(Nth([1,2,[3,4,5,6,7],6], 2)), 5)
Assert([], nil)
Assert(Length([]), 0)
Assert(Sum(Map(Double, Filter(lambda (x) 10 % x == 0; end, Cons(Cons([1, 2], 3), 4)))), 6)
Assert(FoldL(lambda (x, y) x + y; end, 0, [1,2,3,4,5]), 15)
Assert(FoldR(lambda (x, y) x + y; end, 0, [1,2,3,4,5]), 15)
Assert(Any(lambda (x) x == 2; end, [1,2,3,4]), true)
Assert(Any(lambda (x) x == 2; end, [2,1,3,4]), true)
Assert(Any(lambda (x) x == 2; end, [1,3,4,2]), true)
Assert(All(lambda (x) x * x > 4; end, [3,4,5,6]), true)
Assert(All(lambda (x) x * x > 4; end, [2,3,4,5,6]), false)