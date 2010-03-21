def null = fn (x)
  x
end

def assert = fn (id, actual, expected)
  if actual != expected then
    show(id)
  end
end

def length = fn (list)
  def inner = fn(list, accum)
    if list != nil then
      inner(tail(list), accum + 1)
    else
      accum
    end
  end
  inner(list, 0)
end

def map = fn (f, list)
  if list != nil then
    [f(head(list))] ++ map(f, tail(list))
  else
    nil
  end
end

def foldl = fn (f, init, list)
  if list != nil then
    foldl(f, f(init, head(list)), tail(list))
  else
    init
  end
end

def foldr = fn (f, init, list)
  if list != nil then
    f(head(list), foldr(f, init, tail(list)))
  else
    init
  end
end

def filter = fn (f, list)
  if list != nil then
    if f(head(list)) == true then
      [head(list)] ++ filter(f, tail(list))
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
  if tail(list) != nil then
    reverse(tail(list)) ++ [head(list)]
  else
    list
  end
end

def find = fn (x, list)
  if head(list) == x then
    head(list)
  else
    if tail(list) != nil then
      find(x, tail(list))
    else
      nil
    end
  end
end

def sum = fn (list)
  if list != nil then
    head(list) + sum(tail(list))
  else
    0
  end
end

def product = fn (l)
  def inner = fn (l)
    if l != nil then
      head(l) * inner(tail(l))
    else
      1
    end
  end
  if empty(l) then
    0
  else
    inner(l)
  end
end

def nth = fn (list, n)
  def inner = fn (list, x)
    if list != nil then
      if x == n then
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
  if list != nil then
    if pred(head(list)) == true then
      true
    else
      any(pred, tail(list))
    end
  else
    false
  end
end

def all = fn (pred, list)
  if list != nil then
    if pred(head(list)) == true then
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
    if list != nil and x < n then
      [head(list)] ++ inner(x + 1, tail(list))
    else
      []
    end
  end
  inner(0, list)
end

def takewhile = fn (f, list)
  if list != nil and f(head(list)) == true then
    [head(list)] ++ takewhile(f, tail(list))
  else
    []
  end
end

def drop = fn (n, list)
  def inner = fn (x, list)
    if list != nil then
      if x < n then
        inner(x + 1, tail(list))
      else
        list
      end
    else
      list
    end
  end
  if n >= length(list) then
    []
  else
    inner(0, list)
  end
end

def dropwhile = fn (f, list)
  if list != nil then
    if f(head(list)) == true then
      dropwhile(f, tail(list))
    else
      list
    end
  else
    list
  end
end

def sort = fn (l)
  if l != [] then
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
  z mod 2 != 0
end

def even = fn (z)
  z mod 2 == 0
end

def isint = fn (z)
  type(z) == 1
end

def isfloat = fn (z)
  type(z) == 2
end

def ischar = fn (z)
  type(z) == 5
end

def isbool = fn (z)
  type(z) == 3
end

def isfn = fn (z)
  type(z) == 6
end

def zip = fn (l1, l2)
  if l1 != [] and l2 != [] then
    [[head(l1), head(l2)]] ++ zip(tail(l1), tail(l2))
  else
    []
  end
end