BinarySearch: fn (xs, x)
   Inner(xs, x, 0, Length(xs) - 1)
   where Inner: fn (xs, x, s, e)
            if s <= e then
               if x == val then idx
               else if x < val then Inner(xs, x, s, e - 1)
               else Inner(xs, x, s + 1, e)
            else false
            where diff: e - s
                  mid: diff / 2
                  idx: s + mid
                  val: xs at idx
         end         
end

listToSearch: 1..1000

Show(BinarySearch(listToSearch, 500))
Show(BinarySearch(listToSearch, 1))
Show(BinarySearch(listToSearch, 999))
Show(BinarySearch(listToSearch, 1001))
Show(BinarySearch(listToSearch, -1))