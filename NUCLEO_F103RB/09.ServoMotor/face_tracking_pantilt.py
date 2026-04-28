#!/usr/bin/env python3
"""
Face Tracking Pan-Tilt Camera System
=====================================
웹캠으로 얼굴을 감지하고 STM32F103 Pan-Tilt 서보를 제어하여
얼굴을 화면 중앙에 유지하는 프로그램

Requirements:
    pip install opencv-python pyserial

Usage:
    python face_tracking_pantilt.py --port COM3      # Windows
    python face_tracking_pantilt.py --port /dev/ttyUSB0  # Linux
"""

import cv2
import serial
import time
import argparse
from dataclasses import dataclass


@dataclass
class TrackingConfig:
    """추적 설정 파라미터"""
    # 시리얼 설정
    baudrate: int = 115200
    
    # 카메라 설정
    camera_id: int = 0
    frame_width: int = 640
    frame_height: int = 480
    
    # 추적 설정 (화면 중앙 기준 데드존)
    deadzone_x: int = 50  # 수평 데드존 (픽셀)
    deadzone_y: int = 40  # 수직 데드존 (픽셀)
    
    # 명령 전송 간격 (초)
    command_interval: float = 0.1
    
    # 얼굴 감지 설정
    min_face_size: int = 80  # 최소 얼굴 크기 (픽셀)
    scale_factor: float = 1.1
    min_neighbors: int = 5


