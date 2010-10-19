# This version will overflow stack on larger values.
fac1 = fn (n)
   if n == 0 then 1
   else n * fac1 (n - 1)
end

show(fac1(10))

# This version uses an accumulator.
fac2 = fn (n)
   inner (n, 1)
   where inner = fn (i, acc)
            if i == 0 then acc
            else inner (i - 1, acc * i)
         end
end

show(fac2(10))