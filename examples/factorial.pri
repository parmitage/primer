# this version will overflow stack on larger values

val fac1 = fun n ->
   if n == 0
   then 1
   else n * fac1 (n - 1);

show(fac1(10));

# this version uses an accumulator

val fac2 = fun n ->
   let inner = fun i acc ->
                  if i == 0
                  then acc
                  else inner (i - 1, acc * i)
   in inner (n, 1);

show(fac2(10));