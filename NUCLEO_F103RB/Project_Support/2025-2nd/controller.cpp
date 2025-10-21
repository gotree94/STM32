//#define WIN32_LEAN_AND_MEAN
//#define NOMINMAX
//
//// 1) Windows 헤더를 제일 먼저
//#include <windows.h>
//
//// 2) Windows 의존 헤더들 (windows.h 뒤에)
//#include <shellapi.h>    // ShellExecuteW
//#include <winhttp.h>     // WinHTTP
//#include <Xinput.h>      // 게임패드
//
//#pragma comment(lib, "Shell32.lib")
//#pragma comment(lib, "winhttp.lib")
//#pragma comment(lib, "Xinput9_1_0.lib")
//
//// 3) 표준 헤더들
//#include <algorithm>
//#include <thread>
//#include <atomic>
//#include <cstdio>
//#include <cstdint>
//#include <cmath>
//
//// ======= 시리얼(STM32) =======
//#define COM_PORT   L"\\\\.\\COM9"
//#define BAUDRATE   115200
//
//// ======= ESP32-CAM =======
//static const wchar_t* ESP_IP = L"192.168.0.21";
//static const wchar_t* SAVE_DIR = L"C:\\Users\\54\\Desktop\\capture";
//
//#define DEAD_ZONE   8000
//#define MAX_AXIS    32767
//
//std::atomic<bool> g_busy{ false };
//
//// 기본 브라우저로 URL 열기
//static void open_url(const wchar_t* url) {
//    ShellExecuteW(nullptr, L"open", url, nullptr, nullptr, SW_SHOWNORMAL);
//}
//
//// 활성 윈도우에 가상키 보내기 (예: F5, Ctrl+W)
//static void send_virt_key(WORD vk, bool withCtrl) {
//    INPUT in[4] = {};
//    int n = 0;
//    if (withCtrl) { in[n].type = INPUT_KEYBOARD; in[n].ki.wVk = VK_CONTROL; n++; }
//    in[n].type = INPUT_KEYBOARD; in[n].ki.wVk = vk; n++;
//    in[n].type = INPUT_KEYBOARD; in[n].ki.wVk = vk; in[n].ki.dwFlags = KEYEVENTF_KEYUP; n++;
//    if (withCtrl) { in[n].type = INPUT_KEYBOARD; in[n].ki.wVk = VK_CONTROL; in[n].ki.dwFlags = KEYEVENTF_KEYUP; n++; }
//    SendInput(n, in, sizeof(INPUT));
//}
//
//// ---------- 공용 유틸 ----------
//static inline uint8_t map_axis_to_speed(int v) {
//    int a = std::abs(v);
//    if (a < DEAD_ZONE) return 0;
//    if (a > MAX_AXIS) a = MAX_AXIS;
//    return (uint8_t)((a * 255) / MAX_AXIS);
//}
//
//// RS(오른쪽 스틱) → 0..255(중앙=128)
//static inline uint8_t map_axis_to_u8_centered(int v, bool invert = false) {
//    int x = v;
//    if (invert) x = -x;                     // 필요시 상하 반전
//    if (std::abs(x) < DEAD_ZONE) return 128; // 중앙 고정
//    if (x > MAX_AXIS) x = MAX_AXIS;
//    if (x < -MAX_AXIS) x = -MAX_AXIS;
//    // [-32767..32767] -> [-127..127] -> [1..255], 중앙=128
//    int val = (x * 127) / MAX_AXIS;         // -127..127
//    int u8 = 128 + val;                    // 1..255(중앙=128)
//    if (u8 < 1)   u8 = 1;
//    if (u8 > 255) u8 = 255;
//    return (uint8_t)u8;
//}
//
//static void serial_reader(HANDLE h)
//{
//    char line[256]; DWORD br; size_t i = 0;
//    for (;;) {
//        char c;
//        if (!ReadFile(h, &c, 1, &br, nullptr) || br == 0) { Sleep(1); continue; }
//        if (c == '\n' || i >= sizeof(line) - 1) {
//            line[i] = 0;
//            if (i > 0) std::printf("[STM] %s\n", line);   // 예: D,23
//            i = 0;
//        }
//        else if (c != '\r') {
//            line[i++] = c;
//        }
//    }
//}
//
//// ---------- 시리얼 ----------
//// 기존: h = CreateFileW(COM_PORT, GENERIC_WRITE, ...
//// 변경: 읽기/쓰기를 모두 허용
//static bool open_serial(HANDLE& h) {
//    h = CreateFileW(
//        COM_PORT,
//        GENERIC_READ | GENERIC_WRITE,   // ★ 여기!
//        0, nullptr, OPEN_EXISTING, 0, nullptr
//    );
//    if (h == INVALID_HANDLE_VALUE) return false;
//
//    DCB dcb{}; dcb.DCBlength = sizeof(dcb);
//    if (!GetCommState(h, &dcb)) return false;
//    dcb.BaudRate = BAUDRATE; dcb.ByteSize = 8; dcb.Parity = NOPARITY; dcb.StopBits = ONESTOPBIT;
//    if (!SetCommState(h, &dcb)) return false;
//
//    // ★ 읽기 타임아웃도 설정 (읽기 스레드가 부드럽게 동작)
//    COMMTIMEOUTS to{};
//    to.ReadIntervalTimeout = 50;
//    to.ReadTotalTimeoutConstant = 50;
//    to.ReadTotalTimeoutMultiplier = 0;
//    to.WriteTotalTimeoutConstant = 50;
//    to.WriteTotalTimeoutMultiplier = 0;
//    SetCommTimeouts(h, &to);
//    return true;
//}
//
//static void tx_cmd(HANDLE h, char cmd, uint8_t spd) {
//    DWORD bw;
//    if (cmd == ' ') { WriteFile(h, &cmd, 1, &bw, nullptr); std::printf("TX: STOP\n"); }
//    else {
//        WriteFile(h, &cmd, 1, &bw, nullptr);
//        WriteFile(h, &spd, 1, &bw, nullptr);
//        // 주행(w/a/s/d)면 speed, 서보(P/T)면 u8(중앙128)
//        std::printf("TX: %c, val=%u\n", cmd, spd);
//    }
//}
//
//// ---------- XInput ----------
//static int find_active_user_index() {
//    for (int i = 0; i < 4; ++i) {
//        XINPUT_STATE st{};
//        if (XInputGetState(i, &st) == ERROR_SUCCESS) return i;
//    }
//    return -1;
//}
//
//// ---------- 캡처 다운로드(백그라운드) ----------
//static bool http_download_capture(const wchar_t* ip, const wchar_t* savePath) {
//    HINTERNET h = WinHttpOpen(L"pc-cap",
//        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
//        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
//    if (!h) return false;
//
//    HINTERNET c = WinHttpConnect(h, ip, 80, 0);
//    if (!c) { WinHttpCloseHandle(h); return false; }
//
//    HINTERNET r = WinHttpOpenRequest(c, L"GET", L"/capture",
//        nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
//    if (!r) { WinHttpCloseHandle(c); WinHttpCloseHandle(h); return false; }
//
//    BOOL ok = WinHttpSendRequest(r,
//        WINHTTP_NO_ADDITIONAL_HEADERS, 0,
//        WINHTTP_NO_REQUEST_DATA, 0,
//        0, 0) && WinHttpReceiveResponse(r, nullptr);
//
//    HANDLE f = CreateFileW(savePath, GENERIC_WRITE, 0, nullptr,
//        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
//
//    if (!ok || f == INVALID_HANDLE_VALUE) {
//        if (f != INVALID_HANDLE_VALUE) CloseHandle(f);
//        WinHttpCloseHandle(r); WinHttpCloseHandle(c); WinHttpCloseHandle(h);
//        return false;
//    }
//
//    BYTE buf[16 * 1024];
//    DWORD br = 0;
//    for (;;) {
//        DWORD avail = 0;
//        if (!WinHttpQueryDataAvailable(r, &avail) || avail == 0) break;
//
//        while (avail) {
//            DWORD toRead = std::min<DWORD>(avail, sizeof(buf));
//            if (!WinHttpReadData(r, buf, toRead, &br) || br == 0) { avail = 0; break; }
//            DWORD bw = 0; WriteFile(f, buf, br, &bw, nullptr);
//            avail -= br;
//        }
//    }
//
//    CloseHandle(f);
//    WinHttpCloseHandle(r); WinHttpCloseHandle(c); WinHttpCloseHandle(h);
//    return true;
//}
//
//static void make_timestamped_path(wchar_t* out, size_t cap) {
//    CreateDirectoryW(SAVE_DIR, nullptr); // 존재하면 실패해도 OK
//    SYSTEMTIME t; GetLocalTime(&t);
//    swprintf(out, cap, L"%s\\shot_%04u%02u%02u_%02u%02u%02u_%03u.jpg",
//        SAVE_DIR, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
//}
//
//// 간단한 GET /path 호출 (호스트는 IP/호스트명, 포트 80 고정)
//static bool http_get_path(const wchar_t* host, const wchar_t* path)
//{
//    HINTERNET h = WinHttpOpen(L"pc-ctrl",
//        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
//        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
//    if (!h) return false;
//
//    HINTERNET c = WinHttpConnect(h, host, 80, 0);
//    if (!c) { WinHttpCloseHandle(h); return false; }
//
//    HINTERNET r = WinHttpOpenRequest(c, L"GET", path,
//        nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
//    if (!r) { WinHttpCloseHandle(c); WinHttpCloseHandle(h); return false; }
//
//    BOOL ok = WinHttpSendRequest(r,
//        WINHTTP_NO_ADDITIONAL_HEADERS, 0,
//        WINHTTP_NO_REQUEST_DATA, 0,
//        0, 0)
//        && WinHttpReceiveResponse(r, nullptr);
//
//    WinHttpCloseHandle(r);
//    WinHttpCloseHandle(c);
//    WinHttpCloseHandle(h);
//    return ok == TRUE;
//}
//
//int main() {
//    // 시리얼
//    HANDLE hSer;
//    if (!open_serial(hSer)) {
//        std::puts("Serial open failed");
//        return 1;
//    }
//
//    std::thread(serial_reader, hSer).detach();
//
//    int  user = -1;
//    char last_cmd = ' ';
//    uint8_t last_spd = 0;
//    WORD prevButtons = 0;
//
//    // RS 최근 전송값(중앙 128)
//    uint8_t last_pan = 128;
//    uint8_t last_tilt = 128;
//
//    std::puts("Scanning for controller...");
//
//    for (;;) {
//        if (user < 0) user = find_active_user_index();
//
//        if (user >= 0) {
//            XINPUT_STATE st{};
//            DWORD r = XInputGetState(user, &st);
//            if (r != ERROR_SUCCESS) {
//                if (last_cmd != ' ') { tx_cmd(hSer, ' ', 0); last_cmd = ' '; last_spd = 0; }
//                user = -1; Sleep(200); continue;
//            }
//
//            // ===== 버튼 상태/엣지 =====
//            WORD nowButtons = st.Gamepad.wButtons;
//            auto edge = [](WORD now, WORD prev, WORD mask)->bool {
//                return (now & mask) && !(prev & mask); // 눌린 '순간'만 true
//                };
//
//            // X 버튼 ‘눌린 순간’(edge)만 true
//            bool xEdge = ((nowButtons & XINPUT_GAMEPAD_X) && !(prevButtons & XINPUT_GAMEPAD_X));
//            if (xEdge) {
//                const char u = 'U';                 // 거리 요청 1바이트
//                DWORD bw;  WriteFile(hSer, &u, 1, &bw, nullptr);
//                std::puts("[PAD] X -> ultrasonic request");
//            }
//
//            // A 버튼 캡처(비동기)도 같은 방식으로
//            bool aEdge = ((nowButtons & XINPUT_GAMEPAD_A) && !(prevButtons & XINPUT_GAMEPAD_A));
//            if (aEdge && !g_busy.load()) {
//                g_busy = true;
//                std::thread([](){
//                    wchar_t path[512]; make_timestamped_path(path, 512);
//                    bool ok = http_download_capture(ESP_IP, path);
//                    std::printf("[CAP] %s -> %ls\n", ok ? "OK" : "FAIL", path);
//                    g_busy = false;
//                }).detach();
//            }
//            // --- B 버튼: XCLK=8 설정 후 스트림 창 열기 ---
//            if (((nowButtons & XINPUT_GAMEPAD_B) && !(prevButtons & XINPUT_GAMEPAD_B))) {
//                // 1) XCLK 8 MHz (포트 80)
//                bool ok = http_get_path(ESP_IP, L"/xclk?xclk=8");
//                std::printf("[B] set XCLK=8 : %s\n", ok ? "OK" : "FAIL");
//
//                // 2) 스트림 열기: 포트 81 필수
//                wchar_t url[128];
//                swprintf(url, 128, L"http://%s:81/stream", ESP_IP);
//                open_url(url);
//                std::puts("[B] opened :81/stream");
//            }
//
//            // --- LB: 현재 탭 닫기 (Ctrl+W) ---
//            if (edge(nowButtons, prevButtons, XINPUT_GAMEPAD_LEFT_SHOULDER)) {
//                send_virt_key('W', /*withCtrl=*/true);
//                std::puts("[PAD] LB -> close tab");
//            }
//
//            // --- RB: 새로고침 (F5) ---
//            if (edge(nowButtons, prevButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER)) {
//                send_virt_key(VK_F5, /*withCtrl=*/false);
//                std::puts("[PAD] RB -> refresh");
//            }
//
//            // --- Y 버튼: 인덱스 열기 (/) ---
//            if (edge(nowButtons, prevButtons, XINPUT_GAMEPAD_Y)) {
//                wchar_t url[128];
//                swprintf(url, 128, L"http://%s/", ESP_IP);
//                open_url(url);
//                std::puts("[PAD] Y -> open index");
//            }
//
//            // ★ 마지막에 한 번만 갱신 ★
//            prevButtons = nowButtons;
//
//            // ===== 주행 명령(LX/LY → w/a/s/d + speed) =====
//            SHORT LX = st.Gamepad.sThumbLX;
//            SHORT LY = st.Gamepad.sThumbLY;
//
//            char cmd = ' ';
//            uint8_t spd = 0;
//            if (std::abs(LX) < DEAD_ZONE && std::abs(LY) < DEAD_ZONE) { cmd = ' '; spd = 0; }
//            else if (std::abs(LY) >= std::abs(LX)) { spd = map_axis_to_speed(LY); cmd = (LY > 0) ? 'w' : 's'; }
//            else { spd = map_axis_to_speed(LX); cmd = (LX < 0) ? 'a' : 'd'; }
//            if (spd < 3) { cmd = ' '; spd = 0; }
//
//            if (cmd != last_cmd || spd != last_spd) {
//                tx_cmd(hSer, cmd, spd);
//                last_cmd = cmd; last_spd = spd;
//            }
//
//            // ===== RS → 서보(PAN/TILT) 전송 =====
//            SHORT RX = st.Gamepad.sThumbRX;
//            SHORT RY = st.Gamepad.sThumbRY;
//
//            // 필요에 따라 invert를 바꿔서 원하는 방향으로 움직이게 조정
//            uint8_t pan = map_axis_to_u8_centered(RX, /*invert=*/false); // 좌우
//            uint8_t tilt = map_axis_to_u8_centered(RY, /*invert=*/true); // 위로 밀면 값 커지게
//
//            // 변화량이 1 이상일 때만 전송(노이즈/대역폭 절약)
//            if (std::abs((int)pan - (int)last_pan) >= 1) {
//                tx_cmd(hSer, 'P', pan);      // STM32: 'P' 수신 시 PAN(서보) 업데이트
//                last_pan = pan;
//            }
//            if (std::abs((int)tilt - (int)last_tilt) >= 1) {
//                tx_cmd(hSer, 'T', tilt);     // STM32: 'T' 수신 시 TILT(서보) 업데이트
//                last_tilt = tilt;
//            }
//        }
//        else {
//            Sleep(300);
//        }
//
//        Sleep(10);
//    }
//
//    CloseHandle(hSer);
//    return 0;
//}

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

