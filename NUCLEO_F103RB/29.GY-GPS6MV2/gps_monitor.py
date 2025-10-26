#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
GY-GPS6MV2 GPS 모듈 모니터링 프로그램
GPS 데이터를 시리얼로 받아서 좌표, 위성 정보를 표시하고 지도에 시각화
"""

import serial
import pynmea2
import folium
from datetime import datetime
import threading
import time
import sys
import os

class GPSMonitor:
    def __init__(self, port='/dev/ttyUSB0', baudrate=9600):
        """
        GPS 모니터 초기화
        
        Args:
            port: 시리얼 포트 (Linux: /dev/ttyUSB0, Windows: COM3 등)
            baudrate: 통신 속도 (기본값: 9600)
        """
        self.port = port
        self.baudrate = baudrate
        self.ser = None
        self.running = False
        
        # GPS 데이터
        self.latitude = None
        self.longitude = None
        self.altitude = None
        self.satellites = 0
        self.fix_quality = 0
        self.timestamp = None
        self.speed = None
        self.course = None
        
        # 위성 정보
        self.satellites_info = []
        
        # 위치 기록
        self.position_history = []
        
    def connect(self):
        """시리얼 포트 연결"""
        try:
            self.ser = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=1,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE
            )
            print(f"✓ 시리얼 포트 연결 성공: {self.port} @ {self.baudrate} baud")
            return True
        except Exception as e:
            print(f"✗ 시리얼 포트 연결 실패: {e}")
            return False
    
    def disconnect(self):
        """시리얼 포트 연결 해제"""
        if self.ser and self.ser.is_open:
            self.ser.close()
            print("시리얼 포트 연결 해제됨")
    
    def parse_gps_data(self, line):
        """NMEA 문장 파싱"""
        try:
            if line.startswith('$GPGGA') or line.startswith('$GNGGA'):
                # GGA - GPS 고정 데이터
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
                # RMC - 권장 최소 GPS 데이터
                msg = pynmea2.parse(line)
                if msg.spd_over_grnd:
                    self.speed = msg.spd_over_grnd * 1.852  # knots to km/h
                if msg.true_course:
                    self.course = msg.true_course
                    
            elif line.startswith('$GPGSV') or line.startswith('$GNGSV'):
                # GSV - 위성 정보
                msg = pynmea2.parse(line)
                # 위성 정보 업데이트
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
                            'elevation': elevation,
                            'azimuth': azimuth,
                            'snr': snr
                        })
                        
        except pynmea2.ParseError as e:
            pass  # 파싱 에러는 무시
        except Exception as e:
            print(f"데이터 파싱 오류: {e}")
    
    def read_gps(self):
        """GPS 데이터 읽기 (스레드에서 실행)"""
        while self.running:
            try:
                if self.ser and self.ser.in_waiting > 0:
                    line = self.ser.readline().decode('ascii', errors='ignore').strip()
                    if line.startswith('$'):
                        self.parse_gps_data(line)
            except Exception as e:
                print(f"데이터 읽기 오류: {e}")
                time.sleep(0.1)
    
    def start(self):
        """GPS 모니터링 시작"""
        if not self.connect():
            return False
        
        self.running = True
        self.read_thread = threading.Thread(target=self.read_gps, daemon=True)
        self.read_thread.start()
        print("GPS 모니터링 시작...")
        return True
    
    def stop(self):
        """GPS 모니터링 중지"""
        self.running = False
        if hasattr(self, 'read_thread'):
            self.read_thread.join(timeout=2)
        self.disconnect()
    
    def display_status(self):
        """현재 GPS 상태 표시"""
        os.system('clear' if os.name == 'posix' else 'cls')
        
        print("=" * 70)
        print("GY-GPS6MV2 GPS 모듈 모니터".center(70))
        print("=" * 70)
        print()
        
        # GPS 고정 상태
        fix_status = {
            0: "GPS 신호 없음",
            1: "GPS 고정",
            2: "DGPS 고정",
            3: "PPS 고정",
            4: "RTK 고정",
            5: "Float RTK",
            6: "추측 항법"
        }
        
        print(f"📡 GPS 상태: {fix_status.get(self.fix_quality, '알 수 없음')}")
        print(f"🛰️  위성 개수: {self.satellites}개")
        print()
        
        # 좌표 정보
        if self.latitude and self.longitude:
            print(f"📍 위치 정보:")
            print(f"   위도: {self.latitude:.6f}°")
            print(f"   경도: {self.longitude:.6f}°")
            if self.altitude:
                print(f"   고도: {self.altitude:.1f}m")
            print()
        else:
            print("📍 위치 정보: GPS 신호 대기 중...")
            print()
        
        # 속도 및 방향
        if self.speed is not None:
            print(f"🚗 속도: {self.speed:.1f} km/h")
        if self.course is not None:
            print(f"🧭 방향: {self.course:.1f}°")
        
        if self.speed is not None or self.course is not None:
            print()
        
        # 시간
        if self.timestamp:
            print(f"🕐 GPS 시각: {self.timestamp} UTC")
            print()
        
        # 위성 정보
        if self.satellites_info:
            print(f"🛰️  위성 상세 정보 (총 {len(self.satellites_info)}개):")
            print("-" * 70)
            print(f"{'위성 번호':<10} {'고도각':<10} {'방위각':<10} {'신호강도(SNR)':<15}")
            print("-" * 70)
            
            for sat in self.satellites_info[:10]:  # 최대 10개만 표시
                prn = sat['prn'] or 'N/A'
                elev = f"{sat['elevation']}°" if sat['elevation'] else 'N/A'
                azim = f"{sat['azimuth']}°" if sat['azimuth'] else 'N/A'
                snr = f"{sat['snr']} dB" if sat['snr'] else 'N/A'
                print(f"{str(prn):<10} {elev:<10} {azim:<10} {snr:<15}")
            
            if len(self.satellites_info) > 10:
                print(f"... 외 {len(self.satellites_info) - 10}개 위성")
        
        print()
        print("=" * 70)
        print("종료하려면 Ctrl+C를 누르세요")
    
    def create_map(self, filename='gps_map.html'):
        """GPS 위치를 지도에 표시"""
        if not self.latitude or not self.longitude:
            print("❌ 지도 생성 실패: GPS 위치 데이터가 없습니다.")
            return None
        
        # 지도 생성 (현재 위치 중심)
        gps_map = folium.Map(
            location=[self.latitude, self.longitude],
            zoom_start=15,
            tiles='OpenStreetMap'
        )
        
        # 현재 위치 마커
        folium.Marker(
            [self.latitude, self.longitude],
            popup=f'현재 위치<br>위도: {self.latitude:.6f}<br>경도: {self.longitude:.6f}<br>고도: {self.altitude}m',
            tooltip='현재 위치',
            icon=folium.Icon(color='red', icon='info-sign')
        ).add_to(gps_map)
        
        # 이동 경로 표시
        if len(self.position_history) > 1:
            folium.PolyLine(
                self.position_history,
                color='blue',
                weight=3,
                opacity=0.7,
                popup='이동 경로'
            ).add_to(gps_map)
            
            # 시작점 마커
            folium.Marker(
                self.position_history[0],
                popup='시작점',
                icon=folium.Icon(color='green', icon='play')
            ).add_to(gps_map)
        
        # 위성 정보를 지도에 표시
        if self.satellites_info:
            sat_info_html = f"""
            <div style="position: fixed; 
                        top: 10px; 
                        right: 10px; 
                        width: 250px; 
                        background-color: white; 
                        border: 2px solid grey; 
                        z-index: 9999; 
                        padding: 10px;
                        border-radius: 5px;">
                <h4>GPS 정보</h4>
                <p><b>위성 개수:</b> {self.satellites}개</p>
                <p><b>고정 상태:</b> {'고정됨' if self.fix_quality > 0 else '신호 없음'}</p>
                <p><b>고도:</b> {self.altitude:.1f}m</p>
            </div>
            """
            gps_map.get_root().html.add_child(folium.Element(sat_info_html))
        
        # 지도 저장
        output_path = f'/mnt/user-data/outputs/{filename}'
        gps_map.save(output_path)
        print(f"✓ 지도가 생성되었습니다: {filename}")
        
        return output_path


def main():
    """메인 함수"""
    # 시리얼 포트 설정 (환경에 맞게 수정하세요)
    # Windows: 'COM3', 'COM4' 등
    # Linux: '/dev/ttyUSB0', '/dev/ttyAMA0' 등
    
    if sys.platform.startswith('win'):
        default_port = 'COM3'
    else:
        default_port = '/dev/ttyUSB0'
    
    print("GY-GPS6MV2 GPS 모듈 테스트 프로그램")
    print("=" * 50)
    
    port = input(f"시리얼 포트 입력 (기본값: {default_port}): ").strip() or default_port
    
    gps = GPSMonitor(port=port, baudrate=9600)
    
    if not gps.start():
        print("GPS 모니터 시작 실패")
        return
    
    try:
        # GPS 신호를 받을 때까지 대기
        print("\nGPS 신호를 수신하는 중...")
        print("(GPS 모듈을 야외나 창가에 배치하면 신호 수신이 더 좋습니다)")
        
        map_created = False
        last_display = time.time()
        
        while True:
            current_time = time.time()
            
            # 1초마다 화면 업데이트
            if current_time - last_display >= 1.0:
                gps.display_status()
                last_display = current_time
            
            # GPS 고정되면 10초마다 지도 업데이트
            if gps.latitude and gps.longitude and (current_time % 10 < 0.1):
                if not map_created or (current_time % 30 < 0.1):  # 30초마다 재생성
                    gps.create_map()
                    map_created = True
            
            time.sleep(0.1)
            
    except KeyboardInterrupt:
        print("\n\n프로그램을 종료합니다...")
        
        # 최종 지도 생성
        if gps.latitude and gps.longitude:
            print("\n최종 지도를 생성하는 중...")
            gps.create_map('gps_map_final.html')
        
        gps.stop()
        print("종료 완료")


if __name__ == "__main__":
    main()
