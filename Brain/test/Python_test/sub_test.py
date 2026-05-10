#!/usr/bin/env python3
"""Python subscriber mirror of C sub_demo — subscribes IMU_STATE."""
import sys
from pathlib import Path

HERE = Path(__file__).resolve()
sys.path.insert(0, str(HERE.parents[2] / "libipc" / "bindings" / "python"))

import ipc_schema as schema
from ipc import Subscriber


def main():
    sub = Subscriber(schema.IMU_STATE, schema.ImuState)
    print("subscribing IMU_STATE via libipc (Python)...")
    try:
        while True:
            msg = sub.poll(timeout_ms=2000)
            if msg is None:
                print("timeout")
                continue
            print(f"sub seq={msg.h.seq} roll={msg.roll:.2f} "
                  f"pitch={msg.pitch:.2f} yaw={msg.yaw:.2f} az={msg.az:.2f}")
    except KeyboardInterrupt:
        pass
    finally:
        sub.close()


if __name__ == "__main__":
    main()
