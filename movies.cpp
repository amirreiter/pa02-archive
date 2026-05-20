#include "movies.h"

bool AlphabeticalOrdering::operator()(const Movie& lhs, const Movie& rhs) const {
    // cpp apparently already has lexographic string ordering
    return lhs.name < rhs.name;
}

bool RatingOrdering::operator()(const Movie* lhs, const Movie* rhs) const {
    // regular ordering
    if (lhs->rating != rhs->rating) {
        return lhs->rating < rhs->rating;
    }

    // lexographic ordering if they have the same rating
    // this is because lhs will always be lexographically "less" than rhs
    return lhs->name > rhs->name;
}
