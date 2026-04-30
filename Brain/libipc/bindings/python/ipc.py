"""
Python bindings for libipc — wraps the C bus API via ctypes.

Loads liblibipc.so. Search order:
  1. $LIBIPC_PATH (full path to .so)
  2. <repo>/Brain/build/libipc/liblibipc.so
  3. system loader (LD_LIBRARY_PATH)
"""
import ctypes as ct
import os
import time
from pathlib import Path
from typing import Optional, Type

import ipc_schema as schema
from ipc_schema import MessageHeader


def _load_lib() -> ct.CDLL:
    env = os.environ.get("LIBIPC_PATH")
    if env:
        return ct.CDLL(env)

    # repo-local default: Brain/build/libipc/liblibipc.so
    here = Path(__file__).resolve()
    # bindings/python/ipc.py -> parents[3] == Brain/
    brain = here.parents[3]
    candidate = brain / "build" / "libipc" / "liblibipc.so"
    if candidate.exists():
        return ct.CDLL(str(candidate))

    # fall back to system loader
    return ct.CDLL("liblibipc.so")


_lib = _load_lib()

# Opaque handles
_publisher_p  = ct.c_void_p
_subscriber_p = ct.c_void_p

_lib.ipc_publish_open.argtypes = [ct.c_int, ct.c_size_t]
_lib.ipc_publish_open.restype  = _publisher_p

_lib.ipc_publish.argtypes = [_publisher_p, ct.c_void_p, ct.c_size_t]
_lib.ipc_publish.restype  = ct.c_int

_lib.ipc_publish_close.argtypes = [_publisher_p]
_lib.ipc_publish_close.restype  = None

_lib.ipc_subscribe_open.argtypes = [ct.c_int]
_lib.ipc_subscribe_open.restype  = _subscriber_p

_lib.ipc_poll.argtypes = [_subscriber_p, ct.c_void_p, ct.c_size_t, ct.c_int]
_lib.ipc_poll.restype  = ct.c_int

_lib.ipc_subscribe_close.argtypes = [_subscriber_p]
_lib.ipc_subscribe_close.restype  = None


def _now_ns() -> int:
    return time.clock_gettime_ns(time.CLOCK_MONOTONIC)


class Publisher:
    """
    Auto-fills MessageHeader (topic, seq, ts_ns) on each publish().
    User only sets payload fields.
    """
    def __init__(self, topic_id: int, payload_size: int):
        self._topic = topic_id
        self._size  = payload_size
        self._seq   = 0
        self._h = _lib.ipc_publish_open(topic_id, payload_size)
        if not self._h:
            raise RuntimeError(f"ipc_publish_open failed (topic={topic_id})")

    def publish(self, msg: ct.Structure) -> int:
        msg.h.topic = self._topic
        msg.h.seq   = self._seq
        msg.h.ts_ns = _now_ns()
        self._seq += 1
        rc = _lib.ipc_publish(self._h, ct.byref(msg), ct.sizeof(msg))
        return rc

    def close(self) -> None:
        if self._h:
            _lib.ipc_publish_close(self._h)
            self._h = None

    def __enter__(self):
        return self

    def __exit__(self, *_):
        self.close()

    def __del__(self):
        self.close()


class Subscriber:
    """
    poll(timeout_ms) returns:
      - Structure instance with new data
      - None on timeout / no new data
    Raises RuntimeError on transport error.
    """
    def __init__(self, topic_id: int, struct_cls: Type[ct.Structure]):
        self._cls = struct_cls
        self._h = _lib.ipc_subscribe_open(topic_id)
        if not self._h:
            raise RuntimeError(f"ipc_subscribe_open failed (topic={topic_id})")

    def poll(self, timeout_ms: int = 0) -> Optional[ct.Structure]:
        buf = self._cls()
        rc = _lib.ipc_poll(self._h, ct.byref(buf), ct.sizeof(buf), timeout_ms)
        if rc == 0:
            return buf
        if rc == 1:
            return None
        raise RuntimeError(f"ipc_poll failed rc={rc}")

    def close(self) -> None:
        if self._h:
            _lib.ipc_subscribe_close(self._h)
            self._h = None

    def __enter__(self):
        return self

    def __exit__(self, *_):
        self.close()

    def __del__(self):
        self.close()
