# mp4를 animated gif

```

(base) C:\Users\Administrator\Desktop\Vector>python mp4_to_gif2.py LCD.mp4 LCD2.gif 15 640 high
변환 중...
  원본: 1920x1080, 30.0fps, 627프레임
  출력: 640x360, 15fps
  진행: 95.7%
  627개 프레임 추출 완료, GIF 저장 중...
변환 완료!
  입력: LCD.mp4 (7.95 MB)
  출력: LCD2.gif (81.37 MB)

(base) C:\Users\Administrator\Desktop\Vector>python mp4_to_gif2.py LCD-I2C.mp4 LCD2-I2C.gif 15 640 high
변환 중...
  원본: 1280x720, 24.0fps, 258프레임
  출력: 640x360, 15fps
  진행: 77.5%
  258개 프레임 추출 완료, GIF 저장 중...
변환 완료!
  입력: LCD-I2C.mp4 (2.66 MB)
  출력: LCD2-I2C.gif (43.04 MB)

(base) C:\Users\Administrator\Desktop\Vector>python mp4_to_gif2.py LCD-SPI.mp4 LCD2-SPI.gif 15 640 high
변환 중...
  원본: 1920x1080, 30.0fps, 412프레임
  출력: 640x360, 15fps
  진행: 97.1%
  412개 프레임 추출 완료, GIF 저장 중...
변환 완료!
  입력: LCD-SPI.mp4 (5.31 MB)
  출력: LCD2-SPI.gif (56.13 MB)

(base) C:\Users\Administrator\Desktop\Vector>python mp4_to_gif2.py LCD.mp4 LCD2.gif -o 5
Traceback (most recent call last):
  File "C:\Users\Administrator\Desktop\Vector\mp4_to_gif2.py", line 122, in <module>
    fps = int(sys.argv[3]) if len(sys.argv) > 3 else 10
          ^^^^^^^^^^^^^^^^
ValueError: invalid literal for int() with base 10: '-o'

(base) C:\Users\Administrator\Desktop\Vector>python mp4_to_gif2.py LCD.mp4 -o 5
변환 중...
  원본: 1920x1080, 30.0fps, 627프레임
  출력: 480x270, 5fps
  진행: 95.7%
  126개 프레임 추출 완료, GIF 저장 중...
Traceback (most recent call last):
  File "C:\ProgramData\anaconda3\Lib\site-packages\PIL\Image.py", line 2543, in save
    format = EXTENSION[ext]
             ~~~~~~~~~^^^^^
KeyError: ''

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "C:\Users\Administrator\Desktop\Vector\mp4_to_gif2.py", line 126, in <module>
    convert_mp4_to_gif(input_file, output_file, fps, width, quality)
  File "C:\Users\Administrator\Desktop\Vector\mp4_to_gif2.py", line 86, in convert_mp4_to_gif
    frames[0].save(
  File "C:\ProgramData\anaconda3\Lib\site-packages\PIL\Image.py", line 2546, in save
    raise ValueError(msg) from e
ValueError: unknown file extension:

(base) C:\Users\Administrator\Desktop\Vector>python mp4_to_gif.py LCD.mp4 LCD2-1.gif 10 480 medium 10
변환 중...
  원본: 1920x1080, 30.0fps, 627프레임
  출력: 480x270, 10fps
  진행: 95.7%
  314개 프레임 추출 완료, GIF 저장 중...
변환 완료!
  입력: LCD.mp4 (7.95 MB)
  출력: LCD2-1.gif (20.50 MB)

(base) C:\Users\Administrator\Desktop\Vector>python mp4_to_gif2.py LCD.mp4 LCD2-1.gif 10 480 medium 10

==================================================
시도 1: fps=10, width=480, quality=medium
==================================================
원본: 1920x1080, 30.0fps, 627프레임
출력: 480x270, 10fps, 64색상, 간격=2
  프레임 추출: 96%
  314개 프레임 → GIF 저장 중...

현재 크기: 14.86 MB (목표: 10.0 MB 이하)
용량 1.5배 초과, 설정 조정 중...

==================================================
시도 2: fps=10, width=480, quality=low
==================================================
원본: 1920x1080, 30.0fps, 627프레임
출력: 480x270, 10fps, 32색상, 간격=2
  프레임 추출: 96%
  314개 프레임 → GIF 저장 중...

현재 크기: 10.92 MB (목표: 10.0 MB 이하)
용량 1.1배 초과, 설정 조정 중...

==================================================
시도 3: fps=8, width=480, quality=low
==================================================
원본: 1920x1080, 30.0fps, 627프레임
출력: 480x270, 8fps, 32색상, 간격=3
  프레임 추출: 96%
  209개 프레임 → GIF 저장 중...

현재 크기: 7.29 MB (목표: 10.0 MB 이하)

==================================================
✓ 목표 달성!
==================================================
변환 완료!
  입력: LCD.mp4 (7.95 MB)
  출력: LCD2-1.gif (7.29 MB)
  압축률: 8.3% 감소

✓ GitHub README에 직접 삽입 가능 (7.3MB ≤ 10.0MB)
  ![Demo](LCD2-1.gif)

(base) C:\Users\Administrator\Desktop\Vector>python mp4_to_gif2.py LCD-I2C.mp4 LCD2-I2C.gif 10 480 medium 10

==================================================
시도 1: fps=10, width=480, quality=medium
==================================================
원본: 1280x720, 24.0fps, 258프레임
출력: 480x270, 10fps, 64색상, 간격=2
  프레임 추출: 78%
  129개 프레임 → GIF 저장 중...

현재 크기: 7.64 MB (목표: 10.0 MB 이하)

==================================================
✓ 목표 달성!
==================================================
변환 완료!
  입력: LCD-I2C.mp4 (2.66 MB)
  출력: LCD2-I2C.gif (7.64 MB)
  압축률: -187.6% 감소

✓ GitHub README에 직접 삽입 가능 (7.6MB ≤ 10.0MB)
  ![Demo](LCD2-I2C.gif)

(base) C:\Users\Administrator\Desktop\Vector>python mp4_to_gif2.py LCD-SPI.mp4 LCD2-SPI.gif 10 480 medium 10

==================================================
시도 1: fps=10, width=480, quality=medium
==================================================
원본: 1920x1080, 30.0fps, 412프레임
출력: 480x270, 10fps, 64색상, 간격=2
  프레임 추출: 97%
  206개 프레임 → GIF 저장 중...

현재 크기: 11.14 MB (목표: 10.0 MB 이하)
용량 1.1배 초과, 설정 조정 중...

==================================================
시도 2: fps=10, width=480, quality=low
==================================================
원본: 1920x1080, 30.0fps, 412프레임
출력: 480x270, 10fps, 32색상, 간격=2
  프레임 추출: 97%
  206개 프레임 → GIF 저장 중...

현재 크기: 8.75 MB (목표: 10.0 MB 이하)

==================================================
✓ 목표 달성!
==================================================
변환 완료!
  입력: LCD-SPI.mp4 (5.31 MB)
  출력: LCD2-SPI.gif (8.75 MB)
  압축률: -64.5% 감소

✓ GitHub README에 직접 삽입 가능 (8.7MB ≤ 10.0MB)
  ![Demo](LCD2-SPI.gif)

(base) C:\Users\Administrator\Desktop\Vector>python mp4_to_gif2.py LCD-SPI.mp4 LCD2-SPI.gif 10 480 medium 10

==================================================
시도 1: fps=10, width=480, quality=medium
==================================================
원본: 1280x720, 24.0fps, 537프레임
출력: 480x270, 10fps, 64색상, 간격=2
  프레임 추출: 93%
  269개 프레임 → GIF 저장 중...

현재 크기: 10.96 MB (목표: 10.0 MB 이하)
용량 1.1배 초과, 설정 조정 중...

==================================================
시도 2: fps=10, width=480, quality=low
==================================================
원본: 1280x720, 24.0fps, 537프레임
출력: 480x270, 10fps, 32색상, 간격=2
  프레임 추출: 93%
  269개 프레임 → GIF 저장 중...

현재 크기: 8.27 MB (목표: 10.0 MB 이하)

==================================================
✓ 목표 달성!
==================================================
변환 완료!
  입력: LCD-SPI.mp4 (5.49 MB)
  출력: LCD2-SPI.gif (8.27 MB)
  압축률: -50.6% 감소

✓ GitHub README에 직접 삽입 가능 (8.3MB ≤ 10.0MB)
  ![Demo](LCD2-SPI.gif)

(base) C:\Users\Administrator\Desktop\Vector>python mp4_to_gif2.py LCD-SPI.mp4 LCD2-SPI.gif 10 480 medium 10

==================================================
시도 1: fps=10, width=480, quality=medium
==================================================
원본: 974x720, 24.0fps, 474프레임
출력: 480x354, 10fps, 64색상, 간격=2
  프레임 추출: 84%
  237개 프레임 → GIF 저장 중...

현재 크기: 11.93 MB (목표: 10.0 MB 이하)
용량 1.2배 초과, 설정 조정 중...

==================================================
시도 2: fps=10, width=480, quality=low
==================================================
원본: 974x720, 24.0fps, 474프레임
출력: 480x354, 10fps, 32색상, 간격=2
  프레임 추출: 84%
  237개 프레임 → GIF 저장 중...

현재 크기: 8.33 MB (목표: 10.0 MB 이하)

==================================================
✓ 목표 달성!
==================================================
변환 완료!
  입력: LCD-SPI.mp4 (4.93 MB)
  출력: LCD2-SPI.gif (8.33 MB)
  압축률: -68.9% 감소

✓ GitHub README에 직접 삽입 가능 (8.3MB ≤ 10.0MB)
  ![Demo](LCD2-SPI.gif)

(base) C:\Users\Administrator\Desktop\Vector>python mp4_to_gif2.py LCD-I2C.mp4 LCD2-I2C.gif 10 480 medium 10

==================================================
시도 1: fps=10, width=480, quality=medium
==================================================
원본: 714x720, 24.0fps, 366프레임
출력: 480x484, 10fps, 64색상, 간격=2
  프레임 추출: 82%
  183개 프레임 → GIF 저장 중...

현재 크기: 11.42 MB (목표: 10.0 MB 이하)
용량 1.1배 초과, 설정 조정 중...

==================================================
시도 2: fps=10, width=480, quality=low
==================================================
원본: 714x720, 24.0fps, 366프레임
출력: 480x484, 10fps, 32색상, 간격=2
  프레임 추출: 82%
  183개 프레임 → GIF 저장 중...

현재 크기: 8.46 MB (목표: 10.0 MB 이하)

==================================================
✓ 목표 달성!
==================================================
변환 완료!
  입력: LCD-I2C.mp4 (3.81 MB)
  출력: LCD2-I2C.gif (8.46 MB)
  압축률: -122.1% 감소

✓ GitHub README에 직접 삽입 가능 (8.5MB ≤ 10.0MB)
  ![Demo](LCD2-I2C.gif)

(base) C:\Users\Administrator\Desktop\Vector>python mp4_to_gif2.py LCD.mp4 LCD2.gif 10 480 medium 10

==================================================
시도 1: fps=10, width=480, quality=medium
==================================================
원본: 616x720, 24.0fps, 421프레임
출력: 480x561, 10fps, 64색상, 간격=2
  프레임 추출: 95%
  211개 프레임 → GIF 저장 중...

현재 크기: 10.79 MB (목표: 10.0 MB 이하)
용량 1.1배 초과, 설정 조정 중...

==================================================
시도 2: fps=10, width=480, quality=low
==================================================
원본: 616x720, 24.0fps, 421프레임
출력: 480x561, 10fps, 32색상, 간격=2
  프레임 추출: 95%
  211개 프레임 → GIF 저장 중...

현재 크기: 8.33 MB (목표: 10.0 MB 이하)

==================================================
✓ 목표 달성!
==================================================
변환 완료!
  입력: LCD.mp4 (4.33 MB)
  출력: LCD2.gif (8.33 MB)
  압축률: -92.4% 감소

✓ GitHub README에 직접 삽입 가능 (8.3MB ≤ 10.0MB)
  ![Demo](LCD2.gif)

(base) C:\Users\Administrator\Desktop\Vector>
```
