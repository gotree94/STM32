# C vs Verilog HDL vs VHDL 비교

- **C 언어**: CPU가 **순차적으로 실행**하는 소프트웨어를 작성하는 언어
- **Verilog HDL**: 동시에 동작하는 하드웨어 회로를 **간결한 C 유사 문법**으로 기술하는 언어
- **VHDL**: 같은 하드웨어 기술 언어이지만 **Ada/Pascal 계열의 엄격한 타입 체계**를 가진 언어

세 언어 모두 문법은 비슷해 보이는 부분이 있지만, C는 "실행되는 명령", Verilog와 VHDL은 "존재하는 회로"를 기술한다는 점이 근본적으로 다릅니다.

---

## 1. 언어 특성 한눈에 보기

| 구분 | C | Verilog HDL | VHDL |
|---|---|---|---|
| **용도** | 소프트웨어 (CPU 실행) | 하드웨어 기술·검증 | 하드웨어 기술·검증 |
| **문법 계열** | C | C 유사 | Ada / Pascal 유사 |
| **타입 체계** | 정적 타입, 암시적 변환 허용 | 느슨함 (비트폭이 달라도 연산 가능, 자동 확장/절단) | **매우 엄격** (타입이 다르면 명시적 변환 필요) |
| **대소문자** | 구분함 | 구분함 | **구분하지 않음** |
| **문장 종결 / 블록** | `;`, `{ }` | `;`, `begin ~ end` | `;`, `begin ~ end ~ ;` (`if ~ end if;`, `loop ~ end loop;`) |
| **주석** | `//`, `/* */` | `//`, `/* */` | `--` (VHDL-2008부터 `/* */` 지원) |
| **기본 실행 모델** | 순차 실행 | 병렬 (`always`, `assign`, 인스턴스가 동시에 동작) | 병렬 (`process`, 동시 할당문, 컴포넌트가 동시에 동작) |
| **코드 장황함** | 짧음 | 짧음 | 길고 장황함 |
| **주 사용 분야** | 임베디드 SW, 시스템 | 반도체(ASIC), 북미·아시아 중심 | FPGA, 항공·방산·유럽 중심 |

---

## 2. 문법 요소 비교

| 구분 | C | Verilog HDL | VHDL |
|---|---|---|---|
| **상수** | `#define MAX 100`<br>`const int N = 10;` | `parameter W = 8;`<br>`localparam W = 8;`<br>`8'hFF`, `4'b1010` | `constant W : integer := 8;`<br>`generic` (모듈 파라미터)<br>`x"FF"`, `"1010"` |
| **변수 / 신호** | `int a;` (RAM에 저장) | `wire` (연결선), `reg` (always 안에서 값을 받는 변수) | `signal` (회로의 신호선/기억소자), `variable` (process 내부 임시값) |
| **기본 데이터 타입** | `int`, `char`, `float` | `wire`, `reg`, `integer` | `std_logic`, `std_logic_vector`, `unsigned`, `signed`, `integer`, `boolean` |
| **할당 연산자** | `=` | 조합: `=` (블로킹)<br>순차: `<=` (논블로킹)<br>연속 할당: `assign x = ...` | 신호: `<=`<br>변수: `:=` |
| **순환문** | `for`, `while`, `do~while` | `for`, `while` (합성 시 회로로 펼쳐짐)<br>`generate for` | `for ... loop` (합성 시 회로로 펼쳐짐)<br>`for ... generate` |
| **조건문** | `if ~ else`<br>`switch ~ case` | `if ~ else`<br>`case`, `casez`, `casex`<br>`? :` | `if ~ elsif ~ else ~ end if;`<br>`case ~ is ~ when`<br>`when ~ else`, `with ~ select` |
| **배열** | `int a[10];`<br>`int m[4][4];` | 벡터: `reg [7:0] d;`<br>메모리: `reg [7:0] m [0:255];` | `type mem_t is array (0 to 255) of std_logic_vector(7 downto 0);`<br>`signal m : mem_t;` |
| **포인터 / 동적 메모리** | `*`, `malloc`, `free` | 없음 | 없음 (`access` 타입은 시뮬레이션 전용, 합성 불가) |
| **함수** | `int f(int a) { return a; }` | `function` (지연 없음, 조합 회로용) | `function` (지연 없음, `return` 값 필수) |
| **서브루틴 (시간 소모)** | 해당 없음 | `task` | `procedure` |
| **동시성** | 없음 (스레드는 OS 기능) | 기본 병렬 | 기본 병렬 |
| **모듈 / 계층** | 함수, 파일 분할 | `module` + 인스턴스화 | `entity` (인터페이스) + `architecture` (내부 구현) + `component` 또는 직접 인스턴스화 |
| **라이브러리** | `#include <stdio.h>` | `` `include `` | `library ieee;`<br>`use ieee.std_logic_1164.all;` |
| **시뮬레이션 출력** | `printf` | `$display`, `$monitor`, `$finish` | `report`, `assert`, `std.env.finish` (VHDL-2008), `wait` |
| **시간 제어** | `sleep()` 등 OS 함수 | `#10`, `@(posedge clk)` | `wait for 10 ns;`, `wait until rising_edge(clk);` |
| **전처리기** | `#define`, `#ifdef` | `` `define ``, `` `ifdef ``, `` `timescale `` | 전처리기 없음 (`generic`, `generate`로 대체) |

---

## 3. 예제로 보는 비교

### 3.1 상수와 신호

**C**

```c
#define WIDTH 8
int a = 5;
a = a + 1;                    // 메모리 값을 읽어 1을 더해 다시 저장
```

**Verilog**

```verilog
localparam WIDTH = 8;
wire [WIDTH-1:0] sum;
reg  [WIDTH-1:0] cnt;

