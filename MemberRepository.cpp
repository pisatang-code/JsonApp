#include "MemberRepository.h"
#include <ctime>
#include <algorithm>
#include <cctype>

static std::string currentDate()
{
    std::time_t t = std::time(nullptr);
    std::tm tm{};
    localtime_s(&tm, &t);
    char buf[11];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
    return buf;
}

MemberRepository::MemberRepository(const std::string& filePath)
    : m_filePath(filePath)
{
    load();
}

void MemberRepository::load()
{
    try
    {
        JsonValue root = JsonParser::parseFile(m_filePath);
        for (const auto& item : root.asArray())
            m_members.push_back(memberFromJson(item));
    }
    catch (...) {}  // 파일 없음 또는 빈 배열 → 무시하고 빈 상태로 시작
}

void MemberRepository::save() const
{
    JsonValue root;
    for (const auto& m : m_members)
        root.push(memberToJson(m));
    JsonWriter::saveFile(root, m_filePath);
}

int MemberRepository::nextId() const
{
    int maxId = 0;
    for (const auto& m : m_members)
        if (m.id > maxId) maxId = m.id;
    return maxId + 1;
}

Member MemberRepository::create(const std::string& name,
                                 const std::string& email,
                                 const std::string& phone)
{
    Member m;
    m.id        = nextId();
    m.name      = name;
    m.email     = email;
    m.phone     = phone;
    m.active    = true;
    m.createdAt = currentDate();
    m_members.push_back(m);
    save();
    return m;
}

std::vector<Member> MemberRepository::findAll() const
{
    return m_members;
}

std::optional<Member> MemberRepository::findById(int id) const
{
    for (const auto& m : m_members)
        if (m.id == id) return m;
    return std::nullopt;
}

bool MemberRepository::update(int id,
                               const std::string& name,
                               const std::string& email,
                               const std::string& phone)
{
    for (auto& m : m_members)
    {
        if (m.id == id)
        {
            m.name  = name;
            m.email = email;
            m.phone = phone;
            save();
            return true;
        }
    }
    return false;
}

bool MemberRepository::remove(int id)
{
    for (auto it = m_members.begin(); it != m_members.end(); ++it)
    {
        if (it->id == id)
        {
            m_members.erase(it);
            save();
            return true;
        }
    }
    return false;
}

std::vector<Member> MemberRepository::findByKeyword(const std::string& field,
                                                     const std::string& keyword) const
{
    auto toLower = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return s;
    };

    std::string kw = toLower(keyword);
    std::vector<Member> result;

    for (const auto& m : m_members)
    {
        std::string val = (field == "name") ? m.name : m.email;
        if (toLower(val).find(kw) != std::string::npos)
            result.push_back(m);
    }
    return result;
}

bool MemberRepository::existsEmail(const std::string& email, int excludeId) const
{
    for (const auto& m : m_members)
        if (m.id != excludeId && m.email == email)
            return true;
    return false;
}
