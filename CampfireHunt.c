// to let raygui run
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
float TextToFloat(const char *text);
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
float TextToFloat(const char *text) {
    return strtof(text, NULL);
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SCREENWIDTH 1200
#define SCREENHEIGHT 800
#define MARGIN (SCREENWIDTH / 5)
#define TIMELIMIT 360.0f
#define FRAMESIZE 192

//Player
#define PLAYER_MAX_LIFE 3
#define PLAYER_HEALTH 5
#define MAX_CHARACTER_TYPE 4

//Screen
typedef struct GameScreen{
    bool gameOver;
    bool pause;
    bool showMenu;
    bool changesinglecharacter;
    bool changemulticharacter;
    bool singleplayer;
    bool multiplayer;
    bool manual;
} Screen;

//Sound
typedef struct Sounds{
    Sound buttonclick;
    Sound WalkingSound;
    Sound SwordSound;
    Music gameplay_music;
    Music stage2_music;
    Music stage3_music;
    Sound MoneySound;
    Sound healsound;
    Sound monsterdead;
    Sound minusheart;
    Sound monmonsound;
    Sound gamecomplete;
    Sound gameoversound;
} SoundAsset;

//Stage
typedef struct GameStage{
    bool stage1;
    bool stage2;
    bool stage3;
} Stage;

typedef struct Player{
    Vector2 position;
    int life;
    int health;
    float lastAttack;
    int money_collected;
    int goblin_killed;
    bool isAttack;
    int stage;
} player;

typedef enum skin{
    warrior_red = 0,
    warrior_blue,
    warrior_yellow,
    warrior_purple
} CharacterType;

//Goblin
#define GOBLIN_HEALTH 4
#define MONSTER_INTERVAL 900 //frames; 15s

typedef struct Monster{
    Vector2 position;
    int health;
    int color;
    Vector2 velocity;
    bool attack;
    float lastAttack;
    bool dying;
    int deathFrame;
} monster;

typedef enum {
    goblin_red = 0,
    goblin_blue,
    goblin_yellow,
    goblin_purple
} GoblinType;

//Generate goblin
monster GenerateGoblinPosition(Texture2D GoblinAnimation[], Stage *gamestage) {
    monster goblinPos;
    goblinPos.position.x = MARGIN + rand() % (SCREENWIDTH - 2 * (MARGIN + 50) );
    goblinPos.position.y = MARGIN + rand() % (SCREENHEIGHT - 2 * (MARGIN + 50) );
    goblinPos.color = rand() % MAX_CHARACTER_TYPE;
    if (gamestage->stage2){
        goblinPos.health = GOBLIN_HEALTH;
    } else {
        goblinPos.health = GOBLIN_HEALTH + 1;
    }
    
    goblinPos.velocity.x = rand() % 3 - 1;
    goblinPos.velocity.y = rand() % 3 - 1;
    goblinPos.attack = false;
    goblinPos.dying = false;
    return goblinPos;
}

//Monmon
#define MONMON_HEALTH 3
#define MONMON_INTERVAL 1500 //frames; 25s
#define MONMON_AMOUNT 3

typedef struct Mon{
    Vector2 position;
    int health;
    int color;
    Vector2 velocity;
    bool dying;
    int deathFrame;
} monmon;

typedef enum {
    monmon_red = 0,
    monmon_blue,
    monmon_yellow,
    monmon_purple
} MonType;

//Generate monmon
monmon GenerateMonMonPosition(Texture2D MonmonAnimation[], Stage *gamestage) {
    monmon monmonPos;
    monmonPos.position.x = MARGIN + rand() % (SCREENWIDTH - 2 * (MARGIN + 50) );
    monmonPos.position.y = MARGIN + rand() % (SCREENHEIGHT - 2 * (MARGIN + 50) );
    monmonPos.color = rand() % MAX_CHARACTER_TYPE;
    monmonPos.health = MONMON_HEALTH;
    monmonPos.dying = false;
    if (gamestage->stage1){
        monmonPos.velocity.x = rand() % 3 - 1;
        monmonPos.velocity.y = rand() % 3 - 1;
    } else {
        monmonPos.velocity.x = (rand() % 3 - 1) * 1.5; 
        monmonPos.velocity.y = (rand() % 3 - 1) * 1.5;
    }

    return monmonPos;
}

//Map tiles
#define TILE_WIDTH 16
#define TILE_HEIGHT 16
#define MAP_WIDTH 85
#define MAP_HEIGHT 90

typedef struct {
    int x;
    int y;
    int tileIndex;
} sTile;

typedef struct CoordSize{
    int x;
    int y;
    int size;
} coordSize;

typedef struct MarginDeco{
    Texture2D tree;
    Texture2D bush;
    Texture2D tent1;
    Texture2D tent2;
    Texture2D fire;
    Texture2D wood;
    Texture2D pumpkin;
    Texture2D mushroom;
    Texture2D scarecrow;
} Deco;

typedef struct image{
    Texture2D heart;
    Texture2D money;
    Texture2D meat;
    Texture2D moneyIdle;
    Texture2D skull;
    Texture2D dead;
    Texture2D keyboard;
    Texture2D keyboard_single;
    Texture2D maptile;
} GameTexture;

//Meat 
#define MEAT_INTERVAL 2000 //33.3s
Vector2 GenerateMeatPosition(){
    Vector2 meatPos;
    meatPos.x = MARGIN + rand() % (SCREENWIDTH - 2 * MARGIN);
    meatPos.y = MARGIN + rand() % (SCREENHEIGHT - 2 * MARGIN);
    return meatPos;
}

//Money
#define MONEY_AMOUNT 10
#define GENERATE_INTERVAL 160 //2.6s
#define MONEY_EXPIRY 420 //7s
typedef struct Money{
    int x;
    int y;
    int expiry;
} moneyAtt;
//Generate money position
moneyAtt GenerateMoneyPosition() {
    moneyAtt moneyPos;
    moneyPos.x = MARGIN + rand() % (SCREENWIDTH - 2 * MARGIN);
    moneyPos.y = MARGIN + rand() % (SCREENHEIGHT - 2 * MARGIN);
    moneyPos.expiry = MONEY_EXPIRY;
    return moneyPos;
};

//Button Click Sound
bool GuiButtonWithSound(Sound buttonclick, Rectangle bounds, const char *text) {
    SetSoundVolume(buttonclick, 0.3f);
    bool clicked = GuiButton(bounds, text);
    if (clicked) {
        PlaySound(buttonclick);
    }
    return clicked;
}
bool GuiCheckboxWithSound(Sound buttonclick, Rectangle bounds, const char *text, bool *pointer) {
    bool clicked = GuiCheckBox(bounds, text, pointer);
    if (clicked) {
        PlaySound(buttonclick);
    }
    return clicked;
}

void ShuffleTile(int array[], int n) { //Use for shuffling the tiles
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

int ReadSingleHighscore() { 
    FILE *fptr = fopen("single.txt", "r"); 
    
    int score = 0; 
    int highscore = 0; 

    if (fptr == NULL) { 
        printf("Error opening file\n"); 
        return -1; 
    } else {
        while (fscanf(fptr, "%d", &score) != EOF) { 
            if (score > highscore) { 
                highscore = score; 
            } 
        } 
        fclose(fptr); 
    }
    return highscore; 
}

int ReadMultiHighscore() { 
    FILE *fptr = fopen("multi.txt", "r"); 
    
    int score = 0; 
    int highscore = 0; 

    if (fptr == NULL) { 
        printf("Error opening file\n"); 
        return -1; 
    } else {
        while (fscanf(fptr, "%d", &score) != EOF) { 
            if (score > highscore) { 
                highscore = score; 
            } 
        } 
        fclose(fptr); 
    }
    return highscore; 
}

void InitGame(float *remainingTime, Camera2D *singlecamera, sTile (*world)[MAP_HEIGHT], int numTiles, int *active_money, int *moneyloop, Screen *gamescreen, player *player1, player *player2, player *player3, int *goblin_spawn, int *goblinloop, int *meatloop, int *active_meat, Camera2D *multicamera1, Camera2D *multicamera2, int *monmon_spawn, bool *highscoreWritten, Stage *gamestage, int *monmonloop){
    *remainingTime = TIMELIMIT;
    *active_money = 0;
    *moneyloop = 0;
    *goblin_spawn = 0;
    *goblinloop = 0;
    *monmon_spawn = 0;
    *monmonloop = 0;
    *meatloop = 0;
    *active_meat = 0;
    gamestage->stage1 = false;
    gamestage->stage2 = false;
    gamestage->stage3 = false;
    
    //Initialize singleplayer
    player1->position = (Vector2){SCREENWIDTH/2, SCREENHEIGHT/2};
    player1->life = PLAYER_MAX_LIFE;
    player1->health = PLAYER_HEALTH;
    player1->money_collected = 0;
    player1->goblin_killed = 0;
    player1->stage = 1;

    //Initialize multiplayer
    player2->position = (Vector2){SCREENWIDTH/4, SCREENHEIGHT/2};
    player2->life = PLAYER_MAX_LIFE;
    player2->health = PLAYER_HEALTH;
    player2->money_collected = 0;
    player2->goblin_killed = 0;
    player2->stage = 1;
    player3->position = (Vector2){SCREENWIDTH*3/4, SCREENHEIGHT/2};
    player3->life = PLAYER_MAX_LIFE;
    player3->health = PLAYER_HEALTH;
    player3->money_collected = 0;
    player3->goblin_killed = 0;
    player3->stage = 1;

    int tileMap[MAP_WIDTH * MAP_HEIGHT];
    for (int i = 0; i < 100; i++) { //Dirt tile
        tileMap[i] = numTiles - 1;  
    }
    for (int i = 100; i < MAP_WIDTH * MAP_HEIGHT; i++) {
        tileMap[i] = GetRandomValue(0, numTiles - 2);  
    }

    ShuffleTile(tileMap, MAP_WIDTH * MAP_HEIGHT);

    int idx = 0;
    for (int i = 0; i < MAP_WIDTH; i++) {
        for (int j = 0; j < MAP_HEIGHT; j++) {
            world[i][j].x = i; 
            world[i][j].y = j; 
            world[i][j].tileIndex = tileMap[idx++];
        }
    }

    //Initialize Singleplayer Camera
    singlecamera->target = (Vector2){ 0, 0 };
    singlecamera->offset = (Vector2){SCREENWIDTH / 2, SCREENHEIGHT / 2};
    singlecamera->rotation = 0.0f;
    singlecamera->zoom = 3.0f;

    //Initialize Multiplayer Camera
    multicamera1->target = (Vector2){ 0, 0 };
    multicamera1->offset = (Vector2){SCREENWIDTH / 4, SCREENHEIGHT / 2};
    multicamera1->rotation = 0.0f;
    multicamera1->zoom = 3.0f;

    multicamera2->target = (Vector2){ 0, 0 };
    multicamera2->offset = (Vector2){SCREENWIDTH * 3 / 4, SCREENHEIGHT / 2};
    multicamera2->rotation = 0.0f;
    multicamera2->zoom = 3.0f;
    
    *highscoreWritten = false; 
}

void ChangeSingleCharacter(Sound buttonclick, Texture2D CharacterAnimation[], int *selected_character, Texture2D CharacterTexture, int action, int currentFrame, Screen *gamescreen, player *player1){
    DrawTexture(CharacterTexture, 0, 0, WHITE); 
    DrawRectangle(0, 0, SCREENWIDTH, SCREENHEIGHT, (Color){129, 133, 137, 30}); 

    DrawText("Choose Your Character", SCREENWIDTH/2 - MeasureText("Choose Your Character", 40)/2, 300, 40, (Color){130, 162, 159, 255});

    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT) || GuiButtonWithSound(buttonclick, (Rectangle){ SCREENWIDTH/2 + MeasureText("Choose Your Character", 40)/2, 500, 30, 30}, "#119#") || GetMouseWheelMove()>0) {
        (*selected_character)++;
    } else if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)|| GuiButtonWithSound(buttonclick, (Rectangle){ SCREENWIDTH/2 - MeasureText("Choose Your Character", 40)/2 - 30, 500, 30, 30}, "#118#") || GetMouseWheelMove()<0) {
        (*selected_character)--;
    }
    
    if (*selected_character >= MAX_CHARACTER_TYPE){
        *selected_character = 0;
    } else if (*selected_character < 0) {
        *selected_character = MAX_CHARACTER_TYPE - 1;
    }
    
    if (GuiButtonWithSound(buttonclick, (Rectangle){ SCREENWIDTH/2 - 95, 650, 200, 50}, "READY")) {
        gamescreen->changesinglecharacter = false;
        gamescreen->singleplayer = true;
    } else if (IsKeyPressed(KEY_ESCAPE) || GuiButtonWithSound(buttonclick, (Rectangle){ 30, 40, 120, 50}, "BACK")){
        gamescreen->changesinglecharacter = false;
        gamescreen->showMenu = true;
    }

    Rectangle source = (Rectangle){ currentFrame * FRAMESIZE, action * FRAMESIZE, FRAMESIZE, FRAMESIZE };
    Rectangle dest = { player1->position.x - 250, 270, 500, 500 };
    DrawTexturePro(CharacterAnimation[*selected_character], source, dest, (Vector2){ 0, 0 }, 0.0f, WHITE);
}

