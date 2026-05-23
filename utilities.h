#ifndef UTILITIES_H
#define UTILITIES_H

#include "movies.h"
#include <string>
#include <vector>

std::string read_file_to_string(const std::string& path);

void lexisort(std::vector<Movie>& movies);

void lexisort_fast(std::vector<Movie>& movies);

#endif
