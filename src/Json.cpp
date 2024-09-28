/**
 * @file Json.cpp
 * 
 * This module contains the implementation of
 * the Json::Json class.
 * 
 * © 2024 by Hatem Nabli
 */

#include <map>
#include <vector>
#include <stack>
#include <memory>
#include <Utf8/Utf8.hpp>
#include <Json/Json.hpp>
#include <StringUtils/StringUtils.hpp>

namespace 
{   
    /**
     * Thes are the characters that are considered "whitespace"
     * by the JSON standard (RFC 7159).
     */
    const std::set< Utf8::UnicodeCodePoint> WHITESPACE_CHARACTERS {
        0x20,   // ' '
        0x09,   // '\t'
        0x0D,   // '\r'
        0x0A    // '\n'
    };

    /**
     * These are the characters that must be escaped in a quoted
     * string in JSON.
     */
    const std::map< Utf8::UnicodeCodePoint, Utf8::UnicodeCodePoint > POPULAR_CHARACTERS_ESCAPED {
        { 0x22, 0x22 }, // '"'
        { 0x5C, 0x5C }, // '\\'
        { 0x08, 0x62 }, // '\b'
        { 0x0C, 0x66 }, // '\f'
        { 0x0A, 0x6E }, // '\n'
        { 0x0D, 0x72 }, // '\r'
        { 0x09, 0x74 }, // '\t'
    };
    
    /**
     * This method finds the offset of the first code point in the
     * given vector of code points that is not in the given search 
     * set.
     * 
     * @param[in] codePoints
     *      This is the vector of code points to search.
     * 
     * @param[in] searchSet
     *      This is the set of code points to skip.
     * 
     * @param[in] forwardDirection
     *      This indicates whether or not to search forward rather 
     *      than backward.
     * 
     * @return
     *      The offset of the first code point that isn't in the given
     *      search set of code points is returned.
     * 
     * @retval codePoints.size()
     *      This is returned if all the code points are in the search set.
     */
    size_t FindFirstNotOf(
        const std::vector< Utf8::UnicodeCodePoint >& codePoints,
        const std::set< Utf8::UnicodeCodePoint > searchSet,
        bool forwardDirection
    ) {
        size_t offset = 0;
        while (offset < codePoints.size()) {
            const auto entry = searchSet.find(
                forwardDirection
                ? codePoints[offset]
                : codePoints[codePoints.size() - offset - 1]
            );
            if (entry == searchSet.end()) {
                break;
            }
            ++offset;
        }
        if (offset < codePoints.size()) {
            return forwardDirection ? offset : codePoints.size() - offset - 1;
        } else {
            return offset;
        }
    }

    /**
     * This string returns a string consisting of the four hex digits
     * matching the given code point in hexadecimal.
     * 
     * @param[in] cp
     *      This is the code point to render as four hex digits.
     * 
     * @return
     *      This is the four hex digit rendering of the given code
     *      point.
     */
    std::string CodePointToFourHexDigits(Utf8::UnicodeCodePoint cp) {
        std::string rendering;
        for (size_t i = 0; i < 4; ++i) {
            const auto nibble = ((cp >> ( 4 * (3 - i))) & 0x0F );
            if (nibble < 10) {
                rendering += (char)nibble + '0';
            } else {
                rendering += (char)(nibble - 10) + 'A';
            }
        }
        return rendering;
    }

    /**
     * This function produces the escaped version of the given string.
     * 
     * @param[in] s
     *      This is the string which needs to be escaped.
     * 
     * @param[in] options
     *      This is used to configure various options in 
     *      JSON object encoding to string format.
     * 
     * @return
     *      The escaped string is returned.
     */
    std::string Escape(
        const std::string& s, 
        const Json::JsonEncodingOptions& options
    ) {
        Utf8::Utf8 encoderDecoder;
        std::string output;
        for (const auto cp: encoderDecoder.Decode(s)) {
            if (
                (cp == 0x22)
                || (cp == 0x5C)
                || (cp < 0x20)
            ) {
                output += '\\';
                const auto entry = POPULAR_CHARACTERS_ESCAPED.find(cp);
                if (entry == POPULAR_CHARACTERS_ESCAPED.end()) {
                    output += 'u';
                    output += CodePointToFourHexDigits(cp);
                } else {
                    output += (char)entry->second;
                }
            } else if (
                 options.escapeNonAscii
                 && (cp > 0x7F)
            ) {
                if (cp > 0xFFFF) {
                    output += "\\u";
                    output += CodePointToFourHexDigits(0xD800 + (((cp - 0x10000) >> 10) & 0x3FF));
                    output += "\\u";
                    output += CodePointToFourHexDigits(0xDC00 + ((cp -0x10000) & 0x3FF));
                } else {
                    output += "\\u";
                    output += CodePointToFourHexDigits(cp);
                }
            } else {
                const auto encoding = encoderDecoder.Encode({cp});
                output += std::string(
                    encoding.begin(),
                    encoding.end()
                );
            }
        }
        return output;
    }

