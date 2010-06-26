W = 0 S = 1 E = 2 N = 3
dirs = ['W', 'S', 'E', 'N']

position = fn (x, y, h) [x, y, h] end
print = fn (r) show([r:0, r:1, dirs:(r:2)]) end

move = fn (r, s)
    reduce(transform, r, s)
    where transform = fn (r, c)
              if c == 'L' then [r:0, r:1, (r:2 + 1) mod 4]
              else if c == 'R' then [r:0, r:1, rotr(r:2) - 1]
              else translate(r, c - '0')
          end
          rotr = fn (d) if d == 0 then 4 else d end
          translate = fn (r, c)
              if even(r:2) then [r:0 + ((r:2 - 1) * c), r:1, r:2]
              else [r:0, r:1 + ((r:2 - 2) * c), r:2]
          end
end

print(move(position(10, 10, N), "R1R3L2L1"))