# Verilog HDL vs C 언어 비교

C 언어는 **CPU가 순차적으로 실행하는 소프트웨어**를 작성하는 언어이고, Verilog HDL은 **동시에 동작하는 하드웨어 회로를 기술(Description)** 하는 언어입니다. 문법은 비슷해 보여도 코드가 의미하는 바가 근본적으로 다릅니다.

---

## 1. 기본 문법 요소 비교

| 구분 | C 언어 (Software) | Verilog HDL (Hardware Description) |
|---|---|---|
| **상수 (Constants)** | • 리터럴: `10`, `3.14`, `'A'`<br>• 매크로: `#define MAX 100`<br>• 상수 변수: `const int LIMIT = 10;` | • 진수 지정 표기: `8'b1010_0101` (2진수), `16'hFF00` (16진수), `32'd100` (10진수)<br>• 파라미터: `parameter WIDTH = 8;`, `localparam`을 통한 상수 정의 |
| **변수 (Variables)** | • 메모리 저장 공간 (`int a;`, `char c;`)<br>• 프로그램 실행 중 값이 자유롭게 변경되며 RAM에 적재됨 | • 하드웨어 신호 및 저장 요소<br>• `wire`: 값을 유지하지 못하는 단순 연결선 (조합 회로)<br>• `reg`: `always` 블록 등에서 값을 유지·할당받는 변수 (이름과 달리 항상 실제 레지스터/플립플롭이 되는 것은 아님) |
| **순환문 (Loops)** | • `while`, `for`, `do ~ while`<br>• CPU가 명령어를 순차 실행하며 반복 횟수만큼 연산 수행 | • `for`, `while` (주로 테스트벤치 및 `always`/`generate` 문 내부에서 사용)<br>• 합성(Synthesis) 시 반복되는 회로 구조를 물리적으로 펼치는(Unrolling) 용도로 주로 활용 |
| **제어문 / 선택문** | • `if`, `if ~ else`, `switch ~ case`<br>• CPU의 분기(Branch) 명령어를 통해 조건에 따라 코드 실행 흐름 변경 | • `if ~ else`, `case`, `casez`, `casex`<br>• 합성 시 멀티플렉서(MUX), 우선순위 인코더 등의 조합 논리 회로로 변환됨 |

---

## 2. 추가 주요 개념 비교

