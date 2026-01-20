/**
 * @file JsonTests.cpp
 *
 * This module contains the unit test of the
 * Json::Json class.
 *
 * © 2024 by Hatem Nabli
 */

#include <gtest/gtest.h>
#include <math.h>
#include <Json/Json.hpp>

TEST(JsonTests, JsonTests_FromNull_Test) {
    Json::Value json(nullptr);
    ASSERT_EQ("null", json.ToEncoding());
}

TEST(JsonTests, JsonTests_ToNull_Test) {
    const auto json = Json::Value::FromEncoding("null");
    ASSERT_TRUE(json == nullptr);
}

TEST(JsonTests, JsonTests_FromBoolean_Test) {
    Json::Value jsonTrue(true), jsonFalse(false);
    ASSERT_EQ("true", jsonTrue.ToEncoding());
    ASSERT_EQ("false", jsonFalse.ToEncoding());
}

TEST(JsonTests, JsonTests_ToBoolean__Test) {
    const auto jsonTrue = Json::Value::FromEncoding("true");
    const auto jsonFalse = Json::Value::FromEncoding("false");
    ASSERT_FALSE((bool)jsonFalse);
    ASSERT_TRUE(jsonFalse == Json::Value(false));
    ASSERT_TRUE((bool)jsonTrue);
    ASSERT_TRUE(jsonTrue == Json::Value(true));
}

TEST(JsonTests, JsonTests_NtBooleanDowncastToBoolean_Test) {
    EXPECT_EQ(false, (bool)Json::Value(nullptr));
    EXPECT_EQ(false, (bool)Json::Value(std::string("")));
}

TEST(JsonTests, JsonTests_NotstringDowncastToEncoding_Test) {
    EXPECT_EQ(std::string(""), (std::string)Json::Value(nullptr));
    EXPECT_EQ(std::string(""), (std::string)Json::Value(false));
    EXPECT_EQ(std::string(""), (std::string)Json::Value(true));
}

TEST(JsonTests, NotIntegerDownCastToInteger) {
    EXPECT_EQ(0, (int)Json::Value(nullptr));
    EXPECT_EQ(0, (int)Json::Value(false));
    EXPECT_EQ(0, (int)Json::Value(true));
    EXPECT_EQ(0, (int)Json::Value("42"));
    EXPECT_EQ(42, (int)Json::Value(42.0));
    EXPECT_EQ(42, (int)Json::Value(42.5));
}

TEST(JsonTests, BadNumbers) {
    EXPECT_EQ(Json::Value(), Json::Value::FromEncoding("-"));
    EXPECT_EQ(Json::Value(), Json::Value::FromEncoding("+"));
    EXPECT_EQ(Json::Value(), Json::Value::FromEncoding("X"));
    EXPECT_EQ(Json::Value(), Json::Value::FromEncoding("0025"));
    EXPECT_EQ(Json::Value(), Json::Value::FromEncoding("-0025"));
    EXPECT_EQ(Json::Value(), Json::Value::FromEncoding(".4"));
    EXPECT_EQ(Json::Value(), Json::Value::FromEncoding(
                                 "99999999999999999999999999999999999999999999999999999999"));
    EXPECT_EQ(Json::Value(), Json::Value::FromEncoding(
                                 "99999999999999999999999999999999999999999999999999999999.0"));
    EXPECT_EQ(Json::Value(), Json::Value::FromEncoding(
                                 "le99999999999999999999999999999999999999999999999999999999"));
    EXPECT_EQ(Json::Value(), Json::Value::FromEncoding("le"));
}

TEST(JsonTests, NotFlotingPointDownCastToFloatingPoint) {
    EXPECT_EQ(0.0, (double)Json::Value(nullptr));
    EXPECT_EQ(0.0, (double)Json::Value(false));
    EXPECT_EQ(0.0, (double)Json::Value(true));
    EXPECT_EQ(0.0, (int)Json::Value("42"));
    EXPECT_EQ(42.0, (int)Json::Value(42));
}

TEST(JsonTests, JsonTests_FromCString_Test) {
    Json::Value json(std::string("Hello, World!"));
    ASSERT_EQ("\"Hello, World!\"", json.ToEncoding());
}

TEST(JsonTests, JsonTests_ToCString_Test) {
    const auto json = Json::Value::FromEncoding("\"Hello, World!\"");
    ASSERT_TRUE(json == "Hello, World!");
}

