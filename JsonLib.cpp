#include "JsonLib.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cassert>
#include <iomanip>

// ============================================================
//  JsonValue — implementation
// ============================================================

JsonValue::JsonValue()                        : m_type(Type::Null),   m_value(std::monostate{}) {}
JsonValue::JsonValue(bool b)                  : m_type(Type::Bool),   m_value(b)  {}
JsonValue::JsonValue(double n)                : m_type(Type::Number), m_value(n)  {}
JsonValue::JsonValue(int n)                   : m_type(Type::Number), m_value(static_cast<double>(n)) {}
JsonValue::JsonValue(long long n)             : m_type(Type::Number), m_value(static_cast<double>(n)) {}
JsonValue::JsonValue(const std::string& s)    : m_type(Type::String), m_value(s)            {}
JsonValue::JsonValue(std::string&& s)         : m_type(Type::String), m_value(std::move(s)) {}
JsonValue::JsonValue(const char* s)           : m_type(Type::String), m_value(std::string(s)) {}
JsonValue::JsonValue(const Array& arr)        : m_type(Type::Array),  m_value(arr)            {}
JsonValue::JsonValue(Array&& arr)             : m_type(Type::Array),  m_value(std::move(arr)) {}
JsonValue::JsonValue(const Object& obj)       : m_type(Type::Object), m_value(obj)            {}
JsonValue::JsonValue(Object&& obj)            : m_type(Type::Object), m_value(std::move(obj)) {}

// --- Accessors ---

bool JsonValue::asBool() const
{
    if (m_type != Type::Bool)
        throw std::runtime_error("JsonValue: type is not Bool");
    return std::get<bool>(m_value);
}

double JsonValue::asNumber() const
{
    if (m_type != Type::Number)
        throw std::runtime_error("JsonValue: type is not Number");
    return std::get<double>(m_value);
}

const std::string& JsonValue::asString() const
{
    if (m_type != Type::String)
        throw std::runtime_error("JsonValue: type is not String");
    return std::get<std::string>(m_value);
}

const JsonValue::Array& JsonValue::asArray() const
{
    if (m_type != Type::Array)
        throw std::runtime_error("JsonValue: type is not Array");
    return std::get<Array>(m_value);
}

JsonValue::Array& JsonValue::asArray()
{
    if (m_type != Type::Array)
        throw std::runtime_error("JsonValue: type is not Array");
    return std::get<Array>(m_value);
}

const JsonValue::Object& JsonValue::asObject() const
{
    if (m_type != Type::Object)
        throw std::runtime_error("JsonValue: type is not Object");
    return std::get<Object>(m_value);
}

JsonValue::Object& JsonValue::asObject()
{
    if (m_type != Type::Object)
        throw std::runtime_error("JsonValue: type is not Object");
    return std::get<Object>(m_value);
}

// --- Array subscript ---

JsonValue& JsonValue::operator[](size_t index)
{
    return asArray().at(index);
}

const JsonValue& JsonValue::operator[](size_t index) const
{
    return asArray().at(index);
}

// --- Object subscript ---

JsonValue& JsonValue::operator[](const std::string& key)
{
    if (m_type == Type::Null)
    {
        // Auto-promote null → object on first key access
        m_type  = Type::Object;
        m_value = Object{};
    }
    return asObject()[key];
}

const JsonValue& JsonValue::operator[](const std::string& key) const
{
    const auto& obj = asObject();
    auto it = obj.find(key);
    if (it == obj.end())
        throw std::out_of_range("JsonValue: key not found: " + key);
    return it->second;
}

bool JsonValue::hasKey(const std::string& key) const
{
    if (m_type != Type::Object) return false;
    return asObject().count(key) > 0;
}

void JsonValue::set(const std::string& key, JsonValue val)
{
    (*this)[key] = std::move(val);
}

void JsonValue::push(JsonValue val)
{
    if (m_type == Type::Null)
    {
        m_type  = Type::Array;
        m_value = Array{};
    }
    asArray().push_back(std::move(val));
}

size_t JsonValue::size() const
{
    if (m_type == Type::Array)  return asArray().size();
    if (m_type == Type::Object) return asObject().size();
    return 0;
}

// ============================================================
//  JsonParser — implementation
// ============================================================

JsonParser::JsonParser(const std::string& json)
    : m_json(json), m_pos(0)
{}

JsonValue JsonParser::parse(const std::string& json)
{
    JsonParser p(json);
    p.skipWhitespace();
    JsonValue result = p.parseValue();
    p.skipWhitespace();
    if (!p.isEnd())
        throw std::runtime_error("JsonParser: unexpected trailing characters");
    return result;
}

