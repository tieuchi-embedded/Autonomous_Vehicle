#!/usr/bin/env python3
"""Python publisher mirror of C pub_demo — publishes IMU_STATE every 1s."""
import sys
import time
from pathlib import Path

# Add bindings/python to path
HERE = Path(__file__).resolve()
sys.path.insert(0, str(HERE.parents[2] / "libipc" / "bindings" / "python"))

import ipc_schema as schema
from ipc import Publisher


def main():
    pub = Publisher(schema.IMU_STATE, payload_size=64)  # payload_size unused for MQ
    print("publishing IMU_STATE via libipc (Python)...")
    seq = 0
    try:
        while True:
            msg = schema.ImuState()
            msg.roll  = 0.01 * seq
            msg.pitch = 0.02 * seq
            msg.yaw   = 0.03 * seq
            msg.az    = 9.81
            pub.publish(msg)
            print(f"pub seq={seq} roll={msg.roll:.2f} pitch={msg.pitch:.2f} yaw={msg.yaw:.2f}")
            seq += 1
            time.sleep(1)
    except KeyboardInterrupt:
        pass
    finally:
        pub.close()


if __name__ == "__main__":
    main()
