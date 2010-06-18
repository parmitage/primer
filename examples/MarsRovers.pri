heading = fn (r) last(r) end
latitude = fn (r) head(r) end
longtitude = fn (r) nth(r, 1) end
position = fn (x, y, h) [x, y, h] end

move = fn (r, s)
    reduce(transform, r, s)
    where transform = fn (r, c)
              if c == 'L' then rotate(r, 'L')
              else if c == 'R' then rotate(r, 'R')
              else translate(r, c - '0')
          end
          rotate = fn (r, c)
              if heading(r) == 'N' and c == 'L' then [latitude(r), longtitude(r), 'W']
              else if heading(r) == 'E' and c == 'L' then [latitude(r), longtitude(r), 'N']
              else if heading(r) == 'S' and c == 'L' then [latitude(r), longtitude(r), 'E']
              else if heading(r) == 'W' and c == 'L' then [latitude(r), longtitude(r), 'W']
              else if heading(r) == 'N' and c == 'R' then [latitude(r), longtitude(r), 'E']
              else if heading(r) == 'E' and c == 'R' then [latitude(r), longtitude(r), 'S']
              else if heading(r) == 'S' and c == 'R' then [latitude(r), longtitude(r), 'W']
              else if heading(r) == 'W' and c == 'R' then [latitude(r), longtitude(r), 'N']
              else "ERROR"
          end
          translate = fn (r, c)
              if heading(r) == 'N' then [latitude(r), longtitude(r) + c, 'N']
              else if heading(r) == 'S' then [latitude(r), longtitude(r) - c, 'S']
              else if heading(r) == 'E' then [latitude(r) + c, longtitude(r), 'E']
              else if heading(r) == 'W' then [latitude(r) - c, longtitude(r), 'W']
              else "ERROR"
          end
end

show(move(position(10, 10, 'N'), "R1R3L2L1"))