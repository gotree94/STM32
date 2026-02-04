# STM32 개발 환경 MISRA-C 정적 분석 도구 가이드

임베디드 개발 환경(STM32CubeIDE, Keil, IAR)에서 MISRA-C 정적 분석을 수행하는 방법을 안내합니다.

---

## 📊 개발 환경별 MISRA-C 지원 현황

### 비교 요약표

| IDE | 내장 MISRA 체커 | 서드파티 통합 | 비용 | 추천도 |
|-----|----------------|--------------|------|--------|
| **IAR Embedded Workbench** | ✅ C-STAT | ✅ | 💰💰💰 | ⭐⭐⭐⭐⭐ |
| **Keil MDK** | ❌ | ✅ PC-lint, Parasoft | 💰💰 | ⭐⭐⭐⭐ |
| **STM32CubeIDE** | ❌ | ⚠️ Cppcheck (수동) | 무료 | ⭐⭐⭐ |

---

## 1. IAR Embedded Workbench + C-STAT

### 개요

IAR은 **C-STAT**이라는 정적 분석 도구를 Add-on으로 제공하며, MISRA 지원이 가장 완벽합니다.

### 지원 표준

| 표준 | 지원 |
|------|------|
| MISRA C:2004 | ✅ |
| MISRA C:2012 | ✅ |
| MISRA C:2023 | ✅ |
| MISRA C++:2008/2023 | ✅ |
| CERT C/C++ | ✅ |
| CWE | ✅ |

### 특징

- **IDE 완전 통합**: 별도 설정 없이 바로 사용
- **TÜV SÜD 인증**: ISO 26262, IEC 61508 인증 프로젝트에 적합
- **SARIF 출력**: CI/CD 파이프라인 통합 용이
- **약 250개 검사 → 600+ 규칙 매핑**

### 사용 방법

1. **Project → Options → C-STAT** 선택
2. **Enable C-STAT analysis** 체크
3. **Rules → MISRA C:2012** 선택
4. **Analyze → Analyze Project** 실행

```
[IAR 메뉴]
Project → Options → C-STAT
├── Enable C-STAT analysis ☑
├── Rules
│   ├── MISRA C:2012 ☑
│   ├── MISRA C:2004 ☐
│   └── CERT C ☐
└── Output format: SARIF
```

### 비용

- C-STAT Add-on 별도 라이선스 필요
- 기능안전(Functional Safety) 에디션: 추가 비용

---

## 2. Keil MDK + PC-lint Plus

### 개요

Keil MDK는 내장 MISRA 체커가 없지만, **PC-lint Plus** 등 서드파티 도구와 잘 통합됩니다.

### 통합 가능 도구

| 도구 | MISRA C:2023 | 특징 |
|------|-------------|------|
| PC-lint Plus | ✅ | Keil 공식 지원 |
| Parasoft C/C++test | ✅ | 프로젝트 직접 Import |
| PVS-Studio | ✅ | ARM Compiler 5/6 지원 |
| LDRA | ✅ | 인증 지원 |

### PC-lint Plus 설정 방법

1. **PC-lint Plus 설치** (유료 라이선스)

2. **Keil에서 External Tool 등록**:
   ```
   [Tools → Customize Tools Menu]
   
   Command: C:\lint\pclp64.exe
   Arguments: -i"C:\lint\lnt" std.lnt co-keil.lnt @project.lnt %F
   ```

3. **MISRA 설정 파일 (au-misra3.lnt)**:
   ```
   // MISRA C:2012 활성화
   +e9000-9999    // MISRA 메시지 활성화
   -append(9020,[MISRA 2012 Rule 1.3, required])
   ```

4. **빌드 후 Lint 실행**: `Project → Lint`

### 비용

- Keil MDK: 유료 (에디션별 상이)
- PC-lint Plus: 별도 유료 라이선스

---

## 3. STM32CubeIDE + Cppcheck (무료 솔루션) ⭐

### 개요

STM32CubeIDE는 공식 MISRA 지원이 없지만, **무료 오픈소스 도구인 Cppcheck**를 연동하여 MISRA 검사가 가능합니다.

### 장점

