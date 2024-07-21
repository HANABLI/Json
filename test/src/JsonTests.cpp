/**
 * @file JsonTests.cpp
 * 
 * This module contains the unit test of the
 * Json::Json class.
 * 
 * © 2024 by Hatem Nabli
 */

#include <math.h>
#include <gtest/gtest.h>
#include <Json/Json.hpp>

TEST(JsonTests, JsonTests_FromNull_Test) {
    Json::Json json(nullptr);
    ASSERT_EQ("null", json.ToEncoding());
}

TEST(JsonTests, JsonTests_ToNull_Test) {
    const auto json = Json::Json::FromString("null");
    ASSERT_TRUE(json == nullptr);
}

TEST(JsonTests, JsonTests_FromBoolean_Test) {
    Json::Json jsonTrue(true), jsonFalse(false);
    ASSERT_EQ("true", jsonTrue.ToEncoding());
    ASSERT_EQ("false", jsonFalse.ToEncoding());
}

TEST(JsonTests, JsonTests_ToBoolean__Test) {
    const auto jsonTrue = Json::Json::FromString("true");
    const auto jsonFalse = Json::Json::FromString("false");
    ASSERT_FALSE((bool)jsonFalse);
    ASSERT_TRUE(jsonFalse == Json::Json(false));
    ASSERT_TRUE((bool)jsonTrue);
    ASSERT_TRUE(jsonTrue == Json::Json(true));
}

TEST(JsonTests, JsonTests_NtBooleanDowncastToBoolean_Test) {
    EXPECT_EQ(false, (bool)Json::Json(nullptr));
    EXPECT_EQ(false, (bool)Json::Json(std::string("")));
}

TEST(JsonTests, JsonTests_NotstringDowncastToEncoding_Test) {
    EXPECT_EQ(std::string(""), (std::string)Json::Json(nullptr));
    EXPECT_EQ(std::string(""), (std::string)Json::Json(false));
    EXPECT_EQ(std::string(""), (std::string)Json::Json(true));
}

TEST(JsonTests, NotIntegerDownCastToInteger) {
    EXPECT_EQ(0, (int)Json::Json(nullptr));
    EXPECT_EQ(0, (int)Json::Json(false));
    EXPECT_EQ(0, (int)Json::Json(true));
    EXPECT_EQ(0, (int)Json::Json("42"));
    EXPECT_EQ(42, (int)Json::Json(42.0));
    EXPECT_EQ(42, (int)Json::Json(42.5));
}

TEST(JsonTests, BadNumbers) {
    EXPECT_EQ(Json::Json(), Json::Json::FromString("-"));
    EXPECT_EQ(Json::Json(), Json::Json::FromString("+"));
    EXPECT_EQ(Json::Json(), Json::Json::FromString("X"));
    EXPECT_EQ(Json::Json(), Json::Json::FromString("0025"));
    EXPECT_EQ(Json::Json(), Json::Json::FromString("-0025"));
    EXPECT_EQ(Json::Json(), Json::Json::FromString(".4"));
    EXPECT_EQ(Json::Json(), Json::Json::FromString("99999999999999999999999999999999999999999999999999999999"));
}

TEST(JsonTests, NotFlotingPointDownCastToFloatingPoint) {
    EXPECT_EQ(0.0, (double)Json::Json(nullptr));
    EXPECT_EQ(0.0, (double)Json::Json(false));
    EXPECT_EQ(0.0, (double)Json::Json(true));
    EXPECT_EQ(0, (int)Json::Json("42"));
    EXPECT_EQ(42.0, (int)Json::Json(42.0));
}

TEST(JsonTests, JsonTests_FromCString_Test) {
    Json::Json json(std::string("Hello, World!"));
    ASSERT_EQ("\"Hello, World!\"", json.ToEncoding());
}

TEST(JsonTests, JsonTests_ToCString_Test) {
    const auto json = Json::Json::FromString("\"Hello, World!\"");
    ASSERT_TRUE(json == "Hello, World!");
}

TEST(JsonTests, JsonTests_FromCppString_Test) {
    Json::Json json(std::string("Hello, World!"));
    ASSERT_EQ("\"Hello, World!\"", json.ToEncoding());
}

TEST(JsonTests, JsonTests_ToCppString_Test) {
    const auto json = Json::Json::FromString("\"Hello, World!\"");
    ASSERT_TRUE(json == "Hello, World!");
}

TEST(JsonTests, JsonTests_ProperlyEscapedCharactersInString_Test) {
    Json::Json json(std::string("These need to be escaped: \", \\, \b, \n, \f, \r, \t"));
    ASSERT_EQ("\"These need to be escaped: \\\", \\\\, \\b, \\n, \\f, \\r, \\t\"", json.ToEncoding());
}

TEST(JsonTests, JsonTests_ProperlyEscapedUnicodeCharacters_Test) {
    std::string testStringDecoded("This is the Greek word 'kosme': κόσμε");
    std::string testStringEncodedDefault("\"This is the Greek word 'kosme': κόσμε\"");
    std::string testStringEncodedEscapeNonAscii("\"This is the Greek word 'kosme': \\u03BA\\u1F79\\u03C3\\u03BC\\u03B5\"");
    Json::Json json(testStringDecoded);
    auto jsonEncoding = json.ToEncoding();
    EXPECT_EQ(testStringEncodedDefault, jsonEncoding);
    Json::JsonEncodingOptions options;
    options.escapeNonAscii = true;
    jsonEncoding = json.ToEncoding(options);
    EXPECT_EQ(testStringEncodedEscapeNonAscii, jsonEncoding);
    json = Json::Json::FromString(testStringEncodedDefault);
    EXPECT_EQ(testStringDecoded, (std::string)json);
    json = Json::Json::FromString(testStringEncodedEscapeNonAscii);
    EXPECT_EQ(testStringDecoded, (std::string)json);
}

TEST(JsonTests, BadlyEscapeUnicodeCharacter) {
    auto json = Json::Json::FromString("\"This is bad: \\u123X\"");
    EXPECT_EQ("This is bad: \\u123X", (std::string)json);
    json = Json::Json::FromString("\"This is bad: \\x\"");
    EXPECT_EQ("This is bad: \\x", (std::string)json);

}

TEST(JsonTests, JsonTests_FromInteger_Test) {
    Json::Json json(42);
    ASSERT_EQ("42", json.ToEncoding());
}

TEST(JsonTests, JsonTests_ToInteger_Test) {
    auto json = Json::Json::FromString("42");
    ASSERT_TRUE(json == Json::Json(42));
    json = Json::Json::FromString("-242");
    ASSERT_TRUE(json == Json::Json(-242));
}

TEST(JsonTests, JsonTests_FromFloatingPoint_Test) {
    Json::Json json(3.14159);
    ASSERT_EQ("3.14159", json.ToEncoding());
} 

TEST(JsonTests, JsonTests_ToFloatingPoint_Test) {
    auto json = Json::Json::FromString("3.14159");
    ASSERT_TRUE(json == Json::Json(3.14159));
    json = Json::Json::FromString("-17.030");
    ASSERT_TRUE(json == Json::Json(-17.030));
    json = Json::Json::FromString("5.3e-4");
    ASSERT_TRUE(json == Json::Json(5.3e-4));
    json = Json::Json::FromString("5.03e+14");
    ASSERT_TRUE(json == Json::Json(5.03e+14));
    json = Json::Json::FromString("5E+0");
    ASSERT_TRUE(json == Json::Json(5E+0));
}
