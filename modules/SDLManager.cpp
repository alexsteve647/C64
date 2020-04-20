#include "SDLManager.h"


SDLManager::SDLManager(){

	video_memory = new pixel_type[SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(pixel_type)];

	video_thread = new thread(&SDLManager::initialize_SDL,this);
	video_thread->detach();

	total_redraws = 0;
	start_time = chrono::steady_clock::now();

	format = SDL_AllocFormat(SDL_PIXELFORMAT_RGB332);

}

SDLManager::~SDLManager(){

	delete[] video_memory;
	SDL_FreeFormat(format);

}

void SDLManager::checkFPS(){
	auto end_time = chrono::steady_clock::now();

	auto totale = chrono::duration_cast<chrono::seconds>(end_time - start_time).count();

	double frames = total_redraws;

	cout<<"Redraws "<<dec<<unsigned(total_redraws)<<endl;

	cout<<"Time "<<dec<<unsigned(totale)<<" s"<<endl;

	cout<<"FPS "<<dec<<frames/totale<<endl;

}

void SDLManager::initialize_SDL(){

	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return;
	}


	window = SDL_CreateWindow("Hello World!", 100, 100, SCREEN_WIDTH*2, SCREEN_HEIGHT*2, SDL_WINDOW_SHOWN);
	
	if (window == nullptr){
		cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	
	if (renderer == nullptr){
		SDL_DestroyWindow(window);
		cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return;
	}

	texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGB332,SDL_TEXTUREACCESS_STREAMING,SCREEN_WIDTH,SCREEN_HEIGHT);

	keyboard_loop();

}

void SDLManager::render_frame(){

	total_redraws++;

	if(texture == nullptr || renderer == nullptr)
		return;

	SDL_UpdateTexture(texture,NULL,video_memory,SCREEN_WIDTH * sizeof(pixel_type));
	SDL_RenderCopy( renderer, texture, NULL, NULL );
	SDL_RenderPresent( renderer );

}


pixel_type* SDLManager::getVideoMemoryPtr(){

	return video_memory;

}

void SDLManager::setCIA1(CIA1* cia1){
	this->cia1 = cia1;
}

SDL_PixelFormat* SDLManager::getPixelFormat(){

	return format;

}


void SDLManager::terminate(){

	SDL_DestroyRenderer(renderer);
	SDL_DestroyTexture(texture);
	SDL_DestroyWindow(window);

	SDL_Quit();

	exit(-1);

}


void SDLManager::keyboard_loop(){

	while(true){

		KeyboardMatrix new_pressed_matrix;
		KeyboardMatrix last_pressed_matrix;
		last_pressed_matrix.row = 0xFF;
		last_pressed_matrix.col = 0xFF;


		SDL_Event event;
		while( SDL_WaitEvent( &event ) ){

		// We are only worried about SDL_KEYDOWN and SDL_KEYUP events
			switch( event.type ){
				case SDL_KEYDOWN:
					cout<<"Key press detected: \n";
					new_pressed_matrix = RowColFromScancode(event.key.keysym.scancode,true);

					last_pressed_matrix.row &= new_pressed_matrix.row;
					last_pressed_matrix.col &= new_pressed_matrix.col;

					cia1->setKeyPressed(last_pressed_matrix);
				
					break;

				case SDL_KEYUP:
					new_pressed_matrix = RowColFromScancode(event.key.keysym.scancode,false);

					last_pressed_matrix.row |= new_pressed_matrix.row;
					last_pressed_matrix.col |= new_pressed_matrix.col;

					cia1->setKeyPressed(last_pressed_matrix);

					cout<<"Key release detected\n";
					break;

				case SDL_QUIT:
					cout<<"Terminating\n";
					terminate();
					//loop = false;
					break;

				/*case SDL_WINDOWEVENT:

					switch (event.window.event) {

					    case SDL_WINDOWEVENT_CLOSE:  		//X of the window
							//cout<<"premuta X"<<endl;
							//machine->quit();
							break;

					    default:
					        break;
					}

        			break;*/

				default:
		    		break;
			}



		}

	}

}