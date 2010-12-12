% An introduction to Functional Programming using Primer and Genetic Algorithms
% Phil Armitage
% 2010

Primer is a functional programming language that you can learn in an afternoon. In this tutorial we'll learn functional programming by writing a simple genetic algorithm in Primer. Along the way we'll discover how functional programs differs from imperative programs as well as the techniques that functional programmers might use to solve problems. At the end of the tutorial you'll be ready to move on to more powerful functional languages like Haskell, Erlang or Clojure.

If you have any comments about the Primer language, the interpreter or this tutorial then please feel free to [email](mailto:philip.armitage@gmail.com) me.

Getting Started
---------------
Primer is currently only available from [github](http://github.com/parmitage/primer). The language design is now complete but the interpreter still lacks some features. It does mostly work though! Once the interpreter is complete, I'll produce binaries for Linux, Mac and Windows but for now I assume that you have git, gcc, make, flex and bison installed on your machine and that you're using bash.

Run the following commands to download, build and and test that Primer is working correctly:

1. git clone git@github.com:parmitage/primer.git
2. cd primer
2. make
3. export PRIMER_LIBRARY_PATH=/path/to/primer/
4. echo Show\(\"Hello, World\"\) > Hello.pri
5. ./primer Hello.pri

There are lots of rather formal definitions of functional programming in books and papers but they tend to focus on what you can't do in a functional language which makes the whole endeavour sound rather unappealing (for a great paper which acknowledges this problem I recommend reading Why Functional Programming Matters by John Hughes). So instead of doing that, let's dive straight into our afternoon of programming.

The Knapsack problem
--------------------
The knapsack problem is an example of combinatorial optimization. Given a rucksack capable of carrying a certain weight and a set of items, we must determine the optimal combination of items to pack for our holiday. Each item has a weight and an importance and we should choose the most useful set of items which don't exceed the capacity of the rucksack.

Let's start by setting up some initial parameters for our program. Open up your editor and enter the following two definitions.

    WeightLimit: 7

    Items: [[ "Book", 2, 0 ],    # Name, Weight, Importance
            [ "Camera", 2, 1 ],
            [ "Tent", 3, 6 ],
            [ "Map", 1, 4 ],
            [ "Food", 1, 6 ],
            [ "Drink", 1, 6 ],
            [ "Clothes", 2, 5 ],
            [ "Torch", 2, 1 ],
            [ "Soap", 1, 2 ],
            [ "Guitar", 3, 0 ],
            [ "Football", 2, 0 ],
            [ "Toothbrush", 1, 2 ]]

A definition is just a symbol bound to an expression. In this case we create a definition for the capacity of our knapsack and a list of the items we'd like to pack. In Primer, lists are the only composite data structure so we use them for everything. If we were using Haskell for example, we might model the Items as a list of 3-tuples or perhaps as an algebraic data type.

Because lists are heterogeneous in Primer I've put a comment next to the definition of Items to make it clear what each element represents.

Genetic Algorithms
------------------
Optimization problems are such that there exists some optimal solution for a given set of inputs. In our case there is a combination of items that we can pack for our holiday which will be most useful to us without breaking our knapsack.

There are many optimization techniques that we could use. We could for example use simple brute force search in which we try every possible combination of items until we find the best. For our program we're going to use a __genetic algorithm__ (GA) which is one of a number of techniques described as __evolutionary algorithms__. These are a class of algorithm which are inspired by the processes of nature. As we write our program we'll create represntations for chromosomes, generations, reproduction and mutation.

Chromosomes
-----------
For our purposes, a chromosome is a set of genes. These genes represent the properties of our chromosome. What we'd like to do is to create a chromosome which represents one combination of choices for the items to pack into our knapsack. We could use a list of boolean values to do this, for example:

    # Book, Camera, Map, Drink, Football, Toothbrush
    [true, true, false, true, false, true, false, false, false, false, true, true]

This is a perfectly reasonable representation except that it's a bit inefficient. Because we'll want to have many chromosomes and many generations of them, the memory consumption will become quite significant as will the time spent operating on the list.

Because we have only 12 possible items, we can store a chromosome as a single integer and use its bitwise representation to act as genes. For example, the chromosome above could be represented as the following 12-bit number:

    # 1100 0010 1011
    3115

With this fresh in our minds we'll write our first function which will create random chromosomes for us.

    ChromosomeNew: fn () Rnd(4095) end

Functions in Primer are said to be first class. That is they can be stored in lists, passed around and bound to symbols. Here you can see that the function itself has no name, it is said to be __anonymous__, but it's bound to the symbol ChromosomeNew so that we can refer to it.

A function body consists of one expression and that expression is the return value of the function - there's no need for an explicit return statement. Note that functional programs have no notion of sequential operations which is why a function can only contain one expression. Programs are constructed by functions calling one another and passing values as parameters and returning values as their results. There is no intermediate state and no mutation.

Our function takes no arguments. All it does is return the result of the built-in function Rnd which returns a random number between 0 and its argument. Here we pass in 4095 to limit the range to 12-bit numbers so as to fit our chromosome model.

As an aside, Primer breaks __referential transparency__ with its Rnd function because it (usually!) returns a different value each time that it's called. That sounds like a rather obvious statement to make but in a __pure__ language such as Haskell, random numbers are a fairly involved topic. For now we can just take advantage of the fact that Primer relaxes some of the rules in order to keep things nice and simple.

Reproduction and Mutation
-------------------------
The two operations that we want to perform on chromosomes are reproduction and mutation.

The idea of reproduction is to take the chromosomes that are considered the fitest of the generation and reproduce them with one another, hopefully producing even fitter children.

    Reproduce: fn (c1, c2)
       (c1 & 4032) | (c2 & 63)
    end

There are many ways of implementing reproduction in a genetic algorithm and this is one of the simplest possible approaches. We take two chromosomes and 'concatenate' the top 6 bits of one with the bottom 6 bits of the other. Genetically speaking this is a very nieve view of reproduction - each child has exactly half of the characteristics of each of its parents - but for the simple problem space that we have it's quite sufficient for our needs.

In a GA, reproduction will occur in every generation as it's the primary means for finding an optimal solution. However, it's entirely possible that a problem has several acceptable solutions but only one or two truly optimal solutions. An optimization algorithm can easily get get 'stuck' in one of these  __local maxima__ and never find the best global solution. Optimization algorithms typically provide mechanisms for dealing with this, usually by sprinkling in a bit of random noise to allow the routine to 'jump out' of a local maxima. In a GA this is done by mutation.

    Mutate: fn (c)
       if Rnd(10000) mod 1000 == 17
       then c ^ (1 << (Rnd(10000) mod (GeneCount - 1)))
       else c
    end

As well as a few new operators, which I'll assume you're familar with, this function introduces the if statement for the first time. The if statement in Primer is subtly different to that found in an imperative language because it's an expression meaning that it must return a value which in turn means that it must have an else branch. In our Mutate function we decide whether to flip a bit in our chromosome if a random number when divided by 1000 leaves a remainder of 17. Getting the Mutate function right for a GA is tricky because we don't want to mutate too often or too infrequently. In effect, it's one of the areas that we could tweak later on if we find our GA isn't giving good enough results.

Fitness
-------
We now need a way to evaluate how good a particular chromosome is in terms of solving our original problem. In a GA, this is what the __fitness__ function is for.

The fitness function is another aspect of a GA which can be adjusted during testing. Initially we'll use a relatively simple way of weighting the quality of a chromosome such that we'll sum up the importance and the weight of the items it represents. If the items fit in the knapsack, we'll judge the fitness to be the weight of the items multiplied by their importance. If the weight of the items exceeds the capacity of the knapsack then we return a negative fitness proportinal to its weight.

    Fitness: fn (c)
       if weight <= WeightLimit
       then importance * weight
       else weight * -5
       where items: ChromosomeItems(c)
             importance: ImportanceOfItems(items)
             weight: WeightOfItems(items)
    end

Our definition of Fitness introduces the where clause. Primer is __lexically scoped__ meaning that we can introduce local definitions in our functions. In this case we create definitions to represent values that we need in our computation but we can also introduce local function definitions that would otherwise clutter up the global scope.

Next we'll implement the ChromosomeItems function. The purpose of it is simply to return a list of the items which this chromosome represents.

    ChromosomeItems: fn (c)
       Inner(0)
       where Inner: fn (x)
                 if x < GeneCount
                 then if BitSet(c, x)
                      then (Items at x) :: Inner(x + 1)
                      else Inner(x + 1)
                 else xs
             end
    end

Because we know how many items we're trying to pack, which is equal to the length of a chromosome, we can test each bit in turn to see if its set. If it is set then that item should be included in the results. However, without mutable state, a functional language has no means of controlling loops so iteration is replaced by recursion. Recursion is absolutely fundamental to functional programming so it's important to become comfortable with it.

The recursion in this function is controlled by a counter variable, x, which we pass into our local function, Inner.

Inner function...
Cons...
at operator...
Accumulator version...

    ChromosomeItems: fn (c)
       Inner(0, [])
       where Inner: fn (x, xs)
                 if x < GeneCount
                 then if BitSet(c, x)
                      then Inner(x + 1, (Items at x) :: xs)
                      else Inner(x + 1, xs)
                 else xs
             end
    end



Recursion...

Generations
-----------
Higher order functions...
