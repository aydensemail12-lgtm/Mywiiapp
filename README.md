# Wii Game Completer

A Wii Homebrew application to automatically complete your games to 100%.

## How to Use
1.  Copy the `WiiGameCompleter` folder to the `/apps/` directory on your SD card or USB drive.
2.  Place 100% completion save files (`data.bin` renamed to `[GAMEID].bin`) in the `/apps/WiiGameCompleter/saves/` folder.
    - Example: For Mario Kart Wii (RMCE01), the file should be `/apps/WiiGameCompleter/saves/RMCE01.bin`.
3.  Launch the app from the Homebrew Channel.
4.  Select your game from the list and press **A** to complete it.

## Requirements
- A Wii with the Homebrew Channel installed.
- Games stored in `.wbfs` format in the `/wbfs/` folder.
- `AHBPROT` must be enabled (included in the provided `meta.xml`).

## Disclaimer
This application modifies your Wii's internal storage (NAND). While it includes a backup mechanism, use it at your own risk. Always keep a backup of your important save data.
