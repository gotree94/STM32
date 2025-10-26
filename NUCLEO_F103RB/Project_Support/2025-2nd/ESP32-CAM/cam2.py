import tkinter as tk
from tkinter import ttk, messagebox, filedialog
import cv2
from PIL import Image, ImageTk
import threading
import time
from datetime import datetime
import math

class ESP32CamViewer:
    def __init__(self, root):
        self.root = root
        self.root.title("ESP32-CAM 원격 제어 뷰어")
        self.root.geometry("1100x720")
        self.root.resizable(False, False)
        
        self.is_streaming = False
        self.stream_thread = None
        self.cap = None
        self.current_frame = None
        self.frame_rate = 5
        
        # 조이스틱 데이터
        self.joy_direction = 0  # 0~360도
        self.joy_value = 0  # 0~100%
        
        # 초음파 센서 데이터
        self.ultrasonic_left = 0  # cm
        self.ultrasonic_right = 0  # cm
        
        # 팬/틸트 각도
        self.pan_angle = 90  # 0~180도
        self.tilt_angle = 90  # 0~180도
        
        self.setup_ui()
        self.start_update_loop()
        
    def setup_ui(self):
        # 메인 컨테이너
        main_container = ttk.Frame(self.root)
        main_container.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # 왼쪽 패널 (영상)
        left_panel = ttk.Frame(main_container)
        left_panel.pack(side=tk.LEFT, fill=tk.BOTH, padx=(0, 10))
        
        # URL 입력 프레임
        url_frame = ttk.LabelFrame(left_panel, text="연결 설정", padding="10")
        url_frame.pack(fill=tk.X, pady=(0, 10))
        
        ttk.Label(url_frame, text="스트림 URL:").pack(side=tk.LEFT, padx=5)
        self.url_entry = ttk.Entry(url_frame, width=35)
        self.url_entry.insert(0, "http://192.168.0.21:81/stream")
        self.url_entry.pack(side=tk.LEFT, padx=5)
        
        self.connect_btn = ttk.Button(url_frame, text="연결", command=self.toggle_stream, width=10)
        self.connect_btn.pack(side=tk.LEFT, padx=5)
        
        self.status_label = ttk.Label(url_frame, text="● 연결 안됨", foreground="red")
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
        
        # 오른쪽 패널 (제어 정보)
        right_panel = ttk.Frame(main_container, width=400)
        right_panel.pack(side=tk.RIGHT, fill=tk.BOTH)
        
        # 조이스틱 방향 표시
        joy_frame = ttk.LabelFrame(right_panel, text="조이스틱 입력", padding="10")
        joy_frame.pack(fill=tk.X, pady=(0, 10))
        
        self.joy_canvas = tk.Canvas(joy_frame, width=200, height=200, bg="white")
        self.joy_canvas.pack(side=tk.LEFT, padx=10)
        self.draw_joystick()
        
        # 속도 게이지
        speed_subframe = ttk.Frame(joy_frame)
        speed_subframe.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=10)
        
        ttk.Label(speed_subframe, text="속도", font=("Arial", 12, "bold")).pack()
        self.speed_canvas = tk.Canvas(speed_subframe, width=150, height=150, bg="white")
        self.speed_canvas.pack(pady=10)
        self.draw_speed_gauge()
        
        self.speed_label = ttk.Label(speed_subframe, text="0%", font=("Arial", 16, "bold"))
        self.speed_label.pack()
        
        # 초음파 센서 거리 표시
        ultrasonic_frame = ttk.LabelFrame(right_panel, text="초음파 센서 (거리)", padding="10")
        ultrasonic_frame.pack(fill=tk.X, pady=(0, 10))
        
        self.ultrasonic_canvas = tk.Canvas(ultrasonic_frame, width=380, height=120, bg="white")
        self.ultrasonic_canvas.pack()
        self.draw_ultrasonic_graph()
        
        # 팬/틸트 각도 표시
        servo_frame = ttk.LabelFrame(right_panel, text="카메라 팬/틸트 각도", padding="10")
        servo_frame.pack(fill=tk.X)
        
        # 팬 각도
        pan_frame = ttk.Frame(servo_frame)
        pan_frame.pack(fill=tk.X, pady=5)
        ttk.Label(pan_frame, text="팬 (Pan):", width=12).pack(side=tk.LEFT)
        self.pan_bar = ttk.Progressbar(pan_frame, length=200, mode='determinate', maximum=180)
        self.pan_bar.pack(side=tk.LEFT, padx=5)
        self.pan_label = ttk.Label(pan_frame, text="90°", width=8)
        self.pan_label.pack(side=tk.LEFT)
        
        # 틸트 각도
        tilt_frame = ttk.Frame(servo_frame)
        tilt_frame.pack(fill=tk.X, pady=5)
        ttk.Label(tilt_frame, text="틸트 (Tilt):", width=12).pack(side=tk.LEFT)
        self.tilt_bar = ttk.Progressbar(tilt_frame, length=200, mode='determinate', maximum=180)
        self.tilt_bar.pack(side=tk.LEFT, padx=5)
        self.tilt_label = ttk.Label(tilt_frame, text="90°", width=8)
        self.tilt_label.pack(side=tk.LEFT)
        
        # 테스트 버튼들
        test_frame = ttk.LabelFrame(right_panel, text="테스트 (임시)", padding="10")
        test_frame.pack(fill=tk.X, pady=10)
        
        btn_row1 = ttk.Frame(test_frame)
        btn_row1.pack(fill=tk.X, pady=2)
        ttk.Button(btn_row1, text="↑", command=lambda: self.test_joystick(90, 50)).pack(side=tk.LEFT, padx=2)
        ttk.Button(btn_row1, text="↓", command=lambda: self.test_joystick(270, 50)).pack(side=tk.LEFT, padx=2)
        ttk.Button(btn_row1, text="←", command=lambda: self.test_joystick(180, 50)).pack(side=tk.LEFT, padx=2)
        ttk.Button(btn_row1, text="→", command=lambda: self.test_joystick(0, 50)).pack(side=tk.LEFT, padx=2)
        
        btn_row2 = ttk.Frame(test_frame)
        btn_row2.pack(fill=tk.X, pady=2)
        ttk.Button(btn_row2, text="거리 테스트", command=self.test_ultrasonic).pack(side=tk.LEFT, padx=2)
        ttk.Button(btn_row2, text="각도 테스트", command=self.test_servo).pack(side=tk.LEFT, padx=2)
    
    def draw_joystick(self):
        # 조이스틱 원형 표시
        self.joy_canvas.delete("all")
        cx, cy = 100, 100
        radius = 80
        
        # 외곽 원
        self.joy_canvas.create_oval(cx-radius, cy-radius, cx+radius, cy+radius, outline="gray", width=2)
        
        # 십자선
        self.joy_canvas.create_line(cx, cy-radius, cx, cy+radius, fill="lightgray", dash=(2, 2))
        self.joy_canvas.create_line(cx-radius, cy, cx+radius, cy, fill="lightgray", dash=(2, 2))
        
        # 방향 텍스트
        self.joy_canvas.create_text(cx, cy-radius-15, text="↑", font=("Arial", 12))
        self.joy_canvas.create_text(cx, cy+radius+15, text="↓", font=("Arial", 12))
        self.joy_canvas.create_text(cx-radius-15, cy, text="←", font=("Arial", 12))
        self.joy_canvas.create_text(cx+radius+15, cy, text="→", font=("Arial", 12))
        
        # 방향 표시 (화살표)
        angle_rad = math.radians(self.joy_direction - 90)  # 12시 방향을 0도로
        arrow_length = radius * 0.7
        end_x = cx + arrow_length * math.cos(angle_rad)
        end_y = cy + arrow_length * math.sin(angle_rad)
        
        self.joy_canvas.create_line(cx, cy, end_x, end_y, fill="blue", width=4, arrow=tk.LAST)
        self.joy_canvas.create_text(cx, cy+radius+35, text=f"{self.joy_direction}°", font=("Arial", 10))
    
    def draw_speed_gauge(self):
        # 속도 게이지 (반원형)
        self.speed_canvas.delete("all")
        cx, cy = 75, 100
        radius = 60
        
        # 배경 호 (회색)
        self.speed_canvas.create_arc(cx-radius, cy-radius, cx+radius, cy+radius, 
                                     start=0, extent=180, outline="lightgray", width=15, style=tk.ARC)
        
        # 속도 호 (파란색)
        speed_extent = (self.joy_value / 100) * 180
        self.speed_canvas.create_arc(cx-radius, cy-radius, cx+radius, cy+radius, 
                                     start=0, extent=speed_extent, outline="blue", width=15, style=tk.ARC)
        
        # 눈금 표시
        for i in range(0, 101, 25):
            angle = math.radians(i * 1.8)
            x1 = cx + (radius - 20) * math.cos(angle)
            y1 = cy - (radius - 20) * math.sin(angle)
            x2 = cx + (radius - 10) * math.cos(angle)
            y2 = cy - (radius - 10) * math.sin(angle)
            self.speed_canvas.create_line(x1, y1, x2, y2, fill="black", width=2)
            
            # 숫자
            tx = cx + (radius - 30) * math.cos(angle)
            ty = cy - (radius - 30) * math.sin(angle)
            self.speed_canvas.create_text(tx, ty, text=str(i), font=("Arial", 8))
    
    def draw_ultrasonic_graph(self):
        # 초음파 센서 거리 그래프 (막대 그래프)
        self.ultrasonic_canvas.delete("all")
        
        # 왼쪽 센서
        left_height = min(self.ultrasonic_left * 2, 100)  # 최대 50cm
        self.ultrasonic_canvas.create_rectangle(50, 110-left_height, 120, 110, fill="green", outline="black")
        self.ultrasonic_canvas.create_text(85, 115, text=f"좌: {self.ultrasonic_left}cm", font=("Arial", 10, "bold"))
        
        # 오른쪽 센서
        right_height = min(self.ultrasonic_right * 2, 100)
        self.ultrasonic_canvas.create_rectangle(260, 110-right_height, 330, 110, fill="orange", outline="black")
        self.ultrasonic_canvas.create_text(295, 115, text=f"우: {self.ultrasonic_right}cm", font=("Arial", 10, "bold"))
        
        # 눈금선
        for i in range(0, 51, 10):
            y = 110 - i * 2
            self.ultrasonic_canvas.create_line(40, y, 340, y, fill="lightgray", dash=(2, 2))
            self.ultrasonic_canvas.create_text(20, y, text=f"{i}", font=("Arial", 8))
    
    def update_displays(self):
        # 모든 디스플레이 업데이트
        self.draw_joystick()
        self.draw_speed_gauge()
        self.speed_label.config(text=f"{self.joy_value}%")
        self.draw_ultrasonic_graph()
        
        # 팬/틸트 바 업데이트
        self.pan_bar['value'] = self.pan_angle
        self.pan_label.config(text=f"{self.pan_angle}°")
        self.tilt_bar['value'] = self.tilt_angle
        self.tilt_label.config(text=f"{self.tilt_angle}°")
    
    def start_update_loop(self):
        # 주기적으로 디스플레이 업데이트
        self.update_displays()
        self.root.after(100, self.start_update_loop)
    
    # 테스트 함수들
    def test_joystick(self, direction, value):
        self.joy_direction = direction
        self.joy_value = value
    
    def test_ultrasonic(self):
        import random
        self.ultrasonic_left = random.randint(5, 50)
        self.ultrasonic_right = random.randint(5, 50)
    
    def test_servo(self):
        import random
        self.pan_angle = random.randint(0, 180)
        self.tilt_angle = random.randint(0, 180)
    
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
            self.connect_btn.config(text="연결 중지")
            self.capture_btn.config(state=tk.NORMAL)
            self.status_label.config(text="● 연결됨", foreground="green")
            
            self.stream_thread = threading.Thread(target=self.update_frame, daemon=True)
            self.stream_thread.start()
            
        except Exception as e:
            messagebox.showerror("오류", f"연결 실패: {str(e)}")
    
    def stop_stream(self):
        self.is_streaming = False
        
        if self.cap:
            self.cap.release()
            self.cap = None
        
        self.connect_btn.config(text="연결")
        self.capture_btn.config(state=tk.DISABLED)
        self.status_label.config(text="● 연결 안됨", foreground="red")
        self.canvas.delete("all")
    
    def update_frame(self):
        frame_delay = 1.0 / self.frame_rate
        
        while self.is_streaming:
            try:
                ret, frame = self.cap.read()
                
                if ret:
                    # 640x480으로 리사이즈
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

def main():
    root = tk.Tk()
    app = ESP32CamViewer(root)
    root.mainloop()

if __name__ == "__main__":
    main()