- ✅ 완전 무료
- ✅ 오픈소스
- ✅ CI/CD 통합 용이
- ✅ 크로스 플랫폼 (Windows, Linux, macOS)

### 단점

- ⚠️ 수동 설정 필요
- ⚠️ MISRA 규칙 커버리지 제한적 (약 60-70%)
- ⚠️ 규칙 텍스트 파일 별도 준비 필요

---

## 🛠️ STM32CubeIDE + Cppcheck 설정 가이드

### Step 1: Cppcheck 설치

#### Windows

```powershell
# Chocolatey 사용
choco install cppcheck

# 또는 공식 설치 프로그램 다운로드
# https://github.com/danmar/cppcheck/releases
# 설치 경로 예: C:\Program Files\Cppcheck
```

#### Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install cppcheck

# 버전 확인
cppcheck --version
```

#### macOS

```bash
brew install cppcheck
```

### Step 2: MISRA 규칙 텍스트 파일 생성

Cppcheck MISRA addon은 규칙 텍스트 파일이 필요합니다. 공식 MISRA 문서는 유료이므로 직접 작성해야 합니다.

**Windows**: `C:\Users\<사용자>\misra_rules.txt`  
**Linux/macOS**: `~/.config/cppcheck/misra_rules.txt`

```
Appendix A Summary of guidelines
Rule 1.1 Required
The program shall contain no violations of the standard C syntax and constraints, and shall not exceed the implementation's translation limits.

Rule 1.2 Advisory
Language extensions should not be used.

Rule 1.3 Required
There shall be no occurrence of undefined or critical unspecified behaviour.

Rule 2.1 Required
A project shall not contain unreachable code.

Rule 2.2 Required
There shall be no dead code.

Rule 2.3 Advisory
A project should not contain unused type declarations.

Rule 2.4 Advisory
A project should not contain unused tag declarations.

Rule 2.5 Advisory
A project should not contain unused macro declarations.

Rule 2.6 Advisory
A function should not contain unused label declarations.

Rule 2.7 Advisory
There should be no unused parameters in functions.

Rule 8.4 Required
A compatible declaration shall be visible when an object or function with external linkage is defined.

Rule 10.1 Required
Operands shall not be of an inappropriate essential type.

Rule 10.3 Required
The value of an expression shall not be assigned to an object with a narrower essential type or of a different essential type category.

Rule 10.4 Required
Both operands of an operator in which the usual arithmetic conversions are performed shall have the same essential type category.

Rule 11.3 Required
A cast shall not be performed between a pointer to object type and a pointer to a different object type.

Rule 14.3 Required
Controlling expressions shall not be invariant.

Rule 15.6 Required
The body of an iteration-statement or a selection-statement shall be a compound-statement.

Rule 16.4 Required
Every switch statement shall have a default label.

Rule 17.2 Required
Functions shall not call themselves, either directly or indirectly.

Rule 17.7 Required
The value returned by a function having non-void return type shall be used.

Rule 18.1 Required
A pointer resulting from arithmetic on a pointer operand shall address an element of the same array as that pointer operand.

Rule 20.7 Required
Expressions resulting from the expansion of macro parameters shall be enclosed in parentheses.

Rule 21.3 Required
The memory allocation and deallocation functions of <stdlib.h> shall not be used.