assign sum = a + b;           // a, b가 바뀌면 항상 따라 바뀌는 가산기

always @(posedge clk)
    cnt <= cnt + 1;           // 클럭마다 증가하는 카운터
```

**VHDL**

```vhdl
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- architecture 선언부
constant WIDTH : integer := 8;
signal   sum   : unsigned(WIDTH-1 downto 0);
signal   cnt   : unsigned(WIDTH-1 downto 0);

-- architecture 본문
sum <= a + b;                 -- a, b도 unsigned 타입이어야 함

process (clk)
begin
    if rising_edge(clk) then
        cnt <= cnt + 1;
    end if;
end process;
```

> VHDL은 `std_logic_vector`에 `+`를 바로 쓸 수 없어 `unsigned`로 선언하거나 `unsigned(...)`로 변환해야 합니다. 번거롭지만 비트폭·부호 실수를 컴파일 단계에서 잡아줍니다.

---

### 3.2 조합 논리: 멀티플렉서 (MUX)

**C**

```c
if (sel == 0)      y = a;
else if (sel == 1) y = b;
else               y = c;
```

**Verilog**

```verilog
// 2:1 MUX
assign y = sel ? b : a;

// 4:1 MUX
always @* begin
    case (sel)
        2'b00:   y = a;
        2'b01:   y = b;
        2'b10:   y = c;
        default: y = d;
    endcase
end
```

**VHDL**

```vhdl
-- 2:1 MUX
y <= b when sel = '1' else a;

-- 4:1 MUX
with sel select
    y <= a when "00",
         b when "01",
         c when "10",
         d when others;          -- 모든 경우를 반드시 명시해야 함
```

> VHDL의 `case`와 `with ~ select`는 **모든 경우를 빠짐없이** 써야 컴파일됩니다. Verilog는 `default`를 빠뜨려도 컴파일되지만 의도치 않은 **래치**가 생길 수 있습니다.

---

### 3.3 순환문

**C** — CPU가 8번 반복 실행

```c
int ones = 0;
for (int i = 0; i < 8; i++)
    ones += (data >> i) & 1;
```

**Verilog** — 합성 시 8개 항을 더하는 회로로 펼쳐짐

```verilog
reg [3:0] ones;
integer   i;

always @* begin
    ones = 0;
    for (i = 0; i < 8; i = i + 1)
        ones = ones + data[i];
end

// 회로 복제 (generate)
genvar k;
generate
    for (k = 0; k < 8; k = k + 1) begin : gen_inv
        assign y[k] = ~x[k];
    end
endgenerate
```

**VHDL** — 같은 방식으로 회로로 펼쳐짐 (`data`: `std_logic_vector(7 downto 0)`, `ones`: `unsigned(3 downto 0)`)

```vhdl
process (data)
    variable v : unsigned(3 downto 0);
begin
    v := (others => '0');
    for i in 0 to 7 loop
        if data(i) = '1' then
            v := v + 1;
        end if;
    end loop;
    ones <= v;
end process;

-- 회로 복제 (generate)
gen_inv : for k in 0 to 7 generate
    y(k) <= not x(k);
end generate;
```

---

### 3.4 배열과 메모리

**C**

```c
unsigned char mem[256];
mem[addr] = data;
out = mem[addr];
```

**Verilog**

```verilog
reg [7:0] mem [0:255];

always @(posedge clk)
    if (we) mem[addr] <= din;

assign dout = mem[addr];
```

**VHDL**

```vhdl
type mem_t is array (0 to 255) of std_logic_vector(7 downto 0);
signal mem : mem_t;

