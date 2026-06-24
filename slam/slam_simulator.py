#!/usr/bin/env python3
"""
SLAM Simulation Data Generator
===============================
Generates simulated ultrasonic + encoder + IMU sensor data for a
differential drive robot traversing a rectangular environment.

Robot follows walls at a fixed offset, scans every 10cm using
two back-to-back ultrasonic sensors with 180 deg servo sweep.
"""

import json
import math
import random
import argparse


def cast_ray_circle(px, py, dx, dy, cx, cy, r, eps=1e-8):
    dx0 = px - cx
    dy0 = py - cy
    a = dx * dx + dy * dy
    b = 2.0 * (dx * dx0 + dy * dy0)
    c = dx0 * dx0 + dy0 * dy0 - r * r
    disc = b * b - 4.0 * a * c
    if disc < 0:
        return float("inf")
    sqrt_disc = math.sqrt(disc)
    t1 = (-b - sqrt_disc) / (2.0 * a)
    t2 = (-b + sqrt_disc) / (2.0 * a)
    t_min = float("inf")
    for t in (t1, t2):
        if t > eps and t < t_min:
            t_min = t
    return t_min


def cast_ray(px, py, angle_deg, w, h, rects=None, circles=None):
    a = math.radians(angle_deg)
    dx = math.cos(a)
    dy = math.sin(a)
    eps = 1e-8
    if abs(dx) < eps:
        dx = eps
    if abs(dy) < eps:
        dy = eps

    best_t = float("inf")

    def check(t):
        nonlocal best_t
        if t > eps and t < best_t:
            xx = px + t * dx
            yy = py + t * dy
            if -eps <= xx <= w + eps and -eps <= yy <= h + eps:
                best_t = t

    if dx > 0:
        check((w - px) / dx)
    elif dx < 0:
        check(-px / dx)
    if dy > 0:
        check((h - py) / dy)
    elif dy < 0:
        check(-py / dy)

    if rects:
        for ox, oy, ow, oh in rects:
            for xx in (ox, ox + ow):
                t = (xx - px) / dx
                if t > eps:
                    yy = py + t * dy
                    if oy - eps <= yy <= oy + oh + eps and t < best_t:
                        best_t = t
            for yy in (oy, oy + oh):
                t = (yy - py) / dy
                if t > eps:
                    xx = px + t * dx
                    if ox - eps <= xx <= ox + ow + eps and t < best_t:
                        best_t = t

    if circles:
        for cx, cy, cr in circles:
            t = cast_ray_circle(px, py, dx, dy, cx, cy, cr, eps)
            if t < best_t:
                best_t = t

    if not math.isfinite(best_t):
        return max(w, h) * 2
    return best_t


def scan_360(px, py, heading, w, h, step_deg, sigma, rects=None, circles=None):
    readings = []
    for a_rel in range(0, 360, step_deg):
        a_world = (heading + a_rel) % 360
        d = cast_ray(px, py, a_world, w, h, rects, circles)
        dn = max(0.1, d + random.gauss(0, sigma))
        sensor_id = 0 if a_rel < 180 else 1
        readings.append({
            "angle_relative": a_rel,
            "angle_world": round(a_world, 2),
            "distance": round(dn, 2),
            "true_distance": round(d, 2),
            "sensor_id": sensor_id
        })
    return readings


def encoder_ticks(dist_cm, radius, cpr):
    return int(dist_cm * cpr / (2 * math.pi * radius))


def simulate_imu(vx, vy, vz, turn_rate, dt=0.01):
    accel = {"x": round(vx, 4), "y": round(vy, 4), "z": round(vz, 4)}
    gyro = {"x": 0.0, "y": 0.0, "z": round(math.degrees(turn_rate), 4)}
    return accel, gyro


