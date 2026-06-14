#include "ipc/topic.h"
#include "messages/ego_state.h"
#include "messages/lane_state.h"
#include "messages/control_cmd.h"
#include "messages/object_state.h"
#include "messages/behaviour_cmd.h"
#include "messages/location.h"

static const TopicDescriptor TOPICS[TOPIC_COUNT] = {
    [CAMERA_FRAME]  = { "/camera_frame",  SHM, 0,                    3 },
    [EGO_STATE]     = { "/ego_state",     SHM, sizeof(EgoState),     3 },
    [LANE_STATE]    = { "/lane_state",    SHM, sizeof(LaneState),    3 },
    [CONTROL_CMD]   = { "/control_cmd",   MQ,  sizeof(ControlCmd),   4 },
    [OBJECT_STATE]  = { "/object_state",  MQ,  sizeof(ObjectState),  4 },
    [BEHAVIOUR_CMD] = { "/behaviour_cmd", MQ,  sizeof(BehaviourCmd), 4 },
    [LOCATION]      = { "/location",      MQ,  sizeof(Location),     8 },
};

const TopicDescriptor* topic_get(TopicId id) {
    if (id <= INVALID || id >= TOPIC_COUNT) return NULL;
    return &TOPICS[id];
}
