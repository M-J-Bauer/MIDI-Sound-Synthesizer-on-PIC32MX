#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/mp_remi_synth_mk2.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/mp_remi_synth_mk2.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

ifdef SUB_IMAGE_ADDRESS

else
SUB_IMAGE_ADDRESS_COMMAND=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../Common/TimeDelay.c ../Drivers/EEPROM_drv.c ../Drivers/I2C_drv.c ../Drivers/SPI_drv.c ../Drivers/LCD_KS0108_drv.c ../Drivers/UART_drv.c ./kernel.c ./LCD_graphics_lib.c ./wave_table_creator.c ./MIDI_comms_lib.c ./console_cli.c ./pic32_low_level.c remi_synth_CLI.c remi_synth_GUI.c remi_synth_config.c remi_synth_data.c remi_synth_engine.c remi_synth_main.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/2108356922/TimeDelay.o ${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o ${OBJECTDIR}/_ext/1904510940/I2C_drv.o ${OBJECTDIR}/_ext/1904510940/SPI_drv.o ${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o ${OBJECTDIR}/_ext/1904510940/UART_drv.o ${OBJECTDIR}/kernel.o ${OBJECTDIR}/LCD_graphics_lib.o ${OBJECTDIR}/wave_table_creator.o ${OBJECTDIR}/MIDI_comms_lib.o ${OBJECTDIR}/console_cli.o ${OBJECTDIR}/pic32_low_level.o ${OBJECTDIR}/remi_synth_CLI.o ${OBJECTDIR}/remi_synth_GUI.o ${OBJECTDIR}/remi_synth_config.o ${OBJECTDIR}/remi_synth_data.o ${OBJECTDIR}/remi_synth_engine.o ${OBJECTDIR}/remi_synth_main.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/2108356922/TimeDelay.o.d ${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o.d ${OBJECTDIR}/_ext/1904510940/I2C_drv.o.d ${OBJECTDIR}/_ext/1904510940/SPI_drv.o.d ${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o.d ${OBJECTDIR}/_ext/1904510940/UART_drv.o.d ${OBJECTDIR}/kernel.o.d ${OBJECTDIR}/LCD_graphics_lib.o.d ${OBJECTDIR}/wave_table_creator.o.d ${OBJECTDIR}/MIDI_comms_lib.o.d ${OBJECTDIR}/console_cli.o.d ${OBJECTDIR}/pic32_low_level.o.d ${OBJECTDIR}/remi_synth_CLI.o.d ${OBJECTDIR}/remi_synth_GUI.o.d ${OBJECTDIR}/remi_synth_config.o.d ${OBJECTDIR}/remi_synth_data.o.d ${OBJECTDIR}/remi_synth_engine.o.d ${OBJECTDIR}/remi_synth_main.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/2108356922/TimeDelay.o ${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o ${OBJECTDIR}/_ext/1904510940/I2C_drv.o ${OBJECTDIR}/_ext/1904510940/SPI_drv.o ${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o ${OBJECTDIR}/_ext/1904510940/UART_drv.o ${OBJECTDIR}/kernel.o ${OBJECTDIR}/LCD_graphics_lib.o ${OBJECTDIR}/wave_table_creator.o ${OBJECTDIR}/MIDI_comms_lib.o ${OBJECTDIR}/console_cli.o ${OBJECTDIR}/pic32_low_level.o ${OBJECTDIR}/remi_synth_CLI.o ${OBJECTDIR}/remi_synth_GUI.o ${OBJECTDIR}/remi_synth_config.o ${OBJECTDIR}/remi_synth_data.o ${OBJECTDIR}/remi_synth_engine.o ${OBJECTDIR}/remi_synth_main.o

# Source Files
SOURCEFILES=../Common/TimeDelay.c ../Drivers/EEPROM_drv.c ../Drivers/I2C_drv.c ../Drivers/SPI_drv.c ../Drivers/LCD_KS0108_drv.c ../Drivers/UART_drv.c ./kernel.c ./LCD_graphics_lib.c ./wave_table_creator.c ./MIDI_comms_lib.c ./console_cli.c ./pic32_low_level.c remi_synth_CLI.c remi_synth_GUI.c remi_synth_config.c remi_synth_data.c remi_synth_engine.c remi_synth_main.c



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/mp_remi_synth_mk2.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=32MX340F512H
MP_LINKER_FILE_OPTION=
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/2108356922/TimeDelay.o: ../Common/TimeDelay.c  .generated_files/flags/default/7754b03aa9fdb0c314147c60c2d92f106c7e4f0e .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/2108356922" 
	@${RM} ${OBJECTDIR}/_ext/2108356922/TimeDelay.o.d 
	@${RM} ${OBJECTDIR}/_ext/2108356922/TimeDelay.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/2108356922/TimeDelay.o.d" -o ${OBJECTDIR}/_ext/2108356922/TimeDelay.o ../Common/TimeDelay.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o: ../Drivers/EEPROM_drv.c  .generated_files/flags/default/db98f1d955b063d003c61a201d3781523a68844b .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1904510940" 
	@${RM} ${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o.d 
	@${RM} ${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o.d" -o ${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o ../Drivers/EEPROM_drv.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1904510940/I2C_drv.o: ../Drivers/I2C_drv.c  .generated_files/flags/default/3273eabfd3a603a1b2f0431f7fd1bdff02c012aa .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1904510940" 
	@${RM} ${OBJECTDIR}/_ext/1904510940/I2C_drv.o.d 
	@${RM} ${OBJECTDIR}/_ext/1904510940/I2C_drv.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1904510940/I2C_drv.o.d" -o ${OBJECTDIR}/_ext/1904510940/I2C_drv.o ../Drivers/I2C_drv.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1904510940/SPI_drv.o: ../Drivers/SPI_drv.c  .generated_files/flags/default/567f77f26cf9ba8a6a816027256e5561d69422d3 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1904510940" 
	@${RM} ${OBJECTDIR}/_ext/1904510940/SPI_drv.o.d 
	@${RM} ${OBJECTDIR}/_ext/1904510940/SPI_drv.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1904510940/SPI_drv.o.d" -o ${OBJECTDIR}/_ext/1904510940/SPI_drv.o ../Drivers/SPI_drv.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o: ../Drivers/LCD_KS0108_drv.c  .generated_files/flags/default/7189fd03c5a33b50cf9e2a926d1e5f55c7b7c925 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1904510940" 
	@${RM} ${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o.d 
	@${RM} ${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o.d" -o ${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o ../Drivers/LCD_KS0108_drv.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1904510940/UART_drv.o: ../Drivers/UART_drv.c  .generated_files/flags/default/5408cef8a786dea11f9fbc0b4670b05543a4662e .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1904510940" 
	@${RM} ${OBJECTDIR}/_ext/1904510940/UART_drv.o.d 
	@${RM} ${OBJECTDIR}/_ext/1904510940/UART_drv.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1904510940/UART_drv.o.d" -o ${OBJECTDIR}/_ext/1904510940/UART_drv.o ../Drivers/UART_drv.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/kernel.o: ./kernel.c  .generated_files/flags/default/6b8ee6c0ca4fe401759ea81f5b9ca91b2db6c96c .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/kernel.o.d 
	@${RM} ${OBJECTDIR}/kernel.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/kernel.o.d" -o ${OBJECTDIR}/kernel.o ./kernel.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/LCD_graphics_lib.o: ./LCD_graphics_lib.c  .generated_files/flags/default/918b19f96218fc84117c2265c2aa9c068a6d8e8 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/LCD_graphics_lib.o.d 
	@${RM} ${OBJECTDIR}/LCD_graphics_lib.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/LCD_graphics_lib.o.d" -o ${OBJECTDIR}/LCD_graphics_lib.o ./LCD_graphics_lib.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/wave_table_creator.o: ./wave_table_creator.c  .generated_files/flags/default/b2590e4c7fd991e27efd0ed65ba6d01b730ab20f .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/wave_table_creator.o.d 
	@${RM} ${OBJECTDIR}/wave_table_creator.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/wave_table_creator.o.d" -o ${OBJECTDIR}/wave_table_creator.o ./wave_table_creator.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/MIDI_comms_lib.o: ./MIDI_comms_lib.c  .generated_files/flags/default/aff450f54c4bcb817211c0e54d591ea48143211b .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/MIDI_comms_lib.o.d 
	@${RM} ${OBJECTDIR}/MIDI_comms_lib.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/MIDI_comms_lib.o.d" -o ${OBJECTDIR}/MIDI_comms_lib.o ./MIDI_comms_lib.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/console_cli.o: ./console_cli.c  .generated_files/flags/default/b8c9e5d85206dfd5a67f221fd2ac7c2ea6daef08 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/console_cli.o.d 
	@${RM} ${OBJECTDIR}/console_cli.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/console_cli.o.d" -o ${OBJECTDIR}/console_cli.o ./console_cli.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/pic32_low_level.o: ./pic32_low_level.c  .generated_files/flags/default/8463446a05834b4413499bf93108f0dd976f3afa .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/pic32_low_level.o.d 
	@${RM} ${OBJECTDIR}/pic32_low_level.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/pic32_low_level.o.d" -o ${OBJECTDIR}/pic32_low_level.o ./pic32_low_level.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_CLI.o: remi_synth_CLI.c  .generated_files/flags/default/51b39532f73595b8e64f2522eebe623654713352 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_CLI.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_CLI.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/remi_synth_CLI.o.d" -o ${OBJECTDIR}/remi_synth_CLI.o remi_synth_CLI.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_GUI.o: remi_synth_GUI.c  .generated_files/flags/default/b680ea4f7da61bf1c60f2874989ceb0561cd007f .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_GUI.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_GUI.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/remi_synth_GUI.o.d" -o ${OBJECTDIR}/remi_synth_GUI.o remi_synth_GUI.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_config.o: remi_synth_config.c  .generated_files/flags/default/7ef9bc56de54397f215440b569409a4f173f75ea .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_config.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_config.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/remi_synth_config.o.d" -o ${OBJECTDIR}/remi_synth_config.o remi_synth_config.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_data.o: remi_synth_data.c  .generated_files/flags/default/22133d6d00ee4f068c90932def046744a9b41fe4 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_data.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_data.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/remi_synth_data.o.d" -o ${OBJECTDIR}/remi_synth_data.o remi_synth_data.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_engine.o: remi_synth_engine.c  .generated_files/flags/default/95904ea2e17de9e6f407b23bd77cd1c0b812428f .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_engine.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_engine.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/remi_synth_engine.o.d" -o ${OBJECTDIR}/remi_synth_engine.o remi_synth_engine.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_main.o: remi_synth_main.c  .generated_files/flags/default/442ba5cb0db03a46dfcb533a514bd2141c73c1ca .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_main.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_main.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/remi_synth_main.o.d" -o ${OBJECTDIR}/remi_synth_main.o remi_synth_main.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
else
${OBJECTDIR}/_ext/2108356922/TimeDelay.o: ../Common/TimeDelay.c  .generated_files/flags/default/2439fc99921674260b03ac0589c1d8428fd36662 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/2108356922" 
	@${RM} ${OBJECTDIR}/_ext/2108356922/TimeDelay.o.d 
	@${RM} ${OBJECTDIR}/_ext/2108356922/TimeDelay.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/2108356922/TimeDelay.o.d" -o ${OBJECTDIR}/_ext/2108356922/TimeDelay.o ../Common/TimeDelay.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o: ../Drivers/EEPROM_drv.c  .generated_files/flags/default/c69e54e16a053ea7b40870f8fcdff4b817cd8b2f .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1904510940" 
	@${RM} ${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o.d 
	@${RM} ${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o.d" -o ${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o ../Drivers/EEPROM_drv.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1904510940/I2C_drv.o: ../Drivers/I2C_drv.c  .generated_files/flags/default/1ffba283954c5c1912c9b6517167372935b58c92 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1904510940" 
	@${RM} ${OBJECTDIR}/_ext/1904510940/I2C_drv.o.d 
	@${RM} ${OBJECTDIR}/_ext/1904510940/I2C_drv.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1904510940/I2C_drv.o.d" -o ${OBJECTDIR}/_ext/1904510940/I2C_drv.o ../Drivers/I2C_drv.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1904510940/SPI_drv.o: ../Drivers/SPI_drv.c  .generated_files/flags/default/c85135eac116c372c73fe5fe590f3e1e449502dc .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1904510940" 
	@${RM} ${OBJECTDIR}/_ext/1904510940/SPI_drv.o.d 
	@${RM} ${OBJECTDIR}/_ext/1904510940/SPI_drv.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1904510940/SPI_drv.o.d" -o ${OBJECTDIR}/_ext/1904510940/SPI_drv.o ../Drivers/SPI_drv.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o: ../Drivers/LCD_KS0108_drv.c  .generated_files/flags/default/17298ef3805f79d3340ed4cb3e773a62b1a874da .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1904510940" 
	@${RM} ${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o.d 
	@${RM} ${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o.d" -o ${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o ../Drivers/LCD_KS0108_drv.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1904510940/UART_drv.o: ../Drivers/UART_drv.c  .generated_files/flags/default/eb2504a2920167d4d4412794caaffa4e7dfa7587 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1904510940" 
	@${RM} ${OBJECTDIR}/_ext/1904510940/UART_drv.o.d 
	@${RM} ${OBJECTDIR}/_ext/1904510940/UART_drv.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1904510940/UART_drv.o.d" -o ${OBJECTDIR}/_ext/1904510940/UART_drv.o ../Drivers/UART_drv.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/kernel.o: ./kernel.c  .generated_files/flags/default/1e53f30790b48d63391c677f68adc52119adfe3 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/kernel.o.d 
	@${RM} ${OBJECTDIR}/kernel.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/kernel.o.d" -o ${OBJECTDIR}/kernel.o ./kernel.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/LCD_graphics_lib.o: ./LCD_graphics_lib.c  .generated_files/flags/default/e00a745be1898f9a195eca37183438d2c4462a94 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/LCD_graphics_lib.o.d 
	@${RM} ${OBJECTDIR}/LCD_graphics_lib.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/LCD_graphics_lib.o.d" -o ${OBJECTDIR}/LCD_graphics_lib.o ./LCD_graphics_lib.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/wave_table_creator.o: ./wave_table_creator.c  .generated_files/flags/default/cbae38ccb7d98e514e3916e816c3fe88fd62ccfb .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/wave_table_creator.o.d 
	@${RM} ${OBJECTDIR}/wave_table_creator.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/wave_table_creator.o.d" -o ${OBJECTDIR}/wave_table_creator.o ./wave_table_creator.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/MIDI_comms_lib.o: ./MIDI_comms_lib.c  .generated_files/flags/default/2cd1a0ea5e164490685a19ebac7074d5a45139a4 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/MIDI_comms_lib.o.d 
	@${RM} ${OBJECTDIR}/MIDI_comms_lib.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/MIDI_comms_lib.o.d" -o ${OBJECTDIR}/MIDI_comms_lib.o ./MIDI_comms_lib.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/console_cli.o: ./console_cli.c  .generated_files/flags/default/6b5b86bd375b2f3618ac4a96711d214172af5e12 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/console_cli.o.d 
	@${RM} ${OBJECTDIR}/console_cli.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/console_cli.o.d" -o ${OBJECTDIR}/console_cli.o ./console_cli.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/pic32_low_level.o: ./pic32_low_level.c  .generated_files/flags/default/4210cbf2f6344dd580d7e17e372d2a1d6f62a918 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/pic32_low_level.o.d 
	@${RM} ${OBJECTDIR}/pic32_low_level.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/pic32_low_level.o.d" -o ${OBJECTDIR}/pic32_low_level.o ./pic32_low_level.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_CLI.o: remi_synth_CLI.c  .generated_files/flags/default/38ee3196ef4f905497d1d3fbf7ead5ec4c31afc8 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_CLI.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_CLI.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/remi_synth_CLI.o.d" -o ${OBJECTDIR}/remi_synth_CLI.o remi_synth_CLI.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_GUI.o: remi_synth_GUI.c  .generated_files/flags/default/47d4a2ec7b834a21f5f3da120c3d7101f17834a5 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_GUI.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_GUI.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/remi_synth_GUI.o.d" -o ${OBJECTDIR}/remi_synth_GUI.o remi_synth_GUI.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_config.o: remi_synth_config.c  .generated_files/flags/default/51cfae63146fa6d53ea472b41b6930906effbede .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_config.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_config.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/remi_synth_config.o.d" -o ${OBJECTDIR}/remi_synth_config.o remi_synth_config.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_data.o: remi_synth_data.c  .generated_files/flags/default/22008ae64226e011686aeac6edc35dfa695b49a5 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_data.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_data.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/remi_synth_data.o.d" -o ${OBJECTDIR}/remi_synth_data.o remi_synth_data.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_engine.o: remi_synth_engine.c  .generated_files/flags/default/ba6595fbc29106bc114e25e5336ee59ae591d116 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_engine.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_engine.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/remi_synth_engine.o.d" -o ${OBJECTDIR}/remi_synth_engine.o remi_synth_engine.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_main.o: remi_synth_main.c  .generated_files/flags/default/53c608688eb4d928a1d6ab4548c07d84d123963e .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_main.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_main.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/remi_synth_main.o.d" -o ${OBJECTDIR}/remi_synth_main.o remi_synth_main.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${DISTDIR}/mp_remi_synth_mk2.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -g   -mprocessor=$(MP_PROCESSOR_OPTION)  -o ${DISTDIR}/mp_remi_synth_mk2.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)      -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D=__DEBUG_D,--memorysummary,${DISTDIR}/memoryfile.xml -mdfp="${DFP_DIR}"
	
else
${DISTDIR}/mp_remi_synth_mk2.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mprocessor=$(MP_PROCESSOR_OPTION)  -o ${DISTDIR}/mp_remi_synth_mk2.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--memorysummary,${DISTDIR}/memoryfile.xml -mdfp="${DFP_DIR}"
	${MP_CC_DIR}\\xc32-bin2hex ${DISTDIR}/mp_remi_synth_mk2.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} 
	@echo Normalizing hex file
	@"C:/Program Files/Microchip/MPLABX/v6.00/mplab_platform/platform/../mplab_ide/modules/../../bin/hexmate" --edf="C:/Program Files/Microchip/MPLABX/v6.00/mplab_platform/platform/../mplab_ide/modules/../../dat/en_msgs.txt" ${DISTDIR}/mp_remi_synth_mk2.X.${IMAGE_TYPE}.hex -o${DISTDIR}/mp_remi_synth_mk2.X.${IMAGE_TYPE}.hex

endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${OBJECTDIR}
	${RM} -r ${DISTDIR}

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
