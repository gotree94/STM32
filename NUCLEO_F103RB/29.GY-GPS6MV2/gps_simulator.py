#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
GPS 시뮬레이터 - 테스트용
실제 GPS 모듈 없이 프로그램을 테스트할 수 있습니다.
"""

import time
import random
import math
from datetime import datetime

class GPSSimulator:
    def __init__(self):
        # 광주 시작 위치 (광주광역시 금남로)
        self.lat = 35.1595
        self.lon = 126.8526
        self.alt = 50.0
        self.speed = 0.0
        self.course = 0.0
        self.satellites = []
        
        # 시뮬레이션 파라미터
        self.time_step = 0.0
        
        # 위성 초기화
        self.init_satellites()
    
    def init_satellites(self):
        """위성 데이터 초기화"""
        # 12개의 가상 위성 생성
        for i in range(1, 13):
            self.satellites.append({
                'prn': i,
                'elevation': random.randint(15, 80),
                'azimuth': random.randint(0, 359),
                'snr': random.randint(20, 45)
            })
    
    def update_position(self):
        """위치 업데이트 (이동 시뮬레이션)"""
        # 원형 경로로 이동 시뮬레이션
        radius = 0.001  # 약 100m 반경
        self.time_step += 0.1
        
        # 원형 이동
        self.lat = 35.1595 + radius * math.sin(self.time_step * 0.1)
        self.lon = 126.8526 + radius * math.cos(self.time_step * 0.1)
        
        # 고도 변화
        self.alt = 50.0 + 5 * math.sin(self.time_step * 0.05)
        
        # 속도 (5-15 km/h)
        self.speed = 10 + 5 * math.sin(self.time_step * 0.2)
        
        # 방향
        self.course = (self.course + 2) % 360
        
        # 위성 신호 강도 변화
        for sat in self.satellites:
            sat['snr'] = max(15, min(45, sat['snr'] + random.randint(-2, 2)))
    
    def generate_gga(self):
        """GGA 문장 생성"""
        now = datetime.utcnow()
        time_str = now.strftime("%H%M%S.00")
        
        # 위도 변환 (도분 형식)
        lat_deg = int(abs(self.lat))
        lat_min = (abs(self.lat) - lat_deg) * 60
        lat_str = f"{lat_deg:02d}{lat_min:07.4f}"
        lat_ns = 'N' if self.lat >= 0 else 'S'
        
        # 경도 변환 (도분 형식)
        lon_deg = int(abs(self.lon))
        lon_min = (abs(self.lon) - lon_deg) * 60
        lon_str = f"{lon_deg:03d}{lon_min:07.4f}"
        lon_ew = 'E' if self.lon >= 0 else 'W'
        
        num_sats = len([s for s in self.satellites if s['snr'] > 20])
        
        gga = (f"$GPGGA,{time_str},{lat_str},{lat_ns},{lon_str},{lon_ew},"
               f"1,{num_sats:02d},1.0,{self.alt:.1f},M,0.0,M,,")
        
        # 체크섬 계산
        checksum = 0
        for char in gga[1:]:
            checksum ^= ord(char)
        
        return f"{gga}*{checksum:02X}\r\n"
    
    def generate_rmc(self):
        """RMC 문장 생성"""
        now = datetime.utcnow()
        time_str = now.strftime("%H%M%S.00")
        date_str = now.strftime("%d%m%y")
        
        # 위도 변환
        lat_deg = int(abs(self.lat))
        lat_min = (abs(self.lat) - lat_deg) * 60
        lat_str = f"{lat_deg:02d}{lat_min:07.4f}"
        lat_ns = 'N' if self.lat >= 0 else 'S'
        
        # 경도 변환
        lon_deg = int(abs(self.lon))
        lon_min = (abs(self.lon) - lon_deg) * 60
        lon_str = f"{lon_deg:03d}{lon_min:07.4f}"
        lon_ew = 'E' if self.lon >= 0 else 'W'
        
        # 속도를 knots로 변환
        speed_knots = self.speed / 1.852
        
        rmc = (f"$GPRMC,{time_str},A,{lat_str},{lat_ns},{lon_str},{lon_ew},"
               f"{speed_knots:.1f},{self.course:.1f},{date_str},,")
        
        # 체크섬 계산
        checksum = 0
        for char in rmc[1:]:
            checksum ^= ord(char)
        
        return f"{rmc}*{checksum:02X}\r\n"
    
    def generate_gsv(self):
        """GSV 문장 생성"""
        messages = []
        total_sats = len(self.satellites)
        total_msgs = (total_sats + 3) // 4
        
        for msg_num in range(1, total_msgs + 1):
            start_idx = (msg_num - 1) * 4
            end_idx = min(start_idx + 4, total_sats)
            sats_in_msg = self.satellites[start_idx:end_idx]
            
            gsv = f"$GPGSV,{total_msgs},{msg_num},{total_sats:02d}"
            
            for sat in sats_in_msg:
                gsv += f",{sat['prn']:02d},{sat['elevation']:02d},{sat['azimuth']:03d},{sat['snr']:02d}"
            
            # 체크섬 계산
            checksum = 0
            for char in gsv[1:]:
                checksum ^= ord(char)
            
            messages.append(f"{gsv}*{checksum:02X}\r\n")
        
        return messages
    
    def get_nmea_sentences(self):
        """모든 NMEA 문장 생성"""
        self.update_position()
        
        sentences = []
        sentences.append(self.generate_gga())
        sentences.append(self.generate_rmc())
        sentences.extend(self.generate_gsv())
        
        return sentences


def main():
    """시뮬레이터 실행"""
    print("GPS 시뮬레이터 시작...")
    print("(Ctrl+C로 중지)")
    print()
    
    simulator = GPSSimulator()
    
    try:
        while True:
            sentences = simulator.get_nmea_sentences()
            
            for sentence in sentences:
                print(sentence.strip())
                time.sleep(0.1)  # 100ms 간격
            
            time.sleep(0.9)  # 총 1초 주기
            
    except KeyboardInterrupt:
        print("\n시뮬레이터 종료")


if __name__ == "__main__":
    main()
