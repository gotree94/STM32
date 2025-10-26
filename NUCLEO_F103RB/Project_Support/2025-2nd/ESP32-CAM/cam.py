import tkinter as tk
from tkinter import ttk, messagebox, filedialog
import cv2
from PIL import Image, ImageTk
import threading
import time
from datetime import datetime
import numpy as np

class ESP32CamViewer:
    def __init__(self, root):
        self.root = root
        self.root.title("ESP32-CAM 원격 뷰어")
        self.root.geometry("450x400")
        
        self.is_streaming = False
        self.stream_thread = None
        self.cap = None
        self.current_frame = None
        self.frame_rate = 5  # 5 FPS
        
        # UI 구성
        self.setup_ui()
        
    def setup_ui(self):
        # 상단 프레임 (연결 설정)
        top_frame = ttk.Frame(self.root, padding="10")
        top_frame.pack(fill=tk.X)
        
        # IP 주소 입력
        ttk.Label(top_frame, text="ESP32-CAM IP:").pack(side=tk.LEFT, padx=5)
        self.ip_entry = ttk.Entry(top_frame, width=15)
        self.ip_entry.insert(0, "192.168.0.100")  # 기본값
        self.ip_entry.pack(side=tk.LEFT, padx=5)
        
        # 포트 입력
        ttk.Label(top_frame, text="포트:").pack(side=tk.LEFT, padx=5)
        self.port_entry = ttk.Entry(top_frame, width=6)
        self.port_entry.insert(0, "80")
        self.port_entry.pack(side=tk.LEFT, padx=5)
        
        # 버튼 프레임
        btn_frame = ttk.Frame(self.root, padding="5")
        btn_frame.pack(fill=tk.X)
        
        # 연결 버튼
        self.connect_btn = ttk.Button(btn_frame, text="연결 시작", command=self.toggle_stream)
        self.connect_btn.pack(side=tk.LEFT, padx=5)
        
        # 캡처 버튼
        self.capture_btn = ttk.Button(btn_frame, text="사진 캡처", command=self.capture_image, state=tk.DISABLED)
        self.capture_btn.pack(side=tk.LEFT, padx=5)
        
        # 상태 표시
        self.status_label = ttk.Label(btn_frame, text="● 연결 안됨", foreground="red")
        self.status_label.pack(side=tk.LEFT, padx=10)
        
        # 비디오 프레임
        video_frame = ttk.Frame(self.root, padding="10")
        video_frame.pack(fill=tk.BOTH, expand=True)
        
        # 캔버스 (영상 표시) - 320x240
        self.canvas = tk.Canvas(video_frame, bg="black", width=320, height=240)
        self.canvas.pack()
        
        # 하단 정보 표시
        bottom_frame = ttk.Frame(self.root, padding="5")
        bottom_frame.pack(fill=tk.X)
        
        self.info_label = ttk.Label(bottom_frame, text="ESP32-CAM 연결 대기 중... (320x240 @ 5 FPS)")
    
    def toggle_stream(self):
        if not self.is_streaming:
            self.start_stream()
        else:
            self.stop_stream()
    
    def start_stream(self):
        ip = self.ip_entry.get().strip()
        port = self.port_entry.get().strip()
        
        if not ip:
            messagebox.showerror("오류", "IP 주소를 입력하세요")
            return
        
        # 스트림 URL 구성 (일반적인 ESP32-CAM 스트림 경로)
        stream_url = f"http://{ip}:{port}/stream"
        
        try:
            # OpenCV로 스트림 연결
            self.cap = cv2.VideoCapture(stream_url)
            
            if not self.cap.isOpened():
                messagebox.showerror("연결 오류", f"{stream_url}에 연결할 수 없습니다.")
                return
            
            self.is_streaming = True
            self.connect_btn.config(text="연결 중지")
            self.capture_btn.config(state=tk.NORMAL)
            self.status_label.config(text="● 연결됨", foreground="green")
            self.info_label.config(text=f"연결: {stream_url} (320x240 @ 5 FPS)")
            
            # 스트리밍 스레드 시작
            self.stream_thread = threading.Thread(target=self.update_frame, daemon=True)
            self.stream_thread.start()
            
        except Exception as e:
            messagebox.showerror("오류", f"연결 실패: {str(e)}")
    
    def stop_stream(self):
        self.is_streaming = False
        
        if self.cap:
            self.cap.release()
            self.cap = None
        
        self.connect_btn.config(text="연결 시작")
        self.capture_btn.config(state=tk.DISABLED)
        self.status_label.config(text="● 연결 안됨", foreground="red")
        self.info_label.config(text="ESP32-CAM 연결 대기 중... (320x240 @ 5 FPS)")
        
        # 캔버스 초기화
        self.canvas.delete("all")
    
    def update_frame(self):
        frame_delay = 1.0 / self.frame_rate  # 5 FPS = 0.2초 간격
        
        while self.is_streaming:
            try:
                ret, frame = self.cap.read()
                
                if ret:
                    # 320x240으로 리사이즈
                    frame = cv2.resize(frame, (320, 240))
                    self.current_frame = frame.copy()
                    
                    # BGR을 RGB로 변환
                    frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                    
                    # PIL Image로 변환
                    img = Image.fromarray(frame_rgb)
                    
                    # PhotoImage로 변환
                    photo = ImageTk.PhotoImage(image=img)
                    
                    # 캔버스에 이미지 표시
                    self.canvas.delete("all")
                    self.canvas.create_image(160, 120, image=photo, anchor=tk.CENTER)
                    self.canvas.image = photo  # 참조 유지
                else:
                    self.root.after(0, self.connection_lost)
                    break
                
                # 프레임 레이트 제어
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