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
FINAL_IMAGE=${DISTDIR}/mp_remi_synth_mx440.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/mp_remi_synth_mx440.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
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
SOURCEFILES_QUOTED_IF_SPACED=../Common/TimeDelay.c ../Drivers/EEPROM_drv.c ../Drivers/I2C_drv.c ../Drivers/LCD_KS0108_drv.c ../Drivers/UART_drv.c ../Drivers/SPI_drv.c ./LCD_graphics_lib.c ./wave_table_creator.c ./MIDI_comms_lib.c ./console_cli.c ./remi_synth_main.c ./remi_synth_CLI.c ./remi_synth_GUI.c ./remi_synth_config.c ./remi_synth_data.c ./remi_synth_engine.c pic32_low_level.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/2108356922/TimeDelay.o ${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o ${OBJECTDIR}/_ext/1904510940/I2C_drv.o ${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o ${OBJECTDIR}/_ext/1904510940/UART_drv.o ${OBJECTDIR}/_ext/1904510940/SPI_drv.o ${OBJECTDIR}/LCD_graphics_lib.o ${OBJECTDIR}/wave_table_creator.o ${OBJECTDIR}/MIDI_comms_lib.o ${OBJECTDIR}/console_cli.o ${OBJECTDIR}/remi_synth_main.o ${OBJECTDIR}/remi_synth_CLI.o ${OBJECTDIR}/remi_synth_GUI.o ${OBJECTDIR}/remi_synth_config.o ${OBJECTDIR}/remi_synth_data.o ${OBJECTDIR}/remi_synth_engine.o ${OBJECTDIR}/pic32_low_level.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/2108356922/TimeDelay.o.d ${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o.d ${OBJECTDIR}/_ext/1904510940/I2C_drv.o.d ${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o.d ${OBJECTDIR}/_ext/1904510940/UART_drv.o.d ${OBJECTDIR}/_ext/1904510940/SPI_drv.o.d ${OBJECTDIR}/LCD_graphics_lib.o.d ${OBJECTDIR}/wave_table_creator.o.d ${OBJECTDIR}/MIDI_comms_lib.o.d ${OBJECTDIR}/console_cli.o.d ${OBJECTDIR}/remi_synth_main.o.d ${OBJECTDIR}/remi_synth_CLI.o.d ${OBJECTDIR}/remi_synth_GUI.o.d ${OBJECTDIR}/remi_synth_config.o.d ${OBJECTDIR}/remi_synth_data.o.d ${OBJECTDIR}/remi_synth_engine.o.d ${OBJECTDIR}/pic32_low_level.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/2108356922/TimeDelay.o ${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o ${OBJECTDIR}/_ext/1904510940/I2C_drv.o ${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o ${OBJECTDIR}/_ext/1904510940/UART_drv.o ${OBJECTDIR}/_ext/1904510940/SPI_drv.o ${OBJECTDIR}/LCD_graphics_lib.o ${OBJECTDIR}/wave_table_creator.o ${OBJECTDIR}/MIDI_comms_lib.o ${OBJECTDIR}/console_cli.o ${OBJECTDIR}/remi_synth_main.o ${OBJECTDIR}/remi_synth_CLI.o ${OBJECTDIR}/remi_synth_GUI.o ${OBJECTDIR}/remi_synth_config.o ${OBJECTDIR}/remi_synth_data.o ${OBJECTDIR}/remi_synth_engine.o ${OBJECTDIR}/pic32_low_level.o

# Source Files
SOURCEFILES=../Common/TimeDelay.c ../Drivers/EEPROM_drv.c ../Drivers/I2C_drv.c ../Drivers/LCD_KS0108_drv.c ../Drivers/UART_drv.c ../Drivers/SPI_drv.c ./LCD_graphics_lib.c ./wave_table_creator.c ./MIDI_comms_lib.c ./console_cli.c ./remi_synth_main.c ./remi_synth_CLI.c ./remi_synth_GUI.c ./remi_synth_config.c ./remi_synth_data.c ./remi_synth_engine.c pic32_low_level.c



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
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/mp_remi_synth_mx440.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=32MX440F256H
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
${OBJECTDIR}/_ext/2108356922/TimeDelay.o: ../Common/TimeDelay.c  .generated_files/flags/default/2a0214c2ea5f363c46796f63e1076bd83fd9e6c5 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/2108356922" 
	@${RM} ${OBJECTDIR}/_ext/2108356922/TimeDelay.o.d 
	@${RM} ${OBJECTDIR}/_ext/2108356922/TimeDelay.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/_ext/2108356922/TimeDelay.o.d" -o ${OBJECTDIR}/_ext/2108356922/TimeDelay.o ../Common/TimeDelay.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o: ../Drivers/EEPROM_drv.c  .generated_files/flags/default/c065066a269a96dc8f61f0e4fd3c1130337abc80 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1904510940" 
	@${RM} ${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o.d 
	@${RM} ${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o.d" -o ${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o ../Drivers/EEPROM_drv.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1904510940/I2C_drv.o: ../Drivers/I2C_drv.c  .generated_files/flags/default/b63bfbba439b48d2f357bc10bb5cc45ab69754e0 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1904510940" 
	@${RM} ${OBJECTDIR}/_ext/1904510940/I2C_drv.o.d 
	@${RM} ${OBJECTDIR}/_ext/1904510940/I2C_drv.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/_ext/1904510940/I2C_drv.o.d" -o ${OBJECTDIR}/_ext/1904510940/I2C_drv.o ../Drivers/I2C_drv.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o: ../Drivers/LCD_KS0108_drv.c  .generated_files/flags/default/1060b9a6a8786b3446348be7eaddabb9d919770 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1904510940" 
	@${RM} ${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o.d 
	@${RM} ${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o.d" -o ${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o ../Drivers/LCD_KS0108_drv.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1904510940/UART_drv.o: ../Drivers/UART_drv.c  .generated_files/flags/default/aafa31a79b8692ce19e84da7f468d836583ca9b3 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1904510940" 
	@${RM} ${OBJECTDIR}/_ext/1904510940/UART_drv.o.d 
	@${RM} ${OBJECTDIR}/_ext/1904510940/UART_drv.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/_ext/1904510940/UART_drv.o.d" -o ${OBJECTDIR}/_ext/1904510940/UART_drv.o ../Drivers/UART_drv.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1904510940/SPI_drv.o: ../Drivers/SPI_drv.c  .generated_files/flags/default/1e93e54ed63f2dde1d3d8607fe00fe9ee2b504fe .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1904510940" 
	@${RM} ${OBJECTDIR}/_ext/1904510940/SPI_drv.o.d 
	@${RM} ${OBJECTDIR}/_ext/1904510940/SPI_drv.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/_ext/1904510940/SPI_drv.o.d" -o ${OBJECTDIR}/_ext/1904510940/SPI_drv.o ../Drivers/SPI_drv.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/LCD_graphics_lib.o: ./LCD_graphics_lib.c  .generated_files/flags/default/fcb6dfb4507cea61347debe4f49884ca337a1b0d .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/LCD_graphics_lib.o.d 
	@${RM} ${OBJECTDIR}/LCD_graphics_lib.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/LCD_graphics_lib.o.d" -o ${OBJECTDIR}/LCD_graphics_lib.o ./LCD_graphics_lib.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/wave_table_creator.o: ./wave_table_creator.c  .generated_files/flags/default/b0d1a467463ce5b6b9420ae7387b2bc2460db880 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/wave_table_creator.o.d 
	@${RM} ${OBJECTDIR}/wave_table_creator.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/wave_table_creator.o.d" -o ${OBJECTDIR}/wave_table_creator.o ./wave_table_creator.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/MIDI_comms_lib.o: ./MIDI_comms_lib.c  .generated_files/flags/default/e385126087c482e369fb47c5e4ac2377938ad760 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/MIDI_comms_lib.o.d 
	@${RM} ${OBJECTDIR}/MIDI_comms_lib.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/MIDI_comms_lib.o.d" -o ${OBJECTDIR}/MIDI_comms_lib.o ./MIDI_comms_lib.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/console_cli.o: ./console_cli.c  .generated_files/flags/default/10b8b1b23bc6a35b44139f36cb5514f475fb94fe .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/console_cli.o.d 
	@${RM} ${OBJECTDIR}/console_cli.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/console_cli.o.d" -o ${OBJECTDIR}/console_cli.o ./console_cli.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_main.o: ./remi_synth_main.c  .generated_files/flags/default/93ad7196f3568b8657a86041033e59c8ca1f6e0d .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_main.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_main.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/remi_synth_main.o.d" -o ${OBJECTDIR}/remi_synth_main.o ./remi_synth_main.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_CLI.o: ./remi_synth_CLI.c  .generated_files/flags/default/6ff939e4d89263aaded874f5e56b032cfb42d494 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_CLI.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_CLI.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/remi_synth_CLI.o.d" -o ${OBJECTDIR}/remi_synth_CLI.o ./remi_synth_CLI.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_GUI.o: ./remi_synth_GUI.c  .generated_files/flags/default/947ab783e6b52f36eaf18c75c7157ed91aa39517 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_GUI.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_GUI.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/remi_synth_GUI.o.d" -o ${OBJECTDIR}/remi_synth_GUI.o ./remi_synth_GUI.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_config.o: ./remi_synth_config.c  .generated_files/flags/default/4ddc8d473ece5d9f340fcf8bc685dbc9957102 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_config.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_config.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/remi_synth_config.o.d" -o ${OBJECTDIR}/remi_synth_config.o ./remi_synth_config.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_data.o: ./remi_synth_data.c  .generated_files/flags/default/5d147bee6a70884bfa1263277c94cf8566d11361 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_data.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_data.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/remi_synth_data.o.d" -o ${OBJECTDIR}/remi_synth_data.o ./remi_synth_data.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_engine.o: ./remi_synth_engine.c  .generated_files/flags/default/c452a8dd6add5916124d65c439ae35468f1fa7a2 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_engine.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_engine.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/remi_synth_engine.o.d" -o ${OBJECTDIR}/remi_synth_engine.o ./remi_synth_engine.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/pic32_low_level.o: pic32_low_level.c  .generated_files/flags/default/118951710ae4aaef855766ae278fbdba20815509 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/pic32_low_level.o.d 
	@${RM} ${OBJECTDIR}/pic32_low_level.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/pic32_low_level.o.d" -o ${OBJECTDIR}/pic32_low_level.o pic32_low_level.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
else
${OBJECTDIR}/_ext/2108356922/TimeDelay.o: ../Common/TimeDelay.c  .generated_files/flags/default/2ce0ef0a0b675a7a8c06914ad65e4e582ff9411e .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/2108356922" 
	@${RM} ${OBJECTDIR}/_ext/2108356922/TimeDelay.o.d 
	@${RM} ${OBJECTDIR}/_ext/2108356922/TimeDelay.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/_ext/2108356922/TimeDelay.o.d" -o ${OBJECTDIR}/_ext/2108356922/TimeDelay.o ../Common/TimeDelay.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o: ../Drivers/EEPROM_drv.c  .generated_files/flags/default/6d11d79f554d383fd4db17411b2e998b8eb26511 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1904510940" 
	@${RM} ${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o.d 
	@${RM} ${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o.d" -o ${OBJECTDIR}/_ext/1904510940/EEPROM_drv.o ../Drivers/EEPROM_drv.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1904510940/I2C_drv.o: ../Drivers/I2C_drv.c  .generated_files/flags/default/68238e58f4c67ec4fd3b1bc3a3fcf2f6611120f .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1904510940" 
	@${RM} ${OBJECTDIR}/_ext/1904510940/I2C_drv.o.d 
	@${RM} ${OBJECTDIR}/_ext/1904510940/I2C_drv.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/_ext/1904510940/I2C_drv.o.d" -o ${OBJECTDIR}/_ext/1904510940/I2C_drv.o ../Drivers/I2C_drv.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o: ../Drivers/LCD_KS0108_drv.c  .generated_files/flags/default/ee950aa7021384d0874d65e7da7f531f862f2853 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1904510940" 
	@${RM} ${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o.d 
	@${RM} ${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o.d" -o ${OBJECTDIR}/_ext/1904510940/LCD_KS0108_drv.o ../Drivers/LCD_KS0108_drv.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1904510940/UART_drv.o: ../Drivers/UART_drv.c  .generated_files/flags/default/8f2fddfe8f2edd3769c6b7f90a7d20f8e77a6d67 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1904510940" 
	@${RM} ${OBJECTDIR}/_ext/1904510940/UART_drv.o.d 
	@${RM} ${OBJECTDIR}/_ext/1904510940/UART_drv.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/_ext/1904510940/UART_drv.o.d" -o ${OBJECTDIR}/_ext/1904510940/UART_drv.o ../Drivers/UART_drv.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/_ext/1904510940/SPI_drv.o: ../Drivers/SPI_drv.c  .generated_files/flags/default/fb5eb46a8c0c97f40889e6858c0221691b19d9d9 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1904510940" 
	@${RM} ${OBJECTDIR}/_ext/1904510940/SPI_drv.o.d 
	@${RM} ${OBJECTDIR}/_ext/1904510940/SPI_drv.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/_ext/1904510940/SPI_drv.o.d" -o ${OBJECTDIR}/_ext/1904510940/SPI_drv.o ../Drivers/SPI_drv.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/LCD_graphics_lib.o: ./LCD_graphics_lib.c  .generated_files/flags/default/bdfbe7d8c14df311af976e98445d4c89ea66c22a .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/LCD_graphics_lib.o.d 
	@${RM} ${OBJECTDIR}/LCD_graphics_lib.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/LCD_graphics_lib.o.d" -o ${OBJECTDIR}/LCD_graphics_lib.o ./LCD_graphics_lib.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/wave_table_creator.o: ./wave_table_creator.c  .generated_files/flags/default/d5d9f648ea9c0c68f60beb8905f9325426612e86 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/wave_table_creator.o.d 
	@${RM} ${OBJECTDIR}/wave_table_creator.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/wave_table_creator.o.d" -o ${OBJECTDIR}/wave_table_creator.o ./wave_table_creator.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/MIDI_comms_lib.o: ./MIDI_comms_lib.c  .generated_files/flags/default/8bd8198e4365675e9e49d49b044fa3c4360dea0e .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/MIDI_comms_lib.o.d 
	@${RM} ${OBJECTDIR}/MIDI_comms_lib.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/MIDI_comms_lib.o.d" -o ${OBJECTDIR}/MIDI_comms_lib.o ./MIDI_comms_lib.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/console_cli.o: ./console_cli.c  .generated_files/flags/default/a5fca327ae033ca5daa6864aa80766b72df414de .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/console_cli.o.d 
	@${RM} ${OBJECTDIR}/console_cli.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/console_cli.o.d" -o ${OBJECTDIR}/console_cli.o ./console_cli.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_main.o: ./remi_synth_main.c  .generated_files/flags/default/6a7abb0fad5f1c415e13930f98ff50199018cff1 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_main.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_main.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/remi_synth_main.o.d" -o ${OBJECTDIR}/remi_synth_main.o ./remi_synth_main.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_CLI.o: ./remi_synth_CLI.c  .generated_files/flags/default/250fe103a57ad74e43c5d5a797fd3f30c0004191 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_CLI.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_CLI.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/remi_synth_CLI.o.d" -o ${OBJECTDIR}/remi_synth_CLI.o ./remi_synth_CLI.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_GUI.o: ./remi_synth_GUI.c  .generated_files/flags/default/ceb63ab1cec6ef72ce64c870143d064f5459bc1f .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_GUI.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_GUI.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/remi_synth_GUI.o.d" -o ${OBJECTDIR}/remi_synth_GUI.o ./remi_synth_GUI.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_config.o: ./remi_synth_config.c  .generated_files/flags/default/d93f64803ca6c8bbeeb0fa3d65f8ecad9f4dcf28 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_config.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_config.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/remi_synth_config.o.d" -o ${OBJECTDIR}/remi_synth_config.o ./remi_synth_config.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_data.o: ./remi_synth_data.c  .generated_files/flags/default/88653feb0cbccc1927577b459de8b2086caa9600 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_data.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_data.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/remi_synth_data.o.d" -o ${OBJECTDIR}/remi_synth_data.o ./remi_synth_data.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/remi_synth_engine.o: ./remi_synth_engine.c  .generated_files/flags/default/5a9779a619e2c911f3d7b11bb31c453318422b5f .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/remi_synth_engine.o.d 
	@${RM} ${OBJECTDIR}/remi_synth_engine.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/remi_synth_engine.o.d" -o ${OBJECTDIR}/remi_synth_engine.o ./remi_synth_engine.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/pic32_low_level.o: pic32_low_level.c  .generated_files/flags/default/5e87a98a05830a8709e0b58c0833283ba5374df4 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/pic32_low_level.o.d 
	@${RM} ${OBJECTDIR}/pic32_low_level.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYNTH_MK3_MX440_MAM -MP -MMD -MF "${OBJECTDIR}/pic32_low_level.o.d" -o ${OBJECTDIR}/pic32_low_level.o pic32_low_level.c    -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${DISTDIR}/mp_remi_synth_mx440.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -g -mdebugger -D__MPLAB_DEBUGGER_PK3=1 -mprocessor=$(MP_PROCESSOR_OPTION)  -o ${DISTDIR}/mp_remi_synth_mx440.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)   -mreserve=data@0x0:0x1FC -mreserve=boot@0x1FC02000:0x1FC02FEF -mreserve=boot@0x1FC02000:0x1FC024FF  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D=__DEBUG_D,--defsym=__MPLAB_DEBUGGER_PK3=1,--defsym=_min_heap_size=2000,--no-code-in-dinit,--no-dinit-in-serial-mem,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml -mdfp="${DFP_DIR}"
	
else
${DISTDIR}/mp_remi_synth_mx440.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mprocessor=$(MP_PROCESSOR_OPTION)  -o ${DISTDIR}/mp_remi_synth_mx440.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=_min_heap_size=2000,--no-code-in-dinit,--no-dinit-in-serial-mem,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml -mdfp="${DFP_DIR}"
	${MP_CC_DIR}\\xc32-bin2hex ${DISTDIR}/mp_remi_synth_mx440.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} 
	@echo Normalizing hex file
	@"C:/Program Files/Microchip/MPLABX/v6.00/mplab_platform/platform/../mplab_ide/modules/../../bin/hexmate" --edf="C:/Program Files/Microchip/MPLABX/v6.00/mplab_platform/platform/../mplab_ide/modules/../../dat/en_msgs.txt" ${DISTDIR}/mp_remi_synth_mx440.X.${IMAGE_TYPE}.hex -o${DISTDIR}/mp_remi_synth_mx440.X.${IMAGE_TYPE}.hex

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
