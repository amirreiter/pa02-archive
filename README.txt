             RUNTIME AND SPACE COMPLEXITY, OPTIMIZATION ANALYSIS
                                                                     Amir Reiter

Assumptions:
    there are n movies in the dataset.
    there are m prefixes specified in a given run of your program.
    at most k movies begin with each prefix.
    l is the maximum number of characters in a movie name

Foreword:
    As per the assignment instructions, the runtime complexity analysis of my
    submission assumes that:

     - "all n movies are already stored in your data structure."
     - "all m prefixes are already stored in an array."

    and as such, the runtime complexity analysis below waves away the time
    required to insert into std::set and the lexographical comparisons. Thus,
    k and l are not factors in the runtime analysis.

Secondary Foreword:
    As a result of part 3 (optimizing for performance), each answer has an
    answer regarding my initial implementation and a secondary answer explaining
    the code as it is currently written, to show the. progression of how my
    thinking changed over the course of working this problem.

    In addition, I've added a section at the end for non algorithmic
    optimizations and how they contributed to the runtime.

                             -- Part 3a --

(1) My initial implementation used std::set: To print all the movies in
ascending alphabetical takes O(n) because the std::set is implemented as a
binary tree which is already lexographically sorted. This means that printing
the movies in ascending order is a simple in-order traversal, which takes O(n).

    After optimizing for speed, I realized that the bst std::set implementation
    was a major performance hurdle due to cache locality. I ended up replacing
    it with a sorted vector. This has very similar advantages to a bst, where
    iterating over the list in order is O(n), but it maintains cpu cache
    locality in the late stage of the sorting process. Moreover, iteration over
    the vector also becomes very cache-friendly.

    I've been working over the past two days on a large algorithmic change to
    the lexigraphic sorting in the initial phase of the data loading and
    parsing. An illustration of the algorithm is available in utilities.cpp.
    The basic idea is as follows: No matter what the actual name of the movie,
    we know that every movie that starts with an 'a' should appear before the
    first 'b'. Then within the 'a's, we can apply a similar logic, where all the
    'aa' movies MUST appear before the 'ab's. This logic continues recursively.
    This is a kind of radix sort, which is significantly faster than std::sort
    (which stackoverflow says is a kind of "introsort").


(2) For the second task (printing the movies by prefix in descending rating
order), I initially chose to create vector of priority queues to get the best
performance characteristics of both ADTs. The vector gives O(1) indexing for a
particular prefix, and is structured such that it aligns with the order of
vector for prefixes. Each bucket in the vector owns a priority queue for the
rating-ordered list. Since popping from the queue is O(logk), printing the whole
queue in order is O(k logk), and looping over the entire list of prefixes is
O(m). Thus this, portion of the algorithm executes in O(m k logk).

    After optimizing for speed, sorting a vector and saving the winner was
    faster. I have two theories as for why: Firstly, the algorithmic complexity
    for priority queue insertion and deletion is not great. Vectors give O(1)
    insertion and deletion, whereas sorting them is the hardest task. However,
    a single O(k logk) sort seems to be better than O(k logk) insertion PLUS
    O(k logk) deletion, making it O(2k logk) in total. For wall-to-wall runtime
    we care about the coefficients. Secondly, the nature of the priority queue
    means only the ordering of the top is guarenteed, which means popping is
    required to iterate, and thus a secondary vector is required to remember
    the winning scores for each prefix.

    Second, when we grab the lexographic range from our sorted movies vector,
    in order to determine where our prefix begins and ends within our range, we
    can take advantage of the fact that all the first letters are in contiguous
    blocks. This allows us to discard 1/26 of the search space in our binary
    search almost immediatly. It would be cool to be able to dot his recursively
    like a binary tree, where instead of a binary search we have a 26th-ary
    search, cutting out 1/26 of the possibilites at any given time. In practice
    this was slower but I think it was more as a result of fragmenting the
    memory and doing pointer chasing. It might be possible to build this tree
    using iterators, thus the actual reading happens in the contigious vector.
    Perhaps a project for after the due-date.

(3) For the third task (printing the highest rated movie with that prefix if it
exists) we take advantage of the priority queue from (2) and save the first item
we pop into a vector of movies which is likewise aligned to the order of the
vector of prefixes. Thus, getting the best performer is a simple O(1) index,
which happens for m prefixes thus making the third task O(m).

    After optimizing for speed, we no longer use a priority queue, and we can
    re-use our sorted vector from the beginning, which preserves O(1) top index.
    We save the top index into a separate vec, since we no longer need the
    entire order rating, just the top.

