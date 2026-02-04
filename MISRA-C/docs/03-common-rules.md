# 03. 주요 규칙 상세

[← 이전: 규칙 분류 체계](02-rule-categories.md) | [메인으로](../README.md) | [다음: 정적 분석 도구 →](04-static-analysis.md)

---

## 📚 학습 목표

이 장을 완료하면 다음을 수행할 수 있습니다:
- Essential Type Model 이해 및 적용
- 자주 위반되는 규칙 식별 및 수정
- 안전한 포인터 사용 방법 습득
- 제어문 관련 규칙 준수

---

## 3.1 Essential Type Model

MISRA C:2012에서 도입된 **Essential Type Model**은 타입 관련 규칙의 기반입니다.

### Essential Type 분류

```
┌─────────────────────────────────────────────────────────────────┐
│                    Essential Type Categories                     │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  1. Boolean                                                     │
│     - _Bool, bool                                               │
│     - 비교/논리 연산의 결과                                      │
│                                                                 │
│  2. Signed                                                      │
│     - signed char, short, int, long, long long                  │
│     - 음수 표현 가능                                             │
│                                                                 │
│  3. Unsigned                                                    │
│     - unsigned char, unsigned short, unsigned int, etc.         │
│     - 0 이상의 값만 표현                                         │
│                                                                 │
│  4. Floating                                                    │
│     - float, double, long double                                │
│     - 부동소수점 타입                                            │
│                                                                 │
│  5. Character                                                   │
│     - char (plain), wchar_t                                     │
│     - 문자 데이터 표현                                           │
│                                                                 │
│  6. Enum                                                        │
│     - enum 타입                                                 │
│     - 명명된 상수 집합                                           │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Rule 10.1 (Required) - 부적절한 Essential Type 사용 금지

**"Operands shall not be of an inappropriate essential type"**

```c
#include <stdint.h>
#include <stdbool.h>

/* ❌ Rule 10.1 위반 예시 */

// 1. Boolean을 산술 연산에 사용
bool flag = true;
int result = flag + 5;            // ❌ Boolean 산술 연산

// 2. Signed와 Unsigned 혼합
int32_t  s32 = -10;
uint32_t u32 = 20U;
if (s32 < u32) { }                // ❌ 부호 혼합 비교

// 3. Char를 산술 연산에 사용
char c = 'A';
int ascii = c + 1;                // ❌ Char 산술 연산

// 4. Float을 비트 연산에 사용
float f = 3.14f;
// int bits = f & 0xFF;           // ❌ 컴파일 에러 (명백한 오류)


/* ✅ Rule 10.1 준수 코드 */

// 1. Boolean은 논리 연산에만
bool flag = true;
bool result = flag && some_condition;  // ✅ 논리 연산

// 2. 같은 부호끼리 비교
int32_t s32_a = -10;
int32_t s32_b = 20;
if (s32_a < s32_b) { }            // ✅ 같은 signed

// 3. 명시적 타입 변환
char c = 'A';
int ascii = (int)(unsigned char)c + 1;  // ✅ 명시적 변환
```

### Rule 10.3 (Required) - Narrowing 변환 금지

**"The value of an expression shall not be assigned to an object with a narrower essential type"**

```c
/* ❌ Rule 10.3 위반 - Narrowing Conversion */

uint32_t large = 0x12345678U;
uint16_t small = large;           // ❌ 32bit → 16bit 손실

int32_t signed_val = -100;
uint32_t unsigned_val = signed_val;  // ❌ 부호 손실

float f = 3.14159f;
int i = f;                        // ❌ 소수점 손실


/* ✅ Rule 10.3 준수 - 명시적 캐스팅 */

uint32_t large = 0x12345678U;

// 방법 1: 명시적 캐스팅 (의도 표현)
uint16_t small = (uint16_t)large;

// 방법 2: 값 검증 후 할당 (더 안전)
if (large <= UINT16_MAX) {
    uint16_t small = (uint16_t)large;
} else {
    // 에러 처리
}

// 방법 3: 마스킹으로 명시적 범위 제한
uint16_t masked = (uint16_t)(large & 0xFFFFU);
```

### Rule 10.4 (Required) - 산술 연산 Essential Type 일치

**"Both operands of an operator... shall have the same essential type category"**

```c
/* ❌ Rule 10.4 위반 */
int32_t  signed_val = 10;
uint32_t unsigned_val = 20U;

// 부호 혼합 산술 연산
int32_t result = signed_val + unsigned_val;  // ❌