    /**
     * This si the unescaped version of the given string.
     * 
     * @param[in] s
     *      This is the string which needs to be unescaped.
     * @param[out] output
     *      This is where to put the unscaped string.
     * 
     * @return
     *      An indication of whether or not the input string was a valid
     *      encoding is returned.
     */
    bool Unescape(const std::string& s, std::string& output) {
        Utf8::Utf8 encoderDecoder;
        Utf8::UnicodeCodePoint cpFromHexDigits = 0;
        Utf8::UnicodeCodePoint firstHalfOfSurrogatePair = 0;
        std::vector< Utf8::UnicodeCodePoint > hexDigitOriginal;
        size_t state = 0;
        for (const auto cp: encoderDecoder.Decode(s)) {
            switch (state)
            {
                case 0: { // initial state 
                    if((0x5C == cp)) {
                        state = 1;
                    } else if (firstHalfOfSurrogatePair == 0) {
                        const auto encoding = encoderDecoder.Encode({cp});
                        output += std::string(
                            encoding.begin(),
                            encoding.end()
                        );
                    } else {
                        return false;
                    }
                } break;

                case 1: { // escape character
                    if (cp == 0x75) {
                        state = 2;
                        cpFromHexDigits = 0;
                        hexDigitOriginal = { 0x5C, 0x75 };
                    } else if (firstHalfOfSurrogatePair == 0) {
                        Utf8::UnicodeCodePoint alterantive = cp;
                        const auto entry = POPULAR_CHARACTERS_ESCAPED.find(cp);
                        if (entry == POPULAR_CHARACTERS_ESCAPED.end()) {
                            return false;
                        } else {
                            const auto encoding = encoderDecoder.Encode({entry->second});
                            output += std::string(
                                encoding.begin(),
                                encoding.end()
                            );
                        }
                        state = 0;
                    } else {
                        return false;
                    }
                } break;

                case 2: // first hexdigit of escape uXXXX
                case 3: // seconde hexdigit of escape uXXXX
                case 4: // third hexdigit of escape uXXXX
                case 5: { // fourth hexdigit of \\uXXXX
                    hexDigitOriginal.push_back(cp);
                    cpFromHexDigits <<= 4;
                    if (
                        (cp >= (Utf8::UnicodeCodePoint)'0')
                    && (cp <= (Utf8::UnicodeCodePoint)'9')
                    ) {
                        cpFromHexDigits += (cp - (Utf8::UnicodeCodePoint)'0');
                    } else if (
                        (cp >= (Utf8::UnicodeCodePoint)'A')
                        && (cp <= (Utf8::UnicodeCodePoint)'F')
                    ) {
                        cpFromHexDigits += (cp - (Utf8::UnicodeCodePoint)'A' + 10);
                    } else if (
                        (cp >= (Utf8::UnicodeCodePoint)'a')
                        && (cp <= (Utf8::UnicodeCodePoint)'f')
                    ) {
                        cpFromHexDigits += (cp - (Utf8::UnicodeCodePoint)'a' + 10);
                    } else {
                        return false;
                    }
                    if (++state == 6) {
                        state = 0;
                        if (
                            (cpFromHexDigits >= 0xd800)
                            && (cpFromHexDigits <= 0xDFFF)
                        ) {
                            if (firstHalfOfSurrogatePair == 0) {
                                firstHalfOfSurrogatePair = cpFromHexDigits;
                            } else {
                                const auto secondHalfOfSurrogatePair = cpFromHexDigits;
                                const auto encoding = encoderDecoder.Encode({
                                    ((firstHalfOfSurrogatePair - 0xD800) << 10)
                                    +  (secondHalfOfSurrogatePair - 0xDC00)
                                    + 0x10000   
                                });
                                output += std::string(
                                    encoding.begin(),
                                    encoding.end()
                                );
                                firstHalfOfSurrogatePair = 0;
                            }
                        } else if (firstHalfOfSurrogatePair == 0) {
                            const auto encoding = encoderDecoder.Encode({cpFromHexDigits});
                            output += std::string(
                                encoding.begin(),
                                encoding.end()
                            );
                        } else {
                            return false;
                        }
                    }
                } break;
            }          
        }
        return (
            (state != 1)
            && (
                (state < 2)
                || (state > 5)
            )
        );
    }
} // namespace 

namespace Json{

    struct Json::Impl
    {

