# SG90 Servo 제어를 위한 Timer 설정 (STM32 예제)

## 1. 기본 조건
- **타이머 클럭** = 64 MHz  
- **Prescaler** = 1280 - 1 = 1279  
- **Period** = 1000 - 1 = 999  

---

## 2. 타이머 카운트 주파수
\[
f_{timer} = \frac{64,000,000}{1280} = 50,000 \, \text{Hz}
\]

- 카운트 주파수 = **50 kHz**  
- Tick 주기 =  
\[
\frac{1}{50,000} = 20 \, \mu s
\]

---

## 3. PWM 주기
\[
T_{PWM} = \frac{Period + 1}{f_{timer}} = \frac{1000}{50,000} = 0.02 \, s = 20 \, ms
\]

✅ 따라서 PWM 주기 = **20 ms (50 Hz)** → SG90 서보 요구사항과 일치  

---

## 4. 펄스 폭 (CCR 값으로 각도 제어)

- **1 ms** 펄스 폭  
\[
\frac{1 \, ms}{20 \, \mu s} = 50 \quad \Rightarrow \quad CCR = 50
\]

- **1.5 ms** 펄스 폭  
\[
\frac{1.5 \, ms}{20 \, \mu s} = 75 \quad \Rightarrow \quad CCR = 75
\]

- **2 ms** 펄스 폭  
\[
\frac{2 \, ms}{20 \, \mu s} = 100 \quad \Rightarrow \quad CCR = 100
\]

---

## 5. 요약
- Prescaler = **1279**, Period = **999** → 정확히 **50 Hz (20 ms)** PWM 생성  
- CCR 값 50 ~ 100 사이로 설정하여 SG90 서보 각도 (0°~180°) 제어 가능  


## 6. 각도별 CCR 값
   * 0° → 1 ms → CCR = 50
   * 90° → 1.5 ms → CCR = 75
   * 180° → 2 ms → CCR = 100
```ㅊ
// 0도
__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 50);

// 90도
__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 75);

// 180도
__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 100);
```

## 7. 각도를 일반화한 함수
```ㅊ
void SG90_SetAngle(uint8_t angle)
{
    // angle: 0 ~ 180도
    // CCR: 50(1ms) ~ 100(2ms)
    uint32_t ccr_val = 50 + ((angle * (100 - 50)) / 180);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ccr_val);
}
```

## 8. 사용 예시:
```ㅊ
SG90_SetAngle(0);    // 0도
HAL_Delay(1000);

SG90_SetAngle(90);   // 90도
HAL_Delay(1000);

SG90_SetAngle(180);  // 180도
HAL_Delay(1000);
```
