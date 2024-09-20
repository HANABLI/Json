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
    const auto json = Json::Json::FromEncoding("null");
    ASSERT_TRUE(json == nullptr);
}

TEST(JsonTests, JsonTests_FromBoolean_Test) {
    Json::Json jsonTrue(true), jsonFalse(false);
    ASSERT_EQ("true", jsonTrue.ToEncoding());
    ASSERT_EQ("false", jsonFalse.ToEncoding());
}

TEST(JsonTests, JsonTests_ToBoolean__Test) {
    const auto jsonTrue = Json::Json::FromEncoding("true");
    const auto jsonFalse = Json::Json::FromEncoding("false");
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
    EXPECT_EQ(Json::Json(), Json::Json::FromEncoding("-"));
    EXPECT_EQ(Json::Json(), Json::Json::FromEncoding("+"));
    EXPECT_EQ(Json::Json(), Json::Json::FromEncoding("X"));
    EXPECT_EQ(Json::Json(), Json::Json::FromEncoding("0025"));
    EXPECT_EQ(Json::Json(), Json::Json::FromEncoding("-0025"));
    EXPECT_EQ(Json::Json(), Json::Json::FromEncoding(".4"));
    EXPECT_EQ(Json::Json(), Json::Json::FromEncoding("99999999999999999999999999999999999999999999999999999999"));
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
    const auto json = Json::Json::FromEncoding("\"Hello, World!\"");
    ASSERT_TRUE(json == "Hello, World!");
}

TEST(JsonTests, JsonTests_FromCppString_Test) {
    Json::Json json(std::string("Hello, World!"));
    ASSERT_EQ("\"Hello, World!\"", json.ToEncoding());
}

TEST(JsonTests, JsonTests_ToCppString_Test) {
    const auto json = Json::Json::FromEncoding("\"Hello, World!\"");
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
    options.reencode = true;
    options.escapeNonAscii = true;
    jsonEncoding = json.ToEncoding(options);
    EXPECT_EQ(testStringEncodedEscapeNonAscii, jsonEncoding);
    json = Json::Json::FromEncoding(testStringEncodedDefault);
    EXPECT_EQ(testStringDecoded, (std::string)json);
    json = Json::Json::FromEncoding(testStringEncodedEscapeNonAscii);
    EXPECT_EQ(testStringDecoded, (std::string)json);
}

TEST(JsonTests, BadlyEscapeUnicodeCharacter) {
    auto json = Json::Json::FromEncoding("\"This is bad: \\u123X\"");
    EXPECT_EQ(Json::Json(), json);
    json = Json::Json::FromEncoding("\"This is bad: \\x\"");
    EXPECT_EQ(Json::Json(), json);

}

TEST(JsonTests, JsonTests_FromInteger_Test) {
    Json::Json json(42);
    ASSERT_EQ("42", json.ToEncoding());
}

TEST(JsonTests, JsonTests_ToInteger_Test) {
    auto json = Json::Json::FromEncoding("42");
    ASSERT_TRUE(json == Json::Json(42));
    json = Json::Json::FromEncoding("-242");
    ASSERT_TRUE(json == Json::Json(-242));
}

TEST(JsonTests, JsonTests_FromFloatingPoint_Test) {
    Json::Json json(3.14159);
    ASSERT_EQ("3.14159", json.ToEncoding());
} 

TEST(JsonTests, JsonTests_ToFloatingPoint_Test) {
    auto json = Json::Json::FromEncoding("3.14159");
    ASSERT_TRUE(json == Json::Json(3.14159));
    json = Json::Json::FromEncoding("-17.03");
    ASSERT_TRUE(json == Json::Json(-17.03));
    json = Json::Json::FromEncoding("5.3e-4");
    ASSERT_TRUE(json == Json::Json(5.3e-4));
    json = Json::Json::FromEncoding("5.03e+14");
    ASSERT_TRUE(json == Json::Json(5.03e+14));
    json = Json::Json::FromEncoding("5E+0");
    ASSERT_TRUE(json == Json::Json(5E+0));
}

