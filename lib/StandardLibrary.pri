def Null = lambda (x)
	x;
end

def List = lambda (x)
	Cons([], x);
end

def Assert = lambda (actual, expected)
	if (actual != expected)
		Show(false);
	end
end

def Length = lambda (list)
	if (list != nil)
		1 + Length(Tail(list));
	else
		0;
	end
end

def Map = lambda (fn, list)
	if (list != nil)
		Cons(Map(fn, Tail(list)), fn(Head(list)));
	else
		nil;
	end
end

def FoldL = lambda (fn, init, list)
	if (list != nil)
		FoldL(fn, fn(init, Head(list)), Tail(list));
	else
		init;
	end
end

def FoldR = lambda (fn, init, list)
	if (list != nil)
		fn(Head(list), FoldR(fn, init, Tail(list)));
	else
		init;
	end
end

def Filter = lambda (fn, list)
	if (list != nil)
		if (fn(Head(list)) == true)
			Cons(Filter(fn, Tail(list)), Head(list));
		else
			Filter(fn, Tail(list));
		end
	else
		nil;
	end
end

def Empty = lambda (list)
	Size(list) == 0;
end

def Reverse = lambda (list)
	if (list != nil)
		Reverse(Tail(list)) ++ Cons([], Head(list));
	else
		[];
	end
end

def Find = lambda (x, list)
	if (Head(list) == x)
		Head(list);
	else
		if (Tail(list) != nil)
			Find(x, Tail(list));
		else
			nil;
		end
	end
end

def Sum = lambda (list)
	if (list != nil)
		Head(list) + Sum(Tail(list));
	else
		0;
	end
end

def Nth = lambda (list, n)
	def Inner = lambda (list, x)
		if (list != nil)
			if (x == n)
				Head(list);
			else
				Inner(Tail(list), x + 1);
			end
		else
			nil;
		end
	end
	Inner(list, 0);
end

def Any = lambda (pred, list)
	if (list != nil)
		if (pred(Head(list)) == true)
			true;
		else
			Any(pred, Tail(list));
		end
	else
		false;
	end
end

def All = lambda (pred, list)
	if (list != nil)
		if (pred(Head(list)) == true)
			All(pred, Tail(list));
		else
			false;
		end
	else
		true;
	end
end