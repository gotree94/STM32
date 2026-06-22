"""
MPU6050 3D + 2D 실시간 시각화
STM32 UART2 -> PC Serial -> 3D Orientation + 2D 그래프
"""

import serial
import time
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.gridspec import GridSpec
from mpl_toolkits.mplot3d import Axes3D
import re
import sys
from collections import deque

SERIAL_PORT = 'COM15'
BAUD_RATE = 115200

ACCEL_SCALE = 16384.0
GYRO_SCALE = 131.0

ALPHA = 0.96

MAX_SAMPLES = 200

CUBE = np.array([
    [-1, -1, -1], [ 1, -1, -1], [ 1,  1, -1], [-1,  1, -1],
    [-1, -1,  1], [ 1, -1,  1], [ 1,  1,  1], [-1,  1,  1]
])

EDGES = [
    (0,1),(1,2),(2,3),(3,0),(4,5),(5,6),(6,7),(7,4),
    (0,4),(1,5),(2,6),(3,7)
]


def rotation_matrix(roll, pitch, yaw):
    cx, sx = np.cos(roll), np.sin(roll)
    cy, sy = np.cos(pitch), np.sin(pitch)
    cz, sz = np.cos(yaw), np.sin(yaw)
    Rx = np.array([[1,0,0],[0,cx,-sx],[0,sx,cx]])
    Ry = np.array([[cy,0,sy],[0,1,0],[-sy,0,cy]])
    Rz = np.array([[cz,-sz,0],[sz,cz,0],[0,0,1]])
    return Rz @ Ry @ Rx


def parse_line(line):
    m = re.match(
        r'ACC:\s+(-?\d+)\s+(-?\d+)\s+(-?\d+)'
        r'\s+GYRO:\s+(-?\d+)\s+(-?\d+)\s+(-?\d+)'
        r'\s+RPY:\s+(-?\d+)\s+(-?\d+)\s+(-?\d+)'
        r'\s+TEMP:\s+([\d.-]+)\s+C',
        line
    )
    if not m:
        return None
    return {
        'accel': (int(m[1]), int(m[2]), int(m[3])),
        'gyro':  (int(m[4]), int(m[5]), int(m[6])),
        'rpy':   (int(m[7]), int(m[8]), int(m[9])),
        'temp':  float(m[10])
    }


def make_2d_ax(ax, title, ylabel, colors, labels, ylim):
    ax.set_title(title)
    ax.set_ylabel(ylabel)
    ax.grid(True, alpha=0.3)
    ax.set_ylim(ylim)
    lines = []
    for c, l in zip(colors, labels):
        line, = ax.plot([], [], c, label=l, lw=1.5)
        lines.append(line)
    ax.legend(loc='upper right', fontsize=8)
    return lines


