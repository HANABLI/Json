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
     * This is used to configure various options in 
     * JSON object encoding to string format.
     */
    struct JsonEncodingOptions {
        
        /**
         * This flag indicates whether or not to escape 
         * non ASCII characters when encoding the JSON
         * object into its string format.
         */
        bool escapeNonAscii = false;
    };
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
        Json(const Json&) = delete;
        Json(Json&&);
        Json& operator=(const Json&) = delete;
        Json& operator=(Json&&);

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
         *      This is the object to wrap in JSON.
         */
        Json(nullptr_t);

        /**
         * This constructs a JSON object consisting of the boolean value.
         * 
         * @param[in] value
         *      This is the object to wrap in JSON.
         */
        Json(bool value);

        /**
         * This construct a JSON object consisting of an integer value.
         * 
         * @param[in] value
         *      This is the object to wrap in JSON
         */
        Json(int value);

        /**
         * This construct a JSON object consisting of a double value.
         * 
         * @param[in] value
         *      This is the object to wrap in JSON
         */
        Json(double value);

        /**
         * This constructs a JSON object consisting of a string value.
         * 
         * @param[in] value
         *      This is the object to wrap in JSON.
         */
        Json(const std::string& value);

        /**
         * This constructs a json object consisting of a C string value.
         * 
         * @param[in] value
         *      This is the object to wrap in JSON.
         */
        Json(const char* value);


        /**
         * This is the equality compariosn operator.
         * 
         * @param[in] other
         *      This is the object to compare with this one.
         * 
         * @return
         *      An indication of whether or not the two JSON objects are equal
         *      is returned.
         */
        bool operator==(const Json& other) const;

        /**
         * This is the typecast to bool operator for the class.
         * 
         * @return
         *      The boolean equivalent of the JSON object is returned.
         * 
         * @retval true
         *      This is returned if the JSON object is a boolean and its
         *      value is true.
         * 
         * @retval false
         *      This is returned if the JSON object is not a boolean or it's 
         *      a boolean and its value is false. 
         */
        operator bool() const;

        /**
         * This is the typecast to C++ string operator for the class.
         * 
         * @return
         *      The C++ string equivalent of the JSON object is returned.
         * 
         * 
         * @retval std::string("")
         *      This is returned if the JSON object is not a string or it's 
         *      a string and its value is the empty string. 
         */
        operator std::string() const;

        /**
         * This is the typecast to C++ integer operator for the class.
         * 
         * @return
         *      The C++ integer equivalent of the JSON object is returned.
         * 
         * 
         * @retval 0
         *      This is returned if the JSON object is not an integer or it's 
         *      an integer and its value is zero. 
         */
        operator int() const;



         /**
         * This is the typecast to C++ double operator for the class.
         * 
         * @return
         *      The C++ double equivalent of the JSON object is returned.
         * 
         * 
         * @retval 0.0
         *      This is returned if the JSON object is not an double or it's 
         *      an double and its value is zero. 
         */
        operator double() const;

        
        /**
         * This encode a JSON object into its string format.
         * 
         * @param[in] options
         *      This is used to configure various options in 
         *      JSON object encoding to string format.
         * 
         * @return
         *      The encoding of the JSON object is returned.
         */
        std::string ToEncoding(const JsonEncodingOptions options = JsonEncodingOptions()) const;

        /**
         * This method returns a new JSON object constructed by parsing
         * the JSON object from the given string.
         * 
         * @param[in] format
         *      This is the string format of the JSON object to construct.
         */
        static Json FromString(const std::string& stringFormat);

        /**
         * This method sets the flag which controls whether or not
         * to escape non-ASCII characters when encoding the JSON
         * object into its string format.
         * 
         * @param[in] escape
         *      This flag indicates whether or not to escape
         *      non ASCII characters when encoding the JSON
         *      object into its string format.
         *      
         */
        void SetEscapeNonAscii(bool escape);

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