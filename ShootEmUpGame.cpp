#include <iostream>
#include <Windows.h>
#include <fstream>
#include <vector>
#include <stdarg.h>

char Field[50][70];
int PlayerPosX = 50, PlayerPosY = 43, TimeLeftBeforeYouCanShoot = 0, TineLeftBeforeSparkDisappears = 0, Distance = 0, TrailPosX = PlayerPosX + 1, TrailPosY = PlayerPosY + 4 + Distance;
int Colors[] = { 7, 6, 4 };
float CurrentFrame = 0.0f;
bool running = true, Paused = false, IsInMainMenu = true;
std::string MenuOptions[] = { "resume  ", "quit    ", "play    " };

struct Bullet
{
    int x, y;
};

struct Enemy
{
    int x, y;
    float frame;
    int distance_between_enemy_and_trail;
    bool dead;
};

std::vector<Bullet> Bullets;
std::vector<Enemy> Enemies;

void SetConsoleWindowSize()
{
    HWND console = GetConsoleWindow();
    RECT ConsoleRect;
    GetWindowRect(console, &ConsoleRect);
    MoveWindow(console, ConsoleRect.left, ConsoleRect.top, 1000, 900, TRUE);
}

void PlayAnimation(std::string AnimationName, int PosX, int PosY, float& Frame, int AnimationLength, float Step, int SpriteWidth)
{
    int x = 0, y = 0;
    std::string filename = AnimationName;
    filename += char(int(Frame) + 48);
    filename += ".txt";
    std::ifstream file(filename);
    if (!file.good()) return;
    while (!file.eof())
    {
        char pixel;
        file >> pixel;
        switch (pixel)
        {
        case '0': pixel = char(219); break;
        case '#': pixel = char(177); break;
        default: pixel = ' '; break;
        }
        if ((PosY + y > 0 && PosY + y < 49) && (PosX + x > 1 && PosX + x < 68)) Field[PosY + y][PosX + x] = pixel;
        x++;
        if (x == SpriteWidth)
        {
            x = 0; y++;
        }
    }
    file.close();
    if (!Paused) Frame += Step;
    if (Frame > AnimationLength) Frame = 0.0f;
}

void DrawField(HANDLE* HND)
{
    int TrailPosX = PlayerPosX + 1;
    int TrailPosY = PlayerPosY + 3 + Distance;
    for (int i = 0; i < 50; i++)
        for (int j = 0; j < 70; j++) ((i == 0 || i == 49 || j == 0 || j == 1 || j == 68 || j == 69) ? (Field[i][j] = char(219)) : (Field[i][j] = ' '));
    for (int i = 0; i < Bullets.size(); i++) Field[Bullets[i].y][Bullets[i].x] = char(219);
    if (TineLeftBeforeSparkDisappears != 0)
    {
        for (int i = -2; i < 3; i++) ((i == -2 || i == 2) ? (Field[PlayerPosY - 2][PlayerPosX + 2 + i] = char(220)) : (Field[PlayerPosY - 2][PlayerPosX + 2 + i] = char(219)));
        Field[PlayerPosY - 3][PlayerPosX + 2] = char(220);
    }
    for (int i = 0; i < Enemies.size(); i++)
    {
        if (!Enemies[i].dead)
        {
            PlayAnimation("Enemy////enemy_anim", Enemies[i].x, Enemies[i].y, Enemies[i].frame, 4, 0.2f, 5);
            if (Enemies[i].y - 1 - Enemies[i].distance_between_enemy_and_trail > 0 && Enemies[i].y - 1 - Enemies[i].distance_between_enemy_and_trail < 49) Field[Enemies[i].y - 1 - Enemies[i].distance_between_enemy_and_trail][Enemies[i].x + 2] = char(Enemies[i].distance_between_enemy_and_trail + 48);
        }
        else PlayAnimation("Explosion////explosion_anim", Enemies[i].x - 2, Enemies[i].y - 1, Enemies[i].frame, 3, 1.0f, 9);
    }
    PlayAnimation("Player////anim", PlayerPosX, PlayerPosY, CurrentFrame, 4, 0.2f, 5);
    for (int i = 0; i < 50; i++)
    {
        for (int j = 0; j < 70; j++)
        {
            bool AlreadyPrintedSomething = false;
            switch (Field[i][j])
            {
            case '0':
                SetConsoleTextAttribute(*HND, 7); std::cout << char(219); AlreadyPrintedSomething = true; break;
            case '1':
                SetConsoleTextAttribute(*HND, 6); std::cout << char(219); AlreadyPrintedSomething = true; SetConsoleTextAttribute(*HND, 7); break;
            case '2':
                SetConsoleTextAttribute(*HND, 4); std::cout << char(219); AlreadyPrintedSomething = true; SetConsoleTextAttribute(*HND, 7); break;
            }
            if (i == TrailPosY && (j == TrailPosX || j == TrailPosX + 1 || j == TrailPosX + 2))
            {
                SetConsoleTextAttribute(*HND, Colors[Distance]);
                std::cout << char(219);
                AlreadyPrintedSomething = true;
                SetConsoleTextAttribute(*HND, 7);
            }
            if (!AlreadyPrintedSomething) std::cout << Field[i][j];
        }
        std::cout << '\n';
    }
    if (!Paused) ((Distance != 2) ? (Distance++) : (Distance = 0));
}

