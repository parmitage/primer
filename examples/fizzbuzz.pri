map(fn (x) if x mod 3 == 0 and x mod 5 == 0 then
             show("fizzbuzz")
           else
             if x mod 3 == 0 then
               show("fizz")
             else
               if x mod 5 == 0 then
                 show("buzz")
               else
                 show(x)
               end
             end
           end
    end,
  range(1, 100))