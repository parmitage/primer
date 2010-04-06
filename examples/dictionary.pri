def dictAdd = fn (dict, key, val)
  cons(dict, [key, val])
end

def dictGet = fn (dict, key)
  def v = findByFn(key, fn (x) nth(x, 0) end, dict)
  if v then
    head(tail(v))
  end
end

def dictUpdate = fn (dict, key, val)
  if head(dict) then
    if head(head(dict)) == key then
      dictAdd(tail(dict), key, val)
    else
      [head(dict)] ++ dictUpdate(tail(dict), key, val)
    end
  end
end

def dictRemove = fn (dict, key)
  if head(dict) then
    if head(head(dict)) == key then
      tail(dict)
    else
      [head(dict)] ++ dictRemove(tail(dict), key)
    end
  end
end

def d = []
def d1 = dictAdd(d, "one", 1)
def d2 = dictAdd(d1, "two", 2)
def d3 = dictAdd(d2, "three", 3)

show(d3)

show(dictGet(d3, "one"))

show(dictUpdate(d3, "one", 5))
show(dictUpdate(d3, "two", 5))
show(dictUpdate(d3, "three", 5))
show(dictUpdate(d3, "four", 5))

show(dictRemove(d3, "one"))
show(dictRemove(d3, "two"))
show(dictRemove(d3, "three"))
show(dictRemove(d3, "four"))