class FaceTracker:
    """얼굴 추적 및 Pan-Tilt 제어 클래스"""
    
    def __init__(self, port: str, config: TrackingConfig = None):
        self.config = config or TrackingConfig()
        self.serial_port = None
        self.cap = None
        self.face_cascade = None
        self.last_command_time = 0
        
        # 시리얼 포트 초기화
        self._init_serial(port)
        
        # 카메라 초기화
        self._init_camera()
        
        # 얼굴 검출기 초기화
        self._init_face_detector()
        
        # 초기 위치로 이동
        self._send_command('i')
        time.sleep(0.5)
    
    def _init_serial(self, port: str):
        """시리얼 포트 초기화"""
        try:
            self.serial_port = serial.Serial(
                port=port,
                baudrate=self.config.baudrate,
                timeout=0.1
            )
            print(f"[OK] 시리얼 포트 연결: {port} @ {self.config.baudrate}bps")
            time.sleep(2)  # STM32 리셋 대기
        except serial.SerialException as e:
            raise RuntimeError(f"시리얼 포트 열기 실패: {e}")
    
    def _init_camera(self):
        """카메라 초기화"""
        self.cap = cv2.VideoCapture(self.config.camera_id)
        if not self.cap.isOpened():
            raise RuntimeError("카메라를 열 수 없습니다")
        
        self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, self.config.frame_width)
        self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, self.config.frame_height)
        
        # 실제 해상도 확인
        actual_width = int(self.cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        actual_height = int(self.cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
        print(f"[OK] 카메라 초기화: {actual_width}x{actual_height}")
        
        # 화면 중심점 계산
        self.center_x = actual_width // 2
        self.center_y = actual_height // 2
    
    def _init_face_detector(self):
        """얼굴 검출기 초기화"""
        cascade_path = cv2.data.haarcascades + 'haarcascade_frontalface_default.xml'
        self.face_cascade = cv2.CascadeClassifier(cascade_path)
        
        if self.face_cascade.empty():
            raise RuntimeError("Haar Cascade 파일 로드 실패")
        print("[OK] 얼굴 검출기 초기화 완료")
    
    def _send_command(self, cmd: str):
        """STM32에 명령 전송"""
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.write(cmd.encode())
            self.serial_port.flush()
            
            # 응답 읽기 (디버그용)
            time.sleep(0.05)
            if self.serial_port.in_waiting:
                response = self.serial_port.read(self.serial_port.in_waiting)
                print(f"  STM32: {response.decode(errors='ignore').strip()}")
    
    def _detect_faces(self, frame):
        """프레임에서 얼굴 검출"""
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        gray = cv2.equalizeHist(gray)  # 히스토그램 평활화
        
        faces = self.face_cascade.detectMultiScale(
            gray,
            scaleFactor=self.config.scale_factor,
            minNeighbors=self.config.min_neighbors,
            minSize=(self.config.min_face_size, self.config.min_face_size)
        )
        return faces
    
    def _get_largest_face(self, faces):
        """가장 큰 얼굴 선택 (가장 가까운 사람)"""
        if len(faces) == 0:
            return None
        
        # 면적 기준 정렬
        largest = max(faces, key=lambda f: f[2] * f[3])
        return largest
    
    def _calculate_error(self, face):
        """얼굴 중심과 화면 중심 간의 오차 계산"""
        x, y, w, h = face
        face_center_x = x + w // 2
        face_center_y = y + h // 2
        
        error_x = face_center_x - self.center_x
        error_y = face_center_y - self.center_y
        
        return error_x, error_y, face_center_x, face_center_y
    
    def _track_face(self, error_x: int, error_y: int):
        """오차에 따라 Pan-Tilt 명령 전송"""
        current_time = time.time()
        
        # 명령 전송 간격 체크
        if current_time - self.last_command_time < self.config.command_interval:
            return
        
        command_sent = False
        
        # 수평 제어 (Pan)
        # 얼굴이 오른쪽에 있으면 -> 카메라를 오른쪽으로 (d)
        # 얼굴이 왼쪽에 있으면 -> 카메라를 왼쪽으로 (a)
        if error_x > self.config.deadzone_x:
            self._send_command('d')  # 오른쪽으로
            print(f"[PAN] 오른쪽 이동 (error_x: {error_x})")
            command_sent = True
        elif error_x < -self.config.deadzone_x:
            self._send_command('a')  # 왼쪽으로
            print(f"[PAN] 왼쪽 이동 (error_x: {error_x})")
            command_sent = True
        
        # 수직 제어 (Tilt)
        # 얼굴이 아래에 있으면 -> 카메라를 아래로 (s)
        # 얼굴이 위에 있으면 -> 카메라를 위로 (w)
        if error_y > self.config.deadzone_y:
            self._send_command('s')  # 아래로
            print(f"[TILT] 아래 이동 (error_y: {error_y})")
            command_sent = True
        elif error_y < -self.config.deadzone_y:
            self._send_command('w')  # 위로
            print(f"[TILT] 위 이동 (error_y: {error_y})")
            command_sent = True
        
        if command_sent:
            self.last_command_time = current_time
    
    def _draw_overlay(self, frame, face, error_x, error_y, face_cx, face_cy):
        """화면에 추적 정보 오버레이"""
        x, y, w, h = face
        
        # 얼굴 박스
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
        
        # 얼굴 중심점
        cv2.circle(frame, (face_cx, face_cy), 5, (0, 255, 0), -1)
        
        # 화면 중심점
        cv2.circle(frame, (self.center_x, self.center_y), 5, (0, 0, 255), -1)
        
        # 중심 십자선
        cv2.line(frame, (self.center_x - 20, self.center_y), 
                 (self.center_x + 20, self.center_y), (0, 0, 255), 1)
        cv2.line(frame, (self.center_x, self.center_y - 20), 
                 (self.center_x, self.center_y + 20), (0, 0, 255), 1)
        
        # 데드존 표시
        dz_x, dz_y = self.config.deadzone_x, self.config.deadzone_y
        cv2.rectangle(frame, 
                      (self.center_x - dz_x, self.center_y - dz_y),
                      (self.center_x + dz_x, self.center_y + dz_y),
                      (255, 255, 0), 1)
        
        # 추적 선 (얼굴 중심 -> 화면 중심)
        cv2.line(frame, (face_cx, face_cy), 
                 (self.center_x, self.center_y), (255, 0, 255), 2)
        
        # 오차 정보 표시
        info_text = f"Error: X={error_x:+4d}, Y={error_y:+4d}"
        cv2.putText(frame, info_text, (10, 30), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)
        
        # 상태 표시
        if abs(error_x) <= self.config.deadzone_x and abs(error_y) <= self.config.deadzone_y:
            status = "LOCKED"
            color = (0, 255, 0)
        else:
            status = "TRACKING"
            color = (0, 165, 255)
        cv2.putText(frame, status, (10, 60), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, color, 2)
        
        return frame
    
    def _draw_no_face(self, frame):
        """얼굴 미검출 시 화면 표시"""
        cv2.putText(frame, "No Face Detected", (10, 30), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)
        
        # 중심 십자선
        cv2.line(frame, (self.center_x - 20, self.center_y), 
                 (self.center_x + 20, self.center_y), (0, 0, 255), 1)
        cv2.line(frame, (self.center_x, self.center_y - 20), 
                 (self.center_x, self.center_y + 20), (0, 0, 255), 1)
        
        return frame
    
    def run(self):
        """메인 추적 루프"""
        print("\n[시작] 얼굴 추적 시작...")
        print("  - 'q': 종료")
        print("  - 'c': 중앙으로 리셋")
        print("  - '+/-': 데드존 조절")
        print()
        
        try:
            while True:
                ret, frame = self.cap.read()
                if not ret:
                    print("[ERROR] 프레임 읽기 실패")
                    break
                
                # 좌우 반전 (거울 모드)
                frame = cv2.flip(frame, 1)
                
                # 얼굴 검출
                faces = self._detect_faces(frame)
                face = self._get_largest_face(faces)
                
                if face is not None:
                    # 오차 계산
                    error_x, error_y, face_cx, face_cy = self._calculate_error(face)
                    
                    # Pan-Tilt 제어
                    self._track_face(error_x, error_y)
                    
                    # 화면 표시
                    frame = self._draw_overlay(frame, face, error_x, error_y, face_cx, face_cy)
                else:
                    frame = self._draw_no_face(frame)
                
                # 조작 안내
                cv2.putText(frame, "Q:Quit  C:Center  +/-:Deadzone", 
                            (10, frame.shape[0] - 10),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (200, 200, 200), 1)
                
                cv2.imshow('Face Tracking Pan-Tilt', frame)
                
                # 키 입력 처리
                key = cv2.waitKey(1) & 0xFF
                if key == ord('q'):
                    break
                elif key == ord('c'):
                    print("[CENTER] 중앙으로 리셋")
                    self._send_command('i')
                elif key == ord('+') or key == ord('='):
                    self.config.deadzone_x += 5
                    self.config.deadzone_y += 5
                    print(f"[DEADZONE] {self.config.deadzone_x}, {self.config.deadzone_y}")
                elif key == ord('-'):
                    self.config.deadzone_x = max(10, self.config.deadzone_x - 5)
                    self.config.deadzone_y = max(10, self.config.deadzone_y - 5)
                    print(f"[DEADZONE] {self.config.deadzone_x}, {self.config.deadzone_y}")
                    
        except KeyboardInterrupt:
            print("\n[종료] 사용자 인터럽트")
        finally:
            self.cleanup()
    
    def cleanup(self):
        """리소스 정리"""
        print("[정리] 리소스 해제 중...")
        
        # 중앙으로 복귀
        if self.serial_port and self.serial_port.is_open:
            self._send_command('i')
            time.sleep(0.3)
            self.serial_port.close()
            print("  - 시리얼 포트 닫힘")
        
        if self.cap:
            self.cap.release()
            print("  - 카메라 해제")
        
        cv2.destroyAllWindows()
        print("[완료] 종료됨")


def main():
    parser = argparse.ArgumentParser(
        description='Face Tracking Pan-Tilt Camera System',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
    python face_tracking_pantilt.py --port COM3
    python face_tracking_pantilt.py --port /dev/ttyUSB0 --baudrate 9600
    python face_tracking_pantilt.py --port COM3 --camera 1 --deadzone 30
        """
    )
    
    parser.add_argument('--port', '-p', required=True,
                        help='시리얼 포트 (예: COM3, /dev/ttyUSB0)')
    parser.add_argument('--baudrate', '-b', type=int, default=115200,
                        help='보레이트 (기본: 115200)')
    parser.add_argument('--camera', '-c', type=int, default=0,
                        help='카메라 ID (기본: 0)')
    parser.add_argument('--deadzone', '-d', type=int, default=50,
                        help='데드존 크기 (기본: 50 픽셀)')
    parser.add_argument('--interval', '-i', type=float, default=0.1,
                        help='명령 전송 간격 (기본: 0.1초)')
    
    args = parser.parse_args()
    
    # 설정 생성
    config = TrackingConfig(
        baudrate=args.baudrate,
        camera_id=args.camera,
        deadzone_x=args.deadzone,
        deadzone_y=args.deadzone,
        command_interval=args.interval
    )
    
    print("=" * 50)
    print("  Face Tracking Pan-Tilt Camera System")
    print("=" * 50)
    print(f"  포트: {args.port}")
    print(f"  보레이트: {config.baudrate}")
    print(f"  카메라: {config.camera_id}")
    print(f"  데드존: {config.deadzone_x} px")
    print("=" * 50)
    print()
    
    try:
        tracker = FaceTracker(args.port, config)
        tracker.run()
    except RuntimeError as e:
        print(f"[ERROR] {e}")
        return 1
    except Exception as e:
        print(f"[ERROR] 예외 발생: {e}")
        import traceback
        traceback.print_exc()
        return 1
    
    return 0


if __name__ == '__main__':
    exit(main())
