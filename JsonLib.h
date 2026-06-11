#pragma once
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <stdexcept>

// ============================================================
//  JsonValue
//  Represents any JSON value: null, bool, number, string,
//  array, or object.
// ============================================================
class JsonValue
{
public:
    enum class Type { Null, Bool, Number, String, Array, Object };

    using Array  = std::vector<JsonValue>;
    using Object = std::map<std::string, JsonValue>;

    // --- Constructors ---
    JsonValue();                                // null
    explicit JsonValue(bool b);
    JsonValue(double n);
    JsonValue(int n);
    JsonValue(long long n);
    JsonValue(const std::string& s);
    JsonValue(std::string&& s);
    JsonValue(const char* s);
    JsonValue(const Array& arr);
    JsonValue(Array&& arr);
    JsonValue(const Object& obj);
    JsonValue(Object&& obj);

    // --- Type queries ---
    Type        type()     const { return m_type; }
    bool        isNull()   const { return m_type == Type::Null;   }
    bool        isBool()   const { return m_type == Type::Bool;   }
    bool        isNumber() const { return m_type == Type::Number; }
    bool        isString() const { return m_type == Type::String; }
    bool        isArray()  const { return m_type == Type::Array;  }
    bool        isObject() const { return m_type == Type::Object; }

    // --- Value accessors (throw on wrong type) ---
    bool               asBool()   const;
    double             asNumber() const;
    const std::string& asString() const;
    const Array&       asArray()  const;
    Array&             asArray();
    const Object&      asObject() const;
    Object&            asObject();

    // --- Subscript for array ---
    JsonValue&       operator[](size_t index);
    const JsonValue& operator[](size_t index) const;

    // --- Subscript for object (creates key if missing) ---
    JsonValue&       operator[](const std::string& key);
    const JsonValue& operator[](const std::string& key) const;

    // --- Object helpers ---
    bool hasKey(const std::string& key) const;
    void set(const std::string& key, JsonValue val);

    // --- Array helpers ---
    void push(JsonValue val);
    size_t size() const;

private:
    Type m_type = Type::Null;
    std::variant<
        std::monostate,   // Null
        bool,             // Bool
        double,           // Number
        std::string,      // String
        Array,            // Array
        Object            // Object
    > m_value;
};

// ============================================================
//  JsonParser
//  Parses a JSON string or file into a JsonValue tree.
// ============================================================
class JsonParser
{
public:
    static JsonValue parse(const std::string& json);
    static JsonValue parseFile(const std::string& filePath);

private:
    explicit JsonParser(const std::string& json);

    JsonValue parseValue();
    JsonValue parseNull();
    JsonValue parseBool();
    JsonValue parseNumber();
    JsonValue parseString();
    JsonValue parseArray();
    JsonValue parseObject();

    std::string parseRawString();   // shared by parseString / parseObject key

    void skipWhitespace();
    char current() const;
    char peek(size_t offset = 1) const;
    char advance();
    void expect(char c);
    bool isEnd() const;

    std::string m_json;
    size_t      m_pos = 0;
};

// ============================================================
//  JsonWriter
//  Serialises a JsonValue tree back to a JSON string or file.
// ============================================================
class JsonWriter
{
public:
    // pretty=true  → indented, human-readable output
    // pretty=false → compact (minified) output
    static std::string serialize(const JsonValue& value, bool pretty = true);
    static void        saveFile(const JsonValue& value,
                                const std::string& filePath,
                                bool pretty = true);

private:
    static std::string write(const JsonValue& value, bool pretty, int depth);
    static std::string indent(int depth);
    static std::string escapeString(const std::string& s);
};
