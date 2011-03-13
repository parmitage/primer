### The number of genes in our chromosome (number of items to pack)
val geneCount = 12;

### The population size of a single generation
val population = 16;

### The number of generations to run the algorithm for
val generations = 250;

### The weight limit of the knapsack
val weightLimit = 7;

### The list of items to pack
val items = [[ "Book", 2, 0 ],
             [ "Camera", 2, 1 ],
             [ "Tent", 3, 6 ],
             [ "Map", 1, 4 ],
             [ "Food", 1, 6 ],
             [ "Drink", 1, 6 ],
             [ "Clothes", 2, 5 ],
             [ "Torch", 2, 1 ],
             [ "Soap", 1, 2 ],
             [ "Guitar", 3, 0 ],
             [ "Football", 2, 0 ],
             [ "Toothbrush", 1, 2 ]];

### Perform simple crossover reproduction of two chromosomes
val reproduce = fun c1 c2 -> (c1 & 4032) | (c2 & 63);

### Randomly (infrequently) mutate a chromosome
val mutate = fun c ->
   if Rnd(10000) mod 1000 == 17
   then c ^ (1 << (Rnd(10000) mod (geneCount - 1)))
   else c;

### Evaluate the fitness of a chromosome
val fitness = fun c ->
   let items = chromosomeItems(c) in
   let importance = importanceOfItems(items) in
   let weight = weightOfItems(items) in
   if weight <= weightLimit
   then importance * weight
   else weight * -5;

### Generate a random 12 bit number to represent a chromosome
val chromosomeNew = fun x -> Rnd(4095);

### Return the list of items that this chromosome represents
val chromosomeItems = fun c ->
   let inner = fun x xs ->
      if x < geneCount
      then if BitSet(c, x)
           then inner(x + 1, (items at x) :: xs)
           else inner(x + 1, xs)
      else xs
   in inner(0, []);

### Return the names of a list of items
val namesOfItems = fun items -> Map(fun i -> i at 0, items);

### Return the weight of a list of items
val weightOfItems = fun items -> FoldL(fun x y -> x + (y at 1), 0, items);

### Return the importance of this chromosome
val importanceOfItems = fun items -> FoldL(fun x y -> x + (y at 2), 0, items);

### Create a new generation of random chromosomes
# TODO how to pass single argument function to Collect?
val generationNew = fun size -> Collect(chromosomeNew, size);

### Create the next generation by reproducing the best of the previous one
val generationNext = fun cur gen ->
   let sortedGeneration = Reverse(QSortBy(cur, fitness)) in
   let bestOfGeneration = Take(population / 2, sortedGeneration) in
   bestOfGeneration
      ++ Map(mutate, MapPair(reproduce, bestOfGeneration))
      ++ generationNew(3);

let initialGeneration = generationNew(population) in
let finalGeneration = FoldL(generationNext, initialGeneration, 1..generations) in
   namesOfItems(chromosomeItems(finalGeneration at 0));