void ChangeMultiCharacter(Sound buttonclick, Texture2D CharacterAnimation[], int *selected_character1, int *selected_character2, Texture2D CharacterTexture, int action, int currentFrame, Screen *gamescreen){
    DrawTexture(CharacterTexture, 0, 0, WHITE); 
    DrawRectangle(0, 0, SCREENWIDTH, SCREENHEIGHT, (Color){129, 133, 137, 30}); 

    static bool player1ready = false;
    static bool player2ready = false;

    DrawText("Choose Your Character", SCREENWIDTH/2 - MeasureText("Choose Your Character", 40)/2, 300, 40, (Color){130, 162, 159, 255});

    //Player 1
    if (!player1ready){
        if (IsKeyPressed(KEY_D) || GuiButtonWithSound(buttonclick, (Rectangle){ SCREENWIDTH*3/8, 500, 30, 30}, "#119#")) {
            (*selected_character1)++;
        } else if (IsKeyPressed(KEY_A)|| GuiButtonWithSound(buttonclick, (Rectangle){ SCREENWIDTH/8, 500, 30, 30}, "#118#")) {
            (*selected_character1)--;
        }

        if (*selected_character1 >= MAX_CHARACTER_TYPE) {
            *selected_character1 = 0;
        } else if (*selected_character1 < 0) {
            *selected_character1 = MAX_CHARACTER_TYPE - 1;
        }
    }
    
    if (GuiCheckboxWithSound(buttonclick, (Rectangle){ SCREENWIDTH/4 - 50, 650, 20, 20}, " READY", &player1ready)){
        player1ready = true;
    } 

    //PLayer2
    if (!player2ready){
        if (IsKeyPressed(KEY_RIGHT) || GuiButtonWithSound(buttonclick,(Rectangle){ SCREENWIDTH*7/8, 500, 30, 30}, "#119#")) {
            (*selected_character2)++;
        } else if (IsKeyPressed(KEY_LEFT)|| GuiButtonWithSound(buttonclick, (Rectangle){ SCREENWIDTH*5/8, 500, 30, 30}, "#118#")) {
            (*selected_character2)--;
        }

        if (*selected_character2 >= MAX_CHARACTER_TYPE) {
            *selected_character2 = 0;
        } else if (*selected_character2 < 0) {
            *selected_character2 = MAX_CHARACTER_TYPE - 1;
        }
    }
    
    if (GuiCheckboxWithSound(buttonclick, (Rectangle){ SCREENWIDTH*3/4 - 50, 650, 20, 20}, " READY", &player2ready)){
        player2ready = true;
    } 

    //Character
    Rectangle source1 = (Rectangle){ currentFrame * FRAMESIZE, action * FRAMESIZE, FRAMESIZE, FRAMESIZE };
    Rectangle dest1 = { SCREENWIDTH/4 - 250, 270, 500, 500 };
    DrawTexturePro(CharacterAnimation[*selected_character1], source1, dest1, (Vector2){ 0, 0 }, 0.0f, WHITE); //Player 1
    
    Rectangle source2 = (Rectangle){ currentFrame * FRAMESIZE, action * FRAMESIZE, FRAMESIZE, FRAMESIZE };
    Rectangle dest2 = { SCREENWIDTH*3/4 - 250, 270, 500, 500 };
    DrawTexturePro(CharacterAnimation[*selected_character2], source2, dest2, (Vector2){ 0, 0 }, 0.0f, WHITE); //Player 2

    if (player1ready && player2ready) {
        gamescreen->changemulticharacter = false;
        gamescreen->multiplayer = true;
        player1ready = false;
        player2ready = false;
    } else if (IsKeyPressed(KEY_ESCAPE) || GuiButtonWithSound(buttonclick, (Rectangle){ 30, 40, 120, 50}, "BACK")){
        gamescreen->changemulticharacter = false;
        gamescreen->showMenu = true;
        player1ready = false;
        player2ready = false;
    }
}

void CollectMoney(Sound moneysound, int *active_money, moneyAtt moneyPosArray[], player *player) {
    for (int i = 0; i < *active_money; i++) {
        moneyPosArray[i].expiry--;
        if (player->position.x < moneyPosArray[i].x + 60 &&
            player->position.x > moneyPosArray[i].x &&
            player->position.y < moneyPosArray[i].y + 60 &&
            player->position.y > moneyPosArray[i].y) {
            for (int j = i; j < *active_money - 1; j++) {
                moneyPosArray[j] = moneyPosArray[j + 1];
            }
            (*active_money)--;
            (player->money_collected)++;
            PlaySound(moneysound); 
        }
        if (moneyPosArray[i].expiry <= 0) {
            for (int j = i; j < *active_money - 1; j++) {
                moneyPosArray[j] = moneyPosArray[j + 1];
            }
            (*active_money)--;
        }
    }
}

void CollectMeat(Sound healsound, player *player, Vector2 meat_post, int *active_meat, int *meatloop){
    if (*active_meat > 0 && (player->position.x < meat_post.x + 60 &&
        player->position.x > meat_post.x &&
        player->position.y < meat_post.y + 60 &&
        player->position.y > meat_post.y)) {
        *active_meat = 0;
        *meatloop = 0;
        PlaySound(healsound);
        player->health = PLAYER_HEALTH;
    }
}

int CheckGoblinCollision(player player, monster goblinPos) {
    return (player.position.x < goblinPos.position.x + 90 && //right
            player.position.x > goblinPos.position.x - 5 && //left
            player.position.y < goblinPos.position.y + 85 && //bottom
            player.position.y > goblinPos.position.y + 10); //top
}

void CollideGoblin(player *player, int *goblin_spawn, monster goblinPosArray[], Sound monsterdead) {
    for (int i = 0; i < *goblin_spawn; i++) {
        float currentTime = GetTime();
        if (CheckGoblinCollision(*player, goblinPosArray[i])){
            if (currentTime - goblinPosArray[i].lastAttack > 2.0f) { 
                goblinPosArray[i].attack = true;
                player->health -= 1;
                goblinPosArray[i].lastAttack = currentTime;
            }
            if (player->isAttack) {
                goblinPosArray[i].health -= 1; 
                player->lastAttack = currentTime;
            }
        }
        
        if (goblinPosArray[i].health <= 0 && !goblinPosArray[i].dying) {
            PlaySound(monsterdead);
            goblinPosArray[i].dying = true;
            goblinPosArray[i].attack = false;
            goblinPosArray[i].velocity.x = 0;
            goblinPosArray[i].velocity.y = 0;
            goblinPosArray[i].deathFrame = 0;
            (player->goblin_killed)++;
        }
    }
}

int CheckMonmonCollision(player player, monmon monmonPos) {
    return (player.position.x < monmonPos.position.x + 80 && //right
            player.position.x > monmonPos.position.x - 5 && //left
            player.position.y < monmonPos.position.y + 85 && //bottom
            player.position.y > monmonPos.position.y + 10); //top
}

void CollideMonmon(player *player, int *monmon_spawn, monmon monmonPosArray[], Sound monmonsound) {
    for (int i = 0; i < *monmon_spawn; i++) {
        float currentTime = GetTime();
        if (CheckMonmonCollision(*player, monmonPosArray[i])){
            if (player->isAttack) { 
                monmonPosArray[i].health -= 1;
                player->lastAttack = currentTime;
            }
        }
        if (monmonPosArray[i].health <= 0 && !monmonPosArray[i].dying) {
            monmonPosArray[i].dying = true;
            monmonPosArray[i].velocity.x = 0;
            monmonPosArray[i].velocity.y = 0;
            monmonPosArray[i].deathFrame = 0;
            (player->money_collected) += 3;
            PlaySound(monmonsound);
        }
    }
}

void UpdateObjects( Texture2D GoblinAnimation[], int *active_money, int *moneyloop, moneyAtt moneyPosArray[], Screen *gamescreen, int *goblin_spawn, int *goblinloop, monster goblinPosArray[], Vector2 *meat_post, int *meatloop, int *active_meat, int *monmonloop, int *monmon_spawn, monmon monmonPosArray[], Texture2D MonmonAnimation[], int goblin_amount, Stage *gamestage) {
    if (*moneyloop >= GENERATE_INTERVAL && *active_money < MONEY_AMOUNT) {
        moneyPosArray[*active_money] = GenerateMoneyPosition();
        (*active_money)++;
        *moneyloop = 0;
    }
    (*moneyloop)++;

    if (*monmonloop >= MONMON_INTERVAL && *monmon_spawn < MONMON_AMOUNT){
        monmonPosArray[*monmon_spawn] = GenerateMonMonPosition(MonmonAnimation, gamestage);
        (*monmon_spawn)++;
        *monmonloop = 0;
    }
    (*monmonloop)++;

    for (int i = 0; i < *monmon_spawn; i++){
        monmonPosArray[i].position.x += monmonPosArray[i].velocity.x;
        monmonPosArray[i].position.y += monmonPosArray[i].velocity.y;

        if (monmonPosArray[i].position.x < MARGIN || monmonPosArray[i].position.x > SCREENWIDTH - MARGIN - 100) {
            monmonPosArray[i].velocity.x *= -1;
        }
        if (monmonPosArray[i].position.y < MARGIN || monmonPosArray[i].position.y > SCREENHEIGHT - MARGIN - 100) {
            monmonPosArray[i].velocity.y *= -1;
        }
    }

    if (gamestage->stage2 || gamestage->stage3) {
        if (gamestage->stage3){
            goblin_amount = 8;
        } else {
            goblin_amount = 5;
        }

        goblinPosArray = realloc(goblinPosArray, goblin_amount * sizeof(monster));

        if (*meatloop >= MEAT_INTERVAL && !*active_meat){
            *meat_post = GenerateMeatPosition();
            *active_meat = 1;
        }
        (*meatloop)++;

        if (*goblinloop >= MONSTER_INTERVAL && *goblin_spawn < goblin_amount) {
            goblinPosArray[*goblin_spawn] = GenerateGoblinPosition(GoblinAnimation, gamestage);
            (*goblin_spawn)++;
            *goblinloop = 0;
        }
        (*goblinloop)++;

        for (int i = 0; i < *goblin_spawn; i++){
            goblinPosArray[i].position.x += goblinPosArray[i].velocity.x;
            goblinPosArray[i].position.y += goblinPosArray[i].velocity.y;

            if (goblinPosArray[i].position.x < MARGIN || goblinPosArray[i].position.x > SCREENWIDTH - MARGIN - 100) {
                goblinPosArray[i].velocity.x *= -1; 
            }
            if (goblinPosArray[i].position.y < MARGIN || goblinPosArray[i].position.y > SCREENHEIGHT - MARGIN - 100) {
                goblinPosArray[i].velocity.y *= -1; 
            }
        }
    }
}

