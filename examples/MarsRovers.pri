##########################################################################
### Set up compass points.
W: 0 S: 1 E: 2 N: 3

##########################################################################
### Constructor and accessors for a rover.
Rover: fn (x, y, h) [x, y, h] end
RoverX: fn (r) r at 0 end
RoverY: fn (r) r at 1 end
RoverH: fn (r) r at 2 end

##########################################################################
### Perform each instruction in the command string.
Navigate: fn (r, s) FoldL(Move, r, s) end

##########################################################################
### Move a rover by a single instruction.
Move: fn (r, c)
   if c == 'L' then [RoverX(r), RoverY(r), (RoverH(r) + 1) mod 4]
   else if c == 'R' then [RoverX(r), RoverY(r), RotateRight(RoverH(r)) - 1]
   else Translate(r, c - '0')
   where RotateRight: fn (h) if h == 0 then 4 else h end
end

##########################################################################
### Translate a rover by a single command.
Translate: fn (r, c)
   if Even(RoverH(r)) then
      [RoverX(r) + ((RoverH(r) - 1) * c), RoverY(r), RoverH(r)]
   else
      [RoverX(r), RoverY(r) + ((RoverH(r) - 2) *c), RoverH(r)]
end

##########################################################################
### Displays a rover to the console in "x y h" notation.
Print: fn (r)
   Show(x ++ " " ++ y ++ " " ++ d)
   where x: RoverX(r) as string
         y: RoverY(r) as string
         d: (['W', 'S', 'E', 'N'] at RoverH(r)) as string
end

##########################################################################
### Create a rover and move it using a command string.
Print(Navigate(Rover(10, 10, N), "R1R3L2L1"))