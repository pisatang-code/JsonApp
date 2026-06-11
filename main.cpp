#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <string>
#include <limits>
#include <iomanip>
#include "MemberRepository.h"

// UTF-8 문자열의 콘솔 출력 너비를 계산 (한글 2칸, ASCII 1칸)
static int displayWidth(const std::string& s)
{
    int width = 0;
    size_t i = 0;
    while (i < s.size())
    {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if      (c < 0x80) { width += 1; i += 1; }
        else if (c < 0xE0) { width += 1; i += 2; }
        else if (c < 0xF0) { width += 2; i += 3; }
        else               { width += 2; i += 4; }
    }
    return width;
}

// 지정 너비에 맞게 오른쪽 공백 패딩 (한글 고려)
static std::string padRight(const std::string& s, int width)
{
    int pad = width - displayWidth(s);
    return s + std::string(pad > 0 ? pad : 0, ' ');
}

static void printMember(const Member& m)
{
    std::cout << "  ID      : " << m.id << "\n";
    std::cout << "  이름    : " << m.name << "\n";
    std::cout << "  이메일  : " << m.email << "\n";
    std::cout << "  전화번호: " << m.phone << "\n";
    std::cout << "  상태    : " << (m.active ? "활성" : "비활성") << "\n";
    std::cout << "  등록일  : " << m.createdAt << "\n";
}

static void printMenu()
{
    std::cout << "\n==========================================\n";
    std::cout << "      회원 관리 시스템 (Member CRUD)\n";
    std::cout << "==========================================\n";
    std::cout << "  1. 회원 등록    (Create)\n";
    std::cout << "  2. 회원 목록    (Read)\n";
    std::cout << "  3. 회원 수정    (Update)\n";
    std::cout << "  4. 회원 삭제    (Delete)\n";
    std::cout << "  0. 종료\n";
    std::cout << "==========================================\n";
    std::cout << "선택 > ";
}

static void handleCreate(MemberRepository& repo)
{
    std::cout << "\n=== 회원 등록 ===\n";

    std::string name, email, phone;

    std::cout << "  이름     : "; std::getline(std::cin, name);
    std::cout << "  이메일   : "; std::getline(std::cin, email);
    std::cout << "  전화번호 : "; std::getline(std::cin, phone);

    if (name.empty() || email.empty() || phone.empty())
    {
        std::cout << "\n  [오류] 이름, 이메일, 전화번호는 필수 입력입니다.\n";
        return;
    }

    if (repo.existsEmail(email))
    {
        std::cout << "\n  [오류] 이미 등록된 이메일입니다.\n";
        return;
    }

    Member m = repo.create(name, email, phone);
    std::cout << "\n  등록 완료 — ID: " << m.id << " / " << m.name << "\n";
}

static void printMemberTable(const std::vector<Member>& members)
{
    if (members.empty())
    {
        std::cout << "  검색 결과가 없습니다.\n";
        return;
    }

    std::cout << "  총 " << members.size() << "명\n\n";

    const int W_ID    =  4;
    const int W_NAME  = 12;
    const int W_EMAIL = 26;
    const int W_PHONE = 16;
    const int W_STATE =  6;

    auto line = [&]() {
        std::cout << "  +" << std::string(W_ID + 2, '-')
                  << "+" << std::string(W_NAME + 2, '-')
                  << "+" << std::string(W_EMAIL + 2, '-')
                  << "+" << std::string(W_PHONE + 2, '-')
                  << "+" << std::string(W_STATE + 2, '-')
                  << "+" << std::string(12, '-') << "+\n";
    };
    auto row = [&](const std::string& id,    const std::string& name,
                   const std::string& email, const std::string& phone,
                   const std::string& state, const std::string& date)
    {
        std::cout << "  | " << padRight(id,    W_ID)
                  << " | " << padRight(name,   W_NAME)
                  << " | " << padRight(email,  W_EMAIL)
                  << " | " << padRight(phone,  W_PHONE)
                  << " | " << padRight(state,  W_STATE)
                  << " | " << padRight(date,   10) << " |\n";
    };

    line();
    row("ID", "이름", "이메일", "전화번호", "상태", "등록일");
    line();
    for (const auto& m : members)
        row(std::to_string(m.id), m.name, m.email, m.phone,
            m.active ? "활성" : "비활성", m.createdAt);
    line();
}

