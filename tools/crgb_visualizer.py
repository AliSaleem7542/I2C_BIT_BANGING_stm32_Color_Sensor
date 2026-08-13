#!/usr/bin/env python3
"""
CRGB Live Visualizer
---------------------
Reads lines like:
    Sensor 0: C=1234 R=456 G=321 B=210
    Sensor 1: C=980  R=300 G=290 B=280
    ...
from a serial port (STM32 UART output) and shows a LIVE dashboard with:
  - An actual color swatch per sensor (normalized RGB -> real color)
  - A bar chart of raw C/R/G/B counts per sensor
  - A "difference" panel that highlights how far each sensor's color
    is from the average color of all sensors (Euclidean distance in RGB)

Usage:
    pip install pyserial matplotlib --break-system-packages
    python3 crgb_visualizer.py --port /dev/ttyUSB1 --baud 115200

If you don't pass --port, the script will list available ports and ask you
to pick one.
"""

import argparse
import re
import sys
import threading
import time
from collections import deque

import matplotlib

# Force an interactive GUI backend by actually testing the underlying
# toolkit import (matplotlib.use() alone is lazy and won't fail here).
_backend_ok = None
try:
    import tkinter  # noqa: F401
    matplotlib.use("TkAgg", force=True)
    _backend_ok = "TkAgg"
except ImportError:
    pass

if _backend_ok is None:
    try:
        import PyQt5  # noqa: F401
        matplotlib.use("Qt5Agg", force=True)
        _backend_ok = "Qt5Agg"
    except ImportError:
        pass

if _backend_ok is None:
    try:
        import PySide6  # noqa: F401
        matplotlib.use("QtAgg", force=True)
        _backend_ok = "QtAgg"
    except ImportError:
        pass

if _backend_ok is None:
    print("No interactive GUI backend available for matplotlib.")
    print("Install one of these and re-run:")
    print("    sudo apt install python3-tk        # recommended, simplest")
    print("    pip install PyQt5 --break-system-packages")
    sys.exit(1)

import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import matplotlib.animation as animation
import numpy as np

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("pyserial not found. Install it with:")
    print("    pip install pyserial --break-system-packages")
    sys.exit(1)


# ----------------------------------------------------------------------
# Parsing
# ----------------------------------------------------------------------

LINE_RE = re.compile(
    r"Sensor\s+(\d+):\s*C=(\d+)\s+R=(\d+)\s+G=(\d+)\s+B=(\d+)", re.IGNORECASE
)

NUM_SENSORS = 5


def normalize_rgb(c, r, g, b):
    """
    Convert raw TCS3472x RGBC counts to a displayable 0-1 RGB color.
    Standard trick: divide R/G/B by C (clear channel) to remove
    brightness dependence, then scale so the brightest channel hits 1.0.
    """
    c = max(c, 1)  # avoid div by zero
    rn, gn, bn = r / c, g / c, b / c
    m = max(rn, gn, bn, 1e-6)
    return (min(rn / m, 1.0), min(gn / m, 1.0), min(bn / m, 1.0))


# ----------------------------------------------------------------------
# Serial reader thread
# ----------------------------------------------------------------------

class SerialReader(threading.Thread):
    def __init__(self, port, baud, data_lock, sensor_data):
        super().__init__(daemon=True)
        self.port = port
        self.baud = baud
        self.data_lock = data_lock
        self.sensor_data = sensor_data
        self.running = True
        self.connected = False
        self.ser = None

    def run(self):
        while self.running:
            try:
                self.ser = serial.Serial(self.port, self.baud, timeout=1)
                self.connected = True
                print(f"Connected to {self.port} @ {self.baud} baud")
                while self.running:
                    raw = self.ser.readline()
                    if not raw:
                        continue
                    try:
                        line = raw.decode("utf-8", errors="ignore").strip()
                    except Exception:
                        continue
                    if not line:
                        continue
                    m = LINE_RE.search(line)
                    if m:
                        ch = int(m.group(1))
                        c, r, g, b = (int(m.group(i)) for i in range(2, 6))
                        if 0 <= ch < NUM_SENSORS:
                            with self.data_lock:
                                self.sensor_data[ch] = (c, r, g, b, time.time())
            except serial.SerialException as e:
                self.connected = False
                print(f"Serial error: {e}. Retrying in 2s...")
                time.sleep(2)
            except Exception as e:
                print(f"Unexpected error: {e}")
                time.sleep(2)

    def stop(self):
        self.running = False
        if self.ser and self.ser.is_open:
            self.ser.close()


# ----------------------------------------------------------------------
# Picking a serial port interactively if not given
# ----------------------------------------------------------------------

def pick_port():
    ports = list(serial.tools.list_ports.comports())
    if not ports:
        print("No serial ports found. Plug in the board and try again.")
        sys.exit(1)
    print("Available serial ports:")
    for i, p in enumerate(ports):
        print(f"  [{i}] {p.device} - {p.description}")
    idx = input(f"Pick a port [0-{len(ports)-1}]: ").strip()
    try:
        return ports[int(idx)].device
    except (ValueError, IndexError):
        print("Invalid choice.")
        sys.exit(1)