TEST(JsonTests, JsonTests_FromCppString_Test) {
    Json::Value json(std::string("Hello, World!"));
    ASSERT_EQ("\"Hello, World!\"", json.ToEncoding());
}

TEST(JsonTests, JsonTests_ToCppString_Test) {
    const auto json = Json::Value::FromEncoding("\"Hello, World!\"");
    ASSERT_TRUE(json == "Hello, World!");
}

TEST(JsonTests, JsonTests_ProperlyEscapedCharactersInString_Test) {
    Json::Value json(std::string("These need to be escaped: \", \\, \b, \n, \f, \r, \t"));
    ASSERT_EQ("\"These need to be escaped: \\\", \\\\, \\b, \\n, \\f, \\r, \\t\"",
              json.ToEncoding());
}

TEST(JsonTests, JsonTests_ProperlyEscapedUnicodeCharacters_Test) {
    std::string testStringDecoded("This is the Greek word 'kosme': κόσμε");
    std::string testStringEncodedDefault("\"This is the Greek word 'kosme': κόσμε\"");
    std::string testStringEncodedEscapeNonAscii(
        "\"This is the Greek word 'kosme': "
        "\\u03BA\\u1F79\\u03C3\\u03BC\\u03B5\"");
    Json::Value json(testStringDecoded);
    auto jsonEncoding = json.ToEncoding();
    EXPECT_EQ(testStringEncodedDefault, jsonEncoding);
    Json::JsonEncodingOptions options;
    options.reencode = true;
    options.escapeNonAscii = true;
    jsonEncoding = json.ToEncoding(options);
    EXPECT_EQ(testStringEncodedEscapeNonAscii, jsonEncoding);
    json = Json::Value::FromEncoding(testStringEncodedDefault);
    EXPECT_EQ(testStringDecoded, (std::string)json);
    json = Json::Value::FromEncoding(testStringEncodedEscapeNonAscii);
    EXPECT_EQ(testStringDecoded, (std::string)json);
}

TEST(JsonTests, BadlyEscapeUnicodeCharacter) {
    auto json = Json::Value::FromEncoding("\"This is bad: \\u123X\"");
    EXPECT_EQ(Json::Value(), json);
    json = Json::Value::FromEncoding("\"This is bad: \\x\"");
    EXPECT_EQ(Json::Value(), json);
}

TEST(JsonTests, JsonTests_FromInteger_Test) {
    Json::Value json(42);
    ASSERT_EQ("42", json.ToEncoding());
}

TEST(JsonTests, JsonTests_ToInteger_Test) {
    auto json = Json::Value::FromEncoding("42");
    ASSERT_TRUE(json == Json::Value(42));
    json = Json::Value::FromEncoding("-242");
    ASSERT_TRUE(json == Json::Value(-242));
}

TEST(JsonTests, JsonTests_FromFloatingPoint_Test) {
    Json::Value json(3.14159);
    ASSERT_EQ("3.14159", json.ToEncoding());
}

TEST(JsonTests, JsonTests_FloatingPoint_Test) {
    Json::Value value = -17.03;
    double json = value;
    double expected = -17.03;
    double tolerance = 1e-6;  // Adjust the tolerance as needed

    std::cout << "json: " << json << std::endl;
    std::cout << "expected: " << expected << std::endl;

    ASSERT_NEAR(json, expected, tolerance);
}

// TEST(JsonTests, JsonTests_ToFloatingPoint_Test) {
//     auto json = Json::Value::FromEncoding("3.14159");
//     ASSERT_TRUE(json == Json::Value(3.14159));
//     json = Json::Value::FromEncoding("5.3e-4");
//     ASSERT_TRUE(json == Json::Value(5.3e-4));
//     json = Json::Value::FromEncoding("5.03e+14");
//     ASSERT_TRUE(json == Json::Value(5.03e+14));
//     json = Json::Value::FromEncoding("5E+0");
//     ASSERT_TRUE(json == Json::Value(5E+0));
// }

TEST(JsonTests, JsonTests_SurrogatePairEncoding_Test) {
    Json::Value json(std::string("This should be encoded as a UTF-16 surrogate pair: 𣎴"));
    Json::JsonEncodingOptions options;
    options.escapeNonAscii = true;
    ASSERT_EQ("\"This should be encoded as a UTF-16 surrogate pair: \\uD84C\\uDFB4\"",
              json.ToEncoding(options));
    json = Json::Value(std::string("This should be encoded as a UTF-16 surrogate pair: 💩"));
    ASSERT_EQ("\"This should be encoded as a UTF-16 surrogate pair: \\uD83D\\uDCA9\"",
              json.ToEncoding(options));
}

