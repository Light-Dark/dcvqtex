/*
	PVR VQ TEXTURE EXAMPLE
	- Uses textured sprite
	
	By: Liam "Dragmire" Ewasko
	2014
*/


#include <kos.h>



typedef unsigned int Uint32;
typedef unsigned char Uint8, uint8;


/*
	Structure for holding texture information
	w - texture width
	h - texture height
	fmt - PVR texture format flags
	txt - PVR video memory pointer to texture data
	Allocated - 1 if texture is allocated, 0 if not
*/
typedef struct {
	uint32 w,h;
	uint32 fmt;
	pvr_ptr_t txt;
	uint8 Allocated;
}Texture;

/*
	Initialize PVR to defaults and sndoggvorbis
*/
void Init(){
	vid_set_mode(DM_640x480,PM_RGB565);
	vid_border_color(0,255,0);
	pvr_init_defaults();
	
	snd_stream_init();
	sndoggvorbis_init();
	
}

/*
	Load in a VQ compressed texture with twiddling, format ARGB4444
	fn - File pointer
	text - texture structure to load into
*/
void Load_VQTexture(const char* fn,Texture* text) {

	kos_img_t img;
	if(text->Allocated == 1){
		printf("TEXTURE ALREADY ALLOCATED!\n");
		return;
	}
	if(kmg_to_img(fn,&img)){
		return;
	}
	text->Allocated = 1;
	text->w = img.w;
	text->h = img.h;
	text->fmt = PVR_TXRFMT_ARGB4444 | PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_TWIDDLED;
	text->txt = pvr_mem_malloc(img.byte_count);
	
    pvr_txr_load_kimg(&img, text->txt, 0);
    kos_img_free(&img, 0);
}
/*
	Frees an allocated texture
*/
void DeleteTexture(Texture *tex)
{
	if(tex->Allocated == 0) {
		printf("Texture isn't allocated!\n");
		return;
	}
	pvr_mem_free(tex->txt);
	tex->Allocated = 0;
}

extern uint8 romdisk[];

KOS_INIT_FLAGS(INIT_DEFAULT);
KOS_INIT_ROMDISK(romdisk);


int main(int argc,char** argv){
	pvr_sprite_txr_t v;
	Texture spr;
	pvr_sprite_cxt_t s_cxt;
	pvr_sprite_hdr_t s_hdr;
	
	Init();
	
	Load_VQTexture("/rd/billy.kmg",&spr);
	
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
	
		pvr_sprite_cxt_txr(&s_cxt,PVR_LIST_TR_POLY,spr.fmt,spr.w,spr.h,spr.txt,PVR_FILTER_BILINEAR);
		pvr_sprite_compile(&s_hdr,&s_cxt);
		s_hdr.argb = 0xffffffff;
		pvr_prim(&s_hdr,sizeof(s_hdr)); // submit header
		v.flags = PVR_CMD_VERTEX_EOL;
		
		
		v.ax = 0.0;
		v.ay = 480.0;
		v.az = 1.0;
		v.auv = PVR_PACK_16BIT_UV(0.0,1.0);
		
		v.bx = 0.0;
		v.by = 0.0;
		v.bz = 1.0;
		v.buv = PVR_PACK_16BIT_UV(0.0,0.0);
		
		v.cx = 640.0;
		v.cy = 0.0;
		v.cz = 1.0;
		v.cuv = PVR_PACK_16BIT_UV(1.0,0.0);
		
		v.dx = 640.0;
		v.dy = 480.0;
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



