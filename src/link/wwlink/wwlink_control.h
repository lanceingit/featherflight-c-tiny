#pragma once


#define WWLINK_ITEM_CONTROL_JOYSTICK	0
#define WWLINK_ITEM_CONTROL_STATUS_RATE	1
#define WWLINK_ITEM_CONTROL_CALIBRATE	2
#define WWLINK_ITEM_CONTROL_MODE		3

typedef struct {
	int16_t roll;
	int16_t pitch;
	int16_t yaw;
	int16_t throttle;
}wwlink_control_joystick_s;

typedef struct {
	uint8_t type;
	uint8_t rate;
}wwlink_control_status_rate_s;

typedef struct {
	uint8_t type;
	uint8_t command;
}wwlink_control_calibrate_s;

typedef struct PACKED{
	uint8_t mode;
	float param1;
	float param2;
}wwlink_control_mode_s;


static inline void wwlink_decode_control_joystick(wwlink_message_t* msg_rev, float* roll, float* pitch, float* yaw, float* thr)
{
	wwlink_control_joystick_s * data;
	data = (wwlink_control_joystick_s *)msg_rev->data;
	
	*roll = (float)data->roll / 1000.0f;
	*pitch = -(float)data->pitch / 1000.0f;
	*yaw	= (float)data->yaw / 1000.0f;
	*thr	= (float)data->throttle/ 1000.0f;
	// printf("r:%f p:%f y:%f t:%f\n", roll, pitch, yaw, thr); 
}

static inline void wwlink_decode_control_status_rate(wwlink_message_t* msg_rev)
{
	// wwlink_control_status_rate_s * data;
	// data = (wwlink_control_status_rate_s *)msg_rev->data;
	// printf("recv rate: %d %d\n", data->type,data->rate);
//	wwlink_stream_set_rate(data->type,data->rate);
}

static inline void wwlink_decode_control_calibrate(wwlink_message_t* msg_rev)
{
	wwlink_control_calibrate_s * data;
	data = (wwlink_control_calibrate_s *)msg_rev->data;
	
	if(data->type == 0){
		// if(data->command == 0)
			//imu_set_acc_calib(true);
	}
}

static inline void wwlink_decode_control_mode(wwlink_message_t* msg_rev)
{
	// wwlink_control_mode_s * data;
	// data = (wwlink_control_mode_s *)msg_rev->data;	
}


static inline void wwlink_control_handle(wwlink_message_t* msg)
{
	switch(msg->subitem_id){
		// case WWLINK_ITEM_CONTROL_JOYSTICK:
		// 	wwlink_decode_control_joystick(msg);
		// 	break;
		case WWLINK_ITEM_CONTROL_STATUS_RATE:
			wwlink_encode_system_ack(WWLINK_ACK_OK,WWLINK_ITEM_CONTROL,msg->subitem_id);
			wwlink_decode_control_status_rate(msg);
			break;
		case WWLINK_ITEM_CONTROL_CALIBRATE:
			wwlink_encode_system_ack(WWLINK_ACK_OK,WWLINK_ITEM_CONTROL,msg->subitem_id);
			wwlink_decode_control_calibrate(msg);
			break;
		case WWLINK_ITEM_CONTROL_MODE:
			wwlink_encode_system_ack(WWLINK_ACK_OK,WWLINK_ITEM_CONTROL,msg->subitem_id);
			wwlink_decode_control_mode(msg);							
			break;
        default:break;    
	}
}


