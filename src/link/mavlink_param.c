#include "board.h"

#include "link_mavlink.h"
#include "mavlink_param.h"

#include "param.h"


MavlinkParam mavlink_param = {
//    .status = LOG_IDLE,
};

static MavlinkParam* this = &mavlink_param;


//static void handle_log_request_list(mavlink_message_t* msg)
//{
////    mavlink_log_request_list_t packet;
////    mavlink_msg_log_request_list_decode(msg, &packet);

//    mavlink_msg_log_entry_send(MAV_CH,
//    						     0, 1, 1, 0, log_get_size());
//}

//static void handle_log_request_data(mavlink_message_t* msg)
//{
//	log_stop();
//    
//    mavlink_log_request_data_t req;
//    
//    mavlink_msg_log_request_data_decode(msg, &req);
//    
//    this->log_data.ofs = req.ofs;

//	uint32_t last_data = log_get_size() - this->log_data.ofs;

////	while(last_data > 0)
//	{
//		memset(this->log_data.data, 0, MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN);
//		this->log_data.count = log_read(this->log_data.ofs, this->log_data.data, last_data > MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN ? MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN : last_data);
//        if(this->log_data.count > 0) {
//            this->log_data.id = 0;
//            mavlink_msg_log_data_send_struct(MAV_CH, &this->log_data);

//            this->log_data.ofs += this->log_data.count;
//            last_data =	log_get_size() - this->log_data.ofs;
//        }
//	}
//    
//    if(last_data > 0)
//    {
//        this->status = LOG_READ;
//    }
//}

void mavlink_param_handle(mavlink_message_t* msg)
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

void mavlink_param_run(void)
{
//	if(this->status == LOG_READ)
//	{
//        uint32_t last_data = log_get_size() - this->log_data.ofs;

//    //	while(last_data > 0)
//        {
//            memset(this->log_data.data, 0, MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN);
//            this->log_data.count = log_read(this->log_data.ofs, this->log_data.data, last_data > MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN ? MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN : last_data);
//            if(this->log_data.count > 0) {
//                this->log_data.id = 0;
//                mavlink_msg_log_data_send_struct(MAV_CH, &this->log_data);

//                this->log_data.ofs += this->log_data.count;
//                last_data =	log_get_size() - this->log_data.ofs;
//            }            
//        }
//        
//        if(last_data <= 0)
//        {
//            this->status = LOG_IDLE;
//            
//            this->log_data.ofs = 0;
//        }
//	}
}
