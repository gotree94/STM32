#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ESP8266 (ESP-01) 모듈 설정 GUI 프로그램
- AP 목록 스캔 및 선택
- WiFi 연결 설정
- 서버 모드 설정
"""

import serial
import time
import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox
import threading
import re


class ESP8266Setup:
    def __init__(self, port, baudrate=9600):
        """ESP8266 설정 클래스 초기화"""
        self.port = port
        self.baudrate = baudrate
        self.ser = None
        self.ap_list = []
        
    def connect(self):
        """시리얼 포트 연결"""
        try:
            self.ser = serial.Serial(self.port, self.baudrate, timeout=2)
            time.sleep(2)  # 모듈 초기화 대기
            return True
        except Exception as e:
            print(f"연결 실패: {e}")
            return False
    
    def disconnect(self):
        """시리얼 포트 연결 해제"""
        if self.ser and self.ser.is_open:
            self.ser.close()
    
    def send_command(self, cmd, wait_time=1):
        """AT 명령어 전송 및 응답 수신"""
        if not self.ser or not self.ser.is_open:
            return None
        
        try:
            # 버퍼 비우기
            self.ser.reset_input_buffer()
            
            # 명령어 전송
            self.ser.write((cmd + '\r\n').encode())
            time.sleep(wait_time)
            
            # 응답 수신
            response = ""
            while self.ser.in_waiting > 0:
                response += self.ser.read(self.ser.in_waiting).decode('utf-8', errors='ignore')
                time.sleep(0.1)
            
            return response
        except Exception as e:
            print(f"명령어 전송 실패: {e}")
            return None
    
    def test_connection(self):
        """AT 명령어로 연결 테스트"""
        response = self.send_command("AT")
        return response and "OK" in response
    
    def set_station_mode(self):
        """Station 모드로 설정 (AP에 연결)"""
        response = self.send_command("AT+CWMODE=1")
        return response and "OK" in response
    
    def scan_ap(self, log_callback=None):
        """사용 가능한 AP 목록 스캔"""
        def log(msg):
            if log_callback:
                log_callback(msg)
            else:
                print(msg)
        
        # 버퍼 완전히 비우기
        if self.ser:
            self.ser.reset_input_buffer()
            self.ser.reset_output_buffer()
        
        # AP 스캔 명령 전송
        log("AT+CWLAP 명령 전송...")
        self.ser.write(b'AT+CWLAP\r\n')
        
        # 응답 수집 (최대 15초 대기)
        response = ""
        start_time = time.time()
        timeout = 15
        last_data_time = start_time
        
        while (time.time() - start_time) < timeout:
            if self.ser.in_waiting > 0:
                chunk = self.ser.read(self.ser.in_waiting).decode('utf-8', errors='ignore')
                response += chunk
                last_data_time = time.time()
                
                # OK가 나오면 스캔 완료
                if 'OK' in response:
                    log("스캔 완료 신호(OK) 수신")
                    time.sleep(0.5)  # 마지막 데이터 수신 대기
                    if self.ser.in_waiting > 0:
                        response += self.ser.read(self.ser.in_waiting).decode('utf-8', errors='ignore')
                    break
            else:
                # 데이터가 3초간 없으면 타임아웃
                if response and (time.time() - last_data_time) > 3:
                    log("3초간 추가 데이터 없음 - 스캔 종료")
                    break
            time.sleep(0.2)
        
        log(f"스캔 소요 시간: {time.time() - start_time:.1f}초")
        log(f"수신 데이터 크기: {len(response)} bytes")
        
        if not response:
            log("⚠️ 응답 없음")
            return []
        
        # 응답 일부 출력 (디버깅용)
        if len(response) > 200:
            log(f"응답 샘플: {response[:200]}...")
        else:
            log(f"전체 응답: {response}")
        
        # AP 정보 파싱
        # 형식: +CWLAP:(ecn,"ssid",rssi,"mac",channel)
        pattern = r'\+CWLAP:\((\d+),"([^"]+)",(-?\d+),"([^"]+)",(\d+)\)'
        matches = re.findall(pattern, response)
        
        log(f"파싱된 AP 개수: {len(matches)}")
        
        ap_list = []
        for match in matches:
            ecn, ssid, rssi, mac, channel = match
            # ECN 타입 변환
            ecn_type = {
                '0': 'OPEN',
                '1': 'WEP',
                '2': 'WPA_PSK',
                '3': 'WPA2_PSK',
                '4': 'WPA_WPA2_PSK'
            }.get(ecn, 'UNKNOWN')
            
            ap_list.append({
                'ssid': ssid,
                'rssi': int(rssi),
                'security': ecn_type,
                'mac': mac,
                'channel': int(channel)
            })
            log(f"  → {ssid} ({rssi} dBm, {ecn_type})")
        
        # RSSI 순으로 정렬 (신호 강도가 높은 순)
        ap_list.sort(key=lambda x: x['rssi'], reverse=True)
        self.ap_list = ap_list
        return ap_list
    
    def connect_ap(self, ssid, password):
        """AP에 연결"""
        cmd = f'AT+CWJAP="{ssid}","{password}"'
        response = self.send_command(cmd, wait_time=10)
        return response and "OK" in response
    
    def enable_multiple_connections(self):
        """다중 연결 모드 활성화"""
        response = self.send_command("AT+CIPMUX=1")
        return response and "OK" in response
    
    def start_server(self, port=80):
        """서버 시작"""
        cmd = f"AT+CIPSERVER=1,{port}"
        response = self.send_command(cmd)
        return response and "OK" in response
    
    def get_ip_address(self):
        """IP 주소 확인"""
        response = self.send_command("AT+CIFSR", wait_time=2)
        return response
    
    def get_current_mode(self):
        """현재 WiFi 모드 확인"""
        response = self.send_command("AT+CWMODE?")
        if response:
            match = re.search(r'\+CWMODE:(\d+)', response)
            if match:
                mode = match.group(1)
                mode_map = {'1': 'Station', '2': 'AP', '3': 'Station+AP'}
                return mode_map.get(mode, f'Unknown({mode})')
        return None
    
    def get_connected_ap(self):
        """현재 연결된 AP 확인"""
        response = self.send_command("AT+CWJAP?")
        if response:
            match = re.search(r'\+CWJAP:"([^"]+)"', response)
            if match:
                return match.group(1)
        return None
    
    def get_connection_status(self):
        """연결 상태 확인"""
        response = self.send_command("AT+CIPSTATUS")
        if response:
            # STATUS 추출
            match = re.search(r'STATUS:(\d+)', response)
            if match:
                status = match.group(1)
                status_map = {
                    '2': 'Got IP',
                    '3': 'Connected',
                    '4': 'Disconnected',
                    '5': 'WiFi Disconnected'
                }
                return status_map.get(status, f'Unknown({status})')
        return None
    
    def get_multiplex_mode(self):
        """다중 연결 모드 확인"""
        response = self.send_command("AT+CIPMUX?")
        if response:
            match = re.search(r'\+CIPMUX:(\d+)', response)
            if match:
                mode = match.group(1)
                return 'Multiple' if mode == '1' else 'Single'
        return None
    
    def get_ap_config(self):
        """SoftAP 설정 확인"""
        response = self.send_command("AT+CWSAP?")
        if response:
            match = re.search(r'\+CWSAP:"([^"]+)","([^"]+)",(\d+),(\d+)', response)
            if match:
                return {
                    'ssid': match.group(1),
                    'password': match.group(2),
                    'channel': match.group(3),
                    'encryption': match.group(4)
                }
        return None
    
    def get_mac_address(self):
        """MAC 주소 확인"""
        response = self.send_command("AT+CIPSTAMAC?")
        if response:
            match = re.search(r'\+CIPSTAMAC:"([^"]+)"', response)
            if match:
                return match.group(1)
        return None


class ESP8266GUI:
    def __init__(self, root):
        """GUI 초기화"""
        self.root = root
        self.root.title("ESP8266 (ESP-01) 설정 프로그램")
        self.root.geometry("700x600")
        
        self.esp = None
        self.setup_ui()
        
    def setup_ui(self):
        """UI 구성"""
        # 스타일 설정
        style = ttk.Style()
        style.theme_use('clam')
        
        # 메인 프레임
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # 1. 연결 설정 섹션
        connection_frame = ttk.LabelFrame(main_frame, text="연결 설정", padding="10")
        connection_frame.grid(row=0, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=5)
        
        ttk.Label(connection_frame, text="포트:").grid(row=0, column=0, sticky=tk.W, padx=5)
        self.port_entry = ttk.Entry(connection_frame, width=20)
        self.port_entry.insert(0, "COM3")  # 기본값
        self.port_entry.grid(row=0, column=1, sticky=tk.W, padx=5)
        
        ttk.Label(connection_frame, text="속도:").grid(row=0, column=2, sticky=tk.W, padx=5)
        self.baudrate_entry = ttk.Entry(connection_frame, width=10)
        self.baudrate_entry.insert(0, "9600")
        self.baudrate_entry.grid(row=0, column=3, sticky=tk.W, padx=5)
        
        self.connect_btn = ttk.Button(connection_frame, text="연결", command=self.connect_esp)
        self.connect_btn.grid(row=0, column=4, padx=5)
        
        # 1-1. 현재 상태 표시 섹션 (새로 추가)
        status_frame = ttk.LabelFrame(main_frame, text="현재 상태", padding="10")
        status_frame.grid(row=1, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=5)
        
        # 상태 정보 레이블들
        status_info_frame = ttk.Frame(status_frame)
        status_info_frame.grid(row=0, column=0, sticky=(tk.W, tk.E))
        
        # 첫 번째 줄
        ttk.Label(status_info_frame, text="WiFi 모드:").grid(row=0, column=0, sticky=tk.W, padx=5, pady=2)
        self.mode_label = ttk.Label(status_info_frame, text="-", foreground="gray")
        self.mode_label.grid(row=0, column=1, sticky=tk.W, padx=5, pady=2)
        
        ttk.Label(status_info_frame, text="연결 상태:").grid(row=0, column=2, sticky=tk.W, padx=15, pady=2)
        self.status_label = ttk.Label(status_info_frame, text="-", foreground="gray")
        self.status_label.grid(row=0, column=3, sticky=tk.W, padx=5, pady=2)
        
        ttk.Label(status_info_frame, text="다중연결:").grid(row=0, column=4, sticky=tk.W, padx=15, pady=2)
        self.mux_label = ttk.Label(status_info_frame, text="-", foreground="gray")
        self.mux_label.grid(row=0, column=5, sticky=tk.W, padx=5, pady=2)
        
        # 두 번째 줄
        ttk.Label(status_info_frame, text="연결된 AP:").grid(row=1, column=0, sticky=tk.W, padx=5, pady=2)
        self.ap_label = ttk.Label(status_info_frame, text="-", foreground="gray")
        self.ap_label.grid(row=1, column=1, columnspan=2, sticky=tk.W, padx=5, pady=2)
        
        ttk.Label(status_info_frame, text="IP 주소:").grid(row=1, column=2, sticky=tk.W, padx=15, pady=2)
        self.ip_label = ttk.Label(status_info_frame, text="-", foreground="gray")
        self.ip_label.grid(row=1, column=3, columnspan=2, sticky=tk.W, padx=5, pady=2)
        
        # 세 번째 줄
        ttk.Label(status_info_frame, text="MAC 주소:").grid(row=2, column=0, sticky=tk.W, padx=5, pady=2)
        self.mac_label = ttk.Label(status_info_frame, text="-", foreground="gray")
        self.mac_label.grid(row=2, column=1, columnspan=2, sticky=tk.W, padx=5, pady=2)
        
        # 상태 새로고침 버튼
        self.refresh_status_btn = ttk.Button(status_frame, text="⟳ 상태 새로고침", 
                                             command=self.refresh_status, state='disabled')
        self.refresh_status_btn.grid(row=1, column=0, pady=5)
        
        # 2. AP 스캔 섹션
        scan_frame = ttk.LabelFrame(main_frame, text="WiFi AP 검색", padding="10")
        scan_frame.grid(row=2, column=0, columnspan=2, sticky=(tk.W, tk.E, tk.N, tk.S), pady=5)
        
        
        # 2. AP 스캔 섹션
        scan_frame = ttk.LabelFrame(main_frame, text="WiFi AP 검색", padding="10")
        scan_frame.grid(row=2, column=0, columnspan=2, sticky=(tk.W, tk.E, tk.N, tk.S), pady=5)
        
        self.scan_btn = ttk.Button(scan_frame, text="AP 스캔", command=self.scan_networks, state='disabled')
        self.scan_btn.grid(row=0, column=0, pady=5)
        
        # AP 목록 테이블
        columns = ('SSID', 'RSSI', '보안', 'MAC', '채널')
        self.ap_tree = ttk.Treeview(scan_frame, columns=columns, show='headings', height=8)
        
        for col in columns:
            self.ap_tree.heading(col, text=col)
            if col == 'SSID':
                self.ap_tree.column(col, width=150)
            elif col == 'MAC':
                self.ap_tree.column(col, width=130)
            elif col == '보안':
                self.ap_tree.column(col, width=100)
            else:
                self.ap_tree.column(col, width=60)
        
        self.ap_tree.grid(row=1, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        self.ap_tree.bind('<<TreeviewSelect>>', self.on_ap_select)
        
        # 스크롤바
        scrollbar = ttk.Scrollbar(scan_frame, orient=tk.VERTICAL, command=self.ap_tree.yview)
        scrollbar.grid(row=1, column=1, sticky=(tk.N, tk.S))
        self.ap_tree.configure(yscrollcommand=scrollbar.set)
        
        # 3. WiFi 연결 섹션
        wifi_frame = ttk.LabelFrame(main_frame, text="WiFi 연결", padding="10")
        wifi_frame.grid(row=3, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=5)
        
        ttk.Label(wifi_frame, text="SSID:").grid(row=0, column=0, sticky=tk.W, padx=5)
        self.ssid_entry = ttk.Entry(wifi_frame, width=30, state='readonly')
        self.ssid_entry.grid(row=0, column=1, sticky=tk.W, padx=5)
        
        ttk.Label(wifi_frame, text="비밀번호:").grid(row=1, column=0, sticky=tk.W, padx=5, pady=5)
        self.password_entry = ttk.Entry(wifi_frame, width=30, show='*')
        self.password_entry.grid(row=1, column=1, sticky=tk.W, padx=5, pady=5)
        
        self.connect_wifi_btn = ttk.Button(wifi_frame, text="WiFi 연결", 
                                           command=self.connect_wifi, state='disabled')
        self.connect_wifi_btn.grid(row=1, column=2, padx=5)
        
        # 4. 서버 설정 섹션
        server_frame = ttk.LabelFrame(main_frame, text="서버 설정", padding="10")
        server_frame.grid(row=4, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=5)
        
        ttk.Label(server_frame, text="포트:").grid(row=0, column=0, sticky=tk.W, padx=5)
        self.server_port_entry = ttk.Entry(server_frame, width=10)
        self.server_port_entry.insert(0, "80")
        self.server_port_entry.grid(row=0, column=1, sticky=tk.W, padx=5)
        
        self.start_server_btn = ttk.Button(server_frame, text="서버 시작", 
                                           command=self.start_server, state='disabled')
        self.start_server_btn.grid(row=0, column=2, padx=5)
        
        self.get_ip_btn = ttk.Button(server_frame, text="IP 주소 확인", 
                                     command=self.get_ip, state='disabled')
        self.get_ip_btn.grid(row=0, column=3, padx=5)
        
        # 5. 디버깅 섹션 (AT 명령어 직접 입력)
        debug_frame = ttk.LabelFrame(main_frame, text="디버깅 (AT 명령어)", padding="10")
        debug_frame.grid(row=5, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=5)
        
        # 명령어 입력
        cmd_input_frame = ttk.Frame(debug_frame)
        cmd_input_frame.grid(row=0, column=0, sticky=(tk.W, tk.E))
        
        ttk.Label(cmd_input_frame, text="명령어:").grid(row=0, column=0, sticky=tk.W, padx=5)
        self.cmd_entry = ttk.Entry(cmd_input_frame, width=40)
        self.cmd_entry.grid(row=0, column=1, sticky=(tk.W, tk.E), padx=5)
        self.cmd_entry.bind('<Return>', lambda e: self.send_custom_command())
        
        ttk.Label(cmd_input_frame, text="대기(초):").grid(row=0, column=2, sticky=tk.W, padx=5)
        self.wait_time_spinbox = ttk.Spinbox(cmd_input_frame, from_=0.5, to=30, increment=0.5, width=8)
        self.wait_time_spinbox.set(2)
        self.wait_time_spinbox.grid(row=0, column=3, sticky=tk.W, padx=5)
        
        self.send_cmd_btn = ttk.Button(cmd_input_frame, text="전송", 
                                       command=self.send_custom_command, state='disabled')
        self.send_cmd_btn.grid(row=0, column=4, padx=5)
        
        # 빠른 명령어 버튼들
        quick_cmd_frame = ttk.Frame(debug_frame)
        quick_cmd_frame.grid(row=1, column=0, sticky=(tk.W, tk.E), pady=5)
        
        ttk.Label(quick_cmd_frame, text="빠른 명령:").grid(row=0, column=0, sticky=tk.W, padx=5)
        
        quick_commands = [
            ("AT", "AT"),
            ("버전", "AT+GMR"),
            ("모드확인", "AT+CWMODE?"),
            ("AP확인", "AT+CWJAP?"),
            ("상태", "AT+CIPSTATUS"),
            ("리셋", "AT+RST")
        ]
        
        for idx, (label, cmd) in enumerate(quick_commands):
            btn = ttk.Button(quick_cmd_frame, text=label, width=8,
                           command=lambda c=cmd: self.quick_command(c))
            btn.grid(row=0, column=idx+1, padx=2)
            if idx == 0:
                self.quick_cmd_buttons = [btn]
            else:
                self.quick_cmd_buttons.append(btn)
            btn.config(state='disabled')
        
        cmd_input_frame.columnconfigure(1, weight=1)
        
        # 6. 로그 섹션
        log_frame = ttk.LabelFrame(main_frame, text="로그", padding="10")
        log_frame.grid(row=6, column=0, columnspan=2, sticky=(tk.W, tk.E, tk.N, tk.S), pady=5)
        
        
        # 6. 로그 섹션
        log_frame = ttk.LabelFrame(main_frame, text="로그", padding="10")
        log_frame.grid(row=6, column=0, columnspan=2, sticky=(tk.W, tk.E, tk.N, tk.S), pady=5)
        
        self.log_text = scrolledtext.ScrolledText(log_frame, height=10, width=70)
        self.log_text.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # 로그 지우기 버튼
        clear_log_btn = ttk.Button(log_frame, text="로그 지우기", command=self.clear_log)
        clear_log_btn.grid(row=1, column=0, pady=5)
        
        # 그리드 가중치 설정
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        main_frame.columnconfigure(0, weight=1)
        main_frame.rowconfigure(2, weight=1)  # AP 스캔 섹션
        main_frame.rowconfigure(6, weight=1)  # 로그 섹션
        scan_frame.columnconfigure(0, weight=1)
        scan_frame.rowconfigure(1, weight=1)
        log_frame.columnconfigure(0, weight=1)
        log_frame.rowconfigure(0, weight=1)
    
    def log(self, message):
        """로그 메시지 출력"""
        self.log_text.insert(tk.END, f"[{time.strftime('%H:%M:%S')}] {message}\n")
        self.log_text.see(tk.END)
        self.root.update()
    
    def connect_esp(self):
        """ESP8266 연결"""
        port = self.port_entry.get()
        baudrate = int(self.baudrate_entry.get())
        
        self.log(f"ESP8266 연결 시도: {port}, {baudrate}bps")
        
        self.esp = ESP8266Setup(port, baudrate)
        
        if self.esp.connect():
            self.log("시리얼 포트 연결 성공")
            
            # AT 명령어 테스트
            if self.esp.test_connection():
                self.log("ESP8266 모듈 응답 확인")
                
                # Station 모드 설정
                if self.esp.set_station_mode():
                    self.log("Station 모드 설정 완료")
                    self.scan_btn.config(state='normal')
                    self.refresh_status_btn.config(state='normal')
                    self.send_cmd_btn.config(state='normal')
                    for btn in self.quick_cmd_buttons:
                        btn.config(state='normal')
                    self.connect_btn.config(state='disabled')
                    
                    # 초기 상태 읽기
                    self.log("현재 상태 읽는 중...")
                    self.refresh_status()
                    
                    messagebox.showinfo("성공", "ESP8266 연결 성공!")
                else:
                    self.log("Station 모드 설정 실패")
            else:
                self.log("ESP8266 모듈 응답 없음")
                messagebox.showerror("오류", "ESP8266 응답이 없습니다.")
        else:
            self.log("시리얼 포트 연결 실패")
            messagebox.showerror("오류", "시리얼 포트 연결에 실패했습니다.")
    
    def scan_networks(self):
        """WiFi AP 스캔"""
        self.log("AP 스캔 시작... (최대 15초 소요)")
        self.log("주변 WiFi AP를 검색하고 있습니다. 잠시만 기다려주세요...")
        self.scan_btn.config(state='disabled')
        
        # 별도 스레드에서 실행
        def scan_thread():
            # 로그 콜백 함수 전달
            ap_list = self.esp.scan_ap(log_callback=lambda msg: self.root.after(0, self.log, msg))
            
            # UI 업데이트는 메인 스레드에서
            self.root.after(0, self.update_ap_list, ap_list)
        
        threading.Thread(target=scan_thread, daemon=True).start()
    
    def update_ap_list(self, ap_list):
        """AP 목록 업데이트"""
        # 기존 항목 삭제
        for item in self.ap_tree.get_children():
            self.ap_tree.delete(item)
        
        if len(ap_list) == 0:
            self.log("❌ AP를 찾지 못했습니다.")
            self.log("다음을 확인해주세요:")
            self.log("  1. ESP8266이 Station 모드인지 확인")
            self.log("  2. AT+CWLAP 명령어가 정상 동작하는지 확인")
            self.log("  3. 시리얼 모니터에서 직접 AT+CWLAP 테스트")
            messagebox.showwarning("경고", "AP를 찾지 못했습니다.\n로그를 확인해주세요.")
        else:
            # 새 항목 추가
            for ap in ap_list:
                self.ap_tree.insert('', tk.END, values=(
                    ap['ssid'],
                    f"{ap['rssi']} dBm",
                    ap['security'],
                    ap['mac'],
                    ap['channel']
                ))
            
            self.log(f"✓ AP 스캔 완료: {len(ap_list)}개 발견")
        
        self.scan_btn.config(state='normal')
    
    def on_ap_select(self, event):
        """AP 선택 시"""
        selection = self.ap_tree.selection()
        if selection:
            item = self.ap_tree.item(selection[0])
            ssid = item['values'][0]
            
            # SSID 자동 입력
            self.ssid_entry.config(state='normal')
            self.ssid_entry.delete(0, tk.END)
            self.ssid_entry.insert(0, ssid)
            self.ssid_entry.config(state='readonly')
            
            # 비밀번호 입력란 활성화
            self.password_entry.delete(0, tk.END)
            self.password_entry.focus()
            self.connect_wifi_btn.config(state='normal')
            
            self.log(f"선택된 AP: {ssid}")
    
    def connect_wifi(self):
        """WiFi 연결"""
        ssid = self.ssid_entry.get()
        password = self.password_entry.get()
        
        if not ssid:
            messagebox.showwarning("경고", "AP를 선택해주세요.")
            return
        
        self.log(f"WiFi 연결 시도: {ssid}")
        self.connect_wifi_btn.config(state='disabled')
        
        # 별도 스레드에서 실행
        def connect_thread():
            success = self.esp.connect_ap(ssid, password)
            self.root.after(0, self.on_wifi_connect_complete, success, ssid)
        
        threading.Thread(target=connect_thread, daemon=True).start()
    
    def on_wifi_connect_complete(self, success, ssid):
        """WiFi 연결 완료 처리"""
        if success:
            self.log(f"WiFi 연결 성공: {ssid}")
            messagebox.showinfo("성공", f"{ssid}에 연결되었습니다!")
            self.start_server_btn.config(state='normal')
            self.get_ip_btn.config(state='normal')
            
            # 상태 새로고침
            time.sleep(1)  # 연결 안정화 대기
            self.refresh_status()
        else:
            self.log(f"WiFi 연결 실패: {ssid}")
            messagebox.showerror("오류", "WiFi 연결에 실패했습니다.\n비밀번호를 확인해주세요.")
            self.connect_wifi_btn.config(state='normal')
    
    def start_server(self):
        """서버 시작"""
        port = int(self.server_port_entry.get())
        
        self.log("다중 연결 모드 활성화...")
        if self.esp.enable_multiple_connections():
            self.log("다중 연결 모드 활성화 완료")
            
            self.log(f"서버 시작 중... (포트: {port})")
            if self.esp.start_server(port):
                self.log(f"서버 시작 완료 (포트: {port})")
                messagebox.showinfo("성공", f"서버가 포트 {port}에서 시작되었습니다!")
                self.start_server_btn.config(state='disabled')
                
                # 상태 새로고침
                self.refresh_status()
            else:
                self.log("서버 시작 실패")
                messagebox.showerror("오류", "서버 시작에 실패했습니다.")
        else:
            self.log("다중 연결 모드 활성화 실패")
            messagebox.showerror("오류", "다중 연결 모드 활성화에 실패했습니다.")
    
    def get_ip(self):
        """IP 주소 확인"""
        self.log("IP 주소 확인 중...")
        response = self.esp.get_ip_address()
        
        if response:
            self.log("IP 주소 정보:")
            self.log(response)
            
            # IP 주소 파싱
            ip_pattern = r'\d+\.\d+\.\d+\.\d+'
            ips = re.findall(ip_pattern, response)
            if ips:
                messagebox.showinfo("IP 주소", f"ESP8266 IP 주소:\n{ips[0]}")
        else:
            self.log("IP 주소 확인 실패")
    
    def refresh_status(self):
        """현재 상태 새로고침"""
        self.log("상태 정보 읽는 중...")
        self.refresh_status_btn.config(state='disabled')
        
        def refresh_thread():
            status_info = {}
            
            # WiFi 모드 확인
            mode = self.esp.get_current_mode()
            status_info['mode'] = mode
            
            # 연결된 AP 확인
            ap = self.esp.get_connected_ap()
            status_info['ap'] = ap
            
            # 연결 상태 확인
            status = self.esp.get_connection_status()
            status_info['status'] = status
            
            # 다중 연결 모드 확인
            mux = self.esp.get_multiplex_mode()
            status_info['mux'] = mux
            
            # IP 주소 확인
            ip_response = self.esp.get_ip_address()
            if ip_response:
                ip_pattern = r'\d+\.\d+\.\d+\.\d+'
                ips = re.findall(ip_pattern, ip_response)
                status_info['ip'] = ips[0] if ips else None
            else:
                status_info['ip'] = None
            
            # MAC 주소 확인
            mac = self.esp.get_mac_address()
            status_info['mac'] = mac
            
            # UI 업데이트는 메인 스레드에서
            self.root.after(0, self.update_status_display, status_info)
        
        threading.Thread(target=refresh_thread, daemon=True).start()
    
    def update_status_display(self, status_info):
        """상태 정보를 UI에 업데이트"""
        # WiFi 모드
        if status_info.get('mode'):
            self.mode_label.config(text=status_info['mode'], foreground='blue')
            self.log(f"WiFi 모드: {status_info['mode']}")
        else:
            self.mode_label.config(text="알 수 없음", foreground='gray')
        
        # 연결 상태
        if status_info.get('status'):
            status_text = status_info['status']
            color = 'green' if 'IP' in status_text or 'Connected' in status_text else 'orange'
            self.status_label.config(text=status_text, foreground=color)
            self.log(f"연결 상태: {status_text}")
        else:
            self.status_label.config(text="알 수 없음", foreground='gray')
        
        # 다중 연결 모드
        if status_info.get('mux'):
            mux_text = status_info['mux']
            color = 'green' if mux_text == 'Multiple' else 'blue'
            self.mux_label.config(text=mux_text, foreground=color)
            self.log(f"다중 연결: {mux_text}")
        else:
            self.mux_label.config(text="알 수 없음", foreground='gray')
        
        # 연결된 AP
        if status_info.get('ap'):
            self.ap_label.config(text=status_info['ap'], foreground='green')
            self.log(f"연결된 AP: {status_info['ap']}")
        else:
            self.ap_label.config(text="연결 안됨", foreground='gray')
        
        # IP 주소
        if status_info.get('ip'):
            self.ip_label.config(text=status_info['ip'], foreground='green')
            self.log(f"IP 주소: {status_info['ip']}")
        else:
            self.ip_label.config(text="할당 안됨", foreground='gray')
        
        # MAC 주소
        if status_info.get('mac'):
            self.mac_label.config(text=status_info['mac'], foreground='blue')
            self.log(f"MAC 주소: {status_info['mac']}")
        else:
            self.mac_label.config(text="알 수 없음", foreground='gray')
        
        self.log("상태 정보 업데이트 완료")
        self.refresh_status_btn.config(state='normal')
    
    def send_custom_command(self):
        """사용자 정의 AT 명령어 전송"""
        cmd = self.cmd_entry.get().strip()
        if not cmd:
            return
        
        wait_time = float(self.wait_time_spinbox.get())
        
        self.log(f"→ 전송: {cmd}")
        self.send_cmd_btn.config(state='disabled')
        
        def send_thread():
            response = self.esp.send_command(cmd, wait_time=wait_time)
            self.root.after(0, self.on_command_response, cmd, response)
        
        threading.Thread(target=send_thread, daemon=True).start()
    
    def on_command_response(self, cmd, response):
        """명령어 응답 처리"""
        if response:
            self.log(f"← 응답:")
            # 응답을 줄 단위로 분리해서 출력
            for line in response.split('\n'):
                line = line.strip()
                if line:
                    self.log(f"  {line}")
        else:
            self.log("← 응답 없음")
        
        self.send_cmd_btn.config(state='normal')
    
    def quick_command(self, cmd):
        """빠른 명령어 실행"""
        self.cmd_entry.delete(0, tk.END)
        self.cmd_entry.insert(0, cmd)
        self.send_custom_command()
    
    def clear_log(self):
        """로그 지우기"""
        self.log_text.delete(1.0, tk.END)


def main():
    """메인 함수"""
    root = tk.Tk()
    app = ESP8266GUI(root)
    root.mainloop()


if __name__ == "__main__":
    main()