TEST(JsonTests, JsonTests_SurrogatePairDecoding_Test) {
    std::string encoding("\"This should be encoded as a UTF-16 surrogate pair: \\uD84C\\uDFB4\"");
    ASSERT_EQ("This should be encoded as a UTF-16 surrogate pair: 𣎴",
              (std::string)Json::Value::FromEncoding(encoding));
    encoding = "\"This should be encoded as a UTF-16 surrogate pair: \\uD83D\\uDCA9\"";
    ASSERT_EQ("This should be encoded as a UTF-16 surrogate pair: 💩",
              (std::string)Json::Value::FromEncoding(encoding));
}

TEST(JsonTests, JsonTests_EncodingOfInvalidJson_Test) {
    auto json = Json::Value::FromEncoding("\"This is bad: \\u123X\"");
    ASSERT_EQ("(Invalid JSON: \"This is bad: \\u123X\")", json.ToEncoding());
}

TEST(JsonTests, JsonTests_ArrayDecoding_Test) {
    const std::string encoding("[1,\"Hello\",true]");
    const auto json = Json::Value::FromEncoding(encoding);
    ASSERT_TRUE(json.GetType() == Json::Value::Type::Array);
    ASSERT_EQ(3, json.GetSize());
    EXPECT_TRUE(json[0].GetType() == Json::Value::Type::Integer);
    EXPECT_EQ(1, (int)json[0]);
    EXPECT_TRUE(json[1].GetType() == Json::Value::Type::String);
    EXPECT_EQ("Hello", (std::string)json[1]);
    EXPECT_TRUE(json[2].GetType() == Json::Value::Type::Boolean);
    EXPECT_EQ(true, (bool)json[2]);
    EXPECT_TRUE(json[3] == nullptr);
}

TEST(JsonTests, JsonTests_DecodeUnterminatedOuterArray_Test) {
    const std::string encoding("[1, \"Hello\", true");
    const auto json = Json::Value::FromEncoding(encoding);
    ASSERT_EQ(Json::Value::Type::Invalid, json.GetType());
}

TEST(JsonTests, JsonTests_DecodeUnterminatedInnerArray_Test) {
    const std::string encoding("{ \"value\": 1, \"array\": [42, 57, \"flag\": true }");
    const auto json = Json::Value::FromEncoding(encoding);
    ASSERT_EQ(Json::Value::Type::Invalid, json.GetType());
}

TEST(JsonTests, JsonTests_DecodeUnterminatedInnerString_Test) {
    const std::string encoding("[1,\"Hello, true");
    const auto json = Json::Value::FromEncoding(encoding);
    ASSERT_EQ(Json::Value::Type::Invalid, json.GetType());
}

TEST(JsonTests, JsonTests_ArraysWhithernsArray_Test) {
    const std::string encoding("[1,[1,2],true]");
    const auto json = Json::Value::FromEncoding(encoding);
    ASSERT_TRUE(json.GetType() == Json::Value::Type::Array);
    ASSERT_EQ(3, json.GetSize());
    EXPECT_TRUE(json[0].GetType() == Json::Value::Type::Integer);
    EXPECT_EQ(1, (int)json[0]);
    EXPECT_TRUE((json[1])[0].GetType() == Json::Value::Type::Integer);
    EXPECT_EQ(1, (int)(json[1])[0]);
    EXPECT_TRUE((json[1])[1].GetType() == Json::Value::Type::Integer);
    EXPECT_EQ(2, (int)(json[1])[1]);
    EXPECT_TRUE(json[2].GetType() == Json::Value::Type::Boolean);
    EXPECT_EQ(true, (bool)json[2]);
}

