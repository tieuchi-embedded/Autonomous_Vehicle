#include "ipc/topic.h"
#include "messages/serial_data.h"
#include "messages/ego_state.h"
#include "messages/lane_state.h"
#include "messages/control_cmd.h"
#include "messages/behavior_cmd.h"
#include "messages/pose2d.h"

static const TopicDescriptor TOPICS[TOPIC_COUNT] = {
    [CAMERA_FRAME] = { "/camera_frame", SHM, 0,                      3, NEW   },
    [SERIAL]       = { "/serial",       MQ,  sizeof(SerialData),     16, OLD   },
    [EGO_STATE]    = { "/ego_state",    MQ,  sizeof(EgoState),        8, OLD   },
    [LANE_STATE]   = { "/lane_state",   MQ,  sizeof(LaneState),       1, NEW   },
    [CONTROL_CMD]  = { "/control_cmd",  MQ,  sizeof(ControlCmd),      4, NEW   },
    [DETECTIONS]   = { "/detections",   MQ,  0,                       1, NEW   },
    [BEHAVIOR_CMD] = { "/behavior_cmd", MQ,  sizeof(BehaviorCmd),     4, NEVER },
    [POSE2D]       = { "/pose2d",       MQ,  sizeof(Pose2D),          8, OLD   },
};

const TopicDescriptor* topic_get(TopicId id) {
    if (id <= INVALID || id >= TOPIC_COUNT) return NULL;
    return &TOPICS[id];
}
