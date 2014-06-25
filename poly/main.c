/*
	PVR VQ TEXTURE EXAMPLE
	- Uses textured Triangle strip
	
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
	pvr_vertex_t v;
	Texture spr;
	pvr_poly_cxt_t p_cxt;
	pvr_poly_hdr_t p_hdr;
	
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
	
		pvr_poly_cxt_txr(&p_cxt,PVR_LIST_TR_POLY,spr.fmt,spr.w,spr.h,spr.txt,PVR_FILTER_NONE);
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



