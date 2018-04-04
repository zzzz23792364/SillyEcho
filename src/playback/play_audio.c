/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : play_audio.c
 * Author        : Zengchao
 * Date          : 2018-04-05
 * Description   : .C file function description
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-04-05
 *   Author      : Zengchao
 *   Modification: Created file

*************************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>

#include "mad.h"
#include "log.h"
#include "cbb_types.h"
#include "play_audio.h"

typedef U8 ID[4];

typedef struct wave_header_s
{
	ID  id;
	U32 size;
	ID format;
}wave_header_t;

typedef struct fmt_chunk_s
{
	ID subid;
	U32 subsize;
	U16 audio_fmt;//如果为PCM，改值为 1
	U16 num_chn;
	U32 sample_rate; 
	U32 byte_rate;	// = SampleRate*NumChannels*uiBitsPerSample/8 
	U16 block_align; // = NumChannels*BitsPerSample/8
	U16 bits_per_sample;//每个采样点的bit数，一般为8,16,32
}fmt_chunk_t;


typedef struct data_chunk_s
{
	ID subid;
	U32 subsize;
}data_chunk_t;


typedef struct mad_user_data_s
{
	FILE * out_fd;
	unsigned long audio_len;
	unsigned char *audio_data;
}mad_user_data_t;


U16 mp3_channels = 0;
U32 mp3_sample_rate = 0;

play_list_t *play_list_head = NULL;

pthread_mutex_t play_lock = PTHREAD_MUTEX_INITIALIZER;	
pthread_cond_t play_ready = PTHREAD_COND_INITIALIZER;

static inline signed int scale(mad_fixed_t sample)     
{     
    sample += (1L << (MAD_F_FRACBITS - 16));     
    if (sample >= MAD_F_ONE)     
    sample = MAD_F_ONE - 1;     
    else if (sample < -MAD_F_ONE)     
    sample = -MAD_F_ONE;     
    return sample >> (MAD_F_FRACBITS + 1 - 16);     
}  


static inline int sal_sys_aplay(char *audio_name)
{
	int ret;
	char sys_cmd[256];
	memset(sys_cmd,0,sizeof(sys_cmd));

	strcpy(sys_cmd,"aplay ");
	strcat(sys_cmd,audio_name);

	ret = sal_system(sys_cmd);

	return ret;
}

/*****************************************************************************
 * Function      : pcm_to_wav
 * Description   : PCM音频格式转成WAV格式
 * Input         : const S8* pcm_path   
                const S8* wave_path  
                U16 channels         
                U32 rate             
                U16 bits             
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180405
 *   Author      : Zengchao
 *   Modification: Created function

*****************************************************************************/
int 
pcm_to_wav(const S8* pcm_path,const S8* wave_path,
				U16 channels,U32 rate,U16 bits)
{
	FILE *pcm_fp,*wave_fp;
	wave_header_t wav_header;
	fmt_chunk_t fmt_chunk;
	data_chunk_t data_chunk;
	long pcm_file_len,read_len;

	pcm_fp = fopen(pcm_path,"rb");
	if(NULL == pcm_fp)
	{
	  	printf("Open pcm file error.\n");
        return -1;	
	}

	wave_fp = fopen(wave_path,"wb+");
	if(NULL == wave_fp)
	{
	  	printf("Create wav file error.\n");
        return -1;	
	}

	fseek(pcm_fp, 0, SEEK_END);
	pcm_file_len = ftell(pcm_fp);
	fseek(pcm_fp, 0, SEEK_SET);

	/* WAVE_HEADER */
    memcpy(wav_header.id , "RIFF", strlen("RIFF"));
    memcpy(wav_header.format, "WAVE", strlen("WAVE"));
    fseek(wave_fp, sizeof(wave_header_t), SEEK_CUR);

	/* WAVE_FMT */
    memcpy(fmt_chunk.subid, "fmt ", strlen("fmt "));
	fmt_chunk.subsize = 16;
	fmt_chunk.audio_fmt = 1;
	fmt_chunk.sample_rate = rate;
	fmt_chunk.num_chn = channels;
	fmt_chunk.bits_per_sample = bits;
	fmt_chunk.byte_rate = rate*channels*bits/8;
	fmt_chunk.block_align = channels*bits/8;
    fwrite(&fmt_chunk, sizeof(fmt_chunk_t), 1,wave_fp);
	
    /* WAVE_DATA */
    memcpy(data_chunk.subid, "data", strlen("data"));
    data_chunk.subsize = pcm_file_len;
	fwrite(&data_chunk,sizeof(data_chunk_t),1,wave_fp);

	char buf[1024];
	
	while((read_len = fread(buf, 1, sizeof(buf), pcm_fp)) != 0) 
		fwrite(buf, 1, read_len, wave_fp);

	fseek(wave_fp, 0, SEEK_SET);
	wav_header.size = 4 + (8 + fmt_chunk.subsize) + ( 4 + data_chunk.subsize);
	fwrite(&wav_header,sizeof(wav_header),1,wave_fp);

	fclose(pcm_fp);
	fclose(wave_fp);

	return 0;
}




