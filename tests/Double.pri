def a = 10
def l = [1,2,3,4,5]

def Double = lambda (x)
    return 2 * x;
end

def SumWithFun = lambda (fn, x, y)
    return fn(x) + fn(y);
end

Show(Double(2))
Show(Double(12.45))
Show(Double(a))
Show(Map(Double, l))
Show(Map(lambda (x)
             return x >= 3;
         end,
         l))
Show(SumWithFun(Double, 4, a))
Show(SumWithFun(lambda (x) return 12 % x; end, 4, a))