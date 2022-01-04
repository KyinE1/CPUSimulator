#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <algorithm>

class Simulator {
private:
	unsigned int memory[65536] = {};	// 8-bit storage locations.
	std::string addr = ""; 				// Address in binary.
	std::string opcode = ""; 			// Instruction in binary.
	bool flag = false;

	// Define PEP8 registers found in the CPU (A, X, PC, SP, IR):
	struct Registers {
		int accumulator : 16; 					// Result of ALU operation.
		unsigned int indexRegister : 16; 		// Array index.
		unsigned int programCounter : 16; 		// Addressing the program memory.
		unsigned int stackPointer : 16; 		// Addressing the runtime stack.
		unsigned int instructionRegister : 24; 	// The current program instruction.
	}; Registers reg;

public:
	/** Accessor and mutator methods. */
	void setAddr(std::string str) { addr = str; }
	void setOpcode(std::string str) { opcode = str; }
	std::string getAddr() { return addr; }
	std::string getOpcode() { return opcode; }

	/** Load instruction, increment program counter and perform
	 * 	instruction.
	 *
	 *	@param opcode: Instruction specifier.
	 *	@param addr: The address location for memory.
	 **/
	void executeInstruction(std::string opcode, std::string addr) {
		// Load instruction from memory (fetch address or PC register).
		if (flag)
			reg.instructionRegister = memory[reg.programCounter];

		// Set the registers to 0 for the first execution.
		else {
			setRegister();
			flag = true;
		}

		// Increment PC register.
		reg.programCounter += 1;

		// Stop execution.
		if (opcode == "00000000") { return; }

		// Bitwise invert r.
		else if (opcode == "00011000" || opcode == "00011001") {
			bool flag = false;

			if (opcode[7] == '0') { opcode = convertDecimal(reg.accumulator); reg.accumulator = 0; }
			else { opcode = convertDecimal(reg.indexRegister); reg.indexRegister = 0; flag = true; }

			// Alternatively use 2's compliment, flip the 0s and 1s and add 1. 
			if (opcode != "") {
				for (int i = 0; i < opcode.size(); i++) {
					if (opcode[i] == '1') opcode[i] = '0';
					else opcode[i] = '1';
				}
			}

			if (!flag) reg.accumulator = convertBinary(opcode);
			else reg.indexRegister = convertBinary(opcode);
		}

		// Arithmetic shift left r.
		else if (opcode == "00011100" || opcode == "00011101") {
			if (opcode[7] == '0') reg.accumulator = reg.accumulator << 1;
			else reg.indexRegister = reg.accumulator << 1;
		}

		// Arithmetic shift right r.
		else if (opcode == "00011110" || opcode == "00011111") {
			if (opcode[7] == '0') reg.accumulator = reg.accumulator >> 1;
			else reg.indexRegister = reg.accumulator >> 1;
		}

		// Rotate left r.
		else if (opcode == "00100000" || opcode == "00100001") {
			bool flag = false;

			if (opcode[7] == '1') { opcode = convertDecimal(reg.indexRegister); reg.indexRegister = 0; }
			else { opcode = convertDecimal(reg.accumulator); reg.accumulator = 0; flag = true; }

			if (opcode != "") {
				char temp = opcode[opcode.size() - 1];
				opcode[opcode.size() - 1] = opcode[0];
				opcode[0] = temp;

				if (flag) reg.accumulator = convertBinary(opcode);
				else reg.indexRegister = convertBinary(opcode);
			}
		}

		// Rotate right r.
		else if (opcode == "00100010" || opcode == "00100011") {
			bool flag = false;

			if (opcode[7] == '1') { opcode = convertDecimal(reg.indexRegister); reg.indexRegister = 0; }
			else { opcode = convertDecimal(reg.accumulator); reg.accumulator = 0; flag = true; }

			if (opcode != "") {
				char temp = opcode[0];
				opcode[0] = opcode[opcode.size() - 1];
				opcode[opcode.size() - 1] = temp;

				if (flag) reg.accumulator = convertBinary(opcode);
				else reg.indexRegister = convertBinary(opcode);
			}
		}

		// Decimal input.
		else if (opcode == "00110000" || opcode == "00110001") {
			int temp = 0;
			std::cout << "Enter an integer value: ";
			std::cin >> temp;
			memory[convertBinary(addr)] = temp;
		}

		// Decimal output.
		else if (opcode == "00111000" || opcode == "00111001") {
			std::cout << memory[convertBinary(addr)];
		}

		// Character input.
		else if (opcode == "01001000" || opcode == "01001001") {
			char temp;
			std::cout << "Enter a character: ";
			std::cin >> temp;
			memory[convertBinary(addr)] = temp;
		}

		// Character output.
		else if (opcode == "01010000" || opcode == "01010001") {
			std::cout << memory[convertBinary(addr)];
		}

		// Add to r.
		else if (opcode == "01110000" || opcode == "01111000" || opcode == "01111001" || opcode == "01110001") {
			// Add two numbers together and place in register.
			if (opcode[4] == '0') reg.accumulator += memory[convertBinary(addr)];
			else reg.indexRegister += memory[convertBinary(addr)];
		}

		// Subtract from r.
		else if (opcode == "10000000" || opcode == "10001000" || opcode == "10001001" || opcode == "10000001") {
			if (opcode[4] == '0') reg.accumulator -= memory[convertBinary(addr)];
			else reg.indexRegister -= memory[convertBinary(addr)];
		}

		// Bitwise AND to r.
		else if (opcode == "10010000" || opcode == "10011000" || opcode == "10011001" || opcode == "10010001") {
			bool flag = false;
			if (opcode[4] == '1') { opcode = convertDecimal(reg.indexRegister); reg.indexRegister = 0; }
			else { opcode = convertDecimal(reg.accumulator); reg.accumulator = 0; flag = true; }

			std::string bStr = "";
			std::cout << "Enter binary string of 1 byte to compare with (bitwise AND): ";
			std::cin >> bStr;

			// Set to 0 if both string indices are not equal to 1.
			for (int i = 0; i < opcode.size(); i++) {
				if (opcode[i] == '1' && bStr[i] == '1') continue;
				opcode[i] = '0';
			}

			if (flag) reg.accumulator = convertBinary(opcode);
			else reg.indexRegister = convertBinary(opcode);
		}

		// Bitwise OR to r.
		else if (opcode == "10100000" || opcode == "10101000" || opcode == "10101001" || opcode == "10100001") {
			bool flag = false;
			if (opcode[4] == '1') { opcode = convertDecimal(reg.indexRegister); reg.indexRegister = 0; }
			else { opcode = convertDecimal(reg.accumulator); reg.accumulator = 0; flag = true; }

			std::string bStr = "";
			std::cout << "Enter binary string of 1 byte to compare with (bitwise OR): ";
			std::cin >> bStr;

			// Set to 0 if both string indices are not equal to 1.
			for (int i = 0; i < opcode.size(); i++) {
				if (opcode[i] == '1' && bStr[i] == '1') continue;
				opcode[i] = '0';
			}

			if (flag) reg.accumulator = convertBinary(opcode);
			else reg.indexRegister = convertBinary(opcode);
		}

		// Load r from memory.
		else if (opcode == "11000000" || opcode == "11001000" || opcode == "11001001" || opcode == "11000001") {
			if (opcode[4] == '0') reg.accumulator = memory[convertBinary(addr)];
			else reg.indexRegister = memory[convertBinary(addr)];
		}

		// Load byte from memory.
		else if (opcode == "11010000" || opcode == "11011000" || opcode == "11011001" || opcode == "11010001") {
			reg.accumulator = memory[convertBinary(addr)];
		}

		// Store r into memory.
		else if (opcode == "11100000" || opcode == "11101000" || opcode == "11101001" || opcode == "11100001") {
			if (opcode[4] == '0') memory[convertBinary(addr)] = reg.accumulator;
			else memory[convertBinary(addr)] = reg.indexRegister;
		}

		// Store byte into memory.		
		else if (opcode == "11110000" || opcode == "11111000" || opcode == "11111001" || opcode == "11110001") {
			memory[convertBinary(addr)] = reg.accumulator;
		}

		else std::cout << "**Invalid opcode in executeInstruction method.\n";
	}

