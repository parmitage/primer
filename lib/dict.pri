def add = fn (d, key, val)
  cons(d, [key, val])
end

def get = fn (d, key)
  def v = findByFn(key, fn (x) nth(x, 0) end, d)
  if v then
    head(tail(v))
  else
    nil
  end
end

def update = fn (d, key, val)
  if head(d) then
    if head(head(d)) == key then
      add(tail(d), key, val)
    else
      [head(d)] ++ update(tail(d), key, val)
    end
  else
    nil
  end
end

def remove = fn (d, key)
  if head(d) then
    if head(head(d)) == key then
      tail(d)
    else
      [head(d)] ++ remove(tail(d), key)
    end
  else
    nil
  end
end