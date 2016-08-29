#include "zsdl_opengl.h"
#include "zsdl.h"
#include "constants.h"
#include "common.h"

// hack for libsmpeg2 linkage. needs a new compiler?
void operator delete(void* p, unsigned int s) {
	printf("delete called %p %d\n", p, s);
	::operator delete(p);
}

using namespace COMMON;

void InitOpenGL()
{
#ifndef DISABLE_OPENGL
    glEnable(GL_TEXTURE_2D);			// Enable Texture Mapping
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// Clear The Background Color To Black 
    glClearDepth(0.0f);				// Disables(?) Clearing Of The Depth Buffer
	glDepthFunc( GL_ALWAYS );
    glEnable(GL_DEPTH_TEST);			// Enables Depth Testing
    glShadeModel(GL_SMOOTH);			// Enables Smooth Color Shading

	glEnable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glAlphaFunc(GL_GREATER, 0.5);
	glDisable(GL_LIGHTING);

    glEnable(GL_BLEND);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
	glLoadIdentity();				// Reset The View
#endif
}

void ResetOpenGLViewPort(int width, int height)
{
#ifndef DISABLE_OPENGL
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); // Reset The Projection Matrix
    
	glOrtho(0.0f, width, height, 0.0f, 1.0f, -1.0f);
    
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
#endif
}

bool ZSDL_Surface::use_opengl = false;
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
	//sdl_rotozoom = NULL;
	texture = NULL;
	
	gl_texture_loaded = false;
	//rotozoom_loaded = false;

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

	if(sdl_surface) SDL_FreeSurface(sdl_surface);
	//if(sdl_rotozoom) SDL_FreeSurface(sdl_rotozoom);
#ifndef DISABLE_OPENGL
	if(gl_texture_loaded) glDeleteTextures(1, &gl_texture);

	gl_texture = 0;	
#endif

	texture = NULL;
	
	sdl_surface = NULL;
	//sdl_rotozoom = NULL;
	gl_texture_loaded = false;
	//rotozoom_loaded = false;
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

	if (!use_opengl)
	{
		//convert to a guaranteed good format
		SDL_Surface *new_ret;
		//new_ret = SDL_DisplayFormatAlpha(sdl_surface);
		new_ret = SDL_ConvertSurfaceFormat(sdl_surface, SDL_PIXELFORMAT_ARGB8888, 0);
if (!new_ret) printf("NULL\n");
		if(delete_surface) SDL_FreeSurface( sdl_surface );
		sdl_surface = new_ret;
	}

#if 0
#ifdef __amigaos4__
	else
	{
		// Convert to RGBA if needed
		SDL_PixelFormat fmt;

		fmt.palette = NULL;
		fmt.BitsPerPixel = 32;
		fmt.BytesPerPixel = 4;
		
		fmt.Rloss = fmt.Gloss = fmt.Bloss = fmt.Aloss = 0;
		
		fmt.Rshift = 24;
		fmt.Gshift = 16;
		fmt.Bshift = 8;
		fmt.Ashift = 0;
		
		fmt.Rmask = 0xFF000000;
		fmt.Gmask = 0x00FF0000;
		fmt.Bmask = 0x0000FF00;
		fmt.Amask = 0x000000FF;
		
		fmt.colorkey = 0;
		fmt.alpha = 255;
		
		SDL_Surface * converted = SDL_ConvertSurface(sdl_surface, &fmt, SDL_SWSURFACE);
		
		if (converted)
		{
			if (delete_surface)
			{
				SDL_FreeSurface(sdl_surface);
			}
			
			sdl_surface = converted;			
		}
		else
		{
			printf("SDL_ConvertSurface: %s\n", SDL_GetError());
		}
		
	}

#endif
#endif

	//checks
	if ( (sdl_surface->w & (sdl_surface->w - 1)) != 0 )
		;//printf("warning: %s's width is not a power of 2\n", image_filename.c_str());
	
	// Also check if the height is a power of 2
	if ( (sdl_surface->h & (sdl_surface->h - 1)) != 0 )
		;//printf("warning: %s's height is not a power of 2\n", image_filename.c_str());
}

