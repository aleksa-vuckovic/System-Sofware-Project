DIR_INC = inc
DIR_SRC = src
DIR_BUILD = build

ASSEMBLER_SRC = Assembler.cpp Converter.cpp EquDirective.cpp Instruction.cpp InstructionTranslator.cpp Parser.cpp Pool.cpp
ASSEMBLER_SRC += RelocationTable.cpp Section.cpp SymbolTable.cpp
ASSEMBLER_SRC += AssemblerMain.cpp
ASSEMBLER_O = ${ASSEMBLER_SRC:.cpp=.o}

LINKER_SRC = Assembler.cpp Converter.cpp Linker.cpp Parser.cpp RelocationTable.cpp SymbolTable.cpp
LINKER_SRC += EquDirective.cpp Instruction.cpp InstructionTranslator.cpp Pool.cpp Section.cpp
LINKER_SRC += LinkerMain.cpp
LINKER_O = ${LINKER_SRC:.cpp=.o}

EMULATOR_SRC = Converter.cpp Emulator.cpp InstructionCode.cpp Memory.cpp Registers.cpp
EMULATOR_SRC += EmulatorMain.cpp
EMULATOR_O = ${EMULATOR_SRC:.cpp=.o}

TEST_SRC := Assembler.cpp Converter.cpp Emulator.cpp EquDirective.cpp Instruction.cpp InstructionCode.cpp InstructionTranslator.cpp Linker.cpp Memory.cpp Parser.cpp Pool.cpp Registers.cpp RelocationTable.cpp Section.cpp SymbolTable.cpp Test.cpp TestMain.cpp
TEST_O = ${TEST_SRC:.cpp=.o}

ASSEMBLER_INPUT = $(addprefix ${DIR_BUILD}/, ${ASSEMBLER_O})
LINKER_INPUT = $(addprefix ${DIR_BUILD}/, ${LINKER_O})
EMULATOR_INPUT = $(addprefix ${DIR_BUILD}/, ${EMULATOR_O})
TEST_INPUT = $(addprefix ${DIR_BUILD}/, ${TEST_O})

ALL_H = $(wildcard ${DIR_H}/*.h)

COMPILE_OPTIONS = -g -w -c
LINK_OPTIONS = -g -w

${DIR_BUILD}/%.o: ${DIR_SRC}/%.cpp ${ALL_H}
	g++ ${COMPILE_OPTIONS} -o $@ $<

asembler: ${ASSEMBLER_INPUT}
	g++ ${LINK_OPTIONS} -o $@ ${ASSEMBLER_INPUT}

linker: ${LINKER_INPUT}
	g++ ${LINK_OPTIONS} -o $@ ${LINKER_INPUT}

emulator: ${EMULATOR_INPUT}
	g++ ${LINK_OPTIONS} -o $@ ${EMULATOR_INPUT}

test: ${TEST_INPUT}
	g++ ${LINK_OPTIONS} -o $@ ${TEST_INPUT}
	
all: asembler linker emulator test

