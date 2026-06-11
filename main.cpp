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

    Member m = repo.create(name, email, phone);
    std::cout << "\n  등록 완료 — ID: " << m.id << " / " << m.name << "\n";
}

static void handleRead(MemberRepository& repo)
{
    std::cout << "\n=== 회원 목록 ===\n";

    auto members = repo.findAll();

    if (members.empty())
    {
        std::cout << "  등록된 회원이 없습니다.\n";
        return;
    }

    std::cout << "  총 " << members.size() << "명이 등록되어 있습니다.\n\n";

    // 컬럼 너비 (display 기준)
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
        case 3: std::cout << "\n  [회원 수정] 준비 중입니다.\n"; break;
        case 4: std::cout << "\n  [회원 삭제] 준비 중입니다.\n"; break;
        case 0:
            std::cout << "\n  프로그램을 종료합니다.\n\n";
            return 0;
        default:
            std::cout << "\n  [오류] 0~4 사이의 번호를 입력해 주세요.\n";
            break;
        }
    }
}