void ZSDL_Surface::UseDisplayFormat()
{
	if(!sdl_surface) return;

	//this is not needed in opengl (?)
	if(use_opengl) return;

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
	if(!sdl_surface) return;

	//this is not needed in opengl (?)
	if(use_opengl) return;

	ZSDL_ModifyBlack(sdl_surface);
	UseDisplayFormat();
	//SDL_SetColorKey(sdl_surface, SDL_SRCCOLORKEY, 0x000000);
	SDL_SetColorKey(sdl_surface, SDL_TRUE, 0x000000);
}

#if 0
bool ZSDL_Surface::LoadRotoZoomSurface()
{
	if(!sdl_surface)
	{
		//printf("LoadRotoZoomSurface::sdl_surface for %s not loaded\n", image_filename.c_str());
		return false;
	}

	//unload if loaded
	if(sdl_rotozoom) SDL_FreeSurface(sdl_rotozoom);

	sdl_rotozoom = rotozoomSurface(sdl_surface, angle, size, 0);

	if(!sdl_rotozoom)
	{
		printf("LoadRotoZoomSurface::sdl_rotozoom for %s not created (angle:%f size:%f)\n", image_filename.c_str(), angle, size);
		return false;
	}

	rotozoom_loaded = true;
	return true;
}
#endif

#ifndef DISABLE_OPENGL
int ZSDL_Surface::GetHighBit(int number)
{
	int highBit = 0;

	for (int i = 0; i < sizeof(int) * 8; i++)
	{
		if (number & (1 << i))
		{
			highBit = i;
		}
	}

	return highBit;
}

SDL_Surface * ZSDL_Surface::MakeSurfacePOT(SDL_Surface * surface)
{
	int w = 1 << GetHighBit(sdl_surface->w);
	int h = 1 << GetHighBit(sdl_surface->h);		
	
	if (w == surface->w && h == surface->h)
	{
		// no need to convert
		return surface;
	}
	
	if (w != surface->w)
	{
		w <<= 1;
	}
	
	if (h != surface->h)
	{
		h <<= 1;
	}
	
	SDL_Surface * newSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	
	if (!newSurface)
	{
		printf("Failed to make POT surface: %s\n", SDL_GetError());
		return surface;
	}
	
	if (SDL_MUSTLOCK(surface))
	{
		SDL_LockSurface(surface);
	}	
		
	if (SDL_MUSTLOCK(newSurface))
	{
		SDL_LockSurface(surface);
	}	
		
	// Assumes that surface is 32-bit, and with tightly packed pixel data

/*	
	if (w != surface->w || h != surface->h)
	{
		const float yStep = (float)surface->h / h;
		const float xStep = (float)surface->w / w;
		
		for (int y = 0; y < h; y++)
		{
			const Uint32 * const srcPtr = (Uint32 *)surface->pixels + (int)(y * yStep) * surface->w;
			Uint32 * const dstPtr = (Uint32 *)newSurface->pixels + y * newSurface->w;
			
			for (int x = 0; x < w; x++)
			{
				*(dstPtr + x) = *(srcPtr + (int)(x * xStep)); 
			}
		}
	}
*/
	for (int y = 0; y < surface->h; y++)
	{
		const Uint32 * const srcPtr = (Uint32 *)surface->pixels + y * surface->w;
		Uint32 * const dstPtr = (Uint32 *)newSurface->pixels + y * newSurface->w;
		
		for (int x = 0; x < surface->w; x++)
		{
			*(dstPtr + x) = *(srcPtr + x);
		}
	}
	
	texture_width = (float)surface->w / newSurface->w;
	texture_height = (float)surface->h / newSurface->h;
	
	if (SDL_MUSTLOCK(surface))
	{
		SDL_UnlockSurface(surface);
	}
	
	if (SDL_MUSTLOCK(newSurface))
	{
		SDL_UnlockSurface(newSurface);
	}
	
	SDL_FreeSurface(surface);
	
	return newSurface;
}
#endif

