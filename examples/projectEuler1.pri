def pe1 = fn (limit)
  def inner = fn (x, accum)
    if x < limit then
      if x mod 3 == 0 or x mod 5 == 0 then
        inner(x + 1, accum + x)
      else
        inner(x + 1, accum)
      end
    else
      accum
    end
  end
  inner(0, 0)
end

show(pe1(1000))