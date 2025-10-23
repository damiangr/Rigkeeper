# Rigkeeper - JUCE Audio Plugin Copilot Instructions

## Project Overview
Rigkeeper is a JUCE-based VST3/AU plugin for managing Kemper Profiling Amplifier rig files in DAW sessions. It provides drag-and-drop file handling with clipboard integration for seamless workflow between DAWs and Rig Manager.

## Architecture & Key Components

### Core Plugin Structure (Standard JUCE Pattern)
- **PluginProcessor** (`Source/PluginProcessor.h/.cpp`): Main audio processor with state management
- **PluginEditor** (`Source/PluginEditor.h/.cpp`): GUI editor linking processor to custom component
- **Rigkeeper Component** (`Source/Rigkeeper.h`): Custom JUCE component handling all file operations

### Critical Design Patterns

#### State Management via JUCE Value System
```cpp
// Processor stores file data as Values, not direct strings
Value rigFileNameValue;
Value rigFileContentValue;
AudioProcessorValueTreeState apvts; // Parameter tree for DAW state
```
The plugin uses JUCE's Value/Listener pattern for bidirectional state sync between processor and editor.

#### File Handling Strategy
- Files stored as **base64-encoded strings** in plugin state (see `rigContentBase64` in Rigkeeper.h)
- **1MB file size limit** enforced (`FILE_SIZE_LIMIT` constant)
- Temporary files created in system temp directory for drag operations
- Platform-specific clipboard integration (Windows: `ClipboardUtils_win32.h`, Mac: incomplete `ClipboardUtils_mac.cpp`)

#### Custom Component Architecture
The `Rigkeeper` class inherits from multiple JUCE classes:
```cpp
class Rigkeeper : public Label, DragAndDropContainer, 
                  FileDragAndDropTarget, PopupMenu, SystemClipboard
```

## Build System & Dependencies

### JUCE Project Structure
- **Rigkeeper.jucer**: Main project file defining targets (VST3, AU) and module paths
- **JuceLibraryCode/**: Auto-generated JUCE integration code
- **JUCE/**: Complete JUCE framework as git submodule
- **Builds/**: Platform-specific build files (VS2019/2022, Xcode, Linux)

### Build Workflow
1. Build Projucer from `JUCE/extras/Projucer/Builds/`
2. Configure JUCE module paths in Projucer preferences
3. Open `Rigkeeper.jucer` and generate platform builds
4. Compile using platform-specific IDE

### Module Dependencies
Key JUCE modules: `juce_audio_processors`, `juce_gui_basics`, `juce_gui_extra`, `juce_graphics`

## Development Patterns

### Event Handling
- **Mouse events**: Left-click initiates drag, right-click shows context menu
- **File drops**: Validated against size limit before processing
- **Value changes**: Automatic sync between processor state and UI

### Error Handling
- File size validation with user alerts
- Graceful handling of clipboard operations
- Temporary file cleanup after drag operations

### Platform Considerations
- **Windows**: Full clipboard integration via COM/OLE (`ClipboardUtils_win32.h`)
- **macOS**: Incomplete clipboard implementation (needs Objective-C++ work)
- **Cross-platform**: JUCE handles most UI/file operations consistently

## Code Conventions
- JUCE naming conventions (CamelCase for classes, methods)
- Platform-specific code isolated in separate headers
- Extensive use of JUCE smart pointers (`std::unique_ptr`, `ScopedPointer`)
- DBG() macros for debug logging throughout

## Common Development Tasks
- **Adding new file formats**: Modify validation in `checkAllowedFileSize()`
- **UI changes**: Edit SVG background in `Source/Binary/background2.svg`
- **State persistence**: Add properties to `apvts.state` in processor
- **Cross-platform features**: Extend clipboard utilities for target platform