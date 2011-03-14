##############################################################################
# A robotic rover is to be landed by NASA on a rectangular plateau of Mars.
# The rover must navigate the plateau using a set of commands sent from Earth.
# The plateau is divided into a grid to simplify navigation.
# 
# A rovers position is represented by a combination of an x and y co-ordinate
# and a letter representing one of the four cardinal compass points. An example
# position on the grid might be [0,0,N] which means the rover is in the bottom
# left corner and facing North. Assume that the square directly North from
# (x, y) is (x, y+1).
# 
# The input to the rover is via two functions:
# 
# position(x, y, h) deploys the rover to an initial grid location [x,y,h]
# 
# move(s) accepts a command string of the form "L1R2". L spins the rover 90
# degrees left, R spins the rover 90 degrees right and a number moves the rover
# by the appropriate number of grid points in the direction it is currently
# facing. The command string can be of any reasonable length.
# 
# Test your program by starting in an initial position [10,10,N] and then
# issuing the command "R1R3L2L1". The output from the program should be the
# final grid location and heading of the rover.
##############################################################################

using base;

val w = 0;
val s = 1;
val e = 2;
val n = 3;

val rover = fun x y h -> [x, y, h];
val roverX = fun r -> r at 0;
val roverY = fun r -> r at 1;
val roverH = fun r -> r at 2;

val print = fun r ->
   let x = roverX(r) in
   let y = roverY(r) in
   let d = (['W', 'S', 'E', 'N'] at roverH(r)) in
   show([x, y, d]);

val navigate = fun r s -> foldl(move, r, s);

val move = fun r c ->
   if c == 'L' or c == 'R'
   then rotate(r, c)
   else translate(r, c as int - '0' as int);

val translate = fun r c ->
   if even(roverH(r))
   then rover(roverX(r) + ((roverH(r) - 1) * c), roverY(r), roverH(r))
   else rover(roverX(r), roverY(r) + ((roverH(r) - 2) * c), roverH(r));

val rotate = fun r d ->
   if d == 'L'
   then rover(roverX(r), roverY(r), (roverH(r) + 1) mod 4)
   else rover(roverX(r), roverY(r), (if roverH(r) - 1 == 0
                                     then 4
                                     else roverH(r)) - 1);

print(navigate(rover(10, 10, n), "R1R3L2L1"));