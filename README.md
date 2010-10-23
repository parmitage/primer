% Primer
% Phil Armitage
% 2010

Primer is a simple functional programming language. The latest snapshot of the interpreter can be pulled from [github](http://github.com/parmitage/primer). If you have any comments then please [email](mailto:philip.armitage@gmail.com) me.

A definition consists of an expression bound to a symbol. The primitive types are int, float, char and bool. Primer is dynamically typed.

    large: 123456
    pi: 3.14159
    letter: 'f'
    happy: true

Functions are first class so can also be bound to symbols. The body of a function is a single expression, the value of which is the return value of the function.

    AreaOfCircle: fn (r) pi * r * r end
    AreaOfCircle(10)

__Show__ prints a value to stdout. It also returns its argument meaning it can be composed which is useful during debugging.

    Show(AreaOfCircle(Show(x)))

Primer is lexically scoped and bindings can be shadowed. The __where__ clause introduces definitions that are local to a function.

    MyFun: fn (x, y) a + b(y)
       where a: 125
             b: fn (n) n * a end
    end

Immutable closures are supported.

    MakeAdder: fn (y)
       fn (a) y + a end
    end

    Add1: MakeAdder(1)
    Add2: MakeAdder(2)

    Add1(2)   # 3
    Add2(2)   # 4

Lists can be nested to any level and are heterogeneous.

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

An item can be prepended to a list with the __::__ operator (pronounced "cons"). The first argument to cons will be treated as an atom.

    1 :: [2,3,4]                            # [1,2,3,4]
    1 :: 2 :: []                            # [1,2]

To append two lists use the __++__ operator.

    [1,2,3] ++ [4,5,6]                      # [1,2,3,4,5,6]

Strings are lists of characters meaning that most list functions and operators can be used with them.

    "hello" == ['h', 'e', 'l', 'l', 'o']    # true
    Head("hello")                           # 'h'
    "hello" ! 3                             # 'l'

The __if__ statement is an expression so must return a value. As a result, the else branch is mandatory.

    Count: fn (xs)
       if xs != [] then 1 + Count(Tail(xs))
       else 0
    end

Primer will eliminate tail calls for tail-recursive functions as in this version of count which uses a local accumulator function.

    Count: fn (xs) Counter(0, xs)
       where Counter: fn (a, xs)
                if xs != [] then Counter(a + 1, Tail(xs))
                else a
             end
    end

Primer supports higher order and anonymous functions.

    SimpleOpSquared: fn (f, x)
       f(x) * f(x)
    end

    SimpleOpSquared(Add1, 5)                # 36
    SimpleOpSquared(fn (x) x / 3 end, 12)   # 16

The Primer standard library, found in __Library.pri__, defines several useful functions.

    Map(Double, [1,2,3,4,5])                # [2,4,6,8,10]
    FoldL(Add, 0, [1,2,3,4,5])              # ((((0+1)+2)+3)+4)+5 -> 15
    FoldR(Add, 0, [1,2,3,4,5])              # 1+(2+(3+(4+(5+0)))) -> 15
    Filter(odd, [1,2,3,2,4,5,6])            # [1,3,5]
    Zip(l1, [4,5,6])                        # [[1,4],[2,5],[3,6]]
    Reverse("hello")                        # "olleh"
    Intersperse('A', l1)                    # [1,'A',2,'A',3]
    Take(2, l1)                             # [1,2]
    Drop(1, l1)                             # [2,3]
    TakeWhile(Even, l1)                     # []
    DropWhile(Odd, l1)                      # [2,3]
    Any(Even, [1,2,3,4])                    # true
    All(Odd, [3,4,5,6])                     # false
    Min([7,2,4,5,3,8,6])                    # 2
    Max([7,2,4,5,3,8,6])                    # 8
    Sum([1,2,3,4,5])                        # 15
    Product([1,2,3,4,5])                    # 120
    Sort([4,2,8,1])                         # [1,2,4,8]

The __as__ operator converts between types.

    Show(123.45 as string)                  # "123.45"
    Show("123.45" as float)                 # 123.45