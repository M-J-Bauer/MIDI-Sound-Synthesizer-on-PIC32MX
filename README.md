# MIDI-Sound-Synthesizer-on-PIC32MX
Monophonic MIDI-controlled Sound Synthesizer application runs on PIC32MX microcontroller, easily adaptable to other devices.

# Overview

The "REMI" Synth Module is a monophonic MIDI-controlled sound synthesizer designed primarily for use with electronic wind instument (EWI) MIDI controllers. Provision of a standard 'MIDI IN' port allows the synth to be played by any MIDI controller, for example a keyboard with classic MIDI output. Using a low-cost USB-MIDI adapter/cable, the synth can also be controlled by a computer running music software, for example a MIDI sequencer. 

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
which gives adequate sound quality. For improved sound quality, an optional 12-bit SPI DAC chip may be added.
Software DSP computations use 32-bit normalized fixed-point numbers with 20-bit fractional part, allowing the application
to run on 32-bit microcontrollers without hardware floating-point capability.

For details of the project, visit the author's web page: http://www.mjbauer.biz/Build_the_REMI_synth.htm
