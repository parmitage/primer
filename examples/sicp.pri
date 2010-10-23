gcd = fn (a, b)
   if b == 0 then a
   else gcd(b, a mod b)
end

makeRat = fn (n, d)
   (n / g) :: (d / g) :: []
   where g = gcd(n, d)
end

numer = fn (x) x ! 0 end
denom = fn (x) x ! 1 end

printRat = fn (x)
   show(format("%i/%i", numer(x), denom(x)))
end

addRat = fn (x, y)
   makeRat(n, d)
   where n = (numer(x) * denom(y)) + (numer(y) * denom(x))
         d = denom(x) * denom(y)
end

mulRat = fn (x, y)
   makeRat(n, d)
   where n = numer(x) * numer(y)
         d = denom(x) * denom(y)
end

oneHalf = makeRat(1, 2)
oneThird = makeRat(1, 3)

printRat(addRat(oneHalf, oneThird))
printRat(mulRat(oneHalf, oneThird))
printRat(addRat(oneThird, oneThird))
