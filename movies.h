#ifndef MOVIES_H
#define MOVIES_H

#include <string>
#include <string_view>

class Movie {
public:
    std::string_view name;
    double rating;

    Movie(std::string_view name, double rating) : name(name), rating(rating) {};
};

class AlphabeticalOrdering {
public:
    bool operator()(const Movie& lhs, const Movie& rhs) const;
};

class RatingOrdering {
public:
    // because we're manually pointimg to movies this has to be
    // const Movie* and not const Movie&
    bool operator()(const Movie* lhs, const Movie* rhs) const;
};

#endif
