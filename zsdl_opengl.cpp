#include "zsdl_opengl.h"
#include "zsdl.h"
#include "constants.h"
#include "common.h"

#ifdef __amigaos4__
// hack for libsmpeg2 linkage. needs a new compiler?
void operator delete(void* p, unsigned int s) {
	// TODO FIXME printf("delete called %p %d\n", p, s);
	::operator delete(p);
}
#warning "HACK"
#endif

using namespace COMMON;

//SDL_Surface *ZSDL_Surface::screen = NULL;
SDL_Renderer *ZSDL_Surface::renderer = NULL;

int ZSDL_Surface::screen_w = 0;
int ZSDL_Surface::screen_h = 0;
int ZSDL_Surface::map_place_x = 0;
int ZSDL_Surface::map_place_y = 0;
bool ZSDL_Surface::has_hud = true;

ZSDL_Surface::ZSDL_Surface()
{
	sdl_surface = NULL;
	texture = NULL;

	size = 1.0f;
	angle = 0.0f;
	alpha = 255;
}

ZSDL_Surface::~ZSDL_Surface()
{
	Unload();
}

ZSDL_Surface& ZSDL_Surface::operator=(const ZSDL_Surface &rhs)
{
	if(this == &rhs) return *this;

	LoadBaseImage(rhs.sdl_surface, false);

	return *this;
}

ZSDL_Surface& ZSDL_Surface::operator=(SDL_Surface *rhs)
{
	LoadBaseImage(rhs, false);

	return *this;
}

void ZSDL_Surface::Unload()
{
	if (texture) SDL_DestroyTexture(texture);

	if (sdl_surface) SDL_FreeSurface(sdl_surface);

	texture = NULL;
	
	sdl_surface = NULL;
}

SDL_Surface *ZSDL_Surface::GetBaseSurface()
{
	return sdl_surface;
}

void ZSDL_Surface::LoadBaseImage(string filename)
{
	//set this for later debugging purposes
	// hack image_filename = filename;
//printf("%s: %s\n", __FUNCTION__, filename.c_str());
	SDL_Surface *surface = IMG_Load(filename.c_str());

	if (surface == NULL)
	{
		printf("%s: %s\n", __FUNCTION__, filename.c_str());	
	}
	
	LoadBaseImage(surface);
}