process (clk)
begin
    if rising_edge(clk) then
        if we = '1' then
            mem(to_integer(unsigned(addr))) <= din;
        end if;
    end if;
end process;

dout <= mem(to_integer(unsigned(addr)));
```

> VHDL은 배열 인덱스로 `integer`만 쓸 수 있어 `to_integer(unsigned(addr))` 변환이 필요합니다. 배열은 괄호 `( )`로 접근합니다.

---

### 3.5 함수와 서브루틴

**C**

```c
int max(int a, int b) { return (a > b) ? a : b; }
```

**Verilog**

```verilog
function [7:0] max;
    input [7:0] a, b;
    begin
        max = (a > b) ? a : b;
    end
endfunction

task send_byte;                   // 시간 소모 가능 (테스트벤치용)
    input [7:0] d;
    begin
        din = d;
        @(posedge clk);
        din = 8'h00;
    end
endtask
```

**VHDL**

```vhdl
function max_u (a, b : unsigned(7 downto 0)) return unsigned is
begin
    if a > b then
        return a;
    else
        return b;
    end if;
end function;

-- 테스트벤치용 procedure (시간 소모 가능)
procedure send_byte (
    signal   din : out std_logic_vector(7 downto 0);
    constant d   : in  std_logic_vector(7 downto 0)) is
begin
    din <= d;
    wait until rising_edge(clk);
    din <= x"00";
end procedure;
```

---

### 3.6 동시성과 할당 규칙

**C** — 위에서 아래로 순서대로 실행

```c
a = 1;
b = a;      // b = 1
c = b;      // c = 1
```

**Verilog** — 모든 문장이 동시에 존재

```verilog
assign b = a;
assign c = b;                    // 순서와 무관하게 연쇄 동작

// 3단 시프트 레지스터: 논블로킹(<=)
always @(posedge clk) begin
    q1 <= d;
    q2 <= q1;
    q3 <= q2;
end
```

**VHDL** — 모든 동시 할당문과 process가 동시에 동작

```vhdl
b <= a;
c <= b;                          -- 순서와 무관하게 연쇄 동작

-- 3단 시프트 레지스터: 신호 할당(<=)
process (clk)
begin
    if rising_edge(clk) then
        q1 <= d;
        q2 <= q1;
        q3 <= q2;
    end if;
end process;
```

| 개념 | Verilog | VHDL | 갱신 시점 |
|---|---|---|---|
| 순차 회로의 신호 갱신 | `<=` (논블로킹) | `signal` + `<=` | 이벤트 처리가 끝난 뒤 한꺼번에 갱신 |
| 즉시 갱신되는 임시값 | `=` (블로킹) | `variable` + `:=` | 그 문장을 실행하는 즉시 갱신 |

> 위 시프트 레지스터를 블로킹(`=`)으로 쓰거나 `signal` 대신 `variable`을 쓰면 `d`가 한 클럭에 `q3`까지 전달되는 전혀 다른 회로가 됩니다.

---

### 3.7 클럭 동기 회로와 비동기 리셋

**C** — 함수를 호출할 때마다 실행

```c
void tick(void) {
    if (rst) cnt = 0;
    else     cnt++;
}
```

**Verilog**

```verilog
always @(posedge clk or negedge rst_n) begin
    if (!rst_n) cnt <= 4'd0;
    else        cnt <= cnt + 1;
end
```

**VHDL**

```vhdl
process (clk, rst_n)
begin
    if rst_n = '0' then
        cnt <= (others => '0');
    elsif rising_edge(clk) then
        cnt <= cnt + 1;
    end if;
end process;
```

---

### 3.8 모듈 계층 구조 (전가산기로 2비트 가산기 만들기)

**C**

```c
int full_add(int a, int b, int cin) { return a + b + cin; }
```

**Verilog**

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

**VHDL** — `entity`(포트 선언)와 `architecture`(내부 구현)가 분리됨

```vhdl
library ieee;
use ieee.std_logic_1164.all;

entity full_adder is
    port (
        a, b, cin : in  std_logic;
        sum, cout : out std_logic
    );
end entity;

architecture rtl of full_adder is
begin
    sum  <= a xor b xor cin;
    cout <= (a and b) or (cin and (a xor b));
end architecture;

---------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;

entity adder2 is
    port (
        a, b : in  std_logic_vector(1 downto 0);
        sum  : out std_logic_vector(1 downto 0);
        cout : out std_logic
    );
end entity;

architecture rtl of adder2 is
    signal c0 : std_logic;