void DrawWindow(short int PosX, short int PosY, int width, int height, HANDLE* HND, int Color)
{
    char c;
    SetConsoleTextAttribute(*HND, Color);
    for (short int i = PosY; i < PosY + height; i++)
    {
        SetConsoleCursorPosition(*HND, { PosX, i });
        for (int j = 0; j < width; j++)
        {
            (i == PosY || i == PosY + height - 1 || j == 0 || j == 1 || j == width - 2 || j == width - 1) ? (c = char(219)) : (c = ' ');
            std::cout << c;
        }
    }
    SetConsoleTextAttribute(*HND, 7);
}

void CreateMenu(HANDLE* HND, short int PosX, short int PosY, int CursorPosY, short int NumberOfOptions, ...)
{
    DrawWindow(PosX, PosY, 30, NumberOfOptions + 4, &*HND, 7);
    va_list menu_options;
    va_start(menu_options, NumberOfOptions);
    for (short int i = PosY + 2; i < PosY + 2 + NumberOfOptions; i++)
    {
        SetConsoleCursorPosition(*HND, { PosX + 3, i });
        int menu_option = va_arg(menu_options, int);
        (i == PosY + 2 + CursorPosY) ? (SetConsoleTextAttribute(*HND, 112)) : (SetConsoleTextAttribute(*HND, 7));
        std::cout << MenuOptions[menu_option];
        SetConsoleTextAttribute(*HND, 7);
    }
    va_end(menu_options);
}

void DrawConsoleText(short int PosX, short int PosY, std::string Text, HANDLE* HND)
{
    SetConsoleCursorPosition(*HND, { PosX, PosY });
    std::cout << Text;
}

void CheckIfBulletsCollideWithEnemies(int &score)
{
    for (int i = 0; i < Bullets.size(); i++)
        for (int j = 0; j < Enemies.size(); j++)
            if ((Bullets[i].x >= Enemies[j].x && Bullets[i].x <= Enemies[j].x + 4) && (Bullets[i].y >= Enemies[j].y && Bullets[i].y <= Enemies[j].y + 2) && !Enemies[j].dead)
            {
                Enemies[j].dead = true;
                Enemies[j].frame = 0.0f;
                Bullets.erase(Bullets.begin() + i);
                score++;
                break;
            }
}

void MoveBullets(int &SCORE)
{
    for (int i = 0; i < Bullets.size(); i++)
    {
        Bullets[i].y--;
        if (Bullets[i].y <= 0) Bullets.erase(Bullets.begin() + i);
    }
    CheckIfBulletsCollideWithEnemies(SCORE);
}

void MoveEnemies()
{
    for (int i = 0; i < Enemies.size(); i++)
    {
        if (Enemies[i].dead && Enemies[i].frame > 3) Enemies.erase(Enemies.begin() + i);
        else
        {
            Enemies[i].y++;
            (Enemies[i].distance_between_enemy_and_trail != 2) ? (Enemies[i].distance_between_enemy_and_trail++) : (Enemies[i].distance_between_enemy_and_trail = 0);
            if (Enemies[i].y > 51) Enemies.erase(Enemies.begin() + i);
        } 
    }
}

void RestartGame()
{
    if (!Enemies.empty()) Enemies.clear();
    if (!Bullets.empty()) Bullets.clear();
    PlayerPosX = 50, PlayerPosY = 43, TimeLeftBeforeYouCanShoot = 0, TineLeftBeforeSparkDisappears = 0, Distance = 0, TrailPosX = PlayerPosX + 1, TrailPosY = PlayerPosY + 4 + Distance;
    CurrentFrame = 0.0f;
}

