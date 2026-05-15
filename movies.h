#ifndef MOVIES_H
#define MOVIES_H

#include <string>

class Movie {
public:
    std::string name;
    double rating;

    Movie(std::string name, double rating) : name(name), rating(rating) {};
};

class AlphabeticalOrdering {
public:
    bool operator()(const Movie& lhs, const Movie& rhs) const;
};

#endif
