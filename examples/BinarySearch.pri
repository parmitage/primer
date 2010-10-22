binarySearch = fn (xs, x)
   inner(xs, x, 0, length(xs) - 1)
   where inner = fn (xs, x, s, e)
            if s <= e then
               if x == val then idx
               else if x < val then inner(xs, x, s, e - 1)
               else inner(xs, x, s + 1, e)
            else false
            where diff = e - s
                  mid = diff / 2
                  idx = s + mid
                  val = xs at idx
         end         
end

listToSearch = 1..1000

show(binarySearch(listToSearch, 500))
show(binarySearch(listToSearch, 1))
show(binarySearch(listToSearch, 999))
show(binarySearch(listToSearch, 1001))
show(binarySearch(listToSearch, -1))