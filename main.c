
#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

static const char* TITLE_IDS[] = {
    "0004000000125500", // USA
    "0004000000125400", // EUR
    "0004000000125600"  // JPN
};

static char gTitlePath[128];
static char gActiveSeed[256] = "None";

bool pathExists(const char* p) {
    return access(p, F_OK) == 0;
}

bool detectGame(void) {
    for (int i = 0; i < 3; i++) {
        snprintf(gTitlePath, sizeof(gTitlePath), "/luma/titles/%s", TITLE_IDS[i]);
        if (pathExists(gTitlePath)) return true;
    }
    return false;
}

void ensureDir(const char* p) {
    mkdir(p, 0777);
}

void copyFile(const char* src, const char* dst) {
    FILE* in = fopen(src, "rb");
    FILE* out = fopen(dst, "wb");
    if (!in || !out) { if (in) fclose(in); if (out) fclose(out); return; }
    char buf[8192];
    size_t r;
    while ((r = fread(buf, 1, sizeof(buf), in)) > 0) fwrite(buf, 1, r, out);
    fclose(in); fclose(out);
}

void copyDir(const char* src, const char* dst) {
    ensureDir(dst);
    DIR* dir = opendir(src);
    if (!dir) return;
    struct dirent* ent;
    while ((ent = readdir(dir)) != NULL) {
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) continue;
        char s[512], d[512];
        snprintf(s, sizeof(s), "%s/%s", src, ent->d_name);
        snprintf(d, sizeof(d), "%s/%s", dst, ent->d_name);
        struct stat st;
        if (stat(s, &st) == 0 && S_ISDIR(st.st_mode)) copyDir(s, d);
        else copyFile(s, d);
    }
    closedir(dir);
}

void removeDir(const char* path) {
    DIR* dir = opendir(path);
    if (!dir) return;
    struct dirent* ent;
    while ((ent = readdir(dir)) != NULL) {
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) continue;
        char p[512];
        snprintf(p, sizeof(p), "%s/%s", path, ent->d_name);
        struct stat st;
        if (stat(p, &st) == 0 && S_ISDIR(st.st_mode)) {
            removeDir(p);
            rmdir(p);
        } else {
            remove(p);
        }
    }
    closedir(dir);
}

int main() {
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    printf("MM3D Randomizer Manager v1.0\n\n");

    if (!detectGame()) {
        printf("Majora's Mask 3D not detected.\n");
        printf("Ensure game + Luma LayeredFS are present.\n");
    } else {
        printf("Game detected at:\n%s\n\n", gTitlePath);
        printf("A: Install example seed\n");
        printf("B: Disable patch\n");
        printf("START: Exit\n");
    }

    while (aptMainLoop()) {
        hidScanInput();
        u32 k = hidKeysDown();
        if (k & KEY_A) {
            char romfsDst[256];
            snprintf(romfsDst, sizeof(romfsDst), "%s/romfs", gTitlePath);
            ensureDir(gTitlePath);
            removeDir(romfsDst);
            ensureDir(romfsDst);
            copyDir("sdmc:/3ds/MM3DRandoManager/seeds/Seed_Example/romfs", romfsDst);
            strcpy(gActiveSeed, "Seed_Example");
            printf("Installed: %s\n", gActiveSeed);
        }
        if (k & KEY_B) {
            char romfsDst[256];
            snprintf(romfsDst, sizeof(romfsDst), "%s/romfs", gTitlePath);
            removeDir(romfsDst);
            printf("Patch disabled.\n");
        }
        if (k & KEY_START) break;
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    gfxExit();
    return 0;
}
