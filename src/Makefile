TARGET_DIR := ../bin
TARGET := ${TARGET_DIR}/pianoterm

SRC :=  main.cc \
	midi_reader.cc \
	keyboard_events_extractor.cc \
	utils.cc \
	music_player.cc \
	signals_handler.cc \

OBJS := ${SRC:.cc=.o}

LIBS= -ltermbox -lrtmidi

ifeq ($(findstring clang,$(CXX)), clang)
  CXX_WARN_FLAGS ?= -Weverything \
		    -Wno-c++98-compat \
		    -Wno-c++98-compat-pedantic \
		    -Wno-c99-extensions \
		    -Wno-padded # remove padding warning

  SANITIZERS = \
	-fsanitize=address \
	-fsanitize=undefined \
	-fsanitize=integer

# -fsanitize=memory \
# -fsanitize=dataflow \
# -fsanitize=cfi \
# -fsanitize=safe-stack

# -fsanitize=thread #is incompatible with sanitize address
# -fsanitize=leak # leak sanitizer can't be used if either address or thread sanitizer is used

  HARDENING_FLAGS = \
	-fstack-protector-all \
	--param ssp-buffer-size=4 \
	-D_FORTIFY_SOURCE=2 \
	-fPIC \
	-fno-omit-frame-pointer

# -fno-omit-frame-pointer to help the address sanitiser

  LDFLAGS = \
	-pie \
	-z noexecstack \
	-z now\
	-z relro \
	-z nodlopen \
	-z nodump

else
#https://developers.redhat.com/blog/2018/03/21/compiler-and-linker-flags-gcc/
#https://developers.redhat.com/blog/2018/03/26/gnu-toolchain-update-2018/

# for g++ 4.9
  CXX_WARN_FLAGS_FOR_GCC_4.9 := \
	-Werror \
	-Wno-pedantic \
	-Wpointer-arith \
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

  CXX_WARN_FLAGS_FOR_GCC_5 := \
	${CXX_WARN_FLAGS_FOR_GCC_4.9} \
	-Wswitch-bool \
	-Wlogical-not-parentheses \
	-Wsizeof-array-argument \
	-Wbool-compare  \
	-Wodr \
	-Wsuggest-final-types \
	-Wsuggest-final-methods \
	-Wsuggest-override


  CXX_WARN_FLAGS_FOR_GCC_6 := \
	${CXX_WARN_FLAGS_FOR_GCC_5} \
	-Wnull-dereference -fdelete-null-pointer-checks \
	-Wduplicated-cond \
	-Wplacement-new=2 \
	-Wconditionally-supported \
	-fsized-deallocation \
	-Wsized-deallocation \

  CXX_WARN_FLAGS_FOR_GCC_7 := \
	${CXX_WARN_FLAGS_FOR_GCC_6} \
	-Wduplicated-branches \
	-Wrestrict \
	-Wregister \
	-Wdangling-else \
	-Walloc-size-larger-than=1073741824 \
	-Walloc-zero \
	-Walloca \
	-Wformat-overflow=2 \
	-Wformat-truncation=2 \
	-Wstringop-overflow=4 \
	-Waligned-new
#	-Walloc-size-larger-than=1073741824 \ # Allocating more than 1GiB is likely a bug


  GCC_VERSION := $(shell "${CXX}" -v 2>&1 | gawk '($$1 == "gcc") && ($$2 == "version") { sub(/\..*/, "", $$3) ; print $$3 ; exit }' )

  ifeq ($(shell [ "${GCC_VERSION}" -ge 7 ] && echo $$?),0)
	CXX_WARN_FLAGS := ${CXX_WARN_FLAGS_FOR_GCC_7}
  endif

  ifeq (${GCC_VERSION},6)
	CXX_WARN_FLAGS := ${CXX_WARN_FLAGS_FOR_GCC_6}
  endif

  ifeq (${GCC_VERSION},5)
	CXX_WARN_FLAGS := ${CXX_WARN_FLAGS_FOR_GCC_5}
  endif

  ifeq (${CXX_WARN_FLAGS},)
	CXX_WARN_FLAGS := ${CXX_WARN_FLAGS_FOR_GCC_4.9}
  endif

# sanitizers on g++5.2
  SANITIZERS = \
	-fsanitize=address \
	-fsanitize=undefined \
	-fno-sanitize-recover=all \
	-fsanitize=vptr

# -fcheck-pointer-bounds -mmpx # pointer checker is not supported with address sanitiser
# -fsanitize=thread #is incompatible with sanitize address
# -fsanitize=leak # leak sanitizer can't be used if either address or thread sanitizer is used

  HARDENING_FLAGS = \
	-fstack-protector-all \
	--param ssp-buffer-size=4 \
	-D_FORTIFY_SOURCE=2 \
	-fPIC \
	-fbounds-check \
	-fstack-check \
	-fchkp-first-field-has-own-bounds \
	-fchkp-flexible-struct-trailing-arrays \
	-fchkp-narrow-to-innermost-array \
	-fchkp-optimize \
	-fchkp-use-wrappers \
#	-fvtable-verify=preinit \
#  \
	# -D_GLIBCXX_DEBUG \
	# -D_GLIBCXX_DEBUG_PEDANTIC

# can't use these debugged GLIBCXX version with external libraries

  LDFLAGS = \
	-pie \
	-z noexecstack \
	-z now\
	-z relro \
	-z nodlopen \
	-z nodump
endif

BUILD ?= debug

CXXFLAGS ?= -std=c++11 -Werror -fno-rtti -fstrict-enums ${HARDENING_FLAGS} ${CXX_WARN_FLAGS} ${SANITIZERS} ${DEFINES}

ifeq ($(BUILD),release)
  CXXFLAGS += -O3 -flto
  CFLAGS += -O3 -flto
  LDFLAGS += -s
else
  CXXFLAGS += -O0 -ggdb3
endif

ifeq ($(GCOV),1)
  CXXFLAGS += --coverage
endif

ifeq ($(GPROF),1)
  CXXFLAGS += -pg
endif

ifeq ($(32_BITS),1)
  CXXFLAGS += -m32
endif


SCAN_BUILD = scan-build

all: ${TARGET}

${TARGET}: ${OBJS}
	-mkdir -p ${TARGET_DIR}
	${CXX} ${LDFLAGS} ${CXXFLAGS} -o ${TARGET} ${OBJS} ${LIBS}

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