// 1) Windows 헤더를 제일 먼저
#include <windows.h>

// 2) Windows 의존 헤더들 (windows.h 뒤에)
#include <shellapi.h>    // ShellExecuteW
#include <winhttp.h>     // WinHTTP
#include <Xinput.h>      // 게임패드

#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "Xinput9_1_0.lib")

// 3) 표준 헤더들
#include <algorithm>
#include <thread>
#include <atomic>
#include <cstdio>
#include <cstdint>
#include <cmath>

// ======= 시리얼(STM32) =======
#define COM_PORT   L"\\\\.\\COM9"
#define BAUDRATE   115200

// ======= ESP32-CAM =======
static const wchar_t* ESP_IP = L"192.168.0.21";
static const wchar_t* SAVE_DIR = L"C:\\Users\\54\\Desktop\\capture";

#define DEAD_ZONE   8000
#define MAX_AXIS    32767

std::atomic<bool> g_busy{ false };

// 기본 브라우저로 URL 열기
static void open_url(const wchar_t* url) {
    ShellExecuteW(nullptr, L"open", url, nullptr, nullptr, SW_SHOWNORMAL);
}

// 활성 윈도우에 가상키 보내기 (예: F5, Ctrl+W)
static void send_virt_key(WORD vk, bool withCtrl) {
    INPUT in[4] = {};
    int n = 0;
    if (withCtrl) { in[n].type = INPUT_KEYBOARD; in[n].ki.wVk = VK_CONTROL; n++; }
    in[n].type = INPUT_KEYBOARD; in[n].ki.wVk = vk; n++;
    in[n].type = INPUT_KEYBOARD; in[n].ki.wVk = vk; in[n].ki.dwFlags = KEYEVENTF_KEYUP; n++;
    if (withCtrl) { in[n].type = INPUT_KEYBOARD; in[n].ki.wVk = VK_CONTROL; in[n].ki.dwFlags = KEYEVENTF_KEYUP; n++; }
    SendInput(n, in, sizeof(INPUT));
}