bool ZSDL_Surface::LoadGLtexture()
{
#ifndef DISABLE_OPENGL
	GLenum texture_format;
	GLint  nOfColors;
	GLint internalFormat;

	if(!sdl_surface)
	{
		printf("LoadGLtexture::sdl_surface for %s not loaded\n", image_filename.c_str());
		return false;
	}

	//delete texture if it is already loaded
	if(gl_texture_loaded)
	{
		glDeleteTextures(1, &gl_texture);
		gl_texture = 0;
	}
	
	nOfColors = sdl_surface->format->BytesPerPixel;

	texture_width = 1.0f;
	texture_height = 1.0f;
	
	switch(nOfColors)
	{
	case 4:
		internalFormat = GL_RGBA;
		
#ifdef __amigaos4__
		texture_format = GL_RGBA;
#else	
		if (sdl_surface->format->Rmask == 0x000000ff)
		{
			texture_format = GL_RGBA;
		}
		else
		{
			texture_format = GL_BGRA;
		}
#endif		
		break;
		
	case 3:
		internalFormat = GL_RGB;

#ifdef __amigaos4__
		texture_format = GL_RGB;
#else		
		if (sdl_surface->format->Rmask == 0x000000ff)
		{
			texture_format = GL_RGB;
		}
		else
		{
			texture_format = GL_BGR;
		}
#endif		
		break;
		
	default:
		printf("LoadGLtexture::%s does not have proper image format\n", image_filename.c_str());
		return false;
		break;
	}

#ifdef __amigaos4__
	//const int hackW = sdl_surface->w;
	//const int hackH = sdl_surface->h;
	
	sdl_surface = MakeSurfacePOT(sdl_surface);
#endif		
	
	// Have OpenGL generate a texture object handle for us
	glGenTextures( 1, &gl_texture );
  
	// Bind the texture object
	glBindTexture( GL_TEXTURE_2D, gl_texture );
 
	// Set the texture's stretching properties
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  
	if (SDL_MUSTLOCK(sdl_surface))
	{
		SDL_LockSurface(sdl_surface);
	}

	// Edit the texture object's image data using the information SDL_Surface gives us
	glTexImage2D( GL_TEXTURE_2D, 0, internalFormat /*nOfColors*/, sdl_surface->w, sdl_surface->h, 0, texture_format, GL_UNSIGNED_BYTE, sdl_surface->pixels );

	if (SDL_MUSTLOCK(sdl_surface))
	{
		SDL_UnlockSurface(sdl_surface);
	}
	
	gl_texture_loaded = true;

	//sdl_surface->w = hackW;
	//sdl_surface->h = hackH;
	
	return true;
#else
	return false;
#endif
}

void ZSDL_Surface::SetUseOpenGL(bool use_opengl_)
{
printf("use_opengl %d\n", use_opengl_);
	use_opengl = use_opengl_;
}

/*
void ZSDL_Surface::SetMainSoftwareSurface(SDL_Surface *screen_)
{
	screen = screen_;
}
*/

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
#if 0
	//unload rotozoom surface?
	if(!use_opengl && size != size_)
	{
		if(sdl_rotozoom) SDL_FreeSurface(sdl_rotozoom);
		sdl_rotozoom = NULL;
		rotozoom_loaded = false;
	}
#endif
	size = size_;
}

void ZSDL_Surface::SetAngle(float angle_)
{
#if 0
	//unload rotozoom surface?
	if(!use_opengl && angle != angle_)
	{
		if(sdl_rotozoom) SDL_FreeSurface(sdl_rotozoom);
		sdl_rotozoom = NULL;
		rotozoom_loaded = false;
	}
#endif
	angle = angle_;
}

