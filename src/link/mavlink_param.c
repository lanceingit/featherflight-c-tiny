#include "board.h"

#include "link_mavlink.h"
#include "mavlink_param.h"

#include "param.h"
#include "debug.h"
#include "timer.h"



MavlinkParam mavlink_param = {
    .status = PARAM_IDLE,
    .send_index = 0,
};

static MavlinkParam* this = &mavlink_param;

void mavlink_param_send(char* name, float val, uint16_t index)
{
    mavlink_msg_param_value_send(MAV_CH, name, val, MAV_PARAM_TYPE_REAL32, PARAM_LIST_MAX, index);
    PRINT("param send[%d]{%d}:%s->%f\n", index, PARAM_LIST_MAX, name, val);
}

void mavlink_param_handle(mavlink_message_t* msg)
{
    switch(msg->msgid) {
    case MAVLINK_MSG_ID_PARAM_REQUEST_READ: {
        PRINT("MAVLINK_MSG_ID_PARAM_REQUEST_READ!\n");
        this->status = PARAM_IDLE;
        mavlink_param_request_read_t data;
        mavlink_msg_param_request_read_decode(msg, &data);
        PRINT("param request:%s[%d]\n", data.param_id, data.param_index);
        mavlink_param_send(param_list[data.param_index].name, *param_list[data.param_index].val, data.param_index);
        break;
    }
    case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
        PRINT("MAVLINK_MSG_ID_PARAM_REQUEST_LIST!\n");
        this->status = PARAM_SENDING;
        this->send_index = 0;
        break;
    case MAVLINK_MSG_ID_PARAM_VALUE:
        PRINT("MAVLINK_MSG_ID_PARAM_VALUE!\n");
        break;
    case MAVLINK_MSG_ID_PARAM_SET: {
        PRINT("MAVLINK_MSG_ID_PARAM_SET!\n");
        mavlink_param_set_t data;
        mavlink_msg_param_set_decode(msg, &data);
        PRINT("param set:%s->%f type:%d\n", data.param_id, data.param_value, data.param_type);       
        param_set_val(data.param_id, data.param_value);     
        float val;
        param_get_val(data.param_id, &val);
        mavlink_param_send(data.param_id, val, param_get_index(data.param_id));        
        break;
    }

    default:
        break;
    }
}

void mavlink_param_run(void)
{
    TIMER_DEF(param_send_time)
    if(this->status == PARAM_SENDING && timer_check(&param_send_time, 100*1000)) {
        mavlink_param_send(param_list[this->send_index].name, *param_list[this->send_index].val, this->send_index);
        
        this->send_index++;
        if(this->send_index >= PARAM_LIST_MAX) {
            this->status = PARAM_IDLE;
        }
    }
}