static void handleRead(MemberRepository& repo)
{
    std::cout << "\n=== 회원 조회 ===\n";
    std::cout << "  1. 전체 목록\n";
    std::cout << "  2. ID로 검색\n";
    std::cout << "  3. 이름으로 검색\n";
    std::cout << "  4. 이메일로 검색\n";
    std::cout << "  0. 돌아가기\n";
    std::cout << "조회 방법 > ";

    int choice;
    if (!(std::cin >> choice))
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "\n  [오류] 숫자를 입력해 주세요.\n";
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "\n";

    switch (choice)
    {
    case 1:
        printMemberTable(repo.findAll());
        break;
    case 2:
    {
        std::cout << "  검색할 ID: ";
        int id;
        if (!(std::cin >> id))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "\n  [오류] 숫자를 입력해 주세요.\n";
            return;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        auto m = repo.findById(id);
        if (!m) std::cout << "\n  [오류] ID " << id << "인 회원을 찾을 수 없습니다.\n";
        else  { std::cout << "\n"; printMember(*m); }
        break;
    }
    case 3:
    {
        std::cout << "  이름 검색어: ";
        std::string kw; std::getline(std::cin, kw);
        if (kw.empty()) { std::cout << "  [오류] 검색어를 입력해 주세요.\n"; return; }
        printMemberTable(repo.findByKeyword("name", kw));
        break;
    }
    case 4:
    {
        std::cout << "  이메일 검색어: ";
        std::string kw; std::getline(std::cin, kw);
        if (kw.empty()) { std::cout << "  [오류] 검색어를 입력해 주세요.\n"; return; }
        printMemberTable(repo.findByKeyword("email", kw));
        break;
    }
    case 0:
        break;
    default:
        std::cout << "  [오류] 0~4 사이의 번호를 입력해 주세요.\n";
        break;
    }
}

static void handleUpdate(MemberRepository& repo)
{
    std::cout << "\n=== 회원 수정 ===\n";
    std::cout << "  수정할 회원 ID: ";

    int id;
    if (!(std::cin >> id))
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "\n  [오류] 숫자를 입력해 주세요.\n";
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    auto member = repo.findById(id);
    if (!member)
    {
        std::cout << "\n  [오류] ID " << id << "인 회원을 찾을 수 없습니다.\n";
        return;
    }

    std::cout << "\n  현재 정보:\n";
    printMember(*member);

    std::cout << "\n  새 값 입력 (변경하지 않을 항목은 Enter):\n";

    std::string name, email, phone;
    std::cout << "  이름     [" << member->name  << "] : "; std::getline(std::cin, name);
    std::cout << "  이메일   [" << member->email << "] : "; std::getline(std::cin, email);
    std::cout << "  전화번호 [" << member->phone << "] : "; std::getline(std::cin, phone);

    if (name.empty())  name  = member->name;
    if (email.empty()) email = member->email;
    if (phone.empty()) phone = member->phone;

    if (repo.existsEmail(email, id))
    {
        std::cout << "\n  [오류] 이미 다른 회원이 사용 중인 이메일입니다.\n";
        return;
    }

    repo.update(id, name, email, phone);
    std::cout << "\n  수정 완료\n";
}

static void handleDelete(MemberRepository& repo)
{
    std::cout << "\n=== 회원 삭제 ===\n";
    std::cout << "  삭제할 회원 ID: ";

    int id;
    if (!(std::cin >> id))
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "\n  [오류] 숫자를 입력해 주세요.\n";
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    auto member = repo.findById(id);
    if (!member)
    {
        std::cout << "\n  [오류] ID " << id << "인 회원을 찾을 수 없습니다.\n";
        return;
    }

    std::cout << "\n  삭제 대상:\n";
    printMember(*member);

    std::cout << "\n  정말 삭제하시겠습니까? (Y/N): ";
    std::string answer;
    std::getline(std::cin, answer);

    if (answer == "Y" || answer == "y")
    {
        repo.remove(id);
        std::cout << "\n  삭제 완료\n";
    }
    else
    {
        std::cout << "\n  삭제를 취소했습니다.\n";
    }
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    MemberRepository repo("members.json");

    int choice = -1;

    while (true)
    {
        printMenu();

        if (!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "\n  [오류] 숫자를 입력해 주세요.\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice)
        {
        case 1: handleCreate(repo); break;
        case 2: handleRead(repo);   break;
        case 3: handleUpdate(repo); break;
        case 4: handleDelete(repo); break;
        case 0:
            std::cout << "\n  프로그램을 종료합니다.\n\n";
            return 0;
        default:
            std::cout << "\n  [오류] 0~4 사이의 번호를 입력해 주세요.\n";
            break;
        }
    }
}
