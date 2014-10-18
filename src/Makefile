TARGET := main

SRC :=  main.cc \
	midi_reader.cc \
	keyboard_events_extractor.cc \
	utils.cc \
	music_player.cc \


OBJS := ${SRC:.cc=.o}

CXX = clang++

LIBS= -ltermbox -lrtmidi

ifeq ($(findstring clang,$(CXX)), clang)
  CXX_WARN_FLAGS ?= -Weverything \
		    -Wno-c++98-compat \
		    -Wno-c++98-compat-pedantic \
		    -Wno-c99-extensions \
		    -Wno-padded # remove padding warning
else
# for g++ 4.9
  CXX_WARN_FLAGS ?= \
	-Werror \
	-Wno-pedantic \
	-Wall \
	-Wextra \
	-Wformat=2 \
	-Wnonnull \
	-Winit-self \
	-Wmissing-include-dirs \
	-Wswitch-default \
	-Wswitch-enum \
	-Wuninitialized \
	-fstrict-aliasing \
	-Wstrict-overflow=5 \
	-Warray-bounds \
	-Wfloat-equal \
	-Wundef \
	-Wshadow \
	-Wcast-qual \
	-Wcast-align \
	-Wwrite-strings \
	-Wconversion \
	-Wsign-conversion \
	-Wmissing-declarations \
	-Wpacked \
	-Wredundant-decls \
	-Winvalid-pch \
	-Wlong-long \
	-Wvarargs \
	-Wvla \
	-funsafe-loop-optimizations \
	-Wunsafe-loop-optimizations \
	-Wdouble-promotion \
	-Wsuggest-attribute=pure \
	-Wsuggest-attribute=const \
	-Wsuggest-attribute=noreturn \
	-Wsuggest-attribute=format \
	-Wtrampolines \
	-Wzero-as-null-pointer-constant \
	-Wuseless-cast \
	-Wlogical-op \
	-Wvector-operation-performance \
	-Wabi \
	-Wctor-dtor-privacy \
	-Wnoexcept \
	-Weffc++ \
	-Wstrict-null-sentinel \
	-Wold-style-cast \
	-Woverloaded-virtual \
	-Wsign-promo

#	unfortunately g++ reports padding warnings with lambda's variable capturing
#	-Wpadded \

endif

BUILD ?= debug

ifeq ($(BUILD),release)
  CXXFLAGS ?= -std=c++11 -Werror -O3 ${CXX_WARN_FLAGS}
else
  CXXFLAGS ?= -std=c++11 -Werror -O0 -ggdb3 ${CXX_WARN_FLAGS}
endif

ifeq ($(GPROF),1)
  CXXFLAGS += -pg
endif

SCAN_BUILD = scan-build

all: ${TARGET}

${TARGET}: ${OBJS}
	${CXX} ${CXXFLAGS} -o ${TARGET} ${OBJS} ${LIBS}

%.o: %.cc
	${CXX} ${CXXFLAGS} ${INCLUDES} -MD -c -o "$@" "$<"
	 @cp $*.d $*.P; \
	   sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	       -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	   rm -f $*.d

scan-build:
	${SCAN_BUILD} -analyze-headers --use-c++=/usr/bin/clang++ --status-bugs --keep-going  make -B

clean:
	rm -f ${TARGET} ${OBJS} $(SRC:%.cc=$/%.P)

.PHONY: all clean

-include $(SRC:%.cc=$/%.P)
