#include "board.h"

#include "link_mavlink.h"
#include "mavlink_log.h"

#include "log.h"

enum log_status
{
	LOG_READ,
	LOG_IDLE,
};



static mavlink_log_data_t log_data;

static enum log_status status = LOG_IDLE;

static void handle_log_request_list(mavlink_message_t* msg);
static void handle_log_request_data(mavlink_message_t* msg);

void mavlink_log_handle(mavlink_message_t* msg)
{
	switch(msg->msgid)
	{
		case MAVLINK_MSG_ID_LOG_REQUEST_LIST:
			handle_log_request_list(msg);
			break;
		case MAVLINK_MSG_ID_LOG_REQUEST_DATA:
	        handle_log_request_data(msg);
			break;
		case MAVLINK_MSG_ID_LOG_ERASE:
	//                    handle_log_request_erase(msg, dataflash);
			break;
		case MAVLINK_MSG_ID_LOG_REQUEST_END:
	//                    handle_log_request_end(msg, dataflash);
			break;

		default:break;
	}
}

static void handle_log_request_list(mavlink_message_t* msg)
{
//    mavlink_log_request_list_t packet;
//    mavlink_msg_log_request_list_decode(msg, &packet);

    mavlink_msg_log_entry_send(MAV_CH,
    						     0, 1, 1, 0, log_get_size());
}

static void handle_log_request_data(mavlink_message_t* msg)
{
	log_stop();
    
    mavlink_log_request_data_t req;
    
    mavlink_msg_log_request_data_decode(msg, &req);
    
    log_data.ofs = req.ofs;

	uint32_t last_data = log_get_size() - log_data.ofs;

//	while(last_data > 0)
	{
		memset(log_data.data, 0, MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN);
		log_data.count = log_read(log_data.ofs, log_data.data, last_data > MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN ? MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN : last_data);
		log_data.id = 0;
        mavlink_msg_log_data_send_struct(MAV_CH, &log_data);

		log_data.ofs += log_data.count;
		last_data =	log_get_size() - log_data.ofs;
	}
    
    if(last_data > 0)
    {
        status = LOG_READ;
    }
}

void mavlink_log_run(void)
{
	if(status == LOG_READ)
	{
        uint32_t last_data = log_get_size() - log_data.ofs;

    //	while(last_data > 0)
        {
            memset(log_data.data, 0, MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN);
            log_data.count = log_read(log_data.ofs, log_data.data, last_data > MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN ? MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN : last_data);
            log_data.id = 0;
            mavlink_msg_log_data_send_struct(MAV_CH, &log_data);

            log_data.ofs += log_data.count;
            last_data =	log_get_size() - log_data.ofs;
        }
        
        if(last_data <= 0)
        {
            status = LOG_IDLE;
            
            log_data.ofs = 0;
        }

	}
}
