def null = fn (x)
	x
end

def assert = fn (id, actual, expected)
	if (actual != expected)
		show(id)
	end
end

def length = fn (list)
	if (list != nil)
		1 + length(tail(list))
	else
		0
	end
end

def map = fn (f, list)
	if (list != nil)
		cons(map(f, tail(list)), f(head(list)))
	else
		nil
	end
end

def foldl = fn (f, init, list)
	if (list != nil)
		foldl(f, f(init, head(list)), tail(list))
	else
		init
	end
end

def foldr = fn (f, init, list)
	if (list != nil)
		f(head(list), foldr(f, init, tail(list)))
	else
		init
	end
end

def filter = fn (f, list)
	if (list != nil)
		if (f(head(list)) == true)
			cons(filter(f, tail(list)), head(list))
		else
			filter(f, tail(list))
		end
	else
		nil
	end
end

def empty = fn (list)
	length(list) == 0
end

def reverse = fn (list)
	if (tail(list) != nil)
		reverse(tail(list)) ++ [head(list)]
	else
		list
	end
end

def find = fn (x, list)
	if (head(list) == x)
		head(list)
	else
		if (tail(list) != nil)
			find(x, tail(list))
		else
			nil
		end
	end
end

def sum = fn (list)
	if (list != nil)
		head(list) + sum(tail(list))
	else
		0
	end
end

def product = fn (l)
	def inner = fn (l)
		if (l != nil)
			head(l) * inner(tail(l))
		else
			1
		end
	end
	if (empty(l))
		0
	else
		inner(l)
	end
end

def nth = fn (list, n)
	def inner = fn (list, x)
		if (list != nil)
			if (x == n)
				head(list)
			else
				inner(tail(list), x + 1)
			end
		else
			nil
		end
	end
	inner(list, 0)
end

def any = fn (pred, list)
	if (list != nil)
		if (pred(head(list)) == true)
			true
		else
			any(pred, tail(list))
		end
	else
		false
	end
end

def all = fn (pred, list)
	if (list != nil)
		if (pred(head(list)) == true)
			all(pred, tail(list))
		else
			false
		end
	else
		true
	end
end

def take = fn (n, list)
	def inner = fn (x, list)
		if (list != nil && x < n)
			[head(list)] ++ inner(x + 1, tail(list))
		else
			[]
		end
	end
	inner(0, list)
end

def takewhile = fn (f, list)
	if (list != nil && f(head(list)) == true)
		[head(list)] ++ takewhile(f, tail(list))
	else
		[]
	end
end

def drop = fn (n, list)
	def inner = fn (x, list)
		if (list != nil)
			if (x < n)
				inner(x + 1, tail(list))
			else
				list
			end
		else
			list
		end
	end
	if (n >= length(list))
		[]
	else
		inner(0, list)
	end
end

def dropwhile = fn (f, list)
	if (list != nil)
		if (f(head(list)) == true)
			dropwhile(f, tail(list))
		else
			list
		end
	else
		list
	end
end

def sort = fn (l)
	if (l != [])
		def x = head(l)
		def xs = tail(l)
		sort(filter(fn (a) a < x end, xs)) ++ [x] ++ sort(filter(fn (a) a >= x end, xs))
	else
		[]
	end
end

def min = fn (l)
	head(sort(l))
end

def max = fn (l)
	head(reverse(sort(l)))
end

def last = fn (l)
	head(reverse(l))
end

def odd = fn (z)
	z % 2 != 0
end

def even = fn (z)
	z % 2 == 0
end

def integer = fn (z)
	type(z) == 1
end

def float = fn (z)
	type(z) == 2
end

def char = fn (z)
	type(z) == 5
end

def bool = fn (z)
	type(z) == 3
end

def function = fn (z)
	type(z) == 6
end

def zip = fn (l1, l2)
	if (l1 != [] && l2 != [])
		[[head(l1), head(l2)]] ++ zip(tail(l1), tail(l2))
	else
		[]
	end
end