assert = fn (id, act, exp)
  if act != exp then
    show(id)
  else nil
end

map = fn (f, xs)
  if xs then
    cons(map(f, tail(xs)), f(head(xs)))
  else nil
end

reduce = fn (f, init, xs)
  if xs then
    reduce(f, f(init, head(xs)), tail(xs))
  else init
end

reduceRight = fn (f, init, xs)
  if xs then
    f(head(xs), reduceRight(f, init, tail(xs)))
  else init
end

filter = fn (f, xs)
  if xs then
    if f(head(xs)) then
      [head(xs)] ++ filter(f, tail(xs))
    else filter(f, tail(xs))
  else nil
end

empty = fn (xs) length(xs) == 0 end

reverse = fn (xs)
  if tail(xs) then
    reverse(tail(xs)) ++ [head(xs)]
  else xs
end

find = fn (a, xs)
  if head(xs) == a then
    head(xs)
  else
    if tail(xs) then
      find(a, tail(xs))
    else nil
end

findByFn = fn (a, f, xs)
  if f(head(xs)) == a then
    head(xs)
  else
    if tail(xs) then
      findByFn(a, f, tail(xs))
    else nil
end

replace = fn (a, b, xs)
  if xs then  
    if head(xs) == a then
      [b] ++ replace(a, b, tail(xs))
    else [head(xs)] ++ replace(a, b, tail(xs))
  else []
end

sum = fn (xs)
  if xs then
    head(xs) + sum(tail(xs))
  else 0
end

product = fn (xs)
  if empty(xs) then 0
  else inner(xs)
  where inner = fn (xs)
          if xs then head(xs) * inner(tail(xs))
          else 1
        end
end

any = fn (pred, xs)
  if xs then
    if pred(head(xs)) then true
    else any(pred, tail(xs))
  else false
end

all = fn (pred, xs)
  if l then
    if pred(head(xs)) then
      all(pred, tail(xs))
    else false
  else true
end

take = fn (n, xs)
  inner(0, xs)
  where inner = fn (x, xs)
          if xs != nil and x < n then
            [head(xs)] ++ inner(x + 1, tail(xs))
          else []
        end
end

takewhile = fn (f, xs)
  if xs != nil and f(head(xs)) == true then
    [head(xs)] ++ takewhile(f, tail(xs))
  else []
end

drop = fn (n, xs)
  if n >= length(xs) then []
  else inner(0, xs)
  where inner = fn (x, xs)
    if xs then
      if x < n then inner(x + 1, tail(xs))
      else xs
    else xs
  end  
end

dropwhile = fn (f, xs)
  if xs then
    if f(head(xs)) then dropwhile(f, tail(xs))
    else xs
  else xs
end

sort = fn (l)
  if l then sort(filter(lt, xs)) ++ [x] ++ sort(filter(gte, xs))
  else []
  where x = head(l)
        xs = tail(l)
        lt = fn (a) a < x end
        gte = fn (a) a >= x end
end

zip = fn (l1, l2)
  if l1 != [] and l2 != [] then
    [[head(l1), head(l2)]] ++ zip(tail(l1), tail(l2))
  else []
end

range = fn (from, to)
  inner([], to)
  where inner = fn (list, count)
          if count < from then list
          else inner([count] ++ list, count - 1)
        end
end

intersperse = fn (sep, l)
  if length(l) == 1 then [head(l)]
  else [head(l)] ++ [sep] ++ intersperse(sep, tail(l))
end

min = fn (xs) head(sort(xs)) end
max = fn (xs) head(reverse(sort(xs))) end
last = fn (xs) head(reverse(xs)) end
odd = fn (n) n mod 2 != 0 end
even = fn (n) n mod 2 == 0 end

isint = fn (a) type(a) == 1 end
isfloat = fn (a) type(a) == 2 end
ischar = fn (a) type(a) == 5 end
isbool = fn (a) type(a) == 3 end
isfn = fn (a) type(a) == 6 end