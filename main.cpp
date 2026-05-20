// Spring '26
// Instructor: Diba Mirza
// Student name: Amir Reiter

#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>
#include <ctime>
#include <vector>
#include <cstring>
#include <limits.h>
#include <iomanip>
using namespace std;

#include "utilities.h"
#include "movies.h"

Movie parseLine(std::string_view line);

int main(int argc, char** argv){
    auto alphabetordering = AlphabeticalOrdering();
    auto ratingordering = RatingOrdering();

    // prevent buffer from flushing on every newline
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (argc < 2){
        cerr << "Not enough arguments provided (need at least 1 argument)." << '\n';
        cerr << "Usage: " << argv[ 0 ] << " moviesFilename prefixFilename " << '\n';
        exit(1);
    }

    string movieFile = read_file_to_string(argv[1]); // must live for entire app
    string_view movieFileView(movieFile);
    if (movieFile.empty()){
        cerr << "Could not open file " << argv[1];
        exit(1);
    }
    // Create an object of a STL data-structure to store all the movies
    std::vector<Movie> movies;
    movies.reserve(76920);

    // Read each file and store the name and rating
    // updated this to be zero-copy
    size_t read_anchor = 0;
    while (read_anchor < movieFileView.size()) {
        size_t next_newline = movieFileView.find('\n', read_anchor);
        if (next_newline == std::string_view::npos) {
            next_newline = movieFileView.size();
        }

        std::string_view line = movieFileView.substr(read_anchor, next_newline - read_anchor);

        movies.push_back(parseLine(line));

        read_anchor = next_newline + 1;
    }

    std::sort(movies.begin(), movies.end(), alphabetordering);

    if (argc == 2){
            //print all the movies in ascending alphabetical order of movie names
            for (const auto& m : movies) {
                std::cout << m.name << ", " << m.rating << "\n";
            }
            return 0;
    }

    string prefixFile = read_file_to_string(argv[2]);  // must live for entire app
    string_view prefixFileView(prefixFile);

    if (prefixFile.empty()) {
        cerr << "Could not open file " << argv[2];
        exit(1);
    }

    vector<string_view> prefixes;
    prefixes.reserve(17578);

    // updated this to be zero-copy
    read_anchor = 0;
    while (read_anchor < prefixFileView.size()) {
        size_t next_newline = prefixFileView.find('\n', read_anchor);
        if (next_newline == std::string_view::npos) {
            next_newline = prefixFileView.size();
        }

        std::string_view line = prefixFileView.substr(read_anchor, next_newline - read_anchor);

        if (!line.empty()) {
            prefixes.push_back(line);
        }

        read_anchor = next_newline + 1;
    }

    // for debugging
        // cout << "'" << prefixes.back() << "'\n";
        // cout << "'" << movies.back().name << "'\n";
        // return 0;

    //  For each prefix,
    //  Find all movies that have that prefix and store them in an appropriate data structure

    // this is the worst variable definition i've ever written but the compiler will not stop complaining
    // unless i specify every single template parameter
    // vector<priority_queue<Movie, vector<Movie>, RatingOrdering>> prefixed_movies(prefixes.size(), priority_queue<Movie, vector<Movie>, RatingOrdering>());

    vector<vector<Movie>> prefixed_movies;
    prefixed_movies.resize(prefixes.size());

    for (size_t i = 0; i < prefixes.size(); i++) {
        string_view p = prefixes[i];

        // movies are already sorted by lexographic order, find first which begins to match
        auto it = lower_bound(movies.begin(), movies.end(), Movie(p, 0.0), alphabetordering);

        while (it != movies.end() && it->name.compare(0, p.size(), p) == 0) {
            prefixed_movies[i].push_back(*it); // weird casting to stop compiler from complaining
            ++it;
        }

        std::sort(prefixed_movies[i].rbegin(), prefixed_movies[i].rend(), ratingordering);
    }

    // For each prefix
    // Print the movies by prefix in descending order.
    // Save the winner for later
    for (size_t i = 0; i < prefixes.size(); i++) {
        auto& mdb = prefixed_movies[i];

        for (auto& m : mdb) {
            cout << m.name << ", " << std::fixed << std::setprecision(1) << m.rating << '\n';
        }

        if (!mdb.empty()) {
            cout << '\n';
        } else {
            cout << "No movies found with prefix " << prefixes[i] << '\n';
        }
    }

    //  For each prefix,
    //  Print the highest rated movie with that prefix if it exists.
    for (size_t i = 0; i < prefixes.size(); i++) {
        string_view p = prefixes[i];
        if (!prefixed_movies[i].empty()) {
            auto& m = prefixed_movies[i][0];
            cout << "Best movie with prefix " << p << " is: " << m.name
                << " with rating " << std::fixed << std::setprecision(1)
                << m.rating << '\n';
        }
    }

    return 0;
}

