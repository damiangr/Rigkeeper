# Rigkeeper

Rigkeeper is a plugin for saving a file with a DAW session. It was created to store the used Kemper Profiling Amplifier file for a given audio track. This is convenient when you later want to record an additional audio track using the same sound. Just copy or drag the saved file from the plug-in to Rig Manager. 


## Info

JUCE VST3/AU plugin for Windows and Mac OS X

## Installation

### VST3

copy Rigkeeper.vst3 to the directory:
Windows: C:\Program Files\Common Files\VST3
Mac OS X: Library/Audio/Plug-ins/VST3
Rarely used: Users/your username/Library/Audio/Plug-ins/VST3

### AU

Mac OS X: Library/Audio/Plug-ins/Components
Rarely used: Users/your username/Library/Audio/Plug-ins/Components

## Usage

### to plugin 

Inset plugin into track FX then just copy/paste RIG file from Rig Manager or desired folder into the plugin window.
You can also drag and drop files(partially working). 
Save the DAW session.

### from plugin

Use right mouse button to open copy menu and select copy . Paste file into Rig Manager or any other desired location . 
You can also drag and drop files(partially working).

## Build

Go to Rigkeeper\JUCE\extras\Projucer\Builds\
and build projucer with xvode or VisualStudio

Run Projucer and change Juce and Juce Modules  path in preferences to the one in Rigkeeper project.
Open Rigkeeper.jucer and buid the plugin.