/**
 * @file UriTests.cpp
 *
 * This module contains the unit tests of the Uri::Uri class.
 *
 * © 2018 by Richard Walters
 */

#include <gtest/gtest.h>
#include <stddef.h>
#include <Uri/Uri.hpp>

TEST(UriTests, ParseFromStringNoScheme) {
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("foo/bar"));
    ASSERT_EQ("", uri.GetScheme());
    ASSERT_EQ(
        (std::vector< std::string >{
            "foo",
            "bar",
        }),
        uri.GetPath()
    );
}

TEST(UriTests, ParseFromStringUrl) {
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("http://www.example.com/foo/bar"));
    ASSERT_EQ("http", uri.GetScheme());
    ASSERT_EQ("www.example.com", uri.GetHost());
    ASSERT_EQ(
        (std::vector< std::string >{
            "",
            "foo",
            "bar",
        }),
        uri.GetPath()
    );
}

TEST(UriTests, ParseFromStringUrnDefaultPathDelimiter) {
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("urn:book:fantasy:Hobbit"));
    ASSERT_EQ("urn", uri.GetScheme());
    ASSERT_EQ("", uri.GetHost());
    ASSERT_EQ(
        (std::vector< std::string >{
            "book:fantasy:Hobbit",
        }),
        uri.GetPath()
    );
}

TEST(UriTests, ParseFromStringPathCornerCases) {
    struct TestVector {
        std::string pathIn;
        std::vector< std::string > pathOut;
    };
    const std::vector< TestVector > testVectors{
        {"", {}},
        {"/", {""}},
        {"/foo", {"", "foo"} },
        {"foo/", {"foo", ""} },
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.pathIn)) << index;
        ASSERT_EQ(testVector.pathOut, uri.GetPath()) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringHasAPortNumber) {
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("http://www.example.com:8080/foo/bar"));
    ASSERT_EQ("www.example.com", uri.GetHost());
    ASSERT_TRUE(uri.HasPort());
    ASSERT_EQ(8080, uri.GetPort());
}

TEST(UriTests, ParseFromStringDoesNotHaveAPortNumber) {
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("http://www.example.com/foo/bar"));
    ASSERT_EQ("www.example.com", uri.GetHost());
    ASSERT_FALSE(uri.HasPort());
}

TEST(UriTests, ParseFromStringTwiceFirstWithPortNumberThenWithout) {
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("http://www.example.com:8080/foo/bar"));
    ASSERT_TRUE(uri.ParseFromString("http://www.example.com/foo/bar"));
    ASSERT_FALSE(uri.HasPort());
}

TEST(UriTests, ParseFromStringBadPortNumberPurelyAlphabetic) {
    Uri::Uri uri;
    ASSERT_FALSE(uri.ParseFromString("http://www.example.com:spam/foo/bar"));
}

TEST(UriTests, ParseFromStringBadPortNumberStartsNumericEndsAlphabetic) {
    Uri::Uri uri;
    ASSERT_FALSE(uri.ParseFromString("http://www.example.com:8080spam/foo/bar"));
}

TEST(UriTests, ParseFromStringLargestValidPortNumber) {
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("http://www.example.com:65535/foo/bar"));
    ASSERT_TRUE(uri.HasPort());
    ASSERT_EQ(65535, uri.GetPort());
}

TEST(UriTests, ParseFromStringBadPortNumberTooBig) {
    Uri::Uri uri;
    ASSERT_FALSE(uri.ParseFromString("http://www.example.com:65536/foo/bar"));
}

TEST(UriTests, ParseFromStringBadPortNumberNegative) {
    Uri::Uri uri;
    ASSERT_FALSE(uri.ParseFromString("http://www.example.com:-1234/foo/bar"));
}

TEST(UriTests, ParseFromStringEndsAfterAuthority) {
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("http://www.example.com"));
}

