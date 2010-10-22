##############################################################################
#
# This is the Primer standard library. The environment variable
# PRIMER_LIBRARY_PATH should point at the directory where this file lives.
#
##############################################################################

### returns true if xs is the empty list
empty = fn (xs) head(xs) == [] end

### simple assert function used by Tests.pri
assert = fn (id, act, exp)
   if act != exp then show(id)
   else true
end

### map f over xs returning a new list of the resultant values
map = fn (f, xs)
   inner(xs, [])
   where inner = fn (xs, accum)
            if head(xs) != [] then inner(tail(xs), f(head(xs)) :: accum)
            else reverse(accum)
         end
end

### left fold a function f over xs
foldl = fn (f, init, xs)
   if head(xs) != [] then foldl(f, f(init, head(xs)), tail(xs))
   else init
end

### right fold a function f over xs
foldr = fn (f, init, xs)
   if head(xs) != [] then f(head(xs), foldr(f, init, tail(xs)))
   else init
end

### return the elements of xs which satisfy predicate f
filter = fn (f, xs)
   inner(xs, [])
   where inner = fn (ys, accum)
            if head(ys) != [] then
               if f(head(ys)) then inner(tail(ys), accum ++ [head(ys)])
               else inner(tail(ys), accum)
            else accum
         end
end

### return a new list which is the reverse of xs
reverse = fn (xs)
   inner(xs, [])
   where inner = fn (xs, accum)
            if head(xs) != [] then inner(tail(xs), head(xs) :: accum)
            else accum
         end
end

### find element a in xs
find = fn (a, xs)
   if head(xs) == a then head(xs)
   else if tail(xs) != [] then find(a, tail(xs))
   else false
end

### find an element such that f(a) is a member of xs
findByFn = fn (a, f, xs)
   if f(head(xs)) == a then head(xs)
   else if tail(xs) != [] then findByFn(a, f, tail(xs))
   else false
end

### replace all occurences of a in xs with b
replace = fn (a, b, xs)
   if head(xs) != [] then  
      if head(xs) == a then b :: replace(a, b, tail(xs))
      else head(xs) :: replace(a, b, tail(xs))
   else []
end

### return the sum of the elements in xs
sum = fn (xs)
  if head(xs) != [] then head(xs) + sum(tail(xs))
  else 0
end

### return the product of the elements in xs
product = fn (xs)
  if empty(xs) then 0
  else inner(xs)
  where inner = fn (xs)
           if head(xs) != [] then head(xs) * inner(tail(xs))
           else 1
        end
end

### return true if any element of xs passes predicate pred
any = fn (pred, xs)
   if head(xs) != [] then
      if pred(head(xs)) then true
      else any(pred, tail(xs))
   else false
end

### return true if all elements of xs pass predicate pred
all = fn (pred, xs)
   if head(xs) != [] then
      if pred(head(xs)) then all(pred, tail(xs))
      else false
   else true
end

### take n elements from the head of xs
take = fn (n, xs)
   inner(0, xs)
   where inner = fn (a, xs)
            if head(xs) != [] and a < n then
               head(xs) :: inner(a + 1, tail(xs))
            else []
        end
end

### take elements from the head of xs while f is true
takewhile = fn (f, xs)
   if head(xs) != [] and f(head(xs)) then
      head(xs) :: takewhile(f, tail(xs))
   else []
end

### drop n elements from xs
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

### drop elements from the head of xs while f is true
dropwhile = fn (f, xs)
  if head(xs) != [] then
    if f(head(xs)) then dropwhile(f, tail(xs))
    else xs
  else tail(xs)
end

### sort the elements of xs
sort = fn (xs)
  if head(xs) != []
  then sort(filter(lt, tail(xs))) ++ [head(xs)] ++ sort(filter(gte, tail(xs)))
  else []
  where lt = fn (a) a < head(xs) end
        gte = fn (a) a >= head(xs) end
end

### combine the elements of xs and ys pairwise
zip = fn (xs, ys)
   if head(xs) != [] and head(ys) != [] then
      [head(xs), head(ys)] :: zip(tail(xs), tail(ys))
   else []
end

### intersperse the elements of xs with sep
intersperse = fn (sep, xs)
   if head(xs) == [] then []
   else if empty(tail(xs)) then [head(xs)]
   else head(xs) :: sep :: intersperse(sep, tail(xs))
end

### return the smallest element of xs
min = fn (xs) head(sort(xs)) end

### return the largest element of xs
max = fn (xs) head(reverse(sort(xs))) end

### return the last element of xs
last = fn (xs) head(reverse(xs)) end

### return true if n is odd
odd = fn (n) n mod 2 != 0 end

### return true if n is false
even = fn (n) n mod 2 == 0 end

### test the type of a
isint = fn (a) type(a) == int end
isfloat = fn (a) type(a) == float end
isbool = fn (a) type(a) == bool end
ischar = fn (a) type(a) == char end
islist = fn (a) type(a) == 5 end
isfn = fn (a) type(a) == 7 end
isstring = fn (a) islist(a) and all(fn (c) ischar(c) end, a) end