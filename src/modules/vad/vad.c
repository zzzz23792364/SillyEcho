
#include<stdio.h>

#include "vad.h"


vad_status_t current_status = LISTENING;

void 
webrtc_vad_free(VadInst* pVad)
{
	WebRtcVad_Free(pVad); 
}

VadInst*
webrtc_vad_init()
{
	VadInst* pVad = NULL;  
    if (WebRtcVad_Create(&pVad))  
    {  
        perror("WebRtcVad_Create failed!");  
        return NULL;  
    }  
  
    if (WebRtcVad_Init(pVad))  
    {  
        perror("WebRtcVad_Init failed!");  
        return NULL;  
    }  
  
    if (WebRtcVad_set_mode(pVad, 3))  
    {  
        perror("WebRtcVad_set_mode failed!");  
        return NULL;  
    } 

	return pVad;
}

vad_status_t
webrtc_vad_process(VadInst* pVad,short *audio_data,int audio_len,
							int frame_len,int sample_rate)
{
	int i=0,vad_res;
	int have_voice;
	int num_per_judge = audio_len/frame_len;
	int sum_of_judge = 0;
	
	for(i=0;i<num_per_judge;i++)
	{
		vad_res = WebRtcVad_Process(pVad, sample_rate,audio_data+i*frame_len, frame_len); 

		if(vad_res<0)
		{
			printf("webrtc_vad_process failed\n");
			return ERROR;
		}

		sum_of_judge += vad_res;
	}
	
	if((sum_of_judge * 2 ) > num_per_judge)
		have_voice = 1;
	else
		have_voice = 0;

	
	if(current_status == LISTENING)
	{
		if(have_voice)
			current_status = RUNNING;
	}
	else if(current_status == RUNNING)
	{
		if(have_voice == 0)
			current_status = LISTENING;
	}
	return current_status;	
}

