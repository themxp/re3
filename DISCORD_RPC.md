# discord rich presence for re3

discord integration to show your gta III gameplay status to friends

## what it shows

when you're playing, your discord status will display:

- **health** - your current health condition
- **money** - how much cash you have (hover over icon to see)
- **wanted level** - police stars when you're wanted
- **activity** - whether you're driving or exploring
- **play time** - how long you've been playing

## setup

### 1. get the discord rpc library

download from https://github.com/discord/discord-rpc

put the header & library files in the provided folders.

### 2. build the project

**windows:**
```cmd
premake-vs2019.cmd
```
then open it with visual studio

**linux/macos:**
```bash
premake5 gmake2
cd build
make config=release
```

**using cmake:**
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### 3. setup discord developer portal

upload images to your discord app:

- **app id**: `1451455359335141526`
- **large image key**: `rpc-image`
- **small image key**: `money-icon`

## how it works

your discord status updates every 5 seconds w/ your current game state automatically starts when you launch the game & stops when you close it

## troubleshooting

**discord not showing anything?**
- make sure discord is running
- check that your app id is correct
- verify image assets are uploaded w/ the right keys

**build errors?**
- confirm discord rpc library is in the right folder
- make sure you're using the correct library for your platform

**game crashes?**
- check that discord rpc library matches your system
- verify the app id is valid

## credits

- discord rpc sdk: https://github.com/discord/discord-rpc
- re3 project: https://github.com/GTAmodding/re3