| 구분 | C 언어 (소프트웨어 중심) | Verilog HDL (하드웨어 기술 중심) |
|---|---|---|
| **배열 (Arrays)** | • 1차원, 다차원 배열 (`int arr[10];`)<br>• 포인터와 연동되며 자유로운 인덱스 조작 가능 | • 벡터(Vector)와 메모리(Memory) 구조로 구분<br>• `reg [7:0] data;` (8비트 폭 벡터)<br>• `reg [7:0] mem [0:255];` (8비트 × 256개 배열) |
| **포인터 및 동적 메모리** | • 포인터(`*`)로 메모리 주소 직접 제어<br>• `malloc`, `free`로 동적 메모리 할당 | • 포인터 개념 없음 (하드웨어 회로에는 동적으로 변하는 주소 포인터가 성립하지 않음)<br>• 모든 자원은 컴파일(합성) 시점에 고정됨 |
| **함수 및 서브루틴** | • 함수(Function): 값을 계산하고 `return`<br>• CPU가 호출 스택을 이용해 순차적으로 실행 | • `function`: 시간 지연이 없고 조합 논리 합성을 위한 순수 연산용 (반드시 반환값 존재)<br>• `task`: 시간 지연(`#` 등)을 포함할 수 있는 프로시저 (주로 테스트벤치/시뮬레이션용) |
| **동시성 및 타이밍** | • 순차 실행(Sequential): 코드가 기본적으로 위에서 아래로 실행됨 | • 병렬 동시 동작: 모든 `always` 블록, `assign` 문이 동시에 동작<br>• `#` (지연), `@(posedge clk)` (클럭 에지 감지) 등 타이밍 제어 구문 존재 |
| **모듈 및 구조적 계층** | • 함수와 파일 분할 (`#include`, `extern`)<br>• 소스 파일 단위로 분할 컴파일 | • 모듈(Module) 인스턴스화: 실제 IC 칩처럼 하부 모듈을 레고 블록처럼 조립하여 상위 회로를 구성하는 계층 구조 |
| **시스템 입출력 및 제어** | • `printf`, `scanf`<br>• 파일 입출력 (`fopen`, `fread` 등) | • 시뮬레이션 전용 시스템 태스크<br>&nbsp;&nbsp;- `$display`: 출력<br>&nbsp;&nbsp;- `$monitor`: 신호 변화 추적<br>&nbsp;&nbsp;- `$finish`: 시뮬레이션 종료<br>&nbsp;&nbsp;- `$random`: 난수 생성 |
| **전처리기 / 컴파일러 지시어** | • `#define`, `#include`, `#ifdef` 등 샵(`#`) 기호 사용 | • ``` `define ```, ``` `include ```, ``` `timescale ``` (시간 단위 및 정밀도 지정), ``` `ifdef ``` 등 백틱(`` ` ``) 기호 사용 |

---

## 3. 핵심 차이점 요약

### 3.1 변수 개념의 차이
- **C 언어**: 변수는 메모리(RAM) 주소에 할당되어 프로그램 실행 중 값이 읽히고 쓰이는 **데이터 저장소**입니다.
- **Verilog**: 변수(`wire`, `reg`)는 실제 디지털 회로의 **신호선(Wire)** 이나 **플립플롭/래치 등 기억 소자(Storage Element)** 를 추상화하여 표현한 것입니다.

### 3.2 순환문·제어문의 하드웨어적 의미
- **C**: 반복문과 제어문은 CPU 내부의 **명령어 실행 순서**를 제어합니다.
- **Verilog**: 같은 문법이 클럭(Clock)에 맞춰 동시에 동작하는 **병렬 하드웨어 회로(조합 논리 및 순차 논리 회로)** 를 구성하기 위한 기술(Description) 수단으로 사용됩니다.

### 3.3 배열과 포인터의 부재 (Verilog 관점)
- C 언어는 포인터와 실행 중 크기를 늘릴 수 있는 동적 메모리 할당이 핵심입니다.
- Verilog는 물리 회로를 설계하는 언어이므로 **포인터나 동적 할당이 불가능**합니다.
- 대신 **벡터(Vector, 예: `[7:0]`)** 로 여러 가닥의 신호선을 한 번에 묶어 다루며, 다차원 배열은 내부적으로 **메모리(RAM/ROM) 소자**를 합성하는 데 사용됩니다.

### 3.4 함수(Function) vs 태스크(Task)
- C의 함수는 인자를 받아 연산하고 값을 반환합니다.
- Verilog의 `function`은 하드웨어 **조합 회로(게이트 조합)** 로 매핑되어야 하므로 **시간 지연이 절대 들어갈 수 없습니다.**
- 반면 `task`는 내부에 시간 지연(`@`, `#`)을 포함할 수 있어, 주로 **테스트벤치(시뮬레이션 검증 코드)** 에서 클럭 주기에 맞춰 신호를 조작할 때 유용합니다.

### 3.5 컴파일러 지시어 기호의 차이
- C 언어는 전처리기 지시어에 `#` 기호를 사용합니다. (예: `#define`)
- Verilog HDL은 시스템 태스크(`$`)와 구분하기 위해 전처리기 지시어에 **백틱(`` ` ``)** 기호를 사용합니다. (예: `` `define ``)

---

## 4. 예제로 보는 비교

같은 개념을 C와 Verilog로 각각 작성해 비교합니다.

### 4.1 상수와 변수

**C 언어** — 변수는 RAM에 저장되고, 실행 중 값이 갱신됩니다.

```c
#define WIDTH 8
const int LIMIT = 10;

int a = 5;
a = a + 1;          // 메모리의 값을 읽어 1을 더한 뒤 다시 저장
```

**Verilog HDL** — 변수는 신호선(`wire`) 또는 기억 소자가 될 수 있는 변수(`reg`)입니다.

```verilog
localparam WIDTH = 8;               // 컴파일(합성) 시점에 고정되는 상수

wire [WIDTH-1:0] sum;
reg  [WIDTH-1:0] cnt;

assign sum = a + b;                 // a, b가 바뀌면 sum이 항상 따라 바뀌는 가산기 회로

always @(posedge clk)               // 클럭 상승 에지마다 값을 저장하는 8비트 카운터
    cnt <= cnt + 1;

wire [7:0] c = 8'b1010_0101;        // 진수 지정 리터럴
```

> C의 `a = a + 1`은 "한 번 실행되는 명령"이고, Verilog의 `assign sum = a + b`는 "항상 존재하는 회로"입니다.

---

### 4.2 순환문 (Loops)

**C 언어** — CPU가 반복 횟수만큼 명령어를 순서대로 실행합니다.

```c
int ones = 0;
for (int i = 0; i < 8; i++) {
    ones += (data >> i) & 1;        // 8번 반복 실행
}
```

**Verilog HDL** — 반복문은 회로를 **펼쳐서(Unrolling)** 8개 비트를 더하는 가산기 구조로 합성됩니다.

```verilog
reg [3:0] ones;
integer   i;

always @* begin
    ones = 0;
    for (i = 0; i < 8; i = i + 1)
        ones = ones + data[i];      // 8개 항을 더하는 조합 회로로 펼쳐짐
end
```

`generate` 문으로 같은 모듈을 여러 개 복제할 수도 있습니다.

```verilog
genvar k;
generate
    for (k = 0; k < 8; k = k + 1) begin : gen_inv
        assign y[k] = ~x[k];        // 인버터 8개를 물리적으로 생성
    end
endgenerate
```

---

### 4.3 제어문 / 선택문

**C 언어** — 조건에 따라 실행 흐름이 분기합니다.

```c
if (sel == 0)      y = a;
else if (sel == 1) y = b;
else               y = c;
```

**Verilog HDL** — 조건 구조가 멀티플렉서(MUX)로 합성됩니다.

```verilog
// 2:1 MUX
assign y = sel ? b : a;

// 4:1 MUX (case 문)
always @* begin
    case (sel)
        2'b00:   y = a;
        2'b01:   y = b;
        2'b10:   y = c;
        default: y = d;             // default를 빠뜨리면 래치가 생길 수 있음
    endcase
end
```

---

### 4.4 배열과 메모리

**C 언어**

```c
unsigned char mem[256];

mem[addr] = data;                   // 쓰기
data_out  = mem[addr];              // 읽기
```

**Verilog HDL** — 벡터는 신호선 묶음, 배열은 메모리(RAM/ROM) 구조입니다.

```verilog
reg [7:0] data;                     // 8비트 벡터 (신호선 8가닥)
reg [7:0] mem [0:255];              // 8비트 × 256개 메모리

always @(posedge clk) begin
    if (we) mem[addr] <= din;       // 동기 쓰기
end

assign dout = mem[addr];            // 비동기 읽기
```

---

### 4.5 함수와 태스크

**C 언어** — 호출 스택을 이용해 순차적으로 실행되고 값을 반환합니다.

```c
int max(int a, int b) {
    return (a > b) ? a : b;
}

int m = max(3, 7);
```

**Verilog HDL — `function`**: 시간 지연이 없는 순수 조합 연산용입니다.

```verilog
function [7:0] max;
    input [7:0] a, b;
    begin
        max = (a > b) ? a : b;
    end
endfunction

assign m = max(x, y);               // 비교기 + MUX 회로로 합성됨
```

**Verilog HDL — `task`**: 시간 지연을 포함할 수 있어 주로 테스트벤치에서 사용합니다.

```verilog
task send_byte;
    input [7:0] d;
    begin
        din = d;
        @(posedge clk);             // 클럭 에지 대기 (시간 소모)
        din = 8'h00;
    end
endtask

initial send_byte(8'hA5);
```

---

### 4.6 동시성 (Concurrency)

**C 언어** — 위에서 아래로 순서대로 실행됩니다.

```c
a = 1;
b = a;      // a=1이 이미 반영된 뒤 실행 → b = 1
c = b;      // c = 1
```

**Verilog HDL** — 모든 `assign`과 `always` 블록이 **동시에** 존재하고 동작합니다.

```verilog
assign b = a;
assign c = b;       // 순서와 무관. a가 바뀌면 b, c가 연쇄적으로 따라 바뀜
```

순차 회로에서는 **논블로킹 할당(`<=`)** 으로 클럭 에지에서 동시에 갱신합니다.

```verilog
// 3단 시프트 레지스터: d가 3클럭 뒤에 q3에 나타남
always @(posedge clk) begin
    q1 <= d;
    q2 <= q1;       // 같은 에지에서 갱신 전의 q1 값을 사용
    q3 <= q2;
end
```

> C처럼 `q1 = d; q2 = q1; q3 = q2;`(블로킹 `=`)로 쓰면 한 클럭에 `d`가 `q3`까지 전달되는 전혀 다른 동작이 됩니다. 순차 회로에는 `<=`, 조합 회로에는 `=`를 사용하는 것이 기본 규칙입니다.

---

### 4.7 모듈 (계층 구조)

**C 언어** — 함수를 호출해 기능을 재사용합니다.

```c
int full_add(int a, int b, int cin) {
    return a + b + cin;
}
```

**Verilog HDL** — 하부 모듈을 **인스턴스화(부품 배치)** 해서 상위 회로를 조립합니다.

```verilog
module full_adder (
    input  a, b, cin,
    output sum, cout
);
    assign sum  = a ^ b ^ cin;
    assign cout = (a & b) | (cin & (a ^ b));
endmodule

module adder2 (
    input  [1:0] a, b,
    output [1:0] sum,
    output       cout
);
    wire c0;
    full_adder fa0 (.a(a[0]), .b(b[0]), .cin(1'b0), .sum(sum[0]), .cout(c0));
    full_adder fa1 (.a(a[1]), .b(b[1]), .cin(c0),   .sum(sum[1]), .cout(cout));
endmodule
```

함수 호출은 실행이 끝나면 사라지지만, 모듈 인스턴스는 **칩 위에 실제로 존재하는 회로 블록**입니다 (`fa0`, `fa1` 두 개의 가산기가 물리적으로 생성됨).

---

### 4.8 시스템 태스크와 전처리기 (테스트벤치 예제)

**C 언어**

```c
#include <stdio.h>
#define N 10

int main(void) {
    for (int i = 0; i < N; i++)
        printf("i = %d\n", i);
    return 0;
}
```

**Verilog HDL** — 시뮬레이션 전용 `$` 태스크와 백틱(`` ` ``) 지시어를 사용합니다.

```verilog
`timescale 1ns/1ps                  // 시간 단위 / 정밀도
`define N 10

module tb;
    reg clk = 0;
    reg [3:0] cnt = 0;

    always #5 clk = ~clk;           // 10ns 주기 클럭 생성

    always @(posedge clk) cnt <= cnt + 1;

    initial begin
        $monitor("t=%0t cnt=%0d", $time, cnt);   // 값이 바뀔 때마다 출력
        #(`N * 10);                               // 시간 지연 (시뮬레이션 전용)
        $finish;                                  // 시뮬레이션 종료
    end
endmodule
```

---

## 5. 한 줄 정리

> **C는 "순서대로 실행되는 명령"을 쓰는 언어, Verilog는 "동시에 존재하는 회로"를 기술하는 언어이다.**
