#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
GY-GPS6MV2 GPS 모듈 GUI 모니터링 프로그램
tkinter를 사용한 그래픽 사용자 인터페이스
"""

import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext
import serial
import serial.tools.list_ports
import pynmea2
import folium
import webbrowser
import threading
import time
from datetime import datetime
import os
import math

class GPSMonitorGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("GY-GPS6MV2 GPS 모니터")
        self.root.geometry("1200x800")
        self.root.resizable(True, True)
        
        # GPS 데이터
        self.ser = None
        self.running = False
        self.latitude = None
        self.longitude = None
        self.altitude = None
        self.satellites = 0
        self.fix_quality = 0
        self.timestamp = None
        self.speed = None
        self.course = None
        self.satellites_info = []
        self.position_history = []
        
        # GUI 구성
        self.create_widgets()
        self.update_port_list()
        
    def create_widgets(self):
        """GUI 위젯 생성"""
        # 스타일 설정
        style = ttk.Style()
        style.configure('Title.TLabel', font=('Arial', 12, 'bold'))
        style.configure('Status.TLabel', font=('Arial', 10))
        style.configure('Data.TLabel', font=('Arial', 10))
        
        # 메인 프레임
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        main_frame.columnconfigure(1, weight=1)
        main_frame.rowconfigure(2, weight=1)
        
        # ==================== 연결 설정 프레임 ====================
        connection_frame = ttk.LabelFrame(main_frame, text="연결 설정", padding="10")
        connection_frame.grid(row=0, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=(0, 10))
        
        # 포트 선택
        ttk.Label(connection_frame, text="시리얼 포트:").grid(row=0, column=0, padx=5, sticky=tk.W)
        self.port_var = tk.StringVar()
        self.port_combo = ttk.Combobox(connection_frame, textvariable=self.port_var, width=20, state='readonly')
        self.port_combo.grid(row=0, column=1, padx=5)
        
        ttk.Button(connection_frame, text="새로고침", command=self.update_port_list).grid(row=0, column=2, padx=5)
        
        # Baudrate 선택
        ttk.Label(connection_frame, text="Baudrate:").grid(row=0, column=3, padx=5, sticky=tk.W)
        self.baudrate_var = tk.StringVar(value="9600")
        baudrate_combo = ttk.Combobox(connection_frame, textvariable=self.baudrate_var, 
                                       values=["4800", "9600", "19200", "38400", "57600", "115200"],
                                       width=10, state='readonly')
        baudrate_combo.grid(row=0, column=4, padx=5)
        
        # 연결/해제 버튼
        self.connect_btn = ttk.Button(connection_frame, text="연결", command=self.toggle_connection)
        self.connect_btn.grid(row=0, column=5, padx=10)
        
        # 연결 상태 표시
        self.status_label = ttk.Label(connection_frame, text="● 연결 안됨", foreground="red", style='Status.TLabel')
        self.status_label.grid(row=0, column=6, padx=5)
        
        # ==================== 왼쪽 패널: GPS 정보 ====================
        left_frame = ttk.Frame(main_frame)
        left_frame.grid(row=1, column=0, sticky=(tk.W, tk.E, tk.N, tk.S), padx=(0, 5))
        left_frame.rowconfigure(3, weight=1)
        
        # GPS 상태
        status_frame = ttk.LabelFrame(left_frame, text="GPS 상태", padding="10")
        status_frame.grid(row=0, column=0, sticky=(tk.W, tk.E), pady=(0, 5))
        
        ttk.Label(status_frame, text="고정 상태:", style='Data.TLabel').grid(row=0, column=0, sticky=tk.W, pady=2)
        self.fix_status_label = ttk.Label(status_frame, text="신호 없음", foreground="gray", style='Data.TLabel')
        self.fix_status_label.grid(row=0, column=1, sticky=tk.W, pady=2, padx=(10, 0))
        
        ttk.Label(status_frame, text="위성 개수:", style='Data.TLabel').grid(row=1, column=0, sticky=tk.W, pady=2)
        self.satellites_label = ttk.Label(status_frame, text="0개", style='Data.TLabel')
        self.satellites_label.grid(row=1, column=1, sticky=tk.W, pady=2, padx=(10, 0))
        
        ttk.Label(status_frame, text="GPS 시각:", style='Data.TLabel').grid(row=2, column=0, sticky=tk.W, pady=2)
        self.time_label = ttk.Label(status_frame, text="--:--:--", style='Data.TLabel')
        self.time_label.grid(row=2, column=1, sticky=tk.W, pady=2, padx=(10, 0))
        
        # 위치 정보
        position_frame = ttk.LabelFrame(left_frame, text="위치 정보", padding="10")
        position_frame.grid(row=1, column=0, sticky=(tk.W, tk.E), pady=(0, 5))
        
        ttk.Label(position_frame, text="위도:", style='Data.TLabel').grid(row=0, column=0, sticky=tk.W, pady=2)
        self.lat_label = ttk.Label(position_frame, text="--", style='Data.TLabel')
        self.lat_label.grid(row=0, column=1, sticky=tk.W, pady=2, padx=(10, 0))
        
        ttk.Label(position_frame, text="경도:", style='Data.TLabel').grid(row=1, column=0, sticky=tk.W, pady=2)
        self.lon_label = ttk.Label(position_frame, text="--", style='Data.TLabel')
        self.lon_label.grid(row=1, column=1, sticky=tk.W, pady=2, padx=(10, 0))
        
        ttk.Label(position_frame, text="고도:", style='Data.TLabel').grid(row=2, column=0, sticky=tk.W, pady=2)
        self.alt_label = ttk.Label(position_frame, text="--", style='Data.TLabel')
        self.alt_label.grid(row=2, column=1, sticky=tk.W, pady=2, padx=(10, 0))
        
        # 이동 정보
        movement_frame = ttk.LabelFrame(left_frame, text="이동 정보", padding="10")
        movement_frame.grid(row=2, column=0, sticky=(tk.W, tk.E), pady=(0, 5))
        
        ttk.Label(movement_frame, text="속도:", style='Data.TLabel').grid(row=0, column=0, sticky=tk.W, pady=2)
        self.speed_label = ttk.Label(movement_frame, text="-- km/h", style='Data.TLabel')
        self.speed_label.grid(row=0, column=1, sticky=tk.W, pady=2, padx=(10, 0))
        
        ttk.Label(movement_frame, text="방향:", style='Data.TLabel').grid(row=1, column=0, sticky=tk.W, pady=2)
        self.course_label = ttk.Label(movement_frame, text="--°", style='Data.TLabel')
        self.course_label.grid(row=1, column=1, sticky=tk.W, pady=2, padx=(10, 0))
        
        # 위성 정보 테이블
        satellites_frame = ttk.LabelFrame(left_frame, text="위성 상세 정보", padding="10")
        satellites_frame.grid(row=3, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        satellites_frame.rowconfigure(0, weight=1)
        satellites_frame.columnconfigure(0, weight=1)
        
        # 트리뷰 생성
        columns = ('prn', 'elevation', 'azimuth', 'snr')
        self.sat_tree = ttk.Treeview(satellites_frame, columns=columns, show='headings', height=10)
        
        self.sat_tree.heading('prn', text='위성 번호')
        self.sat_tree.heading('elevation', text='고도각')
        self.sat_tree.heading('azimuth', text='방위각')
        self.sat_tree.heading('snr', text='신호강도')
        
        self.sat_tree.column('prn', width=80, anchor=tk.CENTER)
        self.sat_tree.column('elevation', width=80, anchor=tk.CENTER)
        self.sat_tree.column('azimuth', width=80, anchor=tk.CENTER)
        self.sat_tree.column('snr', width=80, anchor=tk.CENTER)
        
        # 스크롤바
        sat_scrollbar = ttk.Scrollbar(satellites_frame, orient=tk.VERTICAL, command=self.sat_tree.yview)
        self.sat_tree.configure(yscroll=sat_scrollbar.set)
        
        self.sat_tree.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        sat_scrollbar.grid(row=0, column=1, sticky=(tk.N, tk.S))
        
        # ==================== 오른쪽 패널: NMEA 로그 및 제어 ====================
        right_frame = ttk.Frame(main_frame)
        right_frame.grid(row=1, column=1, sticky=(tk.W, tk.E, tk.N, tk.S), padx=(5, 0))
        right_frame.rowconfigure(1, weight=1)
        right_frame.columnconfigure(0, weight=1)
        
        # 제어 버튼들
        control_frame = ttk.Frame(right_frame)
        control_frame.grid(row=0, column=0, sticky=(tk.W, tk.E), pady=(0, 5))
        
        ttk.Button(control_frame, text="📍 지도에서 보기", command=self.show_on_map).pack(side=tk.LEFT, padx=5)
        ttk.Button(control_frame, text="🗑️ 로그 지우기", command=self.clear_log).pack(side=tk.LEFT, padx=5)
        ttk.Button(control_frame, text="💾 로그 저장", command=self.save_log).pack(side=tk.LEFT, padx=5)
        
        self.auto_scroll_var = tk.BooleanVar(value=True)
        ttk.Checkbutton(control_frame, text="자동 스크롤", variable=self.auto_scroll_var).pack(side=tk.LEFT, padx=20)
        
        # NMEA 데이터 로그
        log_frame = ttk.LabelFrame(right_frame, text="NMEA 데이터 로그", padding="10")
        log_frame.grid(row=1, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        log_frame.rowconfigure(0, weight=1)
        log_frame.columnconfigure(0, weight=1)
        
        self.log_text = scrolledtext.ScrolledText(log_frame, wrap=tk.WORD, height=20, width=60, 
                                                   font=('Courier', 9))
        self.log_text.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # 태그 설정 (색상)
        self.log_text.tag_config('gga', foreground='blue')
        self.log_text.tag_config('rmc', foreground='green')
        self.log_text.tag_config('gsv', foreground='purple')
        self.log_text.tag_config('error', foreground='red')
        
        # ==================== 하단: 통계 정보 ====================
        stats_frame = ttk.LabelFrame(main_frame, text="통계", padding="10")
        stats_frame.grid(row=2, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=(10, 0))
        
        ttk.Label(stats_frame, text="수신된 패킷:", style='Data.TLabel').grid(row=0, column=0, sticky=tk.W, padx=5)
        self.packet_count_label = ttk.Label(stats_frame, text="0", style='Data.TLabel')
        self.packet_count_label.grid(row=0, column=1, sticky=tk.W, padx=5)
        
        ttk.Label(stats_frame, text="파싱 오류:", style='Data.TLabel').grid(row=0, column=2, sticky=tk.W, padx=20)
        self.error_count_label = ttk.Label(stats_frame, text="0", style='Data.TLabel')
        self.error_count_label.grid(row=0, column=3, sticky=tk.W, padx=5)
        
        ttk.Label(stats_frame, text="실행 시간:", style='Data.TLabel').grid(row=0, column=4, sticky=tk.W, padx=20)
        self.uptime_label = ttk.Label(stats_frame, text="00:00:00", style='Data.TLabel')
        self.uptime_label.grid(row=0, column=5, sticky=tk.W, padx=5)
        
        # 통계 카운터
        self.packet_count = 0
        self.error_count = 0
        self.start_time = None
    
    def update_port_list(self):
        """시리얼 포트 목록 업데이트"""
        ports = serial.tools.list_ports.comports()
        port_list = [port.device for port in ports]
        
        self.port_combo['values'] = port_list
        if port_list:
            self.port_combo.current(0)
        else:
            messagebox.showwarning("경고", "사용 가능한 시리얼 포트가 없습니다.")
    
    def toggle_connection(self):
        """연결/해제 토글"""
        if not self.running:
            self.connect()
        else:
            self.disconnect()
    
    def connect(self):
        """시리얼 포트 연결"""
        port = self.port_var.get()
        if not port:
            messagebox.showerror("오류", "시리얼 포트를 선택하세요.")
            return
        
        try:
            baudrate = int(self.baudrate_var.get())
            self.ser = serial.Serial(
                port=port,
                baudrate=baudrate,
                timeout=1,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE
            )
            
            self.running = True
            self.start_time = time.time()
            
            # 읽기 스레드 시작
            self.read_thread = threading.Thread(target=self.read_gps, daemon=True)
            self.read_thread.start()
            
            # 업데이트 스레드 시작
            self.update_thread = threading.Thread(target=self.update_gui_loop, daemon=True)
            self.update_thread.start()
            
            # UI 업데이트
            self.status_label.config(text="● 연결됨", foreground="green")
            self.connect_btn.config(text="연결 해제")
            self.port_combo.config(state='disabled')
            
            self.log_message(f"시리얼 포트 연결: {port} @ {baudrate} baud\n", 'gga')
            
        except Exception as e:
            messagebox.showerror("연결 오류", f"시리얼 포트 연결 실패:\n{str(e)}")
    
    def disconnect(self):
        """시리얼 포트 연결 해제"""
        self.running = False
        
        if self.ser and self.ser.is_open:
            self.ser.close()
        
        # UI 업데이트
        self.status_label.config(text="● 연결 안됨", foreground="red")
        self.connect_btn.config(text="연결")
        self.port_combo.config(state='readonly')
        
        self.log_message("시리얼 포트 연결 해제\n", 'error')
    
    def read_gps(self):
        """GPS 데이터 읽기 (스레드)"""
        while self.running:
            try:
                if self.ser and self.ser.in_waiting > 0:
                    line = self.ser.readline().decode('ascii', errors='ignore').strip()
                    if line.startswith('$'):
                        self.packet_count += 1
                        self.parse_gps_data(line)
                        self.log_nmea_sentence(line)
            except Exception as e:
                self.error_count += 1
                self.log_message(f"읽기 오류: {str(e)}\n", 'error')
                time.sleep(0.1)
    
    def parse_gps_data(self, line):
        """NMEA 문장 파싱"""
        try:
            if line.startswith('$GPGGA') or line.startswith('$GNGGA'):
                msg = pynmea2.parse(line)
                self.latitude = msg.latitude
                self.longitude = msg.longitude
                self.altitude = msg.altitude
                self.satellites = msg.num_sats
                self.fix_quality = msg.gps_qual
                self.timestamp = msg.timestamp
                
                if self.latitude and self.longitude:
                    self.position_history.append((self.latitude, self.longitude))
                    if len(self.position_history) > 100:
                        self.position_history.pop(0)
                
            elif line.startswith('$GPRMC') or line.startswith('$GNRMC'):
                msg = pynmea2.parse(line)
                if msg.spd_over_grnd:
                    self.speed = msg.spd_over_grnd * 1.852
                if msg.true_course:
                    self.course = msg.true_course
                    
            elif line.startswith('$GPGSV') or line.startswith('$GNGSV'):
                msg = pynmea2.parse(line)
                if msg.msg_num == 1:
                    self.satellites_info = []
                
                for i in range(1, 5):
                    sat_num = getattr(msg, f'sv_prn_num_{i}', None)
                    elevation = getattr(msg, f'elevation_deg_{i}', None)
                    azimuth = getattr(msg, f'azimuth_{i}', None)
                    snr = getattr(msg, f'snr_{i}', None)
                    
                    if sat_num:
                        self.satellites_info.append({
                            'prn': sat_num,
                            'elevation': elevation if elevation else '--',
                            'azimuth': azimuth if azimuth else '--',
                            'snr': snr if snr else '--'
                        })
                        
        except pynmea2.ParseError:
            self.error_count += 1
        except Exception as e:
            self.error_count += 1
    
    def log_nmea_sentence(self, line):
        """NMEA 문장을 로그에 기록"""
        tag = None
        if 'GGA' in line:
            tag = 'gga'
        elif 'RMC' in line:
            tag = 'rmc'
        elif 'GSV' in line:
            tag = 'gsv'
        
        self.root.after(0, lambda: self.log_message(line + '\n', tag))
    
    def log_message(self, message, tag=None):
        """로그 메시지 추가"""
        self.log_text.insert(tk.END, message, tag)
        
        if self.auto_scroll_var.get():
            self.log_text.see(tk.END)
        
        # 로그 크기 제한 (최근 1000줄만 유지)
        lines = int(self.log_text.index('end-1c').split('.')[0])
        if lines > 1000:
            self.log_text.delete('1.0', '2.0')
    
    def update_gui_loop(self):
        """GUI 업데이트 루프 (스레드)"""
        while self.running:
            self.root.after(0, self.update_display)
            time.sleep(1)
    
    def update_display(self):
        """화면 정보 업데이트"""
        # GPS 상태
        fix_status = {
            0: ("신호 없음", "gray"),
            1: ("GPS 고정", "green"),
            2: ("DGPS 고정", "blue"),
            3: ("PPS 고정", "blue"),
            4: ("RTK 고정", "darkgreen"),
            5: ("Float RTK", "orange"),
            6: ("추측 항법", "purple")
        }
        
        status_text, status_color = fix_status.get(self.fix_quality, ("알 수 없음", "gray"))
        self.fix_status_label.config(text=status_text, foreground=status_color)
        
        self.satellites_label.config(text=f"{self.satellites}개")
        
        if self.timestamp:
            self.time_label.config(text=str(self.timestamp))
        
        # 위치 정보
        if self.latitude:
            self.lat_label.config(text=f"{self.latitude:.6f}°")
        else:
            self.lat_label.config(text="--")
        
        if self.longitude:
            self.lon_label.config(text=f"{self.longitude:.6f}°")
        else:
            self.lon_label.config(text="--")
        
        if self.altitude:
            self.alt_label.config(text=f"{self.altitude:.1f} m")
        else:
            self.alt_label.config(text="--")
        
        # 이동 정보
        if self.speed is not None:
            self.speed_label.config(text=f"{self.speed:.1f} km/h")
        else:
            self.speed_label.config(text="-- km/h")
        
        if self.course is not None:
            self.course_label.config(text=f"{self.course:.1f}°")
        else:
            self.course_label.config(text="--°")
        
        # 위성 정보 테이블 업데이트
        for item in self.sat_tree.get_children():
            self.sat_tree.delete(item)
        
        for sat in self.satellites_info:
            elev = f"{sat['elevation']}°" if sat['elevation'] != '--' else '--'
            azim = f"{sat['azimuth']}°" if sat['azimuth'] != '--' else '--'
            snr = f"{sat['snr']} dB" if sat['snr'] != '--' else '--'
            self.sat_tree.insert('', tk.END, values=(sat['prn'], elev, azim, snr))
        
        # 통계 업데이트
        self.packet_count_label.config(text=str(self.packet_count))
        self.error_count_label.config(text=str(self.error_count))
        
        if self.start_time:
            elapsed = int(time.time() - self.start_time)
            hours = elapsed // 3600
            minutes = (elapsed % 3600) // 60
            seconds = elapsed % 60
            self.uptime_label.config(text=f"{hours:02d}:{minutes:02d}:{seconds:02d}")
    
    def show_on_map(self):
        """지도에서 위치 보기"""
        if not self.latitude or not self.longitude:
            messagebox.showwarning("경고", "GPS 위치 데이터가 없습니다.\nGPS 신호를 먼저 수신하세요.")
            return
        
        try:
            # 지도 생성
            gps_map = folium.Map(
                location=[self.latitude, self.longitude],
                zoom_start=15,
                tiles='OpenStreetMap'
            )
            
            # 현재 위치 마커
            popup_html = f"""
            <b>현재 위치</b><br>
            위도: {self.latitude:.6f}°<br>
            경도: {self.longitude:.6f}°<br>
            고도: {self.altitude:.1f if self.altitude else '--'} m<br>
            위성: {self.satellites}개
            """
            
            folium.Marker(
                [self.latitude, self.longitude],
                popup=popup_html,
                tooltip='현재 위치',
                icon=folium.Icon(color='red', icon='info-sign')
            ).add_to(gps_map)
            
            # 이동 경로
            if len(self.position_history) > 1:
                folium.PolyLine(
                    self.position_history,
                    color='blue',
                    weight=3,
                    opacity=0.7,
                    popup='이동 경로'
                ).add_to(gps_map)
                
                folium.Marker(
                    self.position_history[0],
                    popup='시작점',
                    icon=folium.Icon(color='green', icon='play')
                ).add_to(gps_map)
            
            # 지도 저장
            map_file = 'gps_map.html'
            gps_map.save(map_file)
            
            # 브라우저에서 열기
            webbrowser.open(f'file://{os.path.abspath(map_file)}')
            
            messagebox.showinfo("성공", f"지도가 생성되어 브라우저에서 열렸습니다.\n파일: {map_file}")
            
        except Exception as e:
            messagebox.showerror("오류", f"지도 생성 실패:\n{str(e)}")
    
    def clear_log(self):
        """로그 지우기"""
        if messagebox.askyesno("확인", "로그를 모두 지우시겠습니까?"):
            self.log_text.delete('1.0', tk.END)
    
    def save_log(self):
        """로그 저장"""
        try:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = f"gps_log_{timestamp}.txt"
            
            with open(filename, 'w', encoding='utf-8') as f:
                f.write(self.log_text.get('1.0', tk.END))
            
            messagebox.showinfo("성공", f"로그가 저장되었습니다:\n{filename}")
        except Exception as e:
            messagebox.showerror("오류", f"로그 저장 실패:\n{str(e)}")
    
    def on_closing(self):
        """프로그램 종료 시"""
        if self.running:
            if messagebox.askokcancel("종료", "GPS 모니터를 종료하시겠습니까?"):
                self.disconnect()
                self.root.destroy()
        else:
            self.root.destroy()


def main():
    """메인 함수"""
    root = tk.Tk()
    app = GPSMonitorGUI(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()


if __name__ == "__main__":
    main()
