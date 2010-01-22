def Double = lambda(x)
	x * 2;
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

Assert(Map(Double, [1,2,3]), [2,4,6])
Assert(Filter(GreaterThan3, [1,2,3,2,4,5,6]), [4,5,6])
Assert(ListCountdown(5), [5,4,3,2,1])
Assert(Cons(Map(Double, [1,2,3]), 0), [0,2,4,6])