# Testing framework for tatami

![Unit tests](https://github.com/tatami-inc/tatami_test/actions/workflows/run-tests.yaml/badge.svg)
![Documentation](https://github.com/tatami-inc/tatami_test/actions/workflows/doxygenate.yaml/badge.svg)

## Overview

This library contains functions to test the other **tatami** libraries.
To include this in a project, just add the following chunk to the test-specific `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
  tatami_test
  GIT_REPOSITORY https://github.com/tatami-inc/tatami_test
  GIT_TAG master # or any version of interest
)

FetchContent_MakeAvailable(tatami_test)

target_link_libraries(mylib INTERFACE tatami_test)
```

This will automatically pull in [GoogleTest](https://github.com/google/googletest) via `FetchContent` so downstream projects don't have to do it themselves.
Then, to use the library, we just have to add the header:

```cpp
#include "tatami_tests/tatami_tests.hpp"
```

Check out the [reference documentation](https://tatami-inc.github.io/tatami_test/) for the available functionality.

## Simulating data

We can simulate random data easily: 

```cpp
tatami_test::SimulateVectorOptions opt;
opt.density = 0.5;
opt.lower = 5;
opt.upper = 100;
auto res = tatami_test::simulate_vector(100, opt);
```

We can also simulate compressed sparse data to quickly create the corresponding sparse matrices:

```cpp
auto sparse_res = tatami_test::simulate_compressed_sparse(
    /* primary = */ 100, 
    /* secondary = */ 500, 
    /* density = */ 0.1, 
    /* options = */ tatami_test::SimulateCompressedSparseOptions()
);
```

## Testing data access

The workhorses of this library are the various `test_access()` functions.
These test the extraction behavior of a `tatami::Matrix` subclass by comparing it to a reference representation.

```cpp
// reference representation:
auto dense = std::shared_ptr<tatami::Matrix<double, int> >(
    new tatami::DenseRowMatrix<double, int> >(20, 5, std::move(res)) 
);

// matrix to be tested:
auto sparse = tatami::convert_to_compressed_sparse(dense.get(), true); 

// test row access:
tatami_test::TestAccessOptions options;
tatami_test::test_access(*sparse, *dense, /* use_row = */ true, options);
```

We can also test access of a contiguous block on the non-target dimension:

```cpp
tatami_test::test_access(
    *sparse,
    *dense,
    /* use_row = */ false,
    /* block_start = */ 12,
    /* block_length = */ 57,
    options
);
```

Or a random sampling of indices:

```cpp
tatami_test::test_access(
    *sparse,
    *dense,
    /* use_row = */ true,
    tatami_test::define_non_target_indices(
        /* start index */ 10,
        /* upper bound */ 40,
        /* probability = */ 0.05,
        /* seed = */ 42 
    ),
    options
);
```

## Seed wrappers for delayed operations

For `tatami::Matrix` subclasses implementing delayed operations, we can test whether the operation correctly handles edge cases of seed behavior. 
For example, if the delayed operation needs to manipulate the indices during sparse extraction, can it handle seeds that might return unordered indices?
We can test this via by wrapping the seed in a `ReversedIndicesWrapper` before applying the delayed operation, and then using `test_unsorted_access()`:

```cpp
// First, wrapping the seed.
auto wrapped = std::make_shared<tatami_test::ReversedIndicesWrapper<double, int> >(sparse);

// Applying the delayed operation that we want to test.
std::vector<int> odds { 1, 3, 5, 7, 9 };
auto submat = tatami::make_DelayedSubset<double, int>(wrapped, odds, true);

// And now testing row access.
tatami_test::test_unsorted_access(*submat, true, options);
tatami_test::test_unsorted_access(*submat, true, 5, 30, options);
tatami_test::test_unsorted_access(
    *submat,
    true, 
    tatami_test::define_non_target_indices(5, 50, 0.1, {}),
    options
);
```

Another edge case is when a delayed operation subclass's oracular extraction uses different code paths based on whether the seeds benefit from an oracle.
We can test the subclass's with-oracle behavior by forcing one or more seeds to declare that, yes, they do benefit from an oracle:

```cpp
// First, wrapping the seed.
auto wrapped2 = std::make_shared<tatami_test::ForcedOracleWrapper<double, int> >(dense);

// Applying the delayed operation that we want to test.
auto bound = tatami::make_DelayedBind<double, int>({ wrapped2, sparse }, true);

// And now testing access against a reference implementation.
auto ref = tatami::make_DelayedBind<double, int>({ dense, sparse }, true);
tatami_test::test_access(*bound, *ref, false, options);
```

## Other useful things

We can check that errors are thrown with the expected message:

```cpp
tatami_test::throws_error([&]() {
    throw std::runtime_error("foobar");
}, "foobar");
```

We also provide a wrapper around the `fetch` + `copy_n` combination, which directly returns a vector of the desired row/column contents.

```cpp
auto ext = dense->dense_column();
auto column = fetch(*ext, 10, dense->nrow());
```
