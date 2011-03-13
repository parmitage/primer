using "base";

val square = fun x -> x * x;
val cube = fun x -> x * x * x;
val abs = fun x -> if x >= 0 then x else -x;

val sqrt = fun x ->
    let avg = fun x y -> (x + y) / 2 in
    let improve = fun guess x ->
        avg(guess, x / guess) in
    let goodEnough = fun guess x ->
        abs(square(guess) - x) < 0.001 in
    let sqrtIter = fun guess x ->
        if goodEnough(guess, x)
        then guess
        else sqrtIter(improve(guess, x), x) in
    sqrtIter(1.0, x);

val mean = fun s -> sum(s) / length(s);

val median = fun s ->
    let s2 = sort(s) in
    let len = length(s) in
    let mid = floor((len - 1) / 2) in
        if len > 0
        then if odd(len)
             then s2 at mid
             else ((s2 at mid) + (s2 at mid + 1)) / 2
        else "median of empty list";

val mode = fun xs ->
    let counts = map(fun l -> [head(l), length(l)], group(sort(xs))) in
    let bestFirst = sortBy(counts, snd) in
        head(last(bestFirst));

val variance = fun s ->
    sum(map(fun x -> square(x - mean(s)), s)) / length(s);

val standardDeviation = fun s -> sqrt(variance(s));