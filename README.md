An introduction to Functional Programming with Primer
=====================================================
Primer is a functional programming language that can be learnt in an afternoon.
The knowledge you'll gain is transferable to other functional languages such as
Haskell, Clojure or Erlang.

Getting Started
---------------
The Primer interpreter is still under development and lacks many important
features, including GC. Until the interpreter reaches a stable state you
must build it from source. The code is available from GitHub:

     git clone http://github.com/parmitage/primer.git

Primer is tested on Linux, MacOS X and Windows. You need to have GNU make,
gcc, flex and bison installed. Primer requires that the environment variable
`PRIMER_LIBRARY_PATH` points to the path containing the file Library.pri, e.g.
  
    export PRIMER_LIBRARY_PATH=/path/to/primer/

What is functional programming?
-------------------------------
Functional programming is a way of writing programs without relying
on the mutation of state. The result is that a functional
program can be thought of as a single transformation from some input
to some output rather than as a sequence of instructions with
intermediate state.

John Hughes argued that trying to sell functional programming by
saying what it couldn't do was not very appealing. However I believe
that removing mutable bindings is a selling point!

As programs grow larger, the interactions within them become
increasingly complex and the temptation to use intermediate state
becomes overwhelming. That state then becomes the conduit through
which functions interact - some reading, some writing and some doing
both. Inevitably bugs will surface and very often they arise because
the programmer has lost track of where and when these updates occur.

In functional programming, we write functions that act upon their
inputs and return values but which don't alter any state. As a result,
they tend to be simpler and more reusable so the programs that we
create with them are shorter and easier to understand.

Conventions used in this tutorial
---------------------------------
Code can be taken from this tutorial and pasted into a text file
without modification. Some expressions may depend upon
definitions which were previously introduced but for brevity I won't
repeat them.

Comments are introduced with the # character and extend to the end of
the line. I will sometimes place comments above or to the side of an
expression with ==> to signify the output of the expression but this
has no special meaning in Primer.

    2 + 3    # ==> 5

Definitions
-----------
A program is composed of a series of definitions. Each definition
consists of an expression bound to a symbol which yields a
value when evaluated.

    # definition of a variable
    pi = 3.14159
    
    # definition of a function
    areaOfCircle = fn (r) pi * r * r end
    
    # application of a function
    areaOfCircle(10)

A function is an expression and doesn't have a name. If
you need to refer to a function in your program then you bind it to a
symbol as we do above. The body of a function is also an expression
and the value of that expression is the return value of the
function. Because functions may only contain one expression and that
expression must yield a value, there's no need for a return statement
in Primer.

To see the output from a program you must use __show__ which is a built-in
function capable of printing any Primer expression. 

    show(areaOfCircle(5))

In order to avoid cluttering up the text I'll omit future calls to show but you
can always add them if you want to see the result of an
evaluation. Because show is an expression and returns its argument, you
can easily output debugging information in the middle of your expressions.

    show(areaOfCircle(show(x)))

Parameters are evaluated from right to left.

Lists and Strings
-----------------
Lists are the bread and butter data structure of functional
programming. They can be nested and in Primer are heterogeneous.

    xs1 = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    xs2 = [4, 5.32, pi, true, areaOfCircle, areaOfCircle(5), 'a', "aaa"]
    xs3 = [4, [5.32, [pi], [], true], areaOfCircle, 'a', "aaa"]

Several functions provide access to list elements.
 
    head(xs1)    # ==> 0
    last(xs1)    # ==> 9
    tail(xs1)    # ==> [1, 2, 3, 4, 5, 6, 7, 8, 9]
    length(xs1)  # ==> 10

You can access a list by index with the ! operator. Note that lists
are indexed from 0.

    a = 0
    xs1!4        # ==> 4
    xs1!a        # ==> 0

Lists can be constructed programatically with the ++ operator.

    a ++ [a, [b, c], a] ++ 'c' ++ [4, 5, 6]

The Primer standard library (found in Library.pri) provides several
functions for working with lists such as __search__, __sort__ and __reverse__.

