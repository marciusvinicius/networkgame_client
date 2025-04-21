#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h>

#define MAX_ENTITIES 200

#define BULLET_SPEED 100.0f

typedef struct Entity
{
    bool isAlive;
    Vector2 position;
    Vector2 velocity;
    Vector2 direction;
    Color color;

} Entity;

typedef struct Bullet
{
    int ownerId; // Player ID of the owner
    Entity entity;

} Bullet;

typedef struct Player
{
    int id;
    Entity entity;
    char name[32];       // Player name
    float lastShootTime; // Time of the last shoot
    float shootCooldown; // Cooldown time between shots
} Player;

// Local version of the game state
typedef struct GameState
{
    Player player;
    Entity entities[MAX_ENTITIES];
    int entitiesCount;
    float deltaTime;

} GameState;

void shootBulletLocally(GameState *gameState, Vector2 direction)
{
    // Add some interval btwen shoots
    if (gameState->player.lastShootTime + gameState->player.shootCooldown > GetTime())
        return; // Don't shoot yet

    gameState->player.lastShootTime = GetTime(); // Update last shoot time

    // Create a bullet and send it to the server
    Bullet bullet;
    bullet.ownerId = gameState->player.id; // Set the owner ID to the player ID
    bullet.entity.isAlive = true;
    bullet.entity.position = gameState->player.entity.position;
    bullet.entity.velocity = Vector2{BULLET_SPEED, BULLET_SPEED}; // Initialize velocity to zero
    bullet.entity.direction = direction;                          // Use the provided direction
    bullet.entity.color = RED;

    gameState->entities[gameState->entitiesCount++] = bullet.entity; // Add bullet to local game state
}

void DrawAndUpdateBullet(GameState *gameState)
{
    for (int i = 0; i < gameState->entitiesCount; i++)
    {
        Entity *bullet = &gameState->entities[i];

        if (bullet->isAlive)
        {
            // update bullet position based on velocity and direction
            bullet->position.x += bullet->velocity.x * bullet->direction.x * 0.1f;
            bullet->position.y += bullet->velocity.y * bullet->direction.y * 0.1f;

            DrawRectangleV(bullet->position, Vector2{10, 10}, RED);
            // Check if bullet is out of bounds
            if (bullet->position.x < 0 || bullet->position.x > GetScreenWidth() ||
                bullet->position.y < 0 || bullet->position.y > GetScreenHeight())
            {
                bullet->isAlive = false; // Mark bullet as not alive
            }
        }
    }
}

void DrawAndUpdatePlayer(GameState *gameState)
{

    if (gameState->player.entity.isAlive)
    {
        Player player = gameState->player;
        // Draw player
        DrawRectangleV(player.entity.position, Vector2{20, 20}, player.entity.color);
        DrawText(player.name, player.entity.position.x - 10, player.entity.position.y - 30, 20, player.entity.color);
    }
}

void handleInput(GameState *gameState)
{
    // Add friction to velocity
    gameState->player.entity.velocity.x *= 0.9f;
    gameState->player.entity.velocity.y *= 0.9f;

    // make direction vector based on mouse position
    // Update velocity based on input

    if (IsKeyDown(KEY_W))
        gameState->player.entity.velocity.y -= 1;
    if (IsKeyDown(KEY_S))
        gameState->player.entity.velocity.y += 1;
    if (IsKeyDown(KEY_A))
        gameState->player.entity.velocity.x -= 1;
    if (IsKeyDown(KEY_D))
        gameState->player.entity.velocity.x += 1;

    gameState->player.entity.position.x += gameState->player.entity.velocity.x * 0.5f;
    gameState->player.entity.position.y += gameState->player.entity.velocity.y * 0.5f;

    // Clamp player position to screen bounds
    if (gameState->player.entity.position.x < 0)
        gameState->player.entity.position.x = 0;
    if (gameState->player.entity.position.x > GetScreenWidth() - 20)
        gameState->player.entity.position.x = GetScreenWidth() - 20;
    if (gameState->player.entity.position.y < 0)
        gameState->player.entity.position.y = 0;
    if (gameState->player.entity.position.y > GetScreenHeight() - 20)
        gameState->player.entity.position.y = GetScreenHeight() - 20;

    // Check for shooting
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        // Shoot a bullet
        shootBulletLocally(gameState, gameState->player.entity.direction); // Replace 1 with actual player ID
    }
}

void InitGameState(GameState *gameState)
{
    gameState->player.id = 0;
    gameState->player.entity.isAlive = true;
    gameState->player.entity.position = Vector2{0, 0};
    gameState->player.entity.color = BLUE;
    gameState->player.entity.velocity = Vector2{0, 0};
    gameState->player.entity.direction = Vector2{0, 0};
    gameState->player.shootCooldown = 0.5f; // 500ms cooldown
    gameState->player.lastShootTime = 0.0f;

    gameState->entitiesCount = 0;
}

int main()
{
    InitWindow(800, 600, "TCP Client Game - Raylib");
    SetTargetFPS(60);

    int playerId = 1; // Replace with actual player ID
    int socket = 0;   // Replace with actual socket initialization

    // Connect to server (not implemented in this example)

    char playerIdChar[32];
    itoa(playerId, playerIdChar, 10); // Convert player ID to stringname

    GameState gameState;
    InitGameState(&gameState);

    while (!WindowShouldClose())
    {

        // Draw aime line to mouse position from the center of the player
        Vector2 mousePos = GetMousePosition();
        Vector2 centerPlayerPos = {gameState.player.entity.position.x + 10, gameState.player.entity.position.y + 10};

        Vector2 aimDirection = {mousePos.x - centerPlayerPos.x, mousePos.y - centerPlayerPos.y};
        float length = sqrtf(aimDirection.x * aimDirection.x + aimDirection.y * aimDirection.y);
        Vector2 aimEndPos = {centerPlayerPos.x + aimDirection.x, centerPlayerPos.y + aimDirection.y};

        gameState.player.entity.direction = aimDirection / length; // Normalize the direction vector

        handleInput(&gameState);

        BeginDrawing();
        ClearBackground(PURPLE);

        DrawText("Use WASD to move", 10, 40, 20, DARKGRAY);
        DrawAndUpdatePlayer(&gameState);

        // DrawLineV(playerPos, aimEndPos, RED);
        DrawLineEx(centerPlayerPos, aimEndPos, 2, RED);
        DrawRectangleV(aimEndPos, Vector2{10, 10}, RED);

        DrawAndUpdateBullet(&gameState);

        // Show Entities count on screen
        char buffer[32];
        sprintf(buffer, "Entities: %d", gameState.entitiesCount);
        DrawText(buffer, 10, 10, 20, DARKGRAY);
        // Show Player ID on screen
        sprintf(buffer, "Player ID: %d", gameState.player.id);
        DrawText(buffer, 10, 70, 20, DARKGRAY);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}