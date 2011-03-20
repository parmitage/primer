Primer
======
Primer is a simple dynamically typed functional programming language with strict evaluation.

Grab the latest version from [here](http://github.com/parmitage/primer) and feel free to [email](mailto:philip.armitage@gmail.com) me.

Please set the environment variable `PRIMER_LIBRARY_PATH` to point to the Primer `lib` directory on your system.

The interpreter is invoked as follows:

    ./primer program.pri

The distribution contains a number of small programs in the `examples` directory.

Bindings are immutable.

    val pi = 3.14;

Primer is lexically scoped and bindings can be shadowed.

    let pi = 3.14159
        r  = 2
    in pi * r * r;

Functions are first class objects.

    val funSquared = fun f x -> f(x) * f(x);
    funSquared(fun x -> x / 3, 12);

Primer supports closures.

    val makeAdder = fun y -> fun a -> y + a;
    val add1 = makeAdder(1);
    val add2 = makeAdder(2);
    add1(2);
    add2(2);

Lists can be nested and are heterogeneous.

    val xs = [4, [5.32, [pi], [], true], funSquared, 'a', "aaa"];

Several built-in functions operate on lists.
 
    head(xs);
    tail(xs);
    last(xs1);
    length(xs);

A list can by accessed by index with the __at__ operator.

    xs at 4;

An item can be prepended to a list with the __cons__ operator.

    1 :: [2,3,4];

To concatenate two lists use the __append__ operator.

    [1,2,3] ++ [4,5,6];

Strings are lists of characters.

    head("hello");
    "hello" ! 3;

Comments are introduced with the # character.

    # comments extend to the end of the line
    # there are no block comments

The type of a value can be tested with the __is__ operator.

    123.45 is string;                       # false
    pi is float;                            # true

The __as__ operator converts between types.

    123.45 as string;                       # "123.45"
    "123.45" as float;                      # 123.45
    1 as bool;                              # true

The __if__ statement is an expression so the __else__ branch is mandatory.

    val count = fun xs ->
        if xs != []
        then 1 + count(tail(xs))
        else 0;

Tail-recursive functions are optimised as in this accumulator version of count.

    val count = fun xs ->
        let counter = fn (a, xs)
            if xs != []
            then counter(a + 1, tail(xs))
            else a
        in counter(0, xs);

Libraries are loaded from the `lib` directory with the __using__ statement:

    using base;

The base library contains the most commonly used functions including:

__map__ applies a function to every element in a list.

    map(fun x -> 2 * x, [1,2,3,4,5]);       # [2,4,6,8,10]

__foldl__ and __foldr__ combine successive list elements.

    foldl(fun x y -> x + y, 0, [1,2,3,4]);  # (((0+1)+2)+3)+4 = 10
    foldr(fun x y -> x + y, 0, [1,2,3,4]);  # 1+(2+(3+4)) = 10

__filter__ returns list elements which satisfy a predicate function.

    filter(odd, [1,2,3,2,4,5,6]);           # [1,3,5]

__zip__ combines two lists pairwise to return a new list of 2-tuples.

    zip(l1, [4,5,6]);                       # [[1,4],[2,5],[3,6]]

__reverse__ returns a new list in reverse order.

    reverse("hello");                       # "olleh"

__intersperse__ an atom between list elements.

    intersperse('A', l1);                   # [1,'A',2,'A',3]

__take__ the first n elements of a list.

    take(2, [1,2,3,4,5,6]);                 # [1,2]

__drop__ the first n elements from a list.

    drop(1, [1,2,3,4,5,6]);                 # [2,3,4,5,6]

__takeWhile__ returns items from a list until a predicate returns false.

    takeWhile(even, [2,4,6,1,3,5]);         # [2,4,6]

__dropWhile__ drops items from a list until a predicate returns false.

    dropWhile(odd, [1,3,5,2,4,6]);          # [2,4,6]

__any__ returns true if any item in a list satisfies a predicate.

    any(even, [1,2,3,4,5,6]);               # true

__all__ returns true if all items in a list satisfy a predicate.

    all(odd, [1,2,3,4,5,6]);                # false

__min__ returns the smallest item in a list of orderable items.

    min([7,2,4,5,3,8,6]);                   # 2

__max__ returns the largest item in a list of orderable items.

    max([7,2,4,5,3,8,6]);                   # 8

__sum__ returns the sum of items in a numeric list.

    sum([1,2,3,4,5]);                       # 15

__product__ returns the product of items in a numeric list.

    product([1,2,3,4,5]);                   # 120

__sort__ an orderable list of items.

    sort([4,2,8,1]);                        # [1,2,4,8]

__find__ an element in a list.

    find(3, [1,2,3,4,5]);                   # 3

__findByFun__ find an element in a list with a predicate.

    findByFun('b', key, [[a,1], [b,2]]);    # [b,2]

__replace__ an element in a list.

    replace(2, 'a', [1,2,3,4,5]);           # [1,'a',3,4,5]

__partition__ a list into elements which do and don't satisfy a predicate.

    partition(even, [1,2,3,4,5,6]);         # [[6,4,2],[5,3,1]]

__group__ a list into a list of lists of equal adjacent elements.

    group([1,1,2,3,4,4,5,6,6,6]);           # [[1,1],[2],[3],[4,4],[5],[6,6,6]]

__mapPair__

__collect__