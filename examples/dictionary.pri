using base;

val add = fun d k v -> d ++ [[k, v]];

val get = fun d k ->
    let v = findByFun(k, fun x -> x at 0, d) in
        if v != false then head(tail(v))
        else [];

val update = fun ds k v ->
    if head(ds) != []
    then if head(head(ds)) == k
         then add(tail(ds), k, v)
         else head(ds) :: update(tail(ds), k, v)
    else head(ds);

val remove = fun ds k ->
    if head(ds) != []
    then if head(head(ds)) == k
         then tail(ds)
         else head(ds) :: remove(tail(ds), k)
    else head(ds);

val d = [];
val d1 = add(d, "one", 1);
val d2 = add(d1, "two", 2);
val d3 = add(d2, "three", 3);

show(d3);
show(get(d3, "one"));
show(get(d3, "four"));
show(update(d3, "one", 5));
show(update(d3, "two", 5));
show(update(d3, "three", 5));
show(update(d3, "four", 5));
show(remove(d3, "one"));
show(remove(d3, "two"));
show(remove(d3, "three"));
show(remove(d3, "four"));