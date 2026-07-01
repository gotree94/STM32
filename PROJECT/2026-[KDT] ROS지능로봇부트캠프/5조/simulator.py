import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox
import json
import os
import math
import threading
import time
from datetime import datetime

CONFIG_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'car_config.json')

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    serial = None


class CarSimulator:
    def __init__(self, root):
        self.root = root
        self.root.title("Car Simulator v1.0")
        self.root.geometry("860x820")
        self.root.minsize(800, 750)
        self.root.configure(bg='#f5f5f5')

        self.ser = None
        self.reading = False

        self.left_pwm = 0
        self.right_pwm = 0
        self.servo1 = 90
        self.servo2 = 90
        self.flag_left_dir = False
        self.flag_right_dir = False
        self.flag_laser = False

        self.config = {}
        self.load_config()
        self.build_ui()
        self.scan_ports()
        self.draw_all()
        self.root.protocol("WM_DELETE_WINDOW", self.on_close)

    # ── Config ──────────────────────────────────────────────

    def load_config(self):
        if os.path.exists(CONFIG_FILE):
            try:
                with open(CONFIG_FILE) as f:
                    self.config = json.load(f)
            except Exception:
                self.config = {}

    def save_config(self):
        try:
            with open(CONFIG_FILE, 'w') as f:
                json.dump(self.config, f)
        except Exception:
            pass

    # ── UI ──────────────────────────────────────────────────

    def build_ui(self):
        bg = '#f5f5f5'

        # ── Serial bar ─────────────────────────────────────
        top = tk.Frame(self.root, bg=bg)
        top.pack(fill=tk.X, padx=10, pady=(8, 4))

        tk.Label(top, text="Port:", bg=bg).pack(side=tk.LEFT)
        self.port_cb = ttk.Combobox(top, width=18, state='readonly')
        self.port_cb.pack(side=tk.LEFT, padx=4)

        tk.Button(top, text="\u21bb", width=3, command=self.scan_ports,
                  relief=tk.FLAT, bg='#e0e0e0').pack(side=tk.LEFT, padx=2)
        self.btn_conn = tk.Button(top, text="Connect", width=10,
                                  command=self.toggle_serial, bg='#4caf50', fg='white',
                                  relief=tk.FLAT, cursor='hand2')
        self.btn_conn.pack(side=tk.LEFT, padx=6)

        self.lbl_status = tk.Label(top, text="Disconnected", fg='#999', bg=bg)
        self.lbl_status.pack(side=tk.LEFT, padx=4)

        # ── Main content ───────────────────────────────────
        main = tk.Frame(self.root, bg=bg)
        main.pack(fill=tk.BOTH, expand=True, padx=10, pady=(0, 8))

        # ── Wheels ─────────────────────────────────────────
        wf = tk.LabelFrame(main, text=" Wheels ", bg=bg, font=('Segoe UI', 9, 'bold'),
                           padx=8, pady=6)
        wf.pack(fill=tk.X)

        # front row
        frow = tk.Frame(wf, bg=bg)
        frow.pack(pady=2)
        self.cv_fl = tk.Canvas(frow, width=120, height=110, bg='#1e1e1e',
                               highlightthickness=0)
        self.cv_fr = tk.Canvas(frow, width=120, height=110, bg='#1e1e1e',
                               highlightthickness=0)
        self.cv_fl.pack(side=tk.LEFT, padx=30)
        self.cv_fr.pack(side=tk.LEFT, padx=30)

        # chassis
        chassis_frame = tk.Frame(wf, bg=bg)
        chassis_frame.pack(fill=tk.X, pady=3)
        self.cv_chassis = tk.Canvas(chassis_frame, height=36, bg='#1e1e1e',
                                    highlightthickness=0)
        self.cv_chassis.pack(fill=tk.X)

        # rear row
        rrow = tk.Frame(wf, bg=bg)
        rrow.pack(pady=2)
        self.cv_rl = tk.Canvas(rrow, width=120, height=110, bg='#1e1e1e',
                               highlightthickness=0)
        self.cv_rr = tk.Canvas(rrow, width=120, height=110, bg='#1e1e1e',
                               highlightthickness=0)
        self.cv_rl.pack(side=tk.LEFT, padx=30)
        self.cv_rr.pack(side=tk.LEFT, padx=30)

        # labels under wheels
        lrow = tk.Frame(wf, bg=bg)
        lrow.pack()
        for t in ['FL', 'FR']:
            tk.Label(lrow, text=t, font=('Segoe UI', 8, 'bold'),
                     width=21, bg=bg).pack(side=tk.LEFT)
        lrow2 = tk.Frame(wf, bg=bg)
        lrow2.pack()
        for t in ['RL', 'RR']:
            tk.Label(lrow2, text=t, font=('Segoe UI', 8, 'bold'),
                     width=21, bg=bg).pack(side=tk.LEFT)

        # status row (motors + laser)
        srow = tk.Frame(wf, bg=bg)
        srow.pack(pady=(6, 0))
        self.lbl_motor_l = tk.Label(srow, text="L: \u25B6", font=('Segoe UI', 10),
                                    bg='#ffe0e0', fg='#c00', padx=10, pady=2)
        self.lbl_motor_l.pack(side=tk.LEFT, padx=12)
        self.lbl_motor_r = tk.Label(srow, text="R: \u25B6", font=('Segoe UI', 10),
                                    bg='#ffe0e0', fg='#c00', padx=10, pady=2)
        self.lbl_motor_r.pack(side=tk.LEFT, padx=12)
        self.lbl_laser = tk.Label(srow, text="LASER: \u25CB OFF", font=('Segoe UI', 10),
                                  bg='#333', fg='#888', padx=10, pady=2)
        self.lbl_laser.pack(side=tk.LEFT, padx=12)

        # ── Servos ─────────────────────────────────────────
        sf = tk.LabelFrame(main, text=" Servos ", bg=bg, font=('Segoe UI', 9, 'bold'),
                           padx=8, pady=6)
        sf.pack(fill=tk.X, pady=(6, 0))

        sg = tk.Frame(sf, bg=bg)
        sg.pack()
        self.cv_s1 = tk.Canvas(sg, width=210, height=170, bg='white',
                               highlightthickness=1, highlightbackground='#ccc')
        self.cv_s2 = tk.Canvas(sg, width=210, height=170, bg='white',
                               highlightthickness=1, highlightbackground='#ccc')
        self.cv_s1.pack(side=tk.LEFT, padx=15)
        self.cv_s2.pack(side=tk.LEFT, padx=15)

        lab_s = tk.Frame(sf, bg=bg)
        lab_s.pack()
        tk.Label(lab_s, text="Servo 1", font=('Segoe UI', 9, 'bold'),
                 width=30, bg=bg).pack(side=tk.LEFT)
        tk.Label(lab_s, text="Servo 2", font=('Segoe UI', 9, 'bold'),
                 width=30, bg=bg).pack(side=tk.LEFT)

        # ── Log ────────────────────────────────────────────
        lf = tk.LabelFrame(main, text=" Log ", bg=bg, font=('Segoe UI', 9, 'bold'),
                           padx=4, pady=4)
        lf.pack(fill=tk.BOTH, expand=True, pady=(6, 0))

        self.log = scrolledtext.ScrolledText(
            lf, height=10, font=('Consolas', 9), state='disabled',
            bg='#1e1e1e', fg='#0f0', insertbackground='white', relief=tk.FLAT
        )
        self.log.pack(fill=tk.BOTH, expand=True)

    # ── Serial ─────────────────────────────────────────────

    def scan_ports(self):
        if serial is None:
            self.port_cb['values'] = []
            self.port_cb.set('[pyserial not installed]')
            return
        ports = serial.tools.list_ports.comports()
        names = [p.device for p in ports]
        self.port_cb['values'] = names
        pref = self.config.get('last_port')
        if pref and pref in names:
            self.port_cb.set(pref)
        elif names:
            self.port_cb.set(names[0])

    def toggle_serial(self):
        if self.ser and self.ser.is_open:
            self.close_serial()
        else:
            self.open_serial()

    def open_serial(self):
        if serial is None:
            messagebox.showerror("Error",
                "pyserial not installed.\nRun: pip install pyserial")
            return
        port = self.port_cb.get()
        if not port:
            return
        try:
            self.ser = serial.Serial(
                port=port, baudrate=115200,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=0.1
            )
            self.config['last_port'] = port
            self.save_config()
            self.lbl_status.config(text=f"Connected: {port}", fg='#080')
            self.btn_conn.config(text='Disconnect', bg='#e53935')
            self.reading = True
            threading.Thread(target=self.read_loop, daemon=True).start()
            self.add_log(f"Connected to {port} @115200 8N1")
        except Exception as e:
            messagebox.showerror("Connection Error", str(e))

    def close_serial(self):
        self.reading = False
        if self.ser:
            try:
                self.ser.close()
            except Exception:
                pass
            self.ser = None
        self.lbl_status.config(text="Disconnected", fg='#999')
        self.btn_conn.config(text='Connect', bg='#4caf50')
        self.add_log("Disconnected")

    def read_loop(self):
        buf = ''
        while self.reading and self.ser and self.ser.is_open:
            try:
                if self.ser.in_waiting > 0:
                    data = self.ser.read(self.ser.in_waiting).decode('utf-8', errors='replace')
                    buf += data
                    while '{{' in buf and '}}' in buf:
                        a = buf.find('{{')
                        b = buf.find('}}', a)
                        if b == -1:
                            break
                        pkt = buf[a:b+2]
                        buf = buf[b+2:]
                        self.root.after(0, self.handle_packet, pkt)
                else:
                    time.sleep(0.005)
            except Exception as e:
                self.root.after(0, self.add_log, f"Serial error: {e}")
                self.root.after(0, self.close_serial)
                break

    # ── Packet parser ──────────────────────────────────────

    def handle_packet(self, pkt):
        self.add_log(pkt)
        try:
            inner = pkt[2:-2]
            parts = inner.split(',,')
            if len(parts) != 5:
                return
            v = []
            for p in parts:
                try:
                    v.append(int(float(p.strip())))
                except Exception:
                    v.append(0)

            self.left_pwm = max(0, min(999, v[0]))
            self.right_pwm = max(0, min(999, v[1]))

            flags = max(0, min(999, v[2]))
            u = flags % 10
            t = (flags // 10) % 10
            h = flags // 100
            self.flag_left_dir = u > 5
            self.flag_right_dir = t > 5
            self.flag_laser = h > 5

            raw_s1 = max(0, min(999, v[3]))
            raw_s2 = max(0, min(999, v[4]))
            self.servo1 = round(raw_s1 * 180 / 999)
            self.servo2 = round(raw_s2 * 180 / 999)

            self.draw_all()
        except Exception as e:
            self.add_log(f"Parse error: {e}")

    # ── Drawing ────────────────────────────────────────────

    def draw_all(self):
        self.draw_chassis()
        self.draw_wheel(self.cv_fl, self.left_pwm, 'FL')
        self.draw_wheel(self.cv_fr, self.right_pwm, 'FR')
        self.draw_wheel(self.cv_rl, self.left_pwm, 'RL')
        self.draw_wheel(self.cv_rr, self.right_pwm, 'RR')
        self.draw_gauge(self.cv_s1, self.servo1)
        self.draw_gauge(self.cv_s2, self.servo2)
        self.update_indicators()

    def draw_chassis(self):
        self.cv_chassis.delete('all')
        w = max(self.cv_chassis.winfo_width(), 10)
        h = self.cv_chassis.winfo_height() or 36
        cx = w // 2
        margin = 4
        t = margin
        b = h - margin
        half = cx - margin
        n = 50

        self.cv_chassis.create_rectangle(margin, t, w - margin, b,
                                          fill='#1a1a1a', outline='#555', width=2)

        if self.left_pwm > 0:
            fw = (self.left_pwm / 999) * half
            for i in range(n):
                v = self.left_pwm * (i + 0.5) / n
                x1 = cx - fw * (i / n)
                x2 = cx - fw * ((i + 1) / n)
                c = self.val_to_rainbow(v)
                self.cv_chassis.create_rectangle(x2, t, x1, b, fill=c, outline='')

        if self.right_pwm > 0:
            fw = (self.right_pwm / 999) * half
            for i in range(n):
                v = self.right_pwm * (i + 0.5) / n
                x1 = cx + fw * (i / n)
                x2 = cx + fw * ((i + 1) / n)
                c = self.val_to_rainbow(v)
                self.cv_chassis.create_rectangle(x1, t, x2, b, fill=c, outline='')

        self.cv_chassis.create_line(cx, t, cx, b, fill='#aaa', width=2)
        self.cv_chassis.create_text(cx, h // 2, text="CHASSIS", fill='white',
                                     font=('Segoe UI', 9, 'bold'))
        self.cv_chassis.create_text(margin + 2, b - 2, text="0", fill='#888',
                                     font=('Segoe UI', 7), anchor='sw')
        self.cv_chassis.create_text(w - margin - 2, b - 2, text="0", fill='#888',
                                     font=('Segoe UI', 7), anchor='se')
        self.cv_chassis.create_text(margin + 2, t + 2, text="999", fill='#666',
                                     font=('Segoe UI', 7), anchor='nw')
        self.cv_chassis.create_text(w - margin - 2, t + 2, text="999", fill='#666',
                                     font=('Segoe UI', 7), anchor='ne')

    def draw_wheel(self, cv, val, label):
        cv.delete('all')
        w = cv.winfo_width() or 120
        h = cv.winfo_height() or 110
        cx, cy = w // 2, h // 2

        color = self.val_to_rainbow(val) if val > 0 else '#2a2a2a'

        cv.create_rectangle(8, 6, w - 8, h - 6, fill=color, outline='#555',
                            width=2)
        cv.create_text(cx, cy - 8, text=str(val), fill='white',
                       font=('Segoe UI', 22, 'bold'))
        cv.create_text(cx, cy + 18, text=label, fill='#ccc',
                       font=('Segoe UI', 9))

    def val_to_rainbow(self, val):
        hue = (val / 999.0) * 300.0
        return self.hsv_to_hex(hue, 1.0, 0.85)

    @staticmethod
    def hsv_to_hex(h, s, v):
        h %= 360
        c = v * s
        x = c * (1 - abs((h / 60) % 2 - 1))
        m = v - c
        if h < 60:
            r, g, b = c, x, 0
        elif h < 120:
            r, g, b = x, c, 0
        elif h < 180:
            r, g, b = 0, c, x
        elif h < 240:
            r, g, b = 0, x, c
        elif h < 300:
            r, g, b = x, 0, c
        else:
            r, g, b = c, 0, x
        ri = int((r + m) * 255)
        gi = int((g + m) * 255)
        bi = int((b + m) * 255)
        return f'#{ri:02x}{gi:02x}{bi:02x}'

    def draw_gauge(self, cv, angle):
        cv.delete('all')
        w = cv.winfo_width() or 210
        h = cv.winfo_height() or 170
        cx, cy = w // 2, h - 32
        r = 52

        # semicircle arc (0° left → 180° right via bottom)
        cv.create_arc(cx - r, cy - r, cx + r, cy + r,
                      start=0, extent=180, style='arc',
                      width=3, outline='#333')

        # tick marks
        for deg in range(0, 181, 30):
            rad = math.radians(180 + deg)
            x1 = cx + (r - 7) * math.cos(rad)
            y1 = cy + (r - 7) * math.sin(rad)
            x2 = cx + r * math.cos(rad)
            y2 = cy + r * math.sin(rad)
            cv.create_line(x1, y1, x2, y2, width=2, fill='#444')

            # label
            lx = cx + (r + 14) * math.cos(rad)
            ly = cy + (r + 14) * math.sin(rad)
            cv.create_text(lx, ly, text=str(deg), font=('Segoe UI', 7),
                           fill='#555')

        # needle
        rad = math.radians(180 + angle)
        nx = cx + (r - 10) * math.cos(rad)
        ny = cy + (r - 10) * math.sin(rad)
        cv.create_line(cx, cy, nx, ny, width=3, fill='#d32f2f',
                       capstyle=tk.ROUND)
        cv.create_oval(cx - 5, cy - 5, cx + 5, cy + 5,
                       fill='#d32f2f', outline='')

        # angle value
        cv.create_text(cx, 18, text=f"{angle}\u00b0",
                       font=('Segoe UI', 22, 'bold'), fill='#222')

    def update_indicators(self):
        la = '\u25B6' if self.flag_left_dir else '\u25C0'
        ra = '\u25B6' if self.flag_right_dir else '\u25C0'
        l_fg = '#0a0' if self.flag_left_dir else '#c00'
        r_fg = '#0a0' if self.flag_right_dir else '#c00'
        l_bg = '#d0ffd0' if self.flag_left_dir else '#ffe0e0'
        r_bg = '#d0ffd0' if self.flag_right_dir else '#ffe0e0'
        self.lbl_motor_l.config(text=f"L: {la}", fg=l_fg, bg=l_bg)
        self.lbl_motor_r.config(text=f"R: {ra}", fg=r_fg, bg=r_bg)

        if self.flag_laser:
            self.lbl_laser.config(text="LASER: \u25CF ON", fg='#f44', bg='#330000')
        else:
            self.lbl_laser.config(text="LASER: \u25CB OFF", fg='#888', bg='#333')

    # ── Log ────────────────────────────────────────────────

    def add_log(self, msg):
        self.log.config(state='normal')
        ts = datetime.now().strftime('%H:%M:%S.%f')[:-3]
        self.log.insert(tk.END, f"[{ts}] ", 'time')
        self.log.insert(tk.END, f"{msg}\n")
        self.log.tag_config('time', foreground='#888')
        self.log.see(tk.END)
        self.log.config(state='disabled')

    # ── Cleanup ────────────────────────────────────────────

    def on_close(self):
        self.close_serial()
        self.root.destroy()


if __name__ == '__main__':
    root = tk.Tk()
    app = CarSimulator(root)
    root.mainloop()
