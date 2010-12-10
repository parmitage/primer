##############################################################################
#
# This is the Primer standard library. The environment variable
# PRIMER_LIBRARY_PATH should point at the directory where this file lives.
#
##############################################################################

### returns true if xs is the empty list
Empty: fn (xs) Head(xs) == [] end

### simple assert function used by Tests.pri
Assert: fn (id, act, exp)
   if act != exp then Show(id)
   else true
end

### map f over xs returning a new list of the resultant values
Map: fn (f, xs)
   Inner(xs, [])
   where Inner: fn (xs, accum)
            if Head(xs) != [] then Inner(Tail(xs), f(Head(xs)) :: accum)
            else Reverse(accum)
         end
end

### left fold a function f over xs
FoldL: fn (f, init, xs)
   if Head(xs) != [] then FoldL(f, f(init, Head(xs)), Tail(xs))
   else init
end

### right fold a function f over xs
FoldR: fn (f, init, xs)
   if Head(xs) != [] then f(Head(xs), FoldR(f, init, Tail(xs)))
   else init
end

### return the elements of xs which satisfy predicate f
Filter: fn (f, xs)
   Inner(xs, [])
   where Inner: fn (ys, accum)
            if Head(ys) != [] then
               if f(Head(ys)) then Inner(Tail(ys), accum ++ [Head(ys)])
               else Inner(Tail(ys), accum)
            else accum
         end
end

### return a new list which is the reverse of xs
Reverse: fn (xs)
   Inner(xs, [])
   where Inner: fn (xs, accum)
            if Head(xs) != [] then Inner(Tail(xs), Head(xs) :: accum)
            else accum
         end
end

### find element a in xs
Find: fn (a, xs)
   if Head(xs) == a then Head(xs)
   else if Tail(xs) != [] then Find(a, Tail(xs))
   else false
end

### find an element such that f(a) is a member of xs
FindByFn: fn (a, f, xs)
   if f(Head(xs)) == a then Head(xs)
   else if Tail(xs) != [] then FindByFn(a, f, Tail(xs))
   else false
end

### replace all occurences of a in xs with b
Replace: fn (a, b, xs)
   if Head(xs) != [] then  
      if Head(xs) == a then b :: Replace(a, b, Tail(xs))
      else Head(xs) :: Replace(a, b, Tail(xs))
   else []
end

### return the sum of the elements in xs
Sum: fn (xs)
  if Head(xs) != [] then Head(xs) + Sum(Tail(xs))
  else 0
end

### return the product of the elements in xs
Product: fn (xs)
  if Empty(xs) then 0
  else Inner(xs)
  where Inner: fn (xs)
           if Head(xs) != [] then Head(xs) * Inner(Tail(xs))
           else 1
        end
end

### return true if any element of xs passes predicate pred
Any: fn (pred, xs)
   if Head(xs) != [] then
      if pred(Head(xs)) then true
      else Any(pred, Tail(xs))
   else false
end

### return true if all elements of xs pass predicate pred
All: fn (pred, xs)
   if Head(xs) != [] then
      if pred(Head(xs)) then All(pred, Tail(xs))
      else false
   else true
end

### take n elements from the head of xs
Take: fn (n, xs)
   Inner(0, xs)
   where Inner: fn (a, xs)
            if Head(xs) != [] and a < n then
               Head(xs) :: Inner(a + 1, Tail(xs))
            else []
        end
end

### take elements from the head of xs while f is true
TakeWhile: fn (f, xs)
   if Head(xs) != [] and f(Head(xs)) then
      Head(xs) :: TakeWhile(f, Tail(xs))
   else []
end

### drop n elements from xs
Drop: fn (n, xs)
   if n >= Length(xs) then []
   else Inner(0, xs)
   where Inner: fn (a, xs)
            if Head(xs) != [] then
               if a < n then Inner(a + 1, Tail(xs))
               else xs
            else xs
         end  
end

### drop elements from the head of xs while f is true
DropWhile: fn (f, xs)
  if Head(xs) != [] then
    if f(Head(xs)) then DropWhile(f, Tail(xs))
    else xs
  else Tail(xs)
end

### sort the elements of xs
Sort: fn (xs)
  if Head(xs) != []
  then Sort(Filter(Lt, Tail(xs))) ++ [Head(xs)] ++ Sort(Filter(Gte, Tail(xs)))
  else []
  where Lt: fn (a) a < Head(xs) end
        Gte: fn (a) a >= Head(xs) end
end

### sort the elements of xs by means of a selector function
SortBy: fn (xs, f)
  if Head(xs) != []
  then SortBy(Filter(Lt, Tail(xs)), f) ++ [Head(xs)] ++ SortBy(Filter(Gte, Tail(xs)), f)
  else []
  where Lt: fn (a) f(a) < f(Head(xs)) end
        Gte: fn (a) f(a) >= f(Head(xs)) end
end

### combine the elements of xs and ys pairwise
Zip: fn (xs, ys)
   if Head(xs) != [] and Head(ys) != [] then
      [Head(xs), Head(ys)] :: Zip(Tail(xs), Tail(ys))
   else []
end

### intersperse the elements of xs with sep
Intersperse: fn (sep, xs)
   if Head(xs) == [] then []
   else if Empty(Tail(xs)) then [Head(xs)]
   else Head(xs) :: sep :: Intersperse(sep, Tail(xs))
end

### return the smallest element of xs
Min: fn (xs) Head(Sort(xs)) end

### return the largest element of xs
Max: fn (xs) Head(Reverse(Sort(xs))) end

### return the last element of xs
Last: fn (xs) Head(Reverse(xs)) end

### return true if n is odd
Odd: fn (n) n mod 2 != 0 end

### return true if n is false
Even: fn (n) n mod 2 == 0 end

### run f n times collecting the results
Collect: fn (f, n)
   Inner(f, 0)
   where Inner: fn (f, c)
            if c < n
            then f() :: Inner(f, c + 1)
            else []
         end
end

### map a function over a list two elements at a time
MapPair: fn (f, xs)
   if Length(xs) >= 2
   then f(Head(xs), xs at 1) :: MapPair(f, Drop(2, xs))
   else []
end

### is bit b set in integer n
BitSet: fn (n, b) (n & (1 << b)) > 0 end