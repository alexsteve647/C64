#include "vic.h"

VIC::VIC(){

	registers = new uint8_t[0x400];    
	host_charset = new uint8_t[4098 *8];
	host_charset_MCM = new uint8_t[4098 *8];;

	memset(registers,0,0x400);

	registers[CTRL_REG_1_OFF] = 0x9B;
	registers[CTRL_REG_2_OFF] = 0x08;

    control_reg_one(0x9B);
    control_reg_two(0x08);

	screen_memory_base_addr = 0x400;
	char_memory_base_addr = 0xd000;
	bitmap_memory_base_addr = 0x0;

    registers[BASE_ADDR_REG - REG_START] = 0x14;
	registers[IRQ_REQ_REG - REG_START] = 0x0F;
	registers[IRQ_EN_REG - REG_START] = 0x0;

	rasterline = 0;

	interrupt_enabled = false;
	clocks_to_new_render = 1;
	clocks_to_new_line = 63;

	last_time_rendered = chrono::steady_clock::now();

	memset(&color_palette[0],0,16*sizeof(uint8_t));

}

VIC::~VIC(){

	delete[] registers;
	delete[] host_charset;
	delete[] host_charset_MCM;
}

void VIC::init_color_palette(){

    SDL_PixelFormat *format = sdl->getPixelFormat();

	color_palette[0] 	= SDL_MapRGB(format, 0x00, 0x00, 0x00);		//black
	color_palette[1] 	= SDL_MapRGB(format, 0xFF, 0xFF, 0xFF);		//white
	color_palette[2] 	= SDL_MapRGB(format, 0xab, 0x31, 0x26);
	color_palette[3] 	= SDL_MapRGB(format, 0x66, 0xda, 0xff);
	color_palette[4] 	= SDL_MapRGB(format, 0xbb, 0x3f, 0xb8);
	color_palette[5]	= SDL_MapRGB(format, 0x00, 0xcc, 0x55);
	color_palette[6]	= SDL_MapRGB(format, 0x00, 0x00, 0xAA);
	color_palette[7]	= SDL_MapRGB(format, 0xea, 0xf5, 0x7c);
	color_palette[8]	= SDL_MapRGB(format, 0xb9, 0x74, 0x18);
	color_palette[9]	= SDL_MapRGB(format, 0x78, 0x53, 0x00);
	color_palette[10]	= SDL_MapRGB(format, 0xdd, 0x93, 0x87);
	color_palette[11]	= SDL_MapRGB(format, 0x5b, 0x5b, 0x5b);
	color_palette[12]	= SDL_MapRGB(format, 0x8b, 0x8b, 0x8b);
	color_palette[13]	= SDL_MapRGB(format, 0xaa, 0xff, 0x66);
	color_palette[14]	= SDL_MapRGB(format, 0x00, 0x88, 0xff);
	color_palette[15]	= SDL_MapRGB(format, 0xb8, 0xb8, 0xb8);

}

void VIC::draw_bitmap(uint8_t offset, int X, int Y){

	uint8_t bg_color_idx = offset & 0x0F;
	uint8_t fg_color_idx = (offset & 0xF0)>>4;

	uint8_t bg_color = color_palette[bg_color_idx];
	uint8_t fg_color = color_palette[fg_color_idx];

	for(int i=0;i<8;i++){
		uint16_t bitmap_matrix_ptr = bitmap_memory_base_addr + (X+i) * SCREEN_WIDTH/8 + Y + i;
		uint8_t row_pixel_value = memory->VIC_read_byte(bitmap_matrix_ptr);
		uint8_t *ptr = host_video_memory + SCREEN_WIDTH * (i+X) + Y;

		for(int j=0;j<8;j++){

			uint8_t pixel_value = GET_I_BIT(row_pixel_value,j);
			ptr[j] = (pixel_value) ? fg_color : bg_color;

		}

	}

}

void VIC::draw_bitmap_mcm(uint8_t screen_ram, int X, int Y){

	uint8_t bg_color_idx = registers[0xD021-0xD000];
	uint8_t bg_color = color_palette[bg_color_idx];

	uint8_t fg_color_idx = *(guest_color_memory + 40 * X/8 + Y/8);
	uint8_t fg_color = color_palette[fg_color_idx];

	for(int i=0; i<8; i++){

		uint16_t bitmap_matrix_ptr = bitmap_memory_base_addr + X * SCREEN_WIDTH/8 + Y + i;

		uint8_t screen_matrix_value = memory->VIC_read_byte(bitmap_matrix_ptr);
		uint8_t *ptr = host_video_memory + SCREEN_WIDTH * (i+X) + Y;

		for (int j=0; j<8; j+=2){

			uint8_t switch_value = GET_TWO_BITS(screen_matrix_value,(6-j));

			if(switch_value == 0x00){
				ptr[j] = ptr[j+1] = bg_color;
				
			} else if(switch_value == 0x01){
				uint8_t color_idx = (screen_ram & 0xF0)>>4;
				ptr[j] = ptr[j+1] = color_palette[color_idx];

			}else if(switch_value == 0x02){
				uint8_t color_idx = (screen_ram & 0x0F);
				ptr[j] = ptr[j+1] = color_palette[color_idx];
			
			} else if(switch_value == 0x03){
				ptr[j] = ptr[j+1] = fg_color;
			}


		}

	}

}