        /**
         * This indicates the type of the value represented
         * by the JSON object.
         */
        Type type = Type::Invalid;


        union 
        { 
            bool booleanValue;
            std::string* stringValue;
            std::vector< std::shared_ptr< Json >>* arrayValue;
            std::map< std::string, std::shared_ptr< Json >>* objectValue;
            intmax_t integerValue;
            double floatValue;
        };
        
        /**
         * This is a cache of the encoding of the value.
         */
        std::string encoding;

        // Lifecycle management

        ~Impl() {
            switch (type)
            {
                case Type::String: {
                    delete stringValue;
                } break;

                case Type::Array: {
                    delete arrayValue;
                } break;

                case Type::Object: {
                    delete objectValue;
                } break;

                default:
                    break;
            }
        }
        Impl(const Impl&) = delete;
        Impl(Impl&&) = delete;
        Impl& operator=(const Impl&) = delete;
        Impl& operator=(Impl&&) = delete;

        // Methods

        /**
         * This is the default constructor.
         */
        Impl() = default;

        /**
         * This method builds the Json value up as a copy
         * of another Json value.
         * 
         * @param[in] other 
         *      This is the other json value to copy.
         */
        void CopyFrom(const std::unique_ptr< Impl >& other) {
            type = other->type;
            switch (type) {
                case Type::Boolean: {
                    booleanValue = other->booleanValue;
                } break;

                case Type::Integer: {
                    integerValue = other->integerValue;
                } break;

                case Type::Float: {
                    floatValue = other->floatValue;
                } break;

                case Type::String: {
                    stringValue = new std::string(*other->stringValue);
                } break;

                case Type::Array: {
                    arrayValue = new std::vector< std::shared_ptr< Json >>;
                    arrayValue->reserve(other->arrayValue->size());
                    for (const auto& otherElement: *other->arrayValue) {
                        const auto copy = std::make_shared< Json >(*otherElement);
                        arrayValue->push_back(copy);
                    }
                } break;

                case Type::Object: {
                    objectValue = new std::map< std::string, std::shared_ptr< Json >>;
                    for (const auto& otherElement: *other->objectValue) {
                        const auto copy = std::make_shared< Json >(*otherElement.second);
                        (*objectValue)[otherElement.first] = copy;
                    }
                } break;

                default: break;
            }
        }
        
