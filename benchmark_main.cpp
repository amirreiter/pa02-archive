#include <cstdlib>
#include <iostream>
#include <vector>

#include "movies.h"
#include "utilities.h"
#include "benchmark.cpp"

static const AlphabeticalOrdering alphaordering;

// Data loading

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

// Accurate timing on macos

void test_and_benchmark(std::string& movieFilePath) {
    // 1. read file
    std::string movieFile = read_file_to_string(movieFilePath);
    std::string_view movieFileView(movieFile);

    if (movieFile.empty()){
        std::cerr << "Could not open file " << movieFilePath;
        exit(1);
    }

    std::vector<Movie> movies;

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

    // 2. assert behavior is equivalent;
    {
        auto movies_stdsort = movies;
        auto movies_lexsort = movies;

        std::sort(movies_stdsort.begin(), movies_stdsort.end(), alphaordering);
        lexisort(movies_lexsort);

        if (movies_stdsort != movies_lexsort) {
            std::cerr << "BEHAVIOR MISMATCH\n";
            std::exit(-1);
        }
    } // Drop all resources for (2)

    // 3. benchmark

    // BEGIN STD SORT TEST

    for (size_t i = 0; i < 5; i++) {
        auto unsorted = movies;
        std::sort(unsorted.begin(), unsorted.end(), alphaordering);
    }

    bench::Run std_sort_suite;

    for (size_t i = 0; i < 25; i++) {
        auto unsorted = movies;

        std_sort_suite.start();
        std::sort(unsorted.begin(), unsorted.end(), alphaordering);
        std_sort_suite.end();
    }

    std::cout << "std::sort -------------------\n";
    std_sort_suite.analysis();
    std::cout << "-----------------------------\n";

    std::cout << "\n";

    // BEGIN LEX SORT TEST

    for (size_t i = 0; i < 5; i++) {
        auto unsorted = movies;
        lexisort(unsorted);
    }

    bench::Run lex_sort_suite;

    for (size_t i = 0; i < 25; i++) {
        auto unsorted = movies;

        lex_sort_suite.start();
        lexisort(unsorted);
        lex_sort_suite.end();
    }

    std::cout << "lexisort --------------------\n";
    lex_sort_suite.analysis();
    std::cout << "-----------------------------\n";

    std::cout << "\n";

    // BEGIN FAST LEX SORT TEST

    for (size_t i = 0; i < 5; i++) {
        auto unsorted = movies;
        lexisort(unsorted);
    }

    bench::Run fast_lex_sort_suite;

    for (size_t i = 0; i < 25; i++) {
        auto unsorted = movies;

        fast_lex_sort_suite.start();
        lexisort_fast(unsorted);
        fast_lex_sort_suite.end();
    }

    std::cout << "fast lexisort --- -----------\n";
    fast_lex_sort_suite.analysis();
    std::cout << "-----------------------------\n";
}

int main() {
    std::cout << "\n";
    std::string big_movies = "input_76920_random.csv";
    test_and_benchmark(big_movies);
    std::cout << "\n";
}
