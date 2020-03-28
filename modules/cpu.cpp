#include "cpu.h"

CPU::CPU(uint8_t* memory, uint16_t PC){

	this->memory = memory;
	regs.PC = PC;
	regs.SP = 0;

}

CPU::CPU(uint8_t* memory){

	this->memory = memory;

	regs.SP = 0;
	regs.PC = RESET_routine;

}

uint8_t CPU::zero_page(register_name index){
  //Immediate operand
  
	uint8_t addr;

	addr = immediate();

 	//Sum X register
	addr += regs.reg[index];

	return addr;

}

uint8_t CPU::zero_page(){
  
	uint8_t addr;
	addr = memory[regs.PC];

	regs.PC++;

	return addr;

}

uint8_t CPU::immediate(){

	uint8_t addr;
	addr = read_byte(regs.PC);

	regs.PC++;

	return addr;
}

uint8_t CPU::read_byte(uint16_t addr){

	uint8_t data = memory[addr];

	return data;

}

uint16_t CPU::read_word(uint16_t addr){

	uint16_t data = memory[addr];
	uint16_t tmp = memory[addr+1];

	tmp = tmp <<8;
	data |= tmp;

	return data;

}

uint16_t CPU::absolute(){

	/*Prima parte bassa e poi parte alta
	uint16_t addr;
	addr = read_byte(regs.PC);

	regs.PC++;

	uint16_t tmp;
	tmp = memory[regs.PC];
	regs.PC++;

	tmp = tmp <<8;
	addr |= tmp;
*/

	uint16_t addr = read_word(regs.PC);

	regs.PC += 2;

	return addr;

}

uint16_t CPU::absolute(register_name index){

	uint16_t addr = absolute();
	addr+= regs.reg[index];

	return addr;

}

uint16_t CPU::indirect_X(){

	uint16_t addr;

	//zero page addr!!
	uint8_t zero_page_addr = immediate();
	//uint16_t tmp;

	zero_page_addr += regs.reg[regX];

	//least significant byte
	addr = read_word(zero_page_addr);

	/*addr = memory[zero_page_addr];

	tmp = memory[zero_page_addr+1];
	tmp = tmp << 8;

	addr |= tmp;*/

	return addr; 


}

uint16_t CPU::indirect_Y(){

	uint16_t addr;

	//zero page addr!!
	uint8_t zero_page_addr = immediate();
	
	//uint16_t tmp;

	addr = read_word(zero_page_addr);


	//address is stored inside the zero-page
	//Least significant byte
	/*addr = memory[zero_page_addr];

	//Most sig. byte
	tmp = memory[zero_page_addr+1];
	tmp = tmp << 8;
	addr |= tmp;
*/

	addr += regs.reg[regY];

	return addr;

}

void CPU::OR(register_name index, uint8_t operand){

	regs.reg[index] |= operand;
	SET_ZF(regs.reg[index]);
	SET_NF(regs.reg[index]);

}

void CPU::AND(uint8_t operand){

	regs.reg[regA] &= operand;
  	SET_ZF(regs.reg[regA]);
  	SET_NF(regs.reg[regA]);

}

void CPU::LD(register_name index, uint8_t operand){

	regs.reg[index] = operand;
	SET_ZF(regs.reg[index]);
	SET_NF(regs.reg[index]);

}

void CPU::ST(register_name index, uint16_t addr){

	memory[addr] = regs.reg[index];

}

void CPU::JMP(uint16_t addr){
	regs.PC = memory[addr];
}

void CPU::PUSH(register_name index){

	uint16_t addr = STACK_START + regs.SP;
	memory[addr] = regs.reg[index];
	regs.SP--;
}

void CPU::PUSH(uint8_t value){

	uint16_t addr = STACK_START + regs.SP;
	memory[addr] = value;
	regs.SP--;
}

uint8_t CPU::POP(){

  	uint16_t addr = ++regs.SP + STACK_START;

  	return memory[addr];

}

void CPU::JSR(uint16_t addr){

	PUSH((regs.PC -1 ) >>8);
	PUSH((regs.PC -1 ));
	regs.PC = addr;

}


uint8_t CPU::fetch(){

	uint8_t opcode = read_byte(regs.PC);

	regs.PC++;
	return opcode;

}