        /**
         * This function parses the given unicode points as an integer JSON value.
         * 
         * @param[in] codePoints
         *      This is the unicode points vector to parse.
         */
        void ParseToInteger(const std::vector< Utf8::UnicodeCodePoint >& codePoints) {
            size_t index = 0;
            size_t state = 0;
            bool negative = false;
            int value = 0;
            while (index < codePoints.size()) {
                switch (state)
                {
                    case 0: {
                        if (codePoints[index] == (Utf8::UnicodeCodePoint)'-') {
                            negative = true;
                            ++index;
                        }
                        state = 1;
                    } break;

                    case 1: { // zero / 1-9
                        if (codePoints[index] == (Utf8::UnicodeCodePoint)'0') {
                            state = 2;
                        } else if (
                            (codePoints[index] >= (Utf8::UnicodeCodePoint)'1')
                            && (codePoints[index] <= (Utf8::UnicodeCodePoint)'9')
                        ) {
                            state = 3;
                            value = (int)(codePoints[index] - (Utf8::UnicodeCodePoint)'0');
                        } else {
                            return;
                        }
                        ++index;
                    } break;

                    case 2: {
                        return;
                    } break;

                    case 3: {
                        if (
                            (codePoints[index] >= (Utf8::UnicodeCodePoint)'0')
                            && (codePoints[index] <= (Utf8::UnicodeCodePoint)'9')
                        ) {
                            const int previousValue = value;
                            value *= 10;
                            value += (int)(codePoints[index] - (Utf8::UnicodeCodePoint)'0');
                            if (value / 10 != previousValue) {
                                return;
                            }
                            ++index;
                        } else {
                            return;
                        }
                    } break;
                }
            }
            if (state >= 2) {
                type = Type::Integer;
                integerValue = (value * (negative ? -1 : 1));
            }       
        }
        /**
         * This function parses the given code Points as a floating point JSON value.
         * 
         * 
         * @param[in] codePoints
         *      This is the code points to parse.
         * 
         */
        void ParseFloatingPoint(const std::vector< Utf8::UnicodeCodePoint >& codePoints) {
            size_t index = 0;
            size_t state = 0;
            bool negativeMagnitude = false;
            bool negativeExponent = false;
            double magnitude = 0.0;
            double exponent = 0.0;
            double fraction = 0.0;
            size_t fractionDigits = 0;
            while (index < codePoints.size()) {
                switch (state)
                {
                    case 0: {
                        if (codePoints[index] == (Utf8::UnicodeCodePoint)'-') {
                            negativeMagnitude = true;
                            ++index;
                        }
                        state = 1;
                    } break;

                    case 1: { // zero / 1-9
                        if (codePoints[index] == (Utf8::UnicodeCodePoint)'0') {
                            state = 2;
                        } else if (
                            (codePoints[index] >= (Utf8::UnicodeCodePoint)'1')
                            && (codePoints[index] <= (Utf8::UnicodeCodePoint)'9')
                        ) {
                            state = 3;
                            magnitude = (double)(codePoints[index] - (Utf8::UnicodeCodePoint)'0');
                        } else {
                            return;
                        }
                        ++index;
                    } break;

                    case 2: {
                        if (codePoints[index] == (Utf8::UnicodeCodePoint)'.') {
                            state = 4;
                        } else if (
                            (codePoints[index] == (Utf8::UnicodeCodePoint)'e')
                            || (codePoints[index] == (Utf8::UnicodeCodePoint)'E')
                        ) {
                            state = 6;
                        } else {
                            return;
                        }
                        ++index;    
                    } break;

                    case 3: { // *DIGIT / . / e / E
                        if (
                            (codePoints[index] >= (Utf8::UnicodeCodePoint)'0')
                            && (codePoints[index] <= (Utf8::UnicodeCodePoint)'9')
                        ) {
                            const auto oldMagnitude = (intmax_t)magnitude;
                            magnitude *= 10.0;
                            magnitude += (double)(codePoints[index] - (Utf8::UnicodeCodePoint)'0');
                            if ((intmax_t)magnitude / 10 != oldMagnitude) {
                                return;
                            }
                        } else if (codePoints[index] == (Utf8::UnicodeCodePoint)'.') {
                            state = 4;
                        } else if (
                            (codePoints[index] == (Utf8::UnicodeCodePoint)'e') 
                            || (codePoints[index] == (Utf8::UnicodeCodePoint)'E')
                        ) {
                            state = 6;
                        } else {
                            return;
                        }
                        ++index;
                    } break;

                    case 4: { // frac: DIGIT
                        if (
                            (codePoints[index] >= (Utf8::UnicodeCodePoint)'0')
                            && (codePoints[index] <= (Utf8::UnicodeCodePoint)'9')
                        ) {
                            ++fractionDigits;
                            fraction += (
                                (double)(
                                    codePoints[index] - (Utf8::UnicodeCodePoint)'0' 
                                ) / pow(10.0, (double)fractionDigits)
                            );
                        } else {
                            return;
                        }
                        state = 5;
                        ++index;
                    } break;

                    case 5: { // frac: *DIGIT / e / E
                        if (
                            (codePoints[index] >= (Utf8::UnicodeCodePoint)'0')
                            && (codePoints[index] <= (Utf8::UnicodeCodePoint)'9')
                        ) {
                            ++fractionDigits;
                            fraction += (
                                (double)(
                                    codePoints[index] - (Utf8::UnicodeCodePoint)'0'
                                ) / pow(10.0, (double)fractionDigits)
                            );
                        } else if (
                            (codePoints[index] == (Utf8::UnicodeCodePoint)'e') 
                            || (codePoints[index] == (Utf8::UnicodeCodePoint)'E')
                        ) {
                            state = 6;
                        } else {
                            return;
                        }
                        ++index;
                    } break;

                    case 6: { // exp: [minus/plus] / DIGIT
                        if (codePoints[index] == (Utf8::UnicodeCodePoint)'-') {
                            negativeExponent = true;
                            ++index;
                        } else if (codePoints[index] == (Utf8::UnicodeCodePoint)'+') {
                            ++index;
                        } else {

                        }
                        state = 7;
                    } break;

                    case 7: { // exp: DIGIT
                         if (
                            (codePoints[index] >= (Utf8::UnicodeCodePoint)'0')
                            && (codePoints[index] <= (Utf8::UnicodeCodePoint)'9')
                        ) {
                            const auto oldExponent = (intmax_t)exponent;
                            exponent *= 10.0;
                            exponent += (double)(codePoints[index] - (Utf8::UnicodeCodePoint)'0');
                            if ((intmax_t)exponent / 10 != oldExponent) {
                                return;
                            }
                        } else {
                            return;
                        }
                        ++index;
                    } break;
                }
            }
            if (
                (state >= 2)
                && (state != 4)
                && (state != 6)
            ) {
                type = Type::Float;
                floatValue = (
                    (
                        magnitude
                        + fraction
                    )
                    * pow(10.0, exponent * (negativeExponent ? -1.0 : 1.0)) 
                    * (negativeExponent ? -1.0 : 1.0)
                );
            }       
        }

