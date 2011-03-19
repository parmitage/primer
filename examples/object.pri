##############################################################################
###
### A port of Oleg's "A trivial delegation-based OO system"
### (http://okmij.org/ftp/Scheme/oop-in-fp.txt) from Scheme to Primer.
###
### Note that without symbols we have to dispatch on strings and that
### we fake optional parameters by performing no arity checking.
###
### Also note that upper case function names have been used to represent
### 'constructors'.
###
##############################################################################

val Point2D = fun x y ->
    let Transform = fun dx dy -> Point2D(x + dx, y + dy) in
        fun selector ->
            switch selector
              case "GetX"      then x
              case "GetY"      then y
              case "Transform" then Transform(arg1, arg2)
              case _           then "message not supported";

val Point3D = fun x y z ->
    let Transform = fun dx dy dz -> Point3D(x + dx, y + dy, z + dz) in
    let Parent = Point2D(x, y) in
        fun selector arg1 arg2 arg3 ->
            switch selector
              case "GetZ"      then z
              case "Transform" then Transform(arg1, arg2, arg3)
              case _           then Parent(selector);

val delta2D = fun p1 p2 -> [p2("GetX") - p1("GetX"), p2("GetY") - p1("GetY")];

val delta3D = fun p1 p2 ->
   [p2("GetX") - p1("GetX"), p2("GetY") - p1("GetY"), p2("GetZ") - p1("GetZ")];

# Create a point in 3D space
val Start = Point3D(5, 12, 4);

# Transform it to create a new point
val Destination = Start("Transform", 2, 3, 6);

# Query the location of the new point
show(Destination("GetX"));
show(Destination("GetY"));
show(Destination("GetZ"));

# Call a 'free function' which operates on the 'objects'
show(delta3D(Start, Destination));