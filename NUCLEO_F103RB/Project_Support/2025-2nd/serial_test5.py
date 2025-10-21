import serial
import threading
import time
from collections import deque
import tkinter as tk
from tkinter import ttk
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure

class SerialRelayUI:
    def __init__(self, root, port_in='COM9', port_out='COM3', baudrate=115200):
        self.root = root
        self.root.title("시리얼 통신 모니터")
        self.root.geometry("1000x700")
        
        self.port_in = port_in
        self.port_out = port_out
        self.baudrate = baudrate
        self.running = False
        
        self.ser_in = None
        self.ser_out = None
        
        # 데이터 저장용 (최대 100개 포인트)
        self.max_points = 100
        self.distance1_data = deque(maxlen=self.max_points)
        self.distance2_data = deque(maxlen=self.max_points)
        self.time_data = deque(maxlen=self.max_points)
        self.start_time = time.time()
        
        # 현재 데이터
        self.current_distance1 = 0
        self.current_distance2 = 0
        self.current_alarm = 0
        self.current_direction = 'x'
        
        self.setup_ui()
        
    def setup_ui(self):
        # 상단 프레임: 연결 상태 및 제어
        top_frame = tk.Frame(self.root, bg='#f0f0f0', padx=10, pady=10)
        top_frame.pack(fill=tk.X)
        
        self.status_label = tk.Label(top_frame, text="연결 대기 중", 
                                     font=('Arial', 12, 'bold'), 
                                     bg='#f0f0f0', fg='red')
        self.status_label.pack(side=tk.LEFT, padx=10)
        
        self.start_button = tk.Button(top_frame, text="시작", 
                                      command=self.start, 
                                      bg='#4CAF50', fg='white',
                                      font=('Arial', 10, 'bold'),
                                      width=10)
        self.start_button.pack(side=tk.LEFT, padx=5)
        
        self.stop_button = tk.Button(top_frame, text="정지", 
                                     command=self.stop,
                                     bg='#f44336', fg='white',
                                     font=('Arial', 10, 'bold'),
                                     width=10, state=tk.DISABLED)
        self.stop_button.pack(side=tk.LEFT, padx=5)
        
        # 중앙 프레임: 데이터 표시 및 그래프
        center_frame = tk.Frame(self.root)
        center_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # 왼쪽: 데이터 표시
        left_frame = tk.Frame(center_frame, bg='white', relief=tk.RAISED, borderwidth=2)
        left_frame.pack(side=tk.LEFT, fill=tk.BOTH, padx=(0, 5))
        
        # Distance 1
        tk.Label(left_frame, text="Distance 1", 
                font=('Arial', 14, 'bold'), bg='white').pack(pady=(10, 5))
        self.distance1_label = tk.Label(left_frame, text="0", 
                                       font=('Arial', 32, 'bold'), 
                                       bg='white', fg='#2196F3')
        self.distance1_label.pack(pady=5)
        
        # Distance 2
        tk.Label(left_frame, text="Distance 2", 
                font=('Arial', 14, 'bold'), bg='white').pack(pady=(20, 5))
        self.distance2_label = tk.Label(left_frame, text="0", 
                                       font=('Arial', 32, 'bold'), 
                                       bg='white', fg='#FF9800')
        self.distance2_label.pack(pady=5)
        
        # 근접 알람
        tk.Label(left_frame, text="근접 알람", 
                font=('Arial', 14, 'bold'), bg='white').pack(pady=(20, 5))
        self.alarm_canvas = tk.Canvas(left_frame, width=100, height=100, 
                                     bg='white', highlightthickness=0)
        self.alarm_canvas.pack(pady=10)
        self.alarm_indicator = self.alarm_canvas.create_oval(10, 10, 90, 90, 
                                                             fill='green', outline='')
        
        # 방향 명령
        tk.Label(left_frame, text="방향 명령", 
                font=('Arial', 14, 'bold'), bg='white').pack(pady=(20, 10))
        
        direction_frame = tk.Frame(left_frame, bg='white')
        direction_frame.pack(pady=10)
        
        # 방향 버튼들
        self.btn_up = tk.Button(direction_frame, text="▲\n상(W)", 
                               width=8, height=3, font=('Arial', 10, 'bold'),
                               relief=tk.RAISED)
        self.btn_up.grid(row=0, column=1, padx=5, pady=5)
        
        self.btn_left = tk.Button(direction_frame, text="◄\n좌(A)", 
                                 width=8, height=3, font=('Arial', 10, 'bold'),
                                 relief=tk.RAISED)
        self.btn_left.grid(row=1, column=0, padx=5, pady=5)
        
        self.btn_stop = tk.Button(direction_frame, text="■\n정지(X)", 
                                 width=8, height=3, font=('Arial', 10, 'bold'),
                                 relief=tk.RAISED)
        self.btn_stop.grid(row=1, column=1, padx=5, pady=5)
        
        self.btn_right = tk.Button(direction_frame, text="►\n우(D)", 
                                  width=8, height=3, font=('Arial', 10, 'bold'),
                                  relief=tk.RAISED)
        self.btn_right.grid(row=1, column=2, padx=5, pady=5)
        
        self.btn_down = tk.Button(direction_frame, text="▼\n하(S)", 
                                 width=8, height=3, font=('Arial', 10, 'bold'),
                                 relief=tk.RAISED)
        self.btn_down.grid(row=2, column=1, padx=5, pady=5)
        
        self.direction_buttons = {
            'w': self.btn_up,
            'a': self.btn_left,
            's': self.btn_down,
            'd': self.btn_right,
            'x': self.btn_stop
        }
        
        # 오른쪽: 그래프
        right_frame = tk.Frame(center_frame)
        right_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        # Matplotlib 그래프
        self.fig = Figure(figsize=(6, 6), dpi=100)
        self.ax = self.fig.add_subplot(111)
        self.ax.set_xlabel('시간 (초)', fontsize=10)
        self.ax.set_ylabel('Distance', fontsize=10)
        self.ax.set_title('거리 센서 데이터', fontsize=12, fontweight='bold')
        self.ax.grid(True, alpha=0.3)
        
        self.line1, = self.ax.plot([], [], 'b-', linewidth=2, label='Distance 1')
        self.line2, = self.ax.plot([], [], 'orange', linewidth=2, label='Distance 2')
        self.ax.legend(loc='upper right')
        
        self.canvas = FigureCanvasTkAgg(self.fig, right_frame)
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)
        
    def open_ports(self):
        try:
            self.ser_in = serial.Serial(
                port=self.port_in,
                baudrate=self.baudrate,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=0.1
            )
            
            self.ser_out = serial.Serial(
                port=self.port_out,
                baudrate=self.baudrate,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=0.1
            )
            return True
        except serial.SerialException as e:
            self.status_label.config(text=f"포트 열기 실패: {e}", fg='red')
            return False
    
    def close_ports(self):
        if self.ser_in and self.ser_in.is_open:
            self.ser_in.close()
        if self.ser_out and self.ser_out.is_open:
            self.ser_out.close()
    
    def relay_and_monitor(self):
        buffer = bytearray()
        
        while self.running:
            try:
                # COM9에서 데이터 읽기
                if self.ser_in.in_waiting > 0:
                    data = self.ser_in.read(self.ser_in.in_waiting)
                    self.ser_out.write(data)  # COM3으로 중계
                
                # COM3에서 데이터 읽기 (모니터링)
                if self.ser_out.in_waiting > 0:
                    data = self.ser_out.read(self.ser_out.in_waiting)
                    buffer.extend(data)
                    
                    # "LOG" 문자열 찾기
                    while b'LOG' in buffer:
                        log_index = buffer.find(b'LOG')
                        
                        # LOG 이전 데이터 제거
                        buffer = buffer[log_index:]
                        
                        # LOG + 7바이트 데이터 확인
                        if len(buffer) >= 10:  # 'LOG' (3바이트) + 데이터 (7바이트)
                            packet = buffer[3:10]  # LOG 다음 7바이트
                            buffer = buffer[10:]  # 처리된 데이터 제거
                            
                            # 데이터 파싱
                            # 바이트0: distance1 상위(<<8), 바이트1: distance1 하위
                            distance1 = (packet[0] << 8) | packet[1]
                            # 바이트2: distance2 상위(<<8), 바이트3: distance2 하위
                            distance2 = (packet[2] << 8) | packet[3]
                            alarm = packet[4]
                            direction = chr(packet[5]) if packet[5] in [ord('w'), ord('a'), 
                                                                        ord('s'), ord('d'), 
                                                                        ord('x')] else 'x'
                            
                            # 로그 출력
                            print(f"LOG | D1:{distance1} D2:{distance2} Alarm:{alarm} Dir:{direction}")
                            
                            self.update_data(distance1, distance2, alarm, direction)
                        else:
                            break  # 데이터가 충분하지 않으면 다음에 처리
                        
            except Exception as e:
                print(f"에러: {e}")
                break
    
    def update_data(self, distance1, distance2, alarm, direction):
        self.current_distance1 = distance1
        self.current_distance2 = distance2
        self.current_alarm = alarm
        self.current_direction = direction
        
        # 시간 데이터 추가
        current_time = time.time() - self.start_time
        self.time_data.append(current_time)
        self.distance1_data.append(distance1)
        self.distance2_data.append(distance2)
        
        # UI 업데이트 (메인 스레드에서)
        self.root.after(0, self.update_ui)
    
    def update_ui(self):
        # Distance 값 업데이트
        self.distance1_label.config(text=str(self.current_distance1))
        self.distance2_label.config(text=str(self.current_distance2))
        
        # 알람 색상 변경
        alarm_color = 'red' if self.current_alarm == 1 else 'green'
        self.alarm_canvas.itemconfig(self.alarm_indicator, fill=alarm_color)
        
        # 방향 버튼 업데이트 (현재 방향만 눌린 상태로 표시)
        for key, button in self.direction_buttons.items():
            if key == self.current_direction:
                button.config(relief=tk.SUNKEN, bg='#4CAF50', fg='white')
            else:
                button.config(relief=tk.RAISED, bg='SystemButtonFace', fg='black')
        
        # 그래프 업데이트
        if len(self.time_data) > 0:
            self.line1.set_data(list(self.time_data), list(self.distance1_data))
            self.line2.set_data(list(self.time_data), list(self.distance2_data))
            
            self.ax.relim()
            self.ax.autoscale_view()
            self.canvas.draw()
    
    def start(self):
        if not self.open_ports():
            return
        
        self.running = True
        self.start_time = time.time()
        self.time_data.clear()
        self.distance1_data.clear()
        self.distance2_data.clear()
        
        # 스레드 시작
        threading.Thread(target=self.relay_and_monitor, daemon=True).start()
        
        self.status_label.config(text="연결됨 - 데이터 수신 중", fg='green')
        self.start_button.config(state=tk.DISABLED)
        self.stop_button.config(state=tk.NORMAL)
    
    def stop(self):
        self.running = False
        time.sleep(0.2)
        self.close_ports()
        
        self.status_label.config(text="연결 대기 중", fg='red')
        self.start_button.config(state=tk.NORMAL)
        self.stop_button.config(state=tk.DISABLED)


if __name__ == "__main__":
    root = tk.Tk()
    app = SerialRelayUI(root, port_in='COM9', port_out='COM3', baudrate=115200)
    root.mainloop()