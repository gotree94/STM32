#!/usr/bin/env python3
"""
3D SLAM Data Viewer
====================
Reads 3D ultrasonic scan data and visualizes the point cloud
in an interactive 3D view using matplotlib.
"""

import json
import math
import argparse
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import animation
from mpl_toolkits.mplot3d.art3d import Poly3DCollection


class SlamViewer3D:
    def __init__(self, data_file):
        with open(data_file) as f:
            self.data = json.load(f)

        self.env = self.data["environment"]
        self.frames = self.data["data"]
        self.num_frames = len(self.frames)
        self.current_idx = 0
        self.paused = True
        self.speed = 1.0

        w = self.env["width"]
        h = self.env["height"]
        wall_z = self.env["wall_height"]

        self.fig = plt.figure(figsize=(14, 8))
        self.ax = self.fig.add_subplot(111, projection="3d")
        self.fig.canvas.mpl_connect("key_press_event", self._on_key)

        # ── Draw environment ─────────────────────────────────
        self._draw_walls(w, h, wall_z)
        self._draw_obstacles()

        # ── Scatter plot for point cloud ─────────────────────
        self.scatter = self.ax.scatter([], [], [], s=1, c=[], cmap="gist_rainbow",
                                       alpha=0.6, picker=False)

        # ── Robot position and trajectory ────────────────────
        self.robot_dot, = self.ax.plot([], [], [], "ro", markersize=6, zorder=5)
        self.traj_line, = self.ax.plot([], [], [], "b-", alpha=0.5, linewidth=1)
        self.traj_x, self.traj_y, self.traj_z = [], [], []

        # ── Info text ─────────────────────────────────────────
        self.info = self.ax.text2D(0.02, 0.98, "", transform=self.ax.transAxes,
                                    va="top", fontsize=9, family="monospace",
                                    bbox=dict(boxstyle="round", facecolor="wheat", alpha=0.9))

        self.help_text = self.ax.text2D(
            0.98, 0.02, "SPACE: play/pause  |  +/-: speed  |  <-/->: step  |  Drag: rotate",
            transform=self.ax.transAxes, va="bottom", ha="right",
            fontsize=8, color="gray")

        # ── Pre-accumulate all points for color scaling ──────
        all_x, all_y, all_z = [], [], []
        for fr in self.frames:
            for pt in fr.get("points", []):
                all_x.append(pt["x"])
                all_y.append(pt["y"])
                all_z.append(pt["z"])
        self._global_z_min = min(all_z) if all_z else 0
        self._global_z_max = max(all_z) if all_z else 1

        # ── Set view ─────────────────────────────────────────
        self.ax.set_xlim(-10, w + 10)
        self.ax.set_ylim(-10, h + 10)
        self.ax.set_zlim(-10, wall_z + 10)
        self.ax.set_xlabel("X (cm)")
        self.ax.set_ylabel("Y (cm)")
        self.ax.set_zlabel("Z (cm)")

        # ── Animation ────────────────────────────────────────
        self.anim = animation.FuncAnimation(
            self.fig, self._update, interval=50,
            frames=self.num_frames, repeat=True, blit=False,
            cache_frame_data=False)

    # ── Environment drawing ─────────────────────────────────────
    def _draw_walls(self, w, h, wall_z):
        # Floor
        verts = [[(0, 0, 0), (w, 0, 0), (w, h, 0), (0, h, 0)]]
        self.ax.add_collection3d(Poly3DCollection(
            verts, facecolors=(0.15, 0.15, 0.15, 0.3),
            edgecolors=(0.3, 0.3, 0.3), linewidth=0.5))

        # Walls with grid lines
        for (x1, y1), (x2, y2) in [((0, 0), (w, 0)), ((w, 0), (w, h)),
                                     ((w, h), (0, h)), ((0, h), (0, 0))]:
            verts = [[(x1, y1, 0), (x2, y2, 0), (x2, y2, wall_z), (x1, y1, wall_z)]]
            self.ax.add_collection3d(Poly3DCollection(
                verts, facecolors=(0.35, 0.35, 0.45, 0.15),
                edgecolors=(0.3, 0.3, 0.4), linewidth=1.0))

        # Top edges
        for (x1, y1), (x2, y2) in [((0, 0), (w, 0)), ((w, 0), (w, h)),
                                     ((w, h), (0, h)), ((0, h), (0, 0))]:
            self.ax.plot([x1, x2], [y1, y2], [wall_z, wall_z],
                         color=(0.4, 0.4, 0.5), linewidth=1, alpha=0.5)

        # Vertical corner lines
        for x, y in [(0, 0), (w, 0), (w, h), (0, h)]:
            self.ax.plot([x, x], [y, y], [0, wall_z],
                         color=(0.4, 0.4, 0.5), linewidth=1, alpha=0.5)

    def _draw_obstacles(self):
        # Rectangular obstacles
        for obs in self.env.get("obstacles", []):
            if len(obs) >= 5:
                ox, oy, ow, oh, oz = obs[:5]
                x0, x1 = ox, ox + ow
                y0, y1 = oy, oy + oh
                z0, z1 = 0, oz
                faces = [
                    [(x0, y0, z0), (x1, y0, z0), (x1, y1, z0), (x0, y1, z0)],  # bottom
                    [(x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1)],  # top
                    [(x0, y0, z0), (x0, y1, z0), (x0, y1, z1), (x0, y0, z1)],  # left
                    [(x1, y0, z0), (x1, y1, z0), (x1, y1, z1), (x1, y0, z1)],  # right
                    [(x0, y0, z0), (x1, y0, z0), (x1, y0, z1), (x0, y0, z1)],  # front
                    [(x0, y1, z0), (x1, y1, z0), (x1, y1, z1), (x0, y1, z1)],  # back
                ]
                self.ax.add_collection3d(Poly3DCollection(
                    faces, facecolors=(0.4, 0.2, 0.2, 0.4),
                    edgecolors=(0.6, 0.3, 0.3), linewidth=0.8))

        # Cylindrical obstacles — approximate with 16-sided prism
        for circ in self.env.get("circles", []):
            if len(circ) >= 4:
                cx, cy, cr, oz = circ[:4]
                n = 16
                angles = [2 * math.pi * i / n for i in range(n)]
                pts = [(cx + cr * math.cos(a), cy + cr * math.sin(a)) for a in angles]

                # Side faces
                for i in range(n):
                    x1, y1 = pts[i]
                    x2, y2 = pts[(i + 1) % n]
                    face = [(x1, y1, 0), (x2, y2, 0), (x2, y2, oz), (x1, y1, oz)]
                    self.ax.add_collection3d(Poly3DCollection(
                        [face], facecolors=(0.4, 0.2, 0.2, 0.4),
                        edgecolors=(0.6, 0.3, 0.3), linewidth=0.5))

                # Top and bottom
                bot = [(x, y, 0) for x, y in pts]
                top = [(x, y, oz) for x, y in pts]
                self.ax.add_collection3d(Poly3DCollection(
                    [bot], facecolors=(0.4, 0.2, 0.2, 0.4), linewidth=0))
                self.ax.add_collection3d(Poly3DCollection(
                    [top], facecolors=(0.4, 0.2, 0.2, 0.4), linewidth=0))

    # ── Update ──────────────────────────────────────────────────
    def _update(self, idx):
        self.current_idx = idx
        frame = self.frames[idx]
        gt = frame.get("ground_truth", {})
        px, py = gt.get("x", 0), gt.get("y", 0)
        theta = gt.get("theta", 0)
        pz_val = self.data["robot"].get("sensor_height_cm", 10)

        # ── Accumulate points up to current frame ──────────────
        xs, ys, zs, colors = [], [], [], []
        for i in range(idx + 1):
            for pt in self.frames[i].get("points", []):
                xs.append(pt["x"])
                ys.append(pt["y"])
                zs.append(pt["z"])
                # Color by height
                if self._global_z_max > self._global_z_min:
                    norm = (pt["z"] - self._global_z_min) / (self._global_z_max - self._global_z_min)
                else:
                    norm = 0.5
                colors.append(norm)

        if xs:
            self.scatter._offsets3d = (xs, ys, zs)
            self.scatter.set_array(np.array(colors))
            self.scatter.set_clim(0, 1)
        else:
            self.scatter._offsets3d = ([], [], [])

        # ── Trajectory ─────────────────────────────────────────
        self.traj_x.append(px)
        self.traj_y.append(py)
        self.traj_z.append(pz_val)
        self.traj_line.set_data(self.traj_x, self.traj_y)
        self.traj_line.set_3d_properties(self.traj_z)

        # ── Robot ──────────────────────────────────────────────
        self.robot_dot.set_data([px], [py])
        self.robot_dot.set_3d_properties([pz_val])

        # ── View angle (auto-rotate slowly) ────────────────────
        # Only auto-rotate if not interacting
        if not self.paused:
            pass

        # ── Info ───────────────────────────────────────────────
        status = "TURNING" if frame.get("is_turn") else "SCANNING"
        points_now = len(frame.get("points", []))
        total_pts = len(xs)
        info = (
            f"[{idx + 1}/{self.num_frames}]  t={frame.get('timestamp', 0):.1f}s\n"
            f"Pose: ({px:.1f}, {py:.1f}, {theta:.1f})\n"
            f"Sensor Z: {pz_val:.0f} cm\n"
            f"Points this frame: {points_now}\n"
            f"Total points: {total_pts}\n"
            f"Status: {status}"
        )
        self.info.set_text(info)

        return self.scatter, self.robot_dot, self.traj_line, self.info

    # ── Key events ──────────────────────────────────────────────
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
        elif event.key in ("=", "+"):
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
        plt.tight_layout()
        plt.show()


def main():
    p = argparse.ArgumentParser(description="3D SLAM Data Viewer")
    p.add_argument("file", nargs="?", default="sensor_data_3d.json", help="Input JSON file")
    p.add_argument("-i", "--input", dest="file", help="Input JSON file (alternative)")
    args = p.parse_args()
    viewer = SlamViewer3D(args.file)
    viewer.show()


if __name__ == "__main__":
    main()
