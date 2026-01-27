# STM32 펌웨어 개발을 위한 Git & GitHub 협업 가이드

## 목차
1. [Git 설치 및 초기 설정](#1-git-설치-및-초기-설정)
2. [GitHub 계정 및 저장소 생성](#2-github-계정-및-저장소-생성)
3. [STM32CubeIDE 프로젝트 초기화](#3-stm32cubeide-프로젝트-초기화)
4. [시나리오 1: 일방향 협업 (Push/Pull)](#4-시나리오-1-일방향-협업)
5. [Collaborator 등록 및 양방향 Push](#5-collaborator-등록-및-양방향-push)
6. [시나리오 2: 양방향 협업 (Branch 활용)](#6-시나리오-2-양방향-협업)
7. [충돌 해결 방법](#7-충돌-해결-방법)
8. [유용한 명령어 모음](#8-유용한-명령어-모음)

---

## 1. Git 설치 및 초기 설정

### 1.1 Git 설치

**Windows:**
1. https://git-scm.com/download/win 접속
2. "64-bit Git for Windows Setup" 다운로드
3. 설치 시 대부분 기본 옵션 유지, 아래 항목만 확인:
   - "Use Visual Studio Code as Git's default editor" 선택 (또는 선호하는 에디터)
   - "Git from the command line and also from 3rd-party software" 선택
   - "Checkout Windows-style, commit Unix-style line endings" 선택

### 1.2 설치 확인

Git Bash 또는 명령 프롬프트에서:
```bash
git --version
```
출력 예: `git version 2.43.0.windows.1`

### 1.3 사용자 정보 설정 (필수!)

```bash
# 이름 설정 (GitHub 계정명과 동일하게 권장)
git config --global user.name "홍길동"

# 이메일 설정 (GitHub 가입 이메일과 동일하게)
git config --global user.email "gildong@example.com"

# 설정 확인
git config --list
```

### 1.4 기본 에디터 설정 (선택)

```bash
# VS Code로 설정
git config --global core.editor "code --wait"
```

---

## 2. GitHub 계정 및 저장소 생성

### 2.1 GitHub 계정 생성
1. https://github.com 접속
2. "Sign up" 클릭하여 계정 생성

### 2.2 SSH 키 설정 (권장 - 매번 비밀번호 입력 불필요)

```bash
# SSH 키 생성
ssh-keygen -t ed25519 -C "your_email@example.com"
# Enter 3번 눌러서 기본값으로 생성

# 생성된 공개키 복사
cat ~/.ssh/id_ed25519.pub
```

**GitHub에 SSH 키 등록:**
1. GitHub → 우측 상단 프로필 → Settings
2. 좌측 메뉴 "SSH and GPG keys" 클릭
3. "New SSH key" 클릭
4. Title: "내 PC" (식별용)
5. Key: 위에서 복사한 공개키 붙여넣기
6. "Add SSH key" 클릭

**연결 테스트:**
```bash
ssh -T git@github.com
```
"Hi username! You've successfully authenticated..." 메시지 확인

### 2.3 새 저장소(Repository) 생성

1. GitHub에서 "+" → "New repository" 클릭
2. 설정:
   - Repository name: `STM32F103_Project`
   - Description: "STM32F103 펌웨어 협업 프로젝트"
   - Public 또는 Private 선택
   - "Add a README file" 체크
   - "Add .gitignore" 체크 → 검색창에서 "C" 선택
3. "Create repository" 클릭

---

## 3. STM32CubeIDE 프로젝트 초기화

### 3.1 .gitignore 파일 설정

STM32CubeIDE 프로젝트용 `.gitignore` 파일을 생성합니다.
저장소 루트에 `.gitignore` 파일 생성 (또는 수정):

```gitignore
# STM32CubeIDE 프로젝트용 .gitignore

# 빌드 결과물
/Debug/
/Release/
*.o
*.d
*.elf
*.bin
*.hex
*.map
*.list

# IDE 설정 파일 (선택적 - 팀 간 설정 공유 시 제외 가능)
.settings/
*.launch

# 백업 파일
*~
*.bak

# OS 관련
.DS_Store
Thumbs.db

# 임시 파일
*.tmp
*.temp
```

### 3.2 프로젝트 구조 예시

```
STM32F103_Project/
├── .gitignore
├── README.md
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── stm32f1xx_hal_conf.h
│   │   └── stm32f1xx_it.h
│   └── Src/
│       ├── main.c
│       ├── stm32f1xx_hal_msp.c
│       ├── stm32f1xx_it.c
│       └── system_stm32f1xx.c
├── Drivers/
│   ├── CMSIS/
│   └── STM32F1xx_HAL_Driver/
├── .project
├── .cproject
└── STM32F103_Project.ioc
```

### 3.3 개발자 A: 초기 프로젝트 업로드

```bash
# 1. 작업할 폴더로 이동
cd /c/Users/사용자명/STM32CubeIDE/workspace_1.x

# 2. GitHub 저장소 클론
git clone git@github.com:사용자명/STM32F103_Project.git

# 3. STM32CubeIDE에서 프로젝트 생성
#    - File → New → STM32 Project
#    - 위치를 클론한 폴더 내로 지정

# 4. 프로젝트 폴더로 이동
cd STM32F103_Project

# 5. 파일 상태 확인
git status

# 6. 모든 파일 스테이징
git add .

# 7. 커밋 (변경사항 저장)
git commit -m "Initial STM32F103 project setup"

# 8. GitHub에 업로드
git push origin main
```

### 3.4 개발자 B: 프로젝트 받기

```bash
# 1. 작업할 폴더로 이동
cd /c/Users/사용자명/STM32CubeIDE/workspace_1.x

# 2. 저장소 클론
git clone git@github.com:개발자A사용자명/STM32F103_Project.git

# 3. STM32CubeIDE에서 Import
#    - File → Import → Existing Projects into Workspace
#    - 클론한 폴더 선택
```

---

## 4. 시나리오 1: 일방향 협업

**상황:** 개발자 A가 수정하고 Push, 개발자 B는 Pull만 수행

```
[개발자 A] ──push──→ [GitHub] ──pull──→ [개발자 B]
                        ↑
                    (중앙 저장소)
```

### 4.1 개발자 A의 작업 흐름

```bash
# === 작업 시작 전 항상 최신 상태 확인 ===
git pull origin main

# === 코드 수정 (예: main.c에서 LED 깜빡임 주기 변경) ===
# STM32CubeIDE에서 코드 수정 후 저장

# === 변경 사항 확인 ===
git status

# 출력 예시:
# modified:   Core/Src/main.c

# === 변경 내용 상세 확인 ===
git diff Core/Src/main.c

# === 스테이징 (커밋 준비) ===
git add Core/Src/main.c
# 또는 모든 변경 파일: git add .

# === 커밋 (로컬 저장) ===
git commit -m "LED 깜빡임 주기 500ms에서 1000ms로 변경"

# === Push (GitHub에 업로드) ===
git push origin main
```

### 4.2 개발자 B의 작업 흐름

```bash
# === 최신 코드 받기 ===
git pull origin main

# 출력 예시:
# Updating a1b2c3d..e4f5g6h
# Fast-forward
#  Core/Src/main.c | 2 +-
#  1 file changed, 1 insertion(+), 1 deletion(-)

# === 변경 이력 확인 ===
git log --oneline -5

# 출력 예시:
# e4f5g6h LED 깜빡임 주기 500ms에서 1000ms로 변경
# a1b2c3d Initial STM32F103 project setup
```

### 4.3 실습 예제: LED 제어 코드

**개발자 A가 작성한 초기 main.c 일부:**
```c
/* USER CODE BEGIN 3 */
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    HAL_Delay(500);  // 500ms 딜레이
  }
/* USER CODE END 3 */
```

**개발자 A가 수정 후:**
```c
/* USER CODE BEGIN 3 */
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    HAL_Delay(1000);  // 1000ms로 변경
  }
/* USER CODE END 3 */
```

**커밋 메시지 작성 팁:**
- 무엇을 왜 변경했는지 명확하게
- 예: "LED 깜빡임 주기 1초로 변경 - 시인성 향상을 위해"

---

## 5. Collaborator 등록 및 양방향 Push

**상황:** 4절에서는 개발자 A만 Push하고 개발자 B는 Pull만 했습니다. 이제 개발자 B도 Push할 수 있도록 권한을 부여합니다.

```
[이전: 일방향]
개발자 A ──push──→ GitHub ──pull──→ 개발자 B

[이후: 양방향]
개발자 A ←──push/pull──→ GitHub ←──push/pull──→ 개발자 B
```

### 5.1 왜 Collaborator 등록이 필요한가?

GitHub 저장소는 기본적으로 **소유자(Owner)만 Push 가능**합니다.

| 권한 | 소유자 (개발자 A) | 일반 사용자 (개발자 B) |
|------|------------------|----------------------|
| Clone (복제) | ✅ | ✅ |
| Pull (받기) | ✅ | ✅ |
| Push (올리기) | ✅ | ❌ → Collaborator 등록 필요! |

### 5.2 개발자 A: Collaborator 초대하기

**GitHub 웹에서:**

1. 저장소 페이지 접속 (`STM32F103_Project`)
2. 상단 **Settings** 탭 클릭
3. 좌측 메뉴에서 **Collaborators** 클릭 (또는 "Collaborators and teams")
4. **Add people** 버튼 클릭
5. 개발자 B의 GitHub 사용자명 또는 이메일 입력
6. 검색 결과에서 해당 사용자 선택
7. **Add [사용자명] to this repository** 클릭

```
┌─────────────────────────────────────────┐
│  Settings > Collaborators               │
├─────────────────────────────────────────┤
│                                         │
│  Manage access                          │
│  ┌─────────────────────────────────┐   │
│  │ 🔍 Search by username or email  │   │
│  └─────────────────────────────────┘   │
│                                         │
│  [ Add people ]                         │
│                                         │
└─────────────────────────────────────────┘
```

### 5.3 개발자 B: 초대 수락하기

**방법 1: 이메일 확인**
1. GitHub 가입 이메일로 초대 메일 수신
2. **"View invitation"** 버튼 클릭
3. **"Accept invitation"** 클릭

**방법 2: GitHub 알림 확인**
1. GitHub 접속 → 우측 상단 🔔 알림 아이콘 클릭
2. 초대 알림 확인
3. **"Accept"** 클릭

**방법 3: 직접 URL 접속**
```
https://github.com/개발자A사용자명/STM32F103_Project/invitations
```

### 5.4 개발자 B: Push 테스트

이제 개발자 B도 Push할 수 있습니다!

```bash
# === 현재 프로젝트 폴더로 이동 ===
cd /c/Users/사용자명/STM32CubeIDE/workspace_1.x/STM32F103_Project

# === 최신 코드 받기 ===
git pull origin main

# === 코드 수정 (예: LED 딜레이 변경) ===
# STM32CubeIDE에서 main.c 수정 후 저장

# === 변경 사항 확인 ===
git status

# === 커밋 ===
git add .
git commit -m "LED 딜레이 200ms로 변경 - 빠른 점멸 테스트"

# === Push (이제 가능!) ===
git push origin main
```

**성공 시 출력:**
```
Enumerating objects: 5, done.
Counting objects: 100% (5/5), done.
Delta compression using up to 8 threads
Compressing objects: 100% (3/3), done.
Writing objects: 100% (3/3), 328 bytes | 328.00 KiB/s, done.
Total 3 (delta 2), reused 0 (delta 0)
To github.com:개발자A사용자명/STM32F103_Project.git
   e4f5g6h..a1b2c3d  main -> main
```

### 5.5 Push 실패 시 해결 방법

**오류 1: Permission denied**
```
ERROR: Permission to 사용자명/STM32F103_Project.git denied to 개발자B.
fatal: Could not read from remote repository.
```
→ Collaborator 초대가 아직 수락되지 않았거나, 초대가 안 됨. 5.2~5.3 단계 재확인

**오류 2: Updates were rejected**
```
! [rejected]        main -> main (fetch first)
error: failed to push some refs to 'github.com:...'
hint: Updates were rejected because the remote contains work that you do not have locally.
```
→ 다른 사람이 먼저 Push함. Pull 먼저 받고 다시 Push:
```bash
git pull origin main
# 충돌 있으면 해결 후
git push origin main
```

### 5.6 협업 흐름 정리

Collaborator 등록 후의 양방향 협업 흐름:

```bash
# === 개발자 A 작업 ===
git pull origin main          # 1. 최신 코드 받기
# 코드 수정...
git add .
git commit -m "기능 A 추가"
git push origin main          # 2. Push

# === 개발자 B 작업 ===
git pull origin main          # 3. A의 변경사항 받기
# 코드 수정...
git add .  
git commit -m "기능 B 추가"
git push origin main          # 4. Push

# === 개발자 A 다시 ===
git pull origin main          # 5. B의 변경사항 받기
# ... 반복
```

### 5.7 Collaborator 권한 종류 (참고)

GitHub에서 설정 가능한 권한 레벨:

| 권한 | Pull | Push | 설정 변경 | 삭제 |
|------|------|------|----------|------|
| Read | ✅ | ❌ | ❌ | ❌ |
| Triage | ✅ | ❌ | ❌ | ❌ |
| Write | ✅ | ✅ | ❌ | ❌ |
| Maintain | ✅ | ✅ | 일부 | ❌ |
| Admin | ✅ | ✅ | ✅ | ✅ |

일반적인 Collaborator 초대는 **Write** 권한으로, Push가 가능합니다.

---

## 6. 시나리오 2: 양방향 협업

> **참고:** 이 섹션은 5절에서 Collaborator 등록이 완료된 상태에서 진행합니다.
> 동일한 `STM32F103_Project`에서 계속 실습합니다.

**상황:** 두 개발자가 각자 수정하고, 서로의 변경사항을 확인하며 병합

```
[개발자 A] ←──→ [GitHub] ←──→ [개발자 B]
    │              │              │
 feature/        main          feature/
 sensor                         motor
```

### 6.1 브랜치 전략

```
main (또는 master)
  │
  ├── feature/sensor-init     (개발자 A: 센서 초기화)
  │
  └── feature/motor-control   (개발자 B: 모터 제어)
```

### 6.2 개발자 A: 새 브랜치에서 작업

```bash
# === 최신 main 받기 ===
git checkout main
git pull origin main

# === 새 브랜치 생성 및 이동 ===
git checkout -b feature/sensor-init
# 또는: git switch -c feature/sensor-init

# 현재 브랜치 확인
git branch
# 출력:
#   main
# * feature/sensor-init

# === 코드 작업 (센서 초기화 코드 추가) ===
# STM32CubeIDE에서 작업...

# === 커밋 ===
git add .
git commit -m "MPU6050 센서 I2C 초기화 코드 추가"

# === 추가 작업 후 커밋 ===
git add .
git commit -m "MPU6050 가속도 데이터 읽기 함수 구현"

# === 원격 저장소에 브랜치 Push ===
git push -u origin feature/sensor-init
# -u 옵션: 이후 git push만 해도 이 브랜치로 push됨
```

### 6.3 개발자 B: 별도 브랜치에서 작업

```bash
# === 최신 main 받기 ===
git checkout main
git pull origin main

# === 새 브랜치 생성 ===
git checkout -b feature/motor-control

# === 코드 작업 (모터 제어 코드) ===
# STM32CubeIDE에서 작업...

# === 커밋 및 Push ===
git add .
git commit -m "DC 모터 PWM 제어 초기화 코드 추가"
git push -u origin feature/motor-control
```

### 6.4 상대방 변경사항 확인하기

```bash
# === 원격 저장소 정보 업데이트 ===
git fetch origin

# === 모든 브랜치 목록 보기 ===
git branch -a
# 출력 예시:
# * feature/motor-control
#   main
#   remotes/origin/feature/motor-control
#   remotes/origin/feature/sensor-init
#   remotes/origin/main

# === 개발자 A의 브랜치 변경사항 확인 (병합 없이) ===
git log origin/feature/sensor-init --oneline
# 출력:
# c3d4e5f MPU6050 가속도 데이터 읽기 함수 구현
# a1b2c3d MPU6050 센서 I2C 초기화 코드 추가

# === 변경된 파일 목록 보기 ===
git diff --stat main origin/feature/sensor-init
# 출력 예시:
#  Core/Inc/mpu6050.h |  25 ++++++++++
#  Core/Src/mpu6050.c | 120 ++++++++++++++++++++++++++++++++
#  Core/Src/main.c    |  15 ++++-
#  3 files changed, 159 insertions(+), 1 deletion(-)

# === 특정 파일의 변경 내용 상세 보기 ===
git diff main origin/feature/sensor-init -- Core/Src/main.c
```

### 6.5 Pull Request (PR)를 통한 코드 리뷰

**GitHub 웹에서:**

1. GitHub 저장소 페이지 접속
2. "Pull requests" 탭 클릭
3. "New pull request" 클릭
4. base: `main` ← compare: `feature/sensor-init` 선택
5. 변경사항 확인 후 "Create pull request" 클릭
6. 제목과 설명 작성:
   ```
   제목: MPU6050 센서 드라이버 추가
   
   설명:
   ## 변경 사항
   - MPU6050 I2C 초기화 코드 추가
   - 가속도/자이로 데이터 읽기 함수 구현
   
   ## 테스트
   - I2C 통신 확인 완료
   - 가속도 값 정상 출력 확인
   ```
7. 개발자 B를 Reviewer로 지정

**개발자 B의 리뷰:**
1. "Pull requests" 탭에서 해당 PR 클릭
2. "Files changed" 탭에서 코드 확인
3. 라인별로 코멘트 추가 가능
4. 리뷰 완료 후 "Approve" 또는 "Request changes"

### 6.6 브랜치 병합 (Merge)

**방법 1: GitHub 웹에서 (권장)**
1. PR 페이지에서 "Merge pull request" 클릭
2. "Confirm merge" 클릭
3. (선택) "Delete branch" 클릭하여 브랜치 정리

**방법 2: 로컬에서**
```bash
# === main 브랜치로 이동 ===
git checkout main

# === 최신 상태 받기 ===
git pull origin main

# === 브랜치 병합 ===
git merge feature/sensor-init

# === 병합 결과 Push ===
git push origin main

# === (선택) 로컬 브랜치 삭제 ===
git branch -d feature/sensor-init
```

### 6.7 병합 후 다른 개발자 동기화

```bash
# 개발자 B가 병합된 main 받기
git checkout main
git pull origin main

# 자신의 작업 브랜치를 최신 main과 동기화 (선택)
git checkout feature/motor-control
git rebase main
# 또는: git merge main
```

---

## 7. 충돌 해결 방법

### 7.1 충돌 발생 상황

두 개발자가 같은 파일의 같은 부분을 수정했을 때 발생

**예시: 둘 다 main.c의 같은 위치를 수정**

개발자 A (feature/sensor-init):
```c
/* USER CODE BEGIN 2 */
  MPU6050_Init();  // A가 추가
/* USER CODE END 2 */
```

개발자 B (feature/motor-control):
```c
/* USER CODE BEGIN 2 */
  Motor_PWM_Init();  // B가 추가
/* USER CODE END 2 */
```

### 7.2 충돌 해결 과정

```bash
# === 병합 시도 시 충돌 발생 ===
git merge feature/sensor-init

# 출력:
# Auto-merging Core/Src/main.c
# CONFLICT (content): Merge conflict in Core/Src/main.c
# Automatic merge failed; fix conflicts and then commit the result.

# === 충돌 파일 확인 ===
git status
# 출력:
# Unmerged paths:
#   both modified:   Core/Src/main.c
```

### 7.3 충돌 파일 내용

main.c 파일을 열면:
```c
/* USER CODE BEGIN 2 */
<<<<<<< HEAD
  Motor_PWM_Init();  // 현재 브랜치 (B의 코드)
=======
  MPU6050_Init();    // 병합하려는 브랜치 (A의 코드)
>>>>>>> feature/sensor-init
/* USER CODE END 2 */
```

### 7.4 충돌 해결

**수동으로 수정하여 양쪽 코드 모두 포함:**
```c
/* USER CODE BEGIN 2 */
  MPU6050_Init();    // 센서 초기화
  Motor_PWM_Init();  // 모터 초기화
/* USER CODE END 2 */
```

**주의:** `<<<<<<<`, `=======`, `>>>>>>>` 마커를 모두 제거해야 합니다!

### 7.5 충돌 해결 후 커밋

```bash
# === 수정한 파일 스테이징 ===
git add Core/Src/main.c

# === 병합 완료 커밋 ===
git commit -m "Merge feature/sensor-init: 센서와 모터 초기화 코드 통합"

# === Push ===
git push origin main
```

### 7.6 충돌 예방 팁

1. **자주 Pull 하기:** 작업 전/후로 `git pull` 습관화
2. **작은 단위로 커밋:** 큰 변경보다 작은 변경을 자주
3. **담당 영역 분리:** 가능하면 다른 파일 담당
4. **커뮤니케이션:** "나 지금 main.c 수정 중" 공유

---

## 8. 유용한 명령어 모음

### 8.1 기본 명령어

| 명령어 | 설명 |
|--------|------|
| `git status` | 현재 상태 확인 |
| `git add <파일>` | 스테이징 (커밋 준비) |
| `git add .` | 모든 변경 파일 스테이징 |
| `git commit -m "메시지"` | 커밋 (로컬 저장) |
| `git push origin <브랜치>` | 원격 저장소에 업로드 |
| `git pull origin <브랜치>` | 원격 저장소에서 받기 |

### 8.2 브랜치 명령어

| 명령어 | 설명 |
|--------|------|
| `git branch` | 로컬 브랜치 목록 |
| `git branch -a` | 모든 브랜치 목록 (원격 포함) |
| `git checkout <브랜치>` | 브랜치 이동 |
| `git checkout -b <새브랜치>` | 새 브랜치 생성 및 이동 |
| `git branch -d <브랜치>` | 브랜치 삭제 |
| `git merge <브랜치>` | 브랜치 병합 |

### 8.3 확인/비교 명령어

| 명령어 | 설명 |
|--------|------|
| `git log --oneline` | 커밋 이력 간단히 보기 |
| `git log --oneline -10` | 최근 10개 커밋만 |
| `git diff` | 변경 내용 확인 |
| `git diff <브랜치1> <브랜치2>` | 브랜치 간 차이 |
| `git show <커밋해시>` | 특정 커밋 상세 보기 |

### 8.4 되돌리기 명령어

| 명령어 | 설명 |
|--------|------|
| `git checkout -- <파일>` | 파일 변경 취소 (커밋 전) |
| `git reset HEAD <파일>` | 스테이징 취소 |
| `git reset --soft HEAD~1` | 최근 커밋 취소 (변경 유지) |
| `git reset --hard HEAD~1` | 최근 커밋 완전 삭제 ⚠️ |
| `git revert <커밋해시>` | 커밋 되돌리기 (이력 유지) |

### 8.5 자주 쓰는 조합

```bash
# === 매일 작업 시작 루틴 ===
git checkout main
git pull origin main
git checkout -b feature/오늘작업

# === 작업 완료 루틴 ===
git add .
git commit -m "작업 내용 설명"
git push origin feature/오늘작업

# === 급하게 다른 브랜치 확인해야 할 때 ===
git stash              # 현재 작업 임시 저장
git checkout other-branch
# 확인 후...
git checkout 원래브랜치
git stash pop          # 임시 저장 복원

# === 커밋 메시지 수정 (push 전) ===
git commit --amend -m "새로운 메시지"
```

---

## 부록: STM32CubeIDE Git 통합

### A.1 STM32CubeIDE 내장 Git 사용

1. **Git 관점 열기:**
   - Window → Perspective → Open Perspective → Other → Git

2. **저장소 추가:**
   - Git Repositories 뷰에서 우클릭 → Add existing local Git repository

3. **커밋하기:**
   - Project Explorer에서 프로젝트 우클릭
   - Team → Commit
   - 변경 파일 선택 → 메시지 입력 → Commit and Push

4. **Pull하기:**
   - 프로젝트 우클릭 → Team → Pull

### A.2 .project, .cproject 파일 관리

이 파일들은 IDE 설정을 포함하므로:
- **공유하면:** 팀원 간 동일한 빌드 환경 유지
- **공유 안 하면:** 각자 IDE 설정 가능, 충돌 가능성 감소

권장: 처음에 공유하고, 문제 발생 시 .gitignore에 추가

---

## 실습 체크리스트

### ✅ 기본 설정
- [ ] Git 설치 완료
- [ ] 사용자 이름/이메일 설정
- [ ] GitHub 계정 생성
- [ ] SSH 키 등록

### ✅ 시나리오 1 실습
- [ ] 개발자 A: 프로젝트 생성 및 Push
- [ ] 개발자 B: 클론 및 Import
- [ ] 개발자 A: 코드 수정 → 커밋 → Push
- [ ] 개발자 B: Pull로 변경사항 받기

### ✅ Collaborator 등록 실습
- [ ] 개발자 A: Settings에서 Collaborator 초대
- [ ] 개발자 B: 초대 수락
- [ ] 개발자 B: 코드 수정 → 커밋 → Push 성공 확인

### ✅ 시나리오 2 실습
- [ ] 각자 feature 브랜치 생성
- [ ] 각자 코드 수정 → 커밋 → Push
- [ ] Pull Request 생성
- [ ] 코드 리뷰 진행
- [ ] main에 병합
- [ ] 충돌 해결 연습

---

## 문제 해결 FAQ

**Q: push가 거부될 때?**
```bash
# 먼저 pull 받고 다시 push
git pull origin main
git push origin main
```

**Q: 잘못된 브랜치에 커밋했을 때?**
```bash
# 커밋을 다른 브랜치로 옮기기
git checkout correct-branch
git cherry-pick <커밋해시>
git checkout wrong-branch
git reset --hard HEAD~1
```

**Q: 로컬 변경사항을 버리고 원격 상태로 맞추고 싶을 때?**
```bash
git fetch origin
git reset --hard origin/main
```

---

*작성일: 2025년 1월*
*STM32CubeIDE 버전: 1.x 기준*
