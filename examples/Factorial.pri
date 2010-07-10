# This version will overflow stack on larger values.
factorial = fn (n)
   if n == 0 then 1
   else n * factorial (n - 1)
end

show(factorial(10))

# This version uses an accumulator.
factorial = fn (n)
   inner (n, 1)
   where inner = fn (i, acc)
            if i == 0 then acc
            else inner (i - 1, acc * i)
         end
end

show(factorial(10))