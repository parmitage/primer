add = fn (d, key, val) d ++ [[key, val]] end

get = fn (d, key)
   if v != [] then head(tail(v))
   else false
   where v = findByFn(key, fn (x) x!0 end, d)
end

update = fn (d:ds, key, val)
   if d != [] then
      if head(d) == key then
         add(ds, key, val)
      else d : update(ds, key, val)
   else d
end

remove = fn (d:ds, key)
   if d != [] then
      if head(d) == key then ds
      else d : remove(ds, key)
   else d
end

d = []
d1 = add(d, "one", 1)
d2 = add(d1, "two", 2)
d3 = add(d2, "three", 3)

show(d3)
show(get(d3, "one"))
show(get(d3, "four"))
show(update(d3, "one", 5))
show(update(d3, "two", 5))
show(update(d3, "three", 5))
show(update(d3, "four", 5))
show(remove(d3, "one"))
show(remove(d3, "two"))
show(remove(d3, "three"))
show(remove(d3, "four"))