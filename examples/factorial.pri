def fac1 = fn (n)
  if n == 0 then
     1
  else
     n * fac1 (n - 1)
  end
end

def fac2 = fn (n)
  def inner = fn (i, acc)
    if i == 0 then
      acc
    else
      inner (i - 1, acc * i)
    end
  end
  inner (n, 1)
end

show(fac1(15))
show(fac2(15))