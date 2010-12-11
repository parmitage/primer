### The number of genes in our chromosome the (number of items to pack)
GeneCount: 12

### The population size of a single generation
Population: 16

### The number of generations to run the algorithm for
Generations: 250

### The weight limit of the knapsack that we're trying to fill
WeightLimit: 7

### The list of items we'd like to pack into our knapsack
Items: [[ "Book", 2, 0 ],
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
        [ "Toothbrush", 1, 2 ]]

### Perform simple crossover reproduction of two chromosomes
Reproduce: fn (c1, c2)
   (c1 & 4032) | (c2 & 63)
end

### Randomly (infrequently) mutate a chromosome
Mutate: fn (c)
   if Rnd(10000) mod 1000 == 17
   then c ^ (1 << (Rnd(10000) mod (GeneCount - 1)))
   else c
end

### Evaluate the fitness of a chromosome
Fitness: fn (c)
   if weight <= WeightLimit
   then importance * weight
   else weight * -5
   where items: ChromosomeItems(c)
         importance: ImportanceOfItems(items)
         weight: WeightOfItems(items)
end

### Generate a random 12 bit number to represent a chromosome
ChromosomeNew: fn () Rnd(4095) end

### Return the list of items that this chromosome represents
ChromosomeItems: fn (c)
   Inner(0, [])
   where Inner: fn (x, xs)
             if x < GeneCount
             then if BitSet(c, x)
                  then Inner(x + 1, (Items at x) :: xs)
                  else Inner(x + 1, xs)
             else xs
         end
end

### Return the names of a list of items
NamesOfItems: fn (items) Map(fn (i) i at 0 end, items) end

### Return the weight of a list of items
WeightOfItems: fn (items) FoldL(fn (x, y) x + (y at 1) end, 0, items) end

### Return the importance of this chromosome
ImportanceOfItems: fn (items) FoldL(fn (x, y) x + (y at 2) end, 0, items) end

### Create a new generation of random chromosomes
GenerationNew: fn (size) Collect(ChromosomeNew, size) end

### Create the next generation by reproducing the best of the previous one
GenerationNext: fn (cur, gen)
   BestOfGeneration
      ++ Map(Mutate, MapPair(Reproduce, BestOfGeneration))
      ++ GenerationNew(3)
   where SortedGeneration: Reverse(SortBy(cur, Fitness))
         BestOfGeneration: Take(Population / 2, SortedGeneration)
end

InitialGeneration: GenerationNew(Population)
FinalGeneration: FoldL(GenerationNext, InitialGeneration, 1..Generations)
Show(NamesOfItems(ChromosomeItems(FinalGeneration at 0)))