void ZSDL_Surface::SetAlpha(char alpha_)
{
	alpha = alpha_;

	if(!use_opengl)
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

	if(gl_texture_loaded) 
	{
#ifndef DISABLE_OPENGL
		glDeleteTextures(1, &gl_texture);
		gl_texture = 0;
#endif
		gl_texture_loaded = false;
	}

#if 0
	//unload rotozoom surface?
	if(!use_opengl)
	{
		if(sdl_rotozoom) SDL_FreeSurface(sdl_rotozoom);
		sdl_rotozoom = NULL;
		rotozoom_loaded = false;
	}
#endif
}

void ZSDL_Surface::BlitOnToMe(SDL_Rect *srcrect, SDL_Rect *dstrect, SDL_Surface *src)
{
	if(!sdl_surface) return;

	SDL_BlitSurface(src, srcrect, sdl_surface, dstrect);

	if(gl_texture_loaded) 
	{
#ifndef DISABLE_OPENGL
		glDeleteTextures(1, &gl_texture);
		
		gl_texture = 0;
#endif
		gl_texture_loaded = false;
	}
#if 0
	//unload rotozoom surface?
	if(!use_opengl)
	{
		if(sdl_rotozoom) SDL_FreeSurface(sdl_rotozoom);
		sdl_rotozoom = NULL;
		rotozoom_loaded = false;
	}
#endif
}

//ZSDL_Surface has made itself into an engine it seems...
void ZSDL_Surface::ZSDL_FillRect(SDL_Rect *dstrect, char r, char g, char b, ZSDL_Surface *dst)
{
	if(dst)
	{
		dst->FillRectOnToMe(dstrect, r, g, b);
		return;
	}

	if(use_opengl)
	{
#ifndef DISABLE_OPENGL
		if(dstrect)
		{

			glPushMatrix();

			glColor4ub(r,g,b,255);
			//glColor3f(r,g,b);
			glBindTexture(GL_TEXTURE_2D, 0);

			glTranslatef(dstrect->x,dstrect->y,0.0f);

			glBegin(GL_QUADS);

			glVertex3f(0.0f, 0.0f, 0.0f);		// Top Left
			glVertex3f(dstrect->w, 0.0f, 0.0f);		// Top Right
			glVertex3f(dstrect->w, dstrect->h, 0.0f);		// Bottom Right
			glVertex3f(0.0f, dstrect->h, 0.0f);		// Bottom Left
			glEnd();

			glPopMatrix();

			//glColor3f(1,1,1);
		}
		else
		{
			//we are supposed to fill the whole screen with this color
			//this area is not debuged.
			
			glClearColor(r / 255.0, g / 255.0, b / 255.0, 0.0f); // Clear The Background Color To Black
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
			glLoadIdentity(); // Reset The View
		}
#endif
	}
	else
	{
		//if(screen) SDL_FillRect(screen, dstrect, SDL_MapRGB(screen->format, r,g,b));
		
		if (renderer)
		{
			if (dstrect)
			{
				// TODO
			}
			else
			{
			    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
			    SDL_RenderClear(renderer);
			}
		}
	}
}

