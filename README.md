# OpenEdits

![preview image v1.0.8-dev](screenshot.jpeg)

A 2D game block building inspired by Everybody Edits.
Code structure inspired by Minetest practices.

The project direction can be found in [doc/DIRECTION.md](doc/DIRECTION.md).

## Gameplay

**Hotkeys**

 * W/A/S/D or arrow keys: move player
 * Space: Jump
 * LMB: Place the selected block
 * RMB or Shift+LMB: Block eraser
 * Mouse scroll: Zoom in/out
     * Warning: Rendering performs pretty badly. Do not scroll out too far.
 * 1-9: Hotbar block selector
     * Use RMB to drag & drop a block from the selector into the hotbar
 * `/` or T or Enter: Open chat box
     * Enter: Submit
     * ESC: Cancel and close
     * Tab: Nickname autocompletion
     * Up/Down: Previous chat message / empty chat message
 * E: Toggle block selector
 * G: Toggle god mode
 * M: Toggle minimap
 * F1: Toggle debug information


**Chat commands**

 * See `/help`

Permission/player flag examples:

 * `/fset FOOBAR tmp-edit-draw` temporary edit access (until rejoin)
 * `/fset FOOBAR godmode` persistent god mode access (after the world is saved)
 * `/fdel FOOBAR owner` removes all persistent access except for "banned"
 * `/ffilter` lists all players with any specified flags


**Command line arguments**

 * `--version` outputs the current game version
 * `--unittest` runs the included tests to sanity check
 * `--server` starts a server-only instance without GUI
 * `--setrole USERNAME ROLE`
     * `ROLE` can be one of: `normal`, `moderator`, `admin`.
     * Can be executed while a server is already running.

**World import/export**

 * `*.eelvl` files in `worlds/imports/` are listed by the server as playable read-only worlds
     * Helpful level archive: <https://archive.offlinee.com/>
 * Clients may use `.export FILENAME` to export the current world to `worlds/exports/`
 * See `worlds/README.txt` for further information


## Available archives

### Linux

1. Extract the archive to any location
2. Run `AppRun.sh`
3. In case of issues: run with `gdb`. Debug symbols are included.

### Windows

1. Extract the archive to any location
2. Run the executable
3. In case of issues: use a debugger such as x64dbg


## Compiling

### Dependencies

 * CMake (cmake-gui recommended for desktops)
 * [Irrlicht-Mt](https://github.com/minetest/irrlicht) : GUI/rendering library
 * [enet](http://enet.bespin.org/) : networking library
 * SQLite3 : auth & world saving
 * Threads (pthread?)
 * zlib: world compression (including EELVL import/export)
 * OpenAL: sound (for GUI builds only)

Debian/Ubuntu:

	sudo apt install libenet-dev libopenal-dev libsqlite3-dev zlib1g-dev

Project compiling:

	cmake -B build

**Headless server compiling**

1. Install the required libraries
2. Put the IrrlichtMt headers (include directory) somewhere
3. `cmake -B build -DBUILD_CLIENT=0 -DIRRLICHTMT_BUILD_DIR="/path/to/irrlicht/include/"`
4. `cd build` -> build `make -j` -> start `./OpenEdits-server`

**Distributing**

	cd build
	make install
	bash ../REPO_NAME/misc/pack.sh


## Licenses

Code: LGPL 2.1+

**Imported code**

 * [minimp3](https://github.com/lieff/minimp3) (CC 0), used in GUI builds only
 * [SHA3IUF](https://github.com/brainhub/SHA3IUF) (MIT)


### Fonts

DejaVu Sans bitmaps (Bitstream Vera license, extended MIT)

 * Converted with https://github.com/kahrl/irrtum

### Images

Images that are not listed below were created by SmallJoker (CC BY 4.0).

DailyYouth (CC BY 3.0)

 * [`icon_chat.png`](https://www.iconfinder.com/icons/3643728/balloon_chat_conversation_speak_word_icon)

IconMarket (CC BY 3.0)

 * [`icon_minimap.png`](https://www.iconfinder.com/icons/6442794/compass_direction_discover_location_navigation_icon) (desaturated)

ZUMMACO (CC BY 3.0)

 * [`ìcon_leave.png`](https://www.iconfinder.com/icons/7030313/sign_out_ui_basic_logout_app_user_interface_ui_icon)

### Sounds

Sounds that are not listed below were created by SmallJoker (CC BY 4.0).

Piotr Barcz (CC 0)

 * [`piano_c4.mp3`](https://freepats.zenvoid.org/Piano/honky-tonk-piano.html)
