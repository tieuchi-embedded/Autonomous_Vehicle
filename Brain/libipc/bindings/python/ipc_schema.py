"""
ctypes Structure definitions mirroring messages/*.h (source of truth).
Layouts must match C structs byte-for-byte.
"""
import ctypes as ct


# TopicId — must match libipc/include/ipc/topic.h
INVALID      = 0
CAMERA_FRAME = 1
IMU_STATE    = 2
WHEEL_ODOM   = 3
EGO_STATE    = 4
LANE_STATE   = 5
CONTROL_CMD  = 6

# TransportKind
SHM = 1
MQ  = 2

# DropPolicy
DROP_NEW   = 1
DROP_OLD   = 2
DROP_NEVER = 3


class MessageHeader(ct.Structure):
    _fields_ = [
        ("topic", ct.c_uint32),
        ("seq",   ct.c_uint32),
        ("ts_ns", ct.c_uint64),
    ]


class ImuState(ct.Structure):
    _fields_ = [
        ("h",     MessageHeader),
        ("roll",  ct.c_float),
        ("pitch", ct.c_float),
        ("yaw",   ct.c_float),
        ("wx",    ct.c_float),
        ("wy",    ct.c_float),
        ("wz",    ct.c_float),
        ("ax",    ct.c_float),
        ("ay",    ct.c_float),
        ("az",    ct.c_float),
    ]


class WheelOdom(ct.Structure):
    _fields_ = [
        ("h",     MessageHeader),
        ("speed", ct.c_float),
        ("ticks", ct.c_int32),
        ("dist",  ct.c_float),
    ]


class EgoState(ct.Structure):
    _fields_ = [
        ("h",     MessageHeader),
        ("yaw",   ct.c_float),
        ("pitch", ct.c_float),
        ("roll",  ct.c_float),
    ]


class LaneState(ct.Structure):
    _fields_ = [
        ("h",               MessageHeader),
        ("heading_err_rad", ct.c_float),
    ]


class ControlCmd(ct.Structure):
    _fields_ = [
        ("h",        MessageHeader),
        ("speed",    ct.c_float),
        ("steering", ct.c_float),
        ("brake",    ct.c_float),
    ]


class CameraFrame(ct.Structure):
    """Header for SHM camera frame. Pixel data follows in same SHM slot."""
    _fields_ = [
        ("h",         MessageHeader),
        ("width",     ct.c_uint32),
        ("height",    ct.c_uint32),
        ("stride",    ct.c_uint32),
        ("fmt",       ct.c_uint32),  # 0=BGR 1=RGB 2=YUV420
        ("data_size", ct.c_uint32),
    ]


TOPIC_STRUCT = {
    CAMERA_FRAME: CameraFrame,
    IMU_STATE:    ImuState,
    WHEEL_ODOM:   WheelOdom,
    EGO_STATE:    EgoState,
    LANE_STATE:   LaneState,
    CONTROL_CMD:  ControlCmd,
}
