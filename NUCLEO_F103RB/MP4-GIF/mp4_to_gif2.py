import os
import sys
from PIL import Image
import cv2

def convert_mp4_to_gif(input_path, output_path=None, fps=10, width=480, quality="medium",
                       max_size_mb=10, auto_optimize=True):
    """
    MP4 파일을 GitHub에 올릴 수 있는 크기의 애니메이션 GIF로 변환합니다.
    """
    if not os.path.exists(input_path):
        raise FileNotFoundError(f"입력 파일을 찾을 수 없습니다: {input_path}")
    
    if output_path is None:
        base = os.path.splitext(input_path)[0]
        output_path = f"{base}.gif"
    
    # 초기 설정값
    current_fps = fps
    current_width = width
    current_quality = quality
    
    # 품질별 색상 수
    quality_colors = {"low": 32, "medium": 64, "high": 128}
    quality_order = ["high", "medium", "low"]
    
    max_attempts = 10
    
    for attempt in range(max_attempts):
        print(f"\n{'='*50}")
        print(f"시도 {attempt+1}: fps={current_fps}, width={current_width}, quality={current_quality}")
        print('='*50)
        
        # GIF 생성
        colors = quality_colors.get(current_quality, 64)
        _create_gif(input_path, output_path, current_fps, current_width, colors)
        
        # 파일 크기 확인
        output_size_mb = os.path.getsize(output_path) / (1024 * 1024)
        print(f"\n현재 크기: {output_size_mb:.2f} MB (목표: {max_size_mb} MB 이하)")
        
        if output_size_mb <= max_size_mb:
            print(f"\n{'='*50}")
            print(f"✓ 목표 달성!")
            _print_result(input_path, output_path, max_size_mb)
            return output_path
        
        if not auto_optimize:
            print(f"\n⚠ 파일 크기 초과")
            _print_result(input_path, output_path, max_size_mb)
            return output_path
        
        # === 자동 최적화 전략 ===
        reduction_needed = output_size_mb / max_size_mb
        print(f"용량 {reduction_needed:.1f}배 초과, 설정 조정 중...")
        
        # 큰 폭의 감소가 필요하면 여러 설정 동시 조정
        if reduction_needed > 2.0:
            # 해상도 대폭 축소
            current_width = max(160, int(current_width * 0.6))
            current_fps = max(5, current_fps - 2)
            if current_quality in quality_order:
                idx = quality_order.index(current_quality)
                if idx < len(quality_order) - 1:
                    current_quality = quality_order[idx + 1]
        elif reduction_needed > 1.5:
            # 해상도 축소
            current_width = max(160, int(current_width * 0.75))
            current_fps = max(5, current_fps - 1)
        else:
            # 작은 조정
            if current_quality != "low":
                idx = quality_order.index(current_quality)
                if idx < len(quality_order) - 1:
                    current_quality = quality_order[idx + 1]
            elif current_fps > 5:
                current_fps = max(5, current_fps - 2)
            else:
                current_width = max(160, int(current_width * 0.8))
        
        # 최소값 체크
        if current_width <= 160 and current_fps <= 5 and current_quality == "low":
            print(f"\n⚠ 최소 설정 도달. 더 이상 최적화 불가.")
            break
    
    _print_result(input_path, output_path, max_size_mb)
    return output_path


def _create_gif(input_path, output_path, fps, width, colors):
    """실제 GIF 생성 함수"""
    
    cap = cv2.VideoCapture(input_path)
    if not cap.isOpened():
        raise IOError(f"비디오를 열 수 없습니다: {input_path}")
    
    original_fps = cap.get(cv2.CAP_PROP_FPS)
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    original_width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    original_height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    
    # 출력 크기 계산
    ratio = width / original_width
    height = int(original_height * ratio)
    
    # 프레임 샘플링 간격
    frame_interval = max(1, int(original_fps / fps))
    
    print(f"원본: {original_width}x{original_height}, {original_fps:.1f}fps, {total_frames}프레임")
    print(f"출력: {width}x{height}, {fps}fps, {colors}색상, 간격={frame_interval}")
    
    frames = []
    frame_count = 0
    
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        
        if frame_count % frame_interval == 0:
            frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            frame_resized = cv2.resize(frame_rgb, (width, height), interpolation=cv2.INTER_AREA)
            img = Image.fromarray(frame_resized)
            img = img.quantize(colors=colors, method=Image.Quantize.MEDIANCUT)
            frames.append(img)
        
        frame_count += 1
        
        if frame_count % 100 == 0:
            progress = (frame_count / total_frames) * 100
            print(f"  프레임 추출: {progress:.0f}%", end='\r')
    
    cap.release()
    
    if not frames:
        raise ValueError("추출된 프레임이 없습니다")
    
    print(f"\n  {len(frames)}개 프레임 → GIF 저장 중...")
    
    duration = int(1000 / fps)
    frames[0].save(
        output_path,
        save_all=True,
        append_images=frames[1:],
        duration=duration,
        loop=0,
        optimize=True
    )
    
    return output_path


def _print_result(input_path, output_path, max_size_mb):
    """결과 출력"""
    input_size = os.path.getsize(input_path) / (1024 * 1024)
    output_size = os.path.getsize(output_path) / (1024 * 1024)
    
    print(f"{'='*50}")
    print("변환 완료!")
    print(f"  입력: {input_path} ({input_size:.2f} MB)")
    print(f"  출력: {output_path} ({output_size:.2f} MB)")
    print(f"  압축률: {(1 - output_size/input_size)*100:.1f}% 감소")
    
    if output_size <= max_size_mb:
        print(f"\n✓ GitHub README에 직접 삽입 가능 ({output_size:.1f}MB ≤ {max_size_mb}MB)")
        print(f"  ![Demo]({os.path.basename(output_path)})")
    elif output_size <= 25:
        print(f"\n⚠ 목표 초과 ({output_size:.1f}MB > {max_size_mb}MB)")
        print("  더 낮은 설정으로 재시도하거나 영상을 짧게 편집하세요.")
    elif output_size <= 100:
        print(f"\n⚠ GitHub LFS 사용 필요")
    else:
        print(f"\n✗ GitHub 업로드 불가 (100MB 초과)")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("MP4 to GIF 변환기 (GitHub 최적화)")
        print("="*50)
        print("\n사용법:")
        print("  python mp4_to_gif.py <input.mp4> [output.gif] [fps] [width] [quality] [max_mb]")
        print("\n예시:")
        print("  python mp4_to_gif.py video.mp4")
        print("  python mp4_to_gif.py video.mp4 demo.gif 10 480 medium 10")
        print("  python mp4_to_gif.py video.mp4 -o 5    # 5MB 이하로 자동 최적화")
        sys.exit(1)
    
    input_file = sys.argv[1]
    
    # 간단 모드: mp4_to_gif.py video.mp4 -o 5
    if len(sys.argv) >= 3 and sys.argv[2] == "-o":
        max_mb = float(sys.argv[3]) if len(sys.argv) > 3 else 10
        convert_mp4_to_gif(input_file, max_size_mb=max_mb)
    else:
        output_file = sys.argv[2] if len(sys.argv) > 2 else None
        fps = int(sys.argv[3]) if len(sys.argv) > 3 else 10
        width = int(sys.argv[4]) if len(sys.argv) > 4 else 480
        quality = sys.argv[5] if len(sys.argv) > 5 else "medium"
        max_mb = float(sys.argv[6]) if len(sys.argv) > 6 else 10
        
        convert_mp4_to_gif(input_file, output_file, fps, width, quality, max_mb)