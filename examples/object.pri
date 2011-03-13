##############################################################################
###
### A port of Oleg's "A trivial delegation-based OO system"
### (http://okmij.org/ftp/Scheme/oop-in-fp.txt) from Scheme to primer.
###
### Note that without quoted symbols we have to dispatch on strings and that
### we fake optional parameters by relying on no arity checking.
###
### Note that upper case function names have been used to represent
### constructors.
###
##############################################################################

val Point2D = fun x y ->
    let Transform = fun dx dy -> Point2D(x + dx, y + dy) in
        fun selector ->
            if selector == "GetX" then x
            else if selector == "GetY" then y
            else if selector == "Transform" then Transform(arg1, arg2)
            else "message not supported";

val Point3D = fun x y z ->
    let Transform = fun dx dy dz -> Point3D(x + dx, y + dy, z + dz) in
    let Parent = Point2D(x, y) in
        fun selector arg1 arg2 arg3 ->
            if selector == "GetZ" then z
            else if selector == "Transform" then Transform(arg1, arg2, arg3)
            else Parent(selector);

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