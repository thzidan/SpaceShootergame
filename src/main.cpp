#include <iostream>
#include "raylib.h"
#include <vector>
#include <ctime>

using namespace std;

static const int screenWidth = 600;
static const int screenHeight = 800;
static const float scrollingSpeed = 2.0f;
static const int maxLives = 3;
static const int maxEnemies = 4; // Number of different enemy images
static const int maxBullets = 50;

class Background
{
public:
    Texture2D background;
    float scrollingBack;

    Background()
    {
        background = LoadTexture("assets/background.png");
        scrollingBack = 0.0f;
    }

    void Draw()
    {
        scrollingBack += scrollingSpeed;

        if (scrollingBack >= background.height)
        {
            scrollingBack = 0;
        }

        DrawTextureEx(background, (Vector2){0, scrollingBack}, 0.0f, 1.0f, WHITE);
        DrawTextureEx(background, (Vector2){0, scrollingBack - background.height}, 0.0f, 1.0f, WHITE);
    }

    ~Background()
    {
        UnloadTexture(background);
    }
};

class Player
{
public:
    Texture2D playerimg;
    float initialX;
    float initialY;
    float speedX;
    int lives;
    Rectangle collisionRec;

    Player()
    {
        playerimg = LoadTexture("assets/spaceship.png");
        initialX = screenWidth / 2;
        initialY = screenHeight - playerimg.height - 20;
        speedX = 10;
        lives = maxLives;
        collisionRec = {initialX, initialY, (float)playerimg.width, (float)playerimg.height};
    }

    void Draw()
    {
        DrawTextureEx(playerimg, (Vector2){initialX, initialY}, 0.0f, 1.0f, WHITE);
    }

    void Update()
    {
        if (IsKeyDown(KEY_RIGHT))
            initialX += speedX;
        if (IsKeyDown(KEY_LEFT))
            initialX -= speedX;

        if (initialX <= 0)
            initialX = 0;
        if (initialX + playerimg.width >= screenWidth)
            initialX = screenWidth - playerimg.width;

        collisionRec.x = initialX;

        DrawTextureEx(playerimg, (Vector2){initialX, initialY}, 0.0f, 1.0f, WHITE);
    }

    ~Player()
    {
        UnloadTexture(playerimg);
    }
};

class Bullet
{
public:
    Rectangle bulletRect;
    float speed;
    bool active;

    Bullet(float x, float y)
    {
        bulletRect = {x, y, 5, 10}; // Red rectangle bullet
        speed = 10;
        active = true;
    }

    void Draw()
    {
        if (active)
        {
            DrawRectangleRec(bulletRect, RED);
        }
    }

    void Update()
    {
        if (active)
        {
            bulletRect.y -= speed;

            if (bulletRect.y + bulletRect.height < 0)
            {
                active = false;
            }
        }
    }
};

class Enemy
{
public:
    Texture2D enemyImg;
    float x;
    float y;
    float speed;
    bool active;

    Enemy(float x, float y, const char *imagePath)
    {
        enemyImg = LoadTexture(imagePath); // Load the enemy image
        this->x = x;
        this->y = y;
        speed = 2;
        active = true;
    }

    void Draw()
    {
        if (active)
        {
            DrawTextureEx(enemyImg, (Vector2){x, y}, 0.0f, 1.0f, WHITE);
        }
    }

    void Update()
    {
        if (active)
        {
            y += speed;

            if (y > screenHeight)
            {
                active = false;
            }
        }
    }

    ~Enemy()
    {
        UnloadTexture(enemyImg);
    }
};

class Game
{
public:
    Player player = Player();
    vector<Bullet> bullets;
    vector<Enemy> enemies;
    vector<const char*> enemyImages = { "assets/enemy1.png", "assets/enemy2.png", "assets/enemy3.png", "assets/enemy4.png" }; // Paths to different enemy images

    int score;
    int highScore;
    bool gameOver;

    Game()
    {
        player.Draw();
        score = 0;
        highScore = 0;
        gameOver = false;
        srand(static_cast<unsigned>(time(0)));

        // Initialize enemies
        for (int i = 0; i < maxEnemies; i++)
        {
            SpawnEnemy();
        }
    }

