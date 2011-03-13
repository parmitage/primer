#############################################################################
#
# This is the rational number example from SICP.
#
#############################################################################

val gcd = fun a b ->
    if b == 0
    then a
    else gcd(b, a mod b);

val makeRat = fun n d ->
    let g = gcd(n, d) in
    (n / g) :: (d / g) :: [];

val numer = fun x -> x at 0;
val denom = fun x -> x at 1;

val printRat = fun x -> show(numer(x) as string ++ "/" ++ denom(x) as string);

val addRat = fun x y ->
    let n = (numer(x) * denom(y)) + (numer(y) * denom(x)) in
    let d = denom(x) * denom(y) in
    makeRat(n, d);

val mulRat = fun x y ->
    let n = numer(x) * numer(y) in
    let d = denom(x) * denom(y) in
    makeRat(n, d);

val oneHalf = makeRat(1, 2);
val oneThird = makeRat(1, 3);

printRat(addRat(oneHalf, oneThird));
printRat(mulRat(oneHalf, oneThird));
printRat(addRat(oneThird, oneThird));