	/** Set the register values to 0. */
	void setRegister() {
		reg.accumulator = 0;
		reg.indexRegister = 0;
		reg.programCounter = 0;
		reg.stackPointer = 0;
		reg.instructionRegister = 0;
	}

	/** Break the hexidecimal string into the opcode and address.
	 *	Call convert functions for hex to binary.
	 *
	 *	@param input: Instruction and address.
	 **/
	void parseHex(std::string input) {
		// Set the instruction to opcode data member.
		std::string temp = input.substr(0, 2);
		setOpcode(temp);

		// Set the address to address data member
		temp = input.substr(2, 5);
		setAddr(temp);

		// Convert hex instructions and address to binary.
		setAddr(convertHex(getAddr()));
		setOpcode(convertHex(getOpcode()));
	}

	/** Convert the hex input into a binary instruction or
	 * 	memory address.
	 *
	 * 	@param instruction: The 6 Hexidecimal characters.
	 *	@return Binary string.
	 **/
	std::string convertHex(std::string hStr) {
		std::string bStr = "";
		unsigned int idx = 0;

		// Append bits to binary string.
		while (idx < hStr.size()) {
			if (hStr[idx] == '0') bStr += "0000";
			else if (hStr[idx] == '1') bStr += "0001";
			else if (hStr[idx] == '2') bStr += "0010";
			else if (hStr[idx] == '3') bStr += "0011";
			else if (hStr[idx] == '4') bStr += "0100";
			else if (hStr[idx] == '5') bStr += "0101";
			else if (hStr[idx] == '6') bStr += "0110";
			else if (hStr[idx] == '7') bStr += "0111";
			else if (hStr[idx] == '8') bStr += "1000";
			else if (hStr[idx] == '9') bStr += "1001";
			else if (hStr[idx] == 'A' || hStr[idx] == 'a') bStr += "1010";
			else if (hStr[idx] == 'B' || hStr[idx] == 'b') bStr += "1011";
			else if (hStr[idx] == 'C' || hStr[idx] == 'c') bStr += "1100";
			else if (hStr[idx] == 'D' || hStr[idx] == 'd') bStr += "1101";
			else if (hStr[idx] == 'E' || hStr[idx] == 'e') bStr += "1110";
			else if (hStr[idx] == 'F' || hStr[idx] == 'f') bStr += "1111";
			else std::cout << "\n**Invalid opcode or address in convertHex method.";
			idx++;
		}
		return bStr;
	}

