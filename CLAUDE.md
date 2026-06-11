# CRUD 프로젝트

JSON 파일을 영속성 저장소로 사용하는 회원정보 CRUD 시스템 (C++).

## 프로젝트 개요

- **언어**: C++
- **데이터 저장소**: JSON 파일
- **대상 데이터**: 회원정보 (Member)
- **빌드 시스템**: Visual Studio (`.vcxproj`) — PoC와 동일한 방식 사용

## 회원정보 데이터 구조

회원 한 명을 나타내는 JSON 스키마:

```json
{
  "id": 1,
  "name": "홍길동",
  "email": "hong@example.com",
  "phone": "010-1234-5678",
  "active": true,
  "createdAt": "2026-06-11"
}
```

전체 데이터 파일은 회원 배열을 최상위로 가짐:

```json
[
  { "id": 1, "name": "홍길동", ... },
  { "id": 2, "name": "김철수", ... }
]
```

## JSON 처리 라이브러리 (JsonLib)

> **참고 PoC**: https://github.com/pisatang-code/JsonPoc.git  
> PoC의 `JsonLib.h` / `JsonLib.cpp`를 그대로 가져와 사용한다.

### 핵심 클래스

#### `JsonValue`
모든 JSON 값을 표현하는 단일 타입. 내부적으로 `std::variant`를 사용해 타입 안전성 보장.

| 타입 | 열거값 | 접근자 |
|------|--------|--------|
| null | `Type::Null` | `isNull()` |
| bool | `Type::Bool` | `asBool()` |
| number | `Type::Number` | `asNumber()` |
| string | `Type::String` | `asString()` |
| array | `Type::Array` | `asArray()` |
| object | `Type::Object` | `asObject()` |

**주요 사용 패턴:**
```cpp
// 객체 필드 접근
JsonValue member;
member["id"]    = JsonValue(1);
member["name"]  = JsonValue("홍길동");
member["active"]= JsonValue(true);

// 배열 요소 추가
JsonValue members;   // null → array로 자동 전환 후 push
members.push(member);

// 읽기
int    id   = (int)member["id"].asNumber();
string name = member["name"].asString();
bool   act  = member["active"].asBool();
```

#### `JsonParser`
JSON 문자열 또는 파일을 `JsonValue` 트리로 파싱.

```cpp
// 파일에서 읽기
JsonValue root = JsonParser::parseFile("members.json");

// 문자열에서 파싱
JsonValue root = JsonParser::parse(rawString);
```

- 파싱 실패, 파일 없음, 잘못된 JSON → `std::exception` throw

#### `JsonWriter`
`JsonValue` 트리를 JSON 문자열 또는 파일로 직렬화.

```cpp
// 파일에 저장 (pretty print, 기본값)
JsonWriter::saveFile(root, "members.json");

// 파일에 저장 (compact)
JsonWriter::saveFile(root, "members.json", false);

// 문자열로 직렬화
std::string json = JsonWriter::serialize(root);
```

### PoC에서 확인된 동작 특성

- 객체 키는 `std::map<string, JsonValue>` 기반 → 저장 시 **알파벳 순 정렬**
- `operator[]`로 없는 키 접근 시 `null` 노드 자동 생성
- `hasKey(key)` 메서드로 키 존재 여부 확인
- 숫자는 모두 `double`로 저장 → 정수 ID는 `(int)value.asNumber()`로 캐스팅

## 요구사항

### 개발 원칙
- PoC(`JsonPoc`)에서 사용된 **코드 구조를 유지**한 상태로 CRUD 구현
- `JsonLib.h` / `JsonLib.cpp`를 그대로 가져와 사용 (수정 금지)
- 외부 JSON 라이브러리 도입 금지 — JsonLib만 사용

### CRUD 기능 요구사항

| 기능 | 세부 요구사항 |
|------|--------------|
| **Create** | 새로운 데이터를 입력 받아 JSON 파일에 저장. 자동 ID 채번 (현재 최대 ID + 1) |
| **Read** | 전체 목록 보기 및 특정 ID/키 값으로 검색 기능 |
| **Update** | 기존 데이터를 선택하여 특정 필드 수정 후 JSON 파일에 반영 |
| **Delete** | 특정 데이터를 안전하게 삭제 (삭제 전 확인 절차 포함) |

## 파일 구성 계획

```
CRUD/
├── JsonLib.h          # PoC에서 가져온 JSON 라이브러리 헤더
├── JsonLib.cpp        # PoC에서 가져온 JSON 라이브러리 구현
├── Member.h           # Member 데이터 구조체 + JsonValue 변환 헬퍼
├── MemberRepository.h # JSON 파일 기반 CRUD 인터페이스
├── MemberRepository.cpp
├── main.cpp           # 진입점 (메뉴 또는 테스트 드라이버)
├── members.json       # 데이터 파일 (런타임 생성)
└── CRUD.vcxproj       # Visual Studio 프로젝트 파일
```

## Git 운영 규칙

- commit / push는 **명시적 요청이 있을 때만** 수행한다.
- 단, **"Phase N 문제없음 확인"** 메시지를 받으면 해당 Phase 작업을 즉시 commit & push한다.

## 개발 계획

단계별 구현 목표는 [`docs/PLAN.md`](docs/PLAN.md)에 Phase별로 정리되어 있다.

## 알려진 빌드 주의사항

코드 작성 전 아래 규칙을 확인한다.

| 상황 | 잘못된 방법 | 올바른 방법 |
|------|------------|------------|
| `windows.h` 포함 시 | `#include <windows.h>` 단독 사용 | `#define NOMINMAX` 먼저 선언 후 포함 |

### `NOMINMAX` — windows.h + std::numeric_limits 충돌

`windows.h`는 `min` / `max`를 전역 매크로로 정의한다.
이 매크로가 `std::numeric_limits<T>::max()` 호출을 오염시켜 **빌드 오류**가 발생한다.

```cpp
// 항상 이 순서로 작성
#define NOMINMAX
#include <windows.h>
```

---

## 인코딩 규칙

한글 깨짐 방지를 위해 아래 규칙을 반드시 준수한다.

| 항목 | 설정 |
|------|------|
| 소스 파일 인코딩 | **UTF-8** (BOM 없음) |
| 컴파일러 플래그 | `/utf-8` — MSVC가 소스를 UTF-8로 해석하고 내부 문자열도 UTF-8로 인코딩 |
| 런타임 콘솔 설정 | `SetConsoleOutputCP(CP_UTF8)` + `SetConsoleCP(CP_UTF8)` — `main()` 진입 직후 호출 |

**한글 문자열을 사용하는 모든 `.cpp` / `.h` 파일은 UTF-8로 저장한다.**

```cpp
// main.cpp — 항상 main() 첫 줄에 포함
#define NOMINMAX    // windows.h의 min/max 매크로가 std::numeric_limits와 충돌하는 것을 방지
#include <windows.h>
// ...
int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    // ...
}
```

## 빌드 환경

- Visual Studio 2022 (또는 호환 버전)
- C++17 이상 (`std::variant` 사용)
- 별도 외부 라이브러리 없음 — JsonLib만 사용
