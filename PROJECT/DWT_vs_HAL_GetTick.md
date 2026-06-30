# DWT(Data Watchpoint and Trace) Cycle Counter vs GetTick(HAL_GetTick)

> STM32F103(Cortex-M3 코어 기반)에서 DWT(Data Watchpoint and Trace) Cycle Counter와 <br>
> GetTick(HAL_GetTick) 함수는 모두 시간이나 시스템의 지연(Delay)을 측정할 때 사용되지만, <br>
> 그 작동 원리와 정밀도(Resolution)에서 엄청난 차이가 있습니다.

* 두 방식의 핵심 차이점과 그 의미를 정리해 드립니다.

## 1. DWT Cycle Counter vs GetTick 비교

| 항목 | DWT Cycle Counter (DWT->CYCCNT) | HAL_GetTick () | 
|:---------------:|:---------------:|:---------------:|
| 기준 단위 | CPU 클럭 주기 (Cycle) | 밀리초 (ms, $10^{-3}$초) | 
| 하드웨어 출처 | Cortex-M3 내장 디버그/추적 장치 (DWT) | SysTick 타이머 인터럽트 | 
| 정밀도 (해상도) | 극도로 높음 (72MHz 기준 약 13.8ns, 64MHz 기준 약 15.625ns) | 낮음 (기본 1ms) | 
| 오버헤드 | 없음 (하드웨어 레지스터 직접 읽기) | 매우 적으나 인터럽트 의존적 | 
| 카운터 오버플로우 | 72MHz 기준 약 59.6초마다 발생 | 기본 32비트 변수 기준 약 49.7일 | 
| 주요 용도 | 함수 실행 시간 측정, 마이크로초($\mu s$) 단위 딜레이 | 시스템 업타임 확인, 밀리초 단위 스케줄링 | 

## 2. 세부적인 작동 원리와 차이의 의미

⚙️ DWT Cycle Counter<br>
* DWT는 ARM Cortex-M3 코어 내부에 내장된 디버그 모듈의 일부입니다. 
* DWT->CYCCNT 레지스터는 CPU 클럭이 튈 때마다 1씩 증가합니다.

   * 시간적 의미:
     * STM32F103을 최대 클럭인 64MHz로 구동할 경우, 1 사이클은 약 15.625나노초(ns)입니다.
     * 즉, 단 몇 줄의 어셈블리 명령어가 실행되는 시간까지 정확하게 측정할 수 있습니다.
   * 독립성:
     * 인터럽트를 타지 않고 하드웨어적으로 알아서 값이 올라가므로,
     * 다른 인터럽트가 걸려도 정확한 "클럭 소모량"을 측정합니다.

⏱️ HAL_GetTick()<br>
* 일반적으로 HAL_GetTick()은 SysTick 타이머 인터럽트를 기반으로 작동합니다.
* SysTick 타이머가 1ms마다 인터럽트를 발생시키면, ISR(인터럽트 서비스 루틴)에서 글로벌 변수(uwTick)를 1씩 증가시키는 방식입니다.

   * 시간적 의미:
     * 1ms($1,000,000\text{ ns}$) 단위로만 값이 변합니다.
     * 따라서 1ms 미만의 정밀도가 필요한 작업
     * (예: 센서 데이터 통신 타이밍, 미세한 코드 최적화 성능 측정)에는 전혀 사용할 수 없습니다.
   * 인터럽트 의존성:
     * 만약 HAL_GetTick()보다 우선순위가 높은 인터럽트가 오랫동안 실행되거나,
     * 코드 내에서 인터럽트를 완전히 비활성화(__disable_irq())하면 SysTick 카운터가 누락되어
     * 실제 시간보다 느리게 가는 현상이 발생할 수 있습니다.

## 3. 코드 관점에서의 의미와 활용 예시

① 성능 분석 및 프로파일링 (DWT 활용)어떤 알고리즘이나 함수의 순수 실행 시간을 측정하고 싶을 때는 DWT가 압도적으로 유리합니다.

```C
// DWT 활성화 (최초 1회 필요)
CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

// 측정 시작
uint32_t start_cycle = DWT->CYCCNT;

// [측정 대상 코드]
do_something_heavy();

// 측정 종료
uint32_t end_cycle = DWT->CYCCNT;
uint32_t total_cycles = end_cycle - start_cycle;

// 72MHz 기준 마이크로초(us) 변환
float microseconds = (float)total_cycles / 72.0f; 
```

② 일반적인 타임아웃 및 스케줄링 (GetTick 활용)

정밀도가 낮아도 상관없고 오랫동안 유지되는 타이밍(예: 1초마다 LED 플래싱, 500ms 통신 타임아웃 체크)에는 오버플로우 걱정이 없는 GetTick이 적합합니다.

```C
uint32_t last_time = HAL_GetTick();

while(1) {
    if (HAL_GetTick() - last_time >= 500) { // 500ms 주기
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        last_time = HAL_GetTick();
    }
}
```

## 4. 요약

* 값의 차이:
  * DWT->CYCCNT는 클럭 주기 단위로 매우 빠르게 증가하는 초정밀 데이터이며,
  * HAL_GetTick()은 1ms마다 느리게 증가하는 소프트웨어적 데이터입니다.
* 선택의 기준:
  * 마이크로초($\mu s$) 단위 이하의 미세 제어나 함수 성능 측정이 필요하다면 DWT를,
  * 밀리초($ms$) 단위의 전반적인 시스템 흐름 제어 및 타임아웃 처리에는 GetTick을 사용하는 것이 맞습니다.