Rule 21.6 Required
The Standard Library input/output functions shall not be used.
```

### Step 3: STM32CubeIDE External Tool 설정

#### 3.1 External Tools Configuration 열기

```
메뉴: Run → External Tools → External Tools Configurations...
```

#### 3.2 새 Program 생성

**[New Configuration]** 클릭 후 다음 설정:

##### Windows 설정

| 항목 | 값 |
|------|-----|
| **Name** | MISRA-C Check (Cppcheck) |
| **Location** | `C:\Program Files\Cppcheck\cppcheck.exe` |
| **Working Directory** | `${project_loc}` |
| **Arguments** | (아래 참조) |

**Arguments (Windows)**:
```
--addon=misra
--misra-config="C:\Users\사용자\misra_rules.txt"
--std=c99
--enable=all
-I"${project_loc}\Core\Inc"
-I"${project_loc}\Drivers\STM32F1xx_HAL_Driver\Inc"
-I"${project_loc}\Drivers\CMSIS\Include"
-I"${project_loc}\Drivers\CMSIS\Device\ST\STM32F1xx\Include"
-DSTM32F103xB
-DUSE_HAL_DRIVER
"${project_loc}\Core\Src"
```

##### Linux/macOS 설정

| 항목 | 값 |
|------|-----|
| **Name** | MISRA-C Check (Cppcheck) |
| **Location** | `/usr/bin/cppcheck` |
| **Working Directory** | `${project_loc}` |
| **Arguments** | (아래 참조) |

**Arguments (Linux/macOS)**:
```
--addon=misra
--misra-config="${HOME}/.config/cppcheck/misra_rules.txt"
--std=c99
--enable=all
-I"${project_loc}/Core/Inc"
-I"${project_loc}/Drivers/STM32F1xx_HAL_Driver/Inc"
-I"${project_loc}/Drivers/CMSIS/Include"
-I"${project_loc}/Drivers/CMSIS/Device/ST/STM32F1xx/Include"
-DSTM32F103xB
-DUSE_HAL_DRIVER
"${project_loc}/Core/Src"
```

#### 3.3 출력 설정

**Common 탭**:
- ☑ Allocate console (necessary for input)
- ☑ Launch in background

**Build 탭**:
- ☐ Build before launch (체크 해제 권장)

#### 3.4 저장 및 실행

1. **Apply** → **Close**
2. 실행: `Run → External Tools → MISRA-C Check (Cppcheck)`

### Step 4: 결과 확인

Console 창에 다음과 같이 출력됩니다:

```
Checking Core/Src/main.c ...
Core/Src/main.c:45:3: style: misra violation (use --rule-texts=<file> to get rule texts) [misra-c2012-15.6]
Core/Src/main.c:52:5: style: misra violation [misra-c2012-10.3]
Core/Src/main.c:78:1: style: misra violation [misra-c2012-8.4]
```

### Step 5: XML 보고서 생성 (선택)

보다 상세한 분석을 위해 XML 보고서를 생성할 수 있습니다.

**Arguments에 추가**:
```
--xml --xml-version=2 2>"${project_loc}\misra_report.xml"
```

---

## 📁 프로젝트별 설정 파일

### Makefile 통합

프로젝트 루트에 다음 Makefile 타겟을 추가합니다:

```makefile
# Makefile - MISRA 검사 타겟

CPPCHECK = cppcheck
CPPCHECK_FLAGS = --addon=misra --std=c99 --enable=all

# Include 경로 (프로젝트에 맞게 수정)
INCLUDES = -ICore/Inc \
           -IDrivers/STM32F1xx_HAL_Driver/Inc \
           -IDrivers/CMSIS/Include \
           -IDrivers/CMSIS/Device/ST/STM32F1xx/Include

# 매크로 정의 (프로젝트에 맞게 수정)
DEFINES = -DSTM32F103xB -DUSE_HAL_DRIVER

# 소스 디렉토리
SRC_DIR = Core/Src

.PHONY: misra misra-report misra-clean

# 콘솔 출력
misra:
	$(CPPCHECK) $(CPPCHECK_FLAGS) $(INCLUDES) $(DEFINES) $(SRC_DIR)

# XML 보고서 생성
misra-report:
	$(CPPCHECK) $(CPPCHECK_FLAGS) $(INCLUDES) $(DEFINES) \
		--xml --xml-version=2 $(SRC_DIR) 2> misra_report.xml
	@echo "Report generated: misra_report.xml"

# 보고서 삭제
misra-clean:
	rm -f misra_report.xml