TEST(JsonTests, JsonTests_ObjectsWhithernsObject_Test) {
    const std::string encoding("{\"nested\":{\"value\": 31, \"well\": true}, \"end\": null}");
    const auto json = Json::Value::FromEncoding(encoding);
    ASSERT_TRUE(Json::Value::Type::Object == json.GetType());
    ASSERT_EQ(2, json.GetSize());
    ASSERT_TRUE(json.Has("nested"));
    EXPECT_TRUE(json["nested"].GetType() == Json::Value::Type::Object);
    ASSERT_EQ(2, json["nested"].GetSize());
    ASSERT_TRUE(json.Has("end"));
    ASSERT_TRUE(json["nested"].Has("value"));
    ASSERT_TRUE(json["nested"].Has("well"));
    EXPECT_TRUE((json["nested"])["value"].GetType() == Json::Value::Type::Integer);
    EXPECT_TRUE((json["nested"])["well"].GetType() == Json::Value::Type::Boolean);
    ASSERT_EQ(31, (int)(json["nested"])["value"]);
    ASSERT_EQ(true, (bool)(json["nested"])["well"]);
    EXPECT_TRUE(json["end"].GetType() == Json::Value::Type::Null);
}

TEST(JsonTests, JsonTests_DecodeArrayWithWhiteSpace_Test) {
    const std::string encoding(" [ 1 ,\r \t \"Hello\" \r\n ,\n true ] ");
    const auto json = Json::Value::FromEncoding(encoding);
    ASSERT_TRUE(json.GetType() == Json::Value::Type::Array);
    ASSERT_EQ(3, json.GetSize());
}

TEST(JsonTests, JsonTests_DecodeObject_Test) {
    const std::string encoding(
        "{\"value\": 42, \"name\": \"Toto\", \"handles\":[3,7], \"is,live\": "
        "true}");
    const auto json = Json::Value::FromEncoding(encoding);
    ASSERT_EQ(Json::Value::Type::Object, json.GetType());
    ASSERT_EQ(4, json.GetSize());
    EXPECT_TRUE(json.Has("value"));
    EXPECT_TRUE(json.Has("name"));
    EXPECT_TRUE(json.Has("handles"));
    EXPECT_TRUE(json.Has("is,live"));
    EXPECT_FALSE(json.Has("feels bad"));
    const auto value = json["value"];
    EXPECT_EQ(Json::Value::Type::Integer, value.GetType());
    EXPECT_EQ(42, (int)value);
    const auto name = json["name"];
    EXPECT_EQ(Json::Value::Type::String, name.GetType());
    EXPECT_EQ("Toto", (std::string)name);
    const auto handles = json["handles"];
    EXPECT_EQ(Json::Value::Type::Array, handles.GetType());
    EXPECT_EQ(Json::Value::Type::Integer, (handles)[0].GetType());
    EXPECT_EQ(3, (int)(handles)[0]);
    EXPECT_EQ(Json::Value::Type::Integer, (handles)[1].GetType());
    EXPECT_EQ(7, (int)(handles)[1]);
    const auto isLive = json["is,live"];
    EXPECT_EQ(Json::Value::Type::Boolean, isLive.GetType());
    EXPECT_EQ(true, (bool)isLive);
}

TEST(JsonTests, JsonTests_NumericIndexNotArray__Test) {
    const Json::Value json(42);
    ASSERT_TRUE(json[0] == nullptr);
}

TEST(JsonTests, JsonTests_EncodeArray__Test) {
    Json::Value json(Json::Value::Type::Array);
    json.Add(42);
    json.Insert("Hello", 0);
    json.Add(3);
    json.Insert("World", 1);
    json.Remove(1);
    ASSERT_EQ("[\"Hello\",42,3]", json.ToEncoding());
}

TEST(JsonTests, JsonTests_EncodeObjec_Test) {
    Json::Value json(Json::Value::Type::Object);
    json.Set("number", 42);
    json.Set("Hello", "World");
    json.Set("PopChamp", true);
    json.Set("Nullptr", nullptr);
    json.Set("{\"Hello\":\"World\",\"Nullptr\": null,\"PopChamp\":true,\"number\":42}",
             json.ToEncoding());
    json.Remove("number");
    json.Set("{\"Hello\":\"World\",\"Nullptr\": null,\"PopChamp\":true}", json.ToEncoding());
}

TEST(JsonTests, JsonTests_CompareArrays_Test) {
    const auto json1 = Json::Value::FromEncoding("[31, 7]");
    const auto json2 = Json::Value::FromEncoding(" [31, 7]");
    const auto json3 = Json::Value::FromEncoding(" [32, 6]");
    EXPECT_EQ(json1, json2);
    EXPECT_NE(json1, json3);
    EXPECT_NE(json2, json3);
}

