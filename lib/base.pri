val empty = fun xs -> head(xs) == [] ;
val fst = fun p -> head(p);
val snd = fun p -> head(tail(p));
val cadr = snd;

val assert = fun id act exp ->
   if act != exp then show(id)
   else true ;

val map = fun f xs ->
   let inner = fun xs ac ->
      if head(xs) != []
      then inner(tail(xs), f(head(xs)) :: ac)
      else reverse(ac)
   in inner(xs, []) ;

val foldl = fun f init xs ->
   if head(xs) != []
   then foldl(f, f(init, head(xs)), tail(xs))
   else init ;

val filter = fun f xs ->
   let inner = fun ys accum ->
      if head(ys) != []
      then if f(head(ys))
           then inner(tail(ys), accum ++ [head(ys)])
           else inner(tail(ys), accum)
      else accum
   in inner(xs, []) ;

val foldr = fun f init xs ->
   if head(xs) != []
   then f(head(xs), foldr(f, init, tail(xs)))
   else init ;

val reverse = fun xs ->
   let inner = fun xs accum ->
      if head(xs) != []
      then inner(tail(xs), head(xs) :: accum)
      else accum
   in inner(xs, []) ;

val find = fun a xs ->
   if head(xs) == a
   then head(xs)
   else if tail(xs) != []
        then find(a, tail(xs))
        else false ;

val findByFun = fun a f xs ->
   if f(head(xs)) == a
   then head(xs)
   else if tail(xs) != []
        then findByFun(a, f, tail(xs))
        else false ;

val replace = fun a b xs ->
   if head(xs) != []
   then if head(xs) == a
        then b :: replace(a, b, tail(xs))
        else head(xs) :: replace(a, b, tail(xs))
   else [] ;

val sum = fun xs ->
   let inner = fun xs accum ->
      if head(xs) != []
      then inner(tail(xs), head(xs) + accum)
      else accum
   in inner(xs, 0) ;

val product = fun xs ->
   let inner = fun xs ->
      if head(xs) != []
      then head(xs) * inner(tail(xs))
      else 1
   in if empty(xs)
      then 0
      else inner(xs) ;

val any = fun pred xs ->
   if head(xs) != []
   then if pred(head(xs))
        then true
        else any(pred, tail(xs))
   else false ;

val all = fun pred xs ->
   if head(xs) != []
   then if pred(head(xs))
        then all(pred, tail(xs))
        else false
   else true ;

val take = fun n xs ->
   let inner = fun c xs ac ->
      if head(xs) != [] and c < n
      then inner(c + 1, tail(xs), head(xs) :: ac)
      else reverse(ac)
   in inner(0, xs, []) ;

val takeWhile = fun f xs ->
   if head(xs) != [] and f(head(xs))
   then head(xs) :: takeWhile(f, tail(xs))
   else [] ;

val drop = fun n xs ->
   let inner = fun a xs ->
      if head(xs) != []
      then if a < n
           then inner(a + 1, tail(xs))
           else xs
      else xs
   in if n >= length(xs)
      then []
      else inner(0, xs) ;

val dropWhile = fun f xs ->
   if head(xs) != []
   then if f(head(xs))
        then dropWhile(f, tail(xs))
        else xs
   else tail(xs) ;

val partition = fun f xs ->
   let inner = fun ts fs xs ->
      if head(xs) != []
      then if f(head(xs))
           then inner(head(xs) :: ts, fs, tail(xs))
           else inner(ts, head(xs) :: fs, tail(xs))
      else [ts, fs]
   in inner([], [], xs) ;

### Simple QuickSort - fast for short lists
val sort = fun xs ->
   let lt = fun a -> a < head(xs) in
   let gte = fun a -> a >= head(xs) in
   if head(xs) != []
   then sort(filter(lt, tail(xs))) ++ [head(xs)] ++ sort(filter(gte, tail(xs)))
   else [] ;

### Tail recursive QuickSort using CPS - OK for longer lists
val qSort = fun xs ->
   let loop = fun xs cont ->
      if head(xs) == []
      then cont([])
      else let x = head(xs) in
           let xss = tail(xs) in
           let parts = partition(fun y -> x > y, xss) in
           loop(parts at 0, fun lacc ->
           loop(parts at 1, fun racc -> cont(lacc ++ (x :: racc))))
   in loop(xs, fun x -> x) ;

val sortBy = fun xs f ->
   let lt = fun a -> f(a) < f(head(xs)) in
   let gte = fun a -> f(a) >= f(head(xs)) in
   if head(xs) != []
   then sortBy(filter(lt, tail(xs)), f) ++ [head(xs)] ++ sortBy(filter(gte, tail(xs)), f)
   else [] ;

val qSortBy = fun xs f ->
   let loop = fun xs cont ->
      if head(xs) == []
      then cont([])
      else let x = head(xs) in
           let xss = tail(xs) in
           let parts = partition(fun y -> f(x) > f(y), xss) in
           loop(parts at 0, fun lacc ->
           loop(parts at 1, fun racc -> cont(lacc ++ (x :: racc))))
   in loop(xs, fun x -> x) ;

val zip = fun xs ys ->
   if head(xs) != [] and head(ys) != []
   then [head(xs), head(ys)] :: zip(tail(xs), tail(ys))
   else [] ;

val intersperse = fun sep xs ->
   if head(xs) == []
   then []
   else if empty(tail(xs))
        then [head(xs)]
        else head(xs) :: sep :: intersperse(sep, tail(xs)) ;

val min = fun xs -> head(sort(xs)) ;
val max = fun xs -> head(reverse(sort(xs))) ;
val last = fun xs -> head(reverse(xs)) ;
val odd = fun n -> n mod 2 != 0 ;
val even = fun n -> n mod 2 == 0 ;
val bitSet = fun n b -> (n & (1 << b)) > 0 ;

val group = fun xs ->
    let inner = fun inxs curxs outxs prev ->
        if inxs == []
        then curxs::outxs
        else let x = head(inxs) in
             if x == prev or x == []
             then inner(tail(inxs), x::curxs, outxs, x)
             else inner(tail(inxs), [x], curxs::outxs, x)
    in reverse(inner(xs, [], [], []));

val collect = fun f n ->
   let inner = fun f c ->
      if c < n
      then f() :: inner(f, c + 1)
      else []
   in inner(f, 0) ;

val mapPair = fun f xs ->
   let inner = fun f xs ac ->
      if length(xs) >= 2
      then inner(f, drop(2, xs), f(head(xs), xs at 1) :: ac)
      else reverse(ac)
   in inner(f, xs, []) ;