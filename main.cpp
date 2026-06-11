#include <iostream>
#include <string>
#include <limits>
#define NOMINMAX
#include <windows.h>

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

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

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
        case 1:
            std::cout << "\n  [회원 등록] 준비 중입니다.\n";
            break;
        case 2:
            std::cout << "\n  [회원 목록] 준비 중입니다.\n";
            break;
        case 3:
            std::cout << "\n  [회원 수정] 준비 중입니다.\n";
            break;
        case 4:
            std::cout << "\n  [회원 삭제] 준비 중입니다.\n";
            break;
        case 0:
            std::cout << "\n  프로그램을 종료합니다.\n\n";
            return 0;
        default:
            std::cout << "\n  [오류] 0~4 사이의 번호를 입력해 주세요.\n";
            break;
        }
    }
}