void ZSDL_Surface::LoadNewSurface(int w, int h)
{
	SDL_Surface *new_surface;

	//new_surface = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCALPHA, w, h, 32, 0xFF000000, 0x0000FF00, 0x00FF0000, 0x000000FF);
	new_surface = SDL_CreateRGBSurface(0 /*SDL_SWSURFACE | SDL_SRCALPHA*/, w, h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	LoadBaseImage(new_surface);

	SDL_Rect the_box;
	the_box.x = 0;
	the_box.y = 0;
	the_box.w = w;
	the_box.h = h;
	ZSDL_FillRect(&the_box, 0, 0, 0, this);
}

void ZSDL_Surface::LoadBaseImage(SDL_Surface *sdl_surface_, bool delete_surface)
{
	Unload();

	sdl_surface = sdl_surface_;

	if(!sdl_surface)
	{
		if(image_filename.size()) printf("could not load:%s\n", image_filename.c_str()); 
		return;
	}

	//convert to a guaranteed good format
	SDL_Surface *new_ret;
	//new_ret = SDL_DisplayFormatAlpha(sdl_surface);
	new_ret = SDL_ConvertSurfaceFormat(sdl_surface, SDL_PIXELFORMAT_ARGB8888, 0);
		
	if (!new_ret) printf("%s NULL\n", __FUNCTION__);
		
	if(delete_surface) SDL_FreeSurface( sdl_surface );
	sdl_surface = new_ret;

	//checks
	if ( (sdl_surface->w & (sdl_surface->w - 1)) != 0 )
		;//printf("warning: %s's width is not a power of 2\n", image_filename.c_str());
	
	// Also check if the height is a power of 2
	if ( (sdl_surface->h & (sdl_surface->h - 1)) != 0 )
		;//printf("warning: %s's height is not a power of 2\n", image_filename.c_str());
}

void ZSDL_Surface::UseDisplayFormat()
{
	return; //TODO
	
	if(!sdl_surface) return;

	//this is not needed in opengl (?)
	//if(use_opengl) return;

	SDL_Surface *new_ret;
	//new_ret = SDL_DisplayFormat(sdl_surface);
	new_ret = SDL_ConvertSurfaceFormat(sdl_surface, SDL_PIXELFORMAT_ARGB8888, 0);

	if (!new_ret) printf("NULL\n");

	SDL_FreeSurface( sdl_surface );
	sdl_surface = new_ret;

	SetAlpha(alpha);
}

void ZSDL_Surface::MakeAlphable()
{
	return; //TODO
	
	if(!sdl_surface) return;

	//this is not needed in opengl (?)
	//if(use_opengl) return;

	ZSDL_ModifyBlack(sdl_surface);
	UseDisplayFormat();
	//SDL_SetColorKey(sdl_surface, SDL_SRCCOLORKEY, 0x000000);
	SDL_SetColorKey(sdl_surface, SDL_TRUE, 0x000000);
}

bool ZSDL_Surface::LoadTexture()
{
	if (!sdl_surface)
	{
		printf("LoadTexture::sdl_surface for %s not loaded\n", image_filename.c_str());
		return false;
	}

	if (texture) {
		SDL_DestroyTexture(texture);
		texture = NULL;
	}

	texture = SDL_CreateTextureFromSurface(renderer, sdl_surface);

	if (texture) {
		return true;
	} else {
		printf("LoadTexture: failed to create texture for %s\n", image_filename.c_str());
		return false;
	}
}

void ZSDL_Surface::SetRenderer(SDL_Renderer *renderer_)
{
	printf("Renderer %p set\n", renderer_);
	renderer = renderer_;
}


void ZSDL_Surface::SetScreenDimensions(int w_, int h_)
{
	screen_w = w_;
	screen_h = h_;
}

void ZSDL_Surface::SetMapPlace(int x, int y)
{
	map_place_x = x;
	map_place_y = y;
}

void ZSDL_Surface::SetHasHud(bool has_hud_)
{
	has_hud = has_hud_;
}

void ZSDL_Surface::SetSize(float size_)
{
	size = size_;
}

void ZSDL_Surface::SetAngle(float angle_)
{
	angle = angle_;
}

void ZSDL_Surface::SetAlpha(char alpha_)
{
	alpha = alpha_;

    return;
	
	//if(!use_opengl)
	{
		if(sdl_surface) SDL_SetSurfaceAlphaMod(sdl_surface, /*SDL_RLEACCEL | SDL_SRCALPHA, */ alpha);
		//if(sdl_rotozoom) SDL_SetAlpha(sdl_rotozoom, SDL_RLEACCEL | SDL_SRCALPHA, alpha);
	}

	//SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
}

void ZSDL_Surface::FillRectOnToMe(SDL_Rect *dstrect, char r, char g, char b)
{
	if(!sdl_surface) return;

	SDL_FillRect(sdl_surface, dstrect, SDL_MapRGB(sdl_surface->format, r,g,b));

	if (texture)
	{
		SDL_DestroyTexture(texture);
		texture = NULL;
	}
}

void ZSDL_Surface::BlitOnToMe(SDL_Rect *srcrect, SDL_Rect *dstrect, SDL_Surface *src)
{
	if(!sdl_surface) return;

	SDL_BlitSurface(src, srcrect, sdl_surface, dstrect);

	if (texture)
	{
		SDL_DestroyTexture(texture);
		texture = NULL;
	}
}

//ZSDL_Surface has made itself into an engine it seems...
void ZSDL_Surface::ZSDL_FillRect(SDL_Rect *dstrect, char r, char g, char b, ZSDL_Surface *dst)
{
	if(dst)
	{
		dst->FillRectOnToMe(dstrect, r, g, b);
		return;
	}

	//if(screen) SDL_FillRect(screen, dstrect, SDL_MapRGB(screen->format, r,g,b));
		
	if (renderer)
	{
		SDL_SetRenderDrawColor(renderer, r, g, b, 255);

		SDL_RenderFillRect(renderer, dstrect);
		/*
		if (dstrect)
		{
			SDL_RenderFillRect(renderer, dstrect);
		}
		else
		{
		    SDL_RenderClear(renderer);
		}*/
	}
}

void ZSDL_Surface::RenderSurface(int x, int y, bool render_hit, bool about_center)
{
	if(0)
	{
		//if(!WillRenderOnScreen(x, y, about_center)) return;

		if(render_hit)
		{
			SDL_Rect to_rect;

			to_rect.x = x + map_place_x;
			to_rect.y = y + map_place_y;

			BlitHitSurface(NULL, &to_rect, NULL, true);
			return;
		}

#ifndef DISABLE_OPENGL
		if(!gl_texture_loaded && !LoadGLtexture()) return;

		glPushMatrix();

		glTranslatef(x, y, 0.0f);

		//scale
		glScalef( size, size, 1.0 );

		const int actual_width = texture_width * sdl_surface->w;
		const int actual_height = texture_height * sdl_surface->h;			
		
		//rotate about center
		if(!about_center)
		glTranslatef((actual_width /*sdl_surface->w*/ >> 1), (actual_height /*sdl_surface->h*/ >> 1), 0.0f);
		glRotatef(-angle, 0.0f, 0.0f, 1.0f);
		glTranslatef(-(actual_width /*sdl_surface->w*/ >> 1), -(actual_height /*sdl_surface->h*/ >> 1), 0.0f);		  

		//we are at the center now, so don't translate back
		//if(!about_center) glTranslatef(-(sdl_surface->w >> 1), -(sdl_surface->h >> 1), 0.0f);

		//alpha
		glColor4ub(255,255,255,alpha);

		glBindTexture(GL_TEXTURE_2D, gl_texture);

		glBegin(GL_QUADS);
		
		glTexCoord2f(0.0f, texture_height); glVertex3f( 0.0f, actual_height /*sdl_surface->h*/,  0.0f);	// Bottom Left Of The Texture and Quad
		glTexCoord2f(texture_width, texture_height); glVertex3f( actual_width /*sdl_surface->w*/, actual_height /*sdl_surface->h*/,  0.0f);	// Bottom Right Of The Texture and Quad
		glTexCoord2f(texture_width, 0.0f); glVertex3f( actual_width /*sdl_surface->w*/, 0.0f,  0.0f);	// Top Right Of The Texture and Quad
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.0f, 0.0f,  0.0f);	// Top Left Of The Texture and Quad

		glEnd();

		glPopMatrix();
#endif
	}
	else
	{
		SDL_Surface *render_surface = sdl_surface;

		if(!render_surface) return;

		if(about_center)
		{
			x -= render_surface->w >> 1;
			y -= render_surface->h >> 1;
		}

		SDL_Rect from_rect, to_rect;

		if(GetMapBlitInfo(render_surface, x, y, from_rect, to_rect))
		{
			to_rect.x += map_place_x;
			to_rect.y += map_place_y;

			if(render_hit)	
				BlitHitSurface(&from_rect, &to_rect, NULL, true);
			else
			{
				if (!texture && !LoadTexture()) return;

				//SDL_BlitSurface(render_surface, &from_rect, screen, &to_rect);
				if (!isz(angle) || !is1(size))
				{
					SDL_RenderCopyEx(renderer, texture, &from_rect, &to_rect, angle, NULL, SDL_FLIP_NONE);
				}
				else
				{
					SDL_RenderCopy(renderer, texture, &from_rect, &to_rect);
				}
    			//printf("%d size %f\n", __LINE__, size);
			}
		}
	}
}