// ---------- 공용 유틸 ----------
static inline uint8_t map_axis_to_speed(int v) {
    int a = std::abs(v);
    if (a < DEAD_ZONE) return 0;
    if (a > MAX_AXIS) a = MAX_AXIS;
    return (uint8_t)((a * 255) / MAX_AXIS);
}

// RS(오른쪽 스틱) → 0..255(중앙=128)
static inline uint8_t map_axis_to_u8_centered(int v, bool invert = false) {
    int x = v;
    if (invert) x = -x;                     // 필요시 상하 반전
    if (std::abs(x) < DEAD_ZONE) return 128; // 중앙 고정
    if (x > MAX_AXIS) x = MAX_AXIS;
    if (x < -MAX_AXIS) x = -MAX_AXIS;
    // [-32767..32767] -> [-127..127] -> [1..255], 중앙=128
    int val = (x * 127) / MAX_AXIS;         // -127..127
    int u8 = 128 + val;                    // 1..255(중앙=128)
    if (u8 < 1)   u8 = 1;
    if (u8 > 255) u8 = 255;
    return (uint8_t)u8;
}

static void serial_reader(HANDLE h)
{
    enum { WAIT_HDR, GET_D1, GET_D2 } st = WAIT_HDR;
    BYTE b; DWORD br;
    uint8_t d1 = 0;

    for (;;) {
        if (!ReadFile(h, &b, 1, &br, nullptr) || br == 0) { Sleep(1); continue; }

        switch (st) {
        case WAIT_HDR:
            if (b == 'D') st = GET_D1;               // 헤더
            break;
        case GET_D1:
            d1 = (uint8_t)b; st = GET_D2;            // 첫 번째 거리
            break;
        case GET_D2: {
            uint8_t d2 = (uint8_t)b;                 // 두 번째 거리
            std::printf("[STM] D,%u,%u\n", d1, d2);  // 필요하면 여기서 로직 처리
            st = WAIT_HDR;
            break;
        }
        }
    }
}

