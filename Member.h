#pragma once
#include <string>
#include "JsonLib.h"

struct Member
{
    int         id        = 0;
    std::string name;
    std::string email;
    std::string phone;
    bool        active    = true;
    std::string createdAt;
};

inline JsonValue memberToJson(const Member& m)
{
    JsonValue v;
    v["id"]        = JsonValue(m.id);
    v["name"]      = JsonValue(m.name);
    v["email"]     = JsonValue(m.email);
    v["phone"]     = JsonValue(m.phone);
    v["active"]    = JsonValue(m.active);
    v["createdAt"] = JsonValue(m.createdAt);
    return v;
}

inline Member memberFromJson(const JsonValue& v)
{
    Member m;
    m.id        = static_cast<int>(v["id"].asNumber());
    m.name      = v["name"].asString();
    m.email     = v["email"].asString();
    m.phone     = v["phone"].asString();
    m.active    = v["active"].asBool();
    m.createdAt = v["createdAt"].asString();
    return m;
}