        /**
         * This function extracts the encoding of the next JSON value
         * in the given string, updating the given offset to indicate
         * where the end of the value encoding was found, or whether
         * the encoding was invalid.
         * 
         * @param[in] codePoints
         *      This is the given unicode points vector to parse.
         * @param[in, out] offset
         *      On input, this is the position of the first character
         *      of the encoded value in the unicode points vector.
         *      
         *      On output, this is the position of the first character
         *      past the end of the encoded value in the unicode points 
         *      vector
         * @param[in] delimiter
         *      This is the character that marks the end of the value
         *      if it's encountered.
         * 
         * @return {}
         *      This is returned if the encoded value was invalid. 
         */
        std::vector< Utf8::UnicodeCodePoint > ParseValue(const std::vector< Utf8::UnicodeCodePoint >& codePoints, size_t& offset, char delimiter) {
            std::stack< char > expectedDelimiters; 
            std::vector< Utf8::UnicodeCodePoint > encodedValueCodePoints; 
            const std::vector< Utf8::UnicodeCodePoint > encodingCodePoints(codePoints.begin() + offset, codePoints.end());
            if (encodingCodePoints.empty()) {
                offset = std::string::npos;
                return {};
            }
            bool insideString = false;
            for (const auto cp:encodingCodePoints) {
                encodedValueCodePoints.push_back(cp);
                if (
                    !expectedDelimiters.empty()
                    && (cp == expectedDelimiters.top())
                ) {
                    insideString = false;
                    expectedDelimiters.pop();
                    continue;
                }
                if (!insideString) {
                    if (cp == (Utf8::UnicodeCodePoint)'\"') {
                        insideString = true;
                        expectedDelimiters.push('\"');
                    } else if (cp == (Utf8::UnicodeCodePoint)'[') {
                        expectedDelimiters.push(']');
                    } else if (
                        (cp == (Utf8::UnicodeCodePoint)delimiter)
                        && expectedDelimiters.empty()
                    ) { 
                        break;
                    }
                }
            }
            if (expectedDelimiters.empty()) {
                offset += encodedValueCodePoints.size();
                if (encodedValueCodePoints.back() == (Utf8::UnicodeCodePoint)delimiter) {
                    encodedValueCodePoints.pop_back();
                }
                return encodedValueCodePoints;
            } else {
                return {};
            }
        }

        /**
         * This function parses the given string as an array
         * JSON value.
         * 
         * @param[in] s
         *      This is the given string to parse.
         */
        void ParseAsArray(const std::vector< Utf8::UnicodeCodePoint >& codePoints) {   
            size_t offset = 0;
            std::vector< std::shared_ptr< Json > > newArrayValue;
            while (offset < codePoints.size()) {
                const auto encodedValue = ParseValue(codePoints, offset, ',');
                if (offset == std::string::npos) {
                    return;
                }
                const auto value = std::make_shared< Json >(FromEncoding(encodedValue));
                newArrayValue.push_back(value);
            }
            type = Type::Array;
            arrayValue = new decltype(newArrayValue)(newArrayValue);
        }

        /**
         * This function parses the given string as un object
         * JSON value.
         * 
         * @param[in] s
         *   This is the string to parse 
         */
        void ParseAsObject(const std::vector< Utf8::UnicodeCodePoint >& codePoints) {
            std::map< std::string, std::shared_ptr< Json > > newObjectValue;
            size_t offset = 0;
            while (offset < codePoints.size()) {
                const auto encodedKey = ParseValue(codePoints, offset, ':');
                if (offset == std::string::npos) {
                    return;
                }
                const auto key = std::make_shared< Json >(FromEncoding(encodedKey));
                if (key->GetType() != Type::String) {
                    return;
                }
                const auto encodedValue = ParseValue(codePoints, offset, ',');
                if (offset == std::string::npos) {
                    return;
                }
                const auto value = std::make_shared< Json >(FromEncoding(encodedValue));
                newObjectValue[(std::string)*key] = value;
            }
            type = Type::Object;
            objectValue = new decltype(newObjectValue)(newObjectValue);
            
        }
    };

    Json::~Json() = default;
    Json::Json(const Json& other) : impl_(new Impl) {
        impl_->CopyFrom(other.impl_);
    }
    Json::Json(Json&&) = default;
    Json& Json::operator=(const Json& other) {
        if (this != &other) {
            impl_.reset(new Impl());
            impl_->CopyFrom(other.impl_);
        }
        return *this;
    }
    Json& Json::operator=(Json&&)= default; 

