#define BOOST_TEST_MODULE urlencode tests
#include <boost/test/unit_test.hpp>

#include "../src/urldecode.h"

BOOST_AUTO_TEST_CASE(UrlDecode_tests) {
    BOOST_TEST(UrlDecode("") == "");
    BOOST_TEST(UrlDecode("hello-world") == "hello-world");
    BOOST_TEST(UrlDecode("hello%20world") == "hello world");
    BOOST_TEST(
        UrlDecode(
            "%20%21%22%23%24%25%26%27%28%29%2a%2B%2c%2F%3a%3B%3d%3F%40%5b%5D"
        ) == " !\"#$%&'()*+,/:;=?@[]"
    );
    BOOST_TEST(UrlDecode("hello+world") == "hello world");

    BOOST_CHECK_THROW(UrlDecode("%"), std::invalid_argument);
    BOOST_CHECK_THROW(UrlDecode("%1"), std::invalid_argument);
    BOOST_CHECK_THROW(UrlDecode("%1Z"), std::invalid_argument);
    BOOST_CHECK_THROW(UrlDecode("%ZZ"), std::invalid_argument);
}
