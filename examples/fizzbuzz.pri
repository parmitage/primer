using base;

# An implementaton of the classic Fizz buzz program.
#
# From Wikipedia:
#
# Fizz buzz is a group word game frequently encountered as a drinking game.
# Players generally sit in a circle. The player designated to go first says
# the number "1", and each player thenceforth counts one number in turn.
# However, any number divisible by three is replaced by the word fizz and
# any divisible by five by the word buzz. Numbers divisible by both become
# bizz buzz. A player who hesitates or makes a mistake is either eliminated
# or must pay a forfeit, such as taking a drink.

map(fun x -> match x mod 3, x mod 5
             with 0, 0 then show("fizzbuzz")
             with 0, _ then show("fizz")
             with _, 0 then show("buzz")
             with _, _ then show(x), 1..100);