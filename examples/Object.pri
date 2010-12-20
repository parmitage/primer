##############################################################################
### This is "A trivial delegation-based OO system" based on the one Oleg wrote
### in Scheme (http://okmij.org/ftp/Scheme/oop-in-fp.txt).
###
### Note that without quoted symbols we have to dispatch on strings and that
### we fake optional parameters by relying on no arity checking. But it works.
##############################################################################

Point2D: fn (x, y)
   fn (selector)
      if selector == "GetX" then x
      else if selector == "GetY" then y
      else if selector == "Transform" then Transform(arg1, arg2)
      else "message not supported"
   end
   where Transform: fn (dx, dy) Point2D(x + dx, y + dy) end
end

Point3D: fn (x, y, z)
   fn (selector, arg1, arg2, arg3)
      if selector == "GetZ" then z
      else if selector == "Transform" then Transform(arg1, arg2, arg3)
      else Parent(selector)
   end
   where Transform: fn (dx, dy, dz) Point3D(x + dx, y + dy, z + dz) end
         Parent: Point2D(x, y)
end

Delta2D: fn (p1, p2)
   [p2("GetX") - p1("GetX"), p2("GetY") - p1("GetY")]
end

Delta3D: fn (p1, p2)
   [p2("GetX") - p1("GetX"), p2("GetY") - p1("GetY"), p2("GetZ") - p1("GetZ")]
end

# Create a point in 3D space
Start: Point3D(5, 12, 4)

# Transform it to create a new point
Destination: Start("Transform", 2, 3, 6)

# Query the location of the new point
Show(Destination("GetX"))
Show(Destination("GetY"))
Show(Destination("GetZ"))

# Call a 'free function' which operates on the 'objects'
Show(Delta3D(Start, Destination))