"""
EAI / YDLIDAR S4 LiDAR - Log File Analyzer
Analyzes previously captured raw hex log or CSV files

Usage:
  python s4_analyze_log.py lidar_logs/s4_raw_20260708_122500.log
  python s4_analyze_log.py lidar_logs/s4_parsed_20260708_122500.csv
"""

import re
import sys
import os
import struct
import math
from collections import Counter

PH_HEADER = 0x55AA
SDK_UNIT64 = 64.0
SDK_ANGLE360 = 360.0
MM_PER_UNIT = 4.0

def calc_checksum(packet: bytes, lsn: int) -> int:
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

def verify_checksum(packet: bytes, lsn: int) -> bool:
    if len(packet) < 10 + lsn * 2:
        return False
    cs_stored = struct.unpack_from('<H', packet, 8)[0]
    return cs_stored == calc_checksum(packet, lsn)

def parse_angle(raw: int) -> float:
    return (raw >> 1) / SDK_UNIT64

def parse_distance(si: int) -> tuple:
    dist_mm = si >> 2
    return dist_mm, dist_mm / (MM_PER_UNIT * 1000.0)

def angle_correction_triangle(dist_mm: float) -> float:
    if dist_mm == 0:
        return 0.0
    ca = math.atan(21.8 * (155.3 - dist_mm) / (155.3 * dist_mm))
    return ca * 180.0 / math.pi

def analyze_raw_log(filepath: str):
    print(f"Analyzing raw hex log: {os.path.basename(filepath)}\n")

    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    # Extract hex bytes from dump lines
    hex_bytes = bytearray()
    for line in content.split('\n'):
        if line.startswith('#'):
            continue
        m = re.match(r'^[0-9A-Fa-f]+\s+((?:[0-9A-Fa-f]{2}\s+)+)', line)
        if m:
            for h in m.group(1).split():
                hex_bytes.append(int(h, 16))

    print(f"Total hex bytes extracted: {len(hex_bytes)}")

    packets = []
    samples = []
    zero_count = 0
    cs_ok_count = 0
    cs_fail_count = 0

    i = 0
    while i < len(hex_bytes) - 1:
        if hex_bytes[i] != 0xAA or hex_bytes[i + 1] != 0x55:
            i += 1
            continue

        if i + 10 > len(hex_bytes):
            break

        lsn = hex_bytes[i + 3]
        pkt_len = 10 + lsn * 2
        if i + pkt_len > len(hex_bytes):
            break

        pkt = bytes(hex_bytes[i:i + pkt_len])
        ct = pkt[2]
        fsa_raw = struct.unpack_from('<H', pkt, 4)[0]
        lsa_raw = struct.unpack_from('<H', pkt, 6)[0]

        start_angle = parse_angle(fsa_raw)
        end_angle = parse_angle(lsa_raw)
        if end_angle < start_angle:
            end_angle += SDK_ANGLE360

        cs_ok = verify_checksum(pkt, lsn)
        if cs_ok:
            cs_ok_count += 1
        else:
            cs_fail_count += 1

        is_zero = (ct & 0x01) == 1
        if is_zero:
            zero_count += 1

        for j in range(lsn):
            si = struct.unpack_from('<H', pkt, 10 + j * 2)[0]
            dist_mm, dist_m = parse_distance(si)

            if lsn == 1:
                angle = start_angle
            else:
                angle = start_angle + (end_angle - start_angle) * j / (lsn - 1)
            if angle >= SDK_ANGLE360:
                angle -= SDK_ANGLE360

            corr = angle_correction_triangle(dist_mm)
            angle_c = angle + corr
            if angle_c < 0:
                angle_c += SDK_ANGLE360
            elif angle_c >= SDK_ANGLE360:
                angle_c -= SDK_ANGLE360

            samples.append({
                'angle_raw': angle,
                'angle_corrected': angle_c,
                'dist_mm': dist_mm,
                'dist_m': dist_m,
            })

        packets.append({
            'offset': i,
            'ct': ct,
            'is_zero': is_zero,
            'lsn': lsn,
            'start_angle': start_angle,
            'end_angle': end_angle,
            'cs_ok': cs_ok,
        })

        i += pkt_len

    # Print summary
    print(f"\n{'=' * 55}")
    print(f"Point Cloud Analysis")
    print(f"{'=' * 55}")
    print(f"Total packets:       {len(packets)}")
    print(f"Total samples:       {len(samples)}")
    print(f"Zero packets (rotations): {zero_count}")
    print(f"CS OK:               {cs_ok_count}")
    print(f"CS FAIL:             {cs_fail_count}")
    if cs_fail_count == 0:
        print(f"  -> Checksum:       ALL PASS (YDLIDAR SDK algorithm)")

    if samples:
        angles_raw = [s['angle_raw'] for s in samples]
        angles_corr = [s['angle_corrected'] for s in samples]
        valid = [s for s in samples if s['dist_mm'] > 0]

        print(f"\n{'─' * 55}")
        print(f"Distance Statistics")
        print(f"{'─' * 55}")
        print(f"Valid returns:       {len(valid)} / {len(samples)} ({len(valid) * 100 / len(samples):.1f}%)")
        if valid:
            vd = [s['dist_m'] for s in valid]
            print(f"Min distance:       {min(vd):.3f} m")
            print(f"Max distance:       {max(vd):.3f} m")
            print(f"Avg distance:       {sum(vd) / len(vd):.3f} m")
            print(f"Median:             {sorted(vd)[len(vd) // 2]:.3f} m")

        print(f"\n{'─' * 55}")
        print(f"Angle Coverage")
        print(f"{'─' * 55}")
        print(f"Raw angle range:    {min(angles_raw):.1f}° ~ {max(angles_raw):.1f}°")
        print(f"Corrected range:    {min(angles_corr):.1f}° ~ {max(angles_corr):.1f}°")
        print(f"Zero returns:       {sum(1 for s in samples if s['dist_mm'] == 0)}")

        if zero_count >= 2:
            # Distance distribution by angle (10-degree bins)
            bins = {}
            for s in valid:
                bin_key = int(s['angle_corrected'] / 10) * 10
                if bin_key not in bins:
                    bins[bin_key] = []
                bins[bin_key].append(s['dist_m'])
            print(f"\n{'─' * 55}")
            print(f"Distance by 10° Sector (valid returns)")
            print(f"{'─' * 55}")
            for deg in sorted(bins):
                vals = bins[deg]
                cnt = len(vals)
                avg = sum(vals) / cnt
                bar = '█' * int(cnt / max(len(v) for v in bins.values()) * 30)
                print(f"  {deg:3d}°-{deg + 9:3d}°  {avg:.3f}m  {bar}")

    return packets, samples

