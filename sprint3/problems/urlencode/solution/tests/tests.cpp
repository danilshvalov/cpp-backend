#include <gtest/gtest.h>

#include "../src/urlencode.h"

TEST(UrlEncodeTestSuite, EmptyString) {
    EXPECT_EQ(UrlEncode(""), "");
}

TEST(UrlEncodeTestSuite, OrdinaryCharsAreNotEncoded) {
    EXPECT_EQ(UrlEncode("hello"), "hello");
}

TEST(UrlEncodeTestSuite, ReservedChars) {
    EXPECT_EQ(
        UrlEncode("!\"#$%&'()*+,/:;=?@[]"),
        "%21%22%23%24%25%26%27%28%29%2a%2b%2c%2f%3a%3b%3d%3f%40%5b%5d"
    );
}

TEST(UrlEncodeTestSuite, StringWithSpaces) {
    EXPECT_EQ(UrlEncode("Hello World"), "Hello+World");
}

TEST(UrlEncodeTestSuite, NonAlphabetSymbols) {
    EXPECT_EQ(UrlEncode("\1\2\3\4\5"), "%01%02%03%04%05");
}
