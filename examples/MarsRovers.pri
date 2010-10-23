##############################################################################
### Set up compass positions.
##############################################################################
W: 0 S: 1 E: 2 N: 3
dirs: ['W', 'S', 'E', 'N']

##############################################################################
### Accessor functions to read properties of a rover.
##############################################################################
RoverX: fn (r) r at 0 end
RoverY: fn (r) r at 1 end
RoverH: fn (r) r at 2 end

##############################################################################
### Move a rover using command string.
##############################################################################
Move: fn (r, s)
   foldl(Transform, r, s)
   where Transform: fn (r, c)
            if c == 'L' then
               [RoverX(r), RoverY(r), (RoverH(r) + 1) mod 4]
            else if c == 'R' then
               [RoverX(r), RoverY(r), RotateRight(RoverH(r)) - 1]
            else
               Translate(RoverX(r), RoverY(r), RoverH(r), c - '0')
         end
         RotateRight: fn (h) if h == 0 then 4 else h end
end

##############################################################################
### Translate a set of co-ordinates specified as x, y and heading.
##############################################################################
Translate: fn (x, y, h, c)
   if even(h) then [x + ((h - 1) * c), y, h]
   else [x, y + ((h - 2) * c), h]
end

##############################################################################
### Displays a rover to the console in "x y h" notation.
##############################################################################
Print: fn (r)
   show(x ++ " " ++ y ++ " " ++ d)
   where x: RoverX(r) as string
         y: RoverY(r) as string
         d: (dirs at RoverH(r)) as string
end

##############################################################################
### Create a rover and move it using a command string.
##############################################################################
rover: [10, 10, N]
command: "R1R3L2L1"
Print(Move(rover, command))