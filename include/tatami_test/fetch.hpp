#ifndef TATAMI_TEST_FETCH_HPP
#define TATAMI_TEST_FETCH_HPP

#include "tatami/base/Extractor.hpp"
#include "tatami/base/SparseRange.hpp"
#include "tatami/utils/copy.hpp"

#include <vector>

/**
 * @file fetch.hpp
 * @brief Fetch a row/column into a vector.
 */

namespace tatami_test {

/**
 * @cond
 */
namespace internal {

template<typename Value_, typename Index_>
void trim_sparse(const tatami::SparseRange<Value_, Index_>& raw, std::vector<Value_>& output_v, std::vector<Index_>& output_i) {
    tatami::copy_n(raw.value, raw.number, output_v.data());
    output_v.resize(raw.number);
    tatami::copy_n(raw.index, raw.number, output_i.data());
    output_i.resize(raw.number);
}

}
/**
 * @endcond
 */

/**
 * @tparam Value_ Type of the data.
 * @tparam Index_ Integer type for the row/column index.
 *
 * @param ext An extractor object.
 * @param i Row/column index to extract.
 * @param number Number of elements to extract along the non-target dimension.
 *
 * @return Vector of length `number`, containing the extracted values from row/column `i`.
 */
template<typename Value_, typename Index_>
std::vector<Value_> fetch(tatami::MyopicDenseExtractor<Value_, Index_>& ext, Index_ i, size_t number) {
    std::vector<Value_> output(number);
    auto raw = ext.fetch(i, output.data());
    tatami::copy_n(raw, output.size(), output.data());
    return output;
}

/**
 * @tparam Value_ Type of the data.
 * @tparam Index_ Integer type for the row/column index.
 *
 * @param ext An extractor object.
 * @param number Number of elements to extract along the non-target dimension.
 *
 * @return Vector of length `number`, containing the extracted values from the next row/column. 
 */
template<typename Value_, typename Index_>
std::vector<Value_> fetch(tatami::OracularDenseExtractor<Value_, Index_>& ext, size_t number) {
    std::vector<Value_> output(number);
    auto raw = ext.fetch(output.data());
    tatami::copy_n(raw, output.size(), output.data());
    return output;
}

/**
 * @brief Sparse vector.
 * @tparam Value_ Type of the data.
 * @tparam Index_ Integer type for the row/column index.
 */
template<typename Value_, typename Index_>
struct SparseVector {
    /**
     * @cond
     */
    SparseVector(size_t n) : value(n), index(n) {}
    /**
     * @endcond
     */

    /**
     * Values of the non-zero elements.
     */
    std::vector<Value_> value;

    /**
     * Row/column indices of the non-zero elements.
     * This should have length equal to that of `SparseVector::value`, and entries should be sorted.
     */
    std::vector<Index_> index;
};

/**
 * @tparam Value_ Type of the data.
 * @tparam Index_ Integer type for the row/column index.
 *
 * @param ext An extractor object.
 * @param i Row/column index to extract.
 * @param number Number of elements to extract along the non-target dimension.
 *
 * @return Sparse vector containing up to `number` extracted values from row/column `i`.
 */
template<typename Value_, typename Index_>
SparseVector<Value_, Index_> fetch(tatami::MyopicSparseExtractor<Value_, Index_>& ext, Index_ i, size_t number) {
    SparseVector<Value_, Index_> output(number);
    auto raw = ext.fetch(i, output.value.data(), output.index.data());
    internal::trim_sparse(raw, output.value, output.index);
    return output;
}

/**
 * @tparam Value_ Type of the data.
 * @tparam Index_ Integer type for the row/column index.
 *
 * @param ext An extractor object.
 * @param number Number of elements to extract along the non-target dimension.
 *
 * @return Sparse vector containing up to `number` extracted values from the next row/column.
 */
template<typename Value_, typename Index_>
SparseVector<Value_, Index_> fetch(tatami::OracularSparseExtractor<Value_, Index_>& ext, size_t number) {
    SparseVector<Value_, Index_> output(number);
    auto raw = ext.fetch(output.value.data(), output.index.data());
    internal::trim_sparse(raw, output.value, output.index);
    return output;
}

}

#endif
