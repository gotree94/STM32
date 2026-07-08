"""
EAI / YDLIDAR S4 LiDAR Serial Logger & Packet Analyzer
- Connects at 115200 baud, 8N1
- Sends scan command (A5 60)
- Logs raw hex data to file
- Parses point cloud packets in real-time
- Saves parsed data to CSV

Protocol (from YDLIDAR SDK):
  - Scan data: AA 55 packets immediately after A5 60 (no A5 5A header)
  - Packet: PH(2B)=0x55AA | CT(1B) | LSN(1B) | FSA(2B) | LSA(2B) | CS(2B) | Si(2B*LSN)
  - Checksum: XOR of 16-bit LE words: PH ^ FSA ^ Si[0]... ^ (LSN<<8|CT) ^ LSA
  - Angle: (raw >> 1) / 64.0 deg, interpolated between FSA and LSA
  - Distance: Si >> 2 = mm; meters = mm / 4000.0 (triangle LiDAR)
"""

import serial
import struct
import time
import sys
import os
import math
from datetime import datetime

# ─── Configuration ───────────────────────────────────────────────
COM_PORT = "COM6"
BAUDRATE = 115200
TIMEOUT = 0.1

# YDLIDAR commands
CMD_SCAN = b'\xA5\x60'
CMD_STOP = b'\xA5\x65'

# ─── Protocol Constants ──────────────────────────────────────────
PH_HEADER = 0x55AA       # Packet header (AA 55 in LE bytes)
SDK_UNIT64 = 64.0         # Angle conversion factor
SDK_ANGLE360 = 360.0      # Full circle
TRI_CORR_K = 21.8         # Triangle angle correction constant
TRI_CORR_REF = 155.3      # Triangle angle correction reference
MM_PER_UNIT = 4.0         # Distance: Si >> 2 => mm; 1 unit = 4 mm

# ─── Checksum (YDLIDAR SDK Algorithm) ───────────────────────────

def calc_checksum(packet: bytes, lsn: int) -> int:
    """Calculate checksum: XOR all 16-bit LE words in protocol-doc order.
    
    Order: PH ^ FSA ^ Si[0] ^ Si[1] ^ ... ^ (LSN<<8|CT) ^ LSA
    """
    ph = struct.unpack_from('<H', packet, 0)[0]
    fsa = struct.unpack_from('<H', packet, 4)[0]
    lsa = struct.unpack_from('<H', packet, 6)[0]
    ct = packet[2]

    cs = ph
    cs ^= fsa

    for i in range(lsn):
        si = struct.unpack_from('<H', packet, 10 + i * 2)[0]
        cs ^= si

    cs ^= (lsn << 8) | ct
    cs ^= lsa
    return cs

def verify_checksum(packet: bytes, lsn: int) -> bool:
    """Verify the 2-byte checksum at bytes 8-9."""
    if len(packet) < 10 + lsn * 2:
        return False
    cs_stored = struct.unpack_from('<H', packet, 8)[0]
    cs_computed = calc_checksum(packet, lsn)
    return cs_stored == cs_computed

# ─── Parsing ─────────────────────────────────────────────────────

def parse_angle(raw: int) -> tuple:
    """Convert raw uint16 to degrees. bit0 = check bit (should be 1)."""
    return (raw >> 1) / SDK_UNIT64, raw & 0x01

def parse_distance(si: int) -> tuple:
    """Si >> 2 = distance in mm. meters = mm / 4000.0 for triangle LiDAR."""
    dist_mm = si >> 2
    return dist_mm, dist_mm / (MM_PER_UNIT * 1000.0)

def angle_correction_triangle(dist_mm: float) -> float:
    """Triangle LiDAR angle correction from YDLIDAR SDK."""
    if dist_mm == 0:
        return 0.0
    ca = math.atan(TRI_CORR_K * (TRI_CORR_REF - dist_mm) / (TRI_CORR_REF * dist_mm))
    return ca * 180.0 / math.pi

def find_packet_start(data: bytes) -> int:
    """Find first AA 55 (0x55AA LE) packet header."""
    for i in range(len(data) - 1):
        if data[i] == 0xAA and data[i + 1] == 0x55:
            return i
    return -1

