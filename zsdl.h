#ifndef _ZSDL_H_
#define _ZSDL_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
//#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <string>

#include "zsdl_opengl.h"

using namespace std;

enum sound_setting
{
	SOUND_0, SOUND_25, SOUND_50, SOUND_75, SOUND_100, MAX_SOUND_SETTINGS
};

const string sound_setting_string[MAX_SOUND_SETTINGS] = 
{
	"0%", "25%", "50%", "75%", "100%"
};

SDL_Surface *ZSDL_ConvertImage(SDL_Surface *src);
SDL_Surface *ZSDL_IMG_Load(const string& filename);
SDL_Surface *IMG_Load_Error(const string& filename);
Mix_Music *MUS_Load_Error(const string& filename);
Mix_Chunk *MIX_Load_Error(const string& filename);
SDL_Surface *CopyImage(SDL_Surface *original);
SDL_Surface *CopyImageShifted(SDL_Surface *original, int x, int y);
void put32pixel(SDL_Surface *surface, int x, int y, SDL_Color color);
SDL_Color get32pixel(SDL_Surface *surface, int x, int y);
int ZSDL_PlayMusic(Mix_Music *music, int eh);
int ZMix_PlayChannel(int ch, Mix_Chunk *wav, int repeat);
void ZSDL_SetMusicOn(bool iton);
void draw_box(/*SDL_Surface *surface,*/ SDL_Rect dim, SDL_Color color, int max_x, int max_y);
void draw_selection_box(/*SDL_Surface *surface,*/ SDL_Rect dim, SDL_Color color, int max_x, int max_y);
int AngleFromLoc(float dx, float dy);
SDL_Surface *ZSDL_NewSurface(int w, int h);
void ZSDL_ModifyBlack(SDL_Surface *surface);
void ZSDL_BlitSurface(SDL_Surface *src, int fx, int fy, int fw, int fh, SDL_Surface *dest, int x, int y);
void ZSDL_BlitHitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect, bool render_hit = true);
void ZSDL_BlitTileSurface(SDL_Surface *src, int fx, int fy, SDL_Surface *dest, int x, int y);
void ZSDL_FreeSurface(SDL_Surface *&surface);
void ZSDL_Quit();

#endif
