import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox
import json
import os
import threading
import time
from datetime import datetime
import cv2
from PIL import Image, ImageTk

CONFIG_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'controller_config.json')

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    serial = None

DEFAULT_PORT = 'COM19'

# DroidCam video box: fixed footprint regardless of window resize.
# The "long side" stays constant between landscape/portrait so the block
# doesn't jump around when the orientation is toggled.
VIDEO_LONG_SIDE = 400
VIDEO_SHORT_SIDE = 300


class CarController:
    def __init__(self, root):
        self.root = root
        self.root.title("Car Controller v1.0")
        self.root.geometry("1150x700")
        self.root.minsize(1050, 640)
        self.root.configure(bg='#f0f0f0')

        self.ser = None
        self.reading = False

        self.left_pwm = 0
        self.right_pwm = 0
        self.flags = 5
        self.servo1_val = 500
        self.servo2_val = 500

        self.droidcam_ip = ''
        self.droidcam_orientation = 'landscape'
        self.cap = None
        self.video_running = False
        self.video_thread = None

        self.config = {}
        self.load_config()
        self.build_ui()
        self.scan_ports()
        self.update_packet_display()
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
        bg = '#f0f0f0'

        # ── Serial ─────────────────────────────────────────
        top = tk.Frame(self.root, bg=bg)
        top.pack(fill=tk.X, padx=10, pady=(8, 4))

        tk.Label(top, text="Port:", bg=bg).pack(side=tk.LEFT)
        self.port_cb = ttk.Combobox(top, width=18, state='readonly')
        self.port_cb.pack(side=tk.LEFT, padx=4)

        tk.Button(top, text="\u21bb", width=3, command=self.scan_ports,
                  relief=tk.FLAT, bg='#ddd').pack(side=tk.LEFT, padx=2)
        self.btn_conn = tk.Button(top, text="Connect", width=10,
                                   command=self.toggle_serial, bg='#4caf50',
                                   fg='white', relief=tk.FLAT, cursor='hand2')
        self.btn_conn.pack(side=tk.LEFT, padx=6)
        self.lbl_status = tk.Label(top, text="Disconnected", fg='#999', bg=bg)
        self.lbl_status.pack(side=tk.LEFT, padx=4)

        main = tk.Frame(self.root, bg=bg)
        main.pack(fill=tk.BOTH, expand=True, padx=10, pady=(0, 8))

        # left_col holds the DroidCam block (fixed-size, doesn't stretch)
        left_col = tk.Frame(main, bg=bg)
        left_col.pack(side=tk.LEFT, fill=tk.Y, padx=(0, 10))

        # right_col holds everything else, stacked as before
        right_col = tk.Frame(main, bg=bg)
        right_col.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        # ── Quick Commands ─────────────────────────────────
        qf = tk.LabelFrame(right_col, text=" Quick Commands ", bg=bg,
                           font=('Segoe UI', 9, 'bold'), padx=10, pady=6)
        qf.pack(fill=tk.X)

        s = {'relief': tk.RAISED, 'padx': 14, 'pady': 5, 'cursor': 'hand2',
             'font': ('Segoe UI', 10)}

        r1 = tk.Frame(qf, bg=bg)
        r1.pack(pady=2)
        for t in [('\uC804\uC9C4', 'forward'), ('\uC815\uC9C0', 'stop'),
                  ('\uD6C4\uC9C4', 'reverse')]:
            tk.Button(r1, text=t[0], command=lambda n=t[1]: self.quick_cmd(n),
                      bg='#e3f2fd', **s).pack(side=tk.LEFT, padx=5)

        r2 = tk.Frame(qf, bg=bg)
        r2.pack(pady=2)
        for t in [('\uC88C\uD68C\uC804', 'left'), ('\uC6B0\uD68C\uC804', 'right')]:
            tk.Button(r2, text=t[0], command=lambda n=t[1]: self.quick_cmd(n),
                      bg='#fff3e0', **s).pack(side=tk.LEFT, padx=5)

        r3 = tk.Frame(qf, bg=bg)
        r3.pack(pady=2)
        self.btn_l_on = tk.Button(r3, text='\uB808\uC774\uC800 ON',
                                   command=lambda: self.quick_cmd('laser_on'),
                                   bg='#ffebee', fg='#c00', **s)
        self.btn_l_on.pack(side=tk.LEFT, padx=5)
        self.btn_l_off = tk.Button(r3, text='\uB808\uC774\uC800 OFF',
                                    command=lambda: self.quick_cmd('laser_off'),
                                    bg='#ececec', **s)
        self.btn_l_off.pack(side=tk.LEFT, padx=5)

        # servo
        sv = tk.Frame(qf, bg=bg)
        sv.pack(pady=(6, 0))
        tk.Label(sv, text='Servo 1:', bg=bg, font=('Segoe UI', 9)).pack(side=tk.LEFT)
        self.e_s1 = tk.Entry(sv, width=4, justify='center', font=('Segoe UI', 10))
        self.e_s1.insert(0, '90')
        self.e_s1.pack(side=tk.LEFT, padx=2)
        tk.Label(sv, text='\u00b0', bg=bg).pack(side=tk.LEFT, padx=(0, 10))

        tk.Label(sv, text='Servo 2:', bg=bg, font=('Segoe UI', 9)).pack(side=tk.LEFT)
        self.e_s2 = tk.Entry(sv, width=4, justify='center', font=('Segoe UI', 10))
        self.e_s2.insert(0, '90')
        self.e_s2.pack(side=tk.LEFT, padx=2)
        tk.Label(sv, text='\u00b0', bg=bg).pack(side=tk.LEFT, padx=(0, 10))

        tk.Button(sv, text='\uC801\uC6A9', command=self.apply_servos,
                  bg='#e8f5e9', relief=tk.RAISED, padx=10, cursor='hand2',
                  font=('Segoe UI', 9)).pack(side=tk.LEFT, padx=4)

        # ── Packet Preview ─────────────────────────────────
        pf = tk.LabelFrame(right_col, text=" Transmit Packet ", bg=bg,
                           font=('Segoe UI', 9, 'bold'), padx=8, pady=6)
        pf.pack(fill=tk.X, pady=(6, 0))

        self.pkt_var = tk.StringVar()
        self.e_pkt = tk.Entry(pf, textvariable=self.pkt_var, font=('Consolas', 11),
                               bg='#1e1e1e', fg='#0f0', insertbackground='white')
        self.e_pkt.pack(fill=tk.X, ipady=5, pady=(0, 5))

        btn_send = tk.Button(pf, text="Send", command=self.send_packet,
                              bg='#4caf50', fg='white', relief=tk.FLAT,
                              padx=24, pady=4, cursor='hand2',
                              font=('Segoe UI', 10, 'bold'))
        btn_send.pack()

        # ── Custom ─────────────────────────────────────────
        cf = tk.LabelFrame(right_col, text=" Custom Edit ", bg=bg,
                           font=('Segoe UI', 9, 'bold'), padx=8, pady=6)
        cf.pack(fill=tk.X, pady=(6, 0))

        labels = ['L_PWM', 'R_PWM', 'FLAGS', 'SERVO1', 'SERVO2']
        self.custom_keys = ['custom_l', 'custom_r', 'custom_f', 'custom_s1', 'custom_s2']
        self.custom_entries = {}
        crow = tk.Frame(cf, bg=bg)
        crow.pack()
        for i, lab in enumerate(labels):
            sub = tk.Frame(crow, bg=bg)
            sub.pack(side=tk.LEFT, padx=5)
            tk.Label(sub, text=lab, font=('Segoe UI', 8, 'bold'), bg=bg).pack()
            e = tk.Entry(sub, width=9, justify='center', font=('Consolas', 10))
            default = self.config.get(self.custom_keys[i], '0')
            e.insert(0, str(default))
            e.pack()
            self.custom_entries[lab] = e

        btn_cust = tk.Button(cf, text="Custom Send",
                              command=self.send_custom, bg='#7b1fa2',
                              fg='white', relief=tk.FLAT, padx=16, pady=4,
                              cursor='hand2', font=('Segoe UI', 10, 'bold'))
        btn_cust.pack(pady=(5, 0))

        # ── DroidCam ───────────────────────────────────────
        df = tk.LabelFrame(left_col, text=" DroidCam ", bg=bg,
                           font=('Segoe UI', 9, 'bold'), padx=8, pady=6)
        df.pack(fill=tk.Y)

        dc_row = tk.Frame(df, bg=bg)
        dc_row.pack(fill=tk.X, pady=(0, 4))
        tk.Label(dc_row, text="IP:", bg=bg, font=('Segoe UI', 9)).pack(side=tk.LEFT)
        self.e_dc_ip = tk.Entry(dc_row, width=14, font=('Consolas', 10))
        default_ip = self.config.get('droidcam_ip', '')
        self.e_dc_ip.insert(0, default_ip)
        self.e_dc_ip.pack(side=tk.LEFT, padx=4)
        tk.Label(dc_row, text=":4747", bg=bg, font=('Segoe UI', 9)).pack(side=tk.LEFT)

        dc_row_btn = tk.Frame(df, bg=bg)
        dc_row_btn.pack(fill=tk.X, pady=(0, 4))
        self.btn_dc_conn = tk.Button(dc_row_btn, text="Connect", width=10,
                                      command=self.toggle_droidcam, bg='#4caf50',
                                      fg='white', relief=tk.FLAT, cursor='hand2')
        self.btn_dc_conn.pack(side=tk.LEFT)
        self.lbl_dc_status = tk.Label(dc_row_btn, text="Disconnected", fg='#999',
                                       bg=bg, font=('Segoe UI', 8))
        self.lbl_dc_status.pack(side=tk.LEFT, padx=6)

        dc_row2 = tk.Frame(df, bg=bg)
        dc_row2.pack(fill=tk.X, pady=(0, 4))
        self.btn_dc_orient = tk.Button(dc_row2, text="Landscape",
                                        command=self.toggle_orientation,
                                        bg='#e0e0e0', relief=tk.RAISED, padx=8,
                                        cursor='hand2', font=('Segoe UI', 9))
        self.btn_dc_orient.pack(side=tk.LEFT)

        # Fixed-size box: pack_propagate(False) locks this frame's pixel
        # size regardless of its child, so the video block never stretches
        # or jumps with window resizing. Landscape and portrait both use
        # the same VIDEO_LONG_SIDE for their longer dimension so the block
        # doesn't change overall footprint when orientation is toggled.
        self.video_box = tk.Frame(df, bg='#000',
                                   width=VIDEO_LONG_SIDE, height=VIDEO_SHORT_SIDE)
        self.video_box.pack_propagate(False)
        self.video_box.pack(pady=(4, 0))

        self.lbl_video = tk.Label(self.video_box, bg='#000')
        self.lbl_video.pack(fill=tk.BOTH, expand=True)
        self.lbl_video.bind('<Configure>', self.on_video_resize)

        # ── Log ────────────────────────────────────────────
        lf = tk.LabelFrame(right_col, text=" Log ", bg=bg,
                           font=('Segoe UI', 9, 'bold'), padx=4, pady=4)
        lf.pack(fill=tk.X, pady=(6, 0))

        self.log = scrolledtext.ScrolledText(
            lf, height=6, font=('Consolas', 9), state='disabled',
            bg='#1e1e1e', fg='#0f0', insertbackground='white', relief=tk.FLAT
        )
        self.log.pack(fill=tk.BOTH, expand=True)

    # ── Serial ─────────────────────────────────────────────

    def scan_ports(self):
        if serial is None:
            self.port_cb['values'] = ['pyserial not installed']
            self.port_cb.set('pyserial not installed')
            return
        ports = serial.tools.list_ports.comports()
        names = [p.device for p in ports]
        self.port_cb['values'] = names
        pref = self.config.get('last_port', DEFAULT_PORT)
        if pref in names:
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
            messagebox.showerror("Error", "pyserial not installed")
            return
        port = self.port_cb.get()
        if not port:
            return
        try:
            self.ser = serial.Serial(
                port=port, baudrate=115200,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE, timeout=0.1
            )
            self.config['last_port'] = port
            self.save_config()
            self.lbl_status.config(text=f"Connected: {port}", fg='#080')
            self.btn_conn.config(text='Disconnect', bg='#e53935')
            self.add_log(f"Connected to {port} @115200 8N1")
        except Exception as e:
            messagebox.showerror("Connection Error", str(e))

    def close_serial(self):
        if self.ser:
            try:
                self.ser.close()
            except Exception:
                pass
            self.ser = None
        self.lbl_status.config(text="Disconnected", fg='#999')
        self.btn_conn.config(text='Connect', bg='#4caf50')
        self.add_log("Disconnected")

    # ── Packet helpers ─────────────────────────────────────

    def make_packet(self, left, right, flags, s1, s2):
        return "{{" + \
               f"{left:07.3f},,{right:07.3f},,{flags:07.3f},,{s1:07.3f},,{s2:07.3f}" + \
               "}}"

    def update_packet_display(self):
        p = self.make_packet(self.left_pwm, self.right_pwm, self.flags,
                             self.servo1_val, self.servo2_val)
        self.pkt_var.set(p)

    def send_data(self, packet):
        self.add_log(f"TX: {packet}")
        if self.ser and self.ser.is_open:
            try:
                self.ser.write(packet.encode('utf-8'))
                self.add_log("  -> OK")
            except Exception as e:
                self.add_log(f"  -> ERROR: {e}")
        else:
            self.add_log("  -> (not connected)")

    def read_servo_angle(self, entry):
        try:
            a = int(entry.get())
            a = max(0, min(180, a))
            return round(a * 999 / 180)
        except ValueError:
            return 500

    # ── Commands ───────────────────────────────────────────

    def quick_cmd(self, name):
        s1_pwm = self.read_servo_angle(self.e_s1)
        s2_pwm = self.read_servo_angle(self.e_s2)

        left = self.left_pwm
        right = self.right_pwm
        flags = self.flags

        # Preserve direction bits from current flags
        cur_l = self.flags % 10
        cur_t = (self.flags // 10) % 10

        if name == 'forward':
            left, right = 500, 500
            cur_l, cur_t = 0, 0
        elif name == 'stop':
            left, right = 0, 0
            cur_l, cur_t = 0, 0
        elif name == 'reverse':
            left, right = 500, 500
            cur_l, cur_t = 7, 7
        elif name == 'left':
            left, right = 200, 500
            cur_l, cur_t = 0, 0
        elif name == 'right':
            left, right = 500, 200
            cur_l, cur_t = 0, 0
        elif name == 'laser_on':
            pass  # only change flags below
        elif name == 'laser_off':
            pass

        laser_h = 7 if name == 'laser_on' else (0 if name == 'laser_off' else (self.flags // 100))
        flags = laser_h * 100 + cur_t * 10 + cur_l

        self.left_pwm = left
        self.right_pwm = right
        self.flags = flags
        self.servo1_val = s1_pwm
        self.servo2_val = s2_pwm

        self.update_packet_display()
        self.send_data(self.pkt_var.get())

    def apply_servos(self):
        self.servo1_val = self.read_servo_angle(self.e_s1)
        self.servo2_val = self.read_servo_angle(self.e_s2)
        self.update_packet_display()
        self.send_data(self.pkt_var.get())

    def send_packet(self):
        pkt = self.pkt_var.get().strip()
        if pkt.startswith('{{') and pkt.endswith('}}'):
            self.send_data(pkt)
        else:
            messagebox.showwarning("Invalid",
                                    "Packet must start with {{ and end with }}")

    def send_custom(self):
        try:
            vals = []
            for lab in ['L_PWM', 'R_PWM', 'FLAGS', 'SERVO1', 'SERVO2']:
                v = int(self.custom_entries[lab].get())
                vals.append(max(0, min(999, v)))
            l, r, f, s1, s2 = vals
            self.left_pwm = l
            self.right_pwm = r
            self.flags = f
            self.servo1_val = s1
            self.servo2_val = s2
            self.config['custom_l'] = l
            self.config['custom_r'] = r
            self.config['custom_f'] = f
            self.config['custom_s1'] = s1
            self.config['custom_s2'] = s2
            self.save_config()
            pkt = self.make_packet(l, r, f, s1, s2)
            self.pkt_var.set(pkt)
            self.send_data(pkt)
        except ValueError:
            messagebox.showerror("Error", "Enter valid integers (0~999)")

    # ── DroidCam ───────────────────────────────────────────

    def toggle_droidcam(self):
        if self.cap and self.cap.isOpened():
            self.disconnect_droidcam()
        else:
            self.connect_droidcam()

    def connect_droidcam(self):
        ip = self.e_dc_ip.get().strip()
        if not ip:
            messagebox.showerror("Error", "Enter DroidCam IP address")
            return
        url = f"http://{ip}:4747/video"
        try:
            self.cap = cv2.VideoCapture(url)
            if not self.cap.isOpened():
                self.cap = None
                raise Exception("Cannot open video stream")
            self.config['droidcam_ip'] = ip
            self.save_config()
            self.droidcam_ip = ip
            self.video_running = True
            self.btn_dc_conn.config(text='Disconnect', bg='#e53935')
            self.lbl_dc_status.config(text=f"Connected: {ip}", fg='#080')
            self.add_log(f"DroidCam connected to {ip}:4747")
            self.video_thread = threading.Thread(target=self.video_loop, daemon=True)
            self.video_thread.start()
        except Exception as e:
            self.add_log(f"DroidCam error: {e}")
            self.lbl_dc_status.config(text=f"Error: {e}", fg='#c00')

    def disconnect_droidcam(self):
        if not self.video_running and self.cap is None and self.video_thread is None:
            return  # already disconnected; avoid duplicate cleanup/log
        self.video_running = False
        if self.video_thread:
            self.video_thread.join(timeout=1)
            self.video_thread = None
        if self.cap:
            try:
                self.cap.release()
            except Exception:
                pass
            self.cap = None
        self.lbl_video.config(image='', bg='#000')
        self.btn_dc_conn.config(text='Connect', bg='#4caf50')
        self.lbl_dc_status.config(text="Disconnected", fg='#999')
        self.add_log("DroidCam disconnected")

    def video_loop(self):
        while self.video_running and self.cap and self.cap.isOpened():
            try:
                ret, frame = self.cap.read()
                if not ret:
                    time.sleep(0.1)
                    continue
                frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                if self.droidcam_orientation == 'portrait':
                    # 반시계 방향(90_COUNTERCLOCKWISE)에서 시계 방향(90_CLOCKWISE)으로 변경하여 180도 회전 효과를 줍니다.
                    frame = cv2.rotate(frame, cv2.ROTATE_90_CLOCKWISE)
                self.root.after(0, self.update_frame, frame)
                time.sleep(0.03)
            except Exception as e:
                self.root.after(0, self.add_log, f"DroidCam stream error: {e}")
                break
        self.root.after(0, self.disconnect_droidcam)

    def update_frame(self, frame):
        if not self.video_running:
            return
        try:
            h, w = frame.shape[:2]
            lbl_w = self.lbl_video.winfo_width()
            lbl_h = self.lbl_video.winfo_height()
            if lbl_w > 10 and lbl_h > 10:
                scale = min(lbl_w / w, lbl_h / h)
                new_w = int(w * scale)
                new_h = int(h * scale)
                frame = cv2.resize(frame, (new_w, new_h), interpolation=cv2.INTER_AREA)
            img = ImageTk.PhotoImage(Image.fromarray(frame))
            self.lbl_video.config(image=img, bg='#000')
            self.lbl_video.image = img
        except Exception:
            pass

    def toggle_orientation(self):
        if self.droidcam_orientation == 'landscape':
            self.droidcam_orientation = 'portrait'
            self.btn_dc_orient.config(text='Portrait')
            self.video_box.config(width=VIDEO_SHORT_SIDE, height=VIDEO_LONG_SIDE)
        else:
            self.droidcam_orientation = 'landscape'
            self.btn_dc_orient.config(text='Landscape')
            self.video_box.config(width=VIDEO_LONG_SIDE, height=VIDEO_SHORT_SIDE)

    def on_video_resize(self, event):
        pass

    # ── Log ────────────────────────────────────────────────

    def add_log(self, msg):
        self.log.config(state='normal')
        ts = datetime.now().strftime('%H:%M:%S.%f')[:-3]
        self.log.insert(tk.END, f"[{ts}] ", 'time')
        self.log.insert(tk.END, f"{msg}\n")
        self.log.tag_config('time', foreground='#aaa')
        self.log.see(tk.END)
        self.log.config(state='disabled')

    # ── Cleanup ────────────────────────────────────────────

    def on_close(self):
        self.close_serial()
        self.disconnect_droidcam()
        self.root.destroy()


if __name__ == '__main__':
    root = tk.Tk()
    app = CarController(root)
    root.mainloop()