def generate(args):
    w, h = args.width, args.height
    off = args.offset
    si = args.scan_interval
    astep = args.angle_step
    sigma = args.noise
    wr = args.wheel_radius
    wb = args.wheel_base
    cpr = args.cpr

    mode = args.obstacle_mode
    rects = list(args.obstacles) if args.obstacles else []
    circles = []
    if mode == "square":
        cx, cy = w / 2.0, h / 2.0
        half = 10.0
        rects = [(cx - half, cy - half, 20.0, 20.0)]
    elif mode == "cylinder":
        circles = [(w / 2.0, h / 2.0, 10.0)]

    random.seed(args.seed)

    x = y = off
    heading = 0.0
    records = []
    t = 0.0
    left_enc = right_enc = 0
    total_dist = 0.0

    def save(scan, is_turn=False):
        nonlocal t
        records.append({
            "timestamp": round(t, 2),
            "ground_truth": {"x": round(x, 2), "y": round(y, 2), "theta": round(heading, 2)},
            "odometry": {
                "left_encoder_ticks": left_enc,
                "right_encoder_ticks": right_enc,
                "distance_cm": round(total_dist, 2)
            },
            "imu": {"accel": {"x": 0.0, "y": 0.0, "z": 9.81},
                    "gyro": {"x": 0.0, "y": 0.0, "z": 0.0}},
            "scan": scan,
            "is_turn": is_turn
        })
        t += 1.0

    corners = [
        (w - off, off),
        (w - off, h - off),
        (off, h - off),
        (off, off),
    ]

    save(scan_360(x, y, heading, w, h, astep, sigma, rects, circles))

    for cx, cy in corners:
        dx, dy = cx - x, cy - y
        dist = math.hypot(dx, dy)
        steps = max(1, round(dist / si))
        sx, sy = dx / steps, dy / steps

        for _ in range(steps):
            x += sx
            y += sy
            step_dist = math.hypot(sx, sy)
            total_dist += step_dist
            ticks = encoder_ticks(step_dist, wr, cpr)
            left_enc += ticks
            right_enc += ticks
            save(scan_360(x, y, heading, w, h, astep, sigma, rects, circles))

        x, y = cx, cy

        turn_rad = math.radians(90)
        wheel_travel = (wb / 2.0) * turn_rad
        left_enc -= encoder_ticks(wheel_travel, wr, cpr)
        right_enc += encoder_ticks(wheel_travel, wr, cpr)
        heading = (heading + 90) % 360
        save([], is_turn=True)

    return {
        "environment": {"width": w, "height": h, "obstacles": rects, "circles": circles},
        "robot": {
            "wheel_radius_cm": wr,
            "wheel_base_cm": wb,
            "encoder_cpr": cpr,
            "speed_cm_s": args.speed,
            "wall_offset_cm": off,
            "scan_interval_cm": si,
            "scan_angle_step_deg": astep,
            "noise_sigma": sigma
        },
        "data": records
    }


def main():
    p = argparse.ArgumentParser(description="SLAM Simulation Data Generator")
    p.add_argument("--width", type=float, default=150.0, help="Room width (cm)")
    p.add_argument("--height", type=float, default=150.0, help="Room height (cm)")
    p.add_argument("--speed", type=float, default=10.0, help="Robot speed (cm/s)")
    p.add_argument("--offset", type=float, default=10.0, help="Distance from walls (cm)")
    p.add_argument("--scan-interval", type=float, default=10.0, help="Distance between scans (cm)")
    p.add_argument("--angle-step", type=int, default=2, help="Scan angular resolution (deg)")
    p.add_argument("--noise", type=float, default=0.5, help="Ultrasonic noise sigma (cm)")
    p.add_argument("--output", default="sensor_data.json", help="Output JSON file")
    p.add_argument("--wheel-radius", type=float, default=3.0, help="Wheel radius (cm)")
    p.add_argument("--wheel-base", type=float, default=12.0, help="Wheel base (cm)")
    p.add_argument("--cpr", type=int, default=20, help="Encoder counts per revolution")
    p.add_argument("--seed", type=int, default=42, help="Random seed")
    p.add_argument("--obstacle-mode", choices=["none", "square", "cylinder"],
                   default="none", help="Obstacle preset (none | square pillar | cylinder)")
    p.add_argument("--obstacles", nargs="*", help="Custom rectangles as x,y,w,h ...")
    args = p.parse_args()

    custom_rects = []
    if args.obstacles:
        for s in args.obstacles:
            parts = [float(v) for v in s.split(",")]
            if len(parts) == 4:
                custom_rects.append(tuple(parts))
    args.obstacles = custom_rects

    data = generate(args)
    with open(args.output, "w") as f:
        json.dump(data, f, indent=2)
    print(f"Generated {len(data['data'])} records -> {args.output}")
    print(f"  Scan records: {sum(1 for d in data['data'] if d['scan'])}")
    print(f"  Turn records: {sum(1 for d in data['data'] if d['is_turn'])}")


if __name__ == "__main__":
    main()