TEST(UriTests, ParseFromStringRelativeVsNonRelativeReferences) {
    struct TestVector {
        std::string uriString;
        bool isRelativeReference;
    };
    const std::vector< TestVector > testVectors{
        {"http://www.example.com/", false},
        {"http://www.example.com", false},
        {"/", true},
        {"foo", true},
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.uriString)) << index;
        ASSERT_EQ(testVector.isRelativeReference, uri.IsRelativeReference()) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringRelativeVsNonRelativePaths) {
    struct TestVector {
        std::string uriString;
        bool containsRelativePath;
    };
    const std::vector< TestVector > testVectors{
        {"http://www.example.com/", false},
        {"http://www.example.com", true},
        {"/", false},
        {"foo", true},

        /*
         * This is only a valid test vector if we understand
         * correctly that an empty string IS a valid
         * "relative reference" URI with an empty path.
         */
        {"", true},
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.uriString)) << index;
        ASSERT_EQ(testVector.containsRelativePath, uri.ContainsRelativePath()) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringQueryAndFragmentElements) {
    struct TestVector {
        std::string uriString;
        std::string host;
        std::string query;
        std::string fragment;
    };
    const std::vector< TestVector > testVectors{
        {"http://www.example.com/", "www.example.com", "", ""},
        {"http://example.com?foo", "example.com", "foo", ""},
        {"http://www.example.com#foo", "www.example.com", "", "foo"},
        {"http://www.example.com?foo#bar", "www.example.com", "foo", "bar"},
        {"http://www.example.com?earth?day#bar", "www.example.com", "earth?day", "bar"},
        {"http://www.example.com/spam?foo#bar", "www.example.com", "foo", "bar"},

        /*
         * NOTE: curiously, but we think this is correct, that
         * having a trailing question mark is equivalent to not having
         * any question mark, because in both cases, the query element
         * is empty string.  Perhaps research deeper to see if this is right.
         */
        {"http://www.example.com/?", "www.example.com", "", ""},
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.uriString)) << index;
        ASSERT_EQ(testVector.host, uri.GetHost()) << index;
        ASSERT_EQ(testVector.query, uri.GetQuery()) << index;
        ASSERT_EQ(testVector.fragment, uri.GetFragment()) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringUserInfo) {
    struct TestVector {
        std::string uriString;
        std::string userInfo;
    };
    const std::vector< TestVector > testVectors{
        {"http://www.example.com/", ""},
        {"http://joe@www.example.com", "joe"},
        {"http://pepe:feelsbadman@www.example.com", "pepe:feelsbadman"},
        {"//www.example.com", ""},
        {"//bob@www.example.com", "bob"},
        {"/", ""},
        {"foo", ""},
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.uriString)) << index;
        ASSERT_EQ(testVector.userInfo, uri.GetUserInfo()) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringTwiceFirstUserInfoThenWithout) {
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("http://joe@www.example.com/foo/bar"));
    ASSERT_TRUE(uri.ParseFromString("/foo/bar"));
    ASSERT_TRUE(uri.GetUserInfo().empty());
}

TEST(UriTests, ParseFromStringSchemeIllegalCharacters) {
    const std::vector< std::string > testVectors{
        {"://www.example.com/"},
        {"0://www.example.com/"},
        {"+://www.example.com/"},
        {"@://www.example.com/"},
        {".://www.example.com/"},
        {"h@://www.example.com/"},
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_FALSE(uri.ParseFromString(testVector)) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringSchemeBarelyLegal) {
    struct TestVector {
        std::string uriString;
        std::string scheme;
    };
    const std::vector< TestVector > testVectors{
        {"h://www.example.com/", "h"},
        {"x+://www.example.com/", "x+"},
        {"y-://www.example.com/", "y-"},
        {"z.://www.example.com/", "z."},
        {"aa://www.example.com/", "aa"},
        {"a0://www.example.com/", "a0"},
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.uriString)) << index;
        ASSERT_EQ(testVector.scheme, uri.GetScheme());
        ++index;
    }
}

TEST(UriTests, ParseFromStringSchemeMixedCase) {
    const std::vector< std::string > testVectors{
        {"http://www.example.com/"},
        {"hTtp://www.example.com/"},
        {"HTTP://www.example.com/"},
        {"Http://www.example.com/"},
        {"HttP://www.example.com/"},
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector)) << index;
        ASSERT_EQ("http", uri.GetScheme()) << ">>> Failed for test vector element " << index << " <<<";
        ++index;
    }
}

TEST(UriTests, ParseFromStringUserInfoIllegalCharacters) {
    const std::vector< std::string > testVectors{
        {"//%X@www.example.com/"},
        {"//{@www.example.com/"},
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_FALSE(uri.ParseFromString(testVector)) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringUserInfoBarelyLegal) {
    struct TestVector {
        std::string uriString;
        std::string userInfo;
    };
    const std::vector< TestVector > testVectors{
        {"//%41@www.example.com/", "A"},
        {"//@www.example.com/", ""},
        {"//!@www.example.com/", "!"},
        {"//'@www.example.com/", "'"},
        {"//(@www.example.com/", "("},
        {"//;@www.example.com/", ";"},
        {"http://:@www.example.com/", ":"},
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.uriString)) << index;
        ASSERT_EQ(testVector.userInfo, uri.GetUserInfo());
        ++index;
    }
}