TEST(JsonTests, JsonTests_SurrogatePairEncoding_Test) {
    Json::Json json(std::string("This should be encoded as a UTF-16 surrogate pair: 𣎴"));
    Json::JsonEncodingOptions options;
    options.escapeNonAscii = true;
    ASSERT_EQ("\"This should be encoded as a UTF-16 surrogate pair: \\uD84C\\uDFB4\"", json.ToEncoding(options));
    json = Json::Json(std::string("This should be encoded as a UTF-16 surrogate pair: 💩"));
    ASSERT_EQ("\"This should be encoded as a UTF-16 surrogate pair: \\uD83D\\uDCA9\"", json.ToEncoding(options));
}

TEST(JsonTests, JsonTests_SurrogatePairDecoding_Test) {
    std::string encoding("\"This should be encoded as a UTF-16 surrogate pair: \\uD84C\\uDFB4\"");
    ASSERT_EQ("This should be encoded as a UTF-16 surrogate pair: 𣎴", (std::string)Json::Json::FromEncoding(encoding));
    encoding = "\"This should be encoded as a UTF-16 surrogate pair: \\uD83D\\uDCA9\"";
    ASSERT_EQ("This should be encoded as a UTF-16 surrogate pair: 💩", (std::string)Json::Json::FromEncoding(encoding));

}

TEST(JsonTests, JsonTests_EncodingOfInvalidJson_Test) {
    auto json = Json::Json::FromEncoding("\"This is bad: \\u123X\"");
    ASSERT_EQ("(Invalid JSON: \"This is bad: \\u123X\")", json.ToEncoding());
}

TEST(JsonTests, JsonTests_ArrayDecoding_Test) {
    const std::string encoding("[1,\"Hello\",true]");
    const auto json = Json::Json::FromEncoding(encoding);
    ASSERT_TRUE(json.GetType() == Json::Json::Type::Array);
    ASSERT_EQ(3, json.GetSize());
    EXPECT_TRUE(json[0]->GetType() == Json::Json::Type::Integer);
    EXPECT_EQ(1, (int)*json[0]);
    EXPECT_TRUE(json[1]->GetType() == Json::Json::Type::String);
    EXPECT_EQ("Hello", (std::string)*json[1]);
    EXPECT_TRUE(json[2]->GetType() == Json::Json::Type::Boolean);
    EXPECT_EQ(true, (bool)*json[2]);
    EXPECT_TRUE(json[3] == nullptr);
}

TEST(JsonTests, JsonTests_DecodeUnterminatedOuterArray_Test) {
    const std::string encoding("[1, \"Hello\", true");
    const auto json = Json::Json::FromEncoding(encoding);
    ASSERT_EQ(Json::Json::Type::Invalid, json.GetType());
}

TEST(JsonTests, JsonTests_DecodeUnterminatedInnerArray_Test) {
    const std::string encoding("{ \"value\": 1, \"array\": [42, 57, \"flag\": true }");
    const auto json = Json::Json::FromEncoding(encoding);
    ASSERT_EQ(Json::Json::Type::Invalid, json.GetType());
} 

TEST(JsonTests, JsonTests_DecodeUnterminatedInnerString_Test) {
    const std::string encoding("[1,\"Hello, true");
    const auto json = Json::Json::FromEncoding(encoding);
    ASSERT_EQ(Json::Json::Type::Invalid, json.GetType());
} 

TEST(JsonTests, JsonTests_ArraysWhithernsArray_Test) {
    const std::string encoding("[1,[1,2],true]");
    const auto json = Json::Json::FromEncoding(encoding);
    ASSERT_TRUE(json.GetType() == Json::Json::Type::Array);
    ASSERT_EQ(3, json.GetSize());
    EXPECT_TRUE(json[0]->GetType() == Json::Json::Type::Integer);
    EXPECT_EQ(1, (int)*json[0]);
    EXPECT_TRUE((*json[1])[0]->GetType() == Json::Json::Type::Integer);
    EXPECT_EQ(1, (int)*(*json[1])[0]);
    EXPECT_TRUE((*json[1])[1]->GetType() == Json::Json::Type::Integer);
    EXPECT_EQ(2, (int)*(*json[1])[1]);
    EXPECT_TRUE(json[2]->GetType() == Json::Json::Type::Boolean);
    EXPECT_EQ(true, (bool)*json[2]);
}

TEST(JsonTests, JsonTests_DecodeArrayWithWhiteSpace_Test) {
    const std::string encoding(" [ 1 ,\r \t \"Hello\" \r\n ,\n true ] ");
    const auto json = Json::Json::FromEncoding(encoding);
    ASSERT_TRUE(json.GetType() == Json::Json::Type::Array);
    ASSERT_EQ(3, json.GetSize());
} 

