#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
BPM Algorithm Comparison Tool
=============================
Load a CSV recording and compare multiple BPM estimation algorithms
side-by-side on the same PPG waveform.
"""

import sys
import os
import numpy as np
import matplotlib
matplotlib.use("TkAgg")
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import tkinter as tk
from tkinter import filedialog, messagebox
from collections import deque
from datetime import datetime

# ── BPM Estimator variants ──────────────────────────────────────────────

def _edge_convolve(data, kernel):
    """Edge-padded convolution (the fix)."""
    win = len(kernel)
    padded = np.pad(data, win // 2, mode='edge')
    return np.convolve(padded, kernel, mode='valid')[:len(data)]


def estimate_bpm_org(ir_data, fs=50.0):
    """Current implementation with edge-padded convolution."""
    data = np.array(ir_data, dtype=float)
    if np.std(data) < 50 or np.max(data) - np.min(data) < 200:
        return 0.0, data, np.array([])

    win = int(fs * 0.5)
    kernel = np.ones(win) / win
    baseline = _edge_convolve(data, kernel)
    ac = data[:len(baseline)] - baseline

    swin = max(3, int(fs * 0.06))
    skernel = np.ones(swin) / swin
    ac_smooth = _edge_convolve(ac, skernel)

    thr = ac_smooth.min() + (ac_smooth.max() - ac_smooth.min()) * 0.6

    half = int(fs * 0.1)
    min_dist = int(fs * 0.3)
    peaks = []
    for i in range(half, len(ac_smooth) - half):
        if ac_smooth[i] > thr and ac_smooth[i] == max(ac_smooth[i-half:i+half+1]):
            if not peaks or i - peaks[-1] >= min_dist:
                peaks.append(i)

    if len(peaks) < 2:
        return 0.0, data, np.array([])

    intervals = np.diff(peaks) / fs
    valid = intervals[(intervals >= 0.3) & (intervals <= 2.0)]
    if len(valid) < 2:
        return 0.0, data, np.array([])

    bpm = 60.0 / np.median(valid)
    return max(30.0, min(220.0, bpm)), data, np.array(peaks)


def estimate_bpm_strict(ir_data, fs=50.0):
    """Stricter signal quality gate: std<150 or range<500 → 0."""
    data = np.array(ir_data, dtype=float)
    if np.std(data) < 150 or np.max(data) - np.min(data) < 500:
        return 0.0, data, np.array([])

    win = int(fs * 0.5)
    kernel = np.ones(win) / win
    baseline = _edge_convolve(data, kernel)
    ac = data[:len(baseline)] - baseline

    swin = max(3, int(fs * 0.06))
    skernel = np.ones(swin) / swin
    ac_smooth = _edge_convolve(ac, skernel)

    thr = ac_smooth.min() + (ac_smooth.max() - ac_smooth.min()) * 0.6

    half = int(fs * 0.1)
    min_dist = int(fs * 0.3)
    peaks = []
    for i in range(half, len(ac_smooth) - half):
        if ac_smooth[i] > thr and ac_smooth[i] == max(ac_smooth[i-half:i+half+1]):
            if not peaks or i - peaks[-1] >= min_dist:
                peaks.append(i)

    if len(peaks) < 2:
        return 0.0, data, np.array([])

    intervals = np.diff(peaks) / fs
    valid = intervals[(intervals >= 0.3) & (intervals <= 2.0)]
    if len(valid) < 2:
        return 0.0, data, np.array([])

    bpm = 60.0 / np.median(valid)
    return max(30.0, min(220.0, bpm)), data, np.array(peaks)


def estimate_bpm_th50(ir_data, fs=50.0):
    """Threshold at 50% of AC range instead of 60%."""
    data = np.array(ir_data, dtype=float)
    if np.std(data) < 50 or np.max(data) - np.min(data) < 200:
        return 0.0, data, np.array([])

    win = int(fs * 0.5)
    kernel = np.ones(win) / win
    baseline = _edge_convolve(data, kernel)
    ac = data[:len(baseline)] - baseline

    swin = max(3, int(fs * 0.06))
    skernel = np.ones(swin) / swin
    ac_smooth = _edge_convolve(ac, skernel)

    thr = ac_smooth.min() + (ac_smooth.max() - ac_smooth.min()) * 0.5

    half = int(fs * 0.1)
    min_dist = int(fs * 0.3)
    peaks = []
    for i in range(half, len(ac_smooth) - half):
        if ac_smooth[i] > thr and ac_smooth[i] == max(ac_smooth[i-half:i+half+1]):
            if not peaks or i - peaks[-1] >= min_dist:
                peaks.append(i)

    if len(peaks) < 2:
        return 0.0, data, np.array([])

    intervals = np.diff(peaks) / fs
    valid = intervals[(intervals >= 0.3) & (intervals <= 2.0)]
    if len(valid) < 2:
        return 0.0, data, np.array([])

    bpm = 60.0 / np.median(valid)
    return max(30.0, min(220.0, bpm)), data, np.array(peaks)


def estimate_bpm_median_filter(ir_data, fs=50.0):
    """Current algorithm + median filter on BPM output (3-value window)."""
    # Run in sliding windows to produce a BPM timeseries, then median filter
    data = np.array(ir_data, dtype=float)
    window_n = int(fs * 4.0)
    step = int(fs * 0.5)

    bpm_values = []
    for start in range(0, len(data) - window_n, step):
        chunk = data[start:start + window_n]
        bpm, _, _ = estimate_bpm_org(chunk, fs)
        bpm_values.append(bpm)

    if not bpm_values or all(b == 0.0 for b in bpm_values):
        # Fallback: run on full data
        return estimate_bpm_org(ir_data, fs)

    # Median filter with window 3
    bpm_arr = np.array(bpm_values)
    filtered = np.convolve(bpm_arr, np.ones(3)/3, mode='same')
    return max(30.0, min(220.0, filtered[-1])), data, np.array([])


def estimate_bpm_all(ir_data, fs=50.0):
    """Combined: strict gate + TH=50% + median filter."""
    data = np.array(ir_data, dtype=float)
    if np.std(data) < 150 or np.max(data) - np.min(data) < 500:
        return 0.0, data, np.array([])

    win = int(fs * 0.5)
    kernel = np.ones(win) / win
    baseline = _edge_convolve(data, kernel)
    ac = data[:len(baseline)] - baseline

    swin = max(3, int(fs * 0.06))
    skernel = np.ones(swin) / swin
    ac_smooth = _edge_convolve(ac, skernel)

    thr = ac_smooth.min() + (ac_smooth.max() - ac_smooth.min()) * 0.5

    half = int(fs * 0.1)
    min_dist = int(fs * 0.3)
    peaks = []
    for i in range(half, len(ac_smooth) - half):
        if ac_smooth[i] > thr and ac_smooth[i] == max(ac_smooth[i-half:i+half+1]):
            if not peaks or i - peaks[-1] >= min_dist:
                peaks.append(i)

    if len(peaks) < 2:
        return 0.0, data, np.array([])

    intervals = np.diff(peaks) / fs
    valid = intervals[(intervals >= 0.3) & (intervals <= 2.0)]
    if len(valid) < 2:
        return 0.0, data, np.array([])

    bpm = 60.0 / np.median(valid)
    return max(30.0, min(220.0, bpm)), data, np.array(peaks)


# ── Variant descriptors ────────────────────────────────────────────────

VARIANTS = [
    ("ORG",      "Current (edge-conv, TH=60%)",        estimate_bpm_org,            "#00E5A0"),
    ("STRICT",   "Strict gate (std>150,range>500)",     estimate_bpm_strict,         "#00B4D8"),
    ("TH50",     "TH=50% of AC range",                 estimate_bpm_th50,           "#FFD700"),
    ("MF",       "Median filter on BPM output",        estimate_bpm_median_filter,  "#FF8C00"),
    ("ALL",      "STRICT + TH50 + MF",                 estimate_bpm_all,            "#FF69B4"),
]


# ── GUI Application ─────────────────────────────────────────────────────

class BPMAnalysisTool(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("BPM Algorithm Comparison Tool")
        self.geometry("1400x900")
        self.configure(bg="#0D1117")

        self.csv_path = None
        self.ir_data = None
        self.red_data = None
        self.fs = 50.0

        self.var_states = {}
        self._build_ui()

    def _build_ui(self):
        # ── Top bar ──
        top = tk.Frame(self, bg="#161B22", padx=10, pady=10)
        top.pack(fill=tk.X)

        tk.Button(top, text="Load CSV", command=self._load_csv,
                  bg="#238636", fg="#E6EDF3", font=("Segoe UI", 10, "bold"),
                  padx=12).pack(side=tk.LEFT)

        self.file_label = tk.Label(top, text="No file loaded",
                                   bg="#161B22", fg="#8B949E",
                                   font=("Segoe UI", 10))
        self.file_label.pack(side=tk.LEFT, padx=10)

        # ── Variant checkboxes ──
        ctrl = tk.Frame(self, bg="#161B22", padx=10, pady=5)
        ctrl.pack(fill=tk.X)

        tk.Label(ctrl, text="Algorithms:", bg="#161B22", fg="#E6EDF3",
                 font=("Segoe UI", 10, "bold")).pack(side=tk.LEFT)

        self.result_labels = {}
        for key, name, func, color in VARIANTS:
            var = tk.BooleanVar(value=(key == "ORG"))
            self.var_states[key] = var
            cb = tk.Checkbutton(ctrl, text=f"  {key}", variable=var,
                                bg="#161B22", fg="#E6EDF3",
                                selectcolor="#0D1117",
                                activebackground="#161B22",
                                activeforeground=color,
                                font=("Consolas", 10, "bold"))
            cb.pack(side=tk.LEFT, padx=(4, 0))
            # Color stripe
            stripe = tk.Frame(ctrl, bg=color, width=4, height=20)
            stripe.pack(side=tk.LEFT, padx=(1, 2))

            lbl = tk.Label(ctrl, text="--", bg="#161B22", fg="#8B949E",
                           font=("Consolas", 9))
            lbl.pack(side=tk.LEFT, padx=(0, 16))
            self.result_labels[key] = lbl

        tk.Button(ctrl, text="Re-run", command=self._run_analysis,
                  bg="#1C2333", fg="#E6EDF3", font=("Segoe UI", 10),
                  padx=12).pack(side=tk.LEFT, padx=10)

        # ── Matplotlib figure ──
        self.figure = Figure(figsize=(14, 8), dpi=100,
                             facecolor="#0D1117")
        self.ax_wave = self.figure.add_subplot(211, facecolor="#0D1117")
        self.ax_bar = self.figure.add_subplot(212, facecolor="#0D1117")

        self.canvas = FigureCanvasTkAgg(self.figure, master=self)
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

        # Style axes
        for ax in [self.ax_wave, self.ax_bar]:
            ax.tick_params(colors="#8B949E")
            ax.spines["bottom"].set_color("#30363D")
            ax.spines["left"].set_color("#30363D")
            ax.spines["top"].set_visible(False)
            ax.spines["right"].set_visible(False)

        self.ax_wave.set_title("PPG Waveform with Detected Peaks",
                               color="#E6EDF3", fontsize=12, fontweight="bold")
        self.ax_wave.set_xlabel("Sample", color="#8B949E")
        self.ax_wave.set_ylabel("IR Raw Value", color="#8B949E")
        self.ax_wave.grid(True, color="#21262D", linewidth=0.5)

        self.ax_bar.set_title("BPM by Algorithm Variant",
                              color="#E6EDF3", fontsize=12, fontweight="bold")
        self.ax_bar.set_ylabel("BPM", color="#8B949E")
        self.ax_bar.set_ylim(0, 220)

        self.figure.tight_layout(pad=3.0)

    def _load_csv(self):
        path = filedialog.askopenfilename(
            title="Select CSV Recording",
            filetypes=[("CSV files", "*.csv"), ("All files", "*.*")],
        )
        if not path:
            return

        try:
            with open(path, "r") as f:
                header = f.readline().strip()
            cols = [c.strip() for c in header.split(",")]

            has_ts = "Timestamp" in cols
            need_cols = len(cols)
            if has_ts:
                # Skip first column (timestamp string), read the rest as floats
                data = np.genfromtxt(path, delimiter=",", skip_header=1,
                                     usecols=range(1, need_cols))
            else:
                data = np.genfromtxt(path, delimiter=",", skip_header=1)

            # Map column indices in the loaded array
            if has_ts:
                ir_idx = cols.index("IR_raw") - 1
                red_idx = cols.index("Red_raw") - 1
            else:
                ir_idx = cols.index("IR_raw")
                red_idx = cols.index("Red_raw")

            self.ir_data = data[:, ir_idx]
            self.red_data = data[:, red_idx] if data.shape[1] > red_idx else None

        except Exception as e:
            messagebox.showerror("Error", f"Failed to load CSV:\n{e}")
            return

        self.csv_path = path
        self.file_label.config(text=os.path.basename(path), fg="#00E5A0")
        self._run_analysis()

    def _run_analysis(self):
        if self.ir_data is None or len(self.ir_data) < 200:
            messagebox.showwarning("No Data", "Load a CSV file first.")
            return

        # Use a consistent window: last 500 samples (10s @ 50Hz)
        window = min(500, len(self.ir_data))
        ir = self.ir_data[-window:]

        # Clear axes
        self.ax_wave.clear()
        self.ax_bar.clear()

        # Style
        self.ax_wave.set_facecolor("#0D1117")
        self.ax_bar.set_facecolor("#0D1117")
        self.ax_wave.tick_params(colors="#8B949E")
        self.ax_bar.tick_params(colors="#8B949E")
        for sp in ["bottom", "left"]:
            self.ax_wave.spines[sp].set_color("#30363D")
            self.ax_bar.spines[sp].set_color("#30363D")
        for sp in ["top", "right"]:
            self.ax_wave.spines[sp].set_visible(False)
            self.ax_bar.spines[sp].set_visible(False)
        self.ax_wave.grid(True, color="#21262D", linewidth=0.5)
        self.ax_bar.grid(True, axis="y", color="#21262D", linewidth=0.5)

        # Plot raw IR waveform
        self.ax_wave.plot(ir, color="#30363D", linewidth=0.5, alpha=0.3,
                          label="IR raw")

        # Run selected variants
        results = []
        bpm_values = []

        for key, name, func, color in VARIANTS:
            if not self.var_states[key].get():
                continue

            bpm, data, peaks = func(ir, self.fs)
            results.append((key, name, func, color, bpm, data, peaks))
            bpm_values.append(bpm)

            # Update text label
            bpm_str = f"{bpm:.1f}" if bpm > 0 else "--"
            self.result_labels[key].config(text=f"{bpm_str} BPM", fg=color)

            # Plot peaks on waveform
            if len(peaks) > 0:
                self.ax_wave.plot(peaks, ir[peaks], "o", color=color,
                                  markersize=5, label=f"{key} ({bpm_str})",
                                  alpha=0.8)

        # BPM bar chart
        labels = [r[0] for r in results]
        values = [r[4] for r in results]
        colors = [r[3] for r in results]
        bars = self.ax_bar.bar(labels, values, color=colors, alpha=0.8,
                               edgecolor="#30363D", linewidth=0.5)
        for bar, val in zip(bars, values):
            if val > 0:
                self.ax_bar.text(bar.get_x() + bar.get_width()/2,
                                 bar.get_height() + 2,
                                 f"{val:.1f}", ha="center", va="bottom",
                                 color="#E6EDF3", fontsize=10, fontweight="bold")

        self.ax_bar.set_ylim(0, max(220, max(values) * 1.2) if values else 220)

        self.ax_wave.legend(loc="upper right", fontsize=8,
                            facecolor="#161B22", edgecolor="#30363D",
                            labelcolor="#E6EDF3")

        self.ax_wave.set_title("PPG Waveform with Detected Peaks",
                               color="#E6EDF3", fontsize=12, fontweight="bold")
        self.ax_wave.set_xlabel("Sample", color="#8B949E")
        self.ax_wave.set_ylabel("IR Raw Value", color="#8B949E")
        self.ax_bar.set_title("BPM by Algorithm Variant",
                              color="#E6EDF3", fontsize=12, fontweight="bold")
        self.ax_bar.set_ylabel("BPM", color="#8B949E")

        self.figure.tight_layout(pad=3.0)
        self.canvas.draw()


if __name__ == "__main__":
    # CLI mode: if CSV path provided as argument, print results to terminal
    if len(sys.argv) > 1:
        path = sys.argv[1]
        fs = 50.0

        try:
            with open(path, "r") as f:
                header = f.readline().strip()
            cols = [c.strip() for c in header.split(",")]
            has_ts = "Timestamp" in cols
            if has_ts:
                data = np.genfromtxt(path, delimiter=",", skip_header=1,
                                     usecols=range(1, len(cols)))
                ir_idx = cols.index("IR_raw") - 1
            else:
                data = np.genfromtxt(path, delimiter=",", skip_header=1)
                ir_idx = cols.index("IR_raw")
            ir_data = data[:, ir_idx]
        except Exception as e:
            print(f"[ERROR] {e}", file=sys.stderr)
            sys.exit(1)

        # Full window + last 500 samples
        print(f"File: {os.path.basename(path)}")
        print(f"Total samples: {len(ir_data)}")
        print(f"{'Variant':<8} {'BPM':>8} {'Peaks':>6} {'Std':>10} {'Range':>10} {'Mod%':>8}")
        print("-" * 60)

        for key, name, func, color in VARIANTS:
            bpm, _, peaks = func(ir_data, fs)
            std = np.std(ir_data)
            rng = np.max(ir_data) - np.min(ir_data)
            mod_pct = (np.max(ir_data[-500:]) - np.min(ir_data[-500:])) / np.mean(ir_data[-500:]) * 100
            bpm_str = f"{bpm:.1f}" if bpm > 0 else "--"
            print(f"{key:<8} {bpm_str:>8} {len(peaks):>6} {std:>10.1f} {rng:>10.1f} {mod_pct:>8.4f}")

        print()
        # Per-window analysis (last 500 samples)
        window = min(500, len(ir_data))
        ir_win = ir_data[-window:]
        print(f"Window analysis (last {window} samples, {window/fs:.1f}s):")
        print(f"{'Variant':<8} {'BPM':>8} {'Peaks':>6}")
        print("-" * 30)
        for key, name, func, color in VARIANTS:
            bpm, _, peaks = func(ir_win, fs)
            bpm_str = f"{bpm:.1f}" if bpm > 0 else "--"
            print(f"{key:<8} {bpm_str:>8} {len(peaks):>6}")
    else:
        app = BPMAnalysisTool()
        app.mainloop()