```

**사용법**:
```bash
make misra          # 콘솔 출력
make misra-report   # XML 보고서 생성
```

### Suppression 파일

특정 규칙이나 파일을 검사에서 제외하려면 suppression 파일을 사용합니다.

**suppressions.txt**:
```
// HAL 드라이버 (Adopted Code) 제외
*:Drivers/*

// 특정 규칙 제외 (프로젝트 전체)
misra-c2012-21.6

// 특정 파일의 특정 규칙 제외
misra-c2012-2.5:Core/Src/config.c

// 디버그 코드 제외
misra-c2012-21.6:Core/Src/debug.c
```

**사용법**:
```bash
cppcheck --addon=misra --suppressions-list=suppressions.txt Core/Src/
```

---

## 🔄 CI/CD 통합

### GitHub Actions

```yaml
# .github/workflows/misra-check.yml
name: MISRA-C Check

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  misra-analysis:
    runs-on: ubuntu-latest
    
    steps:
    - name: Checkout
      uses: actions/checkout@v4
    
    - name: Install Cppcheck
      run: |
        sudo apt-get update
        sudo apt-get install -y cppcheck
    
    - name: Create MISRA rules file
      run: |
        cat > misra_rules.txt << 'EOF'
        Rule 10.3 Required
        Narrowing conversion
        Rule 15.6 Required
        Compound statement required
        Rule 17.7 Required
        Return value shall be used
        EOF
    
    - name: Run MISRA Check
      run: |
        cppcheck --addon=misra \
                 --misra-config=misra_rules.txt \
                 --std=c99 \
                 --error-exitcode=1 \
                 -ICore/Inc \
                 Core/Src/
    
    - name: Generate Report
      if: always()
      run: |
        cppcheck --addon=misra \
                 --misra-config=misra_rules.txt \
                 --std=c99 \
                 --xml --xml-version=2 \
                 -ICore/Inc \
                 Core/Src/ 2> misra_report.xml
    
    - name: Upload Report
      if: always()
      uses: actions/upload-artifact@v4
      with:
        name: misra-report
        path: misra_report.xml
```

---

## ❓ 문제 해결

### 문제 1: "misra addon not found"

**원인**: Cppcheck 버전이 낮거나 addon이 누락됨

**해결**:
```bash
# 버전 확인 (2.0 이상 필요)
cppcheck --version

# addon 위치 확인
ls /usr/share/cppcheck/addons/misra.py   # Linux
ls "C:\Program Files\Cppcheck\addons\misra.py"  # Windows
```

### 문제 2: "rule-texts file not found"

**원인**: 규칙 텍스트 파일 경로 오류

**해결**:
```bash
# 절대 경로 사용
cppcheck --addon=misra --misra-config=/full/path/to/misra_rules.txt src/
```

### 문제 3: HAL 드라이버에서 대량의 경고

**원인**: ST HAL 라이브러리는 MISRA 100% 준수가 아님

**해결**: Suppression 파일로 Drivers 폴더 제외
```
*:Drivers/*
```

### 문제 4: Include 경로 인식 안됨

**해결**: 프로젝트에 맞게 Include 경로 수정
```bash
# STM32F4 예시
-IDrivers/STM32F4xx_HAL_Driver/Inc
-IDrivers/CMSIS/Device/ST/STM32F4xx/Include
-DSTM32F411xE
```

---

## 📚 참고 자료

### 공식 문서
- [Cppcheck 공식 사이트](https://cppcheck.sourceforge.io/)
- [Cppcheck MISRA Addon](https://github.com/danmar/cppcheck/blob/main/addons/misra.py)
- [MISRA 공식 사이트](https://misra.org.uk/)

### 상용 도구
- [IAR C-STAT](https://www.iar.com/products/c-stat/)
- [PC-lint Plus](https://pclintplus.com/)
- [Polyspace](https://www.mathworks.com/products/polyspace.html)
- [PVS-Studio](https://pvs-studio.com/)

---

## 📝 요약

| 환경 | 권장 도구 | 비용 | 설정 난이도 |
|------|----------|------|------------|
| **기능안전 프로젝트** | IAR + C-STAT | 💰💰💰 | ⭐ 쉬움 |
| **일반 상용 프로젝트** | Keil + PC-lint | 💰💰 | ⭐⭐ 보통 |
| **학습/개인 프로젝트** | CubeIDE + Cppcheck | 무료 | ⭐⭐⭐ 복잡 |
| **CI/CD 환경** | Cppcheck (단독) | 무료 | ⭐⭐ 보통 |

---

**작성일**: 2025년 2월  
**버전**: 1.0
