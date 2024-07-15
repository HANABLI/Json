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
        /**
         * These are the different kinds of values of a JSON object.
         */
        enum class Type {
            Invalid,
            Null,
            Boolean
        };

        /**
         * This indicates the type of the value represented
         * by the JSON object.
         */
        Type type = Type::Invalid;

        union 
        { 
            bool booleanValue;
        };
    };

    Json::~Json() = default;
    Json::Json(Json&&) = default;
    Json& Json::operator=(Json&&) = default;

    Json::Json(): impl_(new Impl) { 

    }
    
    Json::Json(nullptr_t): impl_(new Impl) {
        impl_->type = Impl::Type::Null;
    }

    Json::Json(bool value): impl_(new Impl) {
        impl_->type = Impl::Type::Boolean;
        impl_->booleanValue = value;
    }

    bool Json::operator==(const Json& other) const {
        if (impl_->type != other.impl_->type) {
            return false;
        } else switch (impl_->type)
        {
        case Impl::Type::Null:
            return true;
            break;
        case Impl::Type::Boolean:
            return impl_->booleanValue == other.impl_->booleanValue;
            break;
        default:
            return true;
            break;
        }
    }

    std::string Json::ToString() const {
        switch (impl_->type)
        {
        case Impl::Type::Null:
            return "null";
            break;
        case Impl::Type::Boolean:
            return impl_->booleanValue ? "true" : "false";
            break;
        default:
            return "???";
            break;
        }
    }

    Json Json::FromString(const std::string& stringFormat) {        
        if (stringFormat == "null") {
            return nullptr;
        } else if (stringFormat == "true") {
            return true;
        } else if  (stringFormat == "false") {
            return false;
        } else {
            return Json();
        }
    }
}