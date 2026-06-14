"""
ctypes Structure definitions mirroring messages/*.h (source of truth).
Layouts must match C structs byte-for-byte.
"""
import ctypes as ct


# TopicId — must match libipc/include/ipc/topic.h
INVALID       = 0
CAMERA_FRAME  = 1
EGO_STATE     = 2
LANE_STATE    = 3
CONTROL_CMD   = 4
OBJECT_STATE  = 5
BEHAVIOUR_CMD = 6
LOCATION      = 7

# TransportKind
SHM = 1
MQ  = 2


class MessageHeader(ct.Structure):
    _fields_ = [
        ("topic", ct.c_uint32),
        ("seq",   ct.c_uint32),
        ("ts_ns", ct.c_uint64),
    ]


class EgoState(ct.Structure):
    """Raw vehicle telemetry from the serial port."""
    _fields_ = [
        ("h",     MessageHeader),
        ("yaw",   ct.c_float),
        ("pitch", ct.c_float),
        ("roll",  ct.c_float),
        ("rpm",   ct.c_float),
    ]


class LaneState(ct.Structure):
    _fields_ = [
        ("h",               MessageHeader),
        ("heading_err_rad", ct.c_float),
        ("offset_cm",       ct.c_float),
    ]


class ControlCmd(ct.Structure):
    _fields_ = [
        ("h",         MessageHeader),
        ("rpm",       ct.c_float),
        ("steer_deg", ct.c_float),
    ]


class ObjectState(ct.Structure):
    _fields_ = [
        ("h",          MessageHeader),
        ("cls",        ct.c_uint32),
        ("distance",   ct.c_float),
        ("confidence", ct.c_float),
    ]


class BehaviourCmd(ct.Structure):
    _fields_ = [
        ("h",              MessageHeader),
        ("target_speed",   ct.c_float),
        ("target_heading", ct.c_float),
        ("mode",           ct.c_uint32),
    ]


class Location(ct.Structure):
    _fields_ = [
        ("h",       MessageHeader),
        ("x",       ct.c_float),
        ("y",       ct.c_float),
        ("heading", ct.c_float),
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
    CAMERA_FRAME:  CameraFrame,
    EGO_STATE:     EgoState,
    LANE_STATE:    LaneState,
    CONTROL_CMD:   ControlCmd,
    OBJECT_STATE:  ObjectState,
    BEHAVIOUR_CMD: BehaviourCmd,
    LOCATION:      Location,
}