def parse_packet(packet: bytes):
    """Parse one point cloud packet."""
    if len(packet) < 10:
        return None

    ph = struct.unpack_from('<H', packet, 0)[0]
    if ph != PH_HEADER:
        return None

    ct = packet[2]
    lsn = packet[3]
    expected_len = 10 + lsn * 2

    if len(packet) < expected_len:
        return None

    fsa_raw = struct.unpack_from('<H', packet, 4)[0]
    lsa_raw = struct.unpack_from('<H', packet, 6)[0]

    cs_ok = verify_checksum(packet, lsn)

    start_angle, start_cb = parse_angle(fsa_raw)
    end_angle, end_cb = parse_angle(lsa_raw)

    # Handle 360-degree wrap
    if end_angle < start_angle:
        end_angle += SDK_ANGLE360

    is_zero = (ct & 0x01) == 1

    samples = []
    for i in range(lsn):
        si = struct.unpack_from('<H', packet, 10 + i * 2)[0]
        dist_mm, dist_m = parse_distance(si)

        if lsn == 1:
            angle = start_angle
        else:
            angle = start_angle + (end_angle - start_angle) * i / (lsn - 1)

        if angle >= SDK_ANGLE360:
            angle -= SDK_ANGLE360

        # Apply triangle angle correction
        corr = angle_correction_triangle(dist_mm)
        angle_corrected = angle + corr
        if angle_corrected < 0:
            angle_corrected += SDK_ANGLE360
        elif angle_corrected >= SDK_ANGLE360:
            angle_corrected -= SDK_ANGLE360

        samples.append({
            'index': i,
            'angle_raw': round(angle, 2),
            'angle_corrected': round(angle_corrected, 2),
            'distance_mm': dist_mm,
            'distance_m': round(dist_m, 3),
            'intensity_flag': (si >> 1) & 0x01,
            'raw': si,
        })

    return {
        'ct': ct,
        'is_zero_packet': is_zero,
        'lsn': lsn,
        'start_angle_deg': round(start_angle, 2),
        'end_angle_deg': round(end_angle, 2),
        'start_cb': start_cb,
        'end_cb': end_cb,
        'checksum_ok': cs_ok,
        'samples': samples,
        'raw_length': expected_len,
    }

# ─── Hex Dump ───────────────────────────────────────────────────

def hex_dump(data: bytes, offset: int = 0) -> str:
    lines = []
    for i in range(0, len(data), 16):
        chunk = data[i:i + 16]
        hex_part = ' '.join(f'{b:02X}' for b in chunk)
        hex_part = hex_part.ljust(16 * 3 - 1)
        ascii_part = ''.join(chr(b) if 32 <= b < 127 else '.' for b in chunk)
        lines.append(f'{offset + i:08X}  {hex_part}  |{ascii_part}|')
    return '\n'.join(lines)

# ─── Main ────────────────────────────────────────────────────────

