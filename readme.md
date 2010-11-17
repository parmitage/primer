% Primer
% Phil Armitage
% 2010

Primer is a simple, dynamically typed programming language with lexical scope, immutable variables and first class functions. The latest snapshot can be pulled from [github](http://github.com/parmitage/primer). If you have any comments about Primer then please [email](mailto:philip.armitage@gmail.com) me.

A definition consists of an expression bound to a symbol. Bindings are immutable.

    sides: 8
    pi: 3.14159
    initial: 'F'
    happy: true

The type of a value can be tested with the __is__ operator.

    123.45 is string                        # false
    pi is float                             # true

The __as__ operator converts between types.

    123.45 as string                        # "123.45"
    "123.45" as float                       # 123.45

__Show__ prints a value. It returns its argument so can be composed.

    Show(AreaOfCircle(Show(x)))

The body of a function is a single expression, the value of which is the return value.

    AreaOfCircle: fn (r) pi * r * r end
    AreaOfCircle(10)

The __where__ clause introduces local definitions.

    MyFun: fn (x, y) a + b(y)
       where a: 125
             b: fn (n) n * a end
    end

Primer supports closures.

    MakeAdder: fn (y)
       fn (a) y + a end
    end

    Add1: MakeAdder(1)
    Add2: MakeAdder(2)

    Add1(2)                                 # 3
    Add2(2)                                 # 4

Primer supports higher order functions.

    SimpleOpSquared: fn (f, x)
       f(x) * f(x)
    end

    SimpleOpSquared(Add1, 5)                # 36

Primer supports annonymous functions.

    SimpleOpSquared(fn (x) x / 3 end, 12)   # 16

Lists can be nested and are heterogeneous.

    xs1: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    xs2: [4, 5.32, pi, true, AreaOfCircle, AreaOfCircle(5), 'a', "aaa"]
    xs3: [4, [5.32, [pi], [], true], AreaOfCircle, 'a', "aaa"]

Several functions provide access to list elements.
 
    Head(xs1)                               # 0
    Tail(xs1)                               # [1, 2, 3, 4, 5, 6, 7, 8, 9]
    Last(xs1)                               # 9
    Length(xs1)                             # 10

A list can by accessed by index with the __at__ operator.

    xs1 at 4                                # 4

An item can be prepended to a list with the __cons__ operator.

    1 :: [2,3,4]                            # [1,2,3,4]
    1 :: 2 :: []                            # [1,2]

To concatenate two lists use the __append__ operator.

    [1,2,3] ++ [4,5,6]                      # [1,2,3,4,5,6]

Strings are just lists of characters.

    "hello" == ['h', 'e', 'l', 'l', 'o']    # true
    Head("hello")                           # 'h'
    "hello" ! 3                             # 'l'

The __if__ statement is an expression so the __else__ branch is mandatory.

    Count: fn (xs)
       if xs != []
       then 1 + Count(Tail(xs))
       else 0
    end

Tail-recursive functions are optimised as in this accumulator version of Count.

    Count: fn (xs) Counter(0, xs)
       where Counter: fn (a, xs)
                if xs != [] then Counter(a + 1, Tail(xs))
                else a
             end
    end

Primer has a modest standard library which can be found in __Library.pri__.

__Map__ applies a function to every element in a list and returns a new list containing the transformed values.

    Map(fn (x) 2 * x end, [1,2,3,4,5])      # ==> [2,4,6,8,10]

__FoldL__ and __FoldR__ uses a two parameter function to combine successive list elements.

    Add = fn (x, y) x + y end
    FoldL(Add, 0, [1,2,3,4,5])              # ==> ((((0+1)+2)+3)+4)+5 = 15
    FoldR(Add, 0, [1,2,3,4,5])              # ==> 1+(2+(3+(4+(5+0)))) = 15

__Filter__ applies a predicate function to each element of a list returning those elements for which the function returns true.

    Filter(Odd, [1,2,3,2,4,5,6])            # [1,3,5]

__Zip__ takes two lists and combines them pairwise to return a new list of 2-tuples.

    Zip(l1, [4,5,6])                        # [[1,4],[2,5],[3,6]]

__Reverse__ returns a new list in reverse order.

    Reverse("hello")                        # "olleh"

__Intersperse__ takes an atom and a list and returns a new list with the atom interspersed between the original list elements.

    Intersperse('A', l1)                    # [1,'A',2,'A',3]

__Take__ returns the first n items from a list.

    Take(2, l1)                             # [1,2]

__Drop__ returns a list without its first n items.

    Drop(1, l1)                             # [2,3]

__TakeWhile__ evaluates each item in a list in turn using a supplied function and returns the items from the list until the function returns false.

    TakeWhile(Even, l1)                     # []

__DropWhile__ evaluates each item in a list in turn using a supplied function and drops the items from the list until the function returns false.

    DropWhile(Odd, l1)                      # [2,3]

__Any__ returns true if any items in a list passes the supplied predicate function otherwise it returns false.

    Any(Even, [1,2,3,4])                    # true

__All__ returns true if all items in a list passes the supplied predicate function otherwise it returns false.

    All(Odd, [3,4,5,6])                     # false

__Min__ returns the smallest item in a list. It requires that the list contains items which can be compared.

    Min([7,2,4,5,3,8,6])                    # 2

__Max__ returns the largest item in a list. It requires that the list contains items which can be compared.

    Max([7,2,4,5,3,8,6])                    # 8

__Sum__ returns the sum of the items in the list. It requires that the items in the list are numeric.

    Sum([1,2,3,4,5])                        # 15

__Product__ returns the product of the items in the list. It requires that the items in the list are numeric.

    Product([1,2,3,4,5])                    # 120

__Sort__ is an implementation of the Quicksort algorithm.

    Sort([4,2,8,1])                         # [1,2,4,8]