# ----------------------------------------------------------------------
# Main / plotting
# ----------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="Live CRGB sensor visualizer")
    parser.add_argument("--port", default=None, help="Serial port, e.g. /dev/ttyUSB1")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate")
    args = parser.parse_args()

    port = args.port or pick_port()

    data_lock = threading.Lock()
    # sensor_data[i] = (C, R, G, B, timestamp) or None if never seen
    sensor_data = [None] * NUM_SENSORS

    reader = SerialReader(port, args.baud, data_lock, sensor_data)
    reader.start()

    # ---- Figure layout ----
    # Row of 5 color swatches on top, row of 5 bar charts in middle,
    # one "difference from average" bar chart at the bottom.
    fig = plt.figure(figsize=(15, 9))
    fig.canvas.manager.set_window_title("CRGB Live Visualizer")
    gs = fig.add_gridspec(3, NUM_SENSORS, height_ratios=[1.4, 2, 1.3], hspace=0.55, wspace=0.4)

    swatch_axes = [fig.add_subplot(gs[0, i]) for i in range(NUM_SENSORS)]
    bar_axes = [fig.add_subplot(gs[1, i]) for i in range(NUM_SENSORS)]
    diff_ax = fig.add_subplot(gs[2, :])

    swatch_patches = []
    swatch_texts = []
    bar_containers = []

    for i in range(NUM_SENSORS):
        ax = swatch_axes[i]
        ax.set_xlim(0, 1)
        ax.set_ylim(0, 1)
        ax.set_xticks([])
        ax.set_yticks([])
        ax.set_title(f"Sensor {i}", fontsize=12, fontweight="bold")
        patch = mpatches.Rectangle((0, 0), 1, 1, facecolor=(0.5, 0.5, 0.5))
        ax.add_patch(patch)
        txt = ax.text(0.5, -0.18, "no data", ha="center", va="top",
                       transform=ax.transAxes, fontsize=9)
        swatch_patches.append(patch)
        swatch_texts.append(txt)

        bax = bar_axes[i]
        bars = bax.bar(["C", "R", "G", "B"], [0, 0, 0, 0],
                        color=["#888888", "#e74c3c", "#2ecc71", "#3498db"])
        bax.set_ylim(0, 1)  # rescaled dynamically
        bax.set_ylabel("counts" if i == 0 else "")
        bar_containers.append(bars)

    diff_bars = diff_ax.bar(range(NUM_SENSORS), [0] * NUM_SENSORS, color="#9b59b6")
    diff_ax.set_xticks(range(NUM_SENSORS))
    diff_ax.set_xticklabels([f"Sensor {i}" for i in range(NUM_SENSORS)])
    diff_ax.set_title("Color distance from average of all 5 sensors "
                       "(higher = more different)")
    diff_ax.set_ylabel("distance")

    fig.suptitle(f"CRGB Live Visualizer — {port}", fontsize=14, fontweight="bold")

    def update(frame):
        with data_lock:
            snapshot = list(sensor_data)

        now = time.time()
        colors = []
        for i, entry in enumerate(snapshot):
            if entry is None:
                swatch_patches[i].set_facecolor((0.3, 0.3, 0.3))
                swatch_texts[i].set_text("waiting...")
                for bar, val in zip(bar_containers[i], [0, 0, 0, 0]):
                    bar.set_height(0)
                colors.append((0, 0, 0))
                continue

            c, r, g, b, ts = entry
            stale = (now - ts) > 3.0
            color = normalize_rgb(c, r, g, b)
            colors.append(color)

            swatch_patches[i].set_facecolor(color)
            swatch_patches[i].set_alpha(0.4 if stale else 1.0)
            swatch_texts[i].set_text(
                f"C={c} R={r} G={g} B={b}" + ("  (stale)" if stale else "")
            )

            vals = [c, r, g, b]
            max_val = max(vals + [1])
            bar_axes[i].set_ylim(0, max_val * 1.15)
            for bar, val in zip(bar_containers[i], vals):
                bar.set_height(val)

        # difference-from-average panel (only over sensors that have data)
        valid = [(i, colors[i]) for i in range(NUM_SENSORS) if snapshot[i] is not None]
        if valid:
            avg = np.mean([c for _, c in valid], axis=0)
            dists = [0.0] * NUM_SENSORS
            for i, col in valid:
                dists[i] = float(np.linalg.norm(np.array(col) - avg))
            for i, bar in enumerate(diff_bars):
                bar.set_height(dists[i])
                bar.set_color(colors[i] if snapshot[i] is not None else "#555555")
            diff_ax.set_ylim(0, max(max(dists), 0.05) * 1.3)

        status = "CONNECTED" if reader.connected else "RECONNECTING..."
        fig.suptitle(f"CRGB Live Visualizer — {port}  [{status}]",
                     fontsize=14, fontweight="bold",
                     color="green" if reader.connected else "red")

        artists = list(swatch_patches) + list(swatch_texts) + list(diff_bars)
        for bars in bar_containers:
            artists += list(bars)
        return artists

    ani = animation.FuncAnimation(
        fig, update, interval=200, blit=False, cache_frame_data=False
    )

    try:
        plt.tight_layout(rect=[0, 0, 1, 0.95])
        plt.show()
    finally:
        reader.stop()


if __name__ == "__main__":
    main()