def main():
    port = sys.argv[1] if len(sys.argv) > 1 else SERIAL_PORT

    try:
        ser = serial.Serial(port, BAUD_RATE, timeout=1)
    except Exception as e:
        print(f"Serial port error: {e}")
        sys.exit(1)

    plt.ion()
    fig = plt.figure(figsize=(16, 11))
    gs = GridSpec(3, 2, figure=fig, hspace=0.35, wspace=0.35)

    ax3d = fig.add_subplot(gs[0:2, 0], projection='3d')
    ax_rpy = fig.add_subplot(gs[0, 1])
    ax_acc = fig.add_subplot(gs[1, 1])
    ax_gyro = fig.add_subplot(gs[2, 0])
    ax_temp = fig.add_subplot(gs[2, 1])

    ax3d.set_xlim(-2, 2)
    ax3d.set_ylim(-2, 2)
    ax3d.set_zlim(-2, 2)
    ax3d.set_xlabel('X')
    ax3d.set_ylabel('Y')
    ax3d.set_zlabel('Z')
    ax3d.set_title('MPU6050 3D Orientation', fontsize=11, fontweight='bold')
    ax3d.view_init(elev=30, azim=-60)

    cube_lines = [ax3d.plot([], [], [], 'b-', lw=2)[0] for _ in EDGES]
    ax_x = ax3d.plot([0, 2], [0, 0], [0, 0], 'r-', lw=3, alpha=0.4)[0]
    ax_y = ax3d.plot([0, 0], [0, 2], [0, 0], 'g-', lw=3, alpha=0.4)[0]
    ax_z = ax3d.plot([0, 0], [0, 0], [0, 2], 'b-', lw=3, alpha=0.4)[0]

    info_text = ax3d.text2D(0.55, 0.02, '', transform=ax3d.transAxes,
                            fontsize=9, verticalalignment='bottom',
                            bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.85))

    rpy_lines = make_2d_ax(ax_rpy, 'Roll / Pitch / Yaw', 'Degrees',
                           ['r', 'g', 'b'], ['Roll', 'Pitch', 'Yaw'],
                           ylim=(-180, 180))

    acc_lines = make_2d_ax(ax_acc, 'Accelerometer', 'g',
                           ['r', 'g', 'b'], ['X', 'Y', 'Z'],
                           ylim=(-2, 2))

    gyro_lines = make_2d_ax(ax_gyro, 'Gyroscope', 'deg/s',
                            ['r', 'g', 'b'], ['X', 'Y', 'Z'],
                            ylim=(-250, 250))

    temp_line, = ax_temp.plot([], [], 'm-', lw=1.5, label='Temp')
    ax_temp.set_title('Temperature')
    ax_temp.set_ylabel('deg C')
    ax_temp.set_xlabel('Samples')
    ax_temp.grid(True, alpha=0.3)
    ax_temp.set_ylim(0, 60)
    ax_temp.legend(loc='upper right', fontsize=8)

    t_buf = deque(maxlen=MAX_SAMPLES)
    rpy_buf = [deque(maxlen=MAX_SAMPLES) for _ in range(3)]
    acc_buf = [deque(maxlen=MAX_SAMPLES) for _ in range(3)]
    gyro_buf = [deque(maxlen=MAX_SAMPLES) for _ in range(3)]
    temp_buf = deque(maxlen=MAX_SAMPLES)

    roll = pitch = yaw = 0.0
    filter_init = False
    last_time = time.time()
    data_count = 0
    sample_idx = 0

    print(f"Connecting to {port} @ {BAUD_RATE}...")

    while True:
        try:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
        except:
            continue

        if not line:
            continue

        data = parse_line(line)
        if not data:
            continue

        now = time.time()
        dt = now - last_time
        if dt <= 0 or dt > 1.0:
            dt = 0.5
        last_time = now
        data_count += 1
        sample_idx += 1

        ax_v, ay_v, az_v = [v / ACCEL_SCALE for v in data['accel']]
        gx_v, gy_v, gz_v = [v / GYRO_SCALE for v in data['gyro']]
        gx_rad, gy_rad, gz_rad = np.radians([gx_v, gy_v, gz_v])

        acc_mag = np.sqrt(ax_v**2 + ay_v**2 + az_v**2)
        gyro_mag = np.sqrt(gx_v**2 + gy_v**2 + gz_v**2)

        is_still = (abs(acc_mag - 1.0) < 0.05) and (gyro_mag < 1.0)

        acc_error = abs(acc_mag - 1.0)
        acc_weight = max(0.0, 1.0 - acc_error * 5.0)
        alpha_eff = 1.0 - (1.0 - ALPHA) * acc_weight

        roll_a = np.arctan2(-ay_v, az_v)
        pitch_a = np.arctan2(ax_v, np.sqrt(ay_v**2 + az_v**2))

        if not filter_init:
            roll, pitch, yaw = roll_a, pitch_a, 0.0
            filter_init = True
        else:
            roll_g = roll + gx_rad * dt
            pitch_g = pitch + gy_rad * dt
            roll = alpha_eff * roll_g + (1 - alpha_eff) * roll_a
            pitch = alpha_eff * pitch_g + (1 - alpha_eff) * pitch_a
            if is_still:
                yaw = yaw
            else:
                yaw = yaw + gz_rad * dt

        t_buf.append(sample_idx)
        rpy_buf[0].append(np.degrees(roll))
        rpy_buf[1].append(np.degrees(pitch))
        rpy_buf[2].append(np.degrees(yaw))
        acc_buf[0].append(ax_v)
        acc_buf[1].append(ay_v)
        acc_buf[2].append(az_v)
        gyro_buf[0].append(gx_v)
        gyro_buf[1].append(gy_v)
        gyro_buf[2].append(gz_v)
        temp_buf.append(data['temp'])

        R = rotation_matrix(roll, pitch, yaw)
        rotated = (R @ CUBE.T).T

        for li, (i, j) in enumerate(EDGES):
            cube_lines[li].set_data([rotated[i,0], rotated[j,0]],
                                    [rotated[i,1], rotated[j,1]])
            cube_lines[li].set_3d_properties([rotated[i,2], rotated[j,2]])

        axes_len = 1.5
        origin = np.array([0, 0, 0])
        x_dir = R @ np.array([axes_len, 0, 0])
        y_dir = R @ np.array([0, axes_len, 0])
        z_dir = R @ np.array([0, 0, axes_len])

        ax_x.set_data([origin[0], x_dir[0]], [origin[1], x_dir[1]])
        ax_x.set_3d_properties([origin[2], x_dir[2]])
        ax_y.set_data([origin[0], y_dir[0]], [origin[1], y_dir[1]])
        ax_y.set_3d_properties([origin[2], y_dir[2]])
        ax_z.set_data([origin[0], z_dir[0]], [origin[1], z_dir[1]])
        ax_z.set_3d_properties([origin[2], z_dir[2]])

        roll_d = np.degrees(roll) % 360
        pitch_d = np.degrees(pitch) % 360
        yaw_d = np.degrees(yaw) % 360

        info_text.set_text(
            f"Roll:  {roll_d:7.1f}\n"
            f"Pitch: {pitch_d:7.1f}\n"
            f"Yaw:   {yaw_d:7.1f}\n"
            f"Temp:  {data['temp']:.2f}C\n"
            f"N:     {data_count}"
        )

        xdata = list(t_buf)
        for i in range(3):
            rpy_lines[i].set_data(xdata, list(rpy_buf[i]))
            acc_lines[i].set_data(xdata, list(acc_buf[i]))
            gyro_lines[i].set_data(xdata, list(gyro_buf[i]))
        temp_line.set_data(xdata, list(temp_buf))

        if len(xdata) >= 2:
            xmin = xdata[0]
            xmax = xdata[-1]
            ax_rpy.set_xlim(xmin, xmax)
            ax_acc.set_xlim(xmin, xmax)
            ax_gyro.set_xlim(xmin, xmax)
            ax_temp.set_xlim(xmin, xmax)

        fig.canvas.draw()
        fig.canvas.flush_events()
        plt.pause(0.001)


if __name__ == '__main__':
    main()