    void Update()
    {
        if (gameOver)
        {
            if (IsKeyPressed(KEY_R))
            {
                ResetGame();
            }
            return;
        }

        player.Update();

        // Update bullets
        for (int i = 0; i < bullets.size(); i++)
        {
            bullets[i].Update();
            if (!bullets[i].active)
            {
                bullets.erase(bullets.begin() + i);
            }
        }

        // Shoot bullets
        if (IsKeyPressed(KEY_SPACE))
        {
            if (bullets.size() < maxBullets)
            {
                Bullet bullet(player.initialX + player.playerimg.width / 2 - 2.5f, player.initialY);
                bullets.push_back(bullet);
            }
        }

        // Update enemies
        for (int i = 0; i < enemies.size(); i++)
        {
            enemies[i].Update();
            if (!enemies[i].active)
            {
                enemies.erase(enemies.begin() + i);
                SpawnEnemy();
            }
        }

        // Check collisions between bullets and enemies
        for (int i = 0; i < bullets.size(); i++)
        {
            for (int j = 0; j < enemies.size(); j++)
            {
                if (CheckCollisionRecs(bullets[i].bulletRect, (Rectangle){enemies[j].x, enemies[j].y, enemies[j].enemyImg.width, enemies[j].enemyImg.height}) && bullets[i].active && enemies[j].active)
                {
                    bullets[i].active = false;
                    enemies[j].active = false;
                    score++;
                }
            }
        }

        // Check collisions between player and enemies
        for (int i = 0; i < enemies.size(); i++)
        {
            if (CheckCollisionRecs(player.collisionRec, (Rectangle){enemies[i].x, enemies[i].y, enemies[i].enemyImg.width, enemies[i].enemyImg.height}) && enemies[i].active)
            {
                player.lives--;
                enemies[i].active = false;
                if (player.lives <= 0)
                {
                    gameOver = true;
                }
            }
        }
    }

    void Draw()
    {
        player.Draw();

        // Draw bullets
        for (int i = 0; i < bullets.size(); i++)
        {
            bullets[i].Draw();
        }

        // Draw enemies
        for (int i = 0; i < enemies.size(); i++)
        {
            enemies[i].Draw();
        }

        // Draw score and lives
        DrawText(TextFormat("Score: %d", score), 10, 10, 20, WHITE);
        DrawText(TextFormat("Lives: %d", player.lives), 10, 40, 20, WHITE);
        DrawText(TextFormat("High Score: %d", highScore), 10, 70, 20, WHITE);

        // Game over text
        if (gameOver)
        {
            DrawText("GAME OVER", screenWidth / 2 - MeasureText("GAME OVER", 40) / 2, screenHeight / 2 - 20, 40, RED);
            DrawText("Press 'R' to restart", screenWidth / 2 - MeasureText("Press 'R' to restart", 20) / 2, screenHeight / 2 + 20, 20, WHITE);
        }
    }

    void ResetGame()
    {
        // Reset player
        player.initialX = screenWidth / 2;
        player.initialY = screenHeight - player.playerimg.height - 20;
        player.collisionRec = {player.initialX, player.initialY, (float)player.playerimg.width, (float)player.playerimg.height};
        player.lives = maxLives;

        // Reset score
        if (score > highScore)
        {
            highScore = score;
        }
        score = 0;

        gameOver = false;

        // Reset enemies
        enemies.clear();
        for (int i = 0; i < maxEnemies; i++)
        {
            SpawnEnemy();
        }
    }

    void SpawnEnemy()
    {
        float xPos = static_cast<float>(GetRandomValue(0, screenWidth - 64));
        float yPos = static_cast<float>(GetRandomValue(-500, -50));
        const char* enemyImagePath = enemyImages[GetRandomValue(0, maxEnemies - 1)]; // Randomly select an enemy image path
        enemies.push_back(Enemy(xPos, yPos, enemyImagePath));
    }
};

int main()
{
    cout << "Starting the game..." << endl;
    InitWindow(screenWidth, screenHeight, "Space Invaders");
    SetTargetFPS(60);

    Background bg = Background();
    Game game = Game();

    while (!WindowShouldClose())
    {
        BeginDrawing();
        game.Update();
        ClearBackground(DARKBLUE);
        bg.Draw();
        game.Draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
