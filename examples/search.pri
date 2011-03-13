val binarySearch = fun xs x ->
   let inner = fun xs x s e ->
                  let diff = e - s in
                  let mid = (diff / 2) as int in
                  let idx = s + mid in
                  let num = xs at idx in
                  if s <= e
                  then if x == num
                       then idx
                       else if x < num
                            then inner(xs, x, s, e - 1)
                            else inner(xs, x, s + 1, e)
                  else false
   in inner(xs, x, 0, length(xs) - 1);

val listToSearch = 1..1000;

binarySearch(listToSearch, 500);
binarySearch(listToSearch, 1);
binarySearch(listToSearch, 999);
binarySearch(listToSearch, 1001);
binarySearch(listToSearch, -1);