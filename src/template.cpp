#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#define SQUTIL_IMPLEMENTATION
#include "squtil.h"
#include <iostream>
#include <sstream>
#include <string>
#include <windows.h>

#include <squirrel.h> 
#include <sqstdblob.h>
#include <sqstdio.h> 
#include <sqstdaux.h> 
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <sqstdsystem.h>

using namespace std;
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int FONT_POINT_SIZE = 16;
const int BUFFER = 1024;
const int BACKGROUND_FRAMES = 13;

// scripting stuff
HSQUIRRELVM v; 
FILETIME last_write;

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

// sound stuff
Mix_Music *music = 0;
Mix_Chunk* sample = 0;

// sprite stuff
SDL_Texture* background[ BACKGROUND_FRAMES ] = {0};
int background_frame = 0;
Uint32 background_ticks = 0;
Uint32 background_delta = 0;

void print_init_flags(int flags) {
#define PFLAG(a) if(flags&MIX_INIT_##a) printf(#a " ")
	PFLAG(FLAC);
	PFLAG(MOD);
	PFLAG(MP3);
	PFLAG(OGG);
	if(!flags)
		printf("None");
	printf("\n");
}

SQInteger draw_rect( HSQUIRRELVM v ){
	SQInteger value;
	SDL_Rect rect = {0};
	Uint8 r = 0, g = 0, b = 0, a = 0;
	if( SQ_SUCCEEDED( sq_getinteger( v, -8, &value ) ) ) {
		rect.x = value;
	}
	if( SQ_SUCCEEDED( sq_getinteger( v, -7, &value ) ) ) {
		rect.y = value;
	}
	if( SQ_SUCCEEDED( sq_getinteger( v, -6, &value ) ) ) {
		rect.w = value;
	}
	if( SQ_SUCCEEDED( sq_getinteger( v, -5, &value ) ) ) {
		rect.h = value;
	}
	if( SQ_SUCCEEDED( sq_getinteger( v, -4, &value ) ) ) {
		r = value;
	}
	if( SQ_SUCCEEDED( sq_getinteger( v, -3, &value ) ) ) {
		g = value;
	}
	if( SQ_SUCCEEDED( sq_getinteger( v, -2, &value ) ) ) {
		b = value;
	}
	if( SQ_SUCCEEDED( sq_getinteger( v, -1, &value ) ) ) {
		a = value;
	}
	SDL_SetRenderDrawColor( renderer, r, g, b, a );
	SDL_RenderFillRect( renderer, &rect );
	return 0;
}

SQInteger prev_background( HSQUIRRELVM v ){
	background_frame--;
	if( background_frame < 0 ) {
		background_frame = BACKGROUND_FRAMES - 1;
	} 
	return 0;
}

SQInteger next_background( HSQUIRRELVM v ){
	background_frame++;
	if( background_frame == BACKGROUND_FRAMES ) {
		background_frame = 0;
	} 
	return 0;
}

void register_global_functions() {
	register_global_func( v, next_background, "next_background" );
	register_global_func( v, prev_background, "prev_background" );
	register_global_func( v, draw_rect, "draw_rect" );
}

void register_global_variables() {
	if( SQ_FAILED( register_global_variable( v, "move_left", 0 ) ) ) {
		SDL_assert( !"couldn't set move_left" );
	}
	if( SQ_FAILED( register_global_variable( v, "move_right", 0 ) ) ) {
		SDL_assert( !"couldn't set move_right" );
	}
}

void reload_script() {
	cout << "here we will reload the script" << endl;
	if(SQ_FAILED(sqstd_dofile(v, _SC("../scripts/monster.nut"), SQFalse, SQTrue))) {
		cout << "F'in problem loading script." << endl;
	} 
}