TEST(JsonTests, JsonTests_DecodeObject_Test) {
    const std::string encoding("{\"value\": 42, \"name\": \"Toto\", \"handles\":[3,7], \"is,live\": true}");
    const auto json = Json::Json::FromEncoding(encoding);
    ASSERT_EQ(Json::Json::Type::Object, json.GetType());
    ASSERT_EQ(4, json.GetSize());
    EXPECT_TRUE(json.Has("value"));
    EXPECT_TRUE(json.Has("name"));
    EXPECT_TRUE(json.Has("handles"));
    EXPECT_TRUE(json.Has("is,live"));
    EXPECT_FALSE(json.Has("feels bad"));
    const auto value  = json["value"];
    EXPECT_EQ(Json::Json::Type::Integer, value->GetType());
    EXPECT_EQ(42, (int)*value);
    const auto name = json["name"];
    EXPECT_EQ(Json::Json::Type::String, name->GetType());
    EXPECT_EQ("Toto", (std::string)*name);
    const auto handles = json["handles"];
    EXPECT_EQ(Json::Json::Type::Array, handles->GetType());
    EXPECT_EQ(Json::Json::Type::Integer, (*handles)[0]->GetType());
    EXPECT_EQ(3, (int)*(*handles)[0]);
    EXPECT_EQ(Json::Json::Type::Integer, (*handles)[1]->GetType());
    EXPECT_EQ(7, (int)*(*handles)[1]);
    const auto isLive = json["is,live"];
    EXPECT_EQ(Json::Json::Type::Boolean, isLive->GetType());
    EXPECT_EQ(true, (bool)*isLive);
}

TEST(JsonTests, JsonTests_NumericIndexNotArray__Test) {
    const Json::Json json(42);
    ASSERT_TRUE(json[0] == nullptr);
}

TEST(JsonTests, JsonTests_EncodeArray__Test) {
    Json::Json json(Json::Json::Type::Array);
    json.Add(42);
    json.Insert("Hello", 0);
    json.Add(3);
    json.Insert("World", 1);
    json.Remove(1);
    ASSERT_EQ("[\"Hello\",42,3]", json.ToEncoding());
}

TEST(JsonTests, JsonTests_EncodeObjec_Test) {
    Json::Json json(Json::Json::Type::Object);
    json.Set("number", 42);
    json.Set("Hello", "World");
    json.Set("PopChamp", true);
    json.Set("Nullptr", nullptr);
    json.Set("{\"Hello\":\"World\",\"Nullptr\": null,\"PopChamp\":true,\"number\":42}", json.ToEncoding());
    json.Remove("number");
    json.Set("{\"Hello\":\"World\",\"Nullptr\": null,\"PopChamp\":true}", json.ToEncoding());
}

TEST(JsonTests, JsonTests_CompareArrays_Test) {
    const auto json1 = Json::Json::FromEncoding("[31, 7]");
    const auto json2 = Json::Json::FromEncoding(" [31, 7]");
    const auto json3 = Json::Json::FromEncoding(" [32, 6]");
    EXPECT_EQ(json1, json2);
    EXPECT_NE(json1, json3);
    EXPECT_NE(json2, json3);
}

TEST(JsonTests, JsonTests_CompareObjects_Test) {
    const auto json1 = Json::Json::FromEncoding("{\"number\":31}");
    const auto json2 = Json::Json::FromEncoding("{\"number\": 31}");
    const auto json3 = Json::Json::FromEncoding("{\"number\": 32}");
    const auto json4 = Json::Json::FromEncoding("{\"number\": 32,\"Hello\": [31, 7]}");
    const auto json5 = Json::Json::FromEncoding("{\"number\": 32,\"Hello\": [32, 7]}");
    EXPECT_EQ(json1, json2);
    EXPECT_NE(json1, json3);
    EXPECT_NE(json2, json3);  
    EXPECT_NE(json4, json5);
}

TEST(JsonTests, JsonTests_AddObjectToItSelf__Test) {
    Json::Json json(Json::Json::Type::Array);
    json.Add(31);
    json.Add(json);
    EXPECT_EQ("[31,[31]]", json.ToEncoding());
}