uint8_t CPU::flags()
{
	uint8_t v = 0;

	v |= regs.carry_flag  << 0;
	v |= regs.zero_flag  << 1;
	v |= regs.interrupt_flag << 2;
	v |= regs.decimal_mode_flag << 3;

	v |= 1 << 4;
	/* unused, always set */
	v |= 1     << 5;
	v |= regs.overflow_flag  << 6;
	v |= regs.sign_flag  << 7;

	return v;
}

void CPU::CMP(uint16_t addr){

	uint8_t data = memory[addr];

	uint16_t t;
	t = regs.reg[regA] - data;
	regs.carry_flag = (t<0x100);
	
	t = t & 0xff;
	
	SET_ZF(t);
	SET_NF(t);

}

void CPU::BNE(uint8_t addr){

	uint16_t new_addr = (int8_t) addr + regs.PC;
	//cout<<"addr: "<<hex<<unsigned(new_addr)<<endl;

	if(!regs.zero_flag){
		cout<<"BNE to "<<hex<<unsigned(new_addr)<<endl;
		regs.PC = new_addr;
	}

}

void CPU::BEQ(uint8_t addr){

	uint16_t new_addr = (int8_t) addr + regs.PC;
	//cout<<"BEQ: "<<hex<<unsigned(new_addr)<<endl;

	if(regs.zero_flag) 
		regs.PC = new_addr;

}

void CPU::INY()
{
	regs.reg[regY] +=1;
	SET_ZF(regs.reg[regY]);
	SET_NF(regs.reg[regY]);
}

void CPU::INX()
{
	regs.reg[regX] +=1;
	SET_ZF(regs.reg[regX]);
	SET_NF(regs.reg[regX]);
}

void CPU::CP(register_name index, uint8_t v)
{
	uint16_t t;
	t = regs.reg[index] - v;
	regs.carry_flag = t < 0x100;
	t = t & 0xff;
	SET_ZF(t);
	SET_NF(t);
}

void CPU::DE(register_name index)
{
	regs.reg[index] +=1;
	SET_ZF(regs.reg[index]);
	SET_NF(regs.reg[index]);
}

//DEBUG
void CPU::dump_reg(){

	DEBUG_PRINT(endl<<endl);
	DEBUG_PRINT("---------REG STATUS-----------"<<endl);
	DEBUG_PRINT("RegA: 0x"<<hex<<unsigned(regs.reg[regA])<<endl);
	DEBUG_PRINT("RegX: 0x"<<hex<<unsigned(regs.reg[regX])<<endl);
	DEBUG_PRINT("RegY: 0x"<<hex<<unsigned(regs.reg[regY])<<endl);
	DEBUG_PRINT("PC  : 0x"<<hex<<unsigned(regs.PC)<<endl);
	DEBUG_PRINT("SP  : 0x"<<hex<<unsigned(regs.SP)<<endl);

	DEBUG_PRINT("------------------------------"<<endl);

	DEBUG_PRINT(endl<<endl);

}

