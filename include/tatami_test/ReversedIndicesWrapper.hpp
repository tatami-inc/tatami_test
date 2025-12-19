#ifndef TATAMI_TEST_REVERSED_INDICES_WRAPPER_HPP
#define TATAMI_TEST_REVERSED_INDICES_WRAPPER_HPP

#include <algorithm>
#include <memory>

#include "tatami/base/Matrix.hpp"
#include "tatami/utils/copy.hpp"

/**
 * @file ReversedIndicesWrapper.hpp
 * @brief Reverse sparse indices during extraction.
 */

namespace tatami_test {

/**
 * @cond
 */
template<bool oracle_, typename Value_, typename Index_>
class ReversedIndicesExtractor final : public tatami::SparseExtractor<oracle_, Value_, Index_> {
public:
    ReversedIndicesExtractor(std::unique_ptr<tatami::SparseExtractor<oracle_, Value_, Index_> > host, const bool must_sort) : 
        my_host(std::move(host)), my_must_sort(must_sort) {}

#ifdef TATAMI_STRICT_SIGNATURES
    template<typename ... Args_>
    ReversedIndicesExtractor(std::unique_ptr<tatami::SparseExtractor<oracle_, Value_, Index_> >, Args_...) = delete;
#endif

private:
    std::unique_ptr<tatami::SparseExtractor<oracle_, Value_, Index_> > my_host;
    bool my_must_sort;

public:
    tatami::SparseRange<Value_, Index_> fetch(Index_ i, Value_* vbuffer, Index_* ibuffer) {
        auto range = my_host->fetch(i, vbuffer, ibuffer);
        if (!my_must_sort) {
            if (range.value) {
                tatami::copy_n(range.value, range.number, vbuffer);
                std::reverse(vbuffer, vbuffer + range.number);
                range.value = vbuffer;
            }
            if (range.index) {
                tatami::copy_n(range.index, range.number, ibuffer);
                std::reverse(ibuffer, ibuffer + range.number);
                range.index = ibuffer;
            }
        }
        return range;
    }
};
/**
 * @endcond
 */

/**
 * @brief Reverse indices for sparse extraction.
 * @tparam Value_ Type of matrix value.
 * @tparam Index_ Integer type for the row/column indices.
 *
 * This wrapper reverses the ordering of non-zero elements when `tatami::Options::sparse_ordered_index = false`.
 * The aim is to enable testing of `tatami::Matrix` subclasses that implement delayed operations on an existing "seed" matrix,
 * where we may wish to check that the delayed operation handles unordered sparse extraction correctly.
 * To do so, we wrap the seed in a `ReversedIndicesWrapper`, which is then used in the delayed operation;
 * the resulting matrix can then be tested with `test_unsorted_full_access()` and friends.
 */
template<typename Value_, typename Index_>
class ReversedIndicesWrapper final : public tatami::Matrix<Value_, Index_> {
public:
    /**
     * @param matrix Pointer to a `tatami::Matrix`.
     * This is typically the seed matrix that would otherwise be directly used in a delayed operation.
     */
    ReversedIndicesWrapper(std::shared_ptr<const tatami::Matrix<Value_, Index_> > matrix) : my_matrix(std::move(matrix)) {}

    /**
     * @cond
     */
#ifdef TATAMI_STRICT_SIGNATURES
    // Not much to do here, we want to accept pointers to subclasses so casts are okay.
#endif
    /**
     * @endcond
     */

private:
    std::shared_ptr<const tatami::Matrix<Value_, Index_> > my_matrix;

public:
    Index_ nrow() const {
        return my_matrix->nrow();
    }

    Index_ ncol() const {
        return my_matrix->ncol();
    }

    bool is_sparse() const {
        return my_matrix->is_sparse();
    }

    double is_sparse_proportion() const {
        return my_matrix->is_sparse_proportion();
    }

    bool prefer_rows() const {
        return my_matrix->prefer_rows();
    }

    double prefer_rows_proportion() const {
        return my_matrix->prefer_rows_proportion();
    }

    bool uses_oracle(const bool row) const {
        return my_matrix->uses_oracle(row);
    }

public:
    std::unique_ptr<tatami::MyopicDenseExtractor<Value_, Index_> > dense(
        const bool row,
        const tatami::Options& opt
    ) const { 
        return my_matrix->dense(row, opt); 
    }

