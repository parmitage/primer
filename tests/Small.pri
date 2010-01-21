def nums = [4,1,2,7,3,0]

def BigEnough = lambda (x)
	x > 3;
end

Show(Nth(nums, 2))
Show(Sum(nums))
Show(Map(BigEnough, nums))
Show(Filter(BigEnough, nums))
Show(Any(BigEnough, nums))
Show(All(BigEnough, nums))
Show(FoldL(lambda (x, y) x + y; end, 0, nums))
Show(FoldR(lambda (x, y) x + y; end, 0, nums))
Show(Find(3, nums))
Show(Empty(nums))