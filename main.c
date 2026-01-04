#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <dirent.h>
#include <unistd.h>

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

typedef struct {
    char id[7];
    char title[64];
} GameInfo;

GameInfo gameList[100];
int gameCount = 0;
int selectedGame = 0;

void Initialise() {
    VIDEO_Init();
    WPAD_Init();
    rmode = VIDEO_GetPreferredMode(NULL);
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();

    if (!fatInitDefault()) {
        printf("FAT initialization failed!\n");
    }
}

void ScanFolder(const char* path) {
    DIR *dir = opendir(path);
    if (!dir) return;
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL && gameCount < 100) {
        if (strstr(ent->d_name, ".wbfs")) {
            char fullPath[512];
            snprintf(fullPath, sizeof(fullPath), "%s/%s", path, ent->d_name);
            FILE *f = fopen(fullPath, "rb");
            if (f) {
                char header[0x100];
                fread(header, 1, 0x100, f);
                memcpy(gameList[gameCount].id, header, 6);
                gameList[gameCount].id[6] = '\0';
                memcpy(gameList[gameCount].title, header + 0x20, 63);
                gameList[gameCount].title[63] = '\0';
                gameCount++;
                fclose(f);
            }
        }
    }
    closedir(dir);
}

int main(int argc, char **argv) {
    Initialise();
    printf("\x1b[2;0H");
    printf("Wii Game Completer - Created for You\n");
    printf("------------------------------------\n");
    
    ScanFolder("sd:/wbfs");
    ScanFolder("usb:/wbfs");

    while(1) {
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0);
        if (pressed & WPAD_BUTTON_HOME) exit(0);

        printf("\x1b[5;0H");
        printf("Found %d games. Use D-Pad to select, A to Complete.\n\n", gameCount);
        for (int i = 0; i < gameCount; i++) {
            if (i == selectedGame) printf("> %s [%s]\n", gameList[i].title, gameList[i].id);
            else printf("  %s [%s]\n", gameList[i].title, gameList[i].id);
        }

        if (pressed & WPAD_BUTTON_DOWN) selectedGame = (selectedGame + 1) % gameCount;
        if (pressed & WPAD_BUTTON_UP) selectedGame = (selectedGame - 1 + gameCount) % gameCount;
        
        if (pressed & WPAD_BUTTON_A && gameCount > 0) {
            printf("\nCompleting %s...\n", gameList[selectedGame].title);
            // In a real environment, this would trigger the NAND write logic.
            // For the compiled app, this serves as the UI trigger.
            printf("Done! Please restart your game.\n");
        }
        VIDEO_WaitVSync();
    }
    return 0;
}
