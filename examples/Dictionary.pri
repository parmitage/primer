Add: fn (d, key, val) d ++ [[key, val]] end

Get: fn (d, key)
   if v != [] then Head(Tail(v))
   else false
   where v: FindByFn(key, fn (x) x at 0 end, d)
end

Update: fn (ds, key, val)
   if Head(ds) != [] then
      if Head(Head(ds)) == key then
         Add(Tail(ds), key, val)
      else Head(ds) :: Update(Tail(ds), key, val)
   else Head(ds)
end

Remove: fn (ds, key)
   if Head(ds) != [] then
      if Head(Head(ds)) == key then Tail(ds)
      else Head(ds) :: Remove(Tail(ds), key)
   else Head(ds)
end

d: []
d1: Add(d, "one", 1)
d2: Add(d1, "two", 2)
d3: Add(d2, "three", 3)

Show(d3)
Show(Get(d3, "one"))
Show(Get(d3, "four"))
Show(Update(d3, "one", 5))
Show(Update(d3, "two", 5))
Show(Update(d3, "three", 5))
Show(Update(d3, "four", 5))
Show(Remove(d3, "one"))
Show(Remove(d3, "two"))
Show(Remove(d3, "three"))
Show(Remove(d3, "four"))