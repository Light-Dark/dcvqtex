/*
	PVR VQ TEXTURE EXAMPLE USING Tvspelsfreak's texconv tool
	- Uses textured Triangle strip and the Texconv tool by Tvspelsfreak
	
	By: Liam "Dragmire" Ewasko
	2014
	
	Credit to Tvspelsfreak for texconv tool and texture file format
*/


#include <kos.h>
typedef unsigned int Uint32;
typedef unsigned char Uint8, uint8;

/*
	Tvspelsfreak's texture file header
*/
typedef struct {
	char	id[4];	// 'DTEX'
	short	width;
	short	height;
	Uint32		type;
	int		size;
} header_t;


typedef struct {
	char	id[4];	// 'DPAL'
	int		numcolors;
} pal_header_t;

/*
	My texture structure
*/
typedef struct {
	uint32 w,h;
	uint32 fmt;
	pvr_ptr_t txt;
}Texture;

void Init(){
	vid_set_mode(DM_640x480,PM_RGB565);
	vid_border_color(0,255,0);
	pvr_init_defaults();
	
	//pvr_set_pal_format(PVR_PAL_ARGB8888);
	
	snd_stream_init();
	sndoggvorbis_init();
	
}

void Load_VQTexture(const char* fn, Texture* t){
	FILE* fp;
	header_t  hdr;
	fp = fopen(fn,"r");
	
	fread(&hdr,sizeof(hdr),1,fp);	// read in the header
	
	t->w = hdr.width;
	t->h = hdr.height;
	
	t->fmt = hdr.type;
	//Allocated texture memory
	t->txt = pvr_mem_malloc(hdr.size);
	//Temporary ram storage of texture
	void* temp = malloc(hdr.size);
	// Load texture into ram
	fread(temp,hdr.size,1,fp);
	// DMA into VRAM
	pvr_txr_load(temp,t->txt,hdr.size);
	//Free RAM
	free(temp);
	temp = NULL;
	fclose(fp);
	
	
	if(t->fmt & PVR_TXRFMT_PAL4BPP != 0){
		printf("Loading 4BPP palette\n");
		char *temp = fn;
		pal_header_t phdr;
		strcat(temp,".pal");
		fp = fopen(temp,"r");
		fread(&phdr,sizeof(pal_header_t),1,fp);
		Uint32* tempp;
		tempp = malloc(phdr.numcolors*16*4);
		fread((void*)tempp,16*4*phdr.numcolors,1,fp);
		fclose(fp);
		int i;
		for(i = 0;i < 16;i++){
			pvr_set_pal_entry(i,tempp[i]);
		}
		
		free(tempp);
		temp = NULL;
	}
}

void DeleteTexture(Texture *tex)
{
	pvr_mem_free(tex->txt);
}

extern uint8 romdisk[];

KOS_INIT_FLAGS(INIT_DEFAULT);
KOS_INIT_ROMDISK(romdisk);

int main(int argc,char **argv){
	pvr_vertex_t v;
	Texture spr;
	pvr_poly_cxt_t p_cxt;
	pvr_poly_hdr_t p_hdr;
	
	Init();
	
	Load_VQTexture("/rd/billy.raw",&spr);
	
	sndoggvorbis_start("/rd/billy.ogg",-1);
	int q = 0;
	while(q == 0){
		vid_border_color(255,0,0);
		pvr_wait_ready();
		vid_border_color(0,255,0);
		pvr_scene_begin();
		
		pvr_list_begin(PVR_LIST_OP_POLY);
		
		pvr_list_finish();
		
		pvr_list_begin(PVR_LIST_TR_POLY);
	
		pvr_poly_cxt_txr(&p_cxt,PVR_LIST_TR_POLY,spr.fmt,spr.w,spr.h,spr.txt,PVR_FILTER_BILINEAR);

		pvr_poly_compile(&p_hdr,&p_cxt);
		pvr_prim(&p_hdr,sizeof(p_hdr)); // submit header
		
		v.x = 0.0;
		v.y = 0.0;
		v.z = 1.0;
		v.u = 0.0;
		v.v = 0.0;
		v.argb = 0xffffffff;
		v.oargb = 0;
		v.flags = PVR_CMD_VERTEX;
		pvr_prim(&v,sizeof(v));
		
		
		v.x = 640.0;
		v.y = 0.0;
		v.u = 1.0;
		v.v = 0.0;
		pvr_prim(&v,sizeof(v));
		
		v.x = 0.0;
		v.y = 480.0;
		v.u = 0.0;
		v.v = 1.0;
		pvr_prim(&v,sizeof(v));
		
		v.x = 640.0;
		v.y = 480.0;
		v.u = 1.0;
		v.v = 1.0;
		v.flags = PVR_CMD_VERTEX_EOL;
		pvr_prim(&v,sizeof(v));
		pvr_list_finish();
		pvr_scene_finish();
		vid_border_color(0,0,255);
		
		
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st);
		
			if(st->buttons & CONT_START)
				q = 1;
		
		MAPLE_FOREACH_END();
		
	}
	
	DeleteTexture(&spr);
	
	sndoggvorbis_stop();
	pvr_shutdown();
	sndoggvorbis_shutdown();
	return 0;
}