    Json::Json(Type type): impl_(new Impl) { 
        impl_->type = type;
        switch (type) {
            case Type::String: {
                impl_->stringValue = new std::string();
            } break;

            case Type::Array: {
                impl_->arrayValue = new std::vector< std::shared_ptr< Json >>;
            } break;

            case Type::Object: {
                impl_->objectValue = new std::map< std::string, std::shared_ptr< Json >>;
            } break;

            default: break;
        }
    }
    
    Json::Json(nullptr_t): impl_(new Impl) {
        impl_->type = Type::Null;
    }

    Json::Json(bool value): impl_(new Impl) {
        impl_->type = Type::Boolean;
        impl_->booleanValue = value;
    }

    Json::Json(int value): impl_(new Impl) {
        impl_->type = Type::Integer;
        impl_->integerValue = value;
    }

    Json::Json(double value): impl_(new Impl) {
        impl_->type = Type::Float;
        impl_->floatValue = value;
    }

    Json::Json(const std::string& value): impl_(new Impl) {
        impl_->type = Type::String;
        impl_->stringValue = new std::string(value);
    }

    Json::Json(const char* value): impl_(new Impl) {
         impl_->type = Type::String;
        impl_->stringValue = new std::string(value);
    }

    bool Json::operator==(const Json& other) const {
        if (impl_->type != other.impl_->type) {
            return false;
        } else switch (impl_->type)
        {
            case Type::Invalid:
                return true;
                break;
            case Type::Null:
                return true;
                break;
            case Type::Boolean:
                return impl_->booleanValue == other.impl_->booleanValue;
                break;
            case Type::String: 
                return *impl_->stringValue == *other.impl_->stringValue;
                break;
            case Type::Integer:
                return impl_->integerValue == other.impl_->integerValue;
                break;
            case Type::Float:
                return impl_->floatValue == other.impl_->floatValue;
                break;
            case Type::Array: {
                if (impl_->arrayValue->size() != other.impl_->arrayValue->size()) {
                    return false;
                } 
                for (size_t i = 0; i < impl_->arrayValue->size(); ++i) {
                    if (*(*impl_->arrayValue)[i] != *(*other.impl_->arrayValue)[i]) {
                        return false;
                    }
                }
            } return true;
            case Type::Object: {
                std::set< std::string > keys;
                for (const auto& entry: *impl_->objectValue) {
                    (void)keys.insert(entry.first);
                }
                for (const auto& entry: *other.impl_->objectValue) {
                    const auto otherEntry = keys.find(entry.first);
                    if (otherEntry == keys.end()) {
                        return false;
                    }
                    (void)keys.erase(entry.first);
                }
                if (!keys.empty()) {
                    return false;
                }
                for (auto it = impl_->objectValue->begin();
                    it != impl_->objectValue->end();
                    ++it
                ) {
                    if (*it->second != *(*other.impl_->objectValue)[it->first]) {
                        return false;
                    }
                }
            } return true;

            default:
                return true;
                break;
        }
    }

    bool Json::operator!=(const Json& other) const {
        return !(*this == other);
    }

    Json::operator bool() const {
        if (impl_->type == Type::Boolean) {
            return impl_->booleanValue;
        } else {
            return false;
        }
    }

    Json::operator int() const {
        if (impl_->type == Type::Integer) {
            return impl_->integerValue;
        } else if (impl_->type == Type::Float) {
            return (int)impl_->floatValue;
        } else {
            return 0;
        }
    }

    Json::operator double() const {
        if (impl_->type == Type::Integer) {
            return (double)impl_->integerValue;
        } else if (impl_->type == Type::Float) {
            return impl_->floatValue;
        } else {
            return 0.0;
        }
    }

    Json::operator std::string() const {
        if (impl_->type == Type::String) {
            return *impl_->stringValue;
        } else {
            return "";
        }
    }

    std::shared_ptr< Json > Json::operator[](size_t index) const {
        if (impl_->type == Type::Array) {
            if (index >= impl_->arrayValue->size()) {
                return nullptr;
            }
            return (*impl_->arrayValue)[index];
        } else {
            return nullptr;
        }   
    }

    std::shared_ptr< Json > Json::operator[](int index) const {
        return (*this)[(size_t)index];
    }

    std::shared_ptr< Json > Json::operator[](const std::string& key) const {
        if (impl_->type == Type::Object) {
            const auto entry = impl_->objectValue->find(key);
            if (entry == impl_->objectValue->end()) {
                return nullptr;
            }
            return entry->second;
        } else {
            return nullptr;
        }   
    }

    std::shared_ptr< Json > Json::operator[](const char* key) const {
        return (*this)[std::string(key)];
    }

    void Json::Add(const Json& value) {
        if (impl_->type != Type::Array) {
            return;
        } 
        Insert(value, impl_->arrayValue->size());
    }

