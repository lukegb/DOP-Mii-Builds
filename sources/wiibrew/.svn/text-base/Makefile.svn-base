#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

include $(DEVKITPPC)/wii_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	$(notdir $(CURDIR))
BUILD		:=	build
SOURCES		:=	source
DATA		:=	data  
INCLUDES	:=  include

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------

CFLAGS	= -g -O2 -Wall $(MACHDEP) $(INCLUDE)
CXXFLAGS = $(CFLAGS) -std=gnu++0x

#LDFLAGS	=	-g $(MACHDEP) -Wl,-Map,$(notdir $@).map -Wl,--section-start,.init=0x80003F00
LDFLAGS        =       -g $(MACHDEP) -Wl,-Map,$(notdir $@).map

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:= -lwiiuse -lfat -lbte -logc -lm -lmxml

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(PORTLIBS)

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))
XMLFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.xml)))
PNGFILES	:=  $(foreach dir,%(DATA),$(notdir $(wildcard $(dir)/*.png)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(PNGFILES:.png=.png.o) \
					$(XMLFILES:.xml=.xml.o) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
					$(sFILES:.s=.o) $(SFILES:.S=.o)

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES), -iquote $(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD) \
					-I$(LIBOGC_INC)

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
					-L$(LIBOGC_LIB)

export OUTPUT	:=	$(CURDIR)/$(TARGET)
.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
#lets see what OS we are on and then create svnref file
UNAME := $(shell uname)
#and now make the build list

build:
ifeq ($(UNAME),Linux)
	chmod 777 ./tools/MakeSvnRev.sh
	chmod 777 ./tools/BuildType.sh
	chmod 777 ./*Build.sh
	./tools/MakeSvnRev.sh
else
	SubWCRev.exe "." "./templates/svnrev_template.h" "./include/svnrev.h"
endif
	@[ -d $@ ] || mkdir -p $@
	@./tools/BuildType.sh
	@./PreBuild.sh
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile 
	@./PostBuild.sh $(OUTPUT)

#---------------------------------------------------------------------------------
debug:
ifeq ($(UNAME),Linux)
	chmod 777 ./tools/MakeSvnRev.sh
	chmod 777 ./tools/BuildType.sh
	chmod 777 ./*Build.sh
	./tools/MakeSvnRev.sh
else
	SubWCRev.exe "." "./templates/svnrev_template.h" "./include/svnrev.h"
endif
	@[ -d build ] || mkdir -p build
	@./tools/BuildType.sh DEBUG
	@./PreBuild.sh
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile 
	@./PostBuild.sh $(OUTPUT)
#---------------------------------------------------------------------------------
netdebug:
ifeq ($(UNAME),Linux)
	chmod 777 ./tools/MakeSvnRev.sh
	chmod 777 ./tools/BuildType.sh
	chmod 777 ./*Build.sh
	./tools/MakeSvnRev.sh
else
	SubWCRev.exe "." "./templates/svnrev_template.h" "./include/svnrev.h"
endif
	@[ -d build ] || mkdir -p build
	./tools/BuildType.sh DEBUG NETDEBUG
	@./PreBuild.sh
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
	@./PostBuild.sh $(OUTPUT)
#---------------------------------------------------------------------------------
remake:
	@[ -d build ] || mkdir -p build		
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile 
#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).dol *.zip include/BuildType.h include/svnrev.h
#---------------------------------------------------------------------------------
run:
	/usr/bin/wiiload $(TARGET).dol		

runelf: debug
	/usr/bin/wiiload $(TARGET).elf

release:
	make clean
	make
	cp -f $(OUTPUT).dol "./hbc/apps/DOP-Mii: WiiBrew Edition/boot.dol"
	make -C hbc makezip

	
#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

#---------------------------------------------------------------------------------
# This rule links in binary data with the .xxx extension
#---------------------------------------------------------------------------------
%.certs.o: %.certs
	@echo $(notdir $<)
	$(bin2o)

%.sys.o: %.sys
	@echo $(notdir $<)
	$(bin2o)

%.dat.o: %.dat
	@echo $(notdir $<)
	$(bin2o)

%.tmd.o: %.tmd
	@echo $(notdir $<)
	$(bin2o)

%.tik.o: %.tik
	@echo $(notdir $<)
	$(bin2o)

%.xml.o: %.xml
	@echo $(notdir $<)
	$(bin2o)

%.png.o: %.png
	@echo $(notdir $<)
	$(bin2o)
#---------------------------------------------------------------------------------

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
