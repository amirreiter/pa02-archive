#include "utilities.h"
#include "movies.h"

#include <string>
#include <cstdio>
#include <vector>

// reads a file into a giant string, by using C i/o methods
std::string read_file_to_string(const std::string& path) {
    auto fp = std::fopen(path.c_str(), "rb");
    if (!fp) {
        return "";
    }

    // size
    std::fseek(fp, 0, SEEK_END);
    const auto size = std::ftell(fp);
    std::fseek(fp, 0, SEEK_SET);

    // allocation
    std::string file_buffer;
    file_buffer.resize(size);

    // reading
    size_t x = std::fread(&file_buffer[0], 1, size, fp);
    std::fclose(fp);

    return file_buffer;
}

/*
    LEXISORT ALGORITHM

    this started out as a bucket sorting before quicksort but then i started
    googling and realized this is basically a kind of radix sort, but starting
    at the first digit and going forward, instead of the last digit and going
    backwards

        https://www.youtube.com/watch?v=Y95a-8oNqps

        https://codercorner.com/RadixSortRevisited.htm

    ["abc", "xyz", "gyz", "aaa", "a", "x"]

    | index = 0 sorting

    a: ["abc", "aaa", "a"]

        | index = 1 sorting

        _: ["a"]     // blank goes before everything
        a: ["aaa"]
        b: ["abc"]

    g: ["gyz"]

        | index = 1 sorting

        y: ["gyz"]

    x: ["xyz", "x"]

        | index = 1 sorting

        _: ["x"]
        y: ["xyz"]


    Draining all buckets in order now yields:
    [
        "a",
        "aaa",
        "abc",
        "gyz"
        "x",
        "xyz"
    ]
*/


// here's my first attempt at lexisort
static const AlphabeticalOrdering alphaordering;
void lexisort(
    std::vector<Movie>& movies,
    size_t index,
    size_t max_index
) {
    // No point in doing bucket sorting if we're not gonna fill the buckets.
    if (movies.size() < 257) {
        std::sort(movies.begin(), movies.end(), alphaordering);
        return;
    }

    // one for empty, 256 for ascii
    std::vector<Movie> drain[257];

    // might as well make a copy since we're anyway going to move it
    for (Movie m : movies) {
        if (m.name.size() <= index) {
            drain[0].push_back(m);
        } else {
            unsigned char letter = m.name[index];
            drain[static_cast<size_t>(letter) + 1].push_back(m);
        }
    }

    if (index + 1 != max_index) {
        for (auto& bucket : drain) {
            lexisort(bucket, index + 1, max_index);
        }
    }

    size_t write_head = 0;
    for (auto& bucket : drain) {
        for (auto m : bucket) {
            movies[write_head] = m;
            write_head++;
        }
    }
}

void lexisort(std::vector<Movie>& movies) {
    size_t max_string_size = 0;

    for (auto& m : movies) {
        size_t string_size = m.name.size();
        if (string_size > max_string_size) {
            max_string_size = string_size;
        }
    }

    lexisort(movies, 0, max_string_size);
}



// FAST version of lexisort
void lexisort_fast(
    // modify vector in place rather than make a copy
    std::vector<Movie>::iterator begin,
    std::vector<Movie>::iterator end,
    std::array<std::vector<Movie>, 257>& drain,
    size_t index,
    size_t max_index
) {
    // no point in doing bucket sorting if we're not gonna fill the buckets.
    // the 30 number comes from tuning with benchmark.sh
    if (end - begin < 30) {
        std::sort(begin, end, alphaordering);
        return;
    }

    if (index >= max_index) {
        return;
    }

    std::vector<Movie>::iterator head = begin;
    while (head != end) {
        // might as well make a copy since we're anyway going to move it
        Movie m = *head;
        if (m.name.size() <= index) {
            drain[0].push_back(m);
        } else {
            unsigned char letter = m.name[index];
            drain[static_cast<size_t>(letter) + 1].push_back(m);
        }
        head++;
    }

    // okay, this is definitely the ugliest variable definition i've ever written
    // this is kinda necessary to not heap allocate once per recursion call
    std::array<
        std::pair<
            std::vector<Movie>::iterator,
            std::vector<Movie>::iterator
        >,
        257
    > buckets;

    std::vector<Movie>::iterator bucket_begin = begin;
    head = begin;
    for (size_t i = 0; i < 257; i++) {
        bucket_begin = head;
        for (Movie m : drain[i]) {
            *head = m;
            head++;
        }
        buckets[i] = {bucket_begin, head};

        drain[i].clear();
    }

    if (index + 1 < max_index) {
        // skip bucket 0 because elements with length <= index are already fully sorted
        for (size_t i = 1; i < 257; i++) {
            if (buckets[i].first != buckets[i].second) {
                lexisort_fast(
                    buckets[i].first,
                    buckets[i].second,
                    drain,
                    index + 1,
                    max_index
                );
            }
        }
    }
}

void lexisort_fast(std::vector<Movie>& movies) {
    // early exit before ANY allocations, for lower sized inputs
    if (movies.size() < 257) {
        std::sort(movies.begin(), movies.end(), alphaordering);
        return;
    }

    size_t max_string_size = 0;

    for (auto& m : movies) {
        size_t string_size = m.name.size();
        if (string_size > max_string_size) {
            max_string_size = string_size;
        }
    }

    // reuse the drain every sort!
    std::array<std::vector<Movie>, 257> drain;

    lexisort_fast(
        movies.begin(),
        movies.end(),
        drain,
        0,
        max_string_size
    );
}
