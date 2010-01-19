def Assert = lambda (actual, expected)
	if (actual != expected)
		Show(false);
	end
end

def Length = lambda (list)
	if (list != nil)
		return 1 + Length(Tail(list));
	else
		return 0;
	end
end

def Append = lambda (l1, l2)
	if (l1 == nil)
		return l2;
	else
		return Cons(Append(Tail(l1), l2), Head(l1));
	end
end

def Map = lambda (fn, list)
	if (list != nil)
		return Cons(Map(fn, Tail(list)), fn(Head(list)));
	else
		return nil;
	end
end

def Filter = lambda (fn, list)
	if (list != nil)
		if (fn(Head(list)) == true)
			return Cons(Filter(fn, Tail(list)), Head(list));
		else
			return Filter(fn, Tail(list));
		end
	else
		return nil;
	end
end

def Empty = lambda (list)
	return Size(list) == 0;
end

def Reverse = lambda (list)
	if (list != nil)
		return Cons(Reverse(Tail(list)), Head(list));
	else
		return [];
	end
end

def Find = lambda (x, list)
	if (Head(list) == x)
		return Head(list);
	else
		if (Tail(list) != nil)
			return Find(x, Tail(list));
		else
			return nil;
		end
	end
end

def Sum = lambda (list)
	if (list != nil)
		return Head(list) + Sum(Tail(list));
	else
		return 0;
	end
end

def Nth = lambda (list, n)
	def Inner = lambda (list, x)
		if (list != nil)
			if (x == n)
				return Head(list);
			else
				return Inner(Tail(list), x + 1);
			end
		else
			return nil;
		end
	end
	return Inner(list, 0);
end
		