Primer provides a simple character type and strings are nothing
more than lists of characters with syntactic sugar for creating
them. This means that operators designed for use with lists can often
be used with strings.

    "hello" == ['h', 'e', 'l', 'l', 'o']    # ==> true
    head("hello")                           # ==> 'h'
    "hello"!3                               # ==> 'l'

Recursion
---------
Because we can't mutate a variable, it's not possible to use iteration
in a functional programming language. Instead we use recursive
functions as in this example which counts the number of elements in a list.

    count = fn (xs)
       if xs != [] then 1 + count(tail(xs))
       else 0
    end

As the __if__ statement is an expression, it must return a value which
means that it must always have an __else__ branch. Count recursively calls
itself with the tail of the list, adding one to the result until the
end of the list is reached at which point the recursion terminates and
zero is returned.

The expression inside a function isn't evaluated until the function
is applied, so a definition can refer to a symbol which hasn't yet
been defined. This allows us to define mutually recursive functions.

    mr0 = fn (x) if x < 5 then mr1(x) else "finished!" end
    mr1 = fn (x) mr0(x + 1) end
    mr1(0)

Lexical Scope
-------------
Because functions may only contain one expression, programs tend to be
made up of many small functions. While this is good for modularity and
code reuse, there are times when a function is only needed in one
place. The __where__ clause introduces local definitions.

    f = fn (x, y) a + b(y)
        where a = 125
              b = fn (n) n * a end
    end

The symbols a and b are scoped within f - they can refer to each
other, to the parameters of f and to definitions in the global scope
but they can not be accessed from outside of f.

Primer is said to be lexically scoped which means that a symbol is
bound at a given point if and only if that point is textually enclosed
by a block in which the symbol is defined, or if the symbol is
globally defined.

A binding may temporarily 'overwrite' a more global binding. This is
called shadowing.

    a = 1000
    
    b = fn (x) x * a end
    
    c = fn (x, y) x + a(y)
       where a = fn (x) x * 2 end
    end

Here a is defined to be the integer value 1000 at global scope. The
function b references this value. The function c however has a local
binding of a which shadows the global binding. Notice also that within
the local function a, there's a second instance of shadowing: its
parameter x shadows c's parameter x.

The basic rule is that a function will use the most local definition of a
symbol that it can find and will search outwards to successively wider scopes in
order to resolve a symbol. If no symbol is found even at the global scope then
the symbol is said to be unbound and an error is raised.

Higher Order Functions
----------------------
Because functions are values they can be passed around like any other
value. More precisely, an expression of the form

    fn (...) ... end

is referred to as a function expression (or sometimes a lambda expression).

A function which accepts another function as an argument is called
a higher order function. Let's assume that we want to
find the first number in a list which is evenly divisible by 3.

    xs = 10..30   # the range operator creates a list of integers

    findNumberDivisibleBy3 = fn (xs)
        if xs != [] then
           if head(xs) mod 3 == 0 then head(xs)
           else findNumberDivisibleBy3(tail(xs))
        else []
    end

    show(findNumberDivisibleBy3(xs))

This works but is rather limited. We could parameterise the
function to search for any value of n but we can do better! Looking at
the function we can see that there are two aspects to
it: search and test. By turning search into a higher
order function we can make it more general.

    findByFun = fn (xs, f)
        if xs != [] then
           if f(head(xs)) then head(xs)
           else findByFun(tail(xs), f)
        else false
    end
    
    # is a number divisible by 3
    factor3 = fn (x) x mod 3 == 0 end
    findByFun(xs, factor3)

Admittedly this is more code but we can now write different test
functions to search our list.

    # is a number divisible by 3 and 5
    factor3and5 = fn (x) x mod 3 == 0 and x mod 5 == 0 end
    findByFun(xs, factor3and5)
    
    # is a number odd
    odd = fn (x) x mod 2 != 0 end
    findByFun(xs, odd)

