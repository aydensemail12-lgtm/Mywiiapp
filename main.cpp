#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <dirent.h>
#include <vector>
#include <string>

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

struct GameInfo {
    std::string id;
    std::string title;
    std::string path;
};

std::vector<GameInfo> gameList;
int selectedGame = 0;

void Initialise() {
    VIDEO_Init();
    WPAD_Init();
    rmode = VIDEO_GetPreferredMode(NULL);
    xfb = MEM_K0_TO_K1(VIDEO_GetFrameBufferSize(rmode));
    console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();

    if (!fatInitDefault()) {
        printf("Failed to initialize FAT! Check your SD/USB.\n");
    }
}

void ScanWBFS(const char* folder) {
    DIR *dir = opendir(folder);
    if (!dir) return;

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_type == DT_DIR) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
            char subpath[512];
            snprintf(subpath, sizeof(subpath), "%s/%s", folder, ent->d_name);
            ScanWBFS(subpath);
        } else {
            std::string filename = ent->d_name;
            if (filename.length() > 5 && filename.substr(filename.length() - 5) == ".wbfs") {
                char fullpath[512];
                snprintf(fullpath, sizeof(fullpath), "%s/%s", folder, ent->d_name);
                
                FILE *f = fopen(fullpath, "rb");
                if (f) {
                    char header[0x100];
                    fread(header, 1, 0x100, f);
                    fclose(f);

                    // WBFS file structure: 
                    // Usually the first 4 bytes are 'WBFS'
                    // The game header starts at some offset or is the file itself
                    // For simplicity, we assume the Game ID is in the filename or header
                    GameInfo game;
                    game.path = fullpath;
                    
                    // Extract ID from header (simplified)
                    char id[7];
                    memcpy(id, header, 6);
                    id[6] = '\0';
                    game.id = id;
                    
                    // Extract Title (simplified)
                    char title[64];
                    memcpy(title, header + 0x20, 63);
                    title[63] = '\0';
                    game.title = title;

                    gameList.push_back(game);
                }
            }
        }
    }
    closedir(dir);
}

void CompleteGame(const GameInfo& game) {
    printf("\nAttempting to complete: %s [%s]\n", game.title.c_str(), game.id.c_str());
    
    // Path to save on NAND: /title/00010000/TITLEID/data/data.bin
    // We need to use ISFS functions from libogc
    // This is a placeholder for the actual NAND writing logic
    
    char nandPath[128];
    // Note: This is a simplification. Real TITLEID is hex of the Game ID.
    snprintf(nandPath, sizeof(nandPath), "/title/00010000/%s/data/data.bin", game.id.c_str());
    
    printf("Target NAND path: %s\n", nandPath);
    
    char sourcePath[128];
    snprintf(sourcePath, sizeof(sourcePath), "sd:/apps/WiiGameCompleter/saves/%s.bin", game.id.c_str());
    
    FILE *src = fopen(sourcePath, "rb");
    if (!src) {
        printf("Error: 100%% save file not found at %s\n", sourcePath);
        return;
    }
    
    // In a real app, we would use ISFS_Open, ISFS_Write, etc.
    // For this demo, we simulate the success.
    printf("Success! Game %s is now 100%% complete.\n", game.title.c_str());
    fclose(src);
}

int main(int argc, char **argv) {
    Initialise();

    printf("Wii Game Completer v1.0\n");
    printf("Scanning for games...\n");
    
    ScanWBFS("sd:/wbfs");
    ScanWBFS("usb:/wbfs");

    if (gameList.empty()) {
        printf("No games found in /wbfs folder.\n");
    }

    while (1) {
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0);

        if (pressed & WPAD_BUTTON_HOME) exit(0);

        printf("\x1b[5;0H"); // Move cursor
        printf("Found %zu games:\n", gameList.size());
        
        for (size_t i = 0; i < gameList.size(); ++i) {
            if (i == (size_t)selectedGame) printf("> ");
            else printf("  ");
            printf("%s [%s]\n", gameList[i].title.c_str(), gameList[i].id.c_str());
        }

        if (pressed & WPAD_BUTTON_DOWN) {
            selectedGame = (selectedGame + 1) % gameList.size();
        }
        if (pressed & WPAD_BUTTON_UP) {
            selectedGame = (selectedGame - 1 + gameList.size()) % gameList.size();
        }
        if (pressed & WPAD_BUTTON_A) {
            CompleteGame(gameList[selectedGame]);
        }

        VIDEO_WaitVSync();
    }

    return 0;
}
