/*
	PVR VQ TEXTURE EXAMPLE USING Tvspelsfreak's texconv tool
	- Uses textured Triangle strip
	
	By: Liam "Dragmire" Ewasko
	2014
	
	Credit to Tvspelsfreak for texconv tool and texture file format
*/
#define TXR_NONPALETTED 0 
#define TXR_PALETTED 1



#include <kos.h>
typedef unsigned int Uint32;
typedef unsigned char Uint8, uint8;
/*
	First Half of the 1024 will go to 32 4-bit palettes
*/
static Uint32 Pindex4 = 0;
/*
	Second half will go to 2 8-bit palettes
*/
static Uint32 Pindex8 = 0;
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
	Uint8 palette;
}Texture;

void Init(){
	vid_set_mode(DM_640x480,PM_RGB565);
	vid_border_color(0,255,0);
	pvr_init_defaults();
	
	pvr_set_pal_format(PVR_PAL_ARGB8888);
	
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
	
	/*
		Palette loading and management
	*/
	if( ((t->fmt >> 27) & 7) > 4 ) {
		if(t->fmt & PVR_TXRFMT_PAL4BPP){
			// Append palette suffix to filepath
			char pf[64];
			strcpy(pf,fn);
			strcat(pf,".pal");
			fp = fopen(pf,"r");
			pal_header_t phdr;
			//read in the 8-byte header
			fread(&phdr,sizeof(pal_header_t),1,fp);
			//setup buffer
			void *palette = malloc(phdr.numcolors*4);
			Uint32 i;
			//Make entries readable to PVR
			Uint32* packed = (Uint32*)palette;
			//Load entries in to buffer
			fread(packed,phdr.numcolors*4,1,fp);
			//Load palette into correct location
			for(i = Pindex4*16; i < (Pindex4*16) + phdr.numcolors*4;i++){
				pvr_set_pal_entry(i,packed[i]);
			}
			//Set palette #
			t->palette = Pindex4;
		
			t->fmt |=  PVR_TXRFMT_4BPP_PAL(Pindex4);
		
		//Increase palettte index to prevent overwrite
			Pindex4++;
		
			if(Pindex4 == 32){
				Pindex4 = 0; // overwrite
			}

			packed = NULL;
			free(palette);
			fclose(fp);
		} else if(t->fmt & PVR_TXRFMT_PAL8BPP){
			char pf[64];
			strcpy(pf,fn);
			strcat(pf,".pal");
			fp = fopen(pf,"r");
			pal_header_t phdr;
			fread(&phdr,sizeof(pal_header_t),1,fp);
			void * palette = malloc(phdr.numcolors*4);
			Uint32 i;
			Uint32* packed = (Uint32*)palette;
			fread(packed,phdr.numcolors*4,1,fp);
			for(i = (512 + Pindex8*256); i < (Pindex8*256 + 512) + phdr.numcolors*4;i++){
				pvr_set_pal_entry(i,packed[i]);
			}
		
			t->palette = Pindex8 | 0x80;
			t->fmt |=  PVR_TXRFMT_8BPP_PAL(Pindex8+2);
			Pindex8++;
			if(Pindex8 == 2){
				Pindex8 = 0;
			}
		
			packed = NULL;
			free(palette);
			fclose(fp);
		}
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
	Texture spr2;
	pvr_poly_cxt_t p_cxt;
	pvr_poly_hdr_t p_hdr;
	
	Init();
	
	Load_VQTexture("/rd/billy.raw",&spr);
	Load_VQTexture("/rd/billy2.raw",&spr2);
	
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
		
		
		pvr_poly_cxt_txr(&p_cxt,PVR_LIST_TR_POLY,spr2.fmt,spr2.w,spr2.h,spr2.txt,PVR_FILTER_NONE);

		pvr_poly_compile(&p_hdr,&p_cxt);
		pvr_prim(&p_hdr,sizeof(p_hdr)); // submit header
		
		v.x = 0.0;
		v.y = 480/2.0;
		v.z = 1.0;
		v.u = 0.0;
		v.v = 0.0;
		v.argb = 0xffffffff;
		v.oargb = 0;
		v.flags = PVR_CMD_VERTEX;
		pvr_prim(&v,sizeof(v));
		
		
		v.x = 640/2.0;
		v.y = 480/2.0;
		v.u = 1.0;
		v.v = 0.0;
		pvr_prim(&v,sizeof(v));
		
		v.x = 0.0;
		v.y = 480.0;
		v.u = 0.0;
		v.v = 1.0;
		pvr_prim(&v,sizeof(v));
		
		v.x = 640/2.0;
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
	DeleteTexture(&spr2);
	sndoggvorbis_stop();
	pvr_shutdown();
	sndoggvorbis_shutdown();
	return 0;
}