Not only is our search function reusable with different tests but our
tests can be reused with different search functions. For example,
rather than just finding the first value in a list which passes a
test, we can write a search function which returns all of the values
which pass.

    findAllByFun = fn (f, xs)
        if xs != [] then
           if f(head(xs)) then [head(xs)] ++ findAllByFun(f, tail(xs))
           else findAllByFun(f, tail(xs))
        else []
    end
    
    findAllByFun(factor3, xs)
    findAllByFun(factor3and5, xs)
    findAllByFun(odd, xs)

We can even write a test function which uses our new search function.

    # is a number prime
    prime = fn (p) p > 1 and length(findAllByFun(fn (n) p mod n == 0 end, 1..p)) == 2 end

And then we can use that with either of our search functions.

    findByFun(prime, xs)
    findAllByFun(prime, xs)

Anonymous functions
-------------------
We can use a function without previously assigning it to a
symbol. These anonymous functions are useful when you don't want to
clutter up your program with single-use functions. 

    findByFun(fn (x) x mod 2 != 0 end, xs)

Pattern Matching
----------------
Many functional programming languages allow you to bind symbols to
data using patterns. The support for pattern matching
varies greatly from language to language and Primer supports only
list de-structuring. Here's an example of the
standard head and tail functions which use pattern matching.

    head = fn (x:xs) x end
    tail = fn (x:xs) xs end

Both functions take only a single parameter, which must be a list. The
list is automatically de-structured into the head, which is bound to
x, and the tail, which is bound to xs, when the function is applied.

You can match as far into a list as you need. Each part of the
pattern matches one element in the list except for the last which
matches the rest.

    third = fn (x:y:z:xs) z end

If you're only interested in part of the de-structured list, you can
use a wildcard which binds against any value but doesn't create a
binding.

    head = fn (x:_) x end
    tail = fn (_:xs) xs end

De-structuring is a useful way of simplifying your function definitions.

Mapping, folding and filtering
------------------------------
These three operations are so fundamental that all functional
languages provide an implementation of them. Map applies a function to
every element in a list and returns a new list containing the
transformed values.

    double = fn (x) 2 * x end
    map(double, [1,2,3,4,5])      # ==> [2,4,6,8,10]

The implementation of map in the standard library is quite straightforward.

    map = fn (f, x:xs)
       if x != [] then f(x) ++ map(f, xs)
       else []
    end

Fold, sometimes known as reduce, uses a two parameter
function to combine successive list elements. In the following
example, we sum the elements of the list.

    add = fn (x, y) x + y end
    foldl(add, 0, [1,2,3,4,5])   # ==> ((((0+1)+2)+3)+4)+5 = 15
    foldr(add, 0, [1,2,3,4,5])   # ==> 1+(2+(3+(4+(5+0)))) = 15

Note that there are actually two fold functions.
A left fold (foldl) recursively combines the results of
combining all but the last element from a list with the last one. The
initial value, 0 in the example above, is combined with the first
element in the list. A right fold (foldr) on the other hand
recursively combines the first element with the results of combining
the rest. The initial value is therefore combined with the last
element in the list.

Again, the implementations of these two functions should be fairly
self-explanatory.

    foldl = fn (f, init, x:xs)
       if x != [] then foldl(f, f(init, x), xs)
       else init
    end

    foldr = fn (f, init, x:xs)
       if x != [] then f(x, foldr(f, init, xs))
       else init
    end

Filter returns a new list which contains only the elements which pass
a predicate function.

    filter(odd, [1,2,3,2,4,5,6])  # ==> [1,3,5]

The implementation of filter should look familiar to you from earlier!

    filter = fn (f, y:ys)
       if y != [] then
          if f(y) then [y] ++ filter(f, ys)
          else filter(f, ys)
       else []
    end

There are several other higher order functions supplied with Primer
such as zip, all, any, takeWhile and dropWhile. You can look at the
implementation of these by reading through the standard library code.

Closures
--------
A closure is a first-class function which can access the variables in
its lexical environment for as long as it lives. The function is said
to "close" over its lexical environment, retaining it for future
use. In the following example, the return value of the makeAdder
function is another function.

    makeAdder = fn (y)
       fn (a) y + a end
    end

