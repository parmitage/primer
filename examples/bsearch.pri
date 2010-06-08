def bsearch = fn (xs, x)
  def inner = fn (xs, x, s, e)
    if s <= e then
      def diff = e - s, mid = diff / 2, idx = s + mid
      if x == nth(xs, idx) then
        idx
      elif x < nth(xs, idx) then
        inner(xs, x, s, e - 1)
      else
        inner(xs, x, s + 1, e)
      end
    else
      -1
    end
  end
  inner(xs, x, 0, length(xs) - 1)
end

show(bsearch(range(1, 100), 89))