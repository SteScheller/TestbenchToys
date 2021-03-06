# Nandland Goboard binary creation and simulation using the IceStrom toolchain,
# Icarus Verilog and GTKWave
#

PROJECT_NAME = VGA
SOURCES = $(wildcard *.v)
SOURCES += $(wildcard *.sv)

BUILD_DIR = out

SYNTH = yosys
PNR = nextpnr-ice40
PACK = icepack
PROGRAMMER = iceprog
VERILOG_SIMULATOR = iverilog
WAVEFORM_VIEWER = gtkwave
VERILOG_CPP_COMPILER = verilator

NAME = $(PROJECT_NAME)
OUT = $(BUILD_DIR)

.PHONY: clean flash verilog_sim

default: $(OUT)/$(NAME).bin

$(OUT):
	@echo Creating build directory...
	@mkdir -p $(OUT)

$(OUT)/$(NAME).bin: $(OUT) $(SOURCES) constraints.pcf
	@echo synthesizing netlist...
	@echo
	$(SYNTH) -p "synth_ice40 -top $(NAME)_TLM -json $(OUT)/$(NAME).json" $(SOURCES)
	@echo
	@echo --------------------------------------------------------------------------------
	@echo place and route...
	@echo
	$(PNR) --hx1k --package vq100 --freq 25 --pcf constraints.pcf --asc $(OUT)/$(NAME).asc --json $(OUT)/$(NAME).json
	@echo
	@echo --------------------------------------------------------------------------------
	@echo generating binary image...
	@echo
	$(PACK) $(OUT)/$(NAME).asc $(OUT)/$(NAME).bin 
	@echo
	@echo --------------------------------------------------------------------------------

flash: $(OUT)/$(NAME).bin
	@echo flashing target...
	@echo
	$(PROGRAMMER) $(OUT)/$(NAME).bin

$(OUT)/$(NAME).out: $(NAME)_tb.v $(SOURCES)
	@echo translating verilog source...
	@echo
	$(VERILOG_SIMULATOR) -s $(NAME)_TB $(NAME)_tb.v $(SOURCES) -o $(OUT)/$(NAME).out

verilog_sim: $(OUT)/$(NAME).out
	@echo simulating verilog testbench...
	@echo
	(cd $(OUT) && ./$(NAME).out && $(WAVEFORM_VIEWER) dump.vcd)

verilator:
	@echo compiling cpp models...
	@echo
	$(VERILOG_CPP_COMPILER) -Wall -Wno-DECLFILENAME -Wno-fatal --top $(NAME)_TLM -cc --build $(SOURCES)

clean:
	@echo Cleaning up...
	rm -rf ./$(OUT)
	@echo Done!