The function returned performs a simple addition of
two variables. One of those variables is a parameter to the function
but the other is a parameter to makeAdder itself. So the returned
function is able to access a variable outside of its scope. This in
itself shouldn't be surprising as we've already seen lexical scope.
What is more interesting is that this function still works outside
the scope of makeAdder.

We can think of makeAdder as a function constructor. Here we use
it to make two new functions; one function which will add 1 to
a value and another which will add 2 to a value.

    add1 = makeAdder(1)
    add2 = makeAdder(2)

The symbols add1 and add2 now refer to functions which we can
call in exactly the same way as if we'd defined them
directly. When makeAdder returned a function, that function
captured the binding of the variable y. In the case of add1,
y was bound to 1 and in the case of add2, it was bound to 2.

    add1(2)   # ==> 3
    add2(2)   # ==> 4

Tail Recursion
--------------
Earlier we wrote a function to count the number of elements in a list.

    count = fn (xs)
        if xs != [] then 1 + count(tail(xs))
        else 0
    end

However, this function is not efficient and will crash on a
sufficiently long list. In most programming languages, every time a
function is called, information is pushed onto the stack including
parameter information and where it should return its value to. The
stack is usually of a fixed size meaning that unbounded recursion can
lead to a stack overflow.

Most functional programming languages require that their runtimes
eliminate some of the recursive calls in a users program, turning them
into jumps that do not consume stack space. Typically this can only be done if the
recursive call is the last thing that the function does. In practice,
tail calls are common so this tail call
optimisation is beneficial. However, not all functions are
naturally tail recursive and our count function is one which needs to
be adjusted to take advantage of the optimisation.

The problem with count is that the last thing it does is not the
recursive call but rather a call to the addition operator. We can
rewrite count to use an accumulator function.

    count = fn (xs) counter(0, xs)
       where counter = fn (a, xs)
                if xs != [] then counter(a + 1, tail(xs))
                else a
             end
    end

Accumulators are a common technique used to make this type of summing
function tail recursive by moving the summing operation away from the
tail call.

Problem: Mars Rovers
--------------------
The following is a slightly modified version of a problem which
ThoughtWorks give to interview candidates.

A robotic rover is to be landed by NASA on a rectangular plateau of
Mars. The rover must navigate the plateau using a set of commands sent
from Earth. The plateau is divided into a grid to simplify navigation.

A rover's position is represented by a combination of an x and y
co-ordinate and a letter representing one of the four cardinal compass
points. An example position on the grid might be [0,0,N] which means
the rover is in the bottom left corner and facing North. Assume that
the square directly North from (x, y) is (x, y+1).

The input to the rover is via two functions:

1. position(x, y, h) deploys the rover to an initial grid location [x,y,h].

2. move(s) accepts a command string of the form "L1R2". L spins the
rover 90 degrees left, R spins the rover 90 degrees right and a number
moves the rover by the appropriate number of grid points in the
direction it's currently facing. The command string can be of any
reasonable length.

Test your program by starting in an initial position [10,10,N] and
then issuing the command "R1R3L2L1". The output from the program should
be the final grid location and heading of the rover.

I encourage you to write your own solution but here's my attempt.

    W = 0 S = 1 E = 2 N = 3
    dirs = ['W', 'S', 'E', 'N']
    
    position = fn (x, y, h) [x, y, h] end
    print = fn (x:y:h:_) show([x, y, dirs!h]) end
    
    move = fn (r, s)
        foldl(transform, r, s)
        where transform = fn (x:y:h:_, c)
                  if c == 'L' then [x, y, (h + 1) mod 4]
                  else if c == 'R' then [x, y, rotr(h) - 1]
                  else translate(x, y, h, c - '0')
              end
              rotr = fn (h) if h == 0 then 4 else h end
              translate = fn (x, y, h, c)
                  if even(h) then [x + ((h - 1) * c), y, h]
                  else [x, y + ((h - 2) * c), h]
              end
    end
    
    print(move(position(10, 10, N), "R1R3L2L1"))

The examples directory contains several other simple programs.