TEST(UriTests, ParseFromStringHostIllegalCharacters) {
    const std::vector< std::string > testVectors{
        {"//%X@www.example.com/"},
        {"//@www:example.com/"},
        {"//[vX.:]/"},
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_FALSE(uri.ParseFromString(testVector)) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringHostBarelyLegal) {
    struct TestVector {
        std::string uriString;
        std::string host;
    };
    const std::vector< TestVector > testVectors{
        {"//%41/", "a"},
        {"///", ""},
        {"//!/", "!"},
        {"//'/", "'"},
        {"//(/", "("},
        {"//;/", ";"},
        {"//1.2.3.4/", "1.2.3.4"},
        {"//[v7.:]/", "[v7.:]"},
        {"//[v7.aB]/", "[v7.aB]"},
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.uriString)) << index;
        ASSERT_EQ(testVector.host, uri.GetHost());
        ++index;
    }
}

TEST(UriTests, ParseFromStringHostMixedCase) {
    const std::vector< std::string > testVectors{
        {"http://www.example.com/"},
        {"http://www.EXAMPLE.com/"},
        {"http://www.exAMple.com/"},
        {"http://www.example.cOM/"},
        {"http://wWw.exampLe.Com/"},
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector)) << index;
        ASSERT_EQ("www.example.com", uri.GetHost()) << ">>> Failed for test vector element " << index << " <<<";
        ++index;
    }
}

TEST(UriTests, ParseFromStringDontMisinterpretColonInOtherPlacesAsSchemeDelimiter) {
    const std::vector< std::string > testVectors{
        {"//foo:bar@www.example.com/"},
        {"//www.example.com/a:b"},
        {"//www.example.com/foo?a:b"},
        {"//www.example.com/foo#a:b"},
        {"//[v7.:]/"},
        {"/:/foo"},
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector)) << index;
        ASSERT_TRUE(uri.GetScheme().empty());
        ++index;
    }
}

TEST(UriTests, ParseFromStringPathIllegalCharacters) {
    const std::vector< std::string > testVectors{
        {"http://www.example.com/foo[bar"},
        {"http://www.example.com/]bar"},
        {"http://www.example.com/foo]"},
        {"http://www.example.com/["},
        {"http://www.example.com/abc/foo]"},
        {"http://www.example.com/abc/["},
        {"http://www.example.com/foo]/abc"},
        {"http://www.example.com/[/abc"},
        {"http://www.example.com/foo]/"},
        {"http://www.example.com/[/"},
        {"/foo[bar"},
        {"/]bar"},
        {"/foo]"},
        {"/["},
        {"/abc/foo]"},
        {"/abc/["},
        {"/foo]/abc"},
        {"/[/abc"},
        {"/foo]/"},
        {"/[/"},
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_FALSE(uri.ParseFromString(testVector)) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringPathBarelyLegal) {
    struct TestVector {
        std::string uriString;
        std::vector< std::string > path;
    };
    const std::vector< TestVector > testVectors{
        {"/:/foo", {"", ":", "foo"}},
        {"bob@/foo", {"bob@", "foo"}},
        {"hello!", {"hello!"}},
        {"urn:hello,%20w%6Frld", {"hello, world"}},
        {"//example.com/foo/(bar)/", {"", "foo", "(bar)", ""}},
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.uriString)) << index;
        ASSERT_EQ(testVector.path, uri.GetPath());
        ++index;
    }
}

TEST(UriTests, ParseFromStringQueryIllegalCharacters) {
    const std::vector< std::string > testVectors{
        {"http://www.example.com/?foo[bar"},
        {"http://www.example.com/?]bar"},
        {"http://www.example.com/?foo]"},
        {"http://www.example.com/?["},
        {"http://www.example.com/?abc/foo]"},
        {"http://www.example.com/?abc/["},
        {"http://www.example.com/?foo]/abc"},
        {"http://www.example.com/?[/abc"},
        {"http://www.example.com/?foo]/"},
        {"http://www.example.com/?[/"},
        {"?foo[bar"},
        {"?]bar"},
        {"?foo]"},
        {"?["},
        {"?abc/foo]"},
        {"?abc/["},
        {"?foo]/abc"},
        {"?[/abc"},
        {"?foo]/"},
        {"?[/"},
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_FALSE(uri.ParseFromString(testVector)) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringQueryBarelyLegal) {
    struct TestVector {
        std::string uriString;
        std::string query;
    };
    const std::vector< TestVector > testVectors{
        {"/?:/foo", ":/foo"},
        {"?bob@/foo", "bob@/foo"},
        {"?hello!", "hello!"},
        {"urn:?hello,%20w%6Frld", "hello, world"},
        {"//example.com/foo?(bar)/", "(bar)/"},
        {"http://www.example.com/?foo?bar", "foo?bar" },
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.uriString)) << index;
        ASSERT_EQ(testVector.query, uri.GetQuery());
        ++index;
    }
}

