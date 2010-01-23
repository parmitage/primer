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

Assert(1, 1, 1)
Assert(2, 4.5, 4.5)
Assert(3, true, true)
Assert(4, false, false)
Assert(5, nil, nil)
Assert(6, nil, [])
Assert(7, [], [])
Assert(8, [2], [2])
Assert(9, [1,2,3,4,5], [1,2,3,4,5])
Assert(10, Cons([1,2,3], 4), [4,1,2,3])
Assert(11, Nth([1,2,[3,4,5,6,7],8,9,10], 2), [3,4,5,6,7])
Assert(12, [1,2,[3,[4],5,6,7],8,[9,10],11], [1,2,[3,[4],5,6,7],8,[9,10],11])
Assert(13, 2 + 3, 5)
Assert(14, 2 * 3, 6)
Assert(15, Cons(Cons([0,1], [2,3,4]), [5,[6,7,8]]), [[5,[6,7,8]],[2,3,4],0,1])
Assert(16, Map(Double, [1,2,3]), [2,4,6])
Assert(17, Filter(GreaterThan3, [1,2,3,2,4,5,6]), [4,5,6])
Assert(18, ListCountdown(5), [5,4,3,2,1])
Assert(19, Cons(Map(Double, [1,2,3]), 0), [0,2,4,6])
Assert(20, Cons(Cons(Cons([1,2,3], 0), 5), 6), [6,5,0,1,2,3])
Assert(21, [1,2,3] ++ [4,5,6], [1,2,3,4,5,6])
Assert(22, Map(Double, [1,2,3] ++ [4,5,6]), [2,4,6,8,10,12])
Assert(23, [1,2,3,[4,5]] ++ [4,5,[5,5,5],6], [1,2,3,[4,5],4,5,[5,5,5],6])
Assert(24, [1,2,3,[4,5]] ++ [4,5,[5,5,5],6] ++ [1,2] ++ Cons([], 7), [1,2,3,[4,5],4,5,[5,5,5],6,1,2,7])
Assert(25, SumWithFun(2, 3, Double), 10)
Assert(26, SumWithFun(a, b, Double), 50)
Assert(27, Factorial(5), 120)
Assert(28, test4(test6(), test7(test), 12), 216)
Assert(29, Length(l), 4)
Assert(30, Empty([]), true)
Assert(31, Empty([1]), false)
Assert(32, Empty([true, 12.4, 10]), false)
Assert(33, Length([1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17]), 17)
Assert(34, Length([1,2,3,4] ++ [1,2,3,4]), 8)
Assert(35, Head([1,2,3]), 1)
Assert(36, Head(Tail([1,2,3])), 2)
Assert(37, Head(Tail(Tail([1,2,3]))), 3)
Assert(38, Find(11, [4,6,10,2,4,11]), 11)
Assert(39, Find(12, [4,6,10,2,4,11]), nil)
Assert(40, SumWithFun(2, 3, lambda (x) 3 * x; end), 15)
Assert(41, Length([9,10] ++ [11,12] ++ [13,14,15] ++ [16] ++ [17]), 9)
Assert(42, true || true, true)
Assert(43, true || false, true)
Assert(44, false || true, true)
Assert(45, false || false, false)
Assert(46, true && true, true)
Assert(47, true && false, false)
Assert(48, false && true, false)
Assert(49, false && false, false)
Assert(50, 11 % 3, 2)
Assert(51, Sum([1,2,3]), 6)
Assert(52, Nth([44,12,66,87], 2), 66)
Assert(53, Length([1,2,[3,4,5,6,7],6]), 4)
Assert(54, Length(Nth([1,2,[3,4,5,6,7],6], 2)), 5)
Assert(55, Length([]), 0)
Assert(56, Sum(Map(Double, Filter(lambda (x) 10 % x == 0; end, Cons(Cons([1, 2], 3), 4)))), 6)
Assert(57, FoldL(lambda (x, y) x + y; end, 0, [1,2,3,4,5]), 15)
Assert(58, FoldR(lambda (x, y) x + y; end, 0, [1,2,3,4,5]), 15)
Assert(59, Any(lambda (x) x == 2; end, [1,2,3,4]), true)
Assert(60, Any(lambda (x) x == 2; end, [2,1,3,4]), true)
Assert(61, Any(lambda (x) x == 2; end, [1,3,4,2]), true)
Assert(62, All(lambda (x) x * x > 4; end, [3,4,5,6]), true)
Assert(63, All(lambda (x) x * x > 4; end, [2,3,4,5,6]), false)
Assert(64, Take(0, [1,2,3]), [])
Assert(65, Take(1, [1,2,3]), [1])
Assert(66, Take(2, [1,2,3]), [1,2])
Assert(67, Take(3, [1,2,3]), [1,2,3])
Assert(68, Take(4, [1,2,3]), [1,2,3])
Assert(69, Drop(0, [1,2,3,4,5]), [1,2,3,4,5])
Assert(70, Drop(1, [1,2,3,4,5]), [2,3,4,5])
Assert(71, Drop(4, [1,2,3,4,5]), [5])
Assert(72, Drop(5, [1,2,3,4,5]), [])
Assert(73, Drop(6, [1,2,3,4,5]), [])
Assert(74, Reverse([1,2,3]), [3,2,1])
Assert(75, Reverse([1,2]), [2,1])
Assert(76, Reverse([1]), [1])
Assert(76, Reverse([]), [])
Assert(77, Reverse(Reverse([1,2,3])), [1,2,3])
Assert(78, Reverse(Reverse(Reverse([1,2,3]))), [3,2,1])
Assert(79, TakeWhile(lambda (x) x < 3; end, [1,2,3,4,5]), [1,2])
Assert(80, TakeWhile(lambda (x) x < 0; end, [1,2,3,4,5]), nil)

Assert(82, DropWhile(lambda (x) x < 0; end, [1,2,3,4,5]), [1,2,3,4,5])
Assert(83, DropWhile(lambda (x) x < 3; end, [1,2,3,4,5]), [3,4,5])
Assert(84, DropWhile(lambda (x) x < 5; end, [1,2,3,4,5]), [5])
Assert(85, DropWhile(lambda (x) x < 6; end, [1,2,3,4,5]), [])
Assert(86, Sum([1,2,3,4,5]), 15)
Assert(87, Sum([1]), 1)
Assert(88, Sum([]), 0)
Assert(89, Product([1,2,3,4,5]), 120)
Assert(90, Product([5]), 5)
Assert(91, Product([]), 1)
Assert(92, Sort([7,2,4,5,1,3,8,6,9]), [1,2,3,4,5,6,7,8,9])
Assert(93, Min([7,2,4,5,1,3,8,6,9]), 1)