TEST(JsonTests, JsonTests_CompareObjects_Test) {
    const auto json1 = Json::Value::FromEncoding("{\"number\":31}");
    const auto json2 = Json::Value::FromEncoding("{\"number\": 31}");
    const auto json3 = Json::Value::FromEncoding("{\"number\": 32}");
    const auto json4 = Json::Value::FromEncoding("{\"number\": 32,\"Hello\": [31, 7]}");
    const auto json5 = Json::Value::FromEncoding("{\"number\": 32,\"Hello\": [32, 7]}");
    EXPECT_EQ(json1, json2);
    EXPECT_NE(json1, json3);
    EXPECT_NE(json2, json3);
    EXPECT_NE(json4, json5);
}

TEST(JsonTests, JsonTests_AddObjectToItSelf__Test) {
    Json::Value json(Json::Value::Type::Array);
    json.Add(31);
    json.Add(json);
    EXPECT_EQ("[31,[31]]", json.ToEncoding());
}

TEST(JsonTests, JsonTests_ReassignValue_Test) {
    Json::Value json1(31);
    Json::Value json2(Json::Value::Type::Array);
    json2.Add(31);
    json2.Add("Hello");
    json1 = json2;
    json1.Add(false);
    json2.Remove(0);
    json2.Add(true);
    EXPECT_EQ("[31,\"Hello\",false]", json1.ToEncoding());
    EXPECT_EQ("[\"Hello\",true]", json2.ToEncoding());
}

TEST(JsonTests, JsonTests_PrettyPrinting_Test) {
    const std::string encoding(
        "{\"value\": 31, \"name\": \"Toto\", \"handles\":[3,7], \"is,live\": "
        "true}");
    const auto json = Json::Value::FromEncoding(encoding);
    Json::JsonEncodingOptions options;
    options.reencode = true;
    options.pretty = true;
    options.spacesIndentationLevels = 4;
    options.wrapthreshold = 30;
    ASSERT_EQ(
        "{\r\n"
        "    \"handles\": [3, 7],\r\n"
        "    \"is,live\": true,\r\n"
        "    \"name\": \"Toto\",\r\n"
        "    \"value\": 31\r\n"
        "}",
        json.ToEncoding(options));
}

TEST(JsonTests, JsonTests_PrettyPrintingArray_Test) {
    const std::string encoding("[1,[2,3],4,[4,9,3]]");
    const auto json = Json::Value::FromEncoding(encoding);
    Json::JsonEncodingOptions options;
    options.reencode = true;
    options.pretty = true;
    options.spacesIndentationLevels = 4;
    options.wrapthreshold = 11;
    ASSERT_EQ(("[\r\n"
               "    1,\r\n"
               "    [2, 3],\r\n"
               "    4,\r\n"
               "    [\r\n"
               "        4,\r\n"
               "        9,\r\n"
               "        3\r\n"
               "    ]\r\n"
               "]"),
              json.ToEncoding(options));
}

TEST(JsonTests, JsonTests_JsonArrayInitializerList_Test) {
    Json::Value json{42, "Hello, World!", true};
    ASSERT_EQ("[42,\"Hello, World!\",true]", json.ToEncoding());
}

TEST(JsonTests, JsonTests_GetKey_Test) {
    const std::string encoding(
        "{\"value\": 42, \"\": \"Pepe\", \"the handles\":[3,7], \"is,live\": true}");
    const auto json = Json::Value::FromEncoding(encoding);
    ASSERT_EQ((std::vector<std::string>{"", "is,live", "the handles", "value"}), json.GetKeys());
}

TEST(JsonTests, JsonTests_ArrayIterator_Test) {
    const auto array = Json::Array({"a", "b", "c"});
    std::vector<std::string> values;
    for (auto arrayEntry : array)
    { values.push_back(arrayEntry.value()); }
    ASSERT_EQ(std::vector<std::string>({"a", "b", "c"}), values);
}

// TEST(JsonTests, JsonTests_ObjectIterator_Test) {
//     const auto objects = Json::Object({{"toto", "toto"}, {"titi", "titi"}});
//     std::vector<std::string> keys;
//     std::vector<Json::Value> values;
//     for (auto objectEntry : objects)
//     {
//         keys.push_back(objectEntry.key());
//         values.push_back(objectEntry.value());
//     }

//     EXPECT_EQ(std::vector<Json::Value>({"toto", "titi"}), values);
//     EXPECT_EQ(std::vector<std::string>({"toto", "titi"}), keys);
// }