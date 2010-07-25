pe1 = fn (limit)
   inner(0, 0)
   where inner = fn (x, accum)
            if x < limit then
               if x mod 3 == 0 or x mod 5 == 0 then
                  inner(x + 1, accum + x)
               else inner(x + 1, accum)
            else accum
         end
end

show(pe1(10000))