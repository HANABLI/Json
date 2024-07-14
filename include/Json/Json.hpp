#ifndef JSON_JSON_HPP
#define JSON_JSON_HPP

/**
 * @file Json.hpp
 * 
 * This module declare the Json::Json class.
 * 
 * © 2024 by Hatem Nabli
 */

#include <memory>
#include <string>

namespace Json
{
    /**
     * This class represent a data structure that was parsed
     * from oe can be rendered to, a string in the JavaScript
     * Object Notation "JSON" Data Interchange Format, as specified
     * in RFC 7159
     */
    class Json
    {

    public:

    // Life Cycle Management
    public:
        ~Json();
        Json(const Json) = delete;
        Json(Json&&) = delete;
        Json& operator=(const Json&) = delete;
        Json& operator=(Json&&) = delete;

    // Public methods
    public:
        /**
         * This is the default constructor.
         */
        Json(/* args */);

        /**
         * This constructs a JSON object consisting of the "null" literal.
         * 
         * @param[in] null
         *      This is the object to wrap un JSON.
         */
        Json(nullptr_t);

        /**
         * This encode a JSON object into its string format.
         * 
         * @return
         *      The string format of the JSON object is returned.
         */
        std::string ToString() const;

        /**
         * This method returns a new JSON object constructed by parsing
         * the JSON object from the given string.
         * 
         * @param[in] format
         *      This is the string format of the JSON object to construct.
         */
        static Json FromString(const std::string& format);

        // Private properties
    private:
        /* data */

        /**
         * This is the type of structure that contains the private
         * properties of the instance. It is defined in the implementation
         * and declared here to ensure that iwt is scoped inside the class.
        */
        struct Impl;

        /**
        * This contains the private properties of the instance.
        */       
        std::unique_ptr<struct Impl> impl_;
    };
    
    
} // namespace Json


#endif /* JSON_JSON_HPP */