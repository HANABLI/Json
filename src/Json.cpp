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
    constexpr const char* WHITESPACE_CHARACTERS = " \t\r\n";

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
        Utf8::Utf8 decoder, encoder;
        std::string output;
        for (const auto cp: decoder.Decode(s)) {
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
                const auto encoding = encoder.Encode({cp});
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
        Utf8::Utf8 decoder, encoder;
        Utf8::UnicodeCodePoint cpFromHexDigits = 0;
        Utf8::UnicodeCodePoint firstHalfOfSurrogatePair = 0;
        std::vector< Utf8::UnicodeCodePoint > hexDigitOriginal;
        size_t state = 0;
        for (const auto cp: decoder.Decode(s)) {
            switch (state)
            {
                case 0: { // initial state 
                    if((0x5C == cp)) {
                        state = 1;
                    } else if (firstHalfOfSurrogatePair == 0) {
                        const auto encoding = encoder.Encode({cp});
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
                            const auto encoding = encoder.Encode({entry->second});
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
                                const auto encoding = encoder.Encode({
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
                            const auto encoding = encoder.Encode({cpFromHexDigits});
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
            int integerValue;
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
         * This function parses the given string as an integer JSON value
         * 
         * @param[in] s
         *      This is the string to parse.
         */
        void ParseToInteger(const std::string& s) {
            size_t index = 0;
            size_t state = 0;
            bool negative = false;
            int value = 0;
            while (index < s.length()) {
                switch (state)
                {
                    case 0: {
                        if (s[index] == '-') {
                            negative = true;
                            ++index;
                        }
                        state = 1;
                    } break;

                    case 1: { // zero / 1-9
                        if (s[index] == '0') {
                            state = 2;
                        } else if (
                            (s[index] >= '1')
                            && (s[index] <= '9')
                        ) {
                            state = 3;
                            value = (int)(s[index] - '0');
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
                            (s[index] >= '0')
                            && (s[index] <= '9')
                        ) {
                            const int previousValue = value;
                            value *= 10;
                            value += (int)(s[index] - '0');
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
         * This function parses the given string as a floating point JSON value
         * 
         * 
         * @param[in] s
         *      This is the string to parse.
         * 
         */
        void ParseFloatingPoint(const std::string& s) {
            size_t index = 0;
            size_t state = 0;
            bool negativeMagnitude = false;
            bool negativeExponent = false;
            double magnitude = 0;
            double exponent = 0.0;
            double fraction = 0.0;
            size_t fractionDigits = 0;
            while (index < s.length()) {
                switch (state)
                {
                    case 0: {
                        if (s[index] == '-') {
                            negativeMagnitude = true;
                            ++index;
                        }
                        state = 1;
                    } break;

                    case 1: { // zero / 1-9
                        if (s[index] == '0') {
                            state = 2;
                        } else if (
                            (s[index] >= '1')
                            && (s[index] <= '9')
                        ) {
                            state = 3;
                            magnitude = (int)(s[index] - '0');
                        } else {
                            return;
                        }
                        ++index;
                    } break;

                    case 2: {
                        return;
                    } break;

                    case 3: { // *DIGIT / . / e / E
                        if (
                            (s[index] >= '0')
                            && (s[index] <= '9')
                        ) {
                            magnitude *= 10.0;
                            magnitude += (double)(s[index] - '0');
                        } else if (s[index] == '.') {
                            state = 4;
                        } else if ((s[index] == 'e') || (s[index] == 'E')) {
                            state = 6;
                        } else {
                            return;
                        }
                        ++index;
                    } break;

                    case 4: { // frac: DIGIT
                        if (
                            (s[index] >= '0')
                            && (s[index] <= '9')
                        ) {
                            ++fractionDigits;
                            fraction += (double)(s[index] - '0') / pow(10.0, (double)fractionDigits);
                        } else {
                            return;
                        }
                        state = 5;
                        ++index;
                    } break;

                    case 5: { // frac: *DIGIT / e / E
                        if (
                            (s[index] >= '0')
                            && (s[index] <= '9')
                        ) {
                            ++fractionDigits;
                            fraction += (double)(s[index] - '0') / pow(10.0, (double)fractionDigits);
                        } else if (
                            (s[index] == 'e') 
                            || (s[index] == 'E')
                        ) {
                            state = 6;
                        } else {
                            return;
                        }
                        ++index;
                    } break;

                    case 6: { // exp: [minus/plus] / DIGIT
                        if (s[index] == '-') {
                            negativeExponent = true;
                            ++index;
                        } else if (s[index] == '+') {
                            ++index;
                        } else {

                        }
                        state = 7;
                    } break;

                    case 7: { // exp: DIGIT
                        state = 8;
                    } break;

                    case 8: { // exp: *DIGIT
                        if (
                            (s[index] >= '0')
                            && (s[index] <= '9')
                        ) {
                            exponent *= 10.0;
                            exponent += (double)(s[index] - '0');
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
                && (state != 7)
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
         * @param[in] s
         *      This is the given string value to parse.
         * @param[in, out] offset
         *      On input, this is the position of the first character
         *      of the encoded value in the string.
         *      
         *      On output, this is the position of the first character
         *      past the end of the encoded value in the string, or
         *      std::string::npos if the encoded value was invalid.
         * @param[in] delimiter
         *      This is the character that marks the end of the value
         *      if it's encountered.
         * 
         * @return
         *      The encoding of the next JSON value in the given string
         *      is returned. 
         */
        std::string ParseValue(const std::string& s, size_t& offset, char delimiter) {
            Utf8::Utf8 decoder, encoder;
            std::stack< char > expectedDelimiters; 
            std::vector< Utf8::UnicodeCodePoint > encodedValueCodePoints; 
            const auto encodingCodePoints = decoder.Decode(s.substr(offset));
            if (encodingCodePoints.empty()) {
                offset = std::string::npos;
                return "";
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
                auto encodedValue = encoder.Encode(encodedValueCodePoints);
                offset += encodedValue.size();
                if (encodedValue.back() == (Utf8::UnicodeCodePoint)delimiter) {
                    encodedValue.pop_back();
                }
                return std::string(
                    encodedValue.begin(), 
                    encodedValue.end()
                );
            } else {
                offset = std::string::npos;
                return "";
            }
        }

        /**
         * This function parses the given string as an array
         * JSON value.
         * 
         * @param[in] s
         *      This is the given string to parse.
         */
        void ParseAsArray(const std::string& s) {   
            size_t offset = 0;
            std::vector< std::shared_ptr< Json > > newArrayValue;
            while (offset < s.length()) {
                const auto encodedValue = ParseValue(s, offset, ',');
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
        void ParseAsObject(const std::string& s) {
            std::map< std::string, std::shared_ptr< Json > > newObjectValue;
            size_t offset = 0;
            while (offset < s.length()) {
                const auto encodedKey = ParseValue(s, offset, ':');
                if (offset == std::string::npos) {
                    return;
                }
                const auto key = std::make_shared< Json >(FromEncoding(encodedKey));
                if (key->GetType() != Type::String) {
                    return;
                }
                const auto encodedValue = ParseValue(s, offset, ',');
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
    Json::Json(Json&&) = default;
    Json& Json::operator=(Json&&) = default;

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
                return (fabs(impl_->floatValue - other.impl_->floatValue)
                    < std::numeric_limits< double >::epsilon());
                break;
            default:
                return true;
                break;
        }
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

    void Json::Add(Json&& value) {
        if (impl_->type != Type::Array) {
            return;
        } 
        Insert(std::move(value), impl_->arrayValue->size());
    }

    void Json::Insert(Json&& value, size_t index) {
        if (impl_->type != Type::Array) {
            return;
        }
        (void)impl_->arrayValue->insert(
            impl_->arrayValue->begin() 
            + std::min(index, impl_->arrayValue->size()
            ), 
            std::make_shared< Json >(std::move(value))
        );
    }

    void Json::Remove(size_t index) {
        if (impl_->type != Type::Array) {
            return;
        }
        if (index < impl_->arrayValue->size()) {
            impl_->arrayValue->erase(impl_->arrayValue->begin() + index);
        }
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
                for (const auto value: *impl_->arrayValue) {
                    if (isFirst) {
                        isFirst = false;
                    } else {
                        impl_->encoding += ',';
                    }
                    impl_->encoding += value->ToEncoding(options);
                }
                impl_->encoding += ']';
            } break;
            default:
                impl_->encoding = "???";
                break;
            }
        }
        return impl_->encoding;
    }

    Json Json::FromEncoding(const std::string& stringFormat) {       
        Json json;
        const auto firstNonWhitespaceChar = stringFormat.find_first_not_of(WHITESPACE_CHARACTERS);
        if (firstNonWhitespaceChar == std::string::npos) {
            return json;
        }
        const auto lastNonWhitespaceChar = stringFormat.find_last_not_of(WHITESPACE_CHARACTERS);
        const auto encoding = stringFormat.substr(
            firstNonWhitespaceChar,
            lastNonWhitespaceChar - firstNonWhitespaceChar + 1
        );
        json.impl_->encoding = encoding; 
        if (encoding.empty()) {
            
        } else if (
            ! encoding.empty()
            && (encoding[0] == '{')
            && (encoding[encoding.length() - 1] == '}')
        ) {
            json.impl_->ParseAsObject(
                encoding.substr(1, encoding.length() - 2)
            );          
        } else if (
            !encoding.empty()
            && (encoding[0] == '[')
            && (encoding[encoding.length() - 1] == ']')
        ) {
            json.impl_->ParseAsArray(
                encoding.substr(1, encoding.length() - 2)
            );
        } else if (
            !encoding.empty()
            && (encoding[0] == '"')
            && (encoding[encoding.length() - 1] == '"')
        ) {
            std::string output;
            if (
                Unescape(encoding.substr(1, encoding.length() - 2), output)
            ) {
                json.impl_->type = Type::String;
                json.impl_->stringValue = new std::string(output);
            } 
        } else if (encoding == "null") {
            json.impl_->type = Type::Null;
        } else if (encoding == "true") {
            json.impl_->type = Type::Boolean;
            json.impl_->booleanValue = true;
        } else if  (encoding == "false") {
            json.impl_->type = Type::Boolean;
            json.impl_->booleanValue = false;
        } else {
            if (encoding.find_first_of(".eE") != std::string::npos) {
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