void ZSDL_Surface::RenderSurfaceHorzRepeat(int x, int y, int w_total, bool render_hit)
{
	SDL_Rect from_rect, to_rect;
	int fw, fh;

	if(!sdl_surface) return;

	fw = sdl_surface->w;
	fh = sdl_surface->h;

	while(w_total>0)
	{
		to_rect.x = x;
		to_rect.y = y;

		if(w_total > fw)
		{
			BlitHitSurface(NULL, &to_rect, NULL, render_hit);

			w_total -= fw;
			x += fw;
		}
		else
		{
			from_rect.x=0;
			from_rect.y=0;
			from_rect.w=w_total;
			from_rect.h=fh;

			BlitHitSurface(&from_rect, &to_rect, NULL, render_hit);

			w_total = 0;
		}
	}
}

void ZSDL_Surface::RenderSurfaceVertRepeat(int x, int y, int h_total, bool render_hit)
{
	SDL_Rect from_rect, to_rect;
	int fw, fh;

	if(!sdl_surface) return;

	fw = sdl_surface->w;
	fh = sdl_surface->h;

	while(h_total>0)
	{
		to_rect.x = x;
		to_rect.y = y;

		if(h_total > fh)
		{
			BlitHitSurface(NULL, &to_rect, NULL, render_hit);

			h_total -= fh;
			y += fh;
		}
		else
		{
			from_rect.x=0;
			from_rect.y=0;
			from_rect.w=fw;
			from_rect.h=h_total;

			BlitHitSurface(&from_rect, &to_rect, NULL, render_hit);

			h_total = 0;
		}
	}
}

