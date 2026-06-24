#!/usr/bin/env python3
"""
SLAM Data Viewer
=================
Reads simulated sensor data and visualizes the mapping process.
Left: environment view (walls, robot, scan rays, trajectory)
Right: occupancy grid map being built from ultrasonic data
"""

import json
import math
import argparse
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.patches import Rectangle, Circle as CirclePatch
from matplotlib.lines import Line2D


class SlamViewer:
    def __init__(self, data_file):
        with open(data_file, "r") as f:
            self.data = json.load(f)

        self.env = self.data["environment"]
        self.robot_params = self.data["robot"]
        self.frames = self.data["data"]
        self.num_frames = len(self.frames)

        self.res = 1.0
        self.gw = int(math.ceil(self.env["width"] / self.res))
        self.gh = int(math.ceil(self.env["height"] / self.res))

        self.l_occ = 0.85
        self.l_free = -0.4

        self.frame_grids = self._precompute_grids()

        self.current_idx = 0
        self.paused = True
        self.speed = 1.0

        self.fig, (self.ax1, self.ax2) = plt.subplots(1, 2, figsize=(15, 6.5))
        self.fig.canvas.mpl_connect("key_press_event", self._on_key)
        self.fig.tight_layout(pad=3.0)

        self._setup_plots()

        self.anim = animation.FuncAnimation(
            self.fig, self._update, interval=50,
            frames=self.num_frames, repeat=True, blit=False,
            cache_frame_data=False
        )

    # -----------------------------------------------------------------
    def _world_to_grid(self, x, y):
        gx = int(x / self.res)
        gy = int(y / self.res)
        return np.clip(gx, 0, self.gw - 1), np.clip(gy, 0, self.gh - 1)

    def _bresenham(self, x0, y0, x1, y1):
        points = []
        dx = abs(x1 - x0)
        dy = -abs(y1 - y0)
        sx = 1 if x0 < x1 else -1
        sy = 1 if y0 < y1 else -1
        err = dx + dy
        x, y = x0, y0
        while True:
            points.append((int(x), int(y)))
            if x == x1 and y == y1:
                break
            e2 = 2 * err
            if e2 >= dy:
                err += dy
                x += sx
            if e2 <= dx:
                err += dx
                y += sy
        return points

    def _log_odds_to_prob(self, lo):
        return 1.0 - 1.0 / (1.0 + np.exp(np.clip(lo, -10, 10)))

    def _precompute_grids(self):
        lo = np.zeros((self.gh, self.gw), dtype=np.float32)
        grids = []
        for frame in self.frames:
            scan = frame.get("scan", [])
            if scan:
                px = frame["ground_truth"]["x"]
                py = frame["ground_truth"]["y"]
                gx0, gy0 = self._world_to_grid(px, py)
                for rd in scan:
                    a_world = rd["angle_world"]
                    dist = rd["distance"]
                    ex = px + dist * math.cos(math.radians(a_world))
                    ey = py + dist * math.sin(math.radians(a_world))
                    gx1, gy1 = self._world_to_grid(ex, ey)
                    gx1 = int(np.clip(gx1, 0, self.gw - 1))
                    gy1 = int(np.clip(gy1, 0, self.gh - 1))
                    cells = self._bresenham(gx0, gy0, gx1, gy1)
                    if len(cells) < 2:
                        continue
                    for cx, cy in cells[:-1]:
                        if 0 <= cx < self.gw and 0 <= cy < self.gh:
                            lo[cy, cx] += self.l_free
                    lx, ly = cells[-1]
                    if 0 <= lx < self.gw and 0 <= ly < self.gh:
                        lo[ly, lx] += self.l_occ
            grids.append(lo.copy())
        return grids

    # -----------------------------------------------------------------
    def _setup_plots(self):
        w, h = self.env["width"], self.env["height"]
        ax = self.ax1
        ax.set_xlim(-5, w + 5)
        ax.set_ylim(-5, h + 5)
        ax.set_aspect("equal")
        ax.set_title("Environment View", fontweight="bold")
        ax.set_xlabel("X (cm)")
        ax.set_ylabel("Y (cm)")
        ax.grid(True, alpha=0.3)

        for x1, y1, x2, y2 in [(0, 0, w, 0), (w, 0, w, h),
                                (w, h, 0, h), (0, h, 0, 0)]:
            ax.add_line(Line2D([x1, x2], [y1, y2], color="k", linewidth=3))

        for ox, oy, ow, oh in self.env["obstacles"]:
            ax.add_patch(Rectangle((ox, oy), ow, oh,
                                   facecolor="gray", edgecolor="k", hatch="///"))
        for cx, cy, cr in self.env.get("circles", []):
            ax.add_patch(CirclePatch((cx, cy), cr,
                                     facecolor="gray", edgecolor="k", hatch="///"))

        self.robot_dot = ax.plot([], [], "ro", markersize=8, zorder=5)[0]
        self.robot_dir = ax.plot([], [], "r-", linewidth=2.5, zorder=5)[0]
        self.traj_line = ax.plot([], [], "b-", alpha=0.6, linewidth=1.2)[0]
        self.scan_artists = []
        self.traj_x, self.traj_y = [], []

        self.info = ax.text(
            0.02, 0.98, "", transform=ax.transAxes,
            va="top", fontsize=9, family="monospace",
            bbox=dict(boxstyle="round", facecolor="wheat", alpha=0.9)
        )

        self.help_text = ax.text(
            0.98, 0.02, "SPACE: play/pause  |  +/-: speed  |  <-/->: step",
            transform=ax.transAxes, va="bottom", ha="right",
            fontsize=8, color="gray"
        )

        ax2 = self.ax2
        ax2.set_xlim(0, self.gw)
        ax2.set_ylim(0, self.gh)
        ax2.set_aspect("equal")
        ax2.set_title("Occupancy Grid Map", fontweight="bold")
        ax2.set_xlabel("X (cell)")
        ax2.set_ylabel("Y (cell)")

        self.grid_img = ax2.imshow(
            np.full((self.gh, self.gw), 0.5),
            cmap="gray_r", interpolation="nearest",
            origin="lower", vmin=0, vmax=1,
            extent=[0, self.gw, 0, self.gh]
        )
        self.robot_grid = ax2.plot([], [], "ro", markersize=5, zorder=5)[0]
        self.grid_info = ax2.text(
            0.02, 0.98, "", transform=ax2.transAxes,
            va="top", fontsize=9, family="monospace",
            bbox=dict(boxstyle="round", facecolor="wheat", alpha=0.9)
        )

    # -----------------------------------------------------------------
    def _update(self, idx):
        self.current_idx = idx
        frame = self.frames[idx]
        gt = frame["ground_truth"]
        px, py, theta = gt["x"], gt["y"], gt["theta"]

        self.traj_x.append(px)
        self.traj_y.append(py)
        self.traj_line.set_data(self.traj_x, self.traj_y)
        self.robot_dot.set_data([px], [py])

        arrow = 10
        self.robot_dir.set_data(
            [px, px + arrow * math.cos(math.radians(theta))],
            [py, py + arrow * math.sin(math.radians(theta))]
        )

        for a in self.scan_artists:
            a.remove()
        self.scan_artists = []

        scan = frame.get("scan", [])
        step = max(1, len(scan) // 30)
        for rd in scan[::step]:
            aw = rd["angle_world"]
            d = rd["distance"]
            ex = px + d * math.cos(math.radians(aw))
            ey = py + d * math.sin(math.radians(aw))
            lw = 0.4 if rd.get("sensor_id", 0) == 0 else 0.8
            alpha = 0.06
            line = self.ax1.plot([px, ex], [py, ey], "g-",
                                  alpha=alpha, linewidth=lw)[0]
            self.scan_artists.append(line)

        is_turn = frame.get("is_turn", False)
        status = "TURNING" if is_turn else "SCANNING"
        odom = frame["odometry"]
        info = (
            f"[{idx + 1}/{self.num_frames}]  t={frame['timestamp']:.1f}s\n"
            f"Pose: ({px:.1f}, {py:.1f}, {theta:.1f})\n"
            f"Enc: L={odom['left_encoder_ticks']}  R={odom['right_encoder_ticks']}\n"
            f"Dist: {odom['distance_cm']:.0f} cm\n"
            f"Status: {status}"
        )
        self.info.set_text(info)

        prob = self._log_odds_to_prob(self.frame_grids[idx])
        self.grid_img.set_data(prob)
        gx = int(px / self.res)
        gy = int(py / self.res)
        self.robot_grid.set_data([gx], [gy])

        known = np.sum((prob > 0.45) & (prob < 0.55))
        total = self.gw * self.gh
        mapped = total - known
        pct = 100.0 * mapped / total
        self.grid_info.set_text(f"Mapped: {mapped}/{total} cells ({pct:.1f}%)")

        return (
            self.robot_dot, self.robot_dir, self.traj_line,
            self.grid_img, self.info, self.robot_grid, self.grid_info,
            *self.scan_artists
        )

    # -----------------------------------------------------------------
    def _on_key(self, event):
        if event.key == " ":
            self.paused = not self.paused
            if self.paused:
                self.anim.event_source.stop()
            else:
                self.anim.event_source.start()
        elif event.key == "right":
            nxt = min(self.current_idx + 1, self.num_frames - 1)
            self._update(nxt)
            self.fig.canvas.draw_idle()
        elif event.key == "left":
            prv = max(self.current_idx - 1, 0)
            self._update(prv)
            self.fig.canvas.draw_idle()
        elif event.key == "=" or event.key == "+":
            self.speed = min(5.0, self.speed + 0.5)
            self.anim.event_source.stop()
            self.anim.event_source.interval = int(50 / self.speed)
            if not self.paused:
                self.anim.event_source.start()
        elif event.key == "-":
            self.speed = max(0.5, self.speed - 0.5)
            self.anim.event_source.stop()
            self.anim.event_source.interval = int(50 / self.speed)
            if not self.paused:
                self.anim.event_source.start()

    def show(self):
        plt.show()


def main():
    p = argparse.ArgumentParser(description="SLAM Data Viewer")
    p.add_argument("input", nargs="?", default="sensor_data.json",
                   help="Input JSON data file")
    args = p.parse_args()
    viewer = SlamViewer(args.input)
    viewer.show()


if __name__ == "__main__":
    main()