void ZSDL_Surface::RenderSurface(int x, int y, bool render_hit, bool about_center)
{
	if(use_opengl)
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

#if 0
		//should we be using the rotozoom surface?
		if(!isz(angle) || !is1(size))
		{
			if(!rotozoom_loaded && !LoadRotoZoomSurface()) return;
			render_surface = sdl_rotozoom;
		}
		else
			render_surface = sdl_surface;
#endif
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
				//SDL_BlitSurface(render_surface, &from_rect, screen, &to_rect);
				SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, render_surface);
				if (t)
				{
					if (!isz(angle) || !is1(size))
					{
						//SDL_RenderCopyEx(renderer, texture, );
						SDL_RenderCopyEx(renderer, t, &from_rect, &to_rect, angle, NULL, SDL_FLIP_NONE);
					}
					else
					{
						SDL_RenderCopy(renderer, t, &from_rect, &to_rect);
					}
				printf("%d\n", __LINE__);
					SDL_DestroyTexture(t);
				}
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

	if(use_opengl)
	{
		//blit on to them
		if(dst)
		{
			dst->BlitOnToMe(srcrect, dstrect, sdl_surface);
			return;
		}

#ifndef DISABLE_OPENGL
		if(!gl_texture_loaded && !LoadGLtexture()) return;

		glPushMatrix();

		if(dstrect) glTranslatef(dstrect->x, dstrect->y, 0.0f);
		//else glTranslatef(0.0, 0.0, 0.0f);

		//scale
		glScalef( size, size, 1.0 );

		//alpha
		glColor4ub(255,255,255, alpha);

		const int actual_width = texture_width * sdl_surface->w;
		const int actual_height = texture_height * sdl_surface->h;	
		
		//rotate about center
		glTranslatef((actual_width /*sdl_surface->w*/ >> 1), (actual_height /*sdl_surface->h*/ >> 1), 0.0f);
		glRotatef(angle, 0.0f, 0.0f, 1.0f);
		glTranslatef(-(actual_width /*sdl_surface->w*/ >> 1), -(actual_height /*sdl_surface->h*/ >> 1), 0.0f);

		glBindTexture(GL_TEXTURE_2D, gl_texture);

		glBegin(GL_QUADS);
		
		if(!srcrect)
		{
			glTexCoord2f(0.0f, texture_height); glVertex3f( 0.0f, actual_height /*sdl_surface->h*/,  0.0f);	// Bottom Left Of The Texture and Quad
			glTexCoord2f(texture_width, texture_height); glVertex3f( actual_width /*sdl_surface->w*/, actual_height /*sdl_surface->h*/,  0.0f);	// Bottom Right Of The Texture and Quad
			glTexCoord2f(texture_width, 0.0f); glVertex3f( actual_width /*sdl_surface->w*/, 0.0f,  0.0f);	// Top Right Of The Texture and Quad
			glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.0f, 0.0f,  0.0f);	// Top Left Of The Texture and Quad
		}
		else
		{
			if(srcrect->x + srcrect->w > actual_width /*sdl_surface->w*/) srcrect->w = actual_width /*sdl_surface->w*/ - srcrect->x;
			if(srcrect->y + srcrect->h > actual_height /*sdl_surface->h*/) srcrect->h = actual_height /*sdl_surface->h*/ - srcrect->y;
			
			GLfloat tex_x =  (1.0f * srcrect->x) / actual_width; //sdl_surface->w;
			GLfloat tex_x2 = (1.0f * srcrect->x + srcrect->w) / actual_width; //sdl_surface->w;
			GLfloat tex_y =  (1.0f * srcrect->y) / actual_height; //sdl_surface->h;
			GLfloat tex_y2 = (1.0f * srcrect->y + srcrect->h) / actual_height;//sdl_surface->h;

			glTexCoord2f(tex_x,  tex_y2); glVertex3f( 0.0f, srcrect->h,  0.0f);	// Bottom Left Of The Texture and Quad
			glTexCoord2f(tex_x2, tex_y2); glVertex3f( srcrect->w, srcrect->h,  0.0f);	// Bottom Right Of The Texture and Quad
			glTexCoord2f(tex_x2, tex_y); glVertex3f( srcrect->w, 0.0f,  0.0f);	// Top Right Of The Texture and Quad
			glTexCoord2f(tex_x,  tex_y); glVertex3f( 0.0f, 0.0f,  0.0f);	// Top Left Of The Texture and Quad
		}

		glEnd();

		glPopMatrix();
#endif
	}
	else
	{
		if(!sdl_surface) return;
		//if(!texture) return;

		//software render
		if(!dst)
		{
			//SDL_BlitSurface(sdl_surface, srcrect, screen, dstrect);
			// alpha? rotation/scaling seems not needed here
			// update texture, where?
			SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, sdl_surface);
			if (t)
			{
				SDL_SetTextureAlphaMod(t, 255);
				SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);

				SDL_RenderCopy(renderer, t, srcrect, dstrect);
				printf("%d %d*%d size %f\n", __LINE__, sdl_surface->w, sdl_surface->h, size);

				SDL_DestroyTexture(t);
			}
		}
		else
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
	to_rect.w = 0;
	to_rect.h = 0;

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