void UpdateSingleplayerGame(SoundAsset loadsound, float *remainingTime, Texture2D GoblinAnimation[], Camera2D *singlecamera, int *active_money, int *moneyloop, moneyAtt moneyPosArray[], Screen *gamescreen, player *player1, int *goblin_spawn, int *goblinloop, monster goblinPosArray[], Vector2 *meat_post, int *meatloop, int *active_meat, int *monmonloop, int *monmon_spawn, monmon monmonPosArray[], Texture2D MonmonAnimation[], int goblin_amount, Stage *gamestage) {
    player1->isAttack = false;
    float currentTime = GetTime();
    if (!gamescreen->gameOver) {
        bool isWalking = false;
        if (IsKeyPressed(KEY_ESCAPE)) gamescreen->pause = !gamescreen->pause;
        if (!gamescreen->pause) {
            *remainingTime -= GetFrameTime();
            if (*remainingTime >= 300) {
                gamestage->stage1 = true;
            } else if (*remainingTime >= 120) {
                gamestage->stage1 = false;
                gamestage->stage2 = true;
                player1->stage = 2;
            } else if (*remainingTime > 0) {
                gamestage->stage2 = false;
                gamestage->stage3 = true;
                player1->stage = 3;
            } else {
                gamestage->stage3 = false; 
                *remainingTime = 0; 
                gamescreen->gameOver = true; 
            }

            UpdateObjects(GoblinAnimation, active_money, moneyloop, moneyPosArray, gamescreen, goblin_spawn, goblinloop, goblinPosArray, meat_post, meatloop, active_meat, monmonloop, monmon_spawn, monmonPosArray, MonmonAnimation, goblin_amount, gamestage);

            if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
                player1->position.x -= 1;
                isWalking = true;
            }
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
                player1->position.x += 1;
                isWalking = true;
            }
            if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
                player1->position.y -= 1;
                isWalking = true;
            }
            if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
                player1->position.y += 1;
                isWalking = true;
            }

            // Action sound
            if (isWalking && !IsSoundPlaying(loadsound.WalkingSound)) {
                PlaySound(loadsound.WalkingSound); 
            } 
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && (currentTime - player1->lastAttack > 0.8f)) {
                PlaySound(loadsound.SwordSound);
                player1->isAttack = true;
            }

            CollideGoblin(player1, goblin_spawn, goblinPosArray, loadsound.monsterdead);
            CollideMonmon(player1, monmon_spawn, monmonPosArray, loadsound.monmonsound);

            // The player is at the center of the screen
            singlecamera->target = player1->position;

            //Make sure only the map is display
            if (player1->position.x < MARGIN) {
                player1->position.x = MARGIN;
            } else if (player1->position.x > SCREENWIDTH - MARGIN) {
                player1->position.x = SCREENWIDTH - MARGIN;
            }
            if (player1->position.y < MARGIN) {
                player1->position.y = MARGIN;
            } else if (player1->position.y > SCREENHEIGHT - MARGIN) {
                player1->position.y = SCREENHEIGHT - MARGIN;
            }

            if (player1->health <= 0){
                PlaySound(loadsound.minusheart);
                player1->life -= 1;
                player1->health = PLAYER_HEALTH;
            }
            if (player1->life <= 0) {
                gamescreen->gameOver = true;
            }
        }
    } 
}

void UpdateMultiplayerGame(SoundAsset loadsound, float *remainingTime, Texture2D GoblinAnimation[], Camera2D *multicamera1, Camera2D *multicamera2, int *active_money, int *moneyloop, moneyAtt moneyPosArray[], Screen *gamescreen, player *player2, player *player3, int *goblin_spawn, int *goblinloop, monster goblinPosArray[], Vector2 *meat_post, int *meatloop, int *active_meat, int *monmonloop, int *monmon_spawn, monmon monmonPosArray[], Texture2D MonmonAnimation[], int goblin_amount, Stage *gamestage){
    player2->isAttack = false;
    player3->isAttack = false;
    float currentTime = GetTime();
    if (!gamescreen->gameOver){
        bool isWalking1 = false;
        bool isWalking2 = false;

        if (IsKeyPressed(KEY_ESCAPE)) gamescreen->pause = !gamescreen->pause;
        if(!gamescreen->pause){
            *remainingTime -= GetFrameTime();
            if (*remainingTime >= 300) {
                gamestage->stage1 = true;
            } else if (*remainingTime >= 120) {
                gamestage->stage1 = false;
                gamestage->stage2 = true;
                player2->stage = 2;
                player3->stage = 2;
            } else if (*remainingTime > 0) {
                gamestage->stage2 = false;
                gamestage->stage3 = true;
                player2->stage = 3;
                player3->stage = 3;
            } else {
                gamestage->stage3 = false; 
                *remainingTime = 0; 
                gamescreen->gameOver = true; 
            }

            UpdateObjects(GoblinAnimation, active_money, moneyloop, moneyPosArray, gamescreen, goblin_spawn, goblinloop, goblinPosArray, meat_post, meatloop, active_meat, monmonloop, monmon_spawn, monmonPosArray, MonmonAnimation, goblin_amount, gamestage);

            if (IsKeyDown(KEY_A)){
                player2->position.x -= 1;
                isWalking1 = true;
            }
            if (IsKeyDown(KEY_D)){
                player2->position.x += 1;
                isWalking1 = true;
            }
            if (IsKeyDown(KEY_W)){
                player2->position.y -= 1;
                isWalking1 = true;
            }
            if (IsKeyDown(KEY_S)){
                player2->position.y += 1;
                isWalking1 = true;
            }

            if (IsKeyDown(KEY_LEFT)){
                player3->position.x -= 1;
                isWalking2 = true;
            }
            if (IsKeyDown(KEY_RIGHT)){
                player3->position.x += 1;
                isWalking2 = true;
            }
            if (IsKeyDown(KEY_UP)){
                player3->position.y -= 1;
                isWalking2 = true;
            }
            if (IsKeyDown(KEY_DOWN)){
                player3->position.y += 1;
                isWalking2 = true;
            }

            //Action sound
            if ((isWalking1) && !IsSoundPlaying(loadsound.WalkingSound)){
                PlaySound(loadsound.WalkingSound); 
            }
            if (IsKeyDown(KEY_LEFT_SHIFT) && (currentTime - player2->lastAttack > 0.8f)) {
                PlaySound(loadsound.SwordSound);
                player2->isAttack = true;
            }
            if ((isWalking2) && !IsSoundPlaying(loadsound.WalkingSound)){
                PlaySound(loadsound.WalkingSound); 
            }
            if (IsKeyDown(KEY_RIGHT_SHIFT) && (currentTime - player3->lastAttack > 0.8f)) {
                PlaySound(loadsound.SwordSound);
                player3->isAttack = true;
            }

            CollideGoblin(player2, goblin_spawn, goblinPosArray, loadsound.monsterdead);
            CollideMonmon(player2, monmon_spawn, monmonPosArray, loadsound.monmonsound);
            CollideGoblin(player3, goblin_spawn, goblinPosArray, loadsound.monsterdead);
            CollideMonmon(player3, monmon_spawn, monmonPosArray, loadsound.monmonsound);

            // Camera
            multicamera1->target = player2->position;
            multicamera2->target = player3->position;

            //Make sure only the map is display
            if (player2->position.x < MARGIN) {
                player2->position.x = MARGIN;
            } else if (player2->position.x > SCREENWIDTH - MARGIN) {
                player2->position.x = SCREENWIDTH - MARGIN;
            }
            if (player2->position.y < MARGIN) {
                player2->position.y = MARGIN;
            } else if (player2->position.y > SCREENHEIGHT - MARGIN) {
                player2->position.y = SCREENHEIGHT - MARGIN;
            }

            if (player3->position.x < MARGIN) {
                player3->position.x = MARGIN;
            } else if (player3->position.x > SCREENWIDTH - MARGIN) {
                player3->position.x = SCREENWIDTH - MARGIN;
            }
            if (player3->position.y < MARGIN) {
                player3->position.y = MARGIN;
            } else if (player3->position.y > SCREENHEIGHT - MARGIN) {
                player3->position.y = SCREENHEIGHT - MARGIN;
            }

            if (player2->health <= 0){
                PlaySound(loadsound.minusheart);
                player2->life -= 1;
                player2->health = PLAYER_HEALTH;
            }
            if (player3->health <= 0){
                PlaySound(loadsound.minusheart);
                player3->life -= 1;
                player3->health = PLAYER_HEALTH;
            }
            if (player2->life <= 0 || player3->life <= 0){
                gamescreen->gameOver = true;
            }
        }
    }
}

