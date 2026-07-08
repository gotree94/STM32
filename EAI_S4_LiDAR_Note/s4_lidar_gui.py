"""
EAI / YDLIDAR S4 LiDAR - GUI Point Cloud Viewer
Real-time 2D visualization with tkinter + matplotlib
"""

import tkinter as tk
from tkinter import ttk, messagebox
import threading
import time
import struct
import math
from datetime import datetime

import serial
import serial.tools.list_ports
import numpy as np
import matplotlib
matplotlib.use("TkAgg")
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk

# ─── Protocol Constants ──────────────────────────────────────────
CMD_SCAN = b'\xA5\x60'
CMD_STOP = b'\xA5\x65'
SDK_UNIT64 = 64.0
SDK_ANGLE360 = 360.0
MM_PER_UNIT = 4.0

# ─── Protocol Functions ─────────────────────────────────────────

def calc_checksum(packet, lsn):
    ph = struct.unpack_from('<H', packet, 0)[0]
    fsa = struct.unpack_from('<H', packet, 4)[0]
    lsa = struct.unpack_from('<H', packet, 6)[0]
    ct = packet[2]
    cs = ph ^ fsa
    for i in range(lsn):
        si = struct.unpack_from('<H', packet, 10 + i * 2)[0]
        cs ^= si
    cs ^= (lsn << 8) | ct
    cs ^= lsa
    return cs

def verify_checksum(packet, lsn):
    if len(packet) < 10 + lsn * 2:
        return False
    cs_stored = struct.unpack_from('<H', packet, 8)[0]
    return cs_stored == calc_checksum(packet, lsn)

def parse_angle(raw):
    return (raw >> 1) / SDK_UNIT64

def parse_distance(si):
    return si >> 2, (si >> 2) / (MM_PER_UNIT * 1000.0)

def angle_correction_triangle(dist_mm):
    if dist_mm == 0:
        return 0.0
    ca = math.atan(21.8 * (155.3 - dist_mm) / (155.3 * dist_mm))
    return ca * 180.0 / math.pi

def find_packet_start(data):
    for i in range(len(data) - 1):
        if data[i] == 0xAA and data[i + 1] == 0x55:
            return i
    return -1

def parse_packet(packet):
    if len(packet) < 10:
        return None
    ph = struct.unpack_from('<H', packet, 0)[0]
    if ph != 0x55AA:
        return None
    ct = packet[2]
    lsn = packet[3]
    if len(packet) < 10 + lsn * 2:
        return None
    fsa_raw = struct.unpack_from('<H', packet, 4)[0]
    lsa_raw = struct.unpack_from('<H', packet, 6)[0]
    cs_ok = verify_checksum(packet, lsn)
    start_angle = parse_angle(fsa_raw)
    end_angle = parse_angle(lsa_raw)
    if end_angle < start_angle:
        end_angle += SDK_ANGLE360
    is_zero = (ct & 0x01) == 1
    points = []
    for i in range(lsn):
        si = struct.unpack_from('<H', packet, 10 + i * 2)[0]
        dist_mm, dist_m = parse_distance(si)
        if lsn == 1:
            angle = start_angle
        else:
            angle = start_angle + (end_angle - start_angle) * i / (lsn - 1)
        if angle >= SDK_ANGLE360:
            angle -= SDK_ANGLE360
        corr = angle_correction_triangle(dist_mm)
        angle_c = angle + corr
        if angle_c < 0:
            angle_c += SDK_ANGLE360
        elif angle_c >= SDK_ANGLE360:
            angle_c -= SDK_ANGLE360
        points.append((angle_c, dist_m, si >> 1 & 0x01))
    return {
        'is_zero': is_zero,
        'lsn': lsn,
        'cs_ok': cs_ok,
        'points': points,
        'ct': ct,
    }


# ─── GUI Application ────────────────────────────────────────────

