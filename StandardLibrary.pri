def Null = fn (x)
	x
end

def Assert = fn (id, actual, expected)
	if (actual != expected)
		Show(id)
	end
end

def Length = fn (list)
	if (list != nil)
		1 + Length(Tail(list))
	else
		0
	end
end

def Map = fn (f, list)
	if (list != nil)
		Cons(Map(f, Tail(list)), f(Head(list)))
	else
		nil
	end
end

def FoldL = fn (f, init, list)
	if (list != nil)
		FoldL(f, f(init, Head(list)), Tail(list))
	else
		init
	end
end

def FoldR = fn (f, init, list)
	if (list != nil)
		f(Head(list), FoldR(f, init, Tail(list)))
	else
		init
	end
end

def Filter = fn (f, list)
	if (list != nil)
		if (f(Head(list)) == true)
			Cons(Filter(f, Tail(list)), Head(list))
		else
			Filter(f, Tail(list))
		end
	else
		nil
	end
end

def Empty = fn (list)
	Length(list) == 0
end

def Reverse = fn (list)
	if (Tail(list) != nil)
		Reverse(Tail(list)) ++ [Head(list)]
	else
		list
	end
end

def Find = fn (x, list)
	if (Head(list) == x)
		Head(list)
	else
		if (Tail(list) != nil)
			Find(x, Tail(list))
		else
			nil
		end
	end
end

def Sum = fn (list)
	if (list != nil)
		Head(list) + Sum(Tail(list))
	else
		0
	end
end

def Product = fn (l)
	def Inner = fn (l)
		if (l != nil)
			Head(l) * Inner(Tail(l))
		else
			1
		end
	end
	if (Empty(l))
		0
	else
		Inner(l)
	end
end

def Nth = fn (list, n)
	def Inner = fn (list, x)
		if (list != nil)
			if (x == n)
				Head(list)
			else
				Inner(Tail(list), x + 1)
			end
		else
			nil
		end
	end
	Inner(list, 0)
end

def Any = fn (pred, list)
	if (list != nil)
		if (pred(Head(list)) == true)
			true
		else
			Any(pred, Tail(list))
		end
	else
		false
	end
end

def All = fn (pred, list)
	if (list != nil)
		if (pred(Head(list)) == true)
			All(pred, Tail(list))
		else
			false
		end
	else
		true
	end
end

def Take = fn (n, list)
	def Inner = fn (x, list)
		if (list != nil && x < n)
			[Head(list)] ++ Inner(x + 1, Tail(list))
		else
			[]
		end
	end
	Inner(0, list)
end

def TakeWhile = fn (f, list)
	if (list != nil && f(Head(list)) == true)
		[Head(list)] ++ TakeWhile(f, Tail(list))
	else
		[]
	end
end

def Drop = fn (n, list)
	def Inner = fn (x, list)
		if (list != nil)
			if (x < n)
				Inner(x + 1, Tail(list))
			else
				list
			end
		else
			list
		end
	end
	if (n >= Length(list))
		[]
	else
		Inner(0, list)
	end
end

def DropWhile = fn (f, list)
	if (list != nil)
		if (f(Head(list)) == true)
			DropWhile(f, Tail(list))
		else
			list
		end
	else
		list
	end
end

def Sort = fn (l)
	if (l != [])
		def x = Head(l)
		def xs = Tail(l)
		Sort(Filter(fn (a) a < x end, xs)) ++ [x] ++ Sort(Filter(fn (a) a >= x end, xs))
	else
		[]
	end
end

def Min = fn (l)
	Head(Sort(l))
end

def Max = fn (l)
	Head(Reverse(Sort(l)))
end

def Last = fn (l)
	Head(Reverse(l))
end

def Odd = fn (z)
	z % 2 != 0
end

def Even = fn (z)
	z % 2 == 0
end

def Integer = fn (z)
	Type(z) == 1
end

def Float = fn (z)
	Type(z) == 2
end

def Char = fn (z)
	Type(z) == 5
end

def Bool = fn (z)
	Type(z) == 3
end

def Function = fn (z)
	Type(z) == 6
end

def Zip = fn (l1, l2)
	if (l1 != [] && l2 != [])
		[[Head(l1), Head(l2)]] ++ Zip(Tail(l1), Tail(l2))
	else
		[]
	end
end