void ZSDL_Surface::RenderSurfaceAreaRepeat(int x, int y, int w, int h, bool render_hit)
{
	SDL_Rect from_rect, to_rect;
	int fw, fh;
	int w_left, h_left;
	int ox, oy;

	if(!sdl_surface) return;

	fw = sdl_surface->w;
	fh = sdl_surface->h;

	oy=y;
	h_left=h;
	while(h_left>0)
	{
		ox=x;
		w_left=w;
		while(w_left>0)
		{
			from_rect.x=0;
			from_rect.y=0;
			from_rect.w=w_left;
			from_rect.h=h_left;

			to_rect.x = ox;
			to_rect.y = oy;

			BlitHitSurface(&from_rect, &to_rect, NULL, render_hit);

			ox+=fw;
			w_left-=fw;
		}

		oy+=fh;
		h_left-=fh;
	}
}

void ZSDL_Surface::BlitSurface(ZSDL_Surface *dst, int x, int y)
{
	SDL_Rect to_rect;

	to_rect.x = x;
	to_rect.y = y;

	BlitSurface(NULL, &to_rect, dst);
}

void ZSDL_Surface::BlitSurface(int fx, int fy, int fw, int fh, ZSDL_Surface *dst, int x, int y)
{
	SDL_Rect from_rect, to_rect;

	from_rect.x = fx;
	from_rect.y = fy;
	from_rect.w = fw;
	from_rect.h = fh;

	to_rect.x = x;
	to_rect.y = y;
	to_rect.w = fw;
	to_rect.h = fh;

	BlitSurface(&from_rect, &to_rect, dst);
}

void ZSDL_Surface::BlitSurface(SDL_Rect *srcrect, SDL_Rect *dstrect, ZSDL_Surface *dst)
{
	if(srcrect)
	{
		if(srcrect->h <= 0) return;
		if(srcrect->w <= 0) return;
	}

	if(!sdl_surface) return;

	if(!dst) {
		//SDL_BlitSurface(sdl_surface, srcrect, screen, dstrect);
			
		if (!texture && !LoadTexture()) return;

		//SDL_SetTextureAlphaMod(t, 255);
		//SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);

		if (dstrect) {
			if (srcrect) {
				dstrect->w = srcrect->w;
				dstrect->h = srcrect->h;
			} else {
				dstrect->w = sdl_surface->w;
				dstrect->h = sdl_surface->h;
		    }
		}

		SDL_RenderCopy(renderer, texture, srcrect, dstrect);
		//printf("%d %d*%d size %f\n", __LINE__, sdl_surface->w, sdl_surface->h, size);
	} else {
		dst->BlitOnToMe(srcrect, dstrect, sdl_surface);
	}
}

