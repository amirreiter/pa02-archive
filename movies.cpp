#include "movies.h"

bool AlphabeticalOrdering::operator()(const Movie& lhs, const Movie& rhs) const {
    // cpp apparently already has lexographic string ordering
    return lhs.name < rhs.name;
}
