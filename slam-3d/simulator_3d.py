#!/usr/bin/env python3
"""
3D SLAM Simulation Data Generator
===================================
Generates 3D ultrasonic scan data for a robot traversing a
150x150cm room with 300cm walls. The sensor is tilted 90°
to scan in elevation, producing 3D point clouds.
"""

import json
import math
import random
import argparse


EPS = 1e-8


def solve_quadratic(a, b, c):
    disc = b * b - 4 * a * c
    if disc < 0:
        return []
    s = math.sqrt(disc)
    t1 = (-b - s) / (2 * a)
    t2 = (-b + s) / (2 * a)
    return sorted([t1, t2])


def intersect_box_face(px, py, pz, dx, dy, dz, face_axis, face_val,
                       x0, x1, y0, y1, z0, z1):
    """Intersect ray with axis-aligned plane, check bounds."""
    if abs(dz) < EPS and face_axis == 2:
        return None
    if abs(dy) < EPS and face_axis == 1:
        return None
    if abs(dx) < EPS and face_axis == 0:
        return None

    if face_axis == 0:
        t = (face_val - px) / dx
    elif face_axis == 1:
        t = (face_val - py) / dy
    else:
        t = (face_val - pz) / dz

    if t < EPS:
        return None

    ix = px + t * dx
    iy = py + t * dy
    iz = pz + t * dz

    if not (x0 - EPS <= ix <= x1 + EPS and
            y0 - EPS <= iy <= y1 + EPS and
            z0 - EPS <= iz <= z1 + EPS):
        return None

    return t


def cast_ray_3d(px, py, pz, az, el, w, h, wall_z,
                rects=None, circles=None):
    """Cast 3D ray, return (distance, hit_x, hit_y, hit_z, surface)."""
    a_rad = math.radians(az)
    e_rad = math.radians(el)
    dx = math.cos(e_rad) * math.cos(a_rad)
    dy = math.cos(e_rad) * math.sin(a_rad)
    dz = math.sin(e_rad)

    if abs(dx) < EPS:
        dx = EPS
    if abs(dy) < EPS:
        dy = EPS
    if abs(dz) < EPS:
        dz = EPS

    best_t = float("inf")
    best_surface = "wall"

    # ---- Walls (finite height: z in [0, wall_z]) ----
    for wall_def in [
        (0, w, 0, 0, h, 0, wall_z),       # x=0
        (1, w, w, 0, h, 0, wall_z),       # x=w
        (2, h, 0, w, 0, 0, wall_z),       # y=0
        (3, h, w, 0, 0, 0, wall_z),       # y=h
    ]:
        wid, _, wx, wy, wz = wall_def[0], wall_def[1], None, None, None
        if wid == 0:
            t = -px / dx
            iy = py + t * dy
            iz = pz + t * dz
            if t > EPS and 0 <= iy <= h and 0 <= iz <= wall_z and t < best_t:
                best_t = t
                best_surface = "wall"
                wx, wy, wz = 0.0, iy, iz
        elif wid == 1:
            t = (w - px) / dx
            iy = py + t * dy
            iz = pz + t * dz
            if t > EPS and 0 <= iy <= h and 0 <= iz <= wall_z and t < best_t:
                best_t = t
                best_surface = "wall"
                wx, wy, wz = w, iy, iz
        elif wid == 2:
            t = -py / dy
            ix = px + t * dx
            iz = pz + t * dz
            if t > EPS and 0 <= ix <= w and 0 <= iz <= wall_z and t < best_t:
                best_t = t
                best_surface = "wall"
                wx, wy, wz = ix, 0.0, iz
        elif wid == 3:
            t = (h - py) / dy
            ix = px + t * dx
            iz = pz + t * dz
            if t > EPS and 0 <= ix <= w and 0 <= iz <= wall_z and t < best_t:
                best_t = t
                best_surface = "wall"
                wx, wy, wz = ix, h, iz

    # ---- Ceiling (z = wall_z) ----
    if dz > 0:
        t = (wall_z - pz) / dz
        if t > EPS:
            ix = px + t * dx
            iy = py + t * dy
            if 0 <= ix <= w and 0 <= iy <= h and t < best_t:
                best_t = t
                best_surface = "ceiling"

    # ---- Rectangular pillar obstacles (finite height) ----
    if rects:
        for ox, oy, ow, oh, obs_h in rects:
            x0, x1 = ox, ox + ow
            y0, y1 = oy, oy + oh
            z0, z1 = 0.0, obs_h

            # 4 vertical faces + top face
            face_checks = [
                (0, x0, x0, x0, y0, y1, z0, z1),
                (0, x1, x1, x1, y0, y1, z0, z1),
                (1, y0, x0, x1, y0, y0, z0, z1),
                (1, y1, x0, x1, y1, y1, z0, z1),
                (2, z1, x0, x1, y0, y1, z1, z1),
            ]
            for fa, fv, bx0, bx1, by0, by1, bz0, bz1 in face_checks:
                result = intersect_box_face(px, py, pz, dx, dy, dz,
                                            fa, fv,
                                            bx0, bx1, by0, by1, bz0, bz1)
                if result is not None and result < best_t:
                    best_t = result
                    best_surface = "obstacle"

    # ---- Cylindrical pillar obstacles (finite height) ----
    if circles:
        for cx, cy, cr, obs_h in circles:
            # Cylindrical surface
            Δx = px - cx
            Δy = py - cy
            a = dx * dx + dy * dy
            b = 2 * (dx * Δx + dy * Δy)
            c = Δx * Δx + Δy * Δy - cr * cr
            roots = solve_quadratic(a, b, c)
            for t in roots:
                if t > EPS:
                    iz = pz + t * dz
                    if 0 <= iz <= obs_h and t < best_t:
                        best_t = t
                        best_surface = "obstacle"

            # Top cap (z = obs_h)
            if dz > 0:
                t = (obs_h - pz) / dz
                if t > EPS:
                    ix = px + t * dx
                    iy = py + t * dy
                    d2 = (ix - cx) ** 2 + (iy - cy) ** 2
                    if d2 <= cr * cr + EPS and t < best_t:
                        best_t = t
                        best_surface = "obstacle"

            # Bottom cap (z = 0)
            if dz < 0:
                t = -pz / dz
                if t > EPS:
                    ix = px + t * dx
                    iy = py + t * dy
                    d2 = (ix - cx) ** 2 + (iy - cy) ** 2
                    if d2 <= cr * cr + EPS and t < best_t:
                        best_t = t
                        best_surface = "obstacle"

    if not math.isfinite(best_t):
        return None

    hx = px + best_t * dx
    hy = py + best_t * dy
    hz = pz + best_t * dz
    return (best_t, hx, hy, hz, best_surface)


