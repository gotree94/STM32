import os
import sys
from PIL import Image
import cv2

def convert_mp4_to_gif(input_path, output_path=None, fps=10, width=480, quality="medium"):
    """
    MP4 파일을 애니메이션 GIF로 변환합니다.
    
    Args:
        input_path: 입력 MP4 파일 경로
        output_path: 출력 GIF 파일 경로
        fps: 프레임 레이트 (기본값: 10)
        width: 출력 너비 (기본값: 480)
        quality: "low", "medium", "high"
    """
    if not os.path.exists(input_path):
        raise FileNotFoundError(f"입력 파일을 찾을 수 없습니다: {input_path}")
    
    if output_path is None:
        base = os.path.splitext(input_path)[0]
        output_path = f"{base}.gif"
    
    # 품질에 따른 색상 수 설정
    color_settings = {"low": 64, "medium": 128, "high": 256}
    colors = color_settings.get(quality, 128)
    
    # 비디오 열기
    cap = cv2.VideoCapture(input_path)
    if not cap.isOpened():
        raise IOError(f"비디오를 열 수 없습니다: {input_path}")
    
    # 비디오 정보
    original_fps = cap.get(cv2.CAP_PROP_FPS)
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    original_width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    original_height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    
    # 출력 크기 계산
    ratio = width / original_width
    height = int(original_height * ratio)
    
    # 프레임 샘플링 간격
    frame_interval = max(1, int(original_fps / fps))
    
    print(f"변환 중...")
    print(f"  원본: {original_width}x{original_height}, {original_fps:.1f}fps, {total_frames}프레임")
    print(f"  출력: {width}x{height}, {fps}fps")
    
    frames = []
    frame_count = 0
    
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        
        # 프레임 샘플링
        if frame_count % frame_interval == 0:
            # BGR -> RGB 변환
            frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            # 리사이즈
            frame_resized = cv2.resize(frame_rgb, (width, height), interpolation=cv2.INTER_LANCZOS4)
            # PIL Image로 변환
            img = Image.fromarray(frame_resized)
            # 팔레트 모드로 변환 (색상 수 제한)
            img = img.quantize(colors=colors, method=Image.Quantize.MEDIANCUT)
            frames.append(img)
        
        frame_count += 1
        
        # 진행률 표시
        if frame_count % 100 == 0:
            progress = (frame_count / total_frames) * 100
            print(f"  진행: {progress:.1f}%", end='\r')
    
    cap.release()
    
    if not frames:
        raise ValueError("추출된 프레임이 없습니다")
    
    print(f"\n  {len(frames)}개 프레임 추출 완료, GIF 저장 중...")
    
    # GIF 저장
    duration = int(1000 / fps)  # 밀리초 단위
    frames[0].save(
        output_path,
        save_all=True,
        append_images=frames[1:],
        duration=duration,
        loop=0,
        optimize=True
    )
    
    # 결과 출력
    input_size = os.path.getsize(input_path) / (1024 * 1024)
    output_size = os.path.getsize(output_path) / (1024 * 1024)
    
    print(f"변환 완료!")
    print(f"  입력: {input_path} ({input_size:.2f} MB)")
    print(f"  출력: {output_path} ({output_size:.2f} MB)")
    
    return output_path


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("사용법: python mp4_to_gif.py <input.mp4> [output.gif] [fps] [width] [quality]")
        print("")
        print("예시:")
        print("  python mp4_to_gif.py video.mp4")
        print("  python mp4_to_gif.py video.mp4 output.gif 15 640 high")
        print("")
        print("매개변수:")
        print("  fps: 프레임 레이트 (기본값: 10)")
        print("  width: 출력 너비 픽셀 (기본값: 480)")
        print("  quality: low, medium, high (기본값: medium)")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else None
    fps = int(sys.argv[3]) if len(sys.argv) > 3 else 10
    width = int(sys.argv[4]) if len(sys.argv) > 4 else 480
    quality = sys.argv[5] if len(sys.argv) > 5 else "medium"
    
    convert_mp4_to_gif(input_file, output_file, fps, width, quality)