// ---------- 시리얼 ----------
// 기존: h = CreateFileW(COM_PORT, GENERIC_WRITE, ...
// 변경: 읽기/쓰기를 모두 허용
static bool open_serial(HANDLE& h) {
    h = CreateFileW(
        COM_PORT,
        GENERIC_READ | GENERIC_WRITE,   // ★ 여기!
        0, nullptr, OPEN_EXISTING, 0, nullptr
    );
    if (h == INVALID_HANDLE_VALUE) return false;

    DCB dcb{}; dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(h, &dcb)) return false;
    dcb.BaudRate = BAUDRATE; dcb.ByteSize = 8; dcb.Parity = NOPARITY; dcb.StopBits = ONESTOPBIT;
    if (!SetCommState(h, &dcb)) return false;

    // ★ 읽기 타임아웃도 설정 (읽기 스레드가 부드럽게 동작)
    COMMTIMEOUTS to{};
    to.ReadIntervalTimeout = 50;
    to.ReadTotalTimeoutConstant = 50;
    to.ReadTotalTimeoutMultiplier = 0;
    to.WriteTotalTimeoutConstant = 50;
    to.WriteTotalTimeoutMultiplier = 0;
    SetCommTimeouts(h, &to);
    return true;
}

static void tx_cmd(HANDLE h, char cmd, uint8_t spd) {
    DWORD bw;
    if (cmd == ' ') { WriteFile(h, &cmd, 1, &bw, nullptr); std::printf("TX: STOP\n"); }
    else {
        WriteFile(h, &cmd, 1, &bw, nullptr);
        WriteFile(h, &spd, 1, &bw, nullptr);
        // 주행(w/a/s/d)면 speed, 서보(P/T)면 u8(중앙128)
        std::printf("TX: %c, val=%u\n", cmd, spd);
    }
}

