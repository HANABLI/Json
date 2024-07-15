/**
 * @file JsonTests.cpp
 * 
 * This module contains the unit test of the
 * Json::Json class.
 * 
 * © 2024 by Hatem Nabli
 */

#include <gtest/gtest.h>
#include <Json/Json.hpp>

TEST(JsonTests, JsonTests_FromNull_Test) {
    Json::Json json(nullptr);
    ASSERT_EQ("null", json.ToString());
}

TEST(JsonTests, JsonTests_ToNull_Test) {
    const auto json = Json::Json::FromString("null");
    ASSERT_TRUE(json == nullptr);
}

TEST(JsonTests, JsonTests_FromBoolean_Test) {
    Json::Json jsonTrue(true), jsonFalse(false);
    ASSERT_EQ("true", jsonTrue.ToString());
    ASSERT_EQ("false", jsonFalse.ToString());
}

TEST(JsonTests, JsonTests_ToBoolean__Test) {
    const auto jsonTrue = Json::Json::FromString("true");
    const auto jsonFalse = Json::Json::FromString("false");
    ASSERT_TRUE(jsonFalse == false);
    ASSERT_TRUE(jsonTrue == true);
}