int main()
{
    register float Temperture = 0.0f;
    int Timer = 0, Score = 0, CursorPositionY = 0;
    HANDLE hnd = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO ci;
    ci.dwSize = 100;
    ci.bVisible = FALSE;
    while (running)
    {
        //SetConsoleWindowSize();
        SetConsoleCursorInfo(hnd, &ci);
        SetConsoleCursorPosition(hnd, { 0, 0 });
        if (!IsInMainMenu)
        {
            DrawField(&hnd);
            DrawConsoleText(80, 0, "Temperture:", &hnd);
            DrawWindow(80, 1, 30, 3, &hnd, 7);
            DrawWindow(82, 2, 26, 1, &hnd, 8);
            (int(Temperture) <= 26) ? (DrawWindow(82, 2, int(Temperture), 1, &hnd, 4)) : (DrawWindow(82, 2, 26, 1, &hnd, 4));
            if (!Paused)
            {
                if (Temperture > 0.0f) Temperture -= 0.02f;
                if (Timer == 0)
                {
                    Enemies.push_back({ rand() % 60 + 5, -50, 0.0f, 0, false }); Timer = rand() % 20 + 10;
                }
                if (Timer) Timer--;
                if (!Bullets.empty()) MoveBullets(Score);
                if (!Enemies.empty()) MoveEnemies();

                if ((GetAsyncKeyState('D') & 0x8000) && Field[PlayerPosY][PlayerPosX + 5] != char(219)) PlayerPosX++;
                if ((GetAsyncKeyState('A') & 0x8000) && Field[PlayerPosY][PlayerPosX - 1] != char(219)) PlayerPosX--;
                if ((GetAsyncKeyState('W') & 0x8000) && Field[PlayerPosY - 1][PlayerPosX] != char(219)) PlayerPosY--;
                if ((GetAsyncKeyState('S') & 0x8000) && Field[PlayerPosY + 6][PlayerPosX] != char(219)) PlayerPosY++;
                if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000)) Paused = true;
                if ((GetAsyncKeyState(VK_RETURN) & 0x8000) && TimeLeftBeforeYouCanShoot == 0)
                {
                    Bullets.push_back({ PlayerPosX + 2, PlayerPosY - 2 }); TimeLeftBeforeYouCanShoot = 3; TineLeftBeforeSparkDisappears = 2; Temperture += 0.2f;
                }
                if (TimeLeftBeforeYouCanShoot) TimeLeftBeforeYouCanShoot--;
                if (TineLeftBeforeSparkDisappears) TineLeftBeforeSparkDisappears--;
            }
            else
            {
                CreateMenu(&hnd, 80, 10, CursorPositionY, 2, 0, 1);
                if ((GetAsyncKeyState(VK_UP) & 0x8000)) ((CursorPositionY != 0) ? (CursorPositionY--) : (CursorPositionY = 1));
                if ((GetAsyncKeyState(VK_DOWN) & 0x8000)) ((CursorPositionY != 1) ? (CursorPositionY++) : (CursorPositionY = 0));
                if ((GetAsyncKeyState(VK_RETURN) & 0x8000))
                    switch (CursorPositionY)
                    {
                    case 0: Paused = false; system("cls"); break;
                    case 1: IsInMainMenu = true; Paused = false; system("cls"); break;
                    }
            }
        }
        else
        {
            DrawWindow(10, 2, 30, 5, &hnd, 7);
            DrawConsoleText(13, 4, "Some sort of a game", &hnd);
            CreateMenu(&hnd, 10, 10, CursorPositionY, 2, 2, 1);
            if ((GetAsyncKeyState(VK_UP) & 0x8000)) ((CursorPositionY != 0) ? (CursorPositionY--) : (CursorPositionY = 1));
            if ((GetAsyncKeyState(VK_DOWN) & 0x8000)) ((CursorPositionY != 1) ? (CursorPositionY++) : (CursorPositionY = 0));
            if ((GetAsyncKeyState(VK_RETURN) & 0x8000))
                switch (CursorPositionY)
                {
                case 0: IsInMainMenu = false; system("cls"); RestartGame(); break;
                case 1: running = false; break;
                }
        }
        Sleep(100 * (Paused || IsInMainMenu));
    }
    return 0;
}