#ifndef TATAMI_TEST_FORCED_ORACLE_WRAPPER_HPP
#define TATAMI_TEST_FORCED_ORACLE_WRAPPER_HPP

#include "tatami/base/Matrix.hpp"
#include "tatami/utils/copy.hpp"

#include <algorithm>
#include <memory>

/**
 * @file ForcedOracleWrapper.hpp
 * @brief Forcibly use oracular extraction.
 */

namespace tatami_test {

/**
 * @brief Force oracular extraction.
 * @tparam Value_ Type of matrix value.
 * @tparam Index_ Integer type for the row/column indices.
 *
 * This wrapper ensures that `tatami::Matrix::uses_oracle()` always returns true.
 * The aim is to enable testing of `tatami::Matrix` subclasses that implement delayed operations on an existing "seed" matrix,
 * where the subclass changes its behavior depending on the return value of `tatami::Matrix::uses_oracle()`.
 * By forcing this to be true, we can check all possible behaviors in `test_full_access()` and friends. 
 */
template<typename Value_, typename Index_>
class ForcedOracleWrapper final : public tatami::Matrix<Value_, Index_> {
public:
    /**
     * @param matrix Pointer to a `tatami::Matrix`.
     * This is typically the seed matrix that would otherwise be directly used in a delayed operation.
     */
    ForcedOracleWrapper(std::shared_ptr<const tatami::Matrix<Value_, Index_> > matrix) : my_matrix(std::move(matrix)) {}

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

    bool uses_oracle(bool) const {
        return true;
    }

public:
    std::unique_ptr<tatami::MyopicDenseExtractor<Value_, Index_> > dense(bool row, const tatami::Options& opt) const { 
        return my_matrix->dense(row, opt); 
    }

    std::unique_ptr<tatami::MyopicDenseExtractor<Value_, Index_> > dense(bool row, Index_ bs, Index_ bl, const tatami::Options& opt) const {
        return my_matrix->dense(row, bs, bl, opt);
    }

    std::unique_ptr<tatami::MyopicDenseExtractor<Value_, Index_> > dense(bool row, tatami::VectorPtr<Index_> idx, const tatami::Options& opt) const {
        return my_matrix->dense(row, std::move(idx), opt);
    }

public:
    std::unique_ptr<tatami::MyopicSparseExtractor<Value_, Index_> > sparse(bool row, const tatami::Options& opt) const { 
        return my_matrix->sparse(row, opt); 
    }

    std::unique_ptr<tatami::MyopicSparseExtractor<Value_, Index_> > sparse(bool row, Index_ bs, Index_ bl, const tatami::Options& opt) const {
        return my_matrix->sparse(row, bs, bl, opt);
    }

    std::unique_ptr<tatami::MyopicSparseExtractor<Value_, Index_> > sparse(bool row, tatami::VectorPtr<Index_> idx, const tatami::Options& opt) const {
        return my_matrix->sparse(row, std::move(idx), opt);
    }

public:
    std::unique_ptr<tatami::OracularDenseExtractor<Value_, Index_> > dense(bool row, std::shared_ptr<const tatami::Oracle<Index_> > ora, const tatami::Options& opt) const { 
        return my_matrix->dense(row, std::move(ora), opt); 
    }

    std::unique_ptr<tatami::OracularDenseExtractor<Value_, Index_> > dense(bool row, std::shared_ptr<const tatami::Oracle<Index_> > ora, Index_ bs, Index_ bl, const tatami::Options& opt) const {
        return my_matrix->dense(row, std::move(ora), bs, bl, opt);
    }

    std::unique_ptr<tatami::OracularDenseExtractor<Value_, Index_> > dense(bool row, std::shared_ptr<const tatami::Oracle<Index_> > ora, tatami::VectorPtr<Index_> idx, const tatami::Options& opt) const {
        return my_matrix->dense(row, std::move(ora), std::move(idx), opt);
    }

public:
    std::unique_ptr<tatami::OracularSparseExtractor<Value_, Index_> > sparse(bool row, std::shared_ptr<const tatami::Oracle<Index_> > ora, const tatami::Options& opt) const { 
        return my_matrix->sparse(row, std::move(ora), opt); 
    }

    std::unique_ptr<tatami::OracularSparseExtractor<Value_, Index_> > sparse(bool row, std::shared_ptr<const tatami::Oracle<Index_> > ora, Index_ bs, Index_ bl, const tatami::Options& opt) const {
        return my_matrix->sparse(row, std::move(ora), bs, bl, opt);
    }

    std::unique_ptr<tatami::OracularSparseExtractor<Value_, Index_> > sparse(bool row, std::shared_ptr<const tatami::Oracle<Index_> > ora, tatami::VectorPtr<Index_> idx, const tatami::Options& opt) const {
        return my_matrix->sparse(row, std::move(ora), std::move(idx), opt);
    }
};

}

#endif