/*

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

(2) For the second task (printing the movies by prefix in descending rating
order), I initially chose to create vector of priority queues to get the best
performance characteristics of both ADTs. The vector gives O(1) indexing for a
particular prefix, and is structured such that it aligns with the order of
vector for prefixes. Each bucket in the vector owns a priority queue for the
rating-ordered list. Since popping from the queue is O(logk), printing the whole
queue in order is O(k logk), and looping over the entire list of prefixes is
O(m). Thus this, portion of the algorithm executes in O(m k logk).

    After optimizing for speed, a vector of sorted vectors ended up being
    faster. I have two theories as for why: Firstly, the algorithmic complexity
    for priority queue insertion and deletion is not great. Vectors give O(1)
    insertion and deletion, whereas sorting them is the hardest task. However,
    a single O(k logk) sort seems to be better than O(k logk) insertion PLUS
    O(k logk) deletion, making it O(2n logn) in total. For wall-to-wall runtime
    we care about the coefficients. Secondly, the nature of the priority queue
    means only the ordering of the top is guarenteed, which means popping is
    required to iterate, and thus a secondary vector is required to remember
    the winning scores for each prefix.

(3) For the third task (printing the highest rated movie with that prefix if it
exists) we take advantage of the priority queue from (2) and save the first item
we pop into a vector of movies which is likewise aligned to the order of the
vector of prefixes. Thus, getting the best performer is a simple O(1) index,
which happens for m prefixes thus making the third task O(m).

    After optimizing for speed, we no longer use a priority queue, and we can
    re-use our sorted vector from the beginning, which preserves O(1) top index.

Benchmarks (Apple Silicon M1 Max):

    > time ./runMovies input_20_random.csv prefix_large.txt
        0.01s user 0.03s system 92% cpu 0.037 total

    > time ./runMovies input_100_random.csv prefix_large.txt
        0.01s user 0.03s system 93% cpu 0.039 total

    > time ./runMovies input_1000_random.csv prefix_large.txt
        0.01s user 0.03s system 93% cpu 0.044 total

    > time ./runMovies input_76920_random.csv prefix_large.txt
        0.07s user 0.08s system 97% cpu 0.158 total

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

From there, we move the data to a vector of sorted vectors. The vector is sized
according to the number of prefixes, so of size m, and at most each priority
queue can have k members, including duplicates since a movie may be a part of
more than one prefix. Therfore the space complexity of this data structure is
S(km) worst case.

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
       main function.

*/

inline Movie parseLine(std::string_view line) {
    size_t commaIndex = line.find_last_of(',');
    char* endPtr;
    float movieRating = std::strtod(line.data() + commaIndex + 1, &endPtr);
    std::string_view movieName = line.substr(0, commaIndex);
    if (movieName[0] == '\"') {
        movieName = movieName.substr(1, movieName.length() - 2);
    }
    return Movie(movieName, movieRating);
}