/*****************************************************************************
 * Function      : input
 * Description   : MAD库需要的INPUT函数
 * Input         : void *user_data            
                struct mad_stream *stream  
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180402
 *   Author      : ZengChao
 *   Modification: Created function

*****************************************************************************/
enum mad_flow 
input(void *user_data, struct mad_stream *stream)
{
	mad_user_data_t *data = (mad_user_data_t *)user_data;

	if(0 == data->audio_len)
		return MAD_FLOW_STOP;
	
	/*
		把原始的未解码的 MPEG 数据和 mad_stream 数据结构关联，
		以便使用 mad_frame_decode()来解码 MPEG 帧数据
    */
	mad_stream_buffer(stream, data->audio_data, data->audio_len);     

	data->audio_len = 0;
	
    return MAD_FLOW_CONTINUE;   
}

/*****************************************************************************
 * Function      : output
 * Description   : MAD库需要的OUTPUT函数
 * Input         : void *user_data                  
                struct mad_header const *header  
                struct mad_pcm *pcm              
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180402
 *   Author      : ZengChao
 *   Modification: Created function

*****************************************************************************/
enum mad_flow 
output(void *user_data, struct mad_header const *header,
		struct mad_pcm *pcm)
{
	int i,ret;
	mad_user_data_t *data = (mad_user_data_t *)user_data;
	mad_fixed_t const *left_ch, *right_ch;

	/*
		mad_pcm结构体中的samples成员最大字节数为6912字节
	*/
	unsigned char output_buffer[6912];
	unsigned int  ouput_bytes = 0;

	memset(output_buffer,0,sizeof(output_buffer));

	left_ch = pcm->samples[0];
	right_ch = pcm->samples[1];
	mp3_channels = pcm->channels;
	mp3_sample_rate = pcm->samplerate;
	
	for(i=0;i<pcm->length;i++)
	{
		S32 sample = scale(left_ch[i]);
		output_buffer[ouput_bytes++] = (U8)sample;
		output_buffer[ouput_bytes++] = (U8)(sample>>8);

		if(2 == pcm->channels)
		{
			S32 sample = scale(right_ch[i]);
			output_buffer[ouput_bytes++] = (U8)sample;
			output_buffer[ouput_bytes++] = (U8)(sample>>8);
		}
	}

	if(ouput_bytes != 
		fwrite(output_buffer,sizeof(U8),ouput_bytes,data->out_fd))
	{
		printf("fwrite failed:%s\n",strerror(errno));
	}
       	
}

static
enum mad_flow error(void *data,struct mad_stream *stream,struct mad_frame *frame)
{
//	struct buffer *buffer = data;

//	fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %u\n",
//	  stream->error, mad_stream_errorstr(stream),
//	  stream->this_frame - buffer->start);

	/* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

	return MAD_FLOW_CONTINUE;
}



int
decode(unsigned char const * audio_data, unsigned long audio_len,FILE * out_fd)
{
	int ret;
	mad_user_data_t user_data;
	struct mad_decoder decoder;

	/* initialize our private message structure */
	user_data.out_fd = out_fd;
	user_data.audio_len = audio_len;
	user_data.audio_data = audio_data;