JsonValue JsonParser::parseFile(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
        throw std::runtime_error("JsonParser: cannot open file: " + filePath);

    std::ostringstream ss;
    ss << file.rdbuf();
    return parse(ss.str());
}

// --- Internal helpers ---

bool JsonParser::isEnd() const
{
    return m_pos >= m_json.size();
}

char JsonParser::current() const
{
    if (isEnd())
        throw std::runtime_error("JsonParser: unexpected end of input");
    return m_json[m_pos];
}

char JsonParser::peek(size_t offset) const
{
    size_t idx = m_pos + offset;
    if (idx >= m_json.size()) return '\0';
    return m_json[idx];
}

char JsonParser::advance()
{
    char c = current();
    ++m_pos;
    return c;
}

void JsonParser::expect(char c)
{
    if (advance() != c)
        throw std::runtime_error(std::string("JsonParser: expected '") + c + "'");
}

void JsonParser::skipWhitespace()
{
    while (!isEnd() && (m_json[m_pos] == ' '  ||
                        m_json[m_pos] == '\t' ||
                        m_json[m_pos] == '\r' ||
                        m_json[m_pos] == '\n'))
        ++m_pos;
}

// --- Value dispatch ---

JsonValue JsonParser::parseValue()
{
    skipWhitespace();
    if (isEnd())
        throw std::runtime_error("JsonParser: unexpected end of input");

    char c = current();
    if (c == 'n')               return parseNull();
    if (c == 't' || c == 'f')   return parseBool();
    if (c == '"')               return parseString();
    if (c == '[')               return parseArray();
    if (c == '{')               return parseObject();
    if (c == '-' || std::isdigit(static_cast<unsigned char>(c)))
                                return parseNumber();

    throw std::runtime_error(std::string("JsonParser: unexpected character '") + c + "'");
}

JsonValue JsonParser::parseNull()
{
    expect('n'); expect('u'); expect('l'); expect('l');
    return JsonValue();
}

JsonValue JsonParser::parseBool()
{
    if (current() == 't')
    {
        expect('t'); expect('r'); expect('u'); expect('e');
        return JsonValue(true);
    }
    expect('f'); expect('a'); expect('l'); expect('s'); expect('e');
    return JsonValue(false);
}

JsonValue JsonParser::parseNumber()
{
    size_t start = m_pos;

    if (current() == '-') advance();

    if (isEnd() || !std::isdigit(static_cast<unsigned char>(current())))
        throw std::runtime_error("JsonParser: invalid number");

    while (!isEnd() && std::isdigit(static_cast<unsigned char>(current())))
        advance();

    if (!isEnd() && current() == '.')
    {
        advance();
        while (!isEnd() && std::isdigit(static_cast<unsigned char>(current())))
            advance();
    }

    if (!isEnd() && (current() == 'e' || current() == 'E'))
    {
        advance();
        if (!isEnd() && (current() == '+' || current() == '-')) advance();
        while (!isEnd() && std::isdigit(static_cast<unsigned char>(current())))
            advance();
    }

    std::string token = m_json.substr(start, m_pos - start);
    return JsonValue(std::stod(token));
}

std::string JsonParser::parseRawString()
{
    expect('"');
    std::string result;
    while (true)
    {
        if (isEnd())
            throw std::runtime_error("JsonParser: unterminated string");

        char c = advance();
        if (c == '"') break;

        if (c == '\\')
        {
            if (isEnd())
                throw std::runtime_error("JsonParser: unterminated escape");
            char esc = advance();
            switch (esc)
            {
            case '"':  result += '"';  break;
            case '\\': result += '\\'; break;
            case '/':  result += '/';  break;
            case 'b':  result += '\b'; break;
            case 'f':  result += '\f'; break;
            case 'n':  result += '\n'; break;
            case 'r':  result += '\r'; break;
            case 't':  result += '\t'; break;
            case 'u':
            {
                if (m_pos + 4 > m_json.size())
                    throw std::runtime_error("JsonParser: incomplete \\u escape");
                std::string hex = m_json.substr(m_pos, 4);
                m_pos += 4;
                unsigned int cp = std::stoul(hex, nullptr, 16);
                if (cp < 0x80)
                {
                    result += static_cast<char>(cp);
                }
                else if (cp < 0x800)
                {
                    result += static_cast<char>(0xC0 | (cp >> 6));
                    result += static_cast<char>(0x80 | (cp & 0x3F));
                }
                else
                {
                    result += static_cast<char>(0xE0 | (cp >> 12));
                    result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                    result += static_cast<char>(0x80 | (cp & 0x3F));
                }
                break;
            }
            default:
                throw std::runtime_error(std::string("JsonParser: unknown escape '\\") + esc + "'");
            }
        }
        else
        {
            result += c;
        }
    }
    return result;
}