void DrawMap(Deco deco, Texture2D GoblinAnimation[], int currentFrame, sTile (*world)[MAP_HEIGHT], Vector2 *tileIndices, int *active_money, moneyAtt moneyPosArray[], int *goblin_spawn, monster goblinPosArray[], int frameloop, Vector2 meat_post, int *active_meat, int *monmon_spawn, monmon monmonPosArray[], Texture2D MonmonAnimation[], GameTexture gametexture){
    //Map
    for (int i = 0; i < MAP_WIDTH; i++){
        for (int j = 0; j < MAP_HEIGHT; j++){
            sTile tile = world[i][j];
            //Select the tile index from the image
            int tile_index_x = tileIndices[tile.tileIndex].x;
            int tile_index_y = tileIndices[tile.tileIndex].y;

            Rectangle source = {tile_index_x * TILE_WIDTH, tile_index_y * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT};
            Rectangle dest = {tile.x * TILE_WIDTH, tile.y * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT};
            Vector2 origin = {100, 100};
            DrawTexturePro(gametexture.maptile, source, dest, origin, 0.0f, WHITE);
        }
    }

    //Margin Items
    Vector2 stump[] = { {20, 50}, {100, 120}, {140, 480}, {10, 450}, {630, 40}, {180, 10}, {1030, 330}, {900, 400}, {1030, 170}, {565, 490} };
    int numStump = sizeof(stump) / sizeof(stump[0]);
    for (int i = 0; i < numStump; i++){
        DrawTexturePro(deco.tree, (Rectangle) {0, 2 * FRAMESIZE, FRAMESIZE, FRAMESIZE}, (Rectangle) {stump[i].x, stump[i].y, FRAMESIZE, FRAMESIZE}, (Vector2){ 0, 0 }, 0.0f, WHITE);
    }

    Vector2 woods[] = { {740, 180}, {1025, 260}, {135, 570}, {1060, 500}, {590, 570} };
    int numWoods = sizeof(woods) / sizeof(woods[0]);
    for (int i = 0; i < numWoods; i++){
        DrawTexturePro(deco.wood, (Rectangle) {0, 0, deco.wood.width, deco.wood.height}, (Rectangle) {woods[i].x, woods[i].y, 80, 80}, (Vector2){ 0, 0 }, 0.0f, WHITE);
    }

    DrawTexturePro(deco.tent1, (Rectangle) {0, 0, deco.tent1.width, deco.tent1.height}, (Rectangle) {410, 140, deco.tent1.width/6, deco.tent1.height/6}, (Vector2){ 0, 0 }, 0.0f, WHITE);
    DrawTexturePro(deco.tent2, (Rectangle) {0, 0, deco.tent2.width, deco.tent2.height}, (Rectangle) {900, 120, deco.tent2.width/4, deco.tent2.height/4}, (Vector2){ 0, 0 }, 0.0f, WHITE);

    coordSize bushes1[] = { {370, 190, 70}, {550, 215, 40}, {1080, 340, 50}, {840, 190, 60}, {40, 520, 50} };
    int numBush1 = sizeof(bushes1) / sizeof(bushes1[0]);
    for (int i = 0; i < numBush1; i++) {
        DrawTexturePro(deco.bush, (Rectangle) {0, 0, deco.bush.width, deco.bush.height}, (Rectangle) {bushes1[i].x, bushes1[i].y, bushes1[i].size, bushes1[i].size}, (Vector2){ 0, 0 }, 0.0f, WHITE);
    }
    
    Vector2 trees2[] = { {550, 80}, {1050, 80}, {730, 0}, {950, 300} };
    int numTrees2 = sizeof(trees2) / sizeof(trees2[0]);
    for (int i = 0; i < numTrees2; i++){
        DrawTexturePro(deco.tree, (Rectangle){currentFrame * FRAMESIZE, 0, FRAMESIZE, FRAMESIZE}, (Rectangle){ trees2[i].x, trees2[i].y, FRAMESIZE, FRAMESIZE }, (Vector2){ 0, 0 }, 0.0f, WHITE);
    }

    Vector2 campfire[] = { {290, 180}, {980, 230}, {1000, 480}, {550, 580} };
    int numFire = sizeof(campfire) / sizeof(campfire[0]);
    for (int i = 0; i < numFire; i++){
        DrawTexturePro(deco.fire, (Rectangle) {currentFrame * 128, currentFrame * 128, 128, 128}, (Rectangle) {campfire[i].x, campfire[i].y, 70, 70}, (Vector2){ 0, 0 }, 0.0f, WHITE);
    }

    //Money
    for (int i = 0; i < *active_money; i++) {
        DrawTexturePro(gametexture.money, (Rectangle) {currentFrame * 128, currentFrame * 128, 128, 128}, (Rectangle) {moneyPosArray[i].x, moneyPosArray[i].y, 60, 60}, (Vector2){ 0, 0 }, 0.0f, WHITE);
    }

    //Meat
    if (*active_meat > 0){
        DrawTexturePro(gametexture.meat, (Rectangle) {currentFrame * 128, currentFrame * 128, 128, 128}, (Rectangle) {meat_post.x, meat_post.y, 60, 60}, (Vector2){ 0, 0 }, 0.0f, WHITE);
    }

    //Monmon
    for (int i = 0; i < *monmon_spawn; i++) {
        if (monmonPosArray[i].dying) {
            monmonPosArray[i].deathFrame = currentFrame % 14;
            if (currentFrame % 14 == 0 && monmonPosArray[i].deathFrame == 0) { 
                for (int j = i; j < *monmon_spawn - 1; j++) {
                    monmonPosArray[j] = monmonPosArray[j + 1];
                }
                (*monmon_spawn)--;
                i--;
            } else {
                DrawTexturePro(gametexture.dead, (Rectangle) {monmonPosArray[i].deathFrame * 128, monmonPosArray[i].deathFrame * 128, 128, 128}, (Rectangle) {monmonPosArray[i].position.x + 20, monmonPosArray[i].position.y + 20, 60, 60}, (Vector2){ 0, 0 }, 0.0f, WHITE);
            }
        } else {
            Rectangle source;
            if (monmonPosArray[i].velocity.x < 0) {
                source = (Rectangle) {currentFrame * FRAMESIZE, 5 * FRAMESIZE, -FRAMESIZE, FRAMESIZE};
            } else if (monmonPosArray[i].velocity.x == 0 && monmonPosArray[i].velocity.y == 0){
                source = (Rectangle) {currentFrame * FRAMESIZE, 4 * FRAMESIZE, FRAMESIZE, FRAMESIZE};
            } else {
                source = (Rectangle) {currentFrame * FRAMESIZE, 5 * FRAMESIZE, FRAMESIZE, FRAMESIZE};
            }
            DrawTexturePro(MonmonAnimation[monmonPosArray[i].color], source, (Rectangle) {monmonPosArray[i].position.x, monmonPosArray[i].position.y, 100, 100}, (Vector2){ 0, 0 }, 0.0f, WHITE);

            for (int j = 0; j < monmonPosArray[i].health; j++){
                DrawRectangle(monmonPosArray[i].position.x + 32 + 10 * j, monmonPosArray[i].position.y + 16, 10, 3, GREEN);
            }
        }
    }

    //Goblin
    for (int i = 0; i < *goblin_spawn; i++) {
        if (goblinPosArray[i].dying) {
            goblinPosArray[i].deathFrame = currentFrame % 14;
            if (currentFrame % 14 == 0 && goblinPosArray[i].deathFrame == 0) { 
                for (int j = i; j < *goblin_spawn - 1; j++) {
                    goblinPosArray[j] = goblinPosArray[j + 1];
                }
                (*goblin_spawn)--;
                i--;
            } else {
                DrawTexturePro(gametexture.dead, (Rectangle) {goblinPosArray[i].deathFrame * 128, goblinPosArray[i].deathFrame * 128, 128, 128}, (Rectangle) {goblinPosArray[i].position.x + 20, goblinPosArray[i].position.y + 20, 60, 60}, (Vector2){ 0, 0 }, 0.0f, WHITE);
            }
        } else {
            Rectangle source;
            if (goblinPosArray[i].attack){
                if (goblinPosArray[i].velocity.x < 0){
                    source = (Rectangle) {currentFrame * FRAMESIZE, 2 * FRAMESIZE, -FRAMESIZE, FRAMESIZE};
                } else {
                    source = (Rectangle) {currentFrame * FRAMESIZE, 2 * FRAMESIZE, FRAMESIZE, FRAMESIZE};
                }
                if (frameloop % 3 == 0) {   
                    goblinPosArray[i].attack = false;
                } 
            } else {
                if (goblinPosArray[i].velocity.x < 0) {
                    source = (Rectangle) {currentFrame * FRAMESIZE, 1 * FRAMESIZE, -FRAMESIZE, FRAMESIZE};
                } else if (goblinPosArray[i].velocity.x == 0 && goblinPosArray[i].velocity.y == 0){
                    source = (Rectangle) {currentFrame * FRAMESIZE, 0 * FRAMESIZE, FRAMESIZE, FRAMESIZE};
                } else {
                    source = (Rectangle) {currentFrame * FRAMESIZE, 1 * FRAMESIZE, FRAMESIZE, FRAMESIZE};
                }
            }
            DrawTexturePro(GoblinAnimation[goblinPosArray[i].color], source, (Rectangle) {goblinPosArray[i].position.x, goblinPosArray[i].position.y, 100, 100}, (Vector2){ 0, 0 }, 0.0f, WHITE);

            for (int j = 0; j < goblinPosArray[i].health; j++){
                DrawRectangle(goblinPosArray[i].position.x + 30 + 10 * j, goblinPosArray[i].position.y + 20, 10, 3, GREEN);
            }
        }
    }
}

void DrawMarginItemsFront(Deco deco, int currentFrame){
    Vector2 trees1[] = { {80, 60}, {-20, 160}, {58, 250}, {-5, 330}, {70, 400} };
    int numTrees1 = sizeof(trees1) / sizeof(trees1[0]);
    for (int i = 0; i < numTrees1; i++){
        DrawTexturePro(deco.tree, (Rectangle){currentFrame * FRAMESIZE, 0, FRAMESIZE, FRAMESIZE}, (Rectangle){ trees1[i].x, trees1[i].y, FRAMESIZE, FRAMESIZE }, (Vector2){ 0, 0 }, 0.0f, WHITE);
    }

    coordSize bushes2[] = { {270, 565, 70}, {320, 580, 80}, {485, 600, 40}, {700, 565, 70}, {810, 580, 50}, {730, 585, 90} };
    int numBush2 = sizeof(bushes2) / sizeof(bushes2[0]);
    for (int i = 0; i < numBush2; i++) {
        DrawTexturePro(deco.bush, (Rectangle){0, 0, deco.bush.width, deco.bush.height}, (Rectangle){bushes2[i].x, bushes2[i].y, bushes2[i].size, bushes2[i].size}, (Vector2){ 0, 0 }, 0.0f, WHITE);
    }

    coordSize pumpkins[] = { {420, 560, 70}, {280, 600, 70}, {400, 580, 60}, {445, 580, 50}, {920, 580, 70}, {910, 600, 60}, {945, 600, 50} };
    int numPumpkin = sizeof(pumpkins) / sizeof(pumpkins[0]);
    for (int i = 0; i < numPumpkin; i++) {
        int flip = (i % 2 == 0) ? 1 : -1;
        DrawTexturePro(deco.pumpkin, (Rectangle) {0, 0, flip * deco.pumpkin.width, deco.pumpkin.height}, (Rectangle) {pumpkins[i].x, pumpkins[i].y, pumpkins[i].size, pumpkins[i].size}, (Vector2){ 0, 0 }, 0.0f, WHITE);
    }

    DrawTexturePro(deco.scarecrow, (Rectangle) {0, 0, FRAMESIZE, FRAMESIZE}, (Rectangle) {990, 470, FRAMESIZE, FRAMESIZE}, (Vector2){ 0, 0 }, 20.0f, WHITE);

    Vector2 mushrooms[] = { {830, 610}, {1070, 625}, {50, 630} , {430, 620}, {320, 140}, {890, 120} };
    int numMushroom = sizeof(mushrooms) / sizeof(mushrooms[0]);
    for (int i = 0; i < numMushroom; i++) {
        int flip = (i % 2 == 0) ? -1 : 1;
        DrawTexturePro(deco.mushroom, (Rectangle) {0, 0, flip * deco.mushroom.width, deco.mushroom.height}, (Rectangle) {mushrooms[i].x, mushrooms[i].y, 60, 60}, (Vector2){ 0, 0 }, 0.0f, WHITE);
    }
}

int CalculateScore(player *player, float *remainingTime){
    int calc_score = 0;
    calc_score += player->goblin_killed * 500; 
    calc_score += player->money_collected * 100;
    if (player->stage == 2){
        calc_score += 100;
        calc_score += player->life * 100;
    } else if (player->stage == 3){
        calc_score += 500;
        calc_score += player->life * 300;
    }
    
    if (*remainingTime <= 0){
        calc_score += 500;
    }

    return calc_score;
}

void WriteSingleHighscore(int score){
    FILE *fptr = fopen("single.txt", "a");
    if (fptr == NULL){
        printf("Error opening file");
        return;
    } else {
        fprintf(fptr, "%i\n", score);
        fclose(fptr);
    }
}

void WriteMultiHighscore(int score){
    FILE *fptr = fopen("multi.txt", "a");
    if (fptr == NULL){
        printf("Error opening file");
    } else {
        fprintf(fptr, "%i\n", score);
        fclose(fptr);
    }
}

