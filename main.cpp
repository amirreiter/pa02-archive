// Spring '26
// Instructor: Diba Mirza
// Student name: Amir Reiter

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <vector>
#include <cstring>
#include <algorithm>
#include <limits.h>
#include <iomanip>
#include <set>
#include <queue>
#include <sstream>
using namespace std;

#include "utilities.h"
#include "movies.h"

bool parseLine(string &line, string &movieName, double &movieRating);

int main(int argc, char** argv){
    if (argc < 2){
        cerr << "Not enough arguments provided (need at least 1 argument)." << endl;
        cerr << "Usage: " << argv[ 0 ] << " moviesFilename prefixFilename " << endl;
        exit(1);
    }

    ifstream movieFile (argv[1]);
    if (movieFile.fail()){
        cerr << "Could not open file " << argv[1];
        exit(1);
    }
    // Create an object of a STL data-structure to store all the movies
    std::set<Movie, AlphabeticalOrdering> movies;

    string line, movieName;
    double movieRating;
    // Read each file and store the name and rating
    while (getline (movieFile, line) && parseLine(line, movieName, movieRating)){
            // Use std::string movieName and double movieRating
            // to construct your Movie objects
            // cout << movieName << " has rating " << movieRating << endl;
            // insert elements into your data structure
            movies.insert(
                Movie(movieName, movieRating)
            );
    }

    movieFile.close();

    if (argc == 2){
            //print all the movies in ascending alphabetical order of movie names
            for (auto m : movies) {
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
    //  If no movie with that prefix exists print the following message

    // this is the worst variable definition i've ever written but the compiler will not stop complaining
    // unless i specify every single template parameter
    vector<priority_queue<Movie, vector<Movie>, RatingOrdering>> prefixed_movies(prefixes.size(), priority_queue<Movie, vector<Movie>, RatingOrdering>());

    for (size_t i = 0; i < prefixes.size(); i++) {
        string& p = prefixes[i];

        // movies are already sorted by lexographic order, find first which begins to match
        auto it = movies.lower_bound(Movie(p, 0.0));

        while (it != movies.end() && it->name.compare(0, p.size(), p) == 0) {
            prefixed_movies[i].push(*it);
            ++it;
        }

        if (prefixed_movies[i].size() == 0) {
            cout << "No movies found with prefix " << p << endl;
        }
    }

    // For each prefix
    // Print the movies by prefix in descending order.
    auto prefixed_movies_copy = prefixed_movies;
    for (size_t i = 0; i < prefixes.size(); i++) {
        auto mdb = prefixed_movies_copy[i];
        while (!mdb.empty()) {
            auto m = mdb.top();
            mdb.pop();

            cout << m.name << " " << m.rating << endl;
        }
        cout << endl;
    }

    //  For each prefix,
    //  Print the highest rated movie with that prefix if it exists.
    for (size_t i = 0; i < prefixes.size(); i++) {
        string& p = prefixes[i];
        auto mdb = &prefixed_movies[i];

        if (mdb->size() != 0) {
            auto m = &mdb->top();
            cout << "Best movie with prefix " << p << " is: " << m->name << " with rating " << std::fixed << std::setprecision(1) << m->rating << endl;
        }
    }

    return 0;
}

/* Add your run time analysis for part 3 of the assignment here as commented block*/

bool parseLine(string &line, string &movieName, double &movieRating) {
    int commaIndex = line.find_last_of(",");
    movieName = line.substr(0, commaIndex);
    movieRating = stod(line.substr(commaIndex+1));
    if (movieName[0] == '\"') {
        movieName = movieName.substr(1, movieName.length() - 2);
    }
    return true;
}