JsonValue JsonParser::parseString()
{
    return JsonValue(parseRawString());
}

JsonValue JsonParser::parseArray()
{
    expect('[');
    JsonValue::Array arr;

    skipWhitespace();
    if (!isEnd() && current() == ']')
    {
        advance();
        return JsonValue(std::move(arr));
    }

    while (true)
    {
        skipWhitespace();
        arr.push_back(parseValue());
        skipWhitespace();

        if (isEnd())
            throw std::runtime_error("JsonParser: unterminated array");

        char c = advance();
        if (c == ']') break;
        if (c != ',')
            throw std::runtime_error("JsonParser: expected ',' or ']' in array");
    }
    return JsonValue(std::move(arr));
}

JsonValue JsonParser::parseObject()
{
    expect('{');
    JsonValue::Object obj;

    skipWhitespace();
    if (!isEnd() && current() == '}')
    {
        advance();
        return JsonValue(std::move(obj));
    }

    while (true)
    {
        skipWhitespace();
        if (current() != '"')
            throw std::runtime_error("JsonParser: expected string key in object");

        std::string key = parseRawString();
        skipWhitespace();
        expect(':');
        skipWhitespace();
        obj[key] = parseValue();
        skipWhitespace();

        if (isEnd())
            throw std::runtime_error("JsonParser: unterminated object");

        char c = advance();
        if (c == '}') break;
        if (c != ',')
            throw std::runtime_error("JsonParser: expected ',' or '}' in object");
    }
    return JsonValue(std::move(obj));
}

// ============================================================
//  JsonWriter — implementation
// ============================================================

std::string JsonWriter::indent(int depth)
{
    return std::string(static_cast<size_t>(depth) * 2, ' ');
}

std::string JsonWriter::escapeString(const std::string& s)
{
    std::string result;
    result.reserve(s.size() + 2);
    result += '"';
    for (unsigned char c : s)
    {
        switch (c)
        {
        case '"':  result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '\b': result += "\\b";  break;
        case '\f': result += "\\f";  break;
        case '\n': result += "\\n";  break;
        case '\r': result += "\\r";  break;
        case '\t': result += "\\t";  break;
        default:
            if (c < 0x20)
            {
                std::ostringstream oss;
                oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                result += oss.str();
            }
            else
            {
                result += static_cast<char>(c);
            }
        }
    }
    result += '"';
    return result;
}

std::string JsonWriter::write(const JsonValue& val, bool pretty, int depth)
{
    switch (val.type())
    {
    case JsonValue::Type::Null:
        return "null";

    case JsonValue::Type::Bool:
        return val.asBool() ? "true" : "false";

    case JsonValue::Type::Number:
    {
        double d = val.asNumber();
        if (d == static_cast<long long>(d) &&
            d >= -1e15 && d <= 1e15)
        {
            std::ostringstream oss;
            oss << static_cast<long long>(d);
            return oss.str();
        }
        std::ostringstream oss;
        oss << std::setprecision(17) << d;
        return oss.str();
    }

    case JsonValue::Type::String:
        return escapeString(val.asString());

    case JsonValue::Type::Array:
    {
        const auto& arr = val.asArray();
        if (arr.empty()) return "[]";

        std::string result = "[";
        if (pretty) result += "\n";

        for (size_t i = 0; i < arr.size(); ++i)
        {
            if (pretty) result += indent(depth + 1);
            result += write(arr[i], pretty, depth + 1);
            if (i + 1 < arr.size()) result += ',';
            if (pretty) result += '\n';
        }

        if (pretty) result += indent(depth);
        result += ']';
        return result;
    }

    case JsonValue::Type::Object:
    {
        const auto& obj = val.asObject();
        if (obj.empty()) return "{}";

        std::string result = "{";
        if (pretty) result += "\n";

        size_t i = 0;
        for (const auto& [key, value] : obj)
        {
            if (pretty) result += indent(depth + 1);
            result += escapeString(key);
            result += pretty ? ": " : ":";
            result += write(value, pretty, depth + 1);
            if (i + 1 < obj.size()) result += ',';
            if (pretty) result += '\n';
            ++i;
        }

        if (pretty) result += indent(depth);
        result += '}';
        return result;
    }
    }
    return "null";
}

std::string JsonWriter::serialize(const JsonValue& value, bool pretty)
{
    return write(value, pretty, 0);
}

void JsonWriter::saveFile(const JsonValue& value,
                          const std::string& filePath,
                          bool pretty)
{
    std::ofstream file(filePath);
    if (!file.is_open())
        throw std::runtime_error("JsonWriter: cannot open file for writing: " + filePath);

    file << serialize(value, pretty);
    if (!file)
        throw std::runtime_error("JsonWriter: write error: " + filePath);
}
