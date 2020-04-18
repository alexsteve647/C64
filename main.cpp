#include "modules/library.h"
#include "modules/memory.h"
#include "modules/cpu.h"
#include "modules/SDLManager.h"
#include "modules/cia1.h"

#include "modules/debug.h"

#include <stdlib.h>
#include <SDL2/SDL.h>

//uint8_t memory[sixtyfourK];
void test_cpu(CPU*);

char convert(char);
Memory *mem;
CPU *cpu;

auto totale = chrono::duration_cast<chrono::nanoseconds>(chrono::nanoseconds(0)).count();
uint64_t quanti =  0;

void dump_mem_handler(int s){
	cout<<endl<<"Dump Video Mem.."<<endl;
	mem->dump_memory(0xDB00,1000);					//1000 byte not 1024!

	cpu->changeIRQ();


}

void dump_cpu_handler(int s){
	cout<<endl<<"Dump CPU"<<endl;
	cout<<hex<<unsigned(cpu->regs.PC)<<endl;
	//exit(-1);
}

void chiudi(int s){

	cout<<"AVG"<<endl;
	cout<<dec<<totale/quanti<<endl;
	cout<<"----------"<<endl;

	SDL_Quit();
	exit(-1);
}

int main(){
	//CTRL-Z
	//signal(SIGTSTP,dump_mem_handler);
	signal(SIGTSTP,dump_cpu_handler);
	signal(SIGINT,chiudi);

	VIC *vic = new VIC();
	CIA1 *cia1 = new CIA1();

	mem = new Memory();
	mem->load_kernal_and_basic(KERNAL_BASIC_ROM);
	mem->load_charset(CHARSET_ROM);

	cpu = new CPU(mem);

	cia1->setCPU(cpu);

	SDLManager *sdl = new SDLManager();
	sdl->setCIA1(cia1);

	mem->setVIC(vic);
	mem->setCIA1(cia1);

	vic->setMemory(mem);
	vic->setSDL(sdl);
	vic->setCPU(cpu);
	vic->setCIA1(cia1);

	while(true){
		auto start = chrono::steady_clock::now();
		cpu->clock();
		cia1->clock();
		auto end = chrono::steady_clock::now();
		auto c = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
		totale += c;
		quanti++;

		//cout<<totale/quanti<<endl;
		//cout << "Elapsed time in nanoseconds : " <<dec<<c<< " ns" << endl;
	}

}

void test_cpu(CPU *cpu)
{
	uint16_t pc=0;
	uint8_t opcode;
	bool loop = true;

	while(loop)
	{
		
		if(pc == cpu->regs.PC)
		{
			cout<<"infinite loop at "<<hex<<unsigned(pc)<<endl;
			break;

		} else if(cpu->regs.PC == 0x3463)
		{
			cout<<"test passed!"<<endl;
			break;
		}
		
		pc = cpu->regs.PC;
		
		opcode = cpu->fetch();
		cout<<"OPCODE: "<<hex<<unsigned(opcode)<<endl;

		loop = cpu->decode(opcode);
		if(loop == false){
			cout<<"Bloccato "<<endl;
		}

		cpu->dump_reg();

	}

}


/*char convert(char c){
	cout<<"ciao"<<endl;

	if(c >= 65 && c <= 90){					//lower case letters
		cout<<"lower case";
		c+= (97-65);
	} else if(c>=193 && c<= 218){
		cout<<"upper case";
		c-= (193-65);
	}

	return c;


}*/