Benchmarks (Apple Silicon M1 Max):

    > time ./runMovies input_20_random.csv prefix_large.txt
        0.01s user 0.02s system 88% cpu 0.028 total

    > time ./runMovies input_100_random.csv prefix_large.txt
        0.01s user 0.02s system 92% cpu 0.033 total

    > time ./runMovies input_1000_random.csv prefix_large.txt
        0.01s user 0.03s system 90% cpu 0.039 total

    > time ./runMovies input_76920_random.csv prefix_large.txt
        0.05s user 0.08s system 96% cpu 0.128 total

                             -- Part 3b --

The space complexity analysis gets kind-of complicated depending on how you
choose to count what is and isn't in my "data structure". The trick with this
problem is the order in which things need to be printed. First, we must print
the movies alphabetically. Second, we must sort them by prefix, but by rating.
Then, we need the top rated movies per prefix.

Not one single data structure is suited for these tasks. However, the ordering
of these tasks presents opportunities for restructuring the data efficiently.

First, we put the data into a lexographically sorted vector. This takes S(n)
spaces as we have n movies.

From there, we collect each prefix matching movie into a vector per prefix. Only
one prefix is processed at a time, making the space complexity just one O(k).
The winners must also be stored in an vector of S(m), one per prefix.

Tangent:

    It's worth talking about the importance of the ordering of these
    restructurings, as they are not covered by the assignment's restriction on
    what counts as runtime complexity, and relate to the selection of these ADTs
    in the first place.

    The lexographicly sorted vector allows to quickly locate the start and ends
    of lexographic matches when we're moving from the first vector to the vec
    of vectors. Moreover, the iteration from the set to the vec is linear
    because all lexographic matches must be an a continuous in-order sequence
    inside the first vector.

    From there, accessing the top performers in the rating-sorted vectors
    is O(1)

    The ADTs are not just fast for their specific phase in the assignment, they
    also quickly and efficiently transition to the next ADT.

                             -- Part 3c --

Overall, I would say I traded higher space complexity for lower time
complexity, as this assignment was specifically about execution speed, and
had no target space complexity. I can think of two specific scenarios in
which this decision had a major impact on the algorithm.

1. When sorting movies by prefix AND rating, originally I duplicated data to
   make the index fast, but this came at the cost of wasting space as each
   movie owned its own string. Now with zero-copy logic, each Movie becomes a
   Movie*, meaning that although our big S growth remains the same for these
   copies, their coefficent's become 1 because all pointers have the same size.

2. Any ADT to do with prefixes assumes every prefix has movies that are
   associated with it. This is not all bad, because a vector or priority
   queue of size 0 only consumes stack space, as they allocate on the first
   push. One could debate whether a hashmap would help solve this, but
   although I didn't benchmark memory usage I would recon this would not be
   helpful for the following reasons: Firstly, the hashmap reallocates
   as it grows, just like a vector, which is very costly. It might help to
   reserve a percentage of the size of prefixes, but given that if we're
   wrong and most vector/hashmap reallocation strategies tend to double
   their capacity on resize, so we might end up OVER allocating which is
   worse than the simple vector lookup implementation.

I would also say that I had no specific target time complexity, but
throughout the assignment I was looking in the O(1) to O(n logn) range for
each part of the assignment, and kept iterating until I landed the lowest time
complexity I could think of. I was always willing to sacrifice space complexity
if it made the big O or wall-to-wall timing faster. In the end, though, not
duplicating data has its own performance improvements.

Non-complexity related performance improvements:

    I benchmarked the executable on my machine with Samply, a sampling profiler,
    to identify specific bottlenecks beyond the broad algorthmic architecutre

    1. I changed the data loading to be completely zero-copy. All strings are
       views into the larger file, ensuring there are no duplicated strings or
       copying of strings.

    2. I pre-allocated vectors when their size was known beforehand. For
       example, when initializing the vector for prefixes matches, the length
       is already known since the vector of prefixes has a known size. This
       prevents expensive resizing operations on complex ADTs.

    3. When benchmarking with Samply I noticed a significant portion of the wall
       time was spent on I/O. Per this post on StackOverflow:
           https://stackoverflow.com/questions/43051948/why-is-stdcout-so-time-consuming
       Depending on the operating system, cout can be really slow by hammering
       the terminal, trying to flush whatever internal buffer it has repeatedly
       on every newline. This is a simple 2-line fix, which is at the top of the
       main function. Another I/O optimization is using our own string buffer
       and using sprintf to format floats, flushing only at the very end.