class LidarGUI:
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("EAI S4 LiDAR Viewer")
        self.root.geometry("1100x700")
        self.root.protocol("WM_DELETE_WINDOW", self.on_close)

        # State
        self.serial_port = None
        self.running = False
        self.read_thread = None
        self.buffer = bytearray()
        self.points_buffer = []          # (angle_deg, dist_m, intensity)
        self.max_points = 2000
        self.packet_count = 0
        self.sample_count = 0
        self.rotation_count = 0
        self.last_second = time.monotonic()
        self.pps = 0
        self.fps_counter = 0
        self.scan_hz = 0.0
        self.last_rot_time = 0
        self.cs_errors = 0

        self.build_ui()

        # Matplotlib update timer (50ms)
        self.plot_timer = None
        self.schedule_plot()

    # ─── UI Build ────────────────────────────────────────────────

    def build_ui(self):
        main_frame = ttk.Frame(self.root)
        main_frame.pack(fill=tk.BOTH, expand=True, padx=6, pady=6)

        # Left: control panel
        ctrl = ttk.Frame(main_frame, width=260)
        ctrl.pack(side=tk.LEFT, fill=tk.Y, padx=(0, 6))
        ctrl.pack_propagate(False)

        # Connection frame
        conn_f = ttk.LabelFrame(ctrl, text="Connection", padding=8)
        conn_f.pack(fill=tk.X, pady=(0, 6))

        ttk.Label(conn_f, text="Port:").grid(row=0, column=0, sticky=tk.W)
        self.port_var = tk.StringVar(value="COM6")
        port_combo = ttk.Combobox(conn_f, textvariable=self.port_var,
                                  values=self.get_com_ports(), width=14)
        port_combo.grid(row=0, column=1, padx=(4, 0))
        ttk.Button(conn_f, text="Refresh", command=self.refresh_ports,
                   width=7).grid(row=0, column=2, padx=(4, 0))

        ttk.Label(conn_f, text="Baud:").grid(row=1, column=0, sticky=tk.W, pady=(4, 0))
        self.baud_var = tk.StringVar(value="115200")
        baud_combo = ttk.Combobox(conn_f, textvariable=self.baud_var,
                                  values=["115200", "128000", "153600", "230400"],
                                  width=14)
        baud_combo.grid(row=1, column=1, padx=(4, 0), pady=(4, 0))

        # Buttons
        btn_f = ttk.Frame(ctrl)
        btn_f.pack(fill=tk.X, pady=(0, 4))

        self.btn_connect = ttk.Button(btn_f, text="Connect",
                                      command=self.toggle_connect)
        self.btn_connect.pack(side=tk.LEFT, padx=(0, 4))

        self.btn_clear = ttk.Button(btn_f, text="Clear", command=self.clear_plot)
        self.btn_clear.pack(side=tk.LEFT)

        # Scale control
        scale_f = ttk.Frame(ctrl)
        scale_f.pack(fill=tk.X, pady=(0, 6))

        self.auto_scale = tk.BooleanVar(value=True)
        ttk.Checkbutton(scale_f, text="Auto scale",
                        variable=self.auto_scale,
                        command=self.on_scale_toggle).pack(side=tk.LEFT)

        ttk.Label(scale_f, text="Max (m):").pack(side=tk.LEFT, padx=(8, 2))
        self.max_range_var = tk.StringVar(value="1.0")
        self.max_range_entry = ttk.Entry(scale_f, textvariable=self.max_range_var,
                                         width=5, state="disabled")
        self.max_range_entry.pack(side=tk.LEFT)

        # Status
        stat_f = ttk.LabelFrame(ctrl, text="Status", padding=8)
        stat_f.pack(fill=tk.BOTH, expand=True)

        self.stat_labels = {}
        stats = [
            ("conn", "Connection:"),
            ("pkts", "Packets:"),
            ("pts", "Points:"),
            ("rot", "Rotations:"),
            ("hz", "Scan rate:"),
            ("pps", "Pkts/s:"),
            ("cs", "CS errors:"),
        ]
        for key, label in stats:
            f = ttk.Frame(stat_f)
            f.pack(fill=tk.X, pady=1)
            ttk.Label(f, text=label, width=14, anchor=tk.W).pack(side=tk.LEFT)
            lbl = ttk.Label(f, text="-", anchor=tk.W)
            lbl.pack(side=tk.LEFT, fill=tk.X, expand=True)
            self.stat_labels[key] = lbl

        self.set_status("conn", "Disconnected", "gray")

        # Right: plot
        plot_f = ttk.Frame(main_frame)
        plot_f.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True)

        self.fig = Figure(figsize=(7, 6.5), dpi=100)
        self.fig.patch.set_facecolor('#f0f0f0')
        self.ax = self.fig.add_subplot(111, projection='polar')
        self.ax.set_theta_zero_location('N')
        self.ax.set_theta_direction(-1)
        self.ax.set_ylim(0, 1.0)
        self.ax.set_title("EAI S4 LiDAR - Point Cloud", pad=15)
        self.ax.grid(True, alpha=0.3)

        self.canvas = FigureCanvasTkAgg(self.fig, master=plot_f)
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

        # Toolbar
        toolbar = NavigationToolbar2Tk(self.canvas, plot_f)
        toolbar.update()

    def on_scale_toggle(self):
        state = "disabled" if self.auto_scale.get() else "normal"
        self.max_range_entry.config(state=state)

    # ─── Helpers ─────────────────────────────────────────────────

    def get_com_ports(self):
        return [p.device for p in serial.tools.list_ports.comports()]

    def refresh_ports(self):
        ports = self.get_com_ports()
        combo = self.port_var
        # Find the parent combobox widget
        for w in self.root.winfo_children():
            for c in w.winfo_children():
                for child in c.winfo_children():
                    if isinstance(child, ttk.LabelFrame):
                        for ch in child.winfo_children():
                            if isinstance(ch, ttk.Combobox):
                                ch['values'] = ports

    def set_status(self, key, text, color=None):
        self.stat_labels[key].config(text=text)
        if color:
            self.stat_labels[key].config(foreground=color)

    # ─── Connect / Disconnect ────────────────────────────────────

    def toggle_connect(self):
        if self.running:
            self.stop_scan()
            if self.serial_port and self.serial_port.is_open:
                try:
                    self.serial_port.write(CMD_STOP)
                    time.sleep(0.05)
                    self.serial_port.close()
                except Exception:
                    pass
            self.serial_port = None
            self.btn_connect.config(text="Connect")
            self.set_status("conn", "Disconnected", "gray")
        else:
            self.do_connect()

    def do_connect(self):
        port = self.port_var.get().strip()
        baud = int(self.baud_var.get())
        try:
            ser = serial.Serial(
                port=port, baudrate=baud,
                bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE, timeout=0.05,
            )
        except Exception as e:
            messagebox.showerror("Connection Error", str(e))
            return

        self.serial_port = ser
        self.btn_connect.config(text="Disconnect")
        self.set_status("conn", f"Connected ({port})", "green")

        # Reset state
        self.buffer = bytearray()
        self.points_buffer = []
        self.packet_count = 0
        self.sample_count = 0
        self.rotation_count = 0
        self.cs_errors = 0
        self.last_second = time.monotonic()
        self.pps = 0
        self.fps_counter = 0
        self.scan_hz = 0.0

        # Flush and start
        ser.reset_input_buffer()
        ser.reset_output_buffer()
        time.sleep(0.1)
        ser.write(CMD_SCAN)

        self.running = True
        self.read_thread = threading.Thread(target=self.read_loop, daemon=True)
        self.read_thread.start()

    def stop_scan(self):
        self.running = False
        if self.serial_port and self.serial_port.is_open:
            try:
                self.serial_port.write(CMD_STOP)
            except Exception:
                pass

    # ─── Serial Read Loop ────────────────────────────────────────

    def read_loop(self):
        ser = self.serial_port
        while self.running and ser and ser.is_open:
            try:
                chunk = ser.read(2048)
                if not chunk:
                    time.sleep(0.001)
                    continue
            except Exception:
                break

            self.buffer.extend(chunk)
            self.fps_counter += len(chunk)

            while True:
                ps = find_packet_start(self.buffer)
                if ps < 0:
                    break
                if ps > 0:
                    self.buffer = self.buffer[ps:]

                if len(self.buffer) < 10:
                    break

                lsn = self.buffer[3]
                pkt_len = 10 + lsn * 2
                if len(self.buffer) < pkt_len:
                    break

                pkt = bytes(self.buffer[:pkt_len])
                self.buffer = self.buffer[pkt_len:]

                result = parse_packet(pkt)
                if result is None:
                    continue

                if not result['cs_ok']:
                    self.cs_errors += 1

                self.packet_count += 1
                self.sample_count += result['lsn']

                if result['is_zero']:
                    self.rotation_count += 1
                    now_t = time.monotonic()
                    if self.last_rot_time > 0:
                        dt = now_t - self.last_rot_time
                        if dt > 0:
                            self.scan_hz = 1.0 / dt
                    self.last_rot_time = now_t

                # Add points
                for angle_c, dist_m, intens in result['points']:
                    self.points_buffer.append((angle_c, dist_m, intens))

                # PPS counter
                now = time.monotonic()
                if now - self.last_second >= 1.0:
                    self.pps = int(self.fps_counter / (now - self.last_second))
                    self.fps_counter = 0
                    self.last_second = now

    # ─── Plot Update ─────────────────────────────────────────────

    def schedule_plot(self):
        self.update_plot()
        self.plot_timer = self.root.after(60, self.schedule_plot)

    def update_plot(self):
        if not self.points_buffer:
            return

        # Take all buffered points
        pts = self.points_buffer[:]
        self.points_buffer.clear()

        # Keep only recent points (last N rotations)
        if len(pts) > self.max_points:
            pts = pts[-self.max_points:]

        if not pts:
            return

        angles = np.array([p[0] for p in pts]) * math.pi / 180.0
        dists = np.array([p[1] for p in pts])
        intens = np.array([p[2] for p in pts])

        # Y axis range
        if self.auto_scale.get():
            valid = dists[dists > 0]
            if len(valid) > 0:
                max_d = min(valid.max() * 1.2, 8.0)
                if max_d < 0.1:
                    max_d = 1.0
            else:
                max_d = 1.0
        else:
            try:
                max_d = float(self.max_range_var.get())
            except ValueError:
                max_d = 1.0
            if max_d <= 0:
                max_d = 1.0

        # Clear and redraw
        self.ax.cla()
        self.ax.set_theta_zero_location('N')
        self.ax.set_theta_direction(-1)
        self.ax.set_ylim(0, max_d)
        self.ax.grid(True, alpha=0.3)
        self.ax.set_title(f"EAI S4 LiDAR - {self.sample_count:,} pts / {self.rotation_count} rotations",
                          fontsize=10, pad=15)

        # Color by distance
        colors = dists.copy()
        colors[colors <= 0] = max_d * 1.5  # no-return = gray-ish
        sc = self.ax.scatter(angles, dists, c=colors, s=3, cmap='plasma',
                             alpha=0.8, edgecolors='none')
        max_d_show = max(0.1, max_d)
        self.ax.set_rmax(max_d_show * 1.1)

        # Update stats
        self.set_status("pkts", f"{self.packet_count:,}")
        self.set_status("pts", f"{self.sample_count:,}")
        self.set_status("rot", f"{self.rotation_count}")
        self.set_status("hz", f"{self.scan_hz:.2f}")
        self.set_status("pps", f"{self.pps:,}")
        cs_text = f"{self.cs_errors}" if self.cs_errors == 0 else f"{self.cs_errors} (!)"
        cs_color = "green" if self.cs_errors == 0 else "red"
        self.set_status("cs", cs_text, cs_color)

        self.canvas.draw_idle()

    def clear_plot(self):
        self.points_buffer = []
        self.ax.cla()
        self.ax.set_theta_zero_location('N')
        self.ax.set_theta_direction(-1)
        self.ax.set_ylim(0, 1.0)
        self.ax.grid(True, alpha=0.3)
        self.ax.set_title("EAI S4 LiDAR - Point Cloud", pad=15)
        self.canvas.draw_idle()

    # ─── Cleanup ─────────────────────────────────────────────────

    def on_close(self):
        self.running = False
        if self.plot_timer:
            self.root.after_cancel(self.plot_timer)
        if self.serial_port and self.serial_port.is_open:
            try:
                self.serial_port.write(CMD_STOP)
                time.sleep(0.05)
                self.serial_port.close()
            except Exception:
                pass
        self.root.destroy()


# ─── Entry Point ─────────────────────────────────────────────────

if __name__ == '__main__':
    app = LidarGUI()
    app.root.mainloop()