	/** Convert the binary input into a decimal value.
	 *
	 *	@param bStr: The binary string.
	 *	@return The decimal value.
	 **/
	int convertBinary(std::string bStr) {
		std::string temp;
		int decimal = 0;
		int bit = 0;

		// Multiply bits that are 1 to base 10.
		for (int i = bStr.size() - 1, j = 0; i >= 0; i--, j++) {
			temp = bStr[j];
			bit = atoi(temp.c_str());
			decimal += bit * pow(2, i);
		}

		return decimal;
	}

	/** Convert the decimal input into a binary string.
	 *
	 *	@param bStr: The decimal value.
	 *	@return The binary string.
	 **/
	std::string convertDecimal(int decimal) {
		std::string bStr;

		// Convert integer to base 2.
		while (decimal != 0) {
			bStr += std::to_string(decimal % 2);
			decimal /= 2;
		}

		reverse(bStr.begin(), bStr.end());

		return bStr;
	}

	/** Print PEP8 machine register values. */
	void output() {
		std::cout << "\nRegisters:"
			<< "\n\t**Accumulator:          \t" << reg.accumulator
			<< "\n\t**Index Register:       \t" << reg.indexRegister
			<< "\n\t**Program Counter:      \t" << reg.programCounter
			<< "\n\t**Stack Pointer:        \t" << reg.stackPointer
			<< "\n\t**Instruction Register: \t" << reg.instructionRegister
			<< "\n\n";
	}
};

/**	Driver.
 *
 *	@param argc: Number of arguments.
 *	@param argv: User input (arguments).
 *	@note For an input file, the instructions
 *	should be included in a text file, one per line.
 *
 *	Ex input:
 *		30FFFE
 *		38FFFE
 **/
int main(int argc, const char* argv[]) {
	Simulator obj;

	// Check for input file.
	if (argc > 1) {
		std::string input;
		std::ifstream file;
		file.open(argv[1]);

		// Run the input file contents.
		if (file.is_open()) {
			while (getline(file, input)) {
				obj.parseHex(input);
				obj.executeInstruction(obj.getOpcode(), obj.getAddr());
				obj.output();
			}
			file.close();
		}

		else std::cout << "The file specified could not be opened.\n";
	}

	// User input during program instruction.
	while (argc <= 1) {
		std::string input;
		std::cout << "Enter an instruction and address of 6 Hex characters (-1 to exit): ";
		std::cin >> input;

		if (input == "-1")
			break;

		// Error checking.
		else if (input.size() < 6 || input.size() > 6) {
			std::cout << "Example of input for instruction and address: 51001A";
			break;
		}

		// Set the instruction and address. 
		obj.parseHex(input);

		// Run the user input.
		obj.executeInstruction(obj.getOpcode(), obj.getAddr());
		obj.output();
	}

	std::cout << "\nExiting...";
	return 0;
}