// Boolean과 정수 혼합
bool flag = true;
int32_t val = flag * 10;          // ❌


/* ✅ Rule 10.4 준수 */

// 같은 타입으로 통일
int32_t signed_val = 10;
int32_t other_val = 20;
int32_t result = signed_val + other_val;  // ✅

// 명시적 변환 후 연산
uint32_t u32 = 20U;
int32_t  s32 = 10;
int32_t result2 = s32 + (int32_t)u32;     // ✅ (값 범위 확인 필요)
```

---

## 3.2 포인터 관련 규칙

### Rule 11.3 (Required) - 포인터-정수 변환 금지

**"A cast shall not be performed between a pointer to object type and a pointer to a different object type"**

```c
/* ❌ Rule 11.3 위반 */

uint32_t data = 0x12345678U;
uint8_t *byte_ptr = (uint8_t *)&data;     // ❌ 다른 타입 포인터 변환

struct TypeA { int x; };
struct TypeB { int y; };
struct TypeA a;
struct TypeB *b_ptr = (struct TypeB *)&a;  // ❌


/* ✅ 안전한 대안 */

// 방법 1: memcpy 사용
uint32_t data = 0x12345678U;
uint8_t bytes[4];
memcpy(bytes, &data, sizeof(data));       // ✅

// 방법 2: union 사용 (MISRA 예외)
typedef union {
    uint32_t word;
    uint8_t  bytes[4];
} DataUnion;

DataUnion du;
du.word = 0x12345678U;
uint8_t b0 = du.bytes[0];                 // ✅ (Rule 19.2 참고)
```

### Rule 11.5 (Advisory) - void 포인터 변환

**"A conversion should not be performed from pointer to void into pointer to object"**

```c
/* ⚠️ Rule 11.5 주의 필요 */

// malloc 반환값 사용 (일반적인 패턴)
void *raw = malloc(sizeof(int) * 10);
int *arr = (int *)raw;                    // ⚠️ Advisory 위반

/* ✅ 권장 패턴 */

// 래퍼 함수 사용
static int *allocate_int_array(size_t count) {
    void *ptr = malloc(sizeof(int) * count);
    return (int *)ptr;  // 한 곳에서만 변환
}

// 호출 측
int *arr = allocate_int_array(10);        // ✅ 변환이 캡슐화됨
```

### Rule 18.1 (Required) - 배열 범위 접근

**"A pointer resulting from arithmetic on a pointer operand shall address an element of the same array"**

```c
/* ❌ Rule 18.1 위반 */

int arr[10];
int *ptr = arr;

ptr = ptr + 15;                   // ❌ 배열 범위 초과
*ptr = 100;                       // ❌ 정의되지 않은 동작

int *end = &arr[10];              // ✅ one-past-end는 허용
*end = 0;                         // ❌ 역참조는 금지


/* ✅ Rule 18.1 준수 */

int arr[10];

// 명시적 범위 검사
for (size_t i = 0U; i < 10U; i++) {
    arr[i] = (int)i;              // ✅ 인덱스 기반 접근
}

// 포인터 사용 시 범위 검사
int *ptr = arr;
int *end = &arr[10];              // one-past-end
while (ptr < end) {
    *ptr = 0;
    ptr++;
}
```

### Rule 18.6 (Required) - Dangling Pointer 금지

**"The address of an object with automatic storage shall not be copied to another object that persists after the first object has ceased to exist"**

```c
/* ❌ Rule 18.6 위반 - Dangling Pointer */

int *global_ptr;

void bad_function(void) {
    int local_var = 42;
    global_ptr = &local_var;      // ❌ 지역 변수 주소를 전역에 저장
}  // local_var 소멸 → global_ptr은 dangling

int *return_local(void) {
    int local = 100;
    return &local;                // ❌ 지역 변수 주소 반환
}


/* ✅ Rule 18.6 준수 */

// static 변수 사용
int *return_static(void) {
    static int persistent = 100;
    return &persistent;           // ✅ static은 프로그램 종료까지 유지
}

// 동적 할당 사용
int *create_value(int val) {
    int *ptr = (int *)malloc(sizeof(int));
    if (ptr != NULL) {
        *ptr = val;
    }
    return ptr;                   // ✅ heap 메모리
}

