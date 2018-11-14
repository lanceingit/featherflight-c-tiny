#include <fcntl.h>
#include <unistd.h>
#include <sys/msg.h>


struct msg_rec_s
{
	long msg_type;
	float flow_x;
	float flow_y;
	unsigned char quality;
	unsigned short flow_ver;
};

struct flow_s
{
    float vel_x;
    float vel_y;
    uint8_t quality;
};

struct flow_s flow;
struct flow_s* this=&flow;

int msg_id = -1;
struct msg_rec_s flow_data;

bool flow_linux_init(void)
{
	msg_id = msgget((key_t)1234, 0666 | IPC_CREAT);
	if(msg_id == -1){
		return false;
	}else{
		return true;
	}	
}

void flow_linux_update(void)
{
	if(msgrcv(msg_id, (void*)&flow_data, sizeof(flow_data),0,IPC_NOWAIT) > 0){
		this->vel_x = flow_data.flow_x*0.0011f;
		this->vel_y = flow_data.flow_y*0.0011f;	
        //TODO:rotate
		this->quality = flow_data.quality;
	}	
}