void VIC::show_char_line(uint8_t offset, int X, int Y, int line_offset){

//	cout<<"LINE OFFSET "<<dec<<line_offset<<"\n";

//	cout<<dec<<"X: "<<X<<" Y "<<Y<<" OF: "<<line_offset<<"\n";

	uint8_t bg_color_idx = registers[0xD021-0xD000];
	uint8_t bg_color = color_palette[bg_color_idx];

	uint8_t mcm_color_two_idx = registers[0xD022-0xD000];
	uint8_t mcm_color_two = color_palette[mcm_color_two_idx];


	uint8_t mcm_color_three_idx = registers[0xD023-0xD000];
	uint8_t mcm_color_three = color_palette[mcm_color_three_idx];

	//non gli piace va oltre di 1
	uint8_t fg_color_idx = *(guest_color_memory + 40 * X/8 + Y/8);
	uint8_t fg_color = color_palette[fg_color_idx];

	uint8_t* font_pointer = host_charset + 64 * offset;
	uint8_t* font_pointer_MCM = host_charset_MCM + 64 * offset;

	//non gli piace
	uint8_t *ptr = host_video_memory + SCREEN_WIDTH * (line_offset + X) + Y;

	for(int j=0; j < CHAR_WIDTH; j++){

		if(graphic_mode == CHAR_MODE or (fg_color_idx < 8))

			//non gli piace
			ptr[j] = *(font_pointer + (line_offset * 8) +j) ? fg_color : bg_color;

		else if(fg_color_idx >= 8 and graphic_mode == MCM_TEXT_MODE){			//MCM

			uint8_t value = *(font_pointer_MCM + (line_offset * 8) +j);

			//cout<<hex<<unsigned(value)<<endl;
			if(value == 0x00)
				ptr[j] = bg_color;
			else if(value == 0x01)
				ptr[j] = mcm_color_two;
			else if(value == 0x02){
				ptr[j] = mcm_color_three;
			} else if(value == 0x03){
				
				cout<<"MCM 0 "<<hex<<unsigned(bg_color_idx)<<endl;
				cout<<"MCM 1 "<<hex<<unsigned(mcm_color_two_idx)<<endl;
				cout<<"MCM 2 "<<hex<<unsigned(mcm_color_three_idx)<<endl;
				cout<<"MCM 3 "<<hex<<unsigned(fg_color_idx)<<endl;
				//ptr[j] = color_palette[fg_color_idx];
				ptr[j] = color_palette[1];

			}		
		} 

	}

}


void VIC::clock(){

	clocks_to_new_line--;
	clocks_to_new_render++;

	if(clocks_to_new_line != 0)
		return;

	rasterline++;
	clocks_to_new_line = 63;


	if(rasterline == 312)
		rasterline = 0;

	if(rasterline == 0){
		sdl->render_frame();
		clocks_to_new_render = 0;

		auto current_time = chrono::steady_clock::now();

		auto c = current_time - last_time_rendered;

		c = chrono::milliseconds(20) - c;

		this_thread::sleep_for(c);

		last_time_rendered = chrono::steady_clock::now();
		return;
	}

	if(!(rasterline >= 50 and rasterline <= 249))
		return;
	
	//TODO: fare con altre raster line
	if(interrupt_enabled){
		cpu->setIRQline();
	}

	update_host_charset();

	//50 is the first visible rasterline;
	int crt_row = rasterline - 50;

	uint32_t cursorX = crt_row/8;
	uint32_t cursorY = 0;

	//cout<<"---------------------------------\n";
	//cout<<"cursorX "<<cursorX<<"\n";

	for(int i=0;i < 40; i++){
		
		//cout<<"offset: "<<dec<<cursorX * 40 + i<<endl;
		//cout<<"Y: "<<dec<<cursorY<<endl;

		//Dovrebbe essere OK!
		uint8_t char_offset = memory->VIC_read_byte(screen_memory_base_addr + cursorX * 40 + i);

		if(graphic_mode == MCM_TEXT_MODE or graphic_mode == CHAR_MODE){
			show_char_line(char_offset ,cursorX*8 ,cursorY, crt_row % 8);
		} else if(graphic_mode == BITMAP_MODE)
			draw_bitmap(char_offset, crt_row, cursorY);
		else if(graphic_mode == MCB_BITMAP_MODE)
			draw_bitmap_mcm(char_offset,crt_row,cursorY);

		cursorY +=8;

		if(cursorY >= SCREEN_WIDTH-7){
			cursorY = 0;
			//cursorX += 8;
		}
	}

	//cout<<"---------------------------------\n";

}

