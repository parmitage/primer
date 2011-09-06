#############################################################################
#
# Based on the rational number example from SICP.
#
#############################################################################

val gcd = fun a b ->
    if b == 0
    then a
    else gcd(b, a mod b);

val makeRat = fun n d ->
    let g = gcd(n, d)
    in (n / g) :: (d / g) :: [];

val numer = fun x -> x at 0;
val denom = fun x -> x at 1;

val printRat = fun x ->
    let numer = numer(x) as string
        denom = denom(x) as string
        ret = numer ++ "/" ++ denom
    in show(ret);

val addRat = fun x y ->
    let numer = (numer(x) * denom(y)) + (numer(y) * denom(x))
        denom = denom(x) * denom(y)
    in makeRat(numer, denom);

val mulRat = fun x y ->
    let numer = numer(x) * numer(y)
        denom = denom(x) * denom(y)
    in makeRat(numer, denom);

val oneHalf = makeRat(1, 2);
val oneThird = makeRat(1, 3);

printRat(addRat(oneHalf, oneThird));
printRat(mulRat(oneHalf, oneThird));
printRat(addRat(oneThird, oneThird));