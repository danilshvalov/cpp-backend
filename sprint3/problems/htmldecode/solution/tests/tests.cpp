#include <catch2/catch_test_macros.hpp>

#include "../src/htmldecode.h"

TEST_CASE("Text without mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode("") == "");
    CHECK(HtmlDecode("hello") == "hello");
}

TEST_CASE("Text with mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode("Johnson&amp;Johnson") == "Johnson&Johnson");
    CHECK(HtmlDecode("Johnson&ampJohnson") == "Johnson&Johnson");
    CHECK(HtmlDecode("Johnson&AMPJohnson") == "Johnson&Johnson");
    CHECK(HtmlDecode("Johnson&AMP;Johnson") == "Johnson&Johnson");
    CHECK(HtmlDecode("Johnson&Johnson") == "Johnson&Johnson");
    CHECK(HtmlDecode("M&amp;M&APOSs") == "M&M's");
}

TEST_CASE("Text with mnemonics in different places", "[HtmlDecode]") {
    CHECK(HtmlDecode("abc&amp;") == "abc&");
    CHECK(HtmlDecode("abc&amp;abc") == "abc&abc");
    CHECK(HtmlDecode("&amp;abc") == "&abc");
}

TEST_CASE("Text with incomplete mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode("&am") == "&am");
    CHECK(HtmlDecode("&l") == "&l");
    CHECK(HtmlDecode("&g") == "&g");
    CHECK(HtmlDecode("&apo") == "&apo");
    CHECK(HtmlDecode("&quo") == "&quo");
}

TEST_CASE("Text with pseudo-mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode("&abracadabra") == "&abracadabra");
    CHECK(HtmlDecode("&aPos;") == "&aPos;");
}
