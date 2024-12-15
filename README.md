# HfMDD Video System

## Overview

This software is a rather extensive plugin for OBS Studio, to work in tandem with a slightly modified OBS build and a custom skin (which is included), entirely overriding the common OBS UI. It is custom-made for Carl Maria von Weber College of Music in Dresden, allowing members of the school to record themselves in the concert hall with multiple adjustable cameras.
Third party code licensing demands it to be open-sourced.

### How it works

The Camera Controls started as a fork of [obs-ptz](https://github.com/glikely/obs-ptz) by Grant Likely et al., substantially modified and extended to match client demands: Automatic preview layout for 1-4 cameras, switchable between single- and multi-camera view, both user-bound and global admin-provided presets, each now storing the state of all cameras, integrated record button and menu navigation. Grant's settings menu is heavily extended as well to allow for the complete removal of OBS' own menu bar.

The camera controls are only accessible after selecting a *booking*; these are used to organize and document usage, allow for stopping the recording automatically after the designated time, and help the admin solving scheduling conflicts. Users can organize their bookings via the booking manager, or create an instant booking to record right away.

Recordings are split by source using Exeldro's [obs-source-record](https://github.com/exeldro/obs-source-record), with the filters here being reloaded once a booking is selected; this also takes care of organizing the folder structure.
Recordings are stored in the school's Synology NAS; the Synology Drive API is used (via a Node script) to provide access to the recording files via link.

A mail system takes care of sending download links, warning the admin when conflicting dates are booked, and in turn, gives them the opportunity to inform affected users when a booking is modified and deleted, with a mail template modifiable right from the UI.

## Build & Run

Please note that this software has only ever been built, run, and tested on Windows 10 (64-bit). The build setups for MacOS and Linux are untouched remnants from the OBS plugin template this project started with. The plugin was made for OBS Studio 29.1.2, but should work fine with newer versions.

### Build Dependencies

CMake 3.24-3.26, PowerShell Core 7, current Microsoft Compiler and Windows SDK.
Also requires Node.js version 20.9.0 or above.

### Build

Run `.\.github\scripts\Build-Windows.ps1` from PowerShell.
Copy the `.dll` and `.pdb` files from `.\releaseRelWithDebInfo\obs-plugins\64bit` into `C:\...\obs-studio\obs-plugins\64bit`.

From the `node-scripts` folder, run `npm install`.
Copy the `node-scripts` folder into `C:\...\obs-studio\node-scripts`.

### OBS Setup

* Install these plugins:

  * [obs-source-record](https://github.com/exeldro/obs-source-record)
  * [DistroAV](https://github.com/DistroAV/DistroAV)

* Copy `Mondrian.qss` and the `Mondrian` folder into `...\AppData\Roaming\obs-studio\themes`. Please note that the internal styling of the widgets was made to accommodate the "DaxOT" font, which is not included here.

* Copy all files in `.\config` into `...\AppData\Roaming\obs-studio\plugin_config\hfmdd-video-system`.

* Log in with `admin.default` (password is empty), move along till camera controls, open the settings menu with the small button at the very bottom of the widget. Set all necessary properties. There is no encryption here for these config files, since it wasn't needed with the client's specific setup.