def scan_3d(px, py, pz, heading, w, h, wall_z,
            az_step, el_min, el_max, el_step,
            sigma, rects=None, circles=None):
    """Perform full 3D scan at a position."""
    points = []
    for az_rel in range(0, 360, az_step):
        for el in range(el_min, el_max + 1, el_step):
            az_world = (heading + az_rel) % 360
            result = cast_ray_3d(px, py, pz, az_world, el,
                                 w, h, wall_z, rects, circles)
            if result is None:
                continue
            dist, hx, hy, hz, surface = result
            noisy_dist = max(0.1, dist + random.gauss(0, sigma))
            # Recompute hit point with noisy distance
            e_rad = math.radians(el)
            a_rad = math.radians(az_world)
            ndx = math.cos(e_rad) * math.cos(a_rad)
            ndy = math.cos(e_rad) * math.sin(a_rad)
            ndz = math.sin(e_rad)
            nx = px + noisy_dist * ndx
            ny = py + noisy_dist * ndy
            nz = pz + noisy_dist * ndz

            points.append({
                "x": round(nx, 2),
                "y": round(ny, 2),
                "z": round(nz, 2),
                "true_x": round(hx, 2),
                "true_y": round(hy, 2),
                "true_z": round(hz, 2),
                "distance": round(noisy_dist, 2),
                "true_distance": round(dist, 2),
                "azimuth": round(az_world, 1),
                "elevation": el,
                "surface": surface,
            })
    return points


def encoder_ticks(dist_cm, radius, cpr):
    return int(dist_cm * cpr / (2 * math.pi * radius))