// 호출자 버퍼 사용
void get_value(int *out_ptr) {
    if (out_ptr != NULL) {
        *out_ptr = 100;           // ✅ 호출자가 메모리 관리
    }
}
```

---

## 3.3 제어문 규칙

### Rule 14.3 (Required) - 상수 제어 표현식 금지

**"Controlling expressions shall not be invariant"**

```c
/* ❌ Rule 14.3 위반 */

// 항상 참인 조건
if (1) {                          // ❌ 상수 조건
    do_something();
}

// 항상 거짓인 조건
while (0) {                       // ❌ 상수 조건 (Dead Code)
    never_executed();
}

// 상수 비교
#define MAX_SIZE 100
int size = 50;
if (size < MAX_SIZE) {            // ⚠️ 컴파일러가 상수로 판단할 수 있음
    // ...
}


/* ✅ Rule 14.3 준수 */

// 변수 조건 사용
extern bool debug_enabled;

if (debug_enabled) {              // ✅ 런타임 변수
    log_debug_info();
}

// volatile 사용 (의도적 상수 허용)
volatile bool always_true = true;
if (always_true) {                // ✅ volatile이므로 런타임 평가
    // ...
}
```

### Rule 15.4 (Advisory) - 단일 break/goto

**"There should be no more than one break or goto statement used to terminate any iteration statement"**

```c
/* ⚠️ Rule 15.4 권고 위반 */

for (int i = 0; i < 100; i++) {
    if (condition1) {
        break;                    // break 1
    }
    if (condition2) {
        break;                    // break 2 - 두 번째 break
    }
}


/* ✅ Rule 15.4 준수 */

// 플래그 변수 사용
bool done = false;
for (int i = 0; (i < 100) && (!done); i++) {
    if (condition1) {
        done = true;
    } else if (condition2) {
        done = true;
    }
}

// 함수 분리
static bool search_condition(int index) {
    return condition1 || condition2;
}

for (int i = 0; i < 100; i++) {
    if (search_condition(i)) {
        break;                    // ✅ 단일 break
    }
}
```

### Rule 15.6 (Required) - 복합문 사용

**"The body of an iteration-statement or a selection-statement shall be a compound-statement"**

```c
/* ❌ Rule 15.6 위반 */

if (x > 0)
    x--;                          // ❌ 중괄호 없음

for (int i = 0; i < 10; i++)
    array[i] = 0;                 // ❌ 중괄호 없음

while (ptr != NULL)
    ptr = ptr->next;              // ❌ 중괄호 없음


/* ✅ Rule 15.6 준수 */

if (x > 0) {
    x--;
}

for (int i = 0; i < 10; i++) {
    array[i] = 0;
}

while (ptr != NULL) {
    ptr = ptr->next;
}

// 빈 본문도 복합문으로
while (wait_flag) {
    /* 의도적으로 비움 - 폴링 대기 */
}
```

### Rule 16.4 (Required) - switch default 필수

**"Every switch statement shall have a default label"**

```c
/* ❌ Rule 16.4 위반 */

typedef enum { RED, GREEN, BLUE } Color;

void process_color(Color c) {
    switch (c) {
        case RED:
            handle_red();
            break;
        case GREEN:
            handle_green();
            break;
        case BLUE:
            handle_blue();
            break;
        // default 없음!
    }
}


/* ✅ Rule 16.4 준수 */

void process_color(Color c) {
    switch (c) {
        case RED:
            handle_red();
            break;
        case GREEN:
            handle_green();
            break;
        case BLUE:
            handle_blue();
            break;
        default:
            /* 예상치 못한 값 처리 */
            error_handler();
            break;
    }
}
```

---

## 3.4 함수 규칙

### Rule 17.7 (Required) - 반환값 사용

**"The value returned by a function having non-void return type shall be used"**

```c
/* ❌ Rule 17.7 위반 */

(void)printf("Hello\n");          // printf 반환값 무시

FILE *fp = fopen("file.txt", "r");
fread(buffer, 1, 100, fp);        // ❌ fread 반환값 무시


/* ✅ Rule 17.7 준수 */

// 방법 1: 반환값 사용
int chars_printed = printf("Hello\n");
if (chars_printed < 0) {
    // 에러 처리
}

// 방법 2: 반환값 검사
size_t bytes_read = fread(buffer, 1, 100, fp);
if (bytes_read < 100U) {
    if (feof(fp)) {
        // EOF 처리
    } else {
        // 에러 처리
    }
}

