W = 0 S = 1 E = 2 N = 3
dirs = ['W', 'S', 'E', 'N']

position = fn (x, y, h) [x, y, h] end
print = fn (x:y:h:_) show([x, y, dirs!h]) end

move = fn (r, s)
   foldl(transform, r, s)
   where transform = fn (x:y:h:_, c)
            if c == 'L' then [x, y, (h + 1) mod 4]
            else if c == 'R' then [x, y, rotr(h) - 1]
            else translate(x, y, h, c - '0')
         end
         rotr = fn (h) if h == 0 then 4 else h end
         translate = fn (x, y, h, c)
            if even(h) then [x + ((h - 1) * c), y, h]
            else [x, y + ((h - 2) * c), h]
         end
end

print(move(position(10, 10, N), "R1R3L2L1"))