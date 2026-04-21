#include "ipc/topic.h"
#include "messages/imu_state.h"
#include "messages/wheel_odom.h"
#include "messages/ego_state.h"
#include "messages/lane_state.h"
#include "messages/control_cmd.h"

static const TopicDescriptor TOPICS[TOPIC_COUNT] = {
    [CAMERA_FRAME] = { "/camera_frame", SHM, 0,                  3, NEW   },
    [IMU_STATE]    = { "/imu_state",    MQ,  sizeof(ImuState),  16, OLD   },
    [WHEEL_ODOM]   = { "/wheel_odom",   MQ,  sizeof(WheelOdom), 16, OLD   },
    [EGO_STATE]    = { "/ego_state",    MQ,  sizeof(EgoState),   8, OLD   },
    // depth=1: NEW → consumer always gets latest
    [LANE_STATE]   = { "/lane_state",   MQ,  sizeof(LaneState),  1, NEW   },
    [CONTROL_CMD]  = { "/control_cmd",  MQ,  sizeof(ControlCmd), 4, NEVER },
};

const TopicDescriptor* topic_get(TopicId id) {
    if (id <= INVALID || id >= TOPIC_COUNT) return NULL;
    return &TOPICS[id];
}
