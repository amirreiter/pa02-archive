// Spring '26
// Instructor: Diba Mirza
// Student name: Amir Reiter

#include <algorithm>
#include <iostream>
#include <fstream>
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

bool parseLine(string &line, string_view &movieName, float &movieRating);

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

    ifstream movieFile (argv[1]);
    if (movieFile.fail()){
        cerr << "Could not open file " << argv[1];
        exit(1);
    }
    // Create an object of a STL data-structure to store all the movies
    std::vector<Movie> movies;

    string line;
    string_view movieName; // string view is faster, zero copy
    float movieRating;
    // Read each file and store the name and rating
    while (getline (movieFile, line) && parseLine(line, movieName, movieRating)){
            // Use std::string movieName and float movieRating
            // to construct your Movie objects
            // cout << movieName << " has rating " << movieRating << '\n';
            // insert elements into your data structure
            movies.emplace_back(movieName, movieRating);
    }

    std::sort(movies.begin(), movies.end(), alphabetordering);

    movieFile.close();

    if (argc == 2){
            //print all the movies in ascending alphabetical order of movie names
            for (const auto& m : movies) {
                std::cout << m.name << ", " << m.rating << "\n";
            }
            return 0;
    }

    ifstream prefixFile (argv[2]);

    if (prefixFile.fail()) {
        cerr << "Could not open file " << argv[2];
        exit(1);
    }

    vector<string> prefixes;
    
    while (getline (prefixFile, line)) {
        if (!line.empty()) {
            prefixes.push_back(line);
        }
    }

    //  For each prefix,
    //  Find all movies that have that prefix and store them in an appropriate data structure

    // this is the worst variable definition i've ever written but the compiler will not stop complaining
    // unless i specify every single template parameter
    // vector<priority_queue<Movie, vector<Movie>, RatingOrdering>> prefixed_movies(prefixes.size(), priority_queue<Movie, vector<Movie>, RatingOrdering>());

    vector<vector<Movie>> prefixed_movies;
    prefixed_movies.resize(prefixes.size());

    for (size_t i = 0; i < prefixes.size(); i++) {
        string& p = prefixes[i];

        // movies are already sorted by lexographic order, find first which begins to match
        auto it = lower_bound(movies.begin(), movies.end(), Movie(p, 0.0), alphabetordering);

        while (it != movies.end() && it->name.compare(0, p.size(), p) == 0) {
            prefixed_movies[i].push_back(*it);
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
        string& p = prefixes[i];
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


                             -- Part 3a --

(1) To print all the movies in ascending alphabetical takes O(n) because the
std::set is implemented as a binary tree which is already lexographically
sorted. This means that printing the movies in ascending order is a simple
in-order traversal, which takes O(n).

(2) For the second task (printing the movies by prefix in descending rating
order), I chose to create vector of priority queues to get the best performance
characteristics of both ADTs. The vector gives O(1) indexing for a particular
prefix, and is structured such that it aligns with the order of vector for
prefixes. Each bucket in the vector owns a priority queue for the rating-ordered
list. Since popping from the queue is O(logn), printing the whole queue in order
is O(n logn), and looping over the entire list of prefixes is O(m). Thus this,
portion of the algorithm executes in O(m n logn).

(3) For the third task (printing the highest rated movie with that prefix if it
exists) we take advantage of the priority queue from (2) and save the first item
we pop into a vector of movies which is likewise aligned to the order of the
vector of prefixes. Thus, getting the best performer is a simple O(1) index,
which happens for m prefixes thus making the third task O(m).

Benchmarks (Apple Silicon M1 Max):

    > time ./runMovies input_20_random.csv prefix_large.txt
        0.01s user 0.03s system 41% cpu 0.087 total
    
    > time ./runMovies input_100_random.csv prefix_large.txt
        0.01s user 0.03s system 39% cpu 0.099 total
    
    > time ./runMovies input_1000_random.csv prefix_large.txt
        0.01s user 0.03s system 38% cpu 0.098 total

    > time ./runMovies input_76920_random.csv prefix_large.txt
        0.08s user 0.08s system 56% cpu 0.286 total

                             -- Part 3b --

The space complexity analysis gets kind-of complicated depending on how you
choose to count what is and isn't in my "data structure". The trick with this
problem is the order in which things need to be printed. First, we must print
the movies alphabetically. Second, we must sort them by prefix, but by rating.
Then, we need the top rated movies per prefix.

Not one single data structure is suited for these tasks. However, the ordering
of these tasks presents opportunities for restructuring the data efficiently.

First, we put the data into an std::set, sorted lexographically. This takes S(n)
spaces as we have n movies.

From there, we move the data to a vector of priority queues. The vector is sized
according to the number of prefixes, so of size m, and at most each priority
queue can have k members, including duplicates since a movie may be a part of
more than one prefix. Therfore the space complexity of this data structure is
S(km) worst case.

From there, we pop items off of the priority queues, saving only the best rating
movie for each prefix. Since they are stored in a vectors sized in accordance
with the prefixes vector, this is S(m).

Tangent:

    It's worth talking about the importance of the ordering of these
    restructurings, as they are not covered by the assignment's restriction on
    what counts as runtime complexity, and relate to the selection of these ADTs
    in the first place.

    The lexographic std::set allows to quickly locate the start and ends of
    lexographic matches when we're moving from the std::set to the vec of
    priority queues. Moreover, the iteration from the set to the vec is linear
    because all lexographic matches must be an a continuous in-order sequence
    inside the set.

    From there, saving the top performers in the priority queues is trivial as
    we can just save the first value we pop from them.

    The ADTs are not just fast for their specific phase in the assignment, they
    also quickly and efficiently transition to the next ADT.

                             -- Part 3c --

Overall, I would say I traded higher space complexity for lower time
complexity, as this assignment was specifically about execution speed, and
had no target space complexity. I can think of two specific scenarios in
which this decision had a major impact on the algorithm.

1. When sorting movies by prefix AND rating, I opted to duplicate data in
    order to make the querying fast. This is a waste of space, but makes
    the algorithm fast.

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
each part of the assignment.

*/

bool parseLine(std::string &line, std::string_view &movieName, float &movieRating) {
    size_t commaIndex = line.find_last_of(',');
    char* endPtr;
    movieRating = std::strtod(line.data() + commaIndex + 1, &endPtr);
    std::string_view lineView = line;
    movieName = lineView.substr(0, commaIndex);
    if (movieName[0] == '\"') {
        movieName = movieName.substr(1, movieName.length() - 2);
    }
    return true;
}
