#include "SDL.h"
#include "SDL_ttf.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 768;
const int FONT_POINT_SIZE = 16;

// rendering stuff
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

// fps stuff
TTF_Font *font = NULL;
SDL_Texture* fps_surface = NULL;
Uint32 last_ticks = 0;
Uint32 delta_ticks = 0;
Uint32 frame_count = 0;

bool init() {

	bool success = true;

	if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
		cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << endl;
		success = false;
	}

	window = SDL_CreateWindow( "SDL Template", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
	if( window == NULL ) {
		cout << "Window could not be created! SDL Error: " << SDL_GetError() << endl;
		success = false;
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
	SDL_assert( renderer != NULL );
    if ( TTF_Init() < 0 ) {
		cout << "Couldn't init TTF library" << SDL_GetError() << endl;
		success = false;
    }

	string path( SDL_GetBasePath() );
	path += "../res/sansation.ttf";
    font = TTF_OpenFont(path.c_str(), FONT_POINT_SIZE);
    if ( font == NULL ) {
		cout << "Couldn't load " << path << " font" << SDL_GetError() << endl;
		return false;
    }
    //TTF_SetFontStyle(font, renderstyle);
    //TTF_SetFontOutline(font, outline);
    //TTF_SetFontKerning(font, kerning);
    //TTF_SetFontHinting(font, hinting);

	return success;

}

void handleKeys( SDL_Event& e ) {
	if( e.type == SDL_KEYDOWN ) {
		auto& key = e.key.keysym;
		if( key.mod & KMOD_CTRL ) cout << "ctrl, ";
		if( key.mod & KMOD_ALT ) cout << "alt, ";
		if( key.sym == SDLK_RETURN ) cout << "RETURN: " << endl;
	}
	if( e.type == SDL_TEXTINPUT ) {
		cout << "Type: " << e.text.text << endl;
	}
}

void close() {

    TTF_CloseFont(font);
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( window );
	SDL_Quit();

}

void render() {
	{ // clear screen
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderClear(renderer);
	}
	{ // draw FPS
		int w, h;
		SDL_QueryTexture(fps_surface, NULL, NULL, &w, &h);
		SDL_Rect dest { 0, 0, w, h };
		SDL_RenderCopy( renderer, fps_surface, NULL, &dest );
	}
}

void update() {

	{ // update framerate
		Uint32 current = SDL_GetTicks();
		delta_ticks += current - last_ticks;
		frame_count++;
		if( delta_ticks > 1000 ) {
			{ // update fps string
				ostringstream ss;
				ss << frame_count << " FPS - You are a monster!";
				SDL_Color fgcolor{ 0x00, 0xFF, 0x00, 0xFF };
				SDL_Color bgcolor{ 0x00, 0x00, 0x00, 0xFF };
				SDL_Surface* text = TTF_RenderText_Shaded(font, ss.str().c_str(), fgcolor, bgcolor);
				SDL_DestroyTexture( fps_surface );
				fps_surface = SDL_CreateTextureFromSurface(renderer, text);
				SDL_FreeSurface(text);
		   	}
			frame_count = 0;
			delta_ticks -= 1000;
		} 
		last_ticks = current;
	}
}

int wmain( int argc, wchar_t *argv[ ], wchar_t *envp[ ] ) {
	
	if( !init() ) {
		cout << "failed to initialize" << endl;
	} else {

		bool quit = false;

		last_ticks = SDL_GetTicks();
		delta_ticks = 0;
		SDL_StartTextInput();
		SDL_Event e;
		while( !quit ) {
		
			while( SDL_PollEvent( &e ) != 0 ) {
	
				if( e.type == SDL_QUIT ) {
					quit = true;
				}
				if( e.type == SDL_KEYDOWN ) {
					if( e.key.keysym.sym == SDLK_ESCAPE ) {
						quit = true;
					}
				}

				handleKeys( e );
			}

			update();
			render();
			SDL_RenderPresent( renderer );
		}

		SDL_StopTextInput();
	}

	close();

	return 0;
}
