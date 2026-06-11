#pragma once
#include <vector>
#include <string>
#include <optional>
#include "Member.h"

class MemberRepository
{
public:
    explicit MemberRepository(const std::string& filePath);

    Member                create(const std::string& name,
                                 const std::string& email,
                                 const std::string& phone);
    std::vector<Member>   findAll()   const;
    std::optional<Member> findById(int id) const;
    bool                  update(int id,
                                 const std::string& name,
                                 const std::string& email,
                                 const std::string& phone);
    bool                  remove(int id);
    std::vector<Member>   findByKeyword(const std::string& field,
                                        const std::string& keyword) const;
    bool                  existsEmail(const std::string& email,
                                      int excludeId = -1) const;

private:
    std::string         m_filePath;
    std::vector<Member> m_members;

    void load();
    void save() const;
    int  nextId() const;
};
