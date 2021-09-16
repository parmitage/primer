Primer
======
Primer is the scripting language used in the PAGE1 game engine.

Primer in bullet points
-----------------------
- dynamically typed
- immutable
- strict
- higher order functions and closures
- batch interpreter, REPL and compiler
- builds on Linux, Mac and Windows
- MIT licensed

Download and install
--------------------
Inside the tarball you'll find the following:

- src/       - the C source code
- lib/       - the Primer standard library
- emacs/     - an Emacs major mode for Primer
- examples/  - some example Primer programs
- proto      - the OCaml prototype implementation
- tests/     - some test scripts
- README.md  - this file
- LICENSE    - the MIT license

If you wish to use the C implementation, you must install gcc, GNU Make, flex and bison.

To get started with the prototype implementation, you must install OCaml (at least version 3.11) as well as GNU Make.

The makefile builds across Linux, Mac and Windows without the use of autotools so there is no separate configure step. Once Primer has built, set the environment variable __PRIMER_LIBRARY_PATH__ to point to the __lib__ directory on your computer.

Hello, World!
-------------
I've chosen Quicksort as Primer's "Hello, World!" example:

    val sort = fun xs ->
       let lt  = fun a -> a < head(xs)
           gte = fun a -> a >= head(xs)
       in if head(xs) != [] then
             sort(filter(lt, tail(xs))) ++ [head(xs)]
                             ++ sort(filter(gte, tail(xs)))
          else [];

Note that while this version of Quicksort is easy to read, it isn't tail-recursive. The version in the standard library is implemented using CPS so is suitable for use on longer lists.

Interpreter and compiler
------------------------
The quickest way to get started is at the REPL:

    ./pri

Alternatively, to load an existing Primer source file, simply pass it as an argument:

    ./pri file.pri

To compile to JavaScript:

    ./prc file.pri file.js

To compile to JavaScript embedded inside a HTML file:

    ./prc file.pri file.html

Types
-----
Primer is (currently) dynamically typed and provides int, float, char, bool, string and list types. There is no concept of null in Primer.

The type of a value can be tested with the __is__ operator.

    123.45 is string;                       # false
    pi is float;                            # true

The __as__ operator converts between types.

    123.45 as string;                       # "123.45"
    "123.45" as float;                      # 123.45

Definitions and functions
-------------------------
Bindings are introduced with __val__ and are immutable.

    val pi = 3.14159;

The __let__ keyword introduces local definitions.

    let x = 12 in x + 4;

The body of a function is a single expression, the value of which is the return value.

    val areaOfCircle = fn (r) pi * r * r;

Functions are first class objects, can be passed anonymously and are higher order.

    val simpleOpSquared = fn (f, x) f(x) * f(x);
    simpleOpSquared(fn (x) x / 3, 12)       # 16

Primer supports closures (which are also immutable).

    val makeAdder = fn (y) fn (a) y + a;
    val add2 = MakeAdder(2);
    add2(2);                                # 4

Tail-recursive functions are optimised. This function counts the number of elements in a list using an accumulator to avoid a stack overflow.

    val count = fn (xs)
       let counter = fn (a, xs)
          if xs != []
          then counter(a + 1, tail(xs))
          else a
       in counter(0, xs);

Lists and Strings
-----------------
Lists can be nested and are heterogeneous.

    val xs = [4, [5.32, [pi], [], true], AreaOfCircle, 'a', "aaa"];

Several functions provide access to list elements.
 
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

Strings are just lists of characters so many list operations and functions will work with them.

    head("hello");
    "hello" ! 3;

Conditionals
------------
Because __if__ is an expression, and so must return a value, the __else__ branch is mandatory.

    val count = fn (xs)
       if xs != []
       then 1 + count(tail(xs))
       else 0;

The if expression can be nested although for case analysis, the __match__ expression may be used. It supports very limited patterns but these may be extended in the future. The underscore matches any expression.

    match x, y
      with 1, 2 then "one and two"
      with 2, 3 then "two and three"
      with 3, _ then "three and anything"
      with _, _ then "anything and anything";

Sequencing
----------
Although Primer is broadly speaking a functional programming language, it permits simple imperative programming by sequencing expressions in a __begin__...__end__ block.

    begin
       show(1);
       show(2);
       show(3);
    end;

Each expression is evaluated in turn and the value of the block is the value of the last expression. Blocks are typically used for sequencing output operations although unlike monadic IO in Haskell, they have no special meaning in the type system.

Libraries
---------
You can group related definitions into a library file under your __lib__ directory. Libraries are loaded with the __using__ keyword.

    using "mylib";                          # loads mylib.pri

Note that when a library is loaded, definitions at the top level are interned but other expressions are ignored.

Standard Library
----------------
Primer's modest standard library can be found in __base.pri__ and loaded with:

    using "base";

It contains a small selection of useful functions:

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

__last__ returns the last element of a list.

    last([1,2,3,4]);                       # 4

__takeWhile__ returns items from a list until a predicate returns false.

    takeWhile(even, [2,4,6,1,3,5]);         # [2,4,6]

__dropWhile__ drops items from a list until a predicate returns false.

    dropWhile(odd, [1,3,5,2,4,6]);          # [2,4,6]

__empty__ returns true if a list is empty.

    empty([1,2,3]);                         # false

__any__ returns true if any item in a list satisfies a predicate.

    any(even, [1,2,3,4,5,6]);               # true

__all__ returns true if all items in a list satisfy a predicate.

    all(odd, [1,2,3,4,5,6]);                # false

__min__ returns the smallest item in a list of order-able items.

    min([7,2,4,5,3,8,6]);                   # 2

__max__ returns the largest item in a list of order-able items.

    max([7,2,4,5,3,8,6]);                   # 8

__odd__ returns true if its argument is odd.

    odd(2);                                 # false
    
__even__ returns true if its argument is even.

    even(2);                                # true

__sum__ returns the sum of items in a numeric list.

    sum([1,2,3,4,5]);                       # 15

__product__ returns the product of items in a numeric list.

    product([1,2,3,4,5]);                   # 120

__sort__ an order-able list of items.

    sort([4,2,8,1]);                        # [1,2,4,8]

__qsort__ is a tail recursive version of sort.

    qsort([4,2,8,1]);                       # [1,2,4,8]

__sortBy__ sorts a list by a applying a function to its elements.

    sortBy(dict, key);                      # [['A',1], ['B',2]]

__qsortBy__ tail recursive version of sortBy.

    qsortBy(dict, key);                     # [['A',1], ['B',2]]

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

__mapPair__ apply a function to a list two elements at a time.

    mapPair(fun x y -> x + y, [1,2,3,4]);   # [3, 7]

__collect__ invoke a function a number of times, collecting the results.

    collect(randomInt, 4);                  # [473, 111829, 455, 9]

__bitSet__ tests if a single bit is set in a byte.

    bitSet(byte, 3);                        # true

Example programs
----------------
The __examples__ directory contains a few simple programs:

- dictionary.pri         - a horribly inefficient dictionary
- euler.pri              - a few Project Euler problems
- factorial.pri          - the factorial function
- fizzbuzz.pri           - the fizzbuzz party game
- knapsack.pri           - a genetic algorithm solution to the knapsack problem
- object.pri             - a port of Oleg Kiselyov's "Purely-functional OO system"
- rover.pri              - the Mars Rover problem
- search.pri             - an implementation of binary search

You can compile the example programs with:

    make examples

Editing
-------
Emacs users will find the beginnings of a major mode in the __emacs__ directory of the source distribution. It currently provides only syntax highlighting but may be extended to support automatic indentation as well as integration with Primer's REPL and compiler.
