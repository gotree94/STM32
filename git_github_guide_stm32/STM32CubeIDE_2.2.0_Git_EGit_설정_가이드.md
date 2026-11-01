# STM32CubeIDE 2.2.0에서 Git(EGit) 사용하기

## 1. EGit 설치 확인 및 설치

STM32CubeIDE는 기본적으로 EGit가 포함되어 있는 경우가 많지만, Git 메뉴가 보이지 않거나 동작하지 않는다면 EGit를 수동으로 설치해야 합니다.

### 1.1 EGit 설치 여부 확인

1. `Window → Show View → Other...` 실행
2. `Git` 폴더가 보이면 이미 EGit가 설치되어 있는 상태입니다.
3. 만약 Git 폴더가 보이지 않는다면 아래 1.2를 진행합니다.

### 1.2 EGit 설치하기 (필요한 경우에만)

1. `Help → Install New Software...` 실행
2. `Work with:` 항목에서 STM32CubeIDE 업데이트 사이트 선택 (기본 설정되어 있는 경우가 많습니다)
   - 또는 `https://download.eclipse.org/releases/[사용중인 이클립스 버전]` 입력
3. 필터에 `EGit` 입력
4. `Collaboration` 아래에서 다음 항목 체크
   - `Git integration for Eclipse (EGit)`
   - `Java implementation of Git (JGit)` (보통 자동 선택됨)
5. `Next` 클릭 후 라이선스 동의 → `Finish`
6. 설치 완료 후 STM32CubeIDE 재시작

> STM32CubeIDE 2.2.0은 EGit 기반으로 Git이 동작합니다.

## 2. Git 관련 뷰 열기

Git 작업을 위해 필요한 뷰를 열어줍니다.

1. `Window → Show View → Other...` 실행
2. `Git` 폴더 확장
3. 다음 뷰 추가
   - **Git Repositories** – 로컬/원격 저장소 관리
   - **Git Staging** – 스테이징 및 커밋 (STM32CubeIDE 2.2.0에서 핵심 뷰)

또는 전체 Git 퍼스펙티브를 열어도 편리합니다.

1. `Window → Perspective → Open Perspective → Other...`
2. `Git` 선택 → `Open`

## 3. Git 저장소 연동하기

### 3.1 이미 `.git` 폴더가 존재하는 경우 (기존 로컬 저장소)

1. `Git Repositories` 뷰 열기
2. 뷰 내부 빈 공간에서 우클릭 → `Add an existing local Git repository...`
3. `.git` 폴더가 있는 상위 디렉토리를 `Directory`에 입력하거나 `Browse...`로 선택
4. `Search` 클릭
5. 검색된 저장소를 체크 → `Add` 클릭

### 3.2 프로젝트를 처음 Git에 연결하는 경우 (새 저장소 생성)

아직 `.git`이 없는 프로젝트라면 이 방법을 사용합니다.

1. `Project Explorer`에서 프로젝트 우클릭 → `Team → Share Project...`
2. `Git` 선택 → `Next`
3. `Use or create repository in parent folder of project` 체크
4. 저장소 위치가 올바른지 확인
5. `Create Repository` 클릭 → `Finish`

### 3.3 원격 저장소(Remote) 추가하기

1. `Git Repositories` 뷰에서 연동된 저장소 우클릭 → `Remotes → Create Remote...`
2. `Remote name` 입력 (일반적으로 `origin`)
3. `Configure push` 선택 후 `OK`
4. `URI`에 원격 저장소 URL 입력 (GitHub, GitLab 등)
5. 인증 정보 입력 후 저장

## 4. 커밋 & 푸시하기 (STM32CubeIDE 2.2.0)

> **주의:** 예전과 달리 `Team → Commit` 클릭 시 단일 커밋 다이얼로그가 아닌 **Git Staging 뷰**가 열립니다.

1. 변경사항이 있는 프로젝트에서 `Project Explorer` 우클릭 → `Team → Commit`
2. 자동으로 `Git Staging` 뷰가 열립니다.
3. 스테이징 작업
   - `Unstaged Changes` 영역에서 커밋할 파일 선택
   - `+` 버튼을 눌러 `Staged Changes` 영역으로 이동 (필요한 파일만 스테이징 권장)
4. 상단 `Commit Message` 입력란에 커밋 메시지 작성
5. 작업 선택
   - `Commit` – 로컬 저장소에만 커밋
   - `Commit and Push...` – 로컬 커밋 후 원격 저장소로 푸시

## 5. Pull / Push / 동기화

| 작업 | 경로 |
|---|---|
| Pull (원격 변경사항 가져오기) | `Project Explorer` 우클릭 → `Team → Pull` |
| Push (원격으로 업로드) | `Project Explorer` 우클릭 → `Team → Push Branch...` 또는 `Commit and Push...` 사용 |
| Fetch (원격 정보만 가져오기) | `Project Explorer` 우클릭 → `Team → Fetch` |
| Synchronize (로컬/원격 차이 확인) | `Project Explorer` 우클릭 → `Team → Synchronize` |

## 6. 유용한 팁

- **빌드 산출물 제외**: `Debug/`, `Release/`, `build/` 등은 `.gitignore`에 추가하여 커밋되지 않도록 설정하는 것이 좋습니다.
- **원격에서 바로 클론하기**: 처음부터 원격 저장소를 클론하려면 `File → Import → Git → Projects from Git → Clone URI` 경로가 가장 직관적입니다. (클론과 프로젝트 임포트를 동시에 진행할 수 있습니다.)
- **작업 흐름의 중심은 Git Staging**: 메뉴(`Team`)도 여전히 사용 가능하지만, STM32CubeIDE 2.2.0에서는 `Git Staging` 뷰를 중심으로 커밋 작업을 하는 것이 일반적입니다.
- **브랜치 관리**: 브랜치 생성/전환/병합은 주로 `Git Repositories` 뷰 또는 `Team → Switch To` 메뉴를 통해 수행합니다.