def analyze_csv(filepath: str):
    print(f"Analyzing CSV: {os.path.basename(filepath)}\n")

    import csv
    with open(filepath, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        rows = list(reader)

    print(f"Total rows: {len(rows)}")

    if not rows:
        return

    angles = [float(r['angle_corr_deg']) for r in rows]
    dists_m = [float(r['dist_m']) for r in rows]
    dists_mm = [float(r['dist_mm']) for r in rows]
    valid = [float(r['dist_m']) for r in rows if float(r['dist_mm']) > 0]

    zero_packets = sum(1 for r in rows if r['packet_type'] == 'ZERO')
    print(f"Zero packets (rotations): {zero_packets}")
    print(f"Checksum OK: {sum(1 for r in rows if r['checksum_ok'] == 'True')} / {len(rows)}")

    cs_fails = sum(1 for r in rows if r['checksum_ok'] == 'False')
    if cs_fails == 0:
        print(f"  -> ALL PASS")

    print(f"\n{'─' * 55}")
    print(f"Distance Statistics")
    print(f"{'─' * 55}")
    print(f"Valid returns: {len(valid)} / {len(rows)} ({len(valid) * 100 / len(rows):.1f}%)")
    if valid:
        print(f"Min: {min(valid):.3f}m  Max: {max(valid):.3f}m  Avg: {sum(valid) / len(valid):.3f}m")

    print(f"\n{'─' * 55}")
    print(f"Angle range: {min(angles):.1f}° ~ {max(angles):.1f}°")

    if zero_packets >= 2:
        # Estimate scan frequency from session_time_ms
        times = [float(r['session_time_ms']) for r in rows if r['packet_type'] == 'ZERO']
        times = sorted(set(times))
        if len(times) >= 2:
            total_s = (times[-1] - times[0]) / 1000.0
            freq = (len(times) - 1) / total_s if total_s > 0 else 0
            print(f"Estimated scan frequency: {freq:.2f} Hz")


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Analyze YDLIDAR S4 LiDAR logs\n")
        print("Usage:")
        print("  python s4_analyze_log.py <file.log>    (hex dump)")
        print("  python s4_analyze_log.py <file.csv>    (parsed CSV)")
        sys.exit(1)

    path = sys.argv[1]
    if not os.path.exists(path):
        print(f"File not found: {path}")
        sys.exit(1)

    if path.lower().endswith('.csv'):
        analyze_csv(path)
    else:
        analyze_raw_log(path)
