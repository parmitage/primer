empty = fn (xs) head(xs) == [] end

assert = fn (id, act, exp)
   if act != exp then show(id)
   else true
end

map = fn (f, xs)
   inner(xs, [])
   where inner = fn (xs, accum)
            if head(xs) != [] then inner(tail(xs), f(head(xs)) : accum)
            else reverse(accum)
         end
end

foldl = fn (f, init, xs)
   if head(xs) != [] then foldl(f, f(init, head(xs)), tail(xs))
   else init
end

foldr = fn (f, init, xs)
   if head(xs) != [] then f(head(xs), foldr(f, init, tail(xs)))
   else init
end

filter = fn (f, xs)
   inner(xs, [])
   where inner = fn (ys, accum)
            if head(ys) != [] then
               if f(head(ys)) then inner(tail(ys), accum ++ [head(ys)])
               else inner(tail(ys), accum)
            else accum
         end
end

reverse = fn (xs)
   inner(xs, [])
   where inner = fn (xs, accum)
            if head(xs) != [] then inner(tail(xs), head(xs) : accum)
            else accum
         end
end

find = fn (a, xs)
   if head(xs) == a then head(xs)
   else if tail(xs) != [] then find(a, tail(xs))
   else false
end

findByFn = fn (a, f, xs)
   if f(head(xs)) == a then head(xs)
   else if tail(xs) != [] then findByFn(a, f, tail(xs))
   else false
end

replace = fn (a, b, xs)
   if head(xs) != [] then  
      if head(xs) == a then b : replace(a, b, tail(xs))
      else head(xs) : replace(a, b, tail(xs))
   else []
end

sum = fn (xs)
  if head(xs) != [] then head(xs) + sum(tail(xs))
  else 0
end

product = fn (xs)
  if empty(xs) then 0
  else inner(xs)
  where inner = fn (xs)
           if head(xs) != [] then head(xs) * inner(tail(xs))
           else 1
        end
end

any = fn (pred, xs)
   if head(xs) != [] then
      if pred(head(xs)) then true
      else any(pred, tail(xs))
   else false
end

all = fn (pred, xs)
   if head(xs) != [] then
      if pred(head(xs)) then all(pred, tail(xs))
      else false
   else true
end

take = fn (n, xs)
   inner(0, xs)
   where inner = fn (a, xs)
            if head(xs) != [] and a < n then
               head(xs) : inner(a + 1, tail(xs))
            else []
        end
end

takewhile = fn (f, xs)
   if head(xs) != [] and f(head(xs)) then
      head(xs) : takewhile(f, tail(xs))
   else []
end

drop = fn (n, xs)
   if n >= length(xs) then []
   else inner(0, xs)
   where inner = fn (a, xs)
            if head(xs) != [] then
               if a < n then inner(a + 1, tail(xs))
               else xs
            else xs
         end  
end

dropwhile = fn (f, xs)
  if head(xs) != [] then
    if f(head(xs)) then dropwhile(f, tail(xs))
    else xs
  else tail(xs)
end

sort = fn (xs)
  if head(xs) != []
  then sort(filter(lt, tail(xs))) ++ [head(xs)] ++ sort(filter(gte, tail(xs)))
  else []
  where lt = fn (a) a < head(xs) end
        gte = fn (a) a >= head(xs) end
end

zip = fn (xs, ys)
   if head(xs) != [] and head(ys) != [] then
      [head(xs), head(ys)] : zip(tail(xs), tail(ys))
   else []
end

intersperse = fn (sep, xs)
   if head(xs) == [] then []
   else if empty(tail(xs)) then [head(xs)]
   else head(xs) : sep : intersperse(sep, tail(xs))
end

min = fn (xs) head(sort(xs)) end
max = fn (xs) head(reverse(sort(xs))) end
last = fn (xs) head(reverse(xs)) end
odd = fn (n) n mod 2 != 0 end
even = fn (n) n mod 2 == 0 end

isint = fn (a) type(a) == 0 end
isfloat = fn (a) type(a) == 1 end
isbool = fn (a) type(a) == 2 end
ischar = fn (a) type(a) == 4 end
islist = fn (a) type(a) == 5 end
isfn = fn (a) type(a) == 7 end
isstring = fn (a) islist(a) and all(fn (c) ischar(c) end, a) end