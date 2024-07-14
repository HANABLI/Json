/**
 * @file Json.cpp
 * 
 * This module contains the implementation of
 * the Json::Json class.
 * 
 * © 2024 by Hatem Nabli
 */

#include <Json/Json.hpp>

namespace Json{

    struct Json::Impl
    {
        /* data */
    };
    Json::~Json() = default;
    Json::Json(): impl_(new Impl) { }
    
}