// ---------- XInput ----------
static int find_active_user_index() {
    for (int i = 0; i < 4; ++i) {
        XINPUT_STATE st{};
        if (XInputGetState(i, &st) == ERROR_SUCCESS) return i;
    }
    return -1;
}

// ---------- 캡처 다운로드(백그라운드) ----------
static bool http_download_capture(const wchar_t* ip, const wchar_t* savePath) {
    HINTERNET h = WinHttpOpen(L"pc-cap",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!h) return false;

    HINTERNET c = WinHttpConnect(h, ip, 80, 0);
    if (!c) { WinHttpCloseHandle(h); return false; }

    HINTERNET r = WinHttpOpenRequest(c, L"GET", L"/capture",
        nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    if (!r) { WinHttpCloseHandle(c); WinHttpCloseHandle(h); return false; }

    BOOL ok = WinHttpSendRequest(r,
        WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        WINHTTP_NO_REQUEST_DATA, 0,
        0, 0) && WinHttpReceiveResponse(r, nullptr);

    HANDLE f = CreateFileW(savePath, GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (!ok || f == INVALID_HANDLE_VALUE) {
        if (f != INVALID_HANDLE_VALUE) CloseHandle(f);
        WinHttpCloseHandle(r); WinHttpCloseHandle(c); WinHttpCloseHandle(h);
        return false;
    }

    BYTE buf[16 * 1024];
    DWORD br = 0;
    for (;;) {
        DWORD avail = 0;
        if (!WinHttpQueryDataAvailable(r, &avail) || avail == 0) break;

        while (avail) {
            DWORD toRead = std::min<DWORD>(avail, sizeof(buf));
            if (!WinHttpReadData(r, buf, toRead, &br) || br == 0) { avail = 0; break; }
            DWORD bw = 0; WriteFile(f, buf, br, &bw, nullptr);
            avail -= br;
        }
    }

    CloseHandle(f);
    WinHttpCloseHandle(r); WinHttpCloseHandle(c); WinHttpCloseHandle(h);
    return true;
}

static void make_timestamped_path(wchar_t* out, size_t cap) {
    CreateDirectoryW(SAVE_DIR, nullptr); // 존재하면 실패해도 OK
    SYSTEMTIME t; GetLocalTime(&t);
    swprintf(out, cap, L"%s\\shot_%04u%02u%02u_%02u%02u%02u_%03u.jpg",
        SAVE_DIR, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
}

// 간단한 GET /path 호출 (호스트는 IP/호스트명, 포트 80 고정)
static bool http_get_path(const wchar_t* host, const wchar_t* path)
{
    HINTERNET h = WinHttpOpen(L"pc-ctrl",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!h) return false;

    HINTERNET c = WinHttpConnect(h, host, 80, 0);
    if (!c) { WinHttpCloseHandle(h); return false; }

    HINTERNET r = WinHttpOpenRequest(c, L"GET", path,
        nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    if (!r) { WinHttpCloseHandle(c); WinHttpCloseHandle(h); return false; }

    BOOL ok = WinHttpSendRequest(r,
        WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        WINHTTP_NO_REQUEST_DATA, 0,
        0, 0)
        && WinHttpReceiveResponse(r, nullptr);

    WinHttpCloseHandle(r);
    WinHttpCloseHandle(c);
    WinHttpCloseHandle(h);
    return ok == TRUE;
}

int main() {
    // 시리얼
    HANDLE hSer;
    if (!open_serial(hSer)) {
        std::puts("Serial open failed");
        return 1;
    }

    std::thread(serial_reader, hSer).detach();

    int  user = -1;
    char last_cmd = ' ';
    uint8_t last_spd = 0;
    WORD prevButtons = 0;

    // RS 최근 전송값(중앙 128)
    uint8_t last_pan = 128;
    uint8_t last_tilt = 128;

    std::puts("Scanning for controller...");

    for (;;) {
        if (user < 0) user = find_active_user_index();

        if (user >= 0) {
            XINPUT_STATE st{};
            DWORD r = XInputGetState(user, &st);
            if (r != ERROR_SUCCESS) {
                if (last_cmd != ' ') { tx_cmd(hSer, ' ', 0); last_cmd = ' '; last_spd = 0; }
                user = -1; Sleep(200); continue;
            }

            // ===== 버튼 상태/엣지 =====
            WORD nowButtons = st.Gamepad.wButtons;
            auto edge = [](WORD now, WORD prev, WORD mask)->bool {
                return (now & mask) && !(prev & mask); // 눌린 '순간'만 true
                };

            // X 버튼 ‘눌린 순간’(edge)만 true
            bool xEdge = ((nowButtons & XINPUT_GAMEPAD_X) && !(prevButtons & XINPUT_GAMEPAD_X));
            if (xEdge) {
                const char u = 'U';                 // 거리 요청 1바이트
                DWORD bw;  WriteFile(hSer, &u, 1, &bw, nullptr);
                std::puts("[PAD] X -> ultrasonic request");
            }

            // A 버튼 캡처(비동기)도 같은 방식으로
            bool aEdge = ((nowButtons & XINPUT_GAMEPAD_A) && !(prevButtons & XINPUT_GAMEPAD_A));
            if (aEdge && !g_busy.load()) {
                g_busy = true;
                std::thread([]() {
                    wchar_t path[512]; make_timestamped_path(path, 512);
                    bool ok = http_download_capture(ESP_IP, path);
                    std::printf("[CAP] %s -> %ls\n", ok ? "OK" : "FAIL", path);
                    g_busy = false;
                    }).detach();
            }
            // --- B 버튼: XCLK=8 설정 후 스트림 창 열기 ---
            if (((nowButtons & XINPUT_GAMEPAD_B) && !(prevButtons & XINPUT_GAMEPAD_B))) {
                // 1) XCLK 8 MHz (포트 80)
                bool ok = http_get_path(ESP_IP, L"/xclk?xclk=8");
                std::printf("[B] set XCLK=8 : %s\n", ok ? "OK" : "FAIL");

                // 2) 스트림 열기: 포트 81 필수
                wchar_t url[128];
                swprintf(url, 128, L"http://%s:81/stream", ESP_IP);
                open_url(url);
                std::puts("[B] opened :81/stream");
            }

            // --- LB: 현재 탭 닫기 (Ctrl+W) ---
            if (edge(nowButtons, prevButtons, XINPUT_GAMEPAD_LEFT_SHOULDER)) {
                send_virt_key('W', /*withCtrl=*/true);
                std::puts("[PAD] LB -> close tab");
            }

            // --- RB: 새로고침 (F5) ---
            if (edge(nowButtons, prevButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER)) {
                send_virt_key(VK_F5, /*withCtrl=*/false);
                std::puts("[PAD] RB -> refresh");
            }

            // --- Y 버튼: 인덱스 열기 (/) ---
            if (edge(nowButtons, prevButtons, XINPUT_GAMEPAD_Y)) {
                wchar_t url[128];
                swprintf(url, 128, L"http://%s/", ESP_IP);
                open_url(url);
                std::puts("[PAD] Y -> open index");
            }

            // ★ 마지막에 한 번만 갱신 ★
            prevButtons = nowButtons;

            // ===== 주행 명령(LX/LY → w/a/s/d + speed) =====
            SHORT LX = st.Gamepad.sThumbLX;
            SHORT LY = st.Gamepad.sThumbLY;

            char cmd = ' ';
            uint8_t spd = 0;
            if (std::abs(LX) < DEAD_ZONE && std::abs(LY) < DEAD_ZONE) { cmd = ' '; spd = 0; }
            else if (std::abs(LY) >= std::abs(LX)) { spd = map_axis_to_speed(LY); cmd = (LY > 0) ? 'w' : 's'; }
            else { spd = map_axis_to_speed(LX); cmd = (LX < 0) ? 'a' : 'd'; }
            if (spd < 3) { cmd = ' '; spd = 0; }

            if (cmd != last_cmd || spd != last_spd) {
                tx_cmd(hSer, cmd, spd);
                last_cmd = cmd; last_spd = spd;
            }

            // ===== RS → 서보(PAN/TILT) 전송 =====
            SHORT RX = st.Gamepad.sThumbRX;
            SHORT RY = st.Gamepad.sThumbRY;

            // 필요에 따라 invert를 바꿔서 원하는 방향으로 움직이게 조정
            uint8_t pan = map_axis_to_u8_centered(RX, /*invert=*/false); // 좌우
            uint8_t tilt = map_axis_to_u8_centered(RY, /*invert=*/true); // 위로 밀면 값 커지게

            // 변화량이 1 이상일 때만 전송(노이즈/대역폭 절약)
            if (std::abs((int)pan - (int)last_pan) >= 1) {
                tx_cmd(hSer, 'P', pan);      // STM32: 'P' 수신 시 PAN(서보) 업데이트
                last_pan = pan;
            }
            if (std::abs((int)tilt - (int)last_tilt) >= 1) {
                tx_cmd(hSer, 'T', tilt);     // STM32: 'T' 수신 시 TILT(서보) 업데이트
                last_tilt = tilt;
            }
        }
        else {
            Sleep(300);
        }

        Sleep(10);
    }

    CloseHandle(hSer);
    return 0;
}