void DrawSingleplayerGame(Sound buttonclick, Font romulus, Texture2D CharacterAnimation[], int *selected_character, float *remainingTime, Deco deco, Texture2D GoblinAnimation[], int currentFrame, Camera2D *singlecamera, sTile (*world)[MAP_HEIGHT], Vector2 *tileIndices, int numTiles, int *active_money, moneyAtt moneyPosArray[], Screen *gamescreen, player *player1, int *goblin_spawn, monster goblinPosArray[], int frameloop, Vector2 meat_post, int *active_meat, int *monmon_spawn, monmon monmonPosArray[], Texture2D MonmonAnimation[], Texture2D CharacterTexture, bool *highscoreWritten, Stage *gamestage, GameTexture gametexture){   
    ClearBackground(BLACK);
    if (!gamescreen->gameOver){
        BeginMode2D(*singlecamera);

        DrawMap(deco, GoblinAnimation, currentFrame, world, tileIndices, active_money, moneyPosArray, goblin_spawn, goblinPosArray, frameloop, meat_post, active_meat, monmon_spawn, monmonPosArray, MonmonAnimation, gametexture);

        //Player Animation
        Rectangle source;
        Rectangle dest = { player1->position.x - 60, player1->position.y - 60, 120, 120 };
        
        //Walking animation
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
            source = (Rectangle){ currentFrame * FRAMESIZE, 1 * FRAMESIZE, -FRAMESIZE, FRAMESIZE };
        } else if ((IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) || (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) || (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))) {
            source = (Rectangle){ currentFrame * FRAMESIZE, 1 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } else {
            source = (Rectangle){ currentFrame * FRAMESIZE, 0 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        }   

        //Fighting animation 
        if ((IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) && IsMouseButtonDown(MOUSE_LEFT_BUTTON)){
            source = (Rectangle){ currentFrame * FRAMESIZE, 2 * FRAMESIZE, -FRAMESIZE, FRAMESIZE };
        } else if ((IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            source = (Rectangle){ currentFrame * FRAMESIZE, 2 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } else if ((IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            source = (Rectangle){ currentFrame * FRAMESIZE, 4 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } else if ((IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            source = (Rectangle){ currentFrame * FRAMESIZE, 6 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            source = (Rectangle){ currentFrame * FRAMESIZE, 2 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } 
        DrawTexturePro(CharacterAnimation[*selected_character], source, dest, (Vector2){ 0, 0 }, 0.0f, WHITE);

        DrawMarginItemsFront(deco, currentFrame);
        EndMode2D();

        if(GuiButtonWithSound(buttonclick, (Rectangle){ 30, 40, 50, 50}, "#132#")) gamescreen->pause = !gamescreen->pause;

        //Time Limit
        Color timeBar;
        if (*remainingTime >= 90){
            timeBar = GREEN;
        } else if (*remainingTime >= 40){
            timeBar = ORANGE;
        } else {
            timeBar = RED;
        }
        DrawRectangle(0, 0, *remainingTime / TIMELIMIT * SCREENWIDTH, 10, timeBar);

        if (gamestage->stage1) {
            DrawText("STAGE 1", SCREENWIDTH/2 - MeasureText("STAGE 1", 30)/2, 45, 30, WHITE);
        } else if (gamestage->stage2) {
            DrawText("STAGE 2", SCREENWIDTH/2 - MeasureText("STAGE 2", 30)/2, 45, 30, WHITE);
        } else {
            DrawText("FINAL STAGE", SCREENWIDTH/2 - MeasureText("FINAL STAGE", 30)/2, 45, 30, WHITE);
        }

        //Player Achievements
        DrawTexture(gametexture.moneyIdle, SCREENWIDTH - 240, -20, WHITE);
        DrawTextEx(romulus, TextFormat("X %i", player1->money_collected * 100), (Vector2){SCREENWIDTH - 140, 40}, 35, 1, WHITE);
        DrawTexture(gametexture.skull, SCREENWIDTH - 240, 50, WHITE);
        DrawTextEx(romulus, TextFormat("X %i", player1->goblin_killed), (Vector2){SCREENWIDTH - 140, 100}, 35, 1, WHITE);

        //Player lives
        for (int i = 0; i < player1->health; i++){
            DrawRectangle(25 + 32 * i, SCREENHEIGHT - 80, 30, 5, GREEN);
        }
        for (int i = 0; i < player1->life; i++){   
            Rectangle source = { 0, 0, gametexture.heart.width, gametexture.heart.height }; 
            Rectangle dest = { 20 + 60 * i, SCREENHEIGHT - 60, gametexture.heart.width, gametexture.heart.height }; 
            Vector2 origin = { 0, 0 }; 
            DrawTexturePro(gametexture.heart, source, dest, origin, 0.0f, WHITE); 
        }
    }else{
        DrawTexture(CharacterTexture, 0, 0, WHITE); 
        if (*remainingTime <= 0){
            DrawTextEx(romulus, "YOU SURVIVED THE HUNT", (Vector2){SCREENWIDTH/2 - MeasureTextEx(romulus, "YOU SURVIVED THE HUNT", 50, 5).x/2, 230}, 50, 5, (Color){130, 162, 159, 255});
        } else {
            DrawTextEx(romulus, "GAME OVER", (Vector2){SCREENWIDTH/2 - MeasureTextEx(romulus, "GAME OVER", 50, 5).x/2, 230}, 50, 5, (Color){130, 162, 159, 255});
        }

        int score = CalculateScore(player1, remainingTime);
        int highscore = ReadSingleHighscore();
        if (!*highscoreWritten){
            WriteSingleHighscore(score);
            if (score > highscore){
                highscore = score;
            }
            *highscoreWritten = true;
        }
        
        DrawTextEx(romulus, TextFormat("HIGH SCORE\t%i", highscore), (Vector2){SCREENWIDTH/2 - MeasureTextEx(romulus, TextFormat("HIGH SCORE\t%i", highscore), 30, 2).x/2, 200}, 30, 2, LIGHTGRAY);
        DrawTextEx(romulus, TextFormat("SCORE\t\t\t%i", score), (Vector2){SCREENWIDTH/2 - MeasureTextEx(romulus, TextFormat("SCORE\t\t\t%i", score), 30, 2).x/2, 320}, 30, 2, WHITE);
        DrawTextEx(romulus, TextFormat("ENEMY DEFEATED\t\tX%i", player1->goblin_killed), (Vector2){SCREENWIDTH/2 - MeasureTextEx(romulus, TextFormat("ENEMY DEFEATED\t\tX%i", player1->goblin_killed), 30, 2).x/2, 360}, 30, 2, WHITE);
        DrawTextEx(romulus, TextFormat("MONEY COLLECTED\t\tX%i", player1->money_collected * 100), (Vector2){SCREENWIDTH/2 - MeasureTextEx(romulus, TextFormat("MONEY COLLECTED\t\tX%i", player1->money_collected * 100), 30, 2).x/2, 400}, 30, 2, WHITE);
    }
}

void DrawMultiplayerGame(Sound buttonclick, Font romulus, int *selected_character1, int *selected_character2, float *remainingTime, Deco deco, Texture2D GoblinAnimation[], int currentFrame, Camera2D *multicamera1, Camera2D *multicamera2, sTile (*world)[MAP_HEIGHT], Vector2 *tileIndices, int numTiles, int *active_money, moneyAtt moneyPosArray[], Screen *gamescreen, player *player2, player *player3, int *goblin_spawn, monster goblinPosArray[], int frameloop, Vector2 meat_post, int *active_meat, int *monmon_spawn, monmon monmonPosArray[], Texture2D MonmonAnimation[], Texture2D CharacterAnimation[], Texture2D CharacterTexture, bool *highscoreWritten, Stage *gamestage, GameTexture gametexture){  
    ClearBackground(BLACK);
    if (!gamescreen->gameOver){
        BeginScissorMode(0, 0, SCREENWIDTH / 2, SCREENHEIGHT);
        BeginMode2D(*multicamera1); // Camera 1

        DrawMap(deco, GoblinAnimation, currentFrame, world, tileIndices, active_money, moneyPosArray, goblin_spawn, goblinPosArray, frameloop, meat_post, active_meat, monmon_spawn, monmonPosArray, MonmonAnimation, gametexture);
        
        //Player Animation 1
        Rectangle source1;
        Rectangle dest1 = { player2->position.x - 60, player2->position.y - 60, 120, 120 };

        //Walking animation
        if (IsKeyDown(KEY_A)) {
            source1 = (Rectangle){ currentFrame * FRAMESIZE, 1 * FRAMESIZE, -FRAMESIZE, FRAMESIZE };
        } else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_W) || IsKeyDown(KEY_S)) {
            source1 = (Rectangle){ currentFrame * FRAMESIZE, 1 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } else {
            source1 = (Rectangle){ currentFrame * FRAMESIZE, 0 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        }  

         //Fighting animation 
        if (IsKeyDown(KEY_A) && IsKeyDown(KEY_LEFT_SHIFT)){
            source1 = (Rectangle){ currentFrame * FRAMESIZE, 2 * FRAMESIZE, -FRAMESIZE, FRAMESIZE };
        } else if (IsKeyDown(KEY_D) && IsKeyDown(KEY_LEFT_SHIFT)) {
            source1 = (Rectangle){ currentFrame * FRAMESIZE, 2 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } else if (IsKeyDown(KEY_S) && IsKeyDown(KEY_LEFT_SHIFT)) {
            source1 = (Rectangle){ currentFrame * FRAMESIZE, 4 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } else if (IsKeyDown(KEY_W) && IsKeyDown(KEY_LEFT_SHIFT)) {
            source1 = (Rectangle){ currentFrame * FRAMESIZE, 6 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } else if (IsKeyDown(KEY_LEFT_SHIFT)) {
            source1 = (Rectangle){ currentFrame * FRAMESIZE, 2 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } 
        DrawTexturePro(CharacterAnimation[*selected_character1], source1, dest1, (Vector2){ 0, 0 }, 0.0f, WHITE);

        //Player Animation 2
        Rectangle source2;
        Rectangle dest2 = { player3->position.x - 60, player3->position.y - 60, 120, 120 };

        //Walking animation
        if (IsKeyDown(KEY_LEFT)) {
            source2 = (Rectangle){ currentFrame * FRAMESIZE, 1 * FRAMESIZE, -FRAMESIZE, FRAMESIZE };
        } else if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN)) {
            source2 = (Rectangle){ currentFrame * FRAMESIZE, 1 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } else {
            source2 = (Rectangle){ currentFrame * FRAMESIZE, 0 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        }  

         //Fighting animation 
        if (IsKeyDown(KEY_LEFT) && IsKeyDown(KEY_RIGHT_SHIFT)){
            source2 = (Rectangle){ currentFrame * FRAMESIZE, 2 * FRAMESIZE, -FRAMESIZE, FRAMESIZE };
        } else if (IsKeyDown(KEY_RIGHT) && IsKeyDown(KEY_RIGHT_SHIFT)) {
            source2 = (Rectangle){ currentFrame * FRAMESIZE, 2 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } else if (IsKeyDown(KEY_DOWN) && IsKeyDown(KEY_RIGHT_SHIFT)) {
            source2 = (Rectangle){ currentFrame * FRAMESIZE, 4 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } else if (IsKeyDown(KEY_UP) && IsKeyDown(KEY_RIGHT_SHIFT)) {
            source2 = (Rectangle){ currentFrame * FRAMESIZE, 6 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } else if (IsKeyDown(KEY_RIGHT_SHIFT)) {
            source2 = (Rectangle){ currentFrame * FRAMESIZE, 2 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } 
        DrawTexturePro(CharacterAnimation[*selected_character2], source2, dest2, (Vector2){ 0, 0 }, 0.0f, WHITE);

        DrawMarginItemsFront(deco, currentFrame);
        EndMode2D();

        BeginScissorMode(SCREENWIDTH / 2, 0, SCREENWIDTH / 2, SCREENHEIGHT);
        BeginMode2D(*multicamera2);
        DrawMap(deco, GoblinAnimation, currentFrame, world, tileIndices, active_money, moneyPosArray, goblin_spawn, goblinPosArray, frameloop, meat_post, active_meat, monmon_spawn, monmonPosArray, MonmonAnimation, gametexture);
        
        //Player Animation 1
        Rectangle source3;
        Rectangle dest3 = { player2->position.x - 60, player2->position.y - 60, 120, 120 };

        //Walking animation
        if (IsKeyDown(KEY_A)) {
            source3 = (Rectangle){ currentFrame * FRAMESIZE, 1 * FRAMESIZE, -FRAMESIZE, FRAMESIZE };
        } else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_W) || IsKeyDown(KEY_S)) {
            source3 = (Rectangle){ currentFrame * FRAMESIZE, 1 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } else {
            source3 = (Rectangle){ currentFrame * FRAMESIZE, 0 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        }  

         //Fighting animation 
        if (IsKeyDown(KEY_A) && IsKeyDown(KEY_LEFT_SHIFT)){
            source3 = (Rectangle){ currentFrame * FRAMESIZE, 2 * FRAMESIZE, -FRAMESIZE, FRAMESIZE };
        } else if (IsKeyDown(KEY_D) && IsKeyDown(KEY_LEFT_SHIFT)) {
            source3 = (Rectangle){ currentFrame * FRAMESIZE, 2 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } else if (IsKeyDown(KEY_S) && IsKeyDown(KEY_LEFT_SHIFT)) {
            source3 = (Rectangle){ currentFrame * FRAMESIZE, 4 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } else if (IsKeyDown(KEY_W) && IsKeyDown(KEY_LEFT_SHIFT)) {
            source3 = (Rectangle){ currentFrame * FRAMESIZE, 6 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } else if (IsKeyDown(KEY_LEFT_SHIFT)) {
            source3 = (Rectangle){ currentFrame * FRAMESIZE, 2 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } 
        DrawTexturePro(CharacterAnimation[*selected_character1], source3, dest3, (Vector2){ 0, 0 }, 0.0f, WHITE);

        //Player Animation 2
        Rectangle source4;
        Rectangle dest4 = { player3->position.x - 60, player3->position.y - 60, 120, 120 };

        //Walking animation
        if (IsKeyDown(KEY_LEFT)) {
            source4 = (Rectangle){ currentFrame * FRAMESIZE, 1 * FRAMESIZE, -FRAMESIZE, FRAMESIZE };
        } else if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN)) {
            source4 = (Rectangle){ currentFrame * FRAMESIZE, 1 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } else {
            source4 = (Rectangle){ currentFrame * FRAMESIZE, 0 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        }  

         //Fighting animation 
        if (IsKeyDown(KEY_LEFT) && IsKeyDown(KEY_RIGHT_SHIFT)){
            source4 = (Rectangle){ currentFrame * FRAMESIZE, 2 * FRAMESIZE, -FRAMESIZE, FRAMESIZE };
        } else if (IsKeyDown(KEY_RIGHT) && IsKeyDown(KEY_RIGHT_SHIFT)) {
            source4 = (Rectangle){ currentFrame * FRAMESIZE, 2 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } else if (IsKeyDown(KEY_DOWN) && IsKeyDown(KEY_RIGHT_SHIFT)) {
            source4 = (Rectangle){ currentFrame * FRAMESIZE, 4 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } else if (IsKeyDown(KEY_UP) && IsKeyDown(KEY_RIGHT_SHIFT)) {
            source4 = (Rectangle){ currentFrame * FRAMESIZE, 6 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } else if (IsKeyDown(KEY_RIGHT_SHIFT)) {
            source4 = (Rectangle){ currentFrame * FRAMESIZE, 2 * FRAMESIZE, FRAMESIZE, FRAMESIZE };
        } 
        DrawTexturePro(CharacterAnimation[*selected_character2], source4, dest4, (Vector2){ 0, 0 }, 0.0f, WHITE);

        DrawMarginItemsFront(deco, currentFrame);

        EndMode2D();

        BeginScissorMode(0, 0, SCREENWIDTH, SCREENHEIGHT);

        if (GuiButtonWithSound(buttonclick, (Rectangle){ 30, 40, 50, 50}, "#132#")) gamescreen->pause = !gamescreen->pause;

        DrawRectangle(SCREENWIDTH / 2, 0, 8, SCREENHEIGHT, BLACK);
        
        //Time Limit
        Color timeBar;
        if (*remainingTime >= 90){
            timeBar = GREEN;
        } else if (*remainingTime >= 40){
            timeBar = ORANGE;
        } else {
            timeBar = RED;
        }
        DrawRectangle(0, 0, *remainingTime / TIMELIMIT * SCREENWIDTH, 10, timeBar);

        if (gamestage->stage1) {
            DrawText("STAGE 1", SCREENWIDTH/4 - MeasureText("STAGE 1", 30)/2, 45, 30, WHITE);
            DrawText("STAGE 1", SCREENWIDTH*3/4 - MeasureText("STAGE 1", 30)/2, 45, 30, WHITE);
        } else if (gamestage->stage2) {
            DrawText("STAGE 2", SCREENWIDTH/4 - MeasureText("STAGE 2", 30)/2, 45, 30, WHITE);
            DrawText("STAGE 2", SCREENWIDTH*3/4 - MeasureText("STAGE 2", 30)/2, 45, 30, WHITE);
        } else {
            DrawText("FINAL STAGE", SCREENWIDTH/4 - MeasureText("FINAL STAGE", 30)/2, 45, 30, WHITE);
            DrawText("FINAL STAGE", SCREENWIDTH*3/4 - MeasureText("FINAL STAGE", 30)/2, 45, 30, WHITE);
        }

        //Player Achievements
        DrawTexture(gametexture.moneyIdle, SCREENWIDTH/2 - 240, -20, WHITE);
        DrawTextEx(romulus, TextFormat("X %i", player2->money_collected * 100), (Vector2){SCREENWIDTH/2 - 140, 40}, 35, 1, WHITE);
        DrawTexture(gametexture.skull, SCREENWIDTH/2 - 240, 50, WHITE);
        DrawTextEx(romulus, TextFormat("X %i", player2->goblin_killed), (Vector2){SCREENWIDTH/2 - 140, 100}, 35, 1, WHITE);
        DrawTexture(gametexture.moneyIdle, SCREENWIDTH - 240, -20, WHITE);
        DrawTextEx(romulus, TextFormat("X %i", player3->money_collected * 100), (Vector2){SCREENWIDTH - 140, 40}, 35, 1, WHITE);
        DrawTexture(gametexture.skull, SCREENWIDTH - 240, 50, WHITE);
        DrawTextEx(romulus, TextFormat("X %i", player3->goblin_killed), (Vector2){SCREENWIDTH - 140, 100}, 35, 1, WHITE);

        //Player 1 lives
        for (int i = 0; i < player2->health; i++){
            DrawRectangle(25 + 32 * i, SCREENHEIGHT - 80, 30, 5, GREEN);
        }
        for (int i = 0; i < player2->life; i++){   
            Rectangle source = { 0, 0, gametexture.heart.width, gametexture.heart.height }; 
            Rectangle dest = { 20 + 60 * i, SCREENHEIGHT - 60, gametexture.heart.width, gametexture.heart.height }; 
            Vector2 origin = { 0, 0 }; 
            DrawTexturePro(gametexture.heart, source, dest, origin, 0.0f, WHITE); 
        }
        //Player 2 lives
        for (int i = 0; i < player3->health; i++){
            DrawRectangle(1144 - 32 * i, SCREENHEIGHT - 80, 30, 5, GREEN);
        }
        for (int i = 0; i < player3->life; i++){   
            Rectangle source = { 0, 0, gametexture.heart.width, gametexture.heart.height }; 
            Rectangle dest = { 1130 - 60 * i, SCREENHEIGHT - 60, gametexture.heart.width, gametexture.heart.height }; 
            Vector2 origin = { 0, 0 }; 
            DrawTexturePro(gametexture.heart, source, dest, origin, 0.0f, WHITE); 
        }
    }else{
        int score1 = CalculateScore(player2, remainingTime);
        int score2 = CalculateScore(player3, remainingTime);
        int highscore = ReadMultiHighscore();
        char *player1w = "Waiting...";
        char *player2w = "Waiting...";
        if (!*highscoreWritten){
            if (score1 > score2){
                WriteMultiHighscore(score1);
                if (score1 > highscore){
                    highscore = score1;
                }
            } else {
                WriteMultiHighscore(score2);
                if (score2 > highscore){
                    highscore = score2;
                }
            }
            *highscoreWritten = true;
        }

        if (score1 > score2){
            player1w = "You Win!";
            player2w = "You Lose!";
        } else {
            player1w = "You Lose!";
            player2w = "You Win!";
        }

        DrawTexture(CharacterTexture, 0, 0, WHITE); 
        if (*remainingTime <= 0){
            DrawTextEx(romulus, "YOU SURVIVED THE HUNT", (Vector2){SCREENWIDTH/2 - MeasureTextEx(romulus, "YOU SURVIVED THE HUNT", 50, 5).x/2, 230}, 50, 5, (Color){130, 162, 159, 255});
        } else {
            DrawTextEx(romulus, TextFormat("%s", player1w), (Vector2){SCREENWIDTH/4 - MeasureTextEx(romulus, TextFormat("%s", player1w), 50, 5).x/2, 230}, 50, 5, (Color){130, 162, 159, 255});
            DrawTextEx(romulus, TextFormat("%s", player2w), (Vector2){SCREENWIDTH*3/4 - MeasureTextEx(romulus, TextFormat("%s", player2w), 50, 5).x/2, 230}, 50, 5, (Color){130, 162, 159, 255}); 
        }

        DrawTextEx(romulus, TextFormat("HIGH SCORE\t%i", highscore), (Vector2){SCREENWIDTH/2 - MeasureTextEx(romulus, TextFormat("HIGH SCORE\t%i", highscore), 30, 2).x/2, 200}, 30, 2, LIGHTGRAY);

        //Player 1
        DrawTextEx(romulus, "PLAYER 1", (Vector2){SCREENWIDTH/4 - MeasureTextEx(romulus, "PLAYER 1", 30, 2).x/2, 300}, 30, 2, WHITE);
        DrawTextEx(romulus, TextFormat("SCORE\t\t\t%i", score1), (Vector2){SCREENWIDTH/4 - MeasureTextEx(romulus, TextFormat("SCORE\t\t\t%i", score1), 30, 2).x/2, 360}, 30, 2, WHITE);
        DrawTextEx(romulus, TextFormat("ENEMY DEFEATED\t\tX%i", player2->goblin_killed), (Vector2){SCREENWIDTH/4 - MeasureTextEx(romulus, TextFormat("ENEMY DEFEATED\t\tX%i", player2->goblin_killed), 30, 2).x/2, 400}, 30, 2, WHITE);
        DrawTextEx(romulus, TextFormat("MONEY COLLECTED\t\tX%i", player2->money_collected * 100), (Vector2){SCREENWIDTH/4 - MeasureTextEx(romulus, TextFormat("MONEY COLLECTED\t\tX%i", player2->money_collected * 100), 30, 2).x/2, 440}, 30, 2, WHITE);

        //Player 2
        DrawTextEx(romulus, "PLAYER 2", (Vector2){SCREENWIDTH*3/4 - MeasureTextEx(romulus, "PLAYER 2", 30, 2).x/2, 300}, 30, 2, WHITE);
        DrawTextEx(romulus, TextFormat("SCORE\t\t\t%i", score2), (Vector2){SCREENWIDTH*3/4 - MeasureTextEx(romulus, TextFormat("SCORE\t\t\t%i", score2), 30, 2).x/2, 360}, 30, 2, WHITE);
        DrawTextEx(romulus, TextFormat("ENEMY DEFEATED\t\tX%i", player3->goblin_killed), (Vector2){SCREENWIDTH*3/4 - MeasureTextEx(romulus, TextFormat("ENEMY DEFEATED\t\tX%i", player3->goblin_killed), 30, 2).x/2, 400}, 30, 2, WHITE);
        DrawTextEx(romulus, TextFormat("MONEY COLLECTED\t\tX%i", player3->money_collected * 100), (Vector2){SCREENWIDTH*3/4 - MeasureTextEx(romulus, TextFormat("MONEY COLLECTED\t\tX%i", player3->money_collected * 100), 30, 2).x/2, 440}, 30, 2, WHITE);
    }
}

void DrawManual(sTile (*world)[MAP_HEIGHT], Vector2 *tileIndices, Screen *gamescreen, Sound buttonclick, Font romulus, int currentFrame, Deco deco, Texture2D MonmonAnimation[], Texture2D GoblinAnimation[], int *playermode, GameTexture gametexture){
    //Camera
    Camera2D manual_cam;
    manual_cam.target = (Vector2){ 0, 0 };
    manual_cam.offset = (Vector2){ 0, 0 };
    manual_cam.rotation = 0.0f;
    manual_cam.zoom = 2.8f;

    Vector2 mousePoint = GetScreenToWorld2D(GetMousePosition(), manual_cam);

    BeginMode2D(manual_cam);

    //Map
    for (int i = 0; i < MAP_WIDTH; i++){
        for (int j = 0; j < MAP_HEIGHT; j++){
            sTile tile = world[i][j];
            int tile_index_x = tileIndices[tile.tileIndex].x;
            int tile_index_y = tileIndices[tile.tileIndex].y;

            Rectangle source = {tile_index_x * TILE_WIDTH, tile_index_y * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT};
            Rectangle dest = {tile.x * TILE_WIDTH, tile.y * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT};
            Vector2 origin = {100, 100};
            DrawTexturePro(gametexture.maptile, source, dest, origin, 0.0f, WHITE);
        }
    }

    //Deco
    Vector2 trees[] = { {-50, -60}, {10, -20} };
    int numTrees2 = sizeof(trees) / sizeof(trees[0]);
    for (int i = 0; i < numTrees2; i++){
        DrawTexturePro(deco.tree, (Rectangle){currentFrame * FRAMESIZE, 0, FRAMESIZE, FRAMESIZE}, (Rectangle){ trees[i].x, trees[i].y, 170, 170 }, (Vector2){ 0, 0 }, 0.0f, WHITE);
    }

    DrawTexturePro(deco.tent1, (Rectangle) {0, 0, deco.tent1.width, deco.tent1.height}, (Rectangle) {175, 0, deco.tent1.width/6, deco.tent1.height/6}, (Vector2){ 0, 0 }, 0.0f, WHITE);

    DrawTexturePro(deco.fire, (Rectangle) {currentFrame * 128, currentFrame * 128, 128, 128}, (Rectangle) {140, 50, 70, 70}, (Vector2){ 0, 0 }, 0.0f, WHITE);

    //Gameplay items
    Rectangle items_rect[] = {
        {140, 140, 60, 60},
        {20, 180, 60, 60},
        {80, 190, 100, 100},
        {45, 110, 100, 100}
    };

    DrawTexturePro(gametexture.money, (Rectangle) {currentFrame * 128, currentFrame * 128, 128, 128}, items_rect[0], (Vector2){ 0, 0 }, 0.0f, WHITE);

    DrawTexturePro(gametexture.meat, (Rectangle) {currentFrame * 128, currentFrame * 128, 128, 128}, items_rect[1], (Vector2){ 0, 0 }, 0.0f, WHITE);

    DrawTexturePro(MonmonAnimation[monmon_blue], (Rectangle) {currentFrame * FRAMESIZE, 4 * FRAMESIZE, FRAMESIZE, FRAMESIZE}, items_rect[2], (Vector2){ 0, 0 }, 0.0f, WHITE);

    DrawTexturePro(GoblinAnimation[goblin_yellow], (Rectangle) {currentFrame * FRAMESIZE, 0 * FRAMESIZE, FRAMESIZE, FRAMESIZE}, items_rect[3], (Vector2){ 0, 0 }, 0.0f, WHITE);

    //Check hovered items
    int hoveredItem = -1; 
    for (int i = 0; i < 4; i++) { 
        if (CheckCollisionPointRec(mousePoint, items_rect[i])) { 
            hoveredItem = i; 
            break; 
        } 
    }

    EndMode2D();

    if (IsKeyPressed(KEY_ESCAPE) || GuiButtonWithSound(buttonclick, (Rectangle){ 30, 40, 120, 50}, "BACK")) {
        gamescreen->manual = false;
        gamescreen->showMenu = true;
    }

    DrawTextEx(romulus, "GAMEPLAY GUIDE", (Vector2){SCREENWIDTH/2 - MeasureText("GAMEPLAY GUIDE", 40)/2, 45}, 40, 1, WHITE);

    //Control key guide
    int box_x = 600;
    int box_y = 130;
    int box_width = 570;
    int box_height = 540;
    int border_width = 2;
    DrawRectangleLinesEx((Rectangle){box_x - border_width, box_y - border_width, box_width + 2 * border_width, box_height + 2 * border_width}, 2, (Color){96, 130, 125, 255});
    DrawRectangle(box_x, box_y, box_width, box_height, (Color){44, 51, 52, 240});
    DrawTextEx(romulus, "Control Keys", (Vector2){box_x + box_width/2 - MeasureTextEx(romulus, "Control Keys", 40, 1.5f).x/2, 150}, 40, 1.5f, (Color){96, 130, 125, 255});

    //Change state for displaying different player mode guide
    bool clicked = GuiToggleSlider((Rectangle){box_x + box_width/2 - 400/2, 400, 400, 40}, "Singleplayer;Multiplayer", playermode);
    if (clicked) {
        PlaySound(buttonclick);
    }

    if (*playermode == 0){
        DrawTexturePro(gametexture.keyboard_single, (Rectangle){0, 0, gametexture.keyboard.width, gametexture.keyboard.height}, (Rectangle){box_x + box_width/2 - gametexture.keyboard.width/3.2, 200, gametexture.keyboard.width/1.6, gametexture.keyboard.height/1.6}, (Vector2){ 0, 0 }, 0.0f, WHITE);
        GuiLabel((Rectangle){box_x + box_width / 2 + 20, 400, 200, 40}, "Multiplayer");

        //Control key
        DrawTextEx(romulus, "WASD/", (Vector2){box_x + 100, 460}, 30, 1.5f, (Color){96, 130, 125, 255});

        //Arrow button
        GuiLabel((Rectangle){box_x + 200, 460, 100, 30}, "#121#");
        GuiLabel((Rectangle){box_x + 215, 460, 100, 30}, "#118#");
        GuiLabel((Rectangle){box_x + 230, 460, 100, 30}, "#120#");
        GuiLabel((Rectangle){box_x + 245, 460, 100, 30}, "#119#");

        DrawTextEx(romulus, "Left Click", (Vector2){box_x + 100, 500}, 30, 1.5f, (Color){96, 130, 125, 255});

        DrawTextEx(romulus, "Move", (Vector2){box_x + 320, 460}, 30, 1.5f, (Color){96, 130, 125, 255});
        DrawTextEx(romulus, "Attack", (Vector2){box_x + 320, 500}, 30, 1.5f, (Color){96, 130, 125, 255});
    } else {
        DrawTexturePro(gametexture.keyboard, (Rectangle){0, 0, gametexture.keyboard.width, gametexture.keyboard.height}, (Rectangle){box_x + box_width/2 - gametexture.keyboard.width/3.2, 200, gametexture.keyboard.width/1.6, gametexture.keyboard.height/1.6}, (Vector2){ 0, 0 }, 0.0f, WHITE);
        GuiLabel((Rectangle){box_x + box_width / 2 - 185, 400, 200, 40}, "Singleplayer");

        //PLAYER1 
        DrawTextEx(romulus, "WASD", (Vector2){box_x + 100, 460}, 30, 1.5f, (Color){96, 130, 125, 255});
        DrawTextEx(romulus, "Left Shift", (Vector2){box_x + 100, 500}, 30, 1.5f, (Color){96, 130, 125, 255});

        DrawTextEx(romulus, "PLayer 1 Move", (Vector2){box_x + 300, 460}, 30, 1.5f, (Color){96, 130, 125, 255});
        DrawTextEx(romulus, "Player 1 Attack", (Vector2){box_x + 300, 500}, 30, 1.5f, (Color){96, 130, 125, 255});

        //PLAYER2  
        GuiLabel((Rectangle){box_x + 100, 540, 100, 30}, "#121#");
        GuiLabel((Rectangle){box_x + 115, 540, 100, 30}, "#118#");
        GuiLabel((Rectangle){box_x + 130, 540, 100, 30}, "#120#");
        GuiLabel((Rectangle){box_x + 145, 540, 100, 30}, "#119#");
        DrawTextEx(romulus, "Right Shift", (Vector2){box_x + 100, 580}, 30, 1.5f, (Color){96, 130, 125, 255});

        DrawTextEx(romulus, "Player 2 Move", (Vector2){box_x + 300, 540}, 30, 1.5f, (Color){96, 130, 125, 255});
        DrawTextEx(romulus, "PLayer 2 Attack", (Vector2){box_x + 300, 580}, 30, 1.5f, (Color){96, 130, 125, 255});
    }

    //Draw info for hovered item
    Vector2 infoBoxPosition;
    char *itemType;
    char *itemInfo;
    Vector2 infoBoxSize;
    switch (hoveredItem){
        case 0:
            infoBoxPosition = GetWorldToScreen2D((Vector2){140, 130}, manual_cam);
            itemType = "Money";
            itemInfo = "Value: $100";
            infoBoxSize.x = 200;
            infoBoxSize.y = 70;
            break;
        case 1:
            infoBoxPosition = GetWorldToScreen2D((Vector2){20, 160}, manual_cam);
            itemType = "Meat";
            itemInfo = "Fully restores health but \n\ndoes not revive.";
            infoBoxSize.x = 300;
            infoBoxSize.y = 100;
            break;
        case 2:
            infoBoxPosition = GetWorldToScreen2D((Vector2){90, 160}, manual_cam);
            itemType = "Monmon";
            itemInfo = "It carries three money bags \n\nand roams the area. Defeat \n\nit to claim the money.";
            infoBoxSize.x = 310;
            infoBoxSize.y = 130;
            break;
        case 3:
            infoBoxPosition = GetWorldToScreen2D((Vector2){55, 75}, manual_cam);
            itemType = "Goblin";
            itemInfo = "As you advance, this enemy becomes \n\nmore common. It's a significant threat \n\nand can cause your demise if not \n\nswiftly eliminated. Take it out on sight.";
            infoBoxSize.x = 410;
            infoBoxSize.y = 160;
            break;
        default:
            break;
    }
    if (hoveredItem >= 0){
        DrawRectangle(infoBoxPosition.x, infoBoxPosition.y, infoBoxSize.x, infoBoxSize.y, (Color){44, 51, 52, 230});
        DrawText(TextFormat("Item: %s\n\n%s", itemType, itemInfo), infoBoxPosition.x + 10, infoBoxPosition.y + 10, 20, WHITE);
    }
}

int main(void){
    //Variables
    float remainingTime;

    InitWindow(SCREENWIDTH, SCREENHEIGHT, "Campfire Hunt");
    Image icon = LoadImage("images/tent3.png");
    SetWindowIcon(icon);
    InitAudioDevice();
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    srand(time(NULL));
    GuiLoadStyle("style_jungle.rgs");

    //Background music
    SoundAsset loadsound = {
        .buttonclick = LoadSound("audio/misc-pointer-static.mp3"),
        .WalkingSound = LoadSound("audio/walking.mp3"),
        .SwordSound = LoadSound("audio/sword.mp3"),
        .gameplay_music = LoadMusicStream("audio/campingsound.mp3"),
        .stage2_music = LoadMusicStream("audio/stage2.mp3"),
        .stage3_music = LoadMusicStream("audio/stage3.mp3"),
        .MoneySound = LoadSound("audio/money.wav"),
        .healsound = LoadSound("audio/heal.mp3"),
        .monsterdead = LoadSound("audio/monsterdeath.mp3"),
        .minusheart = LoadSound("audio/loseheart.mp3"),
        .monmonsound = LoadSound("audio/monmon.mp3"),
        .gamecomplete = LoadSound("audio/gamecompleted.wav"),
        .gameoversound = LoadSound("audio/gameoversound.wav")
    };    
    SetSoundVolume(loadsound.WalkingSound, 2.0f);
    SetSoundVolume(loadsound.SwordSound, 0.8f);
    PlayMusicStream(loadsound.gameplay_music);
    SetMusicVolume(loadsound.stage2_music, 0.3f);
    SetMusicVolume(loadsound.stage2_music, 0.5f);
    bool gameEndSound = false;
    
    //Menu
    Font romulus = LoadFont("images/romulus.png");
    float FontSize = 100.0;
    float speed = 0.20;
    bool font_growing = true;
    Texture2D MenuTexture = LoadTexture("images/campingimage.png");

    //Game Screen
    Screen gamescreen = { false, false, true, false, false, false, false, false };
    Stage gamestage = {false, false, false};

    //Camera
    Camera2D singlecamera = { 0 };
    Camera2D multicamera1 = { 0 };
    Camera2D multicamera2 = { 0 };

    //Animation Frame
    int numFrames = 6; 
    int currentFrame = 0;
    float frameTime = 0;
    float frameDelay = 0.1f;
    int action = 0;
    int frameloop = 0;
    int alt_frameloop = 0;

    Texture2D CharacterTexture = LoadTexture("images/characterselection.png");
    //Character Spritesheet
    Texture2D CharacterAnimation[MAX_CHARACTER_TYPE];
    CharacterAnimation[warrior_red] = LoadTexture("images/Warrior_Red.png");
    CharacterAnimation[warrior_blue] = LoadTexture("images/Warrior_Blue.png");
    CharacterAnimation[warrior_yellow] = LoadTexture("images/Warrior_Yellow.png");
    CharacterAnimation[warrior_purple] = LoadTexture("images/Warrior_Purple.png");
    int selected_character, selected_character1, selected_character2 = warrior_red;

    //Player
    player player1 = { 0 };
    player player2 = { 0 };
    player player3 = { 0 };

    //Goblin Spritesheet
    Texture2D GoblinAnimation[MAX_CHARACTER_TYPE];
    GoblinAnimation[goblin_red] = LoadTexture("images/goblin_red.png");
    GoblinAnimation[goblin_blue] = LoadTexture("images/goblin_blue.png");
    GoblinAnimation[goblin_yellow] = LoadTexture("images/goblin_yellow.png");
    GoblinAnimation[goblin_purple] = LoadTexture("images/goblin_purple.png");

    int goblin_spawn = 0;
    int goblinloop = 0;
    int goblin_amount = 8;
    monster *goblinPosArray = malloc(goblin_amount * sizeof(monster));

    //Monmon Spritesheet
    Texture2D MonmonAnimation[MAX_CHARACTER_TYPE];
    MonmonAnimation[monmon_red] = LoadTexture("images/Pawn_Red.png");
    MonmonAnimation[monmon_blue] = LoadTexture("images/Pawn_Blue.png");
    MonmonAnimation[monmon_yellow] = LoadTexture("images/Pawn_Yellow.png");
    MonmonAnimation[monmon_purple] = LoadTexture("images/Pawn_Purple.png");

    int monmon_spawn = 0;
    int monmonloop = 0;
    monmon monmonPosArray[MONMON_AMOUNT];

    sTile world[MAP_WIDTH][MAP_HEIGHT]; //Map
    Vector2 tileIndices[] = { {0, 1}, {6, 3}, {0, 11}, {2, 11}, {8, 3}, {1, 3} }; //tile index from the image
    int numTiles = (sizeof(tileIndices) / sizeof(tileIndices[0]));
    
    Deco deco; //Map Decoration
    deco.tree = LoadTexture("images/Tree.png");
    deco.bush = LoadTexture("images/bush.png");
    deco.tent1 = LoadTexture("images/tent2.png");
    deco.tent2 = LoadTexture("images/tent3.png");
    deco.fire = LoadTexture("images/Fire.png");
    deco.wood = LoadTexture("images/woods.png");
    deco.pumpkin = LoadTexture("images/pumpkin.png");
    deco.mushroom = LoadTexture("images/mushroom.png");
    deco.scarecrow = LoadTexture("images/scarecrow.png");

    //Gameplay Texture
    GameTexture gametexture = {
        .heart = LoadTexture("images/heart.png"),
        .money = LoadTexture("images/money_animation.png"),
        .meat = LoadTexture("images/meat.png"),
        .moneyIdle = LoadTexture("images/moneyidle.png"),
        .skull = LoadTexture("images/skull.png"),
        .dead = LoadTexture("images/Dead.png"),
        .keyboard = LoadTexture("images/keyboard.png"),
        .keyboard_single = LoadTexture("images/keyboard_single.png"),
        .maptile = LoadTexture("images/GRASS+.png")
    };

    //Meat 
    int meatloop = 0;
    int active_meat = 0;
    Vector2 meat_post;

    //Money
    int active_money = 0;
    int moneyloop = 0;
    moneyAtt moneyPosArray[MONEY_AMOUNT];

    int playermode = 0; //Toggleslider state
    bool highscoreWritten = false; //Allow once written to file each game

    InitGame(&remainingTime, &singlecamera, world, numTiles, &active_money, &moneyloop, &gamescreen, &player1, &player2, &player3, &goblin_spawn, &goblinloop, &meatloop, &active_meat, &multicamera1, &multicamera2, &monmon_spawn, &highscoreWritten, &gamestage, &monmonloop);
    
    while (!WindowShouldClose()){
        UpdateMusicStream(loadsound.gameplay_music);

        //Animation Frame
        frameTime += GetFrameTime();
        if (frameTime >= frameDelay) {
            frameTime = 0;
            currentFrame = (currentFrame + 1) % numFrames;

            if (currentFrame == 0) { //for characterselection animation
                frameloop ++;
                if (frameloop % 10 == 0) {
                    action = rand() % 8; 
                    alt_frameloop = 0; 
                } else {
                    if (alt_frameloop == 0) {
                        action = rand() % 2; 
                    }
                    alt_frameloop++;

                    if (alt_frameloop >= 20) {
                        alt_frameloop = 0; 
                    }
                }
            }
        }

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (gamescreen.showMenu) {
            DrawTexture(MenuTexture, 0, 0, WHITE); 

            //Title Font Animation
            if (font_growing){
                FontSize += speed;
                if (FontSize >= 105) font_growing = false; 
            }else{
                FontSize -= speed;
                if (FontSize <= 100) font_growing = true; 
            }
            DrawTextEx(romulus, "CAMPFIRE HUNT", (Vector2){SCREENWIDTH/2 - MeasureTextEx(romulus, "CAMPFIRE HUNT", FontSize, 5).x/2, 280}, FontSize, 5, (Color){ 255, 246, 229, 255 });
            
            //Button
            if (GuiButtonWithSound(loadsound.buttonclick, (Rectangle){ SCREENWIDTH/2 - 150, 450, 300, 50}, "SINGLEPLAYER")){
                gamescreen.showMenu = false;
                gamescreen.changesinglecharacter = true;
            }
            if (GuiButtonWithSound(loadsound.buttonclick, (Rectangle){ SCREENWIDTH/2 - 150, 520, 300, 50}, "MULTIPLAYER")){
                gamescreen.showMenu = false;
                gamescreen.changemulticharacter = true;
            }
            if (GuiButtonWithSound(loadsound.buttonclick, (Rectangle){ SCREENWIDTH/2 - 150, 590, 300, 50}, "GAMEPLAY GUIDE")){
                gamescreen.showMenu = false;
                gamescreen.manual = true;
            }
            if (GuiButtonWithSound(loadsound.buttonclick, (Rectangle){ SCREENWIDTH/2 - 150, 660, 300, 50}, "QUIT")){
                break;
            }
        } else if (gamescreen.changesinglecharacter){
            ChangeSingleCharacter(loadsound.buttonclick, CharacterAnimation, &selected_character, CharacterTexture, action, currentFrame, &gamescreen, &player1);
        } else if (gamescreen.changemulticharacter){
            ChangeMultiCharacter(loadsound.buttonclick, CharacterAnimation, &selected_character1, &selected_character2, CharacterTexture, action, currentFrame, &gamescreen);
        } else if (gamescreen.manual) {
            DrawManual(world, tileIndices, &gamescreen, loadsound.buttonclick, romulus, currentFrame, deco, MonmonAnimation, GoblinAnimation, &playermode, gametexture);
        } else if (gamescreen.singleplayer){
            UpdateSingleplayerGame(loadsound, &remainingTime, GoblinAnimation, &singlecamera, &active_money, &moneyloop, moneyPosArray, &gamescreen, &player1, &goblin_spawn, &goblinloop, goblinPosArray, &meat_post, &meatloop, &active_meat, &monmonloop, &monmon_spawn, monmonPosArray, MonmonAnimation, goblin_amount, &gamestage);
            CollectMeat(loadsound.healsound, &player1, meat_post, &active_meat, &meatloop);
            CollectMoney(loadsound.MoneySound, &active_money, moneyPosArray, &player1);
            DrawSingleplayerGame(loadsound.buttonclick, romulus, CharacterAnimation, &selected_character, &remainingTime, deco, GoblinAnimation, currentFrame, &singlecamera, world, tileIndices, numTiles, &active_money, moneyPosArray, &gamescreen, &player1, &goblin_spawn, goblinPosArray, frameloop, meat_post, &active_meat, &monmon_spawn, monmonPosArray, MonmonAnimation, CharacterTexture, &highscoreWritten, &gamestage, gametexture);
        } else if (gamescreen.multiplayer){
            UpdateMultiplayerGame(loadsound, &remainingTime, GoblinAnimation, &multicamera1, &multicamera2, &active_money, &moneyloop, moneyPosArray, &gamescreen, &player2, &player3, &goblin_spawn, &goblinloop, goblinPosArray, &meat_post, &meatloop, &active_meat, &monmonloop, &monmon_spawn, monmonPosArray, MonmonAnimation, goblin_amount, &gamestage);
            CollectMeat(loadsound.healsound, &player2, meat_post, &active_meat, &meatloop);
            CollectMoney(loadsound.MoneySound, &active_money, moneyPosArray, &player2);
            CollectMeat(loadsound.healsound, &player3, meat_post, &active_meat, &meatloop);
            CollectMoney(loadsound.MoneySound, &active_money, moneyPosArray, &player3);
            DrawMultiplayerGame(loadsound.buttonclick, romulus, &selected_character1, &selected_character2, &remainingTime, deco, GoblinAnimation, currentFrame, &multicamera1, &multicamera2, world, tileIndices, numTiles, &active_money, moneyPosArray, &gamescreen, &player2, &player3, &goblin_spawn, goblinPosArray, frameloop, meat_post, &active_meat, &monmon_spawn, monmonPosArray, MonmonAnimation, CharacterAnimation, CharacterTexture, &highscoreWritten, &gamestage, gametexture);
        } 

        if (gamescreen.pause){
            DrawRectangle(0, 0, SCREENWIDTH, SCREENHEIGHT, (Color){ 0, 0, 0, 200 });
            DrawText("Game Paused", SCREENWIDTH/2 - MeasureText("Game Paused", 40)/2, 280, 40, (Color){130, 162, 159, 255});
            if(GuiButtonWithSound(loadsound.buttonclick, (Rectangle){ SCREENWIDTH/2 - 150, 380, 300, 50}, "RESUME")){
                gamescreen.pause = !gamescreen.pause;
            } else if(GuiButtonWithSound(loadsound.buttonclick, (Rectangle){ SCREENWIDTH/2 - 150, 460, 300, 50}, "RESTART")){
                InitGame(&remainingTime, &singlecamera, world, numTiles, &active_money, &moneyloop, &gamescreen, &player1, &player2, &player3, &goblin_spawn, &goblinloop, &meatloop, &active_meat, &multicamera1, &multicamera2, &monmon_spawn, &highscoreWritten, &gamestage, &monmonloop);
                gamescreen.pause = !gamescreen.pause;
            } else if(GuiButtonWithSound(loadsound.buttonclick, (Rectangle){ SCREENWIDTH/2 - 150, 540, 300, 50}, "BACK TO MENU")){
                InitGame(&remainingTime, &singlecamera, world, numTiles, &active_money, &moneyloop, &gamescreen, &player1, &player2, &player3, &goblin_spawn, &goblinloop, &meatloop, &active_meat, &multicamera1, &multicamera2, &monmon_spawn, &highscoreWritten, &gamestage, &monmonloop);
                gamescreen.showMenu = true;
                gamescreen.singleplayer = false;
                gamescreen.multiplayer = false;
                gamescreen.pause = !gamescreen.pause;
            }
        }

        if (gamestage.stage2){
            PlayMusicStream(loadsound.stage2_music);
            UpdateMusicStream(loadsound.stage2_music);
        } else if (gamestage.stage3){
            PlayMusicStream(loadsound.stage3_music);
            UpdateMusicStream(loadsound.stage3_music);
        }

        if (gamescreen.gameOver){
            if (!gameEndSound){
                if (remainingTime <= 0){
                    PlaySound(loadsound.gamecomplete);
                } else {
                    PlaySound(loadsound.gameoversound);
                }
                gameEndSound = true;
            }
            gamestage.stage1 = false;
            gamestage.stage2 = false;
            gamestage.stage3 = false;
            if (GuiButtonWithSound(loadsound.buttonclick, (Rectangle){ SCREENWIDTH/2 - 150, 490, 300, 50}, "REPLAY")){
                InitGame(&remainingTime, &singlecamera, world, numTiles, &active_money, &moneyloop, &gamescreen, &player1, &player2, &player3, &goblin_spawn, &goblinloop, &meatloop, &active_meat, &multicamera1, &multicamera2, &monmon_spawn, &highscoreWritten, &gamestage, &monmonloop);
                gameEndSound = false;
                gamescreen.gameOver = false;
            } else if (GuiButtonWithSound(loadsound.buttonclick, (Rectangle){ SCREENWIDTH/2 - 150, 550, 300, 50}, "BACK TO MENU")){
                InitGame(&remainingTime, &singlecamera, world, numTiles, &active_money, &moneyloop, &gamescreen, &player1, &player2, &player3, &goblin_spawn, &goblinloop, &meatloop, &active_meat, &multicamera1, &multicamera2, &monmon_spawn, &highscoreWritten, &gamestage, &monmonloop);
                gameEndSound = false;
                gamescreen.gameOver = false;
                gamescreen.showMenu = true;
                gamescreen.singleplayer = false;
                gamescreen.multiplayer = false;
            }
        } 
        EndDrawing();
    }     

    UnloadMusicStream(loadsound.gameplay_music);
    UnloadSound(loadsound.WalkingSound);
    UnloadSound(loadsound.SwordSound);
    UnloadSound(loadsound.buttonclick);
    UnloadSound(loadsound.MoneySound);
    UnloadMusicStream(loadsound.stage2_music);
    UnloadSound(loadsound.healsound);
    UnloadSound(loadsound.monsterdead);
    UnloadSound(loadsound.minusheart);
    UnloadSound(loadsound.monmonsound);
    UnloadSound(loadsound.gamecomplete);
    UnloadSound(loadsound.gameoversound);
    CloseAudioDevice();
    UnloadImage(icon);
    UnloadTexture(gametexture.maptile);
    UnloadTexture(deco.tent1);
    UnloadTexture(deco.tent2);
    UnloadTexture(deco.tree);
    UnloadTexture(deco.bush);
    UnloadTexture(deco.fire);
    UnloadTexture(deco.wood);
    UnloadTexture(deco.pumpkin);
    UnloadTexture(gametexture.money);
    UnloadTexture(deco.mushroom);
    UnloadTexture(deco.scarecrow);
    UnloadTexture(CharacterTexture);
    UnloadTexture(MenuTexture);
    UnloadTexture(gametexture.heart);
    UnloadTexture(gametexture.meat);
    UnloadTexture(gametexture.moneyIdle);
    UnloadTexture(gametexture.skull);
    UnloadTexture(gametexture.keyboard);
    UnloadTexture(gametexture.keyboard_single);
    UnloadFont(romulus);
    for (int i = 0; i < MAX_CHARACTER_TYPE; i++) {
        UnloadTexture(GoblinAnimation[i]);
    }
    for (int i = 0; i < MAX_CHARACTER_TYPE; i++) {
        UnloadTexture(CharacterAnimation[i]);
    }
    free(goblinPosArray);
    CloseWindow();
    return 0;
}