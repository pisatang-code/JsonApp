#pragma once
#include <vector>
#include <string>
#include "Member.h"

class MemberRepository
{
public:
    explicit MemberRepository(const std::string& filePath);

    Member              create(const std::string& name,
                               const std::string& email,
                               const std::string& phone);
    std::vector<Member> findAll() const;

private:
    std::string         m_filePath;
    std::vector<Member> m_members;

    void load();
    void save() const;
    int  nextId() const;
};
