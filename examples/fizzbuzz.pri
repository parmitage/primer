pragma GC_DISABLE;
using base;

# The classic fizzbuzz program implemented three different ways

# 1. using nested if statements

map(fun x -> if x mod 3 == 0 and x mod 5 == 0 then show("fizzbuzz")
             else if x mod 3 == 0 then show("fizz")
             else if x mod 5 == 0 then show("buzz")
             else show(x), 1..100);

# 2. abusing the match statement

map(fun x -> match true
             with x mod 3 == 0 and x mod 5 == 0 then show("fizzbuzz")
             with x mod 3 == 0 then show("fizz")
             with x mod 5 == 0 then show("buzz")
             with _ then show(x), 1..100);

# 3. slightly more conventional (though inefficient) use of match

map(fun x -> match x mod 3, x mod 5
             with 0, 0 then show("fizzbuzz")
             with 0, _ then show("fizz")
             with _, 0 then show("buzz")
             with _, _ then show(x), 1..100);