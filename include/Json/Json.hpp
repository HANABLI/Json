#ifndef JSON_JSON_HPP
#define JSON_JSON_HPP

/**
 * @file Json.hpp
 *
 * This module declare the Json::Json class.
 *
 * © 2024 by Hatem Nabli
 */

#include <Utf8/Utf8.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Json
{
    /**
     * This is used to configure various options in
     * JSON object encoding to string format.
     */
    struct JsonEncodingOptions
    {
        /**
         * This flag indicates whether or not to escape
         * non ASCII characters when encoding the JSON
         * object into its string format.
         */
        bool escapeNonAscii = false;

        /**
         * This flag indicates whether or not to disregard
         * any cached encoding when asked to provide an
         * encoding.
         */
        bool reencode = false;

        /**
         * This flag indicates whether or not to add whitespace
         * to line up and indent nested structurs when encoding
         * JSON values.
         */
        bool pretty = false;

        /**
         * If pretty printing is enebled, this is the number of
         * spaces to be intent when an element of an array or
         * object is wrapped to a new line.
         */
        size_t spacesIndentationLevels = 4;

        /**
         * If pretty printing is enabled, this is the maximum
         * length to allow for any line before breaking up a
         * line between elements of an array or object.
         */
        size_t wrapthreshold = 60;

        /**
         * This is the number of levels of nesting under which
         * to assume the JSON value is located. It's used to
         * compute the actual number of spaces to indent in order
         * to show the value's elements at the correct depth.
         */
        size_t numIndentationLevels = 0;
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
        /* data */
        /**
         * These are the different kinds of values of a JSON object.
         */
        enum class Type
        {
            Invalid,
            Null,
            Boolean,
            String,
            Integer,
            Float,
            Array,
            Object
        };

        // Life Cycle Management
    public:
        ~Json();
        Json(const Json&);
        Json(Json&&);
        Json& operator=(const Json&);
        Json& operator=(Json&&);

        // Public methods
    public:
        /**
         * This is the default constructor.
         *
         * @param type
         *      This is the type of the JSON value to create.
         * @note
         *      Setting the type is only useful for invalid, null
         *      and mutable (array and object) types.
         */
        Json(Type type = Type::Invalid);

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
         * This constructs a JSON array containing copies of the
         * elements in the given initializer list.
         *
         * @param[in] args
         *      These are the values to copie in the new array.
         */
        Json(std::initializer_list<const Json> args);

        /**
         * This constructs a JSON array containing copies of the
         * elements in the given initializer list.
         *
         * @param[in] args
         *      These are the values to copie in the new array.
         */
        Json(std::initializer_list<std::pair<std::string, const Json>> args);

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
         * This is the inequality compariosn operator.
         *
         * @param[in] other
         *      This is the object to compare with this one.
         *
         * @return
         *      An indication of whether or not the two JSON objects are not equal
         *      is returned.
         */
        bool operator!=(const Json& other) const;

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
         * This method returns the element at the given index of the JSON value,
         * if it's an array.
         *
         * @param[in] index
         *      This is the position, relative to the front of the array
         *      of the element to return.
         *
         * @return
         *      The element at the given index of the JSON value is returned.
         *
         * @retval nullptr
         *      This is returned if there is no element at the given index
         *      of the JSON value, or if the JSON value isn't an array.
         */
        std::shared_ptr<Json> operator[](size_t index) const;

        /**
         * This method returns the element at the given index of the JSON value,
         * if it's an array.
         *
         * @param[in] index
         *      This is the position, relative to the front of the array
         *      of the element to return.
         *
         * @return
         *      The element at the given index of the JSON value is returned.
         *
         * @retval nullptr
         *      This is returned if there is no element at the given index
         *      of the JSON value, or if the JSON value isn't an array.
         */
        std::shared_ptr<Json> operator[](int index) const;

        /**
         * This method returns the element with the given name in the JSON value,
         * if it's an object.
         *
         * @param[in] key
         *      This is the name of the element to return.
         *
         * @return
         *      The element at the given name in the JSON value is returned.
         *
         * @retval nullptr
         *      This is returned if there is no element at the given name
         *      of the JSON value, or if the JSON value isn't an object.
         */
        std::shared_ptr<Json> operator[](const std::string& key) const;

        /**
         * This method returns the element with the given name in the JSON value,
         * if it's an object.
         *
         * @param[in] key
         *      This is the name of the element to return.
         *
         * @return
         *      The element at the given name in the JSON value is returned.
         *
         * @retval nullptr
         *      This is returned if there is no element at the given name
         *      of the JSON value, or if the JSON value isn't an object.
         */
        std::shared_ptr<Json> operator[](const char* key) const;

        // TODO Look into move semontics for large Json
        // tree assembly to avoid the cost of copy.

        /**
         * This method makes a copy of the given value to be at the end of the array,
         * if the JSON value is an array.
         *
         * @param[in] value
         *      This is the value to copy into the array.
         */
        void Add(const Json& value);

        /**
         * This method makes a copy of the given value to be at the given index of the
         * array, if the JSON value is an array. Any value previously at
         * or after this index are copied down one position.
         *
         * @param[in] value
         *      This is the value to copy into to the array.
         *
         * @param[in] index
         *      This is the position into which to copy the given value.
         */
        void Insert(const Json& value, size_t index);

        /**
         * This method makes a copy of the given value to be in the object
         * under the given key, if the JSON value is an object.
         *
         * @param[in] key
         *      This is the key under which to copy the given value.
         * @param[in] value
         *      This is the value to copy under the given key.
         */
        void Set(const std::string& key, const Json& value);

        /**
         * This method removes the value at the given key in the object,
         * if the JSON value is an object.
         *
         * @param[in] key
         *      This is the key of the value to remove.
         */
        void Remove(const std::string& key);

        /**
         * This method removes the value at the given index of the array,
         * if the JSON value is an array.
         *
         * @param[in] index
         *      This is the position of the value to remove.
         */
        void Remove(size_t index);

        /**
         * This method return the type of the JSON value.
         *
         * @return
         *      the type of the JSON value is returned.
         */
        Type GetType() const;

        /**
         * This method return the size of the JSON value.
         *
         * @return
         *      the size of the JSON value is returned.
         */
        size_t GetSize() const;

        /**
         * This method returns an indication of whether or not
         * the JSON value is an object with an inner value having
         * the given key for a name.
         *
         * @param[in] key
         *      This is the name of the inner value for which to check.
         *
         * @return
         *      an indication of whether or not the JSON value is
         *      an object with an inner value having the given key
         *      for a name is returned.
         */
        bool Has(const std::string& key) const;
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
        std::string ToEncoding(const JsonEncodingOptions& options = JsonEncodingOptions()) const;

        /**
         * This method returns a new JSON object constructed by parsing
         * the JSON object from the given string.
         *
         * @param[in] stringFormat
         *      This is the string format of the JSON object to construct.
         */
        static Json FromEncoding(const std::string& stringFormat);

        /**
         * This method returns a new JSON object constructed by parsing
         * the JSON object from the given string.
         *
         * @param[in] stringFormat
         *      This is the string format of the JSON object to construct.
         */
        static Json FromEncoding(const std::vector<Utf8::UnicodeCodePoint>& stringFormat);

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

    /**
     * This is a support function for Google Test to print out
     * a Json value.
     *
     * @param[in] json
     *      This is the JSON value to print.
     *
     * @param[in] os
     *      This points to the stream to which to print the
     *      json value.
     */
    void PrintTo(const Json& json, std::ostream* os);

    /**
     * This is a support function for Google Test to print out
     * a Json Type.
     *
     * @param[in] json
     *      This is the JSON type to print.
     *
     * @param[in] os
     *      This points to the stream to which to print the
     *      json type value.
     */
    void PrintTo(Json::Type& type, std::ostream* os);

}  // namespace Json

#endif /* JSON_JSON_HPP */