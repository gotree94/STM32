import tkinter as tk
from tkinter import ttk, messagebox, filedialog
import cv2
from PIL import Image, ImageTk
import threading
import time
from datetime import datetime
import math
import serial
import serial.tools.list_ports
import os
try:
    import pygame
    PYGAME_AVAILABLE = True
except ImportError:
    PYGAME_AVAILABLE = False

class ESP32CamViewer:
    def __init__(self, root):
        self.root = root
        self.root.title("ESP32-CAM 통합 제어 시스템")
        
        # 창 최대화
        self.root.state('zoomed')  # Windows
        # self.root.attributes('-zoomed', True)  # Linux
        
        # 다크 그레이 배경색 설정
        self.root.configure(bg='#2b2b2b')
        
        # 스타일 설정
        style = ttk.Style()
        style.theme_use('clam')
        style.configure('TFrame', background='#2b2b2b')
        style.configure('TLabel', background='#2b2b2b', foreground='white')
        style.configure('TLabelframe', background='#2b2b2b', foreground='white')
        style.configure('TLabelframe.Label', background='#2b2b2b', foreground='white')
        style.configure('TButton', background='#404040', foreground='white')
        style.map('TButton', background=[('active', '#505050')])
        
        # 캡처 저장 경로
        self.capture_dir = r"C:\Users\54\Desktop\capture"
        
        # 스트리밍 관련
        self.is_streaming = False
        self.stream_thread = None
        self.cap = None
        self.current_frame = None
        self.frame_rate = 5
        
        # 시리얼 관련
        self.serial_port = None
        self.serial_thread = None
        self.serial_running = False
        
        # 게임패드 관련
        self.gamepad = None
        self.gamepad_thread = None
        self.gamepad_running = False
        
        # 조이스틱 데이터
        self.joy_lx = 0  # 왼쪽 스틱 X (-32767 ~ 32767)
        self.joy_ly = 0  # 왼쪽 스틱 Y
        self.joy_direction = 0  # 0~360도
        self.joy_value = 0  # 0~100%
        
        # 오른쪽 스틱 (팬/틸트)
        self.joy_rx = 0
        self.joy_ry = 0
        
        # 초음파 센서 데이터
        self.ultrasonic_left = 0
        self.ultrasonic_right = 0
        
        # 팬/틸트 각도
        self.pan_value = 128  # 0~255, 중앙 128
        self.tilt_value = 128
        self.pan_angle = 90  # 표시용 0~180
        self.tilt_angle = 90
        
        # 마지막 전송 값 (중복 전송 방지)
        self.last_cmd = ' '
        self.last_spd = 0
        self.last_pan = 128
        self.last_tilt = 128
        
        # 상수
        self.DEAD_ZONE = 8000
        self.MAX_AXIS = 32767
        
        # log_text 위젯 참조 (나중에 생성됨)
        self.log_text = None
        
        self.setup_ui()
        self.start_update_loop()
        
        # pygame 초기화
        if PYGAME_AVAILABLE:
            pygame.init()
            pygame.joystick.init()
        
        # 캡처 디렉토리 생성
        if not os.path.exists(self.capture_dir):
            try:
                os.makedirs(self.capture_dir)
                print(f"캡처 디렉토리 생성: {self.capture_dir}")
            except Exception as e:
                print(f"캡처 디렉토리 생성 실패: {e}")
    
    def setup_ui(self):
        # 메인 컨테이너
        main_container = ttk.Frame(self.root)
        main_container.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # 왼쪽 패널
        left_panel = ttk.Frame(main_container)
        left_panel.pack(side=tk.LEFT, fill=tk.BOTH, padx=(0, 10))
        
        # 연결 설정
        connection_frame = ttk.LabelFrame(left_panel, text="연결 설정", padding="10")
        connection_frame.pack(fill=tk.X, pady=(0, 5))
        
        # 시리얼 포트
        serial_row = ttk.Frame(connection_frame)
        serial_row.pack(fill=tk.X, pady=2)
        ttk.Label(serial_row, text="시리얼 포트:").pack(side=tk.LEFT, padx=5)
        self.port_combo = ttk.Combobox(serial_row, width=12, state="readonly")
        self.port_combo.pack(side=tk.LEFT, padx=5)
        ttk.Button(serial_row, text="새로고침", command=self.refresh_ports, width=8).pack(side=tk.LEFT, padx=2)
        self.serial_btn = ttk.Button(serial_row, text="연결", command=self.toggle_serial, width=8)
        self.serial_btn.pack(side=tk.LEFT, padx=2)
        self.serial_status = ttk.Label(serial_row, text="● 끊김", foreground="red")
        self.serial_status.pack(side=tk.LEFT, padx=5)
        
        # 게임패드
        gamepad_row = ttk.Frame(connection_frame)
        gamepad_row.pack(fill=tk.X, pady=2)
        ttk.Label(gamepad_row, text="게임패드:").pack(side=tk.LEFT, padx=5)
        self.gamepad_btn = ttk.Button(gamepad_row, text="연결", command=self.toggle_gamepad, width=8, 
                                      state=tk.NORMAL if PYGAME_AVAILABLE else tk.DISABLED)
        self.gamepad_btn.pack(side=tk.LEFT, padx=5)
        self.gamepad_status = ttk.Label(gamepad_row, text="● 끊김", foreground="red")
        self.gamepad_status.pack(side=tk.LEFT, padx=5)
        if not PYGAME_AVAILABLE:
            ttk.Label(gamepad_row, text="(pygame 미설치)", foreground="orange").pack(side=tk.LEFT)
        
        # 스트림 URL
        stream_row = ttk.Frame(connection_frame)
        stream_row.pack(fill=tk.X, pady=2)
        ttk.Label(stream_row, text="스트림 URL:").pack(side=tk.LEFT, padx=5)
        self.url_entry = ttk.Entry(stream_row, width=28)
        self.url_entry.insert(0, "http://192.168.0.21:81/stream")
        self.url_entry.pack(side=tk.LEFT, padx=5)
        self.connect_btn = ttk.Button(stream_row, text="시작", command=self.toggle_stream, width=8)
        self.connect_btn.pack(side=tk.LEFT, padx=5)
        self.status_label = ttk.Label(stream_row, text="● 끊김", foreground="red")
        self.status_label.pack(side=tk.LEFT, padx=5)
        
        # 비디오 프레임
        video_frame = ttk.LabelFrame(left_panel, text="카메라 영상 (640x480)", padding="5")
        video_frame.pack()
        
        self.canvas = tk.Canvas(video_frame, bg="black", width=640, height=480)
        self.canvas.pack()
        
        # 캡처 버튼
        capture_frame = ttk.Frame(left_panel)
        capture_frame.pack(pady=10)
        self.capture_btn = ttk.Button(capture_frame, text="📷 사진 캡처", command=self.capture_image, state=tk.DISABLED)
        self.capture_btn.pack()
        
        # 오른쪽 패널
        right_panel = ttk.Frame(main_container, width=400)
        right_panel.pack(side=tk.RIGHT, fill=tk.BOTH)
        
        # 조이스틱 표시
        joy_frame = ttk.LabelFrame(right_panel, text="왼쪽 스틱 (주행)", padding="10")
        joy_frame.pack(fill=tk.X, pady=(0, 10))
        
        self.joy_canvas = tk.Canvas(joy_frame, width=200, height=200, bg="white")
        self.joy_canvas.pack(side=tk.LEFT, padx=10)
        
        # 속도 게이지
        speed_subframe = ttk.Frame(joy_frame)
        speed_subframe.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=10)
        
        ttk.Label(speed_subframe, text="속도", font=("Arial", 12, "bold")).pack()
        self.speed_canvas = tk.Canvas(speed_subframe, width=150, height=150, bg="white")
        self.speed_canvas.pack(pady=10)
        
        self.speed_label = ttk.Label(speed_subframe, text="0%", font=("Arial", 16, "bold"))
        self.speed_label.pack()
        
        # 초음파 센서
        ultrasonic_frame = ttk.LabelFrame(right_panel, text="초음파 센서 (X버튼 눌러 측정)", padding="10")
        ultrasonic_frame.pack(fill=tk.X, pady=(0, 10))
        
        self.ultrasonic_canvas = tk.Canvas(ultrasonic_frame, width=380, height=120, bg="white")
        self.ultrasonic_canvas.pack()
        
        # 팬/틸트
        servo_frame = ttk.LabelFrame(right_panel, text="오른쪽 스틱 (팬/틸트)", padding="10")
        servo_frame.pack(fill=tk.X)
        
        pan_frame = ttk.Frame(servo_frame)
        pan_frame.pack(fill=tk.X, pady=5)
        ttk.Label(pan_frame, text="팬 (Pan):", width=12).pack(side=tk.LEFT)
        self.pan_bar = ttk.Progressbar(pan_frame, length=200, mode='determinate', maximum=180)
        self.pan_bar.pack(side=tk.LEFT, padx=5)
        self.pan_label = ttk.Label(pan_frame, text="90° (128)", width=12)
        self.pan_label.pack(side=tk.LEFT)
        
        tilt_frame = ttk.Frame(servo_frame)
        tilt_frame.pack(fill=tk.X, pady=5)
        ttk.Label(tilt_frame, text="틸트 (Tilt):", width=12).pack(side=tk.LEFT)
        self.tilt_bar = ttk.Progressbar(tilt_frame, length=200, mode='determinate', maximum=180)
        self.tilt_bar.pack(side=tk.LEFT, padx=5)
        self.tilt_label = ttk.Label(tilt_frame, text="90° (128)", width=12)
        self.tilt_label.pack(side=tk.LEFT)
        
        # 시리얼 로그
        log_frame = ttk.LabelFrame(right_panel, text="시스템 로그", padding="5")
        log_frame.pack(fill=tk.BOTH, expand=True, pady=10)
        
        self.log_text = tk.Text(log_frame, height=8, width=45, state=tk.DISABLED, 
                               bg="black", fg="lime", font=("Consolas", 9))
        log_scroll = ttk.Scrollbar(log_frame, orient=tk.VERTICAL, command=self.log_text.yview)
        self.log_text.config(yscrollcommand=log_scroll.set)
        self.log_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        log_scroll.pack(side=tk.RIGHT, fill=tk.Y)
        
        # UI 생성 완료 후 포트 검색
        self.refresh_ports()
    
    def log_message(self, msg):
        """로그 추가"""
        if not self.log_text:  # log_text가 아직 생성되지 않았으면 무시
            return
        
        self.log_text.config(state=tk.NORMAL)
        timestamp = datetime.now().strftime("%H:%M:%S")
        self.log_text.insert(tk.END, f"[{timestamp}] {msg}\n")
        self.log_text.see(tk.END)
        self.log_text.config(state=tk.DISABLED)
    
    def refresh_ports(self):
        """시리얼 포트 검색"""
        ports = [port.device for port in serial.tools.list_ports.comports()]
        self.port_combo['values'] = ports
        if ports:
            if 'COM9' in ports:
                self.port_combo.set('COM9')
            else:
                self.port_combo.current(0)
        self.log_message(f"포트 검색: {len(ports)}개 발견")
    
    # ========== 시리얼 통신 ==========
    def toggle_serial(self):
        if not self.serial_running:
            self.start_serial()
        else:
            self.stop_serial()
    
    def start_serial(self):
        port = self.port_combo.get()
        if not port:
            messagebox.showerror("오류", "시리얼 포트를 선택하세요")
            return
        
        try:
            self.serial_port = serial.Serial(port, 115200, timeout=0.1)
            self.serial_running = True
            self.serial_btn.config(text="연결 해제")
            self.serial_status.config(text="● 연결됨", foreground="green")
            self.log_message(f"시리얼 연결: {port}")
            
            self.serial_thread = threading.Thread(target=self.serial_reader, daemon=True)
            self.serial_thread.start()
            
        except Exception as e:
            messagebox.showerror("연결 오류", f"시리얼 연결 실패:\n{str(e)}")
            self.log_message(f"시리얼 오류: {str(e)}")
    
    def stop_serial(self):
        self.serial_running = False
        if self.serial_port:
            self.serial_port.close()
            self.serial_port = None
        self.serial_btn.config(text="연결")
        self.serial_status.config(text="● 끊김", foreground="red")
        self.log_message("시리얼 연결 해제")
    
    def serial_reader(self):
        """시리얼 데이터 읽기 - 초음파 센서 데이터 수신"""
        state = 'WAIT_HDR'
        d1 = 0
        
        while self.serial_running:
            try:
                if self.serial_port and self.serial_port.in_waiting > 0:
                    b = self.serial_port.read(1)[0]
                    
                    if state == 'WAIT_HDR':
                        if b == ord('D'):  # 'D' 헤더 감지
                            state = 'GET_D1'
                    elif state == 'GET_D1':
                        d1 = b  # 첫 번째 거리값
                        state = 'GET_D2'
                    elif state == 'GET_D2':
                        d2 = b  # 두 번째 거리값
                        # 초음파 센서 데이터 업데이트
                        self.ultrasonic_left = d1
                        self.ultrasonic_right = d2
                        self.log_message(f"[STM32] D,{d1},{d2} - 초음파: 좌={d1}cm, 우={d2}cm")
                        state = 'WAIT_HDR'
                else:
                    time.sleep(0.01)
            except Exception as e:
                self.log_message(f"시리얼 읽기 오류: {str(e)}")
                break
    
    def tx_cmd(self, cmd, val):
        """시리얼로 명령 전송"""
        if not self.serial_port or not self.serial_running:
            return
        
        try:
            if cmd == ' ':
                self.serial_port.write(cmd.encode())
                self.log_message("TX: STOP")
            else:
                self.serial_port.write(cmd.encode())
                self.serial_port.write(bytes([val]))
                self.log_message(f"TX: {cmd}, val={val}")
        except Exception as e:
            self.log_message(f"전송 오류: {str(e)}")
    
    # ========== 게임패드 ==========
    def toggle_gamepad(self):
        if not self.gamepad_running:
            self.start_gamepad()
        else:
            self.stop_gamepad()
    
    def start_gamepad(self):
        if not PYGAME_AVAILABLE:
            messagebox.showerror("오류", "pygame이 설치되지 않았습니다.\npip install pygame")
            return
        
        try:
            pygame.joystick.quit()
            pygame.joystick.init()
            
            if pygame.joystick.get_count() == 0:
                messagebox.showerror("오류", "게임패드가 연결되지 않았습니다")
                return
            
            self.gamepad = pygame.joystick.Joystick(0)
            self.gamepad.init()
            self.gamepad_running = True
            self.gamepad_btn.config(text="연결 해제")
            self.gamepad_status.config(text="● 연결됨", foreground="green")
            self.log_message(f"게임패드 연결: {self.gamepad.get_name()}")
            
            self.gamepad_thread = threading.Thread(target=self.gamepad_loop, daemon=True)
            self.gamepad_thread.start()
            
        except Exception as e:
            messagebox.showerror("연결 오류", f"게임패드 연결 실패:\n{str(e)}")
            self.log_message(f"게임패드 오류: {str(e)}")
    
    def stop_gamepad(self):
        self.gamepad_running = False
        if self.gamepad:
            self.gamepad.quit()
            self.gamepad = None
        self.gamepad_btn.config(text="연결")
        self.gamepad_status.config(text="● 끊김", foreground="red")
        self.log_message("게임패드 연결 해제")
    
    def map_axis_to_speed(self, v):
        """축 값을 속도(0~255)로 변환"""
        a = abs(v)
        if a < self.DEAD_ZONE:
            return 0
        if a > self.MAX_AXIS:
            a = self.MAX_AXIS
        return int((a * 255) / self.MAX_AXIS)
    
    def map_axis_to_u8_centered(self, v, invert=False):
        """축 값을 중앙 기준 0~255로 변환"""
        x = -v if invert else v
        if abs(x) < self.DEAD_ZONE:
            return 128
        if x > self.MAX_AXIS:
            x = self.MAX_AXIS
        if x < -self.MAX_AXIS:
            x = -self.MAX_AXIS
        val = (x * 127) // self.MAX_AXIS
        u8 = 128 + val
        return max(1, min(255, u8))
    
    def gamepad_loop(self):
        """게임패드 입력 처리 루프"""
        prev_buttons = set()
        
        while self.gamepad_running:
            try:
                pygame.event.pump()
                
                # 왼쪽 스틱 (주행)
                lx = int(self.gamepad.get_axis(0) * self.MAX_AXIS)
                ly = int(self.gamepad.get_axis(1) * -self.MAX_AXIS)  # Y축 반전
                
                self.joy_lx = lx
                self.joy_ly = ly
                
                # 방향 및 속도 계산
                if abs(lx) < self.DEAD_ZONE and abs(ly) < self.DEAD_ZONE:
                    cmd = ' '
                    spd = 0
                    self.joy_direction = 0
                    self.joy_value = 0
                else:
                    # 방향 결정 (상하좌우 우선)
                    if abs(ly) >= abs(lx):
                        spd = self.map_axis_to_speed(ly)
                        cmd = 'w' if ly > 0 else 's'
                        self.joy_direction = 90 if ly > 0 else 270
                    else:
                        spd = self.map_axis_to_speed(lx)
                        cmd = 'd' if lx > 0 else 'a'
                        self.joy_direction = 0 if lx > 0 else 180
                    
                    self.joy_value = int((spd / 255) * 100)
                    
                    if spd < 3:
                        cmd = ' '
                        spd = 0
                
                # 명령 전송 (변경시에만)
                if cmd != self.last_cmd or spd != self.last_spd:
                    self.tx_cmd(cmd, spd)
                    self.last_cmd = cmd
                    self.last_spd = spd
                
                # 오른쪽 스틱 (팬/틸트)
                rx = int(self.gamepad.get_axis(2) * self.MAX_AXIS)
                ry = int(self.gamepad.get_axis(3) * self.MAX_AXIS)
                
                self.joy_rx = rx
                self.joy_ry = ry
                
                pan = self.map_axis_to_u8_centered(rx, False)
                tilt = self.map_axis_to_u8_centered(ry, True)
                
                self.pan_value = pan
                self.tilt_value = tilt
                
                # 변화량 1 이상일 때만 전송
                if abs(pan - self.last_pan) >= 1:
                    self.tx_cmd('P', pan)
                    self.last_pan = pan
                
                if abs(tilt - self.last_tilt) >= 1:
                    self.tx_cmd('T', tilt)
                    self.last_tilt = tilt
                
                # 버튼 처리
                current_buttons = set()
                for i in range(self.gamepad.get_numbuttons()):
                    if self.gamepad.get_button(i):
                        current_buttons.add(i)
                
                # 버튼 눌림 감지 (엣지)
                pressed = current_buttons - prev_buttons
                
                # X 버튼 (2번) - 초음파 센서 요청
                if 2 in pressed:
                    if self.serial_port and self.serial_running:
                        self.tx_cmd('U', 0)
                        self.log_message("[게임패드] X버튼 - 초음파 센서 측정 요청")
                    else:
                        self.log_message("[게임패드] X버튼 - 시리얼 연결 필요!")
                
                # A 버튼 (0번) - 바탕화면에 캡처 저장
                if 0 in pressed:
                    self.root.after(0, self.save_capture_to_desktop)
                
                prev_buttons = current_buttons
                
                time.sleep(0.01)
                
            except Exception as e:
                self.log_message(f"게임패드 오류: {str(e)}")
                break
    
    # ========== 디스플레이 ==========
    def draw_joystick(self):
        self.joy_canvas.delete("all")
        cx, cy = 100, 100
        radius = 80
        
        self.joy_canvas.create_oval(cx-radius, cy-radius, cx+radius, cy+radius, outline="gray", width=2)
        self.joy_canvas.create_line(cx, cy-radius, cx, cy+radius, fill="lightgray", dash=(2, 2))
        self.joy_canvas.create_line(cx-radius, cy, cx+radius, cy, fill="lightgray", dash=(2, 2))
        
        self.joy_canvas.create_text(cx, cy-radius-15, text="↑", font=("Arial", 12))
        self.joy_canvas.create_text(cx, cy+radius+15, text="↓", font=("Arial", 12))
        self.joy_canvas.create_text(cx-radius-15, cy, text="←", font=("Arial", 12))
        self.joy_canvas.create_text(cx+radius+15, cy, text="→", font=("Arial", 12))
        
        if self.joy_value > 0:
            angle_rad = math.radians(self.joy_direction - 90)
            arrow_length = radius * 0.7
            end_x = cx + arrow_length * math.cos(angle_rad)
            end_y = cy + arrow_length * math.sin(angle_rad)
            self.joy_canvas.create_line(cx, cy, end_x, end_y, fill="blue", width=4, arrow=tk.LAST)
        
        if abs(self.joy_lx) > self.DEAD_ZONE or abs(self.joy_ly) > self.DEAD_ZONE:
            dot_x = cx + (self.joy_lx * radius / self.MAX_AXIS)
            dot_y = cy - (self.joy_ly * radius / self.MAX_AXIS)
            self.joy_canvas.create_oval(dot_x-5, dot_y-5, dot_x+5, dot_y+5, fill="red", outline="darkred", width=2)
        
        self.joy_canvas.create_text(cx, cy+radius+35, text=f"{self.joy_direction}°", font=("Arial", 10))
    
    def draw_speed_gauge(self):
        self.speed_canvas.delete("all")
        cx, cy = 75, 100
        radius = 60
        
        self.speed_canvas.create_arc(cx-radius, cy-radius, cx+radius, cy+radius, 
                                     start=0, extent=180, outline="lightgray", width=15, style=tk.ARC)
        
        speed_extent = (self.joy_value / 100) * 180
        self.speed_canvas.create_arc(cx-radius, cy-radius, cx+radius, cy+radius, 
                                     start=0, extent=speed_extent, outline="blue", width=15, style=tk.ARC)
        
        for i in range(0, 101, 25):
            angle = math.radians(i * 1.8)
            x1 = cx + (radius - 20) * math.cos(angle)
            y1 = cy - (radius - 20) * math.sin(angle)
            x2 = cx + (radius - 10) * math.cos(angle)
            y2 = cy - (radius - 10) * math.sin(angle)
            self.speed_canvas.create_line(x1, y1, x2, y2, fill="black", width=2)
            
            tx = cx + (radius - 30) * math.cos(angle)
            ty = cy - (radius - 30) * math.sin(angle)
            self.speed_canvas.create_text(tx, ty, text=str(i), font=("Arial", 8))
    
    def draw_ultrasonic_graph(self):
        self.ultrasonic_canvas.delete("all")
        
        left_height = min(self.ultrasonic_left * 2, 100)
        color_left = "green" if self.ultrasonic_left > 20 else "orange" if self.ultrasonic_left > 10 else "red"
        self.ultrasonic_canvas.create_rectangle(50, 110-left_height, 120, 110, fill=color_left, outline="black")
        self.ultrasonic_canvas.create_text(85, 115, text=f"좌: {self.ultrasonic_left}cm", font=("Arial", 10, "bold"))
        
        right_height = min(self.ultrasonic_right * 2, 100)
        color_right = "green" if self.ultrasonic_right > 20 else "orange" if self.ultrasonic_right > 10 else "red"
        self.ultrasonic_canvas.create_rectangle(260, 110-right_height, 330, 110, fill=color_right, outline="black")
        self.ultrasonic_canvas.create_text(295, 115, text=f"우: {self.ultrasonic_right}cm", font=("Arial", 10, "bold"))
        
        for i in range(0, 51, 10):
            y = 110 - i * 2
            self.ultrasonic_canvas.create_line(40, y, 340, y, fill="lightgray", dash=(2, 2))
            self.ultrasonic_canvas.create_text(20, y, text=f"{i}", font=("Arial", 8))
    
    def update_displays(self):
        self.draw_joystick()
        self.draw_speed_gauge()
        self.speed_label.config(text=f"{self.joy_value}%")
        self.draw_ultrasonic_graph()
        
        self.pan_angle = int((self.pan_value / 255) * 180)
        self.tilt_angle = int((self.tilt_value / 255) * 180)
        
        self.pan_bar['value'] = self.pan_angle
        self.pan_label.config(text=f"{self.pan_angle}° ({self.pan_value})")
        self.tilt_bar['value'] = self.tilt_angle
        self.tilt_label.config(text=f"{self.tilt_angle}° ({self.tilt_value})")
    
    def start_update_loop(self):
        self.update_displays()
        self.root.after(100, self.start_update_loop)
    
    # ========== 스트리밍 ==========
    def toggle_stream(self):
        if not self.is_streaming:
            self.start_stream()
        else:
            self.stop_stream()
    
    def start_stream(self):
        stream_url = self.url_entry.get().strip()
        
        if not stream_url:
            messagebox.showerror("오류", "스트림 URL을 입력하세요")
            return
        
        try:
            self.cap = cv2.VideoCapture(stream_url)
            
            if not self.cap.isOpened():
                messagebox.showerror("연결 오류", f"{stream_url}에 연결할 수 없습니다.")
                return
            
            self.is_streaming = True
            self.connect_btn.config(text="중지")
            self.capture_btn.config(state=tk.NORMAL)
            self.status_label.config(text="● 연결됨", foreground="green")
            self.log_message(f"스트림 시작: {stream_url}")
            
            self.stream_thread = threading.Thread(target=self.update_frame, daemon=True)
            self.stream_thread.start()
            
        except Exception as e:
            messagebox.showerror("오류", f"연결 실패: {str(e)}")
            self.log_message(f"스트림 오류: {str(e)}")
    
    def stop_stream(self):
        self.is_streaming = False
        
        if self.cap:
            self.cap.release()
            self.cap = None
        
        self.connect_btn.config(text="시작")
        self.capture_btn.config(state=tk.DISABLED)
        self.status_label.config(text="● 끊김", foreground="red")
        self.canvas.delete("all")
        self.log_message("스트림 중지")
    
    def update_frame(self):
        frame_delay = 1.0 / self.frame_rate
        
        while self.is_streaming:
            try:
                ret, frame = self.cap.read()
                
                if ret:
                    frame = cv2.resize(frame, (640, 480))
                    self.current_frame = frame.copy()
                    
                    frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                    img = Image.fromarray(frame_rgb)
                    photo = ImageTk.PhotoImage(image=img)
                    
                    self.canvas.delete("all")
                    self.canvas.create_image(320, 240, image=photo, anchor=tk.CENTER)
                    self.canvas.image = photo
                else:
                    self.root.after(0, self.connection_lost)
                    break
                
                time.sleep(frame_delay)
                
            except Exception as e:
                print(f"프레임 업데이트 오류: {e}")
                self.root.after(0, self.connection_lost)
                break
    
    def connection_lost(self):
        if self.is_streaming:
            self.stop_stream()
            messagebox.showwarning("연결 끊김", "ESP32-CAM과의 연결이 끊어졌습니다.")
    
    def capture_image(self):
        """사진 캡처 (파일 다이얼로그)"""
        if self.current_frame is not None:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = filedialog.asksaveasfilename(
                defaultextension=".jpg",
                initialfile=f"esp32cam_{timestamp}.jpg",
                filetypes=[("JPEG 이미지", "*.jpg"), ("모든 파일", "*.*")]
            )
            
            if filename:
                cv2.imwrite(filename, self.current_frame)
                messagebox.showinfo("저장 완료", f"이미지가 저장되었습니다:\n{filename}")
                self.log_message(f"이미지 저장: {filename}")
    
    def save_capture_to_desktop(self):
        """게임패드 A버튼 - 바탕화면에 자동 저장"""
        if self.current_frame is None:
            self.log_message("[게임패드] A버튼 - 캡처할 영상이 없습니다!")
            return
        
        try:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")[:-3]  # 밀리초 포함
            filename = os.path.join(self.capture_dir, f"shot_{timestamp}.jpg")
            
            cv2.imwrite(filename, self.current_frame)
            self.log_message(f"[게임패드] A버튼 - 캡처 저장: shot_{timestamp}.jpg")
            
        except Exception as e:
            self.log_message(f"[게임패드] 캡처 저장 실패: {str(e)}")

def main():
    root = tk.Tk()
    app = ESP32CamViewer(root)
    root.mainloop()

if __name__ == "__main__":
    main()