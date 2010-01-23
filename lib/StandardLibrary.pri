def Null = lambda (x)
	x;
end

def List = lambda (x)
	Cons([], x);
end

def Assert = lambda (id, actual, expected)
	if (actual != expected)
		Show(id);
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
	Length(list) == 0;
end

def Reverse = lambda (list)
	if (Tail(list) != nil)
		Reverse(Tail(list)) ++ List(Head(list));
	else
		list;
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

def Product = lambda (list)
	if (list != nil)
		Head(list) * Product(Tail(list));
	else
		1;
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

def Take = lambda (n, list)
	def Inner = lambda (x, list)
		if (list != nil && x < n)
			List(Head(list)) ++ Inner(x + 1, Tail(list));
		else
			[];
		end
	end
	Inner(0, list);
end

def TakeWhile = lambda (fn, list)
	if (list != nil && fn(Head(list)) == true)
		List(Head(list)) ++ TakeWhile(fn, Tail(list));
	else
		[];
	end
end

def Drop = lambda (n, list)
	def Inner = lambda (x, list)
		if (list != nil)
			if (x < n)
				Inner(x + 1, Tail(list));
			else
				list;
			end
		else
			list;
		end
	end
	Inner(0, list);
end

def DropWhile = lambda (fn, list)
	if (list != nil)
		if (fn(Head(list)) == true)
			DropWhile(fn, Tail(list));
		else
			list;
		end
	else
		list;
	end
end

def Sort = lambda (l)
	if (l != [])
		def x = Head(l)
		def xs = Tail(l)
		Sort(Filter(lambda (a) a < x end, xs))
			++ List(x)
			++ Sort(Filter(lambda (a) a >= x end, xs))
	else
		[]
	end
end

def Min = lambda (l)
	Head(Sort(l))
end

def Max = lambda (l)
	Head(Reverse(Sort(l)))
end