    void Json::Insert(const Json& value, size_t index) {
        if (impl_->type != Type::Array) {
            return;
        }
        (void)impl_->arrayValue->insert(
            impl_->arrayValue->begin() 
            + std::min(index, impl_->arrayValue->size()
            ), 
            std::make_shared< Json >(value)
        );
        //TODO: clear the encoding
    }

    void Json::Set(const std::string& key, const Json& value) {
        if (impl_->type != Type::Object) {
            return;
        }
        (*impl_->objectValue)[key] = std::make_shared<Json>(value);
        //TODO: clear the encoding
    }

    void Json::Remove(const std::string& key) {
        if (impl_->type != Type::Object) {
            return;
        }
        (void)impl_->objectValue->erase(key);
        //TODO: clear the encoding
    }

    void Json::Remove(size_t index) {
        if (impl_->type != Type::Array) {
            return;
        }
        if (index < impl_->arrayValue->size()) {
            impl_->arrayValue->erase(impl_->arrayValue->begin() + index);
        }
        //TODO: clear the encoding
    }

    auto Json::GetType() const -> Type  {
        return impl_->type;
    }

    size_t Json::GetSize() const {
        if (impl_->type == Type::Array) {
            return impl_->arrayValue->size();
        } else if (impl_->type == Type::Object) {
            return impl_->objectValue->size();
        } else {
            return 0;
        }
    }

    bool Json::Has(const std::string& key) const {
        if (impl_->type == Type::Object) {
            return (impl_->objectValue->find(key) != impl_->objectValue->end());
        } else {
            return false;
        }   
    }

    std::string Json::ToEncoding(const JsonEncodingOptions& options) const {
        if (impl_->type == Type::Invalid) {
            return StringUtils::sprintf(
                "(Invalid JSON: %s)",
                impl_->encoding.c_str()
            );
        }
        if (options.reencode) {
            impl_->encoding.clear();
        }
        if(impl_->encoding.empty()) {
            switch (impl_->type)
            {
            case Type::Null:
                impl_->encoding = "null";
                break;
            case Type::Boolean:
                impl_->encoding = impl_->booleanValue ? "true" : "false";
                break;
            case Type::String:
                impl_->encoding = ( "\"" + Escape(*impl_->stringValue, options) + "\"");
                break;
            case Type::Integer:
                impl_->encoding = StringUtils::sprintf("%i", impl_->integerValue);
                break;
            case Type::Float:
                impl_->encoding = StringUtils::sprintf("%lg", impl_->floatValue);
                break;
            case Type::Array: {
                impl_->encoding = '[';
                bool isFirst = true;
                auto nestedOption = options;
                ++nestedOption.numIndentationLevels;
                std::string nestedIndentation((nestedOption.numIndentationLevels * nestedOption.spacesIndentationLevels), ' ');
                std::string wrappedEncoding = "[\r\n";
                for (const auto value: *impl_->arrayValue) {
                    if (isFirst) {
                        isFirst = false;
                    } else {
                        impl_->encoding += (nestedOption.pretty? ", " : ",");
                        wrappedEncoding += ",\r\n";
                    }
                    const auto encodingValue = value->ToEncoding(nestedOption);
                    impl_->encoding += encodingValue;
                    wrappedEncoding += nestedIndentation;
                    wrappedEncoding += encodingValue;
                }
                impl_->encoding += ']';
                std::string indentation(
                    (
                        options.numIndentationLevels
                        * options.spacesIndentationLevels
                    ),
                    ' '
                );
                wrappedEncoding += "\r\n";
                wrappedEncoding += indentation;
                wrappedEncoding += "]";
                if (
                    options.pretty
                    && (indentation.length() + impl_->encoding.length() > options.wrapthreshold)
                ) {
                    impl_->encoding = wrappedEncoding;
                }
            } break;
            case Type::Object: {
                impl_->encoding = '{';
                bool isFirst = true;
                auto nestedOption = options;
                ++nestedOption.numIndentationLevels;
                std::string nestedIndentation(
                    (
                        nestedOption.numIndentationLevels
                        * nestedOption.spacesIndentationLevels
                    ),
                    ' '
                );
                std::string wrappedEncoding = "{\r\n";    
                for (const auto& entry: *impl_->objectValue) {
                    if (isFirst) {
                        isFirst = false;
                    } else {
                        impl_->encoding += (nestedOption.pretty? ", " : ",");
                        wrappedEncoding += ",\r\n";
                    }
                    const Json keyAsJson(entry.first);
                    const auto encodedValue = (
                        keyAsJson.ToEncoding(nestedOption)
                        + (nestedOption.pretty ? ": " : ":")
                        + entry.second->ToEncoding(nestedOption)
                    );
                    impl_->encoding += encodedValue;
                    wrappedEncoding += nestedIndentation;
                    wrappedEncoding += encodedValue;
                }
                impl_->encoding += '}';
                std::string indentation(
                    (options.numIndentationLevels
                    * options.spacesIndentationLevels),
                    ' '
                );
                wrappedEncoding += "\r\n";
                wrappedEncoding += indentation;
                wrappedEncoding += "}";
                if (options.pretty 
                    && (indentation.length() + impl_->encoding.length() > options.wrapthreshold)
                ) {
                    impl_->encoding = wrappedEncoding;
                }
            } break;
            default:
                impl_->encoding = "???";
                break;
            }
        }
        return impl_->encoding;
    }

