Primer
======
Primer is a simple functional programming language. The latest snapshot of the interpreter can be pulled from [github](http://github.com/parmitage/primer). If you have any comments then please [email](mailto:philip.armitage@gmail.com) me.

A definition consists of an expression bound to a symbol. The primitive types are int, float, char and bool. Primer is dynamically typed.

    large = 123456
    pi = 3.14159
    letter = 'f'
    happy = true

Functions are first class so can also be bound to symbols. The body of a function is a single expression, the value of which is the return value of the function.

    areaOfCircle = fn (r) pi * r * r end
    areaOfCircle(10)

The show function prints a value to stdout. It also returns its argument meaning it can be composed which is useful during debugging.

    show(areaOfCircle(show(x)))

Primer is lexically scoped and bindings can be shadowed. The where clause introduces definitions that are local to a function.

    myFun = fn (x, y) a + b(y)
       where a = 125
             b = fn (n) n * a end
    end

Immutable closures are supported.

    makeAdder = fn (y)
       fn (a) y + a end
    end

    add1 = makeAdder(1)
    add2 = makeAdder(2)

    add1(2)   # 3
    add2(2)   # 4

Lists can be nested to any level and are heterogeneous.

    xs1 = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    xs2 = [4, 5.32, pi, true, areaOfCircle, areaOfCircle(5), 'a', "aaa"]
    xs3 = [4, [5.32, [pi], [], true], areaOfCircle, 'a', "aaa"]

Several functions provide access to list elements.
 
    head(xs1)                               # 0
    tail(xs1)                               # [1, 2, 3, 4, 5, 6, 7, 8, 9]
    last(xs1)                               # 9
    length(xs1)                             # 10

A list can by accessed by index with the ! operator.

    xs1 ! 4                                 # 4

An item can be prepended to a list with the :: operator (pronounced "cons"). The first argument to cons will be treated as an atom.

    1 :: [2,3,4]                            # [1,2,3,4]
    1 :: 2 :: []                            # [1,2]

To append two lists use the ++ operator.

    [1,2,3] ++ [4,5,6]                      # [1,2,3,4,5,6]

Strings are lists of characters meaning that most list functions and operators can be used with them.

    "hello" == ['h', 'e', 'l', 'l', 'o']    # true
    head("hello")                           # 'h'
    "hello" ! 3                             # 'l'

The if statement is an expression so must return a value. As a result, the else branch is mandatory.

    count = fn (xs)
       if xs != [] then 1 + count(tail(xs))
       else 0
    end

Primer will eliminate tail calls for tail-recursive functions as in this version of count which uses a local accumulator function.

    count = fn (xs) counter(0, xs)
       where counter = fn (a, xs)
                if xs != [] then counter(a + 1, tail(xs))
                else a
             end
    end

Primer supports higher order and anonymous functions.

    simpleOpSquared = fn (f, x)
       f(x) * f(x)
    end

    simpleOpSquared(add1, 5)                # 36
    simpleOpSquared(fn (x) x / 3 end, 12)   # 16

The Primer standard library, found in Library.pri, defines several useful functions.

    map(double, [1,2,3,4,5])                # [2,4,6,8,10]
    foldl(add, 0, [1,2,3,4,5])              # ((((0+1)+2)+3)+4)+5 = 15
    foldr(add, 0, [1,2,3,4,5])              # 1+(2+(3+(4+(5+0)))) = 15
    filter(odd, [1,2,3,2,4,5,6])            # [1,3,5]
    zip(l1, [4,5,6])                        # [[1,4],[2,5],[3,6]]
    reverse("hello")                        # "olleh"
    intersperse('A', l1)                    # [1,'A',2,'A',3]
    take(2, l1)                             # [1,2]
    drop(1, l1)                             # [2,3]
    takeWhile(even, l1)                     # []
    dropWhile(odd, l1)                      # [2,3]
    any(even, [1,2,3,4])                    # true
    all(odd, [3,4,5,6])                     # false
    min([7,2,4,5,3,8,6])                    # 2
    max([7,2,4,5,3,8,6])                    # 8
    sum([1,2,3,4,5])                        # 15
    product([1,2,3,4,5])                    # 120
    sort([4,2,8,1])                         # [1,2,4,8]


