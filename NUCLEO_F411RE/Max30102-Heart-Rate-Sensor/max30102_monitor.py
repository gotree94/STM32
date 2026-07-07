#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
MAX30102 Heart-Rate Sensor Monitor
===================================
STM32F411 Nucleo + MAX30102 → Serial CSV → Real-time Visualization

Medical-grade dark UI with PPG waveform, vital signs, and event log.

Dependencies (auto-installed on first run):
  - pyserial
  - matplotlib
  - numpy
"""

import sys
import subprocess
import importlib

# ── Matplotlib backend (must be set before other imports) ────────
import matplotlib
matplotlib.use("TkAgg")

# ── Auto-install dependencies ──────────────────────────────────────────────
_REQUIRED = {"pyserial": "serial", "matplotlib": "matplotlib", "numpy": "numpy"}
for _pkg, _mod in _REQUIRED.items():
    try:
        importlib.import_module(_mod)
    except ImportError:
        print(f"[INSTALL] Installing {_pkg}...")
        subprocess.check_call(
            [sys.executable, "-m", "pip", "install", _pkg, "--quiet"]
        )
        print(f"[INSTALL] {_pkg} installed.")

# ── Imports ────────────────────────────────────────────────────────────────
import tkinter as tk
from tkinter import ttk, scrolledtext
import threading
import queue
import time
import csv
from collections import deque
from datetime import datetime

import os
import json
import serial
import serial.tools.list_ports
import numpy as np

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.figure as mpl_figure
import matplotlib.animation as mpl_animation

# ── Constants ──────────────────────────────────────────────────────────────

# Medical Theme Palette
THEME = {
    "bg": "#0D1117",
    "card": "#161B22",
    "card_hover": "#1C2333",
    "border": "#30363D",
    "text": "#E6EDF3",
    "text_sec": "#8B949E",
    "text_dim": "#484F58",
    "accent": "#00E5A0",      # medical teal
    "accent2": "#00B4D8",     # cyan accent
    "warn": "#FF4444",
    "warn_dim": "#FF6B6B",
    "success": "#3FB950",
    "graph_bg": "#0D1117",
    "graph_grid": "#21262D",
    "ir_color": "#00E5A0",    # IR on graph
    "red_color": "#FF4444",   # Red on graph
}

PLOT_WINDOW_SEC = 10         # Rolling window (seconds)
MAX_LOG_LINES = 500
DEFAULT_BAUD = 115200
CONFIG_FILE = "max30102_monitor_config.json"

# ── MAX30102 Register Map ────────────────────────────────────────

REGISTER_NAMES = {
    0x00: "STATUS_1", 0x01: "STATUS_2", 0x02: "INTR_ENABLE_1",
    0x03: "INTR_ENABLE_2", 0x04: "FIFO_WR_PTR", 0x05: "OVF_COUNTER",
    0x06: "FIFO_RD_PTR", 0x07: "FIFO_DATA", 0x08: "FIFO_CONFIG",
    0x09: "MODE_CONFIG", 0x0A: "SPO2_CONFIG", 0x0C: "LED1_PA (IR)",
    0x0D: "LED2_PA (Red)", 0x10: "PILOT_PA", 0x1F: "DIE_TEMP_INT",
    0x20: "DIE_TEMP_FRAC", 0x21: "DIE_TEMP_CONFIG", 0xFE: "REV_ID",
    0xFF: "PART_ID",
}

QUICK_REGS = [
    (0xFF, "Part ID"), (0xFE, "Rev ID"), (0x09, "Mode"),
    (0x08, "FIFO"), (0x0A, "SpO2"), (0x0C, "LED1_IR"),
    (0x0D, "LED2_Red"), (0x00, "Status1"), (0x01, "Status2"),
    (0x04, "WrPtr"), (0x06, "RdPtr"),
]

# ── Serial Reader ──────────────────────────────────────────────────────────

class SerialReader(threading.Thread):
    """Background thread that reads serial data and puts lines into a queue."""

    def __init__(self, port: str, baud: int, data_q: queue.Queue):
        super().__init__(daemon=True)
        self.port = port
        self.baud = baud
        self.data_q = data_q
        self._stop = threading.Event()
        self.ser = None

    def run(self):
        try:
            self.ser = serial.Serial(
                self.port, self.baud, timeout=0.1,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
            )
            self.data_q.put(("__status__", "connected"))
        except Exception as e:
            self.data_q.put(("__error__", f"Serial open failed: {e}"))
            return

        while not self._stop.is_set():
            try:
                if self.ser and self.ser.in_waiting:
                    line = self.ser.readline().decode("utf-8", errors="replace").strip()
                    if line:
                        self.data_q.put(("data", line))
                else:
                    time.sleep(0.001)
            except serial.SerialException as e:
                self.data_q.put(("__error__", f"Serial error: {e}"))
                break
            except Exception as e:
                self.data_q.put(("__error__", str(e)))
                break

        self._close()

    def send_command(self, cmd: str) -> bool:
        """Send a command string to the STM32 over UART."""
        if self.ser and self.ser.is_open:
            try:
                self.ser.write((cmd + "\n").encode("utf-8"))
                return True
            except Exception:
                return False
        return False

    def stop(self):
        self._stop.set()
        self._close()

    def _close(self):
        if self.ser and self.ser.is_open:
            try:
                self.ser.close()
            except Exception:
                pass
        self.data_q.put(("__status__", "disconnected"))


# ── Heart Rate Estimator ──────────────────────────────────────────────────

class HeartRateEstimator:
    """
    Peak-detection based HR estimator from IR PPG signal.

    4-second rolling window captures ~4.8 heartbeats at 72 BPM,
    ensuring enough peaks for reliable estimation.
    Uses adaptive threshold and outlier rejection for robustness.
    """

    def __init__(self, sample_rate_hz: float = 50.0):
        self.fs = sample_rate_hz
        self.window_s = 4.0
        self.window_size = int(self.fs * self.window_s)
        self.buffer = deque(maxlen=self.window_size)
        self.min_peak_distance = int(self.fs * 0.25)
        self.sample_counter = 0

    def set_fs(self, new_fs: float):
        if new_fs == self.fs:
            return
        # Preserve existing data by converting to time and back
        old_data = list(self.buffer)
        self.fs = new_fs
        self.window_size = int(self.fs * self.window_s)
        self.min_peak_distance = int(self.fs * 0.25)
        # Recreate buffer with new size, keep as much old data as fits
        self.buffer = deque(maxlen=self.window_size)
        for v in old_data:
            self.buffer.append(v)

    def add_sample(self, ir_value: float):
        self.buffer.append(ir_value)
        self.sample_counter += 1

    def estimate_bpm(self) -> float:
        if len(self.buffer) < int(self.fs * 2.0):
            return 0.0

        data = np.array(self.buffer, dtype=float)
        sig_std = np.std(data)
        sig_range = np.max(data) - np.min(data)
        if sig_std < 50 or sig_range < 200:
            return 0.0

        # 1. Remove DC offset (running baseline subtraction) — edge-padded convolution
        baseline_win = int(self.fs * 0.5)
        kernel_bl = np.ones(baseline_win) / baseline_win
        padded = np.pad(data, baseline_win // 2, mode='edge')
        baseline = np.convolve(padded, kernel_bl, mode='valid')[:len(data)]
        ac = data - baseline  # AC-coupled: pulsatile component only

        # 2. Smooth the AC signal — edge-padded convolution
        smooth_win = max(3, int(self.fs * 0.02))
        kernel_sm = np.ones(smooth_win) / smooth_win
        padded_ac = np.pad(ac, smooth_win // 2, mode='edge')
        ac_smooth = np.convolve(padded_ac, kernel_sm, mode='valid')[:len(ac)]

        # 3. Adaptive threshold on AC signal (not raw data)
        ac_max = np.max(ac_smooth)
        ac_min = np.min(ac_smooth)
        threshold = ac_min + (ac_max - ac_min) * 0.6  # 60% of AC range

        # 4. Peak detection on AC signal
        half_window = max(3, int(self.fs * 0.08))
        peaks = []
        for i in range(half_window, len(ac_smooth) - half_window):
            if ac_smooth[i] <= threshold:
                continue
            window = ac_smooth[i - half_window : i + half_window + 1]
            if ac_smooth[i] == np.max(window):
                if not peaks or (i - peaks[-1] >= self.min_peak_distance):
                    peaks.append(i)

        if len(peaks) < 2:
            return 0.0

        intervals_s = np.diff(peaks) / self.fs
        valid = intervals_s[(intervals_s >= 0.25) & (intervals_s <= 2.0)]
        if len(valid) < 1:
            return 0.0

        # 5. Reject intervals that deviate >25% from median
        median_iv = np.median(valid)
        consistent = valid[(valid > median_iv * 0.75) & (valid < median_iv * 1.25)]
        if len(consistent) < 1:
            return 0.0

        avg_interval = np.median(consistent)
        bpm = 60.0 / avg_interval
        return max(30.0, min(220.0, bpm))

    def estimate_sq(self) -> float:
        """Signal Quality: AC_amplitude / DC_level * 100 from raw IR buffer."""
        if len(self.buffer) < int(self.fs * 2.0):
            return 0.0

        data = np.array(self.buffer, dtype=float)
        dc = np.mean(data)
        if dc < 1:
            return 0.0

        # Extract AC component — same baseline removal as estimate_bpm
        baseline_win = int(self.fs * 0.5)
        kernel_bl = np.ones(baseline_win) / baseline_win
        padded = np.pad(data, baseline_win // 2, mode='edge')
        baseline = np.convolve(padded, kernel_bl, mode='valid')[:len(data)]
        ac = data - baseline

        ac_amplitude = (np.max(ac) - np.min(ac)) / 2.0
        sq = (ac_amplitude / dc) * 100.0
        return sq

    def reset(self):
        self.buffer.clear()
        self.sample_counter = 0


# ── SpO2 Estimator (Simplified) ───────────────────────────────────────────

class SpO2Estimator:
    """
    SpO2 estimation from Red and IR PPG signals using AC/DC ratio.
    AC is extracted via baseline subtraction (bandpass equivalent).
    Formula: SpO2 = 110 - 25 * R  (where R = AC_red/DC_red / AC_ir/DC_ir)
    NOTE: This is calibration-dependent. Real medical devices need per-device
    calibration. This provides a rough trend estimate only.
    """

    def __init__(self, sample_rate_hz: float = 50.0, window_s: float = 4.0):
        self.fs = sample_rate_hz
        self.window_len = int(self.fs * window_s)
        self.ir_buffer = deque(maxlen=self.window_len)
        self.red_buffer = deque(maxlen=self.window_len)

    def set_fs(self, new_fs: float):
        if new_fs == self.fs:
            return
        old_ir = list(self.ir_buffer)
        old_red = list(self.red_buffer)
        self.fs = new_fs
        self.window_len = int(self.fs * 4.0)
        self.ir_buffer = deque(maxlen=self.window_len)
        self.red_buffer = deque(maxlen=self.window_len)
        for v in old_ir:
            self.ir_buffer.append(v)
        for v in old_red:
            self.red_buffer.append(v)

    def add_sample(self, ir_value: float, red_value: float):
        self.ir_buffer.append(ir_value)
        self.red_buffer.append(red_value)

    def _get_ac_dc(self, data):
        """Extract AC (pulsatile) and DC (baseline) components."""
        dc = np.mean(data)
        # Remove baseline for AC estimation — edge-padded convolution
        win = max(3, int(self.fs * 0.5))
        kernel = np.ones(win) / win
        padded = np.pad(data, win // 2, mode='edge')
        baseline = np.convolve(padded, kernel, mode='valid')[:len(data)]
        ac = data[:len(baseline)] - baseline
        ac_amplitude = (np.max(ac) - np.min(ac)) / 2.0
        return ac_amplitude, dc

    def estimate_spo2(self) -> float:
        """Returns estimated SpO2 percentage or 0.0."""
        if len(self.ir_buffer) < self.window_len * 0.5:
            return 0.0

        ir = np.array(self.ir_buffer, dtype=float)
        red = np.array(self.red_buffer, dtype=float)

        ac_ir, dc_ir = self._get_ac_dc(ir)
        ac_red, dc_red = self._get_ac_dc(red)

        if dc_ir < 1 or dc_red < 1:
            return 0.0

        if ac_ir / dc_ir < 1e-6 or ac_red / dc_red < 1e-6:
            return 0.0

        r = (ac_red / dc_red) / (ac_ir / dc_ir)

        # Empirical formula (calibration-dependent)
        spo2 = 110.0 - 25.0 * r
        spo2 = max(80.0, min(100.0, spo2))  # clamp to plausible range
        return spo2

    def reset(self):
        self.ir_buffer.clear()
        self.red_buffer.clear()


# ── Main Application ──────────────────────────────────────────────────────

class MAX30102Monitor(tk.Tk):
    """Medical-themed MAX30102 heart rate monitor GUI."""

    def __init__(self):
        super().__init__()

        self.title("MAX30102 Patient Monitor v1.0")
        self.configure(bg=THEME["bg"])
        self.minsize(1200, 750)

        # State
        self.connected = False
        self.reader = None
        self.data_queue = queue.Queue()
        self.recording = False
        self.record_file = None

        # Data buffers for plotting
        self.plot_duration = PLOT_WINDOW_SEC
        self.fs = 50.0  # default: 200sps / 4-sample averaging
        self.t_samples = deque(maxlen=2000)
        self.ir_samples = deque(maxlen=2000)
        self.red_samples = deque(maxlen=2000)

        self.last_ir = 0
        self.last_red = 0
        self.last_temp = 0.0
        self.sample_count = 0
        self.csv_header = "Count,IR_raw,Red_raw"

        # Signal processing
        self.hr_estimator = HeartRateEstimator(sample_rate_hz=self.fs)
        self.spo2_estimator = SpO2Estimator(sample_rate_hz=self.fs)

        self._sample_rate_t0 = 0.0

        self.current_bpm = 0.0
        self.current_spo2 = 0.0
        self.bpm_history = deque(maxlen=25)
        self.spo2_history = deque(maxlen=25)
        self.current_sq = 0.0
        self.sq_history = deque(maxlen=25)

        # Restore saved settings
        self.saved_port = ""
        self._load_config()

        # Build UI
        self._build_ui()
        self._scan_ports()

        # Start plot animation
        self.ani = mpl_animation.FuncAnimation(
            self.figure, self._update_plot, interval=50, blit=False,
            cache_frame_data=False,
        )

        # Start UI update timer
        self.after(50, self._process_queue)

        # Handle window close
        self.protocol("WM_DELETE_WINDOW", self._on_close)

    # ── UI Construction ────────────────────────────────────────────────────

    def _build_ui(self):
        self._build_top_bar()
        self._build_main_area()
        self._build_register_panel()
        self._build_log_panel()

    def _build_top_bar(self):
        """Serial connection controls."""
        bar = tk.Frame(self, bg=THEME["card"], height=56)
        bar.pack(fill=tk.X, padx=8, pady=(8, 2))
        bar.pack_propagate(False)

        # Inner container with padding
        inner = tk.Frame(bar, bg=THEME["card"])
        inner.pack(padx=16, pady=8, fill=tk.X)

        # COM Port selector
        tk.Label(inner, text="Port:", bg=THEME["card"],
                 fg=THEME["text_sec"], font=("Segoe UI", 9)).pack(side=tk.LEFT)
        self.port_var = tk.StringVar()
        self.port_combo = ttk.Combobox(
            inner, textvariable=self.port_var, width=16,
            state="readonly", font=("Segoe UI", 9),
        )
        self.port_combo.pack(side=tk.LEFT, padx=(6, 16))

        # Baud rate selector
        tk.Label(inner, text="Baud:", bg=THEME["card"],
                 fg=THEME["text_sec"], font=("Segoe UI", 9)).pack(side=tk.LEFT)
        self.baud_var = tk.StringVar(value=str(DEFAULT_BAUD))
        self.baud_combo = ttk.Combobox(
            inner, textvariable=self.baud_var, width=10,
            values=["9600", "19200", "38400", "57600", "115200", "230400"],
            state="readonly", font=("Segoe UI", 9),
        )
        self.baud_combo.pack(side=tk.LEFT, padx=(6, 16))

        # Refresh ports button
        self.refresh_btn = self._med_button(
            inner, "⟳", self._scan_ports, w_chars=3,
        )
        self.refresh_btn.pack(side=tk.LEFT, padx=(0, 16))

        # Connect / Disconnect button
        self.connect_btn = self._med_button(
            inner, "🔗  Connect", self._toggle_connect,
            bg="#238636", w_chars=16,
        )
        self.connect_btn.pack(side=tk.LEFT)

        # Status indicator
        self.status_indicator = tk.Canvas(
            inner, width=12, height=12, bg=THEME["card"],
            highlightthickness=0,
        )
        self.status_indicator.pack(side=tk.LEFT, padx=(12, 6))
        self._draw_status_dot("disconnected")

        self.status_label = tk.Label(
            inner, text="Disconnected", bg=THEME["card"],
            fg=THEME["text_sec"], font=("Segoe UI", 9),
        )
        self.status_label.pack(side=tk.LEFT)

        # Spacer
        tk.Label(inner, text="", bg=THEME["card"]).pack(side=tk.LEFT, fill=tk.X, expand=True)

        # Recording controls
        self.record_btn = self._med_button(
            inner, "●  Record", self._toggle_record,
            bg="#1F2937", w_chars=12,
        )
        self.record_btn.pack(side=tk.RIGHT, padx=(8, 0))

        # Auto-scale
        self.autoscale_var = tk.BooleanVar(value=True)
        self.autoscale_cb = tk.Checkbutton(
            inner, text="Auto-scale", variable=self.autoscale_var,
            bg=THEME["card"], fg=THEME["text_sec"],
            activebackground=THEME["card"], activeforeground=THEME["text"],
            selectcolor=THEME["bg"], font=("Segoe UI", 9),
        )
        self.autoscale_cb.pack(side=tk.RIGHT, padx=(0, 8))

    def _build_main_area(self):
        """Vital signs + Graph."""
        main = tk.Frame(self, bg=THEME["bg"])
        main.pack(fill=tk.BOTH, expand=True, padx=8, pady=4)

        # ── Left: Vital Signs Panel ──
        vital_frame = tk.Frame(main, bg=THEME["card"], width=280)
        vital_frame.pack(side=tk.LEFT, fill=tk.Y, padx=(0, 6), pady=0)
        vital_frame.pack_propagate(False)

        # Title
        tk.Label(
            vital_frame, text="VITAL SIGNS", bg=THEME["card"],
            fg=THEME["text_sec"], font=("Segoe UI", 9, "bold"),
        ).pack(anchor=tk.W, padx=16, pady=(14, 6))

        # Heart rate (BPM)
        self._create_vital_card(
            vital_frame, "❤", "HEART RATE", "--", "BPM",
            "current_bpm_big", "bpm_color",
        )

        # Signal Quality (SQ)
        sq_card = tk.Frame(vital_frame, bg=THEME["bg"], padx=14, pady=4)
        sq_card.pack(fill=tk.X, padx=12, pady=1)
        sq_row1 = tk.Frame(sq_card, bg=THEME["bg"])
        sq_row1.pack(fill=tk.X)
        tk.Label(sq_row1, text="📶", bg=THEME["bg"],
                 fg=THEME["accent"], font=("Segoe UI", 10)).pack(side=tk.LEFT, padx=(0, 6))
        tk.Label(sq_row1, text="SIGNAL QUALITY", bg=THEME["bg"],
                 fg=THEME["text_sec"], font=("Segoe UI", 8, "bold")).pack(side=tk.LEFT)
        self.current_sq_label = tk.Label(
            sq_card, text="--", bg=THEME["bg"],
            fg=THEME["text_dim"], font=("Segoe UI", 14, "bold"),
            anchor=tk.W,
        )
        self.current_sq_label.pack(fill=tk.X, pady=(2, 0))

        # SpO2
        self._create_vital_card(
            vital_frame, "O₂", "SpO₂", "--", "%",
            "current_spo2_big", "spo2_color",
        )

        # Temperature
        self._create_vital_card(
            vital_frame, "🌡", "TEMP", "--", "°C",
            "current_temp_big", "temp_color",
        )

        # IR value
        self._create_vital_mini(vital_frame, "IR (PPG)", "--", 0)

        # Red value
        self._create_vital_mini(vital_frame, "RED (PPG)", "--", 1)

        # Sample count
        self.sample_count_label = tk.Label(
            vital_frame, text="Samples: 0", bg=THEME["card"],
            fg=THEME["text_sec"], font=("Segoe UI", 8),
        )
        self.sample_count_label.pack(anchor=tk.W, padx=16, pady=(8, 0))

        # ── Right: Graph ──
        graph_frame = tk.Frame(main, bg=THEME["card"])
        graph_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True)

        # Matplotlib figure
        self.figure = mpl_figure.Figure(figsize=(8, 4), dpi=100)
        self.figure.patch.set_facecolor(THEME["graph_bg"])

        self.ax = self.figure.add_subplot(111)
        self.ax.set_facecolor(THEME["graph_bg"])
        self.ax.tick_params(colors=THEME["text_sec"], labelsize=8)
        self.ax.set_xlabel("Time (s)", color=THEME["text_sec"], fontsize=9)
        self.ax.set_ylabel("ADC Value", color=THEME["text_sec"], fontsize=9)
        self.ax.grid(True, color=THEME["graph_grid"], linewidth=0.5, alpha=0.5)
        self.ax.spines["bottom"].set_color(THEME["border"])
        self.ax.spines["left"].set_color(THEME["border"])
        self.ax.spines["top"].set_visible(False)
        self.ax.spines["right"].set_visible(False)

        # Lines (initially empty)
        self.ir_line, = self.ax.plot(
            [], [], color=THEME["ir_color"], linewidth=1.2, alpha=0.9,
            label="IR (PPG)",
        )
        self.red_line, = self.ax.plot(
            [], [], color=THEME["red_color"], linewidth=1.2, alpha=0.7,
            label="Red",
        )
        self.ax.legend(
            loc="upper right", fontsize=8,
            facecolor=THEME["card"], edgecolor=THEME["border"],
            labelcolor=THEME["text"],
        )

        self.canvas = FigureCanvasTkAgg(self.figure, master=graph_frame)
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True, padx=4, pady=4)

    def _create_vital_card(self, parent, icon, label, value, unit,
                          value_attr, color_attr):
        """Large vital sign card: icon + label (top), big value (center), unit."""
        card = tk.Frame(parent, bg=THEME["bg"], padx=14, pady=10)
        card.pack(fill=tk.X, padx=12, pady=3)

        # Row 1: icon + label
        row1 = tk.Frame(card, bg=THEME["bg"])
        row1.pack(fill=tk.X)

        tk.Label(row1, text=icon, bg=THEME["bg"],
                 fg=THEME["accent"], font=("Segoe UI", 12)).pack(side=tk.LEFT, padx=(0, 6))
        tk.Label(row1, text=label, bg=THEME["bg"],
                 fg=THEME["text_sec"], font=("Segoe UI", 8, "bold")).pack(side=tk.LEFT)

        # Row 2: value
        val_label = tk.Label(
            card, text=value, bg=THEME["bg"],
            fg=THEME["accent"], font=("Segoe UI", 28, "bold"),
            anchor=tk.W,
        )
        val_label.pack(fill=tk.X, pady=(2, 0))

        # Row 3: unit
        tk.Label(card, text=unit, bg=THEME["bg"],
                 fg=THEME["text_sec"], font=("Segoe UI", 8)).pack(anchor=tk.W)

        # Store references
        setattr(self, value_attr, val_label)
        setattr(self, color_attr, THEME["accent"])

    def _create_vital_mini(self, parent, label, value, idx):
        """Smaller value display."""
        frame = tk.Frame(parent, bg=THEME["bg"], padx=14, pady=6)
        frame.pack(fill=tk.X, padx=12, pady=2)

        tk.Label(frame, text=label, bg=THEME["bg"],
                 fg=THEME["text_sec"], font=("Segoe UI", 8)).pack(side=tk.LEFT)

        val = tk.Label(frame, text=value, bg=THEME["bg"],
                       fg=THEME["text"], font=("Segoe UI", 12, "bold"),
                       anchor=tk.E)
        val.pack(side=tk.RIGHT)

        attr = "current_mini_" + ("ir" if idx == 0 else "red")
        setattr(self, attr, val)

    def _build_register_panel(self):
        """Register control panel between main area and log."""
        frame = tk.Frame(self, bg=THEME["card"])
        frame.pack(fill=tk.X, padx=8, pady=(2, 2))

        # Title
        title = tk.Label(frame, text="REGISTERS", bg=THEME["card"],
                         fg=THEME["text_sec"], font=("Segoe UI", 9, "bold"))
        title.pack(anchor=tk.W, padx=12, pady=(4, 2))

        # Row 1: Address, Value, Read, Write
        row1 = tk.Frame(frame, bg=THEME["card"])
        row1.pack(fill=tk.X, padx=12, pady=(0, 2))

        tk.Label(row1, text="Addr:", bg=THEME["card"],
                 fg=THEME["text_sec"], font=("Consolas", 9)).pack(side=tk.LEFT)
        self.reg_addr_var = tk.StringVar(value="09")
        self.reg_addr_entry = tk.Entry(row1, textvariable=self.reg_addr_var,
                                       width=6, bg=THEME["bg"], fg=THEME["text"],
                                       insertbackground=THEME["text"],
                                       relief=tk.FLAT, bd=1,
                                       font=("Consolas", 10, "bold"),
                                       justify=tk.CENTER)
        self.reg_addr_entry.pack(side=tk.LEFT, padx=(4, 12))
        self.reg_addr_entry.bind("<Return>", lambda e: self._reg_read_cmd())

        tk.Label(row1, text="Value:", bg=THEME["card"],
                 fg=THEME["text_sec"], font=("Consolas", 9)).pack(side=tk.LEFT)
        self.reg_val_var = tk.StringVar(value="00")
        self.reg_val_entry = tk.Entry(row1, textvariable=self.reg_val_var,
                                      width=6, bg=THEME["bg"], fg=THEME["text"],
                                      insertbackground=THEME["text"],
                                      relief=tk.FLAT, bd=1,
                                      font=("Consolas", 10, "bold"),
                                      justify=tk.CENTER)
        self.reg_val_entry.pack(side=tk.LEFT, padx=(4, 16))
        self.reg_val_entry.bind("<Return>", lambda e: self._reg_write_cmd())

        self.reg_read_btn = self._med_button(row1, "Read", self._reg_read_cmd,
                                             bg="#1F2937", w_chars=6, font_size=9)
        self.reg_read_btn.pack(side=tk.LEFT, padx=(0, 4))

        self.reg_write_btn = self._med_button(row1, "Write", self._reg_write_cmd,
                                              bg="#1F2937", w_chars=6, font_size=9)
        self.reg_write_btn.pack(side=tk.LEFT, padx=(0, 12))

        # Result label
        self.reg_result_var = tk.StringVar(value="")
        self.reg_result_label = tk.Label(
            row1, textvariable=self.reg_result_var,
            bg=THEME["card"], fg=THEME["accent2"],
            font=("Consolas", 9), anchor=tk.W,
        )
        self.reg_result_label.pack(side=tk.LEFT, fill=tk.X, expand=True)

        # Row 2: Quick-access buttons
        row2 = tk.Frame(frame, bg=THEME["card"])
        row2.pack(fill=tk.X, padx=12, pady=(0, 4))

        tk.Label(row2, text="Quick:", bg=THEME["card"],
                 fg=THEME["text_dim"], font=("Segoe UI", 8)).pack(side=tk.LEFT)

        for reg_addr, reg_label in QUICK_REGS:
            btn = tk.Button(
                row2, text=reg_label,
                command=lambda a=reg_addr: self._quick_reg_read(a),
                bg=THEME["bg"], fg=THEME["text_sec"],
                activebackground="#2D3748", activeforeground=THEME["text"],
                relief=tk.FLAT, bd=0,
                padx=6, pady=1,
                font=("Segoe UI", 8),
                cursor="hand2",
            )
            btn.pack(side=tk.LEFT, padx=(2, 0))
            btn.bind("<Enter>", lambda e, c=THEME["bg"]: e.widget.config(bg="#2D3748"))
            btn.bind("<Leave>", lambda e, c=THEME["bg"]: e.widget.config(bg=c))

    def _build_log_panel(self):
        """Bottom log panel."""
        log_frame = tk.Frame(self, bg=THEME["card"])
        log_frame.pack(fill=tk.X, padx=8, pady=(2, 8))

        # Header
        header = tk.Frame(log_frame, bg=THEME["card"])
        header.pack(fill=tk.X, padx=12, pady=(6, 2))

        tk.Label(header, text="EVENT LOG", bg=THEME["card"],
                 fg=THEME["text_sec"], font=("Segoe UI", 9, "bold")).pack(side=tk.LEFT)

        self.clear_log_btn = self._med_button(
            header, "✕ Clear", self._clear_log,
            bg="#1F2937", w_chars=8, font_size=8,
        )
        self.clear_log_btn.pack(side=tk.RIGHT, padx=(4, 0))

        self.export_csv_btn = self._med_button(
            header, "⬇ Export CSV", self._export_csv,
            bg="#1F2937", w_chars=12, font_size=8,
        )
        self.export_csv_btn.pack(side=tk.RIGHT)

        # Log text widget
        self.log_text = scrolledtext.ScrolledText(
            log_frame, height=8, bg=THEME["bg"], fg=THEME["text_sec"],
            insertbackground=THEME["text"], font=("Consolas", 9),
            borderwidth=0, highlightthickness=0,
            padx=10, pady=6,
        )
        self.log_text.pack(fill=tk.X, padx=8, pady=(2, 8))
        self.log_text.config(state=tk.DISABLED)

    def _med_button(self, parent, text, command, bg="#1F2937",
                    w_chars=None, font_size=9):
        """Styled medical theme button."""
        btn = tk.Button(
            parent, text=text, command=command,
            bg=bg, fg=THEME["text"],
            activebackground="#2D3748", activeforeground=THEME["text"],
            relief=tk.FLAT, borderwidth=0,
            padx=10, pady=4,
            font=("Segoe UI", font_size),
            cursor="hand2",
        )
        if w_chars:
            btn.config(width=w_chars)
        # Hover effects
        btn.bind("<Enter>", lambda e, c=bg: btn.config(bg="#2D3748"))
        btn.bind("<Leave>", lambda e, c=bg: btn.config(bg=c))
        return btn

    # ── Config Persistence ──────────────────────────────────────────────────

    def _load_config(self):
        """Restore last-used serial port from JSON config file."""
        try:
            path = os.path.join(os.path.dirname(__file__), CONFIG_FILE)
            with open(path, "r") as f:
                cfg = json.load(f)
            self.saved_port = cfg.get("last_port", "")
        except (FileNotFoundError, json.JSONDecodeError):
            self.saved_port = ""

    def _save_config(self):
        """Save last-used serial port to JSON config file."""
        try:
            path = os.path.join(os.path.dirname(__file__), CONFIG_FILE)
            with open(path, "w") as f:
                json.dump({"last_port": self.saved_port}, f)
        except OSError:
            pass

    # ── Serial Port ─────────────────────────────────────────────────────────

    def _scan_ports(self):
        """Scan for available serial ports."""
        ports = serial.tools.list_ports.comports()
        port_list = [f"{p.device} - {p.description}" for p in ports]
        self.port_combo["values"] = port_list if port_list else ["No ports found"]
        if port_list:
            # Restore last-used port if still available
            if self.saved_port and self.saved_port in port_list:
                self.port_var.set(self.saved_port)
            else:
                # Pre-select the first STMicroelectronics or CP210x port if found
                preferred = [p for p in port_list if "STMicro" in p or "CP210" in p or "CH340" in p or "USB" in p]
                if preferred:
                    self.port_var.set(preferred[0])
                else:
                    self.port_var.set(port_list[0])

    def _get_port_device(self):
        """Extract device path from combo selection."""
        val = self.port_var.get()
        if not val or val == "No ports found":
            return None
        return val.split(" - ")[0]

    # ── Connect / Disconnect ──────────────────────────────────────────────

    def _toggle_connect(self):
        if self.connected:
            self._disconnect()
        else:
            self._connect()

    def _connect(self):
        device = self._get_port_device()
        if not device:
            self._log("ERROR", "No serial port selected")
            return

        try:
            baud = int(self.baud_var.get())
        except ValueError:
            baud = DEFAULT_BAUD

        self.reader = SerialReader(device, baud, self.data_queue)
        self.reader.start()
        self._log("INFO", f"Connecting to {device} @ {baud} baud...")

    def _disconnect(self):
        self._stop_recording()
        if self.reader:
            self.reader.stop()
            self.reader = None

        self.connected = False
        self._update_connection_ui(False)
        self._log("INFO", "Disconnected")

    def _on_connected(self):
        self.connected = True
        self._update_connection_ui(True)
        # Remember this port for next launch
        self.saved_port = self.port_var.get()
        self._save_config()
        self._log("OK", f"Connected to {self._get_port_device()}")
        self._log("INFO", "Waiting for sensor data...")

    def _on_disconnected(self):
        self.connected = False
        self._update_connection_ui(False)
        self._stop_recording()

    def _on_serial_error(self, msg):
        self._log("ERROR", msg)
        self._disconnect()

    def _update_connection_ui(self, connected):
        if connected:
            self.connect_btn.config(text="🔌  Disconnect", bg="#DA3633")
            self.status_label.config(text="Connected", fg=THEME["success"])
            self._draw_status_dot("connected")
            self.port_combo.config(state="disabled")
            self.baud_combo.config(state="disabled")
            self.refresh_btn.config(state="disabled")
        else:
            self.connect_btn.config(text="🔗  Connect", bg="#238636")
            self.status_label.config(text="Disconnected", fg=THEME["text_sec"])
            self._draw_status_dot("disconnected")
            self.port_combo.config(state="readonly")
            self.baud_combo.config(state="readonly")
            self.refresh_btn.config(state="normal")

    def _draw_status_dot(self, state):
        self.status_indicator.delete("dot")
        r = 5
        cx, cy = 6, 6
        if state == "connected":
            color = THEME["success"]
        elif state == "error":
            color = THEME["warn"]
        else:
            color = THEME["text_dim"]
        self.status_indicator.create_oval(
            cx - r, cy - r, cx + r, cy + r,
            fill=color, outline=color, tags="dot",
        )

    # ── Recording ─────────────────────────────────────────────────────────

    def _toggle_record(self):
        if not self.connected:
            self._log("WARN", "Connect to sensor first")
            return

        if self.recording:
            self._stop_recording()
        else:
            self._start_recording()

    def _start_recording(self):
        script_dir = os.path.dirname(os.path.abspath(__file__))
        filename = os.path.join(script_dir, f"max30102_recording_{datetime.now():%Y%m%d_%H%M%S}.csv")
        try:
            self.record_file = open(filename, "w", newline="", encoding="utf-8")
            self.record_writer = csv.writer(self.record_file)
            self.record_writer.writerow(["Timestamp", "Count", "IR_raw", "Red_raw", "BPM", "SpO2"])
            self.record_file.flush()
            self.recording = True
            self.record_btn.config(text="■  Stop", bg="#DA3633")
            self._log("INFO", f"Recording started → {filename}")
        except Exception as e:
            self._log("ERROR", f"Cannot start recording: {e}")

    def _stop_recording(self):
        if self.record_file:
            try:
                self.record_file.close()
            except Exception:
                pass
            self.record_file = None
            self.record_writer = None
        self.recording = False
        self.record_btn.config(text="●  Record", bg="#1F2937")
        self._log("INFO", "Recording stopped")

    # ── Data Processing ───────────────────────────────────────────────────

    def _process_queue(self):
        """Process data from the serial reader queue (called via tk.after)."""
        try:
            while True:
                msg_type, content = self.data_queue.get_nowait()

                if msg_type == "__status__":
                    if content == "connected":
                        self._on_connected()
                    elif content == "disconnected":
                        self._on_disconnected()
                elif msg_type == "__error__":
                    self._on_serial_error(content)
                elif msg_type == "data":
                    self._handle_line(content)
        except queue.Empty:
            pass
        finally:
            self.after(50, self._process_queue)

    def _handle_line(self, line: str):
        """Parse a single line from serial."""
        if not line:
            return

        # Command response from firmware
        if line.startswith("[CMD] "):
            self._handle_command_response(line[6:].strip())
            return

        # Temperature line
        if line.startswith("# TEMP:"):
            try:
                val = float(line.split(":")[1].strip().split()[0])
                self.last_temp = val
                self._update_temp_display(val)
                self._log("TEMP", f"{val:.1f} °C")
            except (ValueError, IndexError):
                self._log("DATA", f"Parse fail: {line}")
            return

        # Boot/status banner lines
        if line.startswith("[OK]") or line.startswith("[FAIL]"):
            tag = "OK" if line.startswith("[OK]") else "FAIL"
            self._log(tag, line)
            return

        if line.startswith("[INFO]"):
            self._log("INFO", line)
            return

        # Skip other comment/header lines silently
        if line.startswith("#") or line.startswith("---") or line.startswith("Count") or line.startswith("=") or line.startswith("==="):
            return

        # CSV data: Count,IR,Red
        try:
            parts = line.split(",")
            if len(parts) >= 3:
                count = int(parts[0].strip())
                ir = int(parts[1].strip())
                red = int(parts[2].strip())
            else:
                return
        except (ValueError, IndexError):
            return

        self.last_ir = ir
        self.last_red = red
        self.sample_count = count

        # Update signal processing
        self.hr_estimator.add_sample(float(ir))
        self.spo2_estimator.add_sample(float(ir), float(red))

        # Dynamically estimate sample rate from first 50 samples
        if self.sample_count == 1:
            self._sample_rate_t0 = time.time()
        elif self.sample_count == 50:
            elapsed = time.time() - self._sample_rate_t0
            estimated_fs = 50.0 / elapsed if elapsed > 0 else self.fs
            if 30 < estimated_fs < 500:
                self.fs = estimated_fs
                self.hr_estimator.set_fs(self.fs)
                self.spo2_estimator.set_fs(self.fs)

        if self.sample_count % 5 == 0:
            self.current_bpm = self.hr_estimator.estimate_bpm()
            self.bpm_history.append(self.current_bpm)
            self.current_sq = self.hr_estimator.estimate_sq()
            self.sq_history.append(self.current_sq)
            self.current_spo2 = self.spo2_estimator.estimate_spo2()
            self.spo2_history.append(self.current_spo2)

        # Update numerical displays
        self._update_vital_displays()

        # Add to plot buffers
        t = self.sample_count / self.fs
        self.t_samples.append(t)
        self.ir_samples.append(float(ir))
        self.red_samples.append(float(red))

        # Recording
        if self.recording and self.record_writer:
            ts = datetime.now().isoformat()
            bpm_val = round(self.current_bpm, 1) if self.current_bpm > 0 else ""
            spo2_val = round(self.current_spo2, 1) if self.current_spo2 > 0 else ""
            self.record_writer.writerow([ts, count, ir, red, bpm_val, spo2_val])
            self.record_file.flush()

    def _update_vital_displays(self):
        """Update all numeric displays."""
        # BPM (median of last 25 estimates for stability)
        bpm_val = self.current_bpm
        if bpm_val > 0:
            avg_bpm = np.median(self.bpm_history) if self.bpm_history else bpm_val
            self.current_bpm_big.config(
                text=f"{avg_bpm:.0f}",
                fg=THEME["accent"] if avg_bpm < 100 else THEME["warn"],
            )
        else:
            self.current_bpm_big.config(text="--", fg=THEME["text_dim"])

        # Signal Quality
        sq_val = self.current_sq
        if sq_val > 0:
            avg_sq = np.median(self.sq_history) if self.sq_history else sq_val
            if avg_sq > 0.3:
                sq_emoji = "🟢"
                sq_color = THEME["success"]
            elif avg_sq > 0.1:
                sq_emoji = "🟡"
                sq_color = THEME["warn_dim"]
            else:
                sq_emoji = "🔴"
                sq_color = THEME["warn"]
            self.current_sq_label.config(
                text=f"{sq_emoji}  SQ: {avg_sq:.2f}%", fg=sq_color,
            )
        else:
            self.current_sq_label.config(text="--", fg=THEME["text_dim"])

        # SpO2 (median of last 25 estimates)
        spo2_val = self.current_spo2
        if spo2_val > 0:
            avg_spo2 = np.median(self.spo2_history) if self.spo2_history else spo2_val
            sp_color = THEME["accent"] if avg_spo2 >= 95 else (THEME["warn"] if avg_spo2 < 90 else THEME["warn_dim"])
            self.current_spo2_big.config(text=f"{avg_spo2:.0f}", fg=sp_color)
        else:
            self.current_spo2_big.config(text="--", fg=THEME["text_dim"])

        # Mini values
        self.current_mini_ir.config(text=f"{self.last_ir}")
        self.current_mini_red.config(text=f"{self.last_red}")

        # Sample count
        self.sample_count_label.config(text=f"Samples: {self.sample_count}")

    def _update_temp_display(self, temp: float):
        self.current_temp_big.config(text=f"{temp:.1f}", fg=THEME["accent2"])

    # ── Register Command Handling ─────────────────────────────────

    def _reg_read_cmd(self):
        """Read register button handler."""
        try:
            addr_str = self.reg_addr_var.get().strip()
            addr = int(addr_str, 16)
            if not (0x00 <= addr <= 0xFF):
                raise ValueError
        except ValueError:
            self.reg_result_var.set("Invalid address")
            self.reg_result_label.config(fg=THEME["warn"])
            return
        cmd = f"CMD:REG_READ:{addr:02X}"
        if self.reader and self.reader.send_command(cmd):
            self._log("CMD", f"→ {cmd}")

    def _reg_write_cmd(self):
        """Write register button handler."""
        try:
            addr_str = self.reg_addr_var.get().strip()
            val_str = self.reg_val_var.get().strip()
            addr = int(addr_str, 16)
            value = int(val_str, 16)
            if not (0x00 <= addr <= 0xFF) or not (0x00 <= value <= 0xFF):
                raise ValueError
        except ValueError:
            self.reg_result_var.set("Invalid address or value")
            self.reg_result_label.config(fg=THEME["warn"])
            return
        cmd = f"CMD:REG_WRITE:{addr:02X}:{value:02X}"
        if self.reader and self.reader.send_command(cmd):
            self._log("CMD", f"→ {cmd}")

    def _quick_reg_read(self, addr: int):
        """Quick-access register read."""
        self.reg_addr_var.set(f"{addr:02X}")
        cmd = f"CMD:REG_READ:{addr:02X}"
        if self.reader and self.reader.send_command(cmd):
            self._log("CMD", f"→ {cmd}")

    def _handle_command_response(self, resp: str):
        """Parse [CMD] response from firmware."""
        self._log("CMD", f"← {resp}")

        if resp.startswith("REG_READ:"):
            body = resp[9:]
            if "=" in body:
                addr_str, val_str = body.split("=", 1)
                try:
                    addr = int(addr_str, 16)
                    value = int(val_str, 16)
                    self._update_reg_display(addr, value)
                except ValueError:
                    pass
            else:
                self.reg_result_var.set(f"Read: {body}")
                self.reg_result_label.config(fg=THEME["warn"])

        elif resp.startswith("REG_WRITE:"):
            body = resp[10:]
            if ":" in body:
                addr_str, status = body.split(":", 1)
                try:
                    addr = int(addr_str, 16)
                    name = REGISTER_NAMES.get(addr, f"0x{addr:02X}")
                    if status == "OK":
                        self.reg_result_var.set(f"Write {name}: OK")
                        self.reg_result_label.config(fg=THEME["success"])
                    else:
                        self.reg_result_var.set(f"Write {name}: {status}")
                        self.reg_result_label.config(fg=THEME["warn"])
                except ValueError:
                    pass

        elif resp.startswith("TEMP="):
            try:
                temp = float(resp[5:])
                self._update_temp_display(temp)
                self.reg_result_var.set(f"Temp: {temp:.1f}°C")
                self.reg_result_label.config(fg=THEME["accent2"])
            except ValueError:
                pass

    def _update_reg_display(self, addr: int, value: int):
        """Update register read result display."""
        name = REGISTER_NAMES.get(addr, f"REG_0x{addr:02X}")
        self.reg_val_var.set(f"{value:02X}")
        interpretation = self._interpret_reg_value(addr, value)
        if interpretation:
            self.reg_result_var.set(f"{name} (0x{addr:02X}) = 0x{value:02X}  |  {interpretation}")
        else:
            self.reg_result_var.set(f"{name} (0x{addr:02X}) = 0x{value:02X}")
        self.reg_result_label.config(fg=THEME["accent"])

    @staticmethod
    def _interpret_reg_value(addr: int, value: int) -> str:
        """Human-readable interpretation of register values."""
        if addr == 0x09:  # MODE_CONFIG
            mode = value & 0x07
            modes = {0: "Standby", 1: "Red only", 2: "Red+IR (SpO2)", 3: "Multi-LED"}
            parts = [f"Mode={modes.get(mode, f'0x{mode:X}')}"]
            if value & 0x80:
                parts.append("SHDN")
            if value & 0x40:
                parts.append("RESET")
            return " | ".join(parts)
        elif addr == 0x0A:  # SPO2_CONFIG
            adc_rge = (value >> 5) & 0x03
            sr = (value >> 2) & 0x07
            pw = value & 0x03
            ranges = {0: "2048nA", 1: "4096nA", 2: "8192nA", 3: "16384nA"}
            rates = {0: "50sps", 1: "100sps", 2: "200sps", 3: "400sps",
                     4: "800sps", 5: "1000sps"}
            widths = {0: "69us(15bit)", 1: "118us(16bit)",
                      2: "215us(17bit)", 3: "411us(18bit)"}
            return f"ADC={ranges.get(adc_rge,'?')} SR={rates.get(sr,'?')} PW={widths.get(pw,'?')}"
        elif addr == 0x08:  # FIFO_CONFIG
            avg = (value >> 5) & 0x07
            avgs = {0: "1", 1: "2", 2: "4", 3: "8", 4: "16", 5: "32"}
            roll = "Y" if value & 0x10 else "N"
            afull = value & 0x0F
            return f"Avg={avgs.get(avg,'?')} Roll={roll} AFULL={afull}"
        elif addr == 0xFF:
            known = "MAX30102" if value == 0x15 else "UNKNOWN"
            return f"Part ID: 0x{value:02X} ({known})"
        elif addr == 0xFE:
            return f"Rev ID: 0x{value:02X}"
        elif addr in (0x0C, 0x0D):
            current_ma = (value / 255.0) * 50.0
            return f"{current_ma:.1f} mA"
        elif addr == 0x00:
            parts = []
            if value & 0x80: parts.append("FIFO_FULL")
            if value & 0x40: parts.append("NEW_DATA")
            if value & 0x01: parts.append("PWR_RDY")
            return ", ".join(parts) if parts else "No status flags"
        elif addr == 0x01:
            parts = []
            if value & 0x02: parts.append("TEMP_RDY")
            return ", ".join(parts) if parts else "No status flags"
        elif addr == 0x04:
            return f"FIFO Write Pointer: {value}"
        elif addr == 0x06:
            return f"FIFO Read Pointer: {value}"
        return ""

    # ── Plot Update ─────────────────────────────────────────────────────

    def _update_plot(self, frame):
        """Called by matplotlib animation."""
        if not self.ir_samples:
            return self.ir_line, self.red_line

        t_arr = np.array(self.t_samples)
        ir_arr = np.array(self.ir_samples)
        red_arr = np.array(self.red_samples)

        if len(t_arr) < 2:
            return self.ir_line, self.red_line

        # Time window: last N seconds
        t_max = t_arr[-1]
        t_min = max(0, t_max - self.plot_duration)
        mask = t_arr >= t_min

        if np.sum(mask) < 2:
            return self.ir_line, self.red_line

        t_plot = t_arr[mask]
        ir_plot = ir_arr[mask]
        red_plot = red_arr[mask]

        # Shift time so most recent is at plot_duration
        t_shifted = t_plot - t_min

        self.ir_line.set_data(t_shifted, ir_plot)
        self.red_line.set_data(t_shifted, red_plot)

        self.ax.set_xlim(0, self.plot_duration)

        if self.autoscale_var.get():
            all_vals = np.concatenate([ir_plot, red_plot])
            if len(all_vals) > 0:
                data_min = np.min(all_vals)
                data_max = np.max(all_vals)
                margin = max(200, (data_max - data_min) * 0.1)
                self.ax.set_ylim(data_min - margin, data_max + margin)
        else:
            # Auto-adjust based on IR range
            pass

        return self.ir_line, self.red_line

    # ── Logging ───────────────────────────────────────────────────────────

    def _log(self, level: str, message: str):
        """Add a log entry to the log panel."""
        timestamp = datetime.now().strftime("%H:%M:%S")

        level_colors = {
            "OK": THEME["success"],
            "INFO": THEME["accent2"],
            "WARN": THEME["warn_dim"],
            "ERROR": THEME["warn"],
            "DATA": THEME["text_sec"],
            "FAIL": THEME["warn"],
            "TEMP": THEME["accent2"],
        }
        color = level_colors.get(level, THEME["text_sec"])
        tag = f"lvl_{level}_{id(message)}"

        self.log_text.config(state=tk.NORMAL)
        self.log_text.insert(tk.END, f"[{timestamp}] ", "timestamp")
        self.log_text.insert(tk.END, f"[{level:5s}] ", ("level", color))
        self.log_text.insert(tk.END, f"{message}\n", "message")

        # Apply tags
        self.log_text.tag_config("timestamp", foreground=THEME["text_dim"], font=("Consolas", 8))
        self.log_text.tag_config("level", foreground=color, font=("Consolas", 8, "bold"))
        self.log_text.tag_config("message", foreground=THEME["text_sec"], font=("Consolas", 9))

        # Trim old lines
        line_count = int(self.log_text.index("end-1c").split(".")[0])
        if line_count > MAX_LOG_LINES:
            self.log_text.delete("1.0", f"{line_count - MAX_LOG_LINES}.0")

        self.log_text.see(tk.END)
        self.log_text.config(state=tk.DISABLED)

    def _clear_log(self):
        self.log_text.config(state=tk.NORMAL)
        self.log_text.delete("1.0", tk.END)
        self.log_text.config(state=tk.DISABLED)
        self._log("INFO", "Log cleared")

    def _export_csv(self):
        """Export current buffer data to CSV."""
        if len(self.ir_samples) < 10:
            self._log("WARN", "Not enough data to export")
            return

        script_dir = os.path.dirname(os.path.abspath(__file__))
        filename = os.path.join(script_dir, f"max30102_export_{datetime.now():%Y%m%d_%H%M%S}.csv")
        try:
            with open(filename, "w", newline="", encoding="utf-8") as f:
                w = csv.writer(f)
                w.writerow(["Time_s", "IR_raw", "Red_raw", "BPM", "SpO2"])
                for i in range(len(self.t_samples)):
                    w.writerow([
                        f"{self.t_samples[i]:.3f}",
                        int(self.ir_samples[i]),
                        int(self.red_samples[i]),
                        f"{self.current_bpm:.1f}" if self.current_bpm > 0 else "",
                        f"{self.current_spo2:.1f}" if self.current_spo2 > 0 else "",
                    ])
            self._log("OK", f"Exported {len(self.t_samples)} samples → {filename}")
        except Exception as e:
            self._log("ERROR", f"Export failed: {e}")

    # ── Cleanup ───────────────────────────────────────────────────────────

    def _on_close(self):
        """Clean shutdown."""
        self._disconnect()
        self._stop_recording()
        try:
            self.ani.event_source.stop()
        except Exception:
            pass
        self.destroy()


# ── Entry Point ───────────────────────────────────────────────────────────

if __name__ == "__main__":
    app = MAX30102Monitor()
    try:
        app.mainloop()
    except KeyboardInterrupt:
        app._on_close()
