# A robotic rover is to be landed by NASA on a rectangular plateau of Mars.
# The rover must navigate the plateau using a set of commands sent from Earth.
# The plateau is divided into a grid to simplify navigation.
#
# A rover's position is represented by a combination of an x and y co-ordinate
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
# by the appropriate number of grid points in the direction it's currently
# facing. The command string can be of any reasonable length.
#
# Test your program by starting in an initial position [10,10,N] and then
# issuing the command "R1R3L2L1". The output from the program should be the
# final grid location and heading of the rover.

W: 0 S: 1 E: 2 N: 3

Rover: fn (x, y, h) [x, y, h] end
RoverX: fn (r) r at 0 end
RoverY: fn (r) r at 1 end
RoverH: fn (r) r at 2 end

Print: fn (r)
   Show(x ++ " " ++ y ++ " " ++ d)
   where x: RoverX(r) as string
         y: RoverY(r) as string
         d: (['W', 'S', 'E', 'N'] at RoverH(r)) as string
end

Navigate: fn (r, s) FoldL(Move, r, s) end

Move: fn (r, c)
   if c == 'L' or c == 'R'
   then Rotate(r, c)
   else Translate(r, c - '0')
end

Translate: fn (r, c)
   if Even(RoverH(r))
   then Rover(RoverX(r) + ((RoverH(r) - 1) * c), RoverY(r), RoverH(r))
   else Rover(RoverX(r), RoverY(r) + ((RoverH(r) - 2) * c), RoverH(r))
end

Rotate: fn (r, d)
   if d == 'L'
   then Rover(RoverX(r), RoverY(r), (RoverH(r) + 1) mod 4)
   else Rover(RoverX(r), RoverY(r), (if RoverH(r) - 1 == 0
                                     then 4
                                     else RoverH(r)) - 1)
end

Print(Navigate(Rover(10, 10, N), "R1R3L2L1"))