void ZSDL_Surface::BlitHitSurface(SDL_Rect *srcrect, SDL_Rect *dstrect, ZSDL_Surface *dst, bool render_hit)
{
	if(!render_hit)
	{
		BlitSurface(srcrect, dstrect, dst);
		return;
	}

	SDL_Rect White_Pix_Rect;
	int i,j;
	SDL_Surface *src;

	src = sdl_surface;

	if(!src) return;

	White_Pix_Rect.w = 1;
	White_Pix_Rect.h = 1;

	//SDL_GetRGB
	//SDL_MapRGB

	int start_x, width_x;
	int start_y, height_y;
	int to_x, to_y;

	if(srcrect)
	{
		start_x = srcrect->x;
		width_x = srcrect->w;

		start_y = srcrect->y;
		height_y = srcrect->h;
	}
	else
	{
		start_x = 0;
		start_y = 0;
		width_x = src->w;
		height_y = src->h;
	}

	if(width_x > src->w) width_x = src->w;
	if(height_y > src->h) height_y = src->h;

	if(dstrect)
	{
		to_x = dstrect->x;
		to_y = dstrect->y;
	}
	else
	{
		to_x = 0;
		to_y = 0;
	}

	to_x -= start_x;
	to_y -= start_y;

	for(i=start_x;i<width_x;i++)
		for(j=start_y;j<height_y;j++)
		{
			Uint8 r, g, b, a;
			Uint32 pixel;

			pixel = *(Uint32*)((Uint8*)src->pixels + j * src->pitch + i * src->format->BytesPerPixel);
			SDL_GetRGBA(pixel, src->format, &r, &g, &b, &a);

			if(a)
			{
				White_Pix_Rect.x = to_x + i;
				White_Pix_Rect.y = to_y + j;
				White_Pix_Rect.w = 1;
				White_Pix_Rect.h = 1;
				ZSDL_FillRect(&White_Pix_Rect, 255, 255, 255, dst);
			}
		}
}

int ZSDL_Surface::GetMapBlitInfo(SDL_Surface *src, int x, int y, SDL_Rect &from_rect, SDL_Rect &to_rect)
{
	if(!src) return 0;

	// 	int full_width = (basic_info.width * 16);
	// 	int full_height = (basic_info.height * 16);

	//int view_w = screen_w - HUD_WIDTH;
	//int view_h = screen_h - HUD_HEIGHT;
	int view_w = screen_w - map_place_x;
	int view_h = screen_h - map_place_y;
	int shift_x = 0;
	int shift_y = 0;

	if(has_hud)
	{
		view_w -= HUD_WIDTH;
		view_h -= HUD_HEIGHT;
	}

	//is this visable at ??
	if(x > shift_x + view_w) return 0;
	if(y > shift_y + view_h) return 0;
	if(x + src->w < shift_x) return 0;
	if(y + src->h < shift_y) return 0;

	//setup to
	to_rect.x = x - shift_x;
	to_rect.y = y - shift_y;
	to_rect.w = from_rect.w; // 0 With SDL2 we need width and height too
	to_rect.h = from_rect.h; // 0

	//setup from
	from_rect.x = shift_x - x;
	from_rect.y = shift_y - y;

	if(to_rect.x + src->w > view_w)
		from_rect.w = view_w - to_rect.x;
	else
		from_rect.w = to_rect.x - view_w;

	if(to_rect.y + src->h > view_h)
		from_rect.h = view_h - to_rect.y;
	else
		from_rect.h = to_rect.y - view_h;

	if(from_rect.x < 0) from_rect.x = 0;
	if(from_rect.y < 0) from_rect.y = 0;

	if(from_rect.w > view_w) from_rect.w = view_w;
	if(from_rect.h > view_h) from_rect.h = view_h;

	to_rect.x += from_rect.x;
	to_rect.y += from_rect.y;

	return 1;
}

bool ZSDL_Surface::WillRenderOnScreen(int x, int y, bool about_center)
{
	if(!sdl_surface) return false;

	if(about_center)
	{
		x -= ((sdl_surface->w >> 1) * size);
		y -= ((sdl_surface->h >> 1) * size);
	}

	if(x > screen_w) return false;
	if(y > screen_h) return false;
	if(x + (sdl_surface->w * size) < 0) return false;
	if(y + (sdl_surface->h * size) < 0) return false;

	return true;
}
