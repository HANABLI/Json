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
    ASSERT_EQ(nullptr, json);
}