	/* configure input, output, and error functions */
	mad_decoder_init(&decoder, &user_data,
			  input, 0 /* header */, 0 /* filter */, output,
			  error, 0 /* message */);

	/* start decoding */
	ret = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

	/* release the decoder */
	mad_decoder_finish(&decoder);

	return 0;
}


/*****************************************************************************
 * Function      : mp3_to_wav
 * Description   : MP3格式转成WAV格式
 * Input         : const S8* mp3_path   
                const S8* wave_path  
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180405
 *   Author      : Zengchao
 *   Modification: Created function

*****************************************************************************/
int 
mp3_to_wav(const S8* mp3_path,const S8* wave_path)
{
	int ret;

	char pcm_path[256] = "temp.pcm";

	int mp3_fd = open(mp3_path,O_RDWR);
	if(mp3_fd<0)
	{
		LOG_ERROR((BSL_META("open [%s] failed:%s\n"),
					mp3_path,strerror(errno)));
		return -1;
	}

	FILE *wav_fp = fopen(pcm_path,"w+");
	if(wav_fp<0)
	{
		LOG_ERROR((BSL_META("open [%s] failed:%s\n"),
					wave_path,strerror(errno)));
		close(mp3_fd);
		return -1;
	}

	struct stat stat; 

	ret = fstat(mp3_fd,&stat);
	if(ret<0)
	{
		LOG_ERROR((BSL_META("fstat failed:%s\n"),strerror(errno)));
		close(mp3_fd);
		fclose(wav_fp);
		return -1;
	}
	
	if(0 == stat.st_size)
	{
		LOG_ERROR((BSL_META("file has no content\n")));
		close(mp3_fd);
		fclose(wav_fp);
		return -1;
	}

	void *in_ptr = mmap(0,stat.st_size,PROT_READ,MAP_SHARED,mp3_fd,0);
	if(MAP_FAILED == in_ptr)
	{
		LOG_ERROR((BSL_META("mmap failed:%s\n"),strerror(errno)));
		close(mp3_fd);
		fclose(wav_fp);
		return -1;
	}

	decode(in_ptr,stat.st_size,wav_fp);
	fclose(wav_fp);	
	pcm_to_wav(pcm_path,wave_path,mp3_channels,mp3_sample_rate,16);
	remove(pcm_path);
	
	ret = munmap(in_ptr, stat.st_size);
	if(ret<0)
	{
		LOG_ERROR((BSL_META("munmap failed:%s\n"),strerror(errno)));
	}
	
	close(mp3_fd);
	return 0;
}

/*****************************************************************************
 * Function      : play_audio_thread
 * Description   : 播放音频线程
 * Input         : void *arg  
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180405
 *   Author      : Zengchao
 *   Modification: Created function

*****************************************************************************/
void 
play_audio_thread(void *arg)
{
	audio_type_t type;
	char audio_name[256];
	char temp_wav_name[256] = "temp_auodio.wav";
	
	for(;;)
	{
		pthread_mutex_lock(&play_lock);
		while(NULL == play_list_head)
			pthread_cond_wait(&play_ready,&play_lock);

		play_list_t *entry = play_list_head;
		memset(audio_name,0,sizeof(audio_name));
		strncpy(audio_name,entry->audio_name,sizeof(audio_name));
		type = entry->audio_type;
		
		play_list_head = entry->next;
		free(entry);
		pthread_mutex_unlock(&play_lock);

		switch (type)
		{
			case AUDIO_PCM:
				pcm_to_wav(audio_name,temp_wav_name,2,16000,16);
				sal_sys_aplay(temp_wav_name);
				break;
			case AUDIO_MP3:
				mp3_to_wav(audio_name,temp_wav_name);
				sal_sys_aplay(temp_wav_name);
				break;
			case AUDIO_WAV:	
				sal_sys_aplay(audio_name);		
				break;
		}
	}
}