    Json Json::FromEncoding(const std::string& stringFormat) {       
        Utf8::Utf8 utf8Decoder;
        return FromEncoding(utf8Decoder.Decode(stringFormat));
    }

     Json Json::FromEncoding(const std::vector< Utf8::UnicodeCodePoint >& encodigFormatNotTrim) {       
        Json json;
        const auto firstNonWhitespaceChar = FindFirstNotOf(encodigFormatNotTrim, WHITESPACE_CHARACTERS, true);
        if (firstNonWhitespaceChar == encodigFormatNotTrim.size()) {
            return json;
        }
        const auto lastNonWhitespaceChar = FindFirstNotOf(encodigFormatNotTrim, WHITESPACE_CHARACTERS, false);
        const std::vector< Utf8::UnicodeCodePoint> encoding(
            encodigFormatNotTrim.begin() + firstNonWhitespaceChar,
            encodigFormatNotTrim.begin() + lastNonWhitespaceChar + 1
        );
        Utf8::Utf8 utf8;
        const auto encodingUtf8 = utf8.Encode(encoding);
        json.impl_->encoding = std::string(encodingUtf8.begin(), encodingUtf8.end()); 
        if (encoding.empty()) {
            
        } else if (
            ! encoding.empty()
            && (encoding[0] == '{')
            && (encoding[encoding.size() - 1] == '}')
        ) {
            json.impl_->ParseAsObject(
                std::vector< Utf8::UnicodeCodePoint >(
                    encoding.begin() + 1,
                    encoding.begin() + encoding.size() - 1
                )
            );          
        } else if (
            !encoding.empty()
            && (encoding[0] == '[')
            && (encoding[encoding.size() - 1] == ']')
        ) {
            json.impl_->ParseAsArray(
                std::vector< Utf8::UnicodeCodePoint >(
                    encoding.begin() + 1,
                    encoding.begin() + encoding.size() - 1
                )
            );
        } else if (
            !encoding.empty()
            && (encoding[0] == '"')
            && (encoding[encoding.size() - 1] == '"')
        ) {
            std::string output;
            Utf8::Utf8 utf8;
            const auto utf8EncodingString = utf8.Encode(
                std::vector< Utf8::UnicodeCodePoint >(
                    encoding.begin() + 1,
                    encoding.begin() + encoding.size() - 1
                )
            );
            if (
                Unescape(std::string(utf8EncodingString.begin(), utf8EncodingString.end()), output)
            ) {
                json.impl_->type = Type::String;
                json.impl_->stringValue = new std::string(output);
            } 
        } else if (json.impl_->encoding == "null") {
            json.impl_->type = Type::Null;
        } else if (json.impl_->encoding == "true") {
            json.impl_->type = Type::Boolean;
            json.impl_->booleanValue = true;
        } else if  (json.impl_->encoding == "false") {
            json.impl_->type = Type::Boolean;
            json.impl_->booleanValue = false;
        } else {
            if (json.impl_->encoding.find_first_of(".eE") != std::string::npos) {
                // TODO: parse as floating point
                json.impl_->ParseFloatingPoint(encoding);
            } else {
                json.impl_->ParseToInteger(encoding);
            }
        }
        return json; 
    }

    void PrintTo(
        Json::Type& type,
        std::ostream* os
    ) {
        switch (type)
        {
        case Json::Type::Invalid:
            *os << "Invalid";
            break;
        case Json::Type::Boolean:
            *os << "Boolean";
            break;
        case Json::Type::Null:
            *os << "Null";
            break;
        case Json::Type::Integer:
            *os << "Integer";
            break;
        case Json::Type::Float:
            *os << "Float";
            break;
        case Json::Type::String:
            *os << "String";
            break;
        case Json::Type::Array:
            *os << "Array";
            break;
        case Json::Type::Object:
            *os << "Object";
            break;
        default:
            *os << "???";
            break;
        }
    }

    void PrintTo(
        const Json& json,
        std::ostream* os
    ) {
        *os << json.ToEncoding();
    }
}
