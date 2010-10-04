head = fn (x:_) x end
tail = fn (_:xs) xs end
empty = fn (x:_) x == [] end

assert = fn (id, act, exp)
   if act != exp then show(id)
   else true
end

map = fn (f, xs)
   inner(xs, [])
   where inner = fn (x:xs, accum)
            if x != [] then inner(xs, f(x) : accum)
            else reverse(accum)
         end
end

foldl = fn (f, init, x:xs)
   if x != [] then foldl(f, f(init, x), xs)
   else init
end

foldr = fn (f, init, x:xs)
   if x != [] then f(x, foldr(f, init, xs))
   else init
end

filter = fn (f, xs)
   inner(xs, [])
   where inner = fn (y:ys, accum)
            if y != [] then
               if f(y) then inner(ys, accum ++ [y])
               else inner(ys, accum)
            else accum
         end
end

reverse = fn (xs)
   inner(xs, [])
   where inner = fn (x:xs, accum)
            if x != [] then inner(xs, x : accum)
            else accum
         end
end

find = fn (a, x:xs)
   if x == a then x
   else if xs != [] then find(a, xs)
   else false
end

findByFn = fn (a, f, x:xs)
   if f(x) == a then x
   else if xs != [] then findByFn(a, f, xs)
   else false
end

replace = fn (a, b, x:xs)
   if x != [] then  
      if x == a then b : replace(a, b, xs)
      else x : replace(a, b, xs)
   else []
end

sum = fn (x:xs)
  if x != [] then x + sum(xs)
  else 0
end

product = fn (xs)
  if empty(xs) then 0
  else inner(xs)
  where inner = fn (x:xs)
           if x != [] then x * inner(xs)
           else 1
        end
end

any = fn (pred, x:xs)
   if x != [] then
      if pred(x) then true
      else any(pred, xs)
   else false
end

all = fn (pred, x:xs)
   if x != [] then
      if pred(x) then all(pred, xs)
      else false
   else true
end

take = fn (n, xs)
   inner(0, xs)
   where inner = fn (a, x:xs)
            if x != [] and a < n then
               x : inner(a + 1, xs)
            else []
        end
end

takewhile = fn (f, x:xs)
   if x != [] and f(x) then
      x : takewhile(f, xs)
   else []
end

drop = fn (n, xs)
   if n >= length(xs) then []
   else inner(0, xs)
   where inner = fn (a, x:xs)
            if x != [] then
               if a < n then inner(a + 1, xs)
               else x : xs
            else x : xs
         end  
end

dropwhile = fn (f, x:xs)
  if x != [] then
    if f(x) then dropwhile(f, xs)
    else x : xs
  else xs
end

sort = fn (x:xs)
  if x != [] then sort(filter(lt, xs)) ++ [x] ++ sort(filter(gte, xs))
  else []
  where lt = fn (a) a < x end
        gte = fn (a) a >= x end
end

zip = fn (x:xs, y:ys)
   if x != [] and y != [] then
      [x, y] : zip(xs, ys)
   else []
end

intersperse = fn (sep, x:xs)
   if x == [] then []
   else if empty(xs) then [x]
   else x : sep : intersperse(sep, xs)
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