begin
    fa0 : entity work.full_adder
        port map (a => a(0), b => b(0), cin => '0', sum => sum(0), cout => c0);
    fa1 : entity work.full_adder
        port map (a => a(1), b => b(1), cin => c0,  sum => sum(1), cout => cout);
end architecture;
```

---

### 3.9 테스트벤치 (시뮬레이션)

**C**

```c
#include <stdio.h>
#define N 10

int main(void) {
    for (int i = 0; i < N; i++)
        printf("i = %d\n", i);
    return 0;
}
```

**Verilog**

```verilog
`timescale 1ns/1ps

module tb;
    reg clk = 0;
    reg [3:0] cnt = 0;

    always #5 clk = ~clk;                       // 10ns 주기 클럭

    always @(posedge clk) cnt <= cnt + 1;

    initial begin
        $monitor("t=%0t cnt=%0d", $time, cnt);  // 값이 바뀔 때마다 출력
        #100;
        $finish;
    end
endmodule
```

**VHDL** (VHDL-2008 기준)

```vhdl
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity tb is
end entity;

architecture sim of tb is
    signal clk : std_logic := '0';
    signal cnt : unsigned(3 downto 0) := (others => '0');
begin
    clk <= not clk after 5 ns;                  -- 10ns 주기 클럭

    process (clk)
    begin
        if rising_edge(clk) then
            cnt <= cnt + 1;
        end if;
    end process;

    process (cnt)                               -- cnt가 바뀔 때마다 출력
    begin
        report "cnt = " & integer'image(to_integer(cnt));
    end process;

    process
    begin
        wait for 100 ns;
        std.env.finish;                         -- 시뮬레이션 종료 (VHDL-2008)
        wait;
    end process;
end architecture;
```

---

## 4. 핵심 차이점 요약

### 4.1 소프트웨어(C) vs 하드웨어 기술 언어(Verilog, VHDL)
- C의 변수는 RAM의 저장 공간이고, 반복문·조건문은 CPU의 실행 흐름을 제어합니다.
- Verilog와 VHDL의 신호는 **실제 배선과 기억 소자**이고, 반복문·조건문은 **회로 구조(복제, MUX 등)** 를 만드는 수단입니다.
- 포인터와 동적 메모리는 하드웨어에 존재할 수 없으므로 두 HDL 모두 합성 가능한 범위에서는 지원하지 않습니다.

### 4.2 Verilog vs VHDL
| 항목 | Verilog | VHDL |
|---|---|---|
| **철학** | 간결·유연, 빠르게 작성 | 엄격·명시적, 실수를 컴파일 단계에서 차단 |
| **타입 변환** | 거의 자동 (비트폭 불일치도 경고 수준) | 명시적 변환 필수 (`to_integer`, `unsigned()` 등) |
| **인터페이스와 구현** | `module` 하나에 통합 | `entity`(인터페이스)와 `architecture`(구현)를 분리, 한 entity에 여러 architecture 가능 |
| **변수와 신호 구분** | `=`(블로킹), `<=`(논블로킹)으로 구분 | `variable`(`:=`), `signal`(`<=`)로 **선언 단계에서** 구분 |
| **래치 위험** | `case`/`if`에서 경우를 빠뜨리면 의도치 않은 래치 | `case`는 모든 경우 명시 필수, `if`는 여전히 래치 가능 |
| **코드량** | 적음 | 많음 |
| **검증 기능** | `$` 시스템 태스크 (SystemVerilog로 확장) | `report`, `assert`, 라이브러리 (VHDL-2008 확장) |
| **주 사용처** | ASIC·반도체 업계, 북미·아시아 | FPGA, 항공·방산·유럽 |

### 4.3 자주 하는 실수 비교
| 실수 | 증상 | 예방 |
|---|---|---|
| 순차 회로에서 블로킹(`=`) 사용 (Verilog) | 시뮬레이션과 합성 결과 불일치, 한 클럭에 값이 끝까지 전달됨 | 클럭 동기 회로는 `<=` |
| `case`/`if`에서 경우 누락 | 의도치 않은 **래치** 생성 | `default` / `others` / `else` 항상 작성 |
| `signal` 대신 `variable` 사용 (VHDL) | 레지스터가 의도와 다른 회로로 합성 | 클럭 동기 상태는 `signal` |
| 비트폭 불일치 | Verilog: 조용히 잘림, VHDL: 컴파일 에러 | 폭을 명시하고 경고를 확인 |

---

## 5. 한 줄 정리

> **C는 "순서대로 실행되는 명령"을, Verilog는 "간결하게 쓰는 동시 회로"를, VHDL은 "엄격하게 검증되는 동시 회로"를 기술한다.**
