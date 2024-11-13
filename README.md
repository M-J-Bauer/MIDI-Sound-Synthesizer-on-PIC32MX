# MIDI Sound Synthesizer on PIC32MX MCU
MIDI-controlled monophonic sound synthesizer application runs on a PIC32MX340 microcontroller.

# Overview

The "REMI" Synth Module was designed originally for use with electronic wind instrument (EWI) MIDI controllers. Provision of a standard 'MIDI IN' port allows the synth to be played by any MIDI controller, for example a MIDI keyboard. Using a low-cost USB-MIDI adapter/cable, the synth can be controlled by a computer running music software, for example a sequencer. 

# Features

    . High quality audio output: 40kHz sample rate, 32-bit precision DSP
    . High accuracy oscillator pitch for musical application
    . Dual wave-table sound synthesis with mix-ratio modulation (morphing)
    . Graphical user interface (2.5" monochrome GLCD, 128x64 pixels) - *optional*
    . Command-line interface (CLI) for setup and patching (using PC as terminal)
    . Instrument presets (8) selectable from GUI, CLI or MIDI input source
    . User-programmable synth patches and wave-table creator (using CLI)
    . Noise Generator and Noise Filter (for "pitched noise" effects)
    . Effect modulation by breath pressure (CC2) and/or modulation messages (CC1)
    . Filter with variable cutoff frequency and resonance, pitch tracking
    . Filter frequency control by Expression (CC2), Ampld Env, Mod'n (CC1) or LFO
    . Reverberation effect

A PIC32MX on-chip timer module is used to generate a PWM audio output signal. The PWM "DAC" has a resolution of 11 bits,
which gives adequate sound quality. For improved sound quality, an optional 12-bit SPI DAC chip (MCP4921) may be added.
Software DSP computations use 32-bit normalized fixed-point numbers with 20-bit fractional part, allowing the application
to run on 32-bit microcontrollers without hardware floating-point capability.

For details of the project, visit the author's web page: http://www.mjbauer.biz/Build_the_REMI_synth_mk2.htm

Note that variants of the REMI synth design exist using different hardware configurations requiring different firmware. 
For example, there is a variant based on a PIC32MX440 MCU and a synth "Lite" variant designed especially for the REMI 2 (EWI controller).
If you need firmware for any variant, please contact me via email: mjbauer@iprimus.com.au