void VIC::update_host_charset(){

	uint8_t byte = 0;
	
	//ROM SIZE
	for(int i=0;i<4096;i++){

		byte = memory->VIC_read_byte(char_memory_base_addr+i);

		for(int m=0;m<8;m++){
			host_charset[i*8+7-m] = (GET_I_BIT(byte,m)) ? 0xFF : 0;
		}
		
		for(int m=0;m<8;m+=2){
			host_charset_MCM[i*8+m] 		= (GET_TWO_BITS(byte,(6-m)));
			host_charset_MCM[i*8+m+1] 		= (GET_TWO_BITS(byte,(6-m)));

		}

	}
/*
	cout<<"-------HOST---------"<<endl;
	hexDump(host_charset,2048);
	cout<<"-------------------------"<<endl;


	cout<<"-------MCM HOST---------"<<endl;
	hexDump(host_charset_MCM,2048*8);
	cout<<"-------------------------"<<endl;
*/
}


void VIC::setMemory(Memory *mem){
	this->memory = mem;
	this->guest_color_memory = mem->getColorMemoryPtr();

}

void VIC::setSDL(SDLManager *sdl){
	this->sdl = sdl;
	this->host_video_memory = sdl->getVideoMemoryPtr();

	memset(host_video_memory,0xE0,64000);

	init_color_palette();

}

void VIC::setCIA1(CIA1 *cia1){

	this->cia1 = cia1;

}

void VIC::setCIA2(CIA2 *cia2){

	this->cia2 = cia2;

}

//TODO: 
uint8_t VIC::read_register(uint16_t addr){
    
    //DEBUG_PRINT("read from VIC memory"<<endl);
    //DEBUG_PRINT(hex<<unsigned(registers[addr-IO_START])<<endl);

	if(addr == RASTER_LINE){
		//cout<<"PC"<<hex<<unsigned(cpu->regs.PC)<<endl;
		//cout<<"reading from RASTER_CNT"<<endl;

		return 0;
	}

    return registers[addr-IO_START];

}

void VIC::write_register(uint16_t addr, uint8_t data){

	//not mapped
	//cout<<"PC"<<hex<<unsigned(cpu->regs.PC)<<endl;

    //DEBUG_PRINT("write to VIC memory"<<endl);
    //DEBUG_PRINT(hex<<unsigned(registers[addr-IO_START])<<endl);

    switch(addr){
        case CTRL_REG_1:
            control_reg_one(data);
            return;
		case CTRL_REG_2:
			control_reg_two(data);
			return;
		case BASE_ADDR_REG:
			char_memory_base_addr   = (data & 0xE) << 10;
    		screen_memory_base_addr = (data & 0xF0) << 6;
			bitmap_memory_base_addr = (data & 0x8) << 10;

    		break;

    	case IRQ_EN_REG:
    		interrupt_enabled = data;
    		break;

		case 0xD019:								//interrupt register
			if(GET_I_BIT(data,0) == 0x0){			//answer to interrupt
				cpu->resetIRQline();
			}

			break;

		case RASTER_LINE:
			cout<<"writing to raster cnt"<<endl;
			cout<<hex<<unsigned(data)<<endl;



    }

    addr -= IO_START;
    registers[addr] = data;


}

void VIC::control_reg_one(uint8_t data){

	
	if(GET_I_BIT(data,3)){
		visible_rows = 24;
	} else {
		visible_rows = 25;
	}

	registers[CTRL_REG_1_OFF] = data;

	set_graphic_mode();

}

void VIC::control_reg_two(uint8_t data){

	if(GET_I_BIT(data,3)){
		visible_cols = 40;
	} else {
		visible_cols = 38;
	}

	registers[CTRL_REG_2_OFF] = data;

	set_graphic_mode();

}

void VIC::set_graphic_mode(){

	bool ecm = GET_I_BIT(registers[CTRL_REG_1_OFF],6);
	bool bmm = GET_I_BIT(registers[CTRL_REG_1_OFF],5); 
	bool mcm = GET_I_BIT(registers[CTRL_REG_2_OFF],4); 

	if(!ecm && !bmm && !mcm){
		cout<<"CHAR"<<endl;

		graphic_mode = CHAR_MODE;
	}else if(!ecm && !bmm && mcm){
		cout<<"MCM CHAR"<<endl;

		graphic_mode = MCM_TEXT_MODE;
	} else if(!ecm && bmm && !mcm){
		cout<<"BITMAP"<<endl;
		graphic_mode = BITMAP_MODE;
	} else if(!ecm && bmm && mcm){
		cout<<"MCM BITMAP"<<endl;
		graphic_mode = MCB_BITMAP_MODE;
	} else
		cout<<"UNIMPL MODE"<<endl;

	//Unimplemented
	/*else if(ecm && !bmm && !mcm)
		graphic_mode_ = kExtBgMode;
	else 
		graphic_mode_ = kIllegalMode;
	*/
}

void VIC::setCPU(CPU* cpu){

	this->cpu = cpu;

}