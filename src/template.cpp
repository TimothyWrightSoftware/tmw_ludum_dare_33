#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 768;
const int FONT_POINT_SIZE = 16;
const int BUFFER = 1024;

// rendering stuff
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

// fps stuff
TTF_Font *font = NULL;
SDL_Texture* fps_surface = NULL;
Uint32 last_ticks = 0;
Uint32 delta_ticks = 0;
Uint32 frame_count = 0;

// mouse stuff
Sint32 mouse_x = 0;
Sint32 mouse_y = 0;
SDL_Texture* mouse_surface = NULL;

// Music stuff
Mix_Music *music;

void print_init_flags(int flags)
{
#define PFLAG(a) if(flags&MIX_INIT_##a) printf(#a " ")
	PFLAG(FLAC);
	PFLAG(MOD);
	PFLAG(MP3);
	PFLAG(OGG);
	if(!flags)
		printf("None");
	printf("\n");
}

bool init() {

	bool success = true;

	if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
		cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << endl;
		success = false;
	}

	window = SDL_CreateWindow( "You are a monster", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
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
	// load support for the OGG and MOD sample/music formats
	
	int flags=0;
	int initted=Mix_Init(MIX_INIT_FLAC | MIX_INIT_MOD | MIX_INIT_MP3 | MIX_INIT_OGG);
	cout << "Mixer flags: ";
	print_init_flags( initted );
	if( (initted & flags) != flags ) {
		cout <<"Mix_Init: Failed to init required ogg and mod support!" << endl;
		cout << "Mix_Init: " << Mix_GetError() << endl;
	}

	Mix_Quit();
	/* initialize sdl mixer, open up the audio device */
	if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, BUFFER ) < 0 ) {
		cout << "couldn't open mixer?!?" << endl;
		return false;
	}

	/* we play no samples, so deallocate the default 8 channels... */
	Mix_AllocateChannels(0);
	/* print out some info on the formats this run of SDL_mixer supports */
	{
		int i,n=Mix_GetNumChunkDecoders();
		printf("There are %d available chunk(sample) decoders:\n",n);
		for(i=0; i<n; ++i)
			printf("	%s\n", Mix_GetChunkDecoder(i));
		n = Mix_GetNumMusicDecoders();
		printf("There are %d available music decoders:\n",n);
		for(i=0; i<n; ++i)
			printf("	%s\n", Mix_GetMusicDecoder(i));
	}
	/* print out some info on the audio device and stream */
	int audio_rate,audio_channels;
	Uint16 audio_format;
	Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
	int bits=audio_format&0xFF;
	//int sample_size=bits/8+audio_channels;
	//int rate=audio_rate;
	printf("Opened audio at %d Hz %d bit %s, %d bytes audio buffer\n", audio_rate,
			bits, audio_channels>1?"stereo":"mono", BUFFER );

	// load the MP3 file "music.mp3" to play as music
	path = string( SDL_GetBasePath() );
	path += "../res/asz.mp3";
	music=Mix_LoadMUS( path.c_str() );
	if(!music) {
		cout << "Failed to load asz.mp3: " << Mix_GetError() << endl;
		return false;
	}	
	if( Mix_PlayMusic( music, -1 ) == -1 ) {
		cout << "Error starting music?!?" << endl;
	}
	Mix_VolumeMusic(MIX_MAX_VOLUME);

	return success;

}

void handle_keys( SDL_Event& e ) {
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

void handle_mouse( SDL_Event& e ) {
	if( e.type == SDL_MOUSEBUTTONDOWN ) {
		auto& button = e.button;
		if( button.button == SDL_BUTTON_LEFT ) cout << "Left Button Down, ";
		if( button.button == SDL_BUTTON_MIDDLE ) cout << "Middle Button Down, ";
		if( button.button == SDL_BUTTON_RIGHT ) cout << "Right Button Down, ";
		cout << endl;
	}
	if( e.type == SDL_MOUSEBUTTONUP ) {
		auto& button = e.button;
		if( button.button == SDL_BUTTON_LEFT ) cout << "Left Button up, ";
		if( button.button == SDL_BUTTON_MIDDLE ) cout << "Middle Button up, ";
		if( button.button == SDL_BUTTON_RIGHT ) cout << "Right Button up, ";
		cout << endl;
	}
	if( e.type == SDL_MOUSEWHEEL ) {
		cout << "Mouse wheel: " << e.wheel.y << endl;
	}
	if( e.type == SDL_MOUSEMOTION ) {
		mouse_x = e.motion.x;
		mouse_y = e.motion.y;
		ostringstream ss;
		ss << "(" << mouse_x << ", " << mouse_y << ")";
		SDL_Color fgcolor{ 0x00, 0xFF, 0x00, 0xFF };
		SDL_Color bgcolor{ 0x00, 0x00, 0x00, 0xFF };
		SDL_Surface* text = TTF_RenderText_Shaded(font, ss.str().c_str(), fgcolor, bgcolor);
		SDL_DestroyTexture( mouse_surface );
		mouse_surface = SDL_CreateTextureFromSurface(renderer, text);
		SDL_FreeSurface(text);
	}
}

void close() {

    TTF_CloseFont(font);
	Mix_FreeMusic(music);
	Mix_CloseAudio();
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
		SDL_Rect dest { 0, h*0, w, h };
		SDL_RenderCopy( renderer, fps_surface, NULL, &dest );
	}
	{ // draw mouse position
		int w, h;
		SDL_QueryTexture(mouse_surface, NULL, NULL, &w, &h);
		SDL_Rect dest { 0, h*1, w, h };
		SDL_RenderCopy( renderer, mouse_surface, NULL, &dest );
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
				ss << frame_count << " FPS - Relax, cheat, and have fun!!!"; 
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

				handle_keys( e );
				handle_mouse( e );
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