// 방법 3: 명시적 void 캐스팅 (의도적 무시)
(void)memset(buffer, 0, sizeof(buffer));  // memset 반환값은 buffer 자신
```

### Rule 17.2 (Required) - 재귀 금지

**"Functions shall not call themselves, either directly or indirectly"**

```c
/* ❌ Rule 17.2 위반 - 직접 재귀 */
uint32_t factorial(uint32_t n) {
    if (n == 0U) {
        return 1U;
    }
    return n * factorial(n - 1U);  // ❌ 직접 재귀
}

/* ❌ Rule 17.2 위반 - 간접 재귀 */
void func_a(void) {
    func_b();                     // b 호출
}

void func_b(void) {
    func_a();                     // ❌ a 호출 → 간접 재귀
}


/* ✅ Rule 17.2 준수 - 반복문 사용 */
uint32_t factorial(uint32_t n) {
    uint32_t result = 1U;
    for (uint32_t i = 2U; i <= n; i++) {
        result *= i;
    }
    return result;
}

/* ✅ 트리 순회 - 스택 기반 */
void traverse_tree(Node *root) {
    Node *stack[MAX_DEPTH];
    int top = 0;
    
    stack[top++] = root;
    
    while (top > 0) {
        Node *current = stack[--top];
        process_node(current);
        
        if (current->right != NULL) {
            stack[top++] = current->right;
        }
        if (current->left != NULL) {
            stack[top++] = current->left;
        }
    }
}
```

---

## 3.5 전처리기 규칙

### Rule 20.4 (Required) - 매크로 함수 주의

**"A macro shall not be defined with the same name as a keyword"**

```c
/* ❌ Rule 20.4 및 관련 규칙 위반 */

#define if while                  // ❌ 키워드 재정의
#define true 0                    // ❌ 표준 매크로 재정의


/* ❌ 매크로 부작용 */
#define SQUARE(x) ((x) * (x))

int a = 5;
int result = SQUARE(a++);         // ❌ a가 두 번 증가
// 전개: ((a++) * (a++)) - 정의되지 않은 동작


/* ✅ 안전한 매크로 또는 inline 함수 */

// inline 함수 권장
static inline int square(int x) {
    return x * x;
}

int a = 5;
int result = square(a++);         // ✅ a는 한 번만 증가

// 매크로 사용 시 부작용 없는 인자만
#define MAX_SIZE 100U             // ✅ 상수 매크로
#define ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))  // ✅
```

### Rule 20.7 (Required) - 매크로 매개변수 괄호

**"Expressions resulting from the expansion of macro parameters shall be enclosed in parentheses"**

```c
/* ❌ Rule 20.7 위반 */

#define ADD(a, b) a + b
int result = ADD(1, 2) * 3;       // 전개: 1 + 2 * 3 = 7 (의도: 9)

#define MUL(a, b) (a * b)
int result2 = MUL(1+2, 3+4);      // 전개: (1+2 * 3+4) = 11 (의도: 21)


/* ✅ Rule 20.7 준수 */

#define ADD(a, b) ((a) + (b))
int result = ADD(1, 2) * 3;       // 전개: ((1) + (2)) * 3 = 9 ✅

#define MUL(a, b) ((a) * (b))
int result2 = MUL(1+2, 3+4);      // 전개: ((1+2) * (3+4)) = 21 ✅
```

---

## 3.6 표준 라이브러리 규칙

### Rule 21.3 (Required) - 동적 메모리 함수

**"The memory allocation and deallocation functions of <stdlib.h> shall not be used"**

```c
/* ❌ Rule 21.3 위반 */

#include <stdlib.h>

void *ptr = malloc(100);          // ❌ malloc 금지
void *ptr2 = calloc(10, sizeof(int));  // ❌ calloc 금지
ptr = realloc(ptr, 200);          // ❌ realloc 금지
free(ptr);                        // ❌ free 금지


/* ✅ 대안: 정적 할당 */

// 컴파일 타임 할당
static uint8_t buffer[BUFFER_SIZE];

// 메모리 풀 패턴
#define POOL_SIZE 1024U
static uint8_t memory_pool[POOL_SIZE];
static size_t pool_index = 0U;

void *pool_alloc(size_t size) {
    void *ptr = NULL;
    if ((pool_index + size) <= POOL_SIZE) {
        ptr = &memory_pool[pool_index];
        pool_index += size;
    }
    return ptr;
}
```

### Rule 21.6 (Required) - 표준 I/O 금지

**"The Standard Library input/output functions shall not be used"**

```c
/* ❌ Rule 21.6 위반 */

#include <stdio.h>