TEST(UriTests, ParseFromStringFragmentIllegalCharacters) {
    const std::vector< std::string > testVectors{
        {"http://www.example.com/#foo[bar"},
        {"http://www.example.com/#]bar"},
        {"http://www.example.com/#foo]"},
        {"http://www.example.com/#["},
        {"http://www.example.com/#abc/foo]"},
        {"http://www.example.com/#abc/["},
        {"http://www.example.com/#foo]/abc"},
        {"http://www.example.com/#[/abc"},
        {"http://www.example.com/#foo]/"},
        {"http://www.example.com/#[/"},
        {"#foo[bar"},
        {"#]bar"},
        {"#foo]"},
        {"#["},
        {"#abc/foo]"},
        {"#abc/["},
        {"#foo]/abc"},
        {"#[/abc"},
        {"#foo]/"},
        {"#[/"},
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_FALSE(uri.ParseFromString(testVector)) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringFragmentBarelyLegal) {
    struct TestVector {
        std::string uriString;
        std::string fragment;
    };
    const std::vector< TestVector > testVectors{
        {"/#:/foo", ":/foo"},
        {"#bob@/foo", "bob@/foo"},
        {"#hello!", "hello!"},
        {"urn:#hello,%20w%6Frld", "hello, world"},
        {"//example.com/foo#(bar)/", "(bar)/"},
        {"http://www.example.com/#foo?bar", "foo?bar" },
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.uriString)) << index;
        ASSERT_EQ(testVector.fragment, uri.GetFragment());
        ++index;
    }
}

TEST(UriTests, ParseFromStringPathsWithPercentEncodedCharacters) {
    struct TestVector {
        std::string uriString;
        std::string pathFirstSegment;
    };
    const std::vector< TestVector > testVectors{
        {"%41", "A"},
        {"%4A", "J"},
        {"%4a", "J"},
        {"%bc", "\xbc"},
        {"%Bc", "\xbc"},
        {"%bC", "\xbc"},
        {"%BC", "\xbc"},
        {"%41%42%43", "ABC"},
        {"%41%4A%43%4b", "AJCK"},
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.uriString)) << index;
        ASSERT_EQ(testVector.pathFirstSegment, uri.GetPath()[0]);
        ++index;
    }
}

TEST(UriTests, NormalizePath) {
    struct TestVector {
        std::string uriString;
        std::vector< std::string > normalizedPathSegments;
    };
    const std::vector< TestVector > testVectors{
        {"/a/b/c/./../../g", {"", "a", "g"}},
        {"mid/content=5/../6", {"mid", "6"}},
        {"http://example.com/a/../b", {"", "b"}},
        {"http://example.com/../b", {"", "b"}},
        {"http://example.com/a/../b", {"", "b"}},
        {"http://example.com/a/../../b", {"", "b"}},
        {"./a/b", {"a", "b"}},
        {"..", {}},
        {"/", {""}},
        {"a/b/..", {"a"}},
        {"a/b/.", {"a", "b"}},
        {"a/b/./c", {"a", "b", "c"}},
        {"a/b/./c/", {"a", "b", "c", ""}},
        {"/a/b/..", {"", "a"}},
        {"/a/b/.", {"", "a", "b"}},
        {"/a/b/./c", {"", "a", "b", "c"}},
        {"/a/b/./c/", {"", "a", "b", "c", ""}},
        {"./a/b/..", {"a"}},
        {"./a/b/.", {"a", "b"}},
        {"./a/b/./c", {"a", "b", "c"}},
        {"./a/b/./c/", {"a", "b", "c", ""}},
        {"../a/b/..", {"a"}},
        {"../a/b/.", {"a", "b"}},
        {"../a/b/./c", {"a", "b", "c"}},
        {"../a/b/./c/", {"a", "b", "c", ""}},
        {"../a/b/../c", {"a", "c"}},
        {"../a/b/./../c/", {"a", "c", ""}},
        {"../a/b/./../c", {"a", "c"}},
        {"../a/b/./../c/", {"a", "c", ""}},
        {"../a/b/.././c/", {"a", "c", ""}},
        {"../a/b/.././c", {"a", "c"}},
        {"../a/b/.././c/", {"a", "c", ""}},
        {"/./c/d", {"", "c", "d"}},
        {"/../c/d", {"", "c", "d"}},
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.uriString)) << index;
        uri.NormalizePath();
        ASSERT_EQ(testVector.normalizedPathSegments, uri.GetPath()) << index;
        ++index;
    }
}

TEST(UriTests, ConstructNormalizeAndCompareEquivalentUris) {
    // This was inspired by section 6.2.2
    // of RFC 3986 (https://tools.ietf.org/html/rfc3986).
    Uri::Uri uri1, uri2;
    ASSERT_TRUE(uri1.ParseFromString("example://a/b/c/%7Bfoo%7D"));
    ASSERT_TRUE(uri2.ParseFromString("eXAMPLE://a/./b/../b/%63/%7bfoo%7d"));
    ASSERT_NE(uri1, uri2);
    uri2.NormalizePath();
    ASSERT_EQ(uri1, uri2);
}