bool CPU::decode(uint8_t opcode){

	//OR LD ST PH fatte

	uint16_t addr,tmp;

	switch(opcode){
		case 0x00:        //BRK
		DEBUG_PRINT("BRK"<<endl);
		return false;

		case 0x01:        //ORA (ind,X)		

			addr = indirect_X();			

			OR(regA,memory[addr]);
			break;

		case 0x05:      //ORA zpg
			addr = zero_page();
			OR(regA, memory[addr]);
			break;

		case 0x06:			//ASL
			break;
	    
	    //TODO
		case 0x08:			//PHP
			PUSH(flags());
			break;

		case 0x09:			//ORA imm
			addr = immediate();
			DEBUG_PRINT("ORA "<<hex<<unsigned(addr)<<endl);
			OR(regA,addr);
			break;

		case 0x0D:
			DEBUG_PRINT("ORA ABS"<<endl);
			addr = absolute();		//ORA ABS
			OR(regA,memory[addr]);
			break;
			
		case 0x0E:						//ASL ABS
			break;

		case 0x10:						//BPL(?)
			break;
		
		case 0x11:						//ORA (ind),Y

			addr = indirect_Y();
			OR(regA,memory[addr]);
			break;

		case 0x15:						//ORA zpg,X

			addr = zero_page(regX);
			OR(regA,memory[addr]);
			break;
		
		case 0x18:						//ORA zpg,X

			regs.carry_flag = false;
			break;

		case 0x1D:						//ORA abs,X

			addr = absolute(regX);
			OR(regA,memory[addr]);
			break;
		
		case 0x20:						//JSR 
			DEBUG_PRINT("JSR"<<endl);
			addr = absolute();
			JSR(addr);
			break;
		
		case 0x29:						//AND imm 
			addr = immediate();
			DEBUG_PRINT("AND "<<hex<<unsigned(addr)<<endl);
			AND(addr);
			break;
		
		case 0x48:						//PHA

			PUSH(regA);
			break;
		
		case 0x4C:						//JMP abs
			addr = absolute();
			DEBUG_PRINT("JMP to "<<hex<<unsigned(addr)<<endl);

			regs.PC = addr;
			break;
		case 0x60:						//RTS
			DEBUG_PRINT("RTS"<<endl);
			addr = (POP() + (POP() << 8)) + 1;
  			regs.PC = addr;
			break;

		case 0x6C:						//JMP (ind)

			tmp = absolute();

			addr = read_word(tmp);

			JMP(tmp);
			break;

		case 0x78:						//SEI
			DEBUG_PRINT("SEI"<<endl);
			regs.interrupt_flag = true;
			break;

	    case 0x81:						//STA (ind,X)
	    	DEBUG_PRINT("STA indirect X"<<endl);

	    	addr = indirect_X();
	    	ST(regA,addr);

	    	//memory[addr] = regs.reg[regA];
	    	break;

	    case 0x84:						//STY zpg
	    	DEBUG_PRINT("STY zpg"<<endl);

	    	addr = zero_page();
	    	ST(regY,addr);

	    	//memory[addr] = regs.reg[regA];
	    	break;

	    case 0x85:						//STA zpg
	    	DEBUG_PRINT("STA zpg"<<endl);

	    	addr = zero_page();
	    	ST(regA,addr);

	    	//memory[addr] = regs.reg[regA];
	    	break;

	    case 0x86:						//STX zpg
	    	DEBUG_PRINT("STX zpg"<<endl);

	    	addr = zero_page();
	    	ST(regX,addr);

	    	//memory[addr] = regs.reg[regA];
	    	break;

		case 0x88:						//DEY
			DEBUG_PRINT("DEY"<<endl);
			DE(regY);
			break;

	    case 0x8C:						//STY abs
	    	DEBUG_PRINT("STY abs"<<endl);

	    	addr = absolute();
	    	ST(regY,addr);

	    	//memory[addr] = regs.reg[regA];
	    	break;

	    case 0x8D:						//STA abs

	    	addr = absolute();
	    	DEBUG_PRINT("STA "<<hex<<unsigned(addr)<<endl);

	    	ST(regA,addr);
	    	break;

	    case 0x8E:						//STX abs
	    	DEBUG_PRINT("STX ABS"<<endl);

	    	addr = absolute();
	    	ST(regX,addr);
	    	break;

		case 0x91:						//STA (ind,x)
	    	DEBUG_PRINT("STA ind y"<<endl);

	    	addr = indirect_Y();
	    	ST(regA,addr);
	    	break;

		case 0x94:						//STA zpg,X
	    	DEBUG_PRINT("STY zero page x"<<endl);

	    	addr = zero_page(regX);
	    	ST(regY,addr);
	    	break;

		case 0x95:						//STA zpg,X
	    	DEBUG_PRINT("STA zero page x"<<endl);

	    	addr = zero_page(regX);
	    	ST(regA,addr);
	    	break;

		case 0x96:						//STA zpg,Y
	    	DEBUG_PRINT("STX zero page x"<<endl);

	    	addr = zero_page(regY);
	    	ST(regX,addr);
	    	break;
		
		case 0x99:						//STA abs,Y
	    	DEBUG_PRINT("STA abs y"<<endl);

	    	addr = absolute(regY);
	    	ST(regA,addr);
	    	break;
		
		case 0x9A:						//TXS
	    	DEBUG_PRINT("TXS"<<endl);
	    	regs.SP = regs.reg[regX];
	    	break;

		case 0x9D:						//STA abs,X
	    	DEBUG_PRINT("STA abs X"<<endl);

	    	addr = absolute(regX);
	    	ST(regA,addr);
	    	break;

		case 0xA0:						//LDY imm
			DEBUG_PRINT("LDY IMM"<<endl);
			addr = immediate();
			LD(regY,addr);
			break;

		case 0xA1:						//LDA (ind,X)
			DEBUG_PRINT("LDY IMM"<<endl);
			addr = indirect_X();
			LD(regA,memory[addr]);
			break;

		case 0xA2:						//LDX imm
			DEBUG_PRINT("LDX IMM"<<endl);
			addr = immediate();
			LD(regX,addr);
			break;

		case 0xA4:						//LDY zpg
			DEBUG_PRINT("LDY zero"<<endl);
			addr = zero_page();
			LD(regY,memory[addr]);
			break;

		case 0xA5:						//LDA zpg
			DEBUG_PRINT("LDA zero"<<endl);
			addr = zero_page();
			LD(regA,memory[addr]);
			break;

		case 0xA6:						//LDX zpg
			DEBUG_PRINT("LDX zero"<<endl);
			addr = zero_page();
			LD(regX,memory[addr]);
			break;

		case 0xA9:						//LDA imm
			addr = immediate();
			DEBUG_PRINT("LDA "<<hex<<unsigned(addr)<<endl);
			LD(regA,addr);
			break;

		case 0xAC:						//LDY abs
			DEBUG_PRINT("LOAD abs"<<endl);
			addr = absolute();
			LD(regY,memory[addr]);
			break;

		case 0xAD:						//LDA abs
			DEBUG_PRINT("LOAD abs"<<endl);
			addr = absolute();
			LD(regA,memory[addr]);
			break;

		case 0xAE:						//LDX abs
			DEBUG_PRINT("LOAD abs"<<endl);
			addr = absolute();
			LD(regX,memory[addr]);
			break;

		case 0xB1:						//LDA ind y
			DEBUG_PRINT("LOAD ind y"<<endl);
			addr = indirect_Y();
			LD(regA,memory[addr]);
			break;

		case 0xB4:						//LDY zpg,X
			DEBUG_PRINT("LOAD ind y"<<endl);
			addr = indirect_X();
			LD(regY,memory[addr]);
			break;

		case 0xB5:						//LDA zpg,X
			DEBUG_PRINT("LOAD ind y"<<endl);
			addr = indirect_X();
			LD(regA,memory[addr]);
			break;
				
		case 0xB6:						//LDX zpg,Y
			DEBUG_PRINT("LOAD ind y"<<endl);
			addr = indirect_Y();
			LD(regX,memory[addr]);
			break;

		case 0xB9:						//LDA abs,Y
			DEBUG_PRINT("LOAD abs y"<<endl);
			addr = absolute(regY);
			LD(regA,memory[addr]);
			break;

		case 0xBC:						//LDY abs,X
			DEBUG_PRINT("LOAD abs X"<<endl);
			addr = absolute(regX);
			LD(regY,memory[addr]);
			break;

		case 0xBD:						//LDA abs,X
			DEBUG_PRINT("LOAD abs X"<<endl);
			addr = absolute(regX);
			LD(regA,memory[addr]);
			break;

		case 0xBE:						//LDX abs,X
			DEBUG_PRINT("LOAD abs Y"<<endl);
			addr = absolute(regY);
			LD(regX,memory[addr]);
			break;
		
		case 0xC0:						//CPY imm
			DEBUG_PRINT("CPY"<<endl);
			addr = immediate();
			CP(regY,addr);
			break;	
		
		case 0xC8:						//INY
			DEBUG_PRINT("INY"<<endl);
			INY();
			break;

		case 0xCA:						//DEX
			DEBUG_PRINT("DEX"<<endl);
			DE(regX);
			break;

		case 0xE0:						//CPX imm
			DEBUG_PRINT("CPX"<<endl);
			addr = immediate();
			CP(regX,addr);
			break;	

		case 0xEA:						//NOP
			DEBUG_PRINT("NOP"<<endl);
			break;
		
		case 0xD0:						//CLD
			DEBUG_PRINT("BNE"<<endl);
			addr = immediate();
			BNE(addr);
			break;	
		
		case 0xD8:						//CLD
			DEBUG_PRINT("CLD"<<endl);
			regs.decimal_mode_flag = false;
			break;
		
		case 0xDD:						//CMP abs,X
			DEBUG_PRINT("CMP"<<endl);
			addr = absolute(regX);
			CMP(addr);
			break;
		
		case 0xE8:						//INX
			DEBUG_PRINT("INX"<<endl);
			INX();
			break;
		
		case 0xF0:						//BEQ rel
			DEBUG_PRINT("BEQ"<<endl);
			addr = immediate();
			BEQ(addr);
			break;
		
		default:
			DEBUG_PRINT("unimplemented: "<<hex<<unsigned(opcode)<<endl);
 	}

 	addr = 0;

  return true;

}