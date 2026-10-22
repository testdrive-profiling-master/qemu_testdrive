#-------------------------------------------------
# For build automation
#-------------------------------------------------
SRC_PATH		:= ./src
MAKEFLAGS		:= $(filter-out -j%,$(MAKEFLAGS))

SRCS			:= \
	$(wildcard $(SRC_PATH)/*.cpp) \
	$(wildcard $(SRC_PATH)/*.c)

all: $(SRCS)
	@scripts\build_qemu.bat -j $(NUMBER_OF_PROCESSORS) install

dep:
	@echo