printf("Debug: %d\n", value);     // ❌ printf
scanf("%d", &input);              // ❌ scanf
FILE *fp = fopen("log.txt", "w"); // ❌ fopen


/* ✅ 대안: 커스텀 출력 함수 */

// UART 출력 예시 (STM32)
void debug_print(const char *msg) {
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), 1000);
}

// 정수 출력 함수
void debug_print_int(const char *label, int value) {
    char buffer[32];
    int len = snprintf(buffer, sizeof(buffer), "%s: %d\r\n", label, value);
    if (len > 0) {
        HAL_UART_Transmit(&huart2, (uint8_t *)buffer, (uint16_t)len, 1000);
    }
}
```

---

## 3.7 자주 위반되는 규칙 TOP 10

| 순위 | 규칙 | 카테고리 | 설명 |
|------|------|----------|------|
| 1 | Rule 10.3 | Required | Narrowing 변환 |
| 2 | Rule 10.4 | Required | 부호 혼합 연산 |
| 3 | Rule 15.6 | Required | 복합문 미사용 |
| 4 | Rule 8.4 | Required | 외부 선언 호환성 |
| 5 | Rule 10.1 | Required | 부적절한 Essential Type |
| 6 | Rule 14.3 | Required | 상수 제어 표현식 |
| 7 | Rule 2.2 | Required | Dead Code |
| 8 | Rule 17.7 | Required | 반환값 미사용 |
| 9 | Rule 11.3 | Required | 포인터 타입 변환 |
| 10 | Rule 20.7 | Required | 매크로 괄호 |

---

## 📝 실습 과제

### 과제 1: 다음 코드의 MISRA 위반을 찾고 수정하세요

```c
#include <stdio.h>
#include <stdlib.h>

#define DOUBLE(x) x * 2

int process_data(int *data, int size) {
    int sum = 0;
    
    for (int i = 0; i <= size; i++)
        sum += data[i];
    
    if (size > 0)
        printf("Sum: %d\n", sum);
        return sum;
    
    return 0;
}

int main() {
    int *arr = malloc(10 * sizeof(int));
    
    for (int i = 0; i < 10; i++)
        arr[i] = DOUBLE(i + 1);
    
    process_data(arr, 10);
    
    return 0;
}
```

<details>
<summary>정답 보기</summary>

```c
#include <stdint.h>
#include <string.h>

/* Rule 20.7: 매크로 매개변수 괄호 */
#define DOUBLE(x) ((x) * 2)

/* 정적 배열 사용 (Rule 21.3: malloc 금지) */
#define ARRAY_SIZE 10U
static int32_t static_array[ARRAY_SIZE];

/* Rule 21.6: printf 대신 커스텀 출력 */
static void print_sum(int32_t sum) {
    /* 실제 구현은 하드웨어에 맞게 */
    (void)sum;
}

int32_t process_data(const int32_t *data, uint32_t size) {
    int32_t sum = 0;
    
    /* Rule 15.6: 복합문 사용 */
    /* 범위 수정: i < size (Rule 18.1: 배열 범위) */
    for (uint32_t i = 0U; i < size; i++) {
        sum += data[i];
    }
    
    /* Rule 15.6: 복합문 사용 */
    if (size > 0U) {
        print_sum(sum);
    }
    
    return sum;
}

int main(void) {
    /* Rule 15.6: 복합문 사용 */
    for (uint32_t i = 0U; i < ARRAY_SIZE; i++) {
        static_array[i] = DOUBLE((int32_t)i + 1);
    }
    
    (void)process_data(static_array, ARRAY_SIZE);
    
    return 0;
}
```

위반 목록:
1. Rule 21.6 - stdio.h 사용
2. Rule 21.3 - malloc 사용
3. Rule 20.7 - 매크로 괄호 누락
4. Rule 15.6 - for/if 복합문 미사용
5. Rule 18.1 - 배열 범위 초과 (i <= size)
6. Rule 17.7 - process_data 반환값 미사용

</details>

---

## 📚 다음 학습

다음 장에서는 정적 분석 도구를 사용하여 MISRA 규칙 위반을 자동으로 검출하는 방법을 학습합니다:
- Cppcheck 설치 및 설정
- MISRA addon 사용법
- CI/CD 통합

[다음: 04. 정적 분석 도구 →](04-static-analysis.md)

---

## 🔗 참고 자료

- [Essential Type Model 설명](https://www.perforce.com/resources/qac/misra-c-cpp)
- [Cppcheck MISRA 체크 목록](https://cppcheck.sourceforge.io/)
