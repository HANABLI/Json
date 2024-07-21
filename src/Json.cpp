/**
 * @file Json.cpp
 * 
 * This module contains the implementation of
 * the Json::Json class.
 * 
 * © 2024 by Hatem Nabli
 */

#include <map>
#include <Utf8/Utf8.hpp>
#include <Json/Json.hpp>
#include <StringUtils/StringUtils.hpp>

namespace 
{   
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
     * This function parses the given string as an integer JSON value
     * 
     * @param[in] s
     *      This is the string to parse.
     * 
     * @return
     *      The parsed JSON value is returned.
     */
    Json::Json ParseInteger(const std::string& s) {
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
                        return Json::Json();
                    }
                    ++index;
                } break;

                case 2: {
                    return Json::Json();
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
                            return Json::Json();
                        }
                        ++index;
                    } else {
                        return Json::Json();
                    }
                } break;
            }
        }
        if (state < 2) {
            return Json::Json();
        } else {
            return Json::Json(value * (negative ? -1 : 1));
        }       
    }

    /**
     * This function parses the given string as a floating point JSON value
     * 
     * @param[in] s
     *      This is the string to parse.
     * 
     * @return
     *      The parsed JSON value is returned.
     */
    Json::Json ParseFloatingPoint(const std::string& s) {
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
                        return Json::Json();
                    }
                    ++index;
                } break;

                case 2: {
                    return Json::Json();
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
                        return Json::Json();
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
                        return Json::Json();
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
                        return Json::Json();
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
                        return Json::Json();
                    }
                    ++index;
                } break;
            }
        }
        if (
            (state < 2)
            || (state == 4)
            || (state == 6)
            || (state == 7)
        ) {
            return Json::Json();
        } else {
            return Json::Json(
                (
                    magnitude
                    + fraction
                )
                * pow(10.0, exponent * (negativeExponent ? -1.0 : 1.0)) 
                * (negativeExponent ? -1.0 : 1.0));
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
                output += "\\u";
                output += CodePointToFourHexDigits(cp);
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
     * 
     * @return
     *      The unescaped string is returned.
     */
    std::string Unescape(const std::string& s) {
        Utf8::Utf8 decoder, encoder;
        std::string output;
        Utf8::UnicodeCodePoint cpFromHexDigits = 0;
        std::vector< Utf8::UnicodeCodePoint > hexDigitOriginal;
        size_t state = 0;
        for (const auto cp: decoder.Decode(s)) {
            switch (state)
            {
                case 0: { // initial state 
                    if((0x5C == cp)) {
                        state = 1;
                    } else {
                        const auto encoding = encoder.Encode({cp});
                        output += std::string(
                            encoding.begin(),
                            encoding.end()
                        );
                    }
                } break;

                case 1: { // escape character
                    if (cp == 0x75) {
                        state = 2;
                        cpFromHexDigits = 0;
                        hexDigitOriginal = { 0x5C, 0x75 };
                    } else {
                        Utf8::UnicodeCodePoint alterantive = cp;
                        const auto entry = POPULAR_CHARACTERS_ESCAPED.find(cp);
                        if (entry == POPULAR_CHARACTERS_ESCAPED.end()) {
                            const auto encoding = encoder.Encode({0x5C, cp});
                            output += std::string(
                                encoding.begin(),
                                encoding.end()
                            );
                        } else {
                            const auto encoding = encoder.Encode({entry->second});
                            output += std::string(
                                encoding.begin(),
                                encoding.end()
                            );
                        }
                        state = 0;
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
                        state = 0;
                        const auto encoding = encoder.Encode(hexDigitOriginal);
                        output += std::string(
                            encoding.begin(),
                            encoding.end()
                        );
                        break;
                    }
                    if (++state == 6) {
                        state = 0;
                        const auto encoding = encoder.Encode({cpFromHexDigits});
                        output += std::string(
                            encoding.begin(),
                            encoding.end()
                        );
                    }
                } break;
            }          
        }
        switch (state)
        {
            case 1: { //escape character
                const auto encoding = encoder.Encode({0x5C});
                output += std::string(
                    encoding.begin(),
                    encoding.end()
                );
            } break;

            case 2: // first hexdigit of escape uXXXX
            case 3: // second hexdigit of escape uXXXX
            case 4: // third hexdigit of escape uXXXX
            case 5: { // fourth hexdigit of escape uXXXX
                const auto encoding = encoder.Encode(hexDigitOriginal);
                output += std::string(
                    encoding.begin(),
                    encoding.end()
                );
            } break;
        }
        return output;
    }
} // namespace 

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
            Boolean,
            String,
            Integer,
            Float
        };

        /**
         * This indicates the type of the value represented
         * by the JSON object.
         */
        Type type = Type::Invalid;


        union 
        { 
            bool booleanValue;
            std::string* stringValue;
            int integerValue;
            double floatValue;

        };

        // Lifecycle management

        ~Impl() {
            switch (type)
            {
                case Impl::Type::String: {
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

    Json::Json(int value): impl_(new Impl) {
        impl_->type = Impl::Type::Integer;
        impl_->integerValue = value;
    }

    Json::Json(double value): impl_(new Impl) {
        impl_->type = Impl::Type::Float;
        impl_->floatValue = value;
    }

    Json::Json(const std::string& value): impl_(new Impl) {
        impl_->type = Impl::Type::String;
        impl_->stringValue = new std::string(value);
    }

    Json::Json(const char* value): impl_(new Impl) {
         impl_->type = Impl::Type::String;
        impl_->stringValue = new std::string(value);
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
            case Impl::Type::String: 
                return *impl_->stringValue == *other.impl_->stringValue;
                break;
            case Impl::Type::Integer:
                return impl_->integerValue == other.impl_->integerValue;
                break;
            case Impl::Type::Float:
                return (fabs(impl_->floatValue - other.impl_->floatValue)
                    < std::numeric_limits< double >::epsilon());
                break;
            default:
                return true;
                break;
        }
    }

    Json::operator bool() const {
        if (impl_->type == Impl::Type::Boolean) {
            return impl_->booleanValue;
        } else {
            return false;
        }
    }

    Json::operator int() const {
        if (impl_->type == Impl::Type::Integer) {
            return impl_->integerValue;
        } else if (impl_->type == Impl::Type::Float) {
            return (int)impl_->floatValue;
        } else {
            return 0;
        }
    }

    Json::operator double() const {
        if (impl_->type == Impl::Type::Integer) {
            return (double)impl_->integerValue;
        } else if (impl_->type == Impl::Type::Float) {
            return impl_->floatValue;
        } else {
            return 0.0;
        }
    }

    Json::operator std::string() const {
        if (impl_->type == Impl::Type::String) {
            return *impl_->stringValue;
        } else {
            return "";
        }
    }

    std::string Json::ToString(const JsonEncodingOptions options) const {
        switch (impl_->type)
        {
        case Impl::Type::Null:
            return "null";
            break;
        case Impl::Type::Boolean:
            return impl_->booleanValue ? "true" : "false";
            break;
        case Impl::Type::String:
            return ( "\"" + Escape(*impl_->stringValue, options) + "\"");
            break;
        case Impl::Type::Integer:
            return StringUtils::sprintf("%i", impl_->integerValue);
            break;
        case Impl::Type::Float:
            return StringUtils::sprintf("%lg", impl_->floatValue);
            break;
        default:
            return "???";
            break;
        }
    }

    Json Json::FromString(const std::string& stringFormat) {        
        if (
            !stringFormat.empty()
            && (stringFormat[0] == '{')
        ) {
            // TODO: parse as an object
            return Json();
        } else if (
            !stringFormat.empty()
            && (stringFormat[0] == '[')
        ) {
            // TODO: parse as an array
            return Json();
        } else if (
            !stringFormat.empty()
            && (stringFormat[0] == '"')
            && (stringFormat[stringFormat.length() - 1] == '"')
        ) {
            return Unescape(stringFormat.substr(1, stringFormat.length() - 2));
        } else if (stringFormat == "null") {
            return nullptr;
        } else if (stringFormat == "true") {
            return true;
        } else if  (stringFormat == "false") {
            return false;
        } else {
            if (stringFormat.find_first_of(".eE") != std::string::npos) {
                // TODO: parse as floating point
                return ParseFloatingPoint(stringFormat);
            } else {
                return ParseInteger(stringFormat);
            }
        }
    }
}