def generate(args):
    w, h = args.width, args.height
    wall_z = args.wall_height
    off = args.offset
    si = args.scan_interval
    sigma = args.noise
    wr = args.wheel_radius
    wb = args.wheel_base
    cpr = args.cpr
    pz = args.sensor_height
    az_step = args.azimuth_step
    el_min = args.elevation_min
    el_max = args.elevation_max
    el_step = args.elevation_step

    mode = args.obstacle_mode
    rects = []
    circles = []
    if mode == "square":
        cx, cy = w / 2.0, h / 2.0
        rects = [(cx - 10, cy - 10, 20.0, 20.0, args.obstacle_height)]
    elif mode == "cylinder":
        circles = [(w / 2.0, h / 2.0, 10.0, args.obstacle_height)]

    random.seed(args.seed)

    x = y = off
    heading = 0.0
    records = []
    t = 0.0
    left_enc = right_enc = 0
    total_dist = 0.0

    def save(points, is_turn=False):
        nonlocal t
        records.append({
            "timestamp": round(t, 2),
            "ground_truth": {
                "x": round(x, 2), "y": round(y, 2),
                "theta": round(heading, 2)
            },
            "odometry": {
                "left_encoder_ticks": left_enc,
                "right_encoder_ticks": right_enc,
                "distance_cm": round(total_dist, 2)
            },
            "points": points,
            "point_count": len(points),
            "is_turn": is_turn,
        })
        t += 1.0

    corners = [
        (w - off, off),
        (w - off, h - off),
        (off, h - off),
        (off, off),
    ]

    pts = scan_3d(x, y, pz, heading, w, h, wall_z,
                  az_step, el_min, el_max, el_step,
                  sigma, rects, circles)
    save(pts)

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
            pts = scan_3d(x, y, pz, heading, w, h, wall_z,
                          az_step, el_min, el_max, el_step,
                          sigma, rects, circles)
            save(pts)

        x, y = cx, cy

        turn_rad = math.radians(90)
        wheel_travel = (wb / 2.0) * turn_rad
        left_enc -= encoder_ticks(wheel_travel, wr, cpr)
        right_enc += encoder_ticks(wheel_travel, wr, cpr)
        heading = (heading + 90) % 360
        save([], is_turn=True)

    total_points = sum(r["point_count"] for r in records)
    return {
        "environment": {
            "width": w, "height": h, "wall_height": wall_z,
            "obstacles": [[ox, oy, ow, oh, oz] for (ox, oy, ow, oh, oz) in rects],
            "circles": [[cx, cy, cr, oz] for (cx, cy, cr, oz) in circles],
        },
        "robot": {
            "sensor_height_cm": pz,
            "azimuth_step_deg": az_step,
            "elevation_range_deg": [el_min, el_max],
            "elevation_step_deg": el_step,
            "wheel_radius_cm": wr,
            "wheel_base_cm": wb,
            "encoder_cpr": cpr,
            "speed_cm_s": args.speed,
            "wall_offset_cm": off,
            "scan_interval_cm": si,
            "noise_sigma": sigma,
        },
        "data": records,
    }


def main():
    p = argparse.ArgumentParser(description="3D SLAM Simulation Data Generator")
    p.add_argument("--width", type=float, default=150.0)
    p.add_argument("--height", type=float, default=150.0)
    p.add_argument("--wall-height", type=float, default=300.0, help="Wall height (cm)")
    p.add_argument("--speed", type=float, default=10.0)
    p.add_argument("--offset", type=float, default=10.0)
    p.add_argument("--scan-interval", type=float, default=10.0)
    p.add_argument("--noise", type=float, default=0.5)
    p.add_argument("--output", "-o", default="sensor_data_3d.json")
    p.add_argument("--wheel-radius", type=float, default=3.0)
    p.add_argument("--wheel-base", type=float, default=12.0)
    p.add_argument("--cpr", type=int, default=20)
    p.add_argument("--seed", type=int, default=42)
    p.add_argument("--sensor-height", type=float, default=15.0,
                   help="Sensor height above ground (cm)")
    p.add_argument("--azimuth-step", type=int, default=5,
                   help="Horizontal scan step (deg)")
    p.add_argument("--elevation-min", type=int, default=-60,
                   help="Min elevation angle (deg)")
    p.add_argument("--elevation-max", type=int, default=60,
                   help="Max elevation angle (deg)")
    p.add_argument("--elevation-step", type=int, default=2,
                   help="Vertical scan step (deg)")
    p.add_argument("--obstacle-height", type=float, default=100.0,
                   help="Obstacle height (cm)")
    p.add_argument("--obstacle-mode", choices=["none", "square", "cylinder"],
                   default="none")
    args = p.parse_args()

    data = generate(args)
    with open(args.output, "w") as f:
        json.dump(data, f, indent=2)

    total_pts = sum(r["point_count"] for r in data["data"])
    print(f"Generated {len(data['data'])} records -> {args.output}")
    print(f"  Scan records: {sum(1 for d in data['data'] if d['points'])}")
    print(f"  Turn records:  {sum(1 for d in data['data'] if d['is_turn'])}")
    print(f"  Total 3D points: {total_pts}")


if __name__ == "__main__":
    main()