def main():
    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
    log_dir = os.path.join(os.path.dirname(__file__) or '.', 'lidar_logs')
    os.makedirs(log_dir, exist_ok=True)

    raw_log_path = os.path.join(log_dir, f's4_raw_{timestamp}.log')
    csv_path = os.path.join(log_dir, f's4_parsed_{timestamp}.csv')

    print(f"=== EAI S4 LiDAR Logger ===")
    print(f"Port: {COM_PORT} @ {BAUDRATE} baud, 8N1")
    print(f"Raw log: {raw_log_path}")
    print(f"CSV:     {csv_path}")
    print()

    # Connect
    try:
        ser = serial.Serial(
            port=COM_PORT,
            baudrate=BAUDRATE,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=TIMEOUT,
        )
    except serial.SerialException as e:
        print(f"ERROR: {e}")
        sys.exit(1)

    print(f"Connected. Sending stop + flush...")
    ser.write(CMD_STOP)
    time.sleep(0.2)
    ser.reset_input_buffer()
    ser.reset_output_buffer()
    time.sleep(0.2)

    f_raw = open(raw_log_path, 'w', encoding='utf-8')
    f_csv = open(csv_path, 'w', encoding='utf-8')

    f_raw.write(f"# EAI S4 LiDAR Raw Log\n")
    f_raw.write(f"# Date: {datetime.now().isoformat()}\n")
    f_raw.write(f"# Port: {COM_PORT} @ {BAUDRATE}\n")
    f_raw.write(f"# Command: SCAN (A5 60)\n")
    f_raw.write(f"# {'=' * 60}\n\n")

    f_csv.write("timestamp,session_time_ms,packet_type,lsn,idx,"
                "angle_raw_deg,angle_corr_deg,dist_mm,dist_m,intensity,checksum_ok\n")

    print("Sending scan command (A5 60)...")
    ser.write(CMD_SCAN)

    buffer = bytearray()
    total_packets = 0
    total_samples = 0
    zero_packets = 0
    parse_errors = 0
    start_time = time.monotonic()

    try:
        while True:
            chunk = ser.read(1024)
            if not chunk:
                continue

            buffer.extend(chunk)

            while True:
                ps = find_packet_start(buffer)
                if ps < 0:
                    break

                if ps > 0:
                    skipped = buffer[:ps]
                    f_raw.write(f"# SKIPPED {len(skipped)} bytes:\n")
                    f_raw.write(hex_dump(skipped) + "\n\n")
                    buffer = buffer[ps:]

                if len(buffer) < 10:
                    break

                lsn = buffer[3]
                pkt_len = 10 + lsn * 2

                if len(buffer) < pkt_len:
                    break

                packet_data = bytes(buffer[:pkt_len])
                buffer = buffer[pkt_len:]

                result = parse_packet(packet_data)
                if result is None:
                    parse_errors += 1
                    f_raw.write(f"# PARSE ERROR:\n")
                    f_raw.write(hex_dump(packet_data) + "\n\n")
                    continue

                total_packets += 1
                total_samples += result['lsn']
                if result['is_zero_packet']:
                    zero_packets += 1

                now = datetime.now()
                elapsed = time.monotonic() - start_time

                # Raw log
                f_raw.write(f"# --- Pkt#{total_packets} @ {now.isoformat()} "
                           f"(+{elapsed * 1000:.0f}ms) ---\n")
                f_raw.write(f"# CT=0x{result['ct']:02X} "
                           f"{'ZERO' if result['is_zero_packet'] else 'DATA'} "
                           f"LSN={result['lsn']} "
                           f"Angle: {result['start_angle_deg']}->{result['end_angle_deg']} "
                           f"CS={'OK' if result['checksum_ok'] else 'FAIL'}\n")
                f_raw.write(hex_dump(packet_data))
                f_raw.write("\n\n")

                # CSV
                ts_ms = f"{elapsed * 1000:.1f}"
                ptype = 'ZERO' if result['is_zero_packet'] else 'DATA'
                for s in result['samples']:
                    f_csv.write(
                        f"{now.strftime('%H:%M:%S.%f')[:-3]},{ts_ms},{ptype},"
                        f"{result['lsn']},{s['index']},"
                        f"{s['angle_raw']},{s['angle_corrected']},"
                        f"{s['distance_mm']},{s['distance_m']},"
                        f"{s['intensity_flag']},{result['checksum_ok']}\n"
                    )

                # Display
                rate = total_packets / elapsed if elapsed > 0 else 0
                if result['is_zero_packet']:
                    sps = total_samples / elapsed if elapsed > 0 else 0
                    sys.stdout.write(
                        f"\r[{now.strftime('%H:%M:%S')}] "
                        f"Pkts:{total_packets} | Pts:{total_samples} | "
                        f"{zero_packets}rot | {sps:.0f}pts/s   "
                    )
                elif total_packets % 20 == 0:
                    sys.stdout.write(
                        f"\r[{now.strftime('%H:%M:%S')}] "
                        f"Pkts:{total_packets} | Pts:{total_samples} | "
                        f"{rate:.0f}p/s   "
                    )
                sys.stdout.flush()

    except KeyboardInterrupt:
        print("\n\nStopped by user.")
    finally:
        ser.write(CMD_STOP)
        time.sleep(0.1)
        ser.close()
        f_raw.close()
        f_csv.close()

        elapsed = time.monotonic() - start_time
        print(f"\n{'=' * 50}")
        print(f"Duration: {elapsed:.1f}s")
        print(f"Packets: {total_packets}  |  Samples: {total_samples}")
        print(f"Rotations (zero pkts): {zero_packets}")
        print(f"Parse errors: {parse_errors}")
        if zero_packets:
            print(f"Avg scan rate: {zero_packets / elapsed:.2f} Hz")
        if total_packets:
            print(f"Avg packet rate: {total_packets / elapsed:.1f} pkts/s")
        print(f"\nLogs saved to: {log_dir}")


if __name__ == '__main__':
    main()
