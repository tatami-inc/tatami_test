#ifndef TATAMI_TEST_THROWS_ERROR_HPP
#define TATAMI_TEST_THROWS_ERROR_HPP

#include <string>
#include <gtest/gtest.h>

/**
 * @file throws_error.hpp
 * @brief Check that an error is thrown.
 */

namespace tatami_test {

/**
 * Checks that an exception is thrown with the expected message.
 *
 * @tparam Function_ A function that takes no arguments.
 * 
 * @param fun A function that may or may not throw an exception.
 * @param msg Any part of the expected error message.
 */
template<class Function_>
void throws_error(Function_ fun, const std::string& msg) {
    try {
        fun();
        FAIL() << "expected error message '" << msg << "', got no error";
    } catch (std::exception& e) {
        std::string observed(e.what());
        if (observed.find(msg) == std::string::npos) {
            FAIL() << "expected error message '" << msg << "', got '" << observed << "'";
        }
    }
}

}

#endif
