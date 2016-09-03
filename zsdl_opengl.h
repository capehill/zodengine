#ifndef _ZSDL_OPENGL_H_
#define _ZSDL_OPENGL_H_

#ifdef _WIN32 //if windows
#include <windows.h>		//win for Sleep(1000)
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <string>

using namespace std;

class ZSDL_Surface
{
public:
	ZSDL_Surface();
	~ZSDL_Surface();

	static void SetRenderer(SDL_Renderer *renderer_);
	static void SetScreenDimensions(int w_, int h_);
	static void ZSDL_FillRect(SDL_Rect *dstrect, char r, char g, char b, ZSDL_Surface *dst = NULL);
	static void SetMapPlace(int x, int y);
	static void SetHasHud(bool has_hud_);
	static void GetScreenDimensions(int &w_, int &h_) { w_ = screen_w; h_ = screen_h; }

	static int GetMapBlitInfo(SDL_Surface *src, int x, int y, SDL_Rect &from_rect, SDL_Rect &to_rect);

	void Unload();
	void LoadBaseImage(string filename);
	void LoadBaseImage(SDL_Surface *sdl_surface_, bool delete_surface = true);
	void LoadNewSurface(int w, int h);
	void UseDisplayFormat();
	void MakeAlphable();
	SDL_Surface *GetBaseSurface();

	void SetSize(float size_);
	void SetAngle(float angle_);
	void SetAlpha(char alpha_);

	void RenderSurface(int x, int y, bool render_hit = false, bool about_center = false);
	void RenderSurfaceHorzRepeat(int x, int y, int w_total, bool render_hit = false);
	void RenderSurfaceVertRepeat(int x, int y, int h_total, bool render_hit = false);
	void RenderSurfaceAreaRepeat(int x, int y, int w, int h, bool render_hit = false);
	void BlitSurface(SDL_Rect *srcrect, SDL_Rect *dstrect, ZSDL_Surface *dst = NULL);
	void BlitSurface(int fx, int fy, int fw, int fh, ZSDL_Surface *dst, int x, int y);
	void BlitSurface(ZSDL_Surface *dst, int x, int y);
	void BlitHitSurface(SDL_Rect *srcrect, SDL_Rect *dstrect, ZSDL_Surface *dst = NULL, bool render_hit = false);

	void BlitOnToMe(SDL_Rect *srcrect, SDL_Rect *dstrect, SDL_Surface *src);
	void FillRectOnToMe(SDL_Rect *dstrect, char r, char g, char b);

	bool WillRenderOnScreen(int x, int y, bool about_center);

	//operator overloads
	ZSDL_Surface& operator=(const ZSDL_Surface &rhs);
	ZSDL_Surface& operator=(SDL_Surface *rhs);
private:

	static SDL_Renderer *renderer;

	static int screen_w;
	static int screen_h;
	static int map_place_x;
	static int map_place_y;
	static bool has_hud;

	bool LoadTexture();

	string image_filename;
	SDL_Surface *sdl_surface;

	SDL_Texture *texture;

	float size, angle;
	char alpha;
};

inline void ZSDL_FillRect(SDL_Rect *dstrect, char r, char g, char b, ZSDL_Surface *dst = NULL)
	{ ZSDL_Surface::ZSDL_FillRect(dstrect, r, g, b, dst); }

#endif
