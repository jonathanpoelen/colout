#pragma once

#include <boost/test/auto_unit_test.hpp>

#define BOOST_CHECK_EQ BOOST_CHECK_EQUAL
#define BOOST_REQUIRE_EQ BOOST_REQUIRE_EQUAL

#ifdef IN_IDE_PARSER
# define FIXTURES_PATH "./tests/fixtures"
# define CFG_PATH "./sys/etc/rdpproxy"
# define BOOST_CHECK_NO_THROW(stmt) do { stmt; } while (0)
# define BOOST_CHECK_THROW(stmt, exception) do { \
    stmt; [](exception) {};                      \
  } while (0)
# define BOOST_CHECK_EXCEPTION(stmt, exception, predicate) do { \
    stmt; [](exception & e) { predicate(e); };                  \
  } while (0)
# define BOOST_CHECK_EQUAL(a, b) (a) == (b)
# define BOOST_CHECK_NE(a, b) (a) != (b)
# define BOOST_CHECK_LT(a, b) (a) < (b)
# define BOOST_CHECK_LE(a, b) (a) <= (b)
# define BOOST_CHECK_GT(a, b) (a) > (b)
# define BOOST_CHECK_GE(a, b) (a) >= (b)
# define BOOST_CHECK(a) (a)
# define BOOST_CHECK_MESSAGE(a, iostream_expr) (a), ""
# define BOOST_CHECK_EQUAL_RANGES(a, b) (a) != (b)

# define BOOST_REQUIRE_NO_THROW(stmt) do { stmt; } while (0)
# define BOOST_REQUIRE_THROW(stmt, exception) do { \
    stmt; [](exception) {};                        \
  } while (0)
# define BOOST_REQUIRE_EXCEPTION(stmt, exception, predicate) do { \
    stmt; [](exception & e) { predicate(e); };                    \
  } while (0)
# define BOOST_REQUIRE_EQUAL(a, b) (a) == (b)
# define BOOST_REQUIRE_NE(a, b) (a) != (b)
# define BOOST_REQUIRE_LT(a, b) (a) < (b)
# define BOOST_REQUIRE_LE(a, b) (a) <= (b)
# define BOOST_REQUIRE_GT(a, b) (a) > (b)
# define BOOST_REQUIRE_GE(a, b) (a) >= (b)
# define BOOST_REQUIRE(a) (a)
# define BOOST_REQUIRE_MESSAGE(a, iostream_expr) (a), ""
# define BOOST_REQUIRE_EQUAL_RANGES(a, b) (a) != (b)
#else
# define BOOST_CHECK_EQUAL_RANGES(a_, b_)           \
  do {                                              \
    auto const & A__CHECK_RANGES = a_;              \
    auto const & B__CHECK_RANGES = b_;              \
    using std::begin;                               \
    using std::end;                                 \
    BOOST_CHECK_EQUAL_COLLECTIONS(                  \
      begin(A__CHECK_RANGES), end(A__CHECK_RANGES), \
      begin(B__CHECK_RANGES), end(B__CHECK_RANGES)  \
    );                                              \
  } while (0)
# define BOOST_REQUIRE_EQUAL_RANGES(a_, b_)         \
  do {                                              \
    auto const & A__CHECK_RANGES = a_;              \
    auto const & B__CHECK_RANGES = b_;              \
    using std::begin;                               \
    using std::end;                                 \
    BOOST_REQUIRE_EQUAL_COLLECTIONS(                \
      begin(A__CHECK_RANGES), end(A__CHECK_RANGES), \
      begin(B__CHECK_RANGES), end(B__CHECK_RANGES)  \
    );                                              \
  } while (0)
#endif

// force line to last checkpoint
#ifndef IN_IDE_PARSER
# undef BOOST_AUTO_TEST_CASE
# define BOOST_AUTO_TEST_CASE(test_name)             \
  BOOST_AUTO_TEST_CASE_NO_DECOR(test_name##_start__) \
  { BOOST_CHECK(true); }                             \
  BOOST_AUTO_TEST_CASE_NO_DECOR(test_name)
#endif