void check_file( bool reload ) {  
	FILETIME creation_time;
	FILETIME last_access;
	FILETIME new_last_write;
	HANDLE file = CreateFile( "../scripts/monster.nut", 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( GetFileTime( file, &creation_time, &last_access, &new_last_write ) ) {
		if( CompareFileTime( &new_last_write, &last_write ) > 0 ) {
			last_write = new_last_write;
			if( reload ) {
				reload_script();
			}
		}
	}

}

void setup_scripting() {

	v = sq_open(1024); // creates a VM with initial stack size 1024 
	sqstd_seterrorhandlers(v); //registers the default error handlers
	sq_setprintfunc(v, printfunc,errorfunc); //sets the print function

	sq_pushroottable( v );
	register_global_functions();
	register_global_variables();

	sqstd_register_bloblib(v);
	sqstd_register_iolib(v);
	sqstd_register_systemlib(v);
	sqstd_register_mathlib(v);
	sqstd_register_stringlib(v);

	if(SQ_FAILED(sqstd_dofile(v, _SC("../scripts/startup.nut"), SQFalse, SQTrue))) {
		cout << "F'in problem loading startup.nut." << endl;
	} 

	check_file( false );
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
	path += "../res/proj1.wav";
	music=Mix_LoadMUS( path.c_str() );
	if(!music) {
		cout << "Failed to load asz.mp3: " << Mix_GetError() << endl;
		return false;
	}	
	//if( Mix_PlayMusic( music, -1 ) == -1 ) {
		//cout << "Error starting music?!?" << endl;
		//return false;
	//}
	Mix_VolumeMusic(MIX_MAX_VOLUME);

	path = string( SDL_GetBasePath() );
	path += "../res/blip.wav";
	sample = Mix_LoadWAV( path.c_str() );
	if( !sample ) {
		cout << "error loading sample" << endl;
		return false;
	}

	{ // load background animations
		for( int i = 0; i < BACKGROUND_FRAMES; ++i ) {
			ostringstream ss;
			if( i < 10 )
				ss << "../res/moving_grid_Animation 1_0" << i << ".png";
			else
				ss << "../res/moving_grid_Animation 1_" << i << ".png";
			path = string( SDL_GetBasePath() );
			path += ss.str(); 
			SDL_Surface* surface = IMG_Load( path.c_str() );
			SDL_assert( surface );
			SDL_DestroyTexture( background[i] );
			background[i]= SDL_CreateTextureFromSurface(renderer, surface);
			SDL_FreeSurface(surface);
			cout << "loaded: " << ss.str() << endl;
		}
	}

	setup_scripting();

	return success;

}

void handle_keys( SDL_Event& e ) {
	if( e.type == SDL_KEYDOWN ) {
		auto& key = e.key.keysym;
		if( key.mod & KMOD_CTRL ) cout << "ctrl, ";
		if( key.mod & KMOD_ALT ) cout << "alt, ";
		if( key.sym == SDLK_RETURN ) cout << "RETURN: " << endl;
		if( key.sym == SDLK_SPACE ) {
			Mix_PlayChannel( -1, sample, 0 );
		}
		if( key.scancode == SDL_SCANCODE_F5 ) {
			reload_script();
		}
	}
	if( e.type == SDL_TEXTINPUT ) {
		cout << "Type: " << e.text.text << endl;
	}
}

void poll_keys() {

	const Uint8 *state = SDL_GetKeyboardState(NULL);
	{ // check for up
		int move_up = state[SDL_SCANCODE_LEFT];
		sq_pushstring( v, _SC( "move_left" ), -1 );
		sq_pushinteger( v, move_up );
		if( SQ_FAILED( sq_newslot( v, -3, SQFalse ) ) ) {
			SDL_assert( !"Failed to set left in script" );
		}
	}
	{ // check for down
		int move_down = state[SDL_SCANCODE_RIGHT];
		sq_pushstring( v, _SC( "move_right" ), -1 );
		sq_pushinteger( v, move_down );
		if( SQ_FAILED( sq_newslot( v, -3, SQFalse ) ) ) {
			SDL_assert( !"Failed to set move_right in script" );
		}
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
	Mix_FreeChunk( sample );
	Mix_CloseAudio();
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( window );
	SDL_Quit();

}

void render() {
	{ // clear screen
		SDL_SetRenderDrawColor(renderer, 0x99, 0x99, 0x99, 0xFF);
		//SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(renderer);
	}
	//{ // draw FPS
		//int w, h;
		//SDL_QueryTexture(fps_surface, NULL, NULL, &w, &h);
		//SDL_Rect dest { 0, h*0, w, h };
		//SDL_RenderCopy( renderer, fps_surface, NULL, &dest );
	//}
	//{ // draw mouse position
		//int w, h;
		//SDL_QueryTexture(mouse_surface, NULL, NULL, &w, &h);
		//SDL_Rect dest { 0, h*1, w, h };
		//SDL_RenderCopy( renderer, mouse_surface, NULL, &dest );
	//}
	{ // render background
		//int w, h;
		//SDL_QueryTexture(background[ background_frame ], NULL, NULL, &w, &h);
		SDL_RenderCopy( renderer, background[ background_frame ], NULL, NULL );
	}
	{ // render script
		get_var( v, "render" );
		sq_pushroottable( v );
		sq_call( v, 1, SQFalse, SQTrue );// leave closure on stack
		sq_pop( v, 1 ); // pop Coroutine and process
	}

}

void update() {

	Uint32 current = SDL_GetTicks();

	{ // update framerate
		delta_ticks += current - last_ticks;
		frame_count++;
		if( delta_ticks > 1000 ) {
			{ // update fps string
				ostringstream ss;
				ss << frame_count << " FPS"; 
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

	{ // update script
		get_var( v, "update" );
		sq_pushroottable( v );
		sq_pushinteger( v, current );
		sq_call( v, 2, SQFalse, SQTrue );// leave closure on stack
		sq_pop( v, 1 ); // pop Coroutine and process
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
		
			check_file( true );
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

			poll_keys();
			update();

			render();
			SDL_RenderPresent( renderer );
		}

		SDL_StopTextInput();
	}

	close();

	return 0;
}
