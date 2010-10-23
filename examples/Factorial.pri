# This version will overflow stack on larger values.
Fac1: fn (n)
   if n == 0 then 1
   else n * Fac1 (n - 1)
end

Show(Fac1(10))

# This version uses an accumulator.
Fac2: fn (n)
   Inner (n, 1)
   where Inner: fn (i, acc)
            if i == 0 then acc
            else Inner (i - 1, acc * i)
         end
end

Show(Fac2(10))