    std::unique_ptr<tatami::MyopicDenseExtractor<Value_, Index_> > dense(
        const bool row,
        const Index_ bs,
        const Index_ bl,
        const tatami::Options& opt
    ) const {
        return my_matrix->dense(row, bs, bl, opt);
    }

    std::unique_ptr<tatami::MyopicDenseExtractor<Value_, Index_> > dense(
        const bool row,
        tatami::VectorPtr<Index_> idx,
        const tatami::Options& opt
    ) const {
        return my_matrix->dense(row, std::move(idx), opt);
    }

public:
    std::unique_ptr<tatami::MyopicSparseExtractor<Value_, Index_> > sparse(
        const bool row,
        const tatami::Options& opt
    ) const { 
        return std::make_unique<ReversedIndicesExtractor<false, Value_, Index_> >(my_matrix->sparse(row, opt), opt.sparse_ordered_index); 
    }

    std::unique_ptr<tatami::MyopicSparseExtractor<Value_, Index_> > sparse(
        const bool row,
        const Index_ bs,
        const Index_ bl,
        const tatami::Options& opt
    ) const {
        return std::make_unique<ReversedIndicesExtractor<false, Value_, Index_> >(my_matrix->sparse(row, bs, bl, opt), opt.sparse_ordered_index);
    }

    std::unique_ptr<tatami::MyopicSparseExtractor<Value_, Index_> > sparse(
        const bool row,
        tatami::VectorPtr<Index_> idx,
        const tatami::Options& opt
    ) const {
        return std::make_unique<ReversedIndicesExtractor<false, Value_, Index_> >(my_matrix->sparse(row, std::move(idx), opt), opt.sparse_ordered_index);
    }

public:
    std::unique_ptr<tatami::OracularDenseExtractor<Value_, Index_> > dense(
        const bool row,
        std::shared_ptr<const tatami::Oracle<Index_> > ora,
        const tatami::Options& opt
    ) const { 
        return my_matrix->dense(row, std::move(ora), opt); 
    }

    std::unique_ptr<tatami::OracularDenseExtractor<Value_, Index_> > dense(
        const bool row,
        std::shared_ptr<const tatami::Oracle<Index_> > ora,
        const Index_ bs,
        const Index_ bl,
        const tatami::Options& opt
    ) const {
        return my_matrix->dense(row, std::move(ora), bs, bl, opt);
    }

    std::unique_ptr<tatami::OracularDenseExtractor<Value_, Index_> > dense(
        const bool row,
        std::shared_ptr<const tatami::Oracle<Index_> > ora,
        tatami::VectorPtr<Index_> idx,
        const tatami::Options& opt
    ) const {
        return my_matrix->dense(row, std::move(ora), std::move(idx), opt);
    }

public:
    std::unique_ptr<tatami::OracularSparseExtractor<Value_, Index_> > sparse(
        const bool row,
        std::shared_ptr<const tatami::Oracle<Index_> > ora,
        const tatami::Options& opt
    ) const { 
        return std::make_unique<ReversedIndicesExtractor<true, Value_, Index_> >(my_matrix->sparse(row, std::move(ora), opt), opt.sparse_ordered_index);
    }

    std::unique_ptr<tatami::OracularSparseExtractor<Value_, Index_> > sparse(
        const bool row,
        std::shared_ptr<const tatami::Oracle<Index_> > ora,
        const Index_ bs,
        const Index_ bl,
        const tatami::Options& opt
    ) const {
        return std::make_unique<ReversedIndicesExtractor<true, Value_, Index_> >(my_matrix->sparse(row, std::move(ora), bs, bl, opt), opt.sparse_ordered_index);
    }

    std::unique_ptr<tatami::OracularSparseExtractor<Value_, Index_> > sparse(
        const bool row,
        std::shared_ptr<const tatami::Oracle<Index_> > ora,
        tatami::VectorPtr<Index_> idx,
        const tatami::Options& opt
    ) const {
        return std::make_unique<ReversedIndicesExtractor<true, Value_, Index_> >(my_matrix->sparse(row, std::move(ora), std::move(idx), opt), opt.sparse_ordered_index);
    }
};

}

#endif
