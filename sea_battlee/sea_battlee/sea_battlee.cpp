#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <algorithm>
#include <cstdlib>
#include <ctime>

using namespace std;

//КОНСТАНТИ
const char EMPTY = '~';
const char SHIP = 'S';
const char HIT = 'X';
const char MISS = '*';
const int FD_SIZE = 10;
const char LETT[FD_SIZE] = { 'A','B','C','D','E','F','G','H','I','J' };

//СТРУКТУРА
struct Ship {
    int x, y;
    int size;
    bool horizontal;
    int hits = 0;
    bool sunk = false;
};

vector<Ship> player1Ships;
vector<Ship> player2Ships;

void setColor(int color);
void resetColor();
void clearScreen();
void printHeader();
void printColumnNumbers();
void printRow(const char field[FD_SIZE][FD_SIZE], int row, bool isMyField, bool showMyShips);
void printBothFields(const char field1[FD_SIZE][FD_SIZE], const char field2[FD_SIZE][FD_SIZE], bool showMyShips);
bool canPlaceShip(char field[FD_SIZE][FD_SIZE], int x, int y, int size, bool horizontal);
bool placeShip(char field[FD_SIZE][FD_SIZE], vector<Ship>& ships, int x, int y, int size, bool horizontal);
void placeShipsRandom(char field[FD_SIZE][FD_SIZE], vector<Ship>& ships);
bool makeShot(char field[FD_SIZE][FD_SIZE], int x, int y);
bool allShipsSunk(const char field[FD_SIZE][FD_SIZE]);
bool checkIfSunk(char field[FD_SIZE][FD_SIZE], vector<Ship>& ships, int hitX, int hitY);
void manualPlaceShips(char field[FD_SIZE][FD_SIZE], vector<Ship>& ships);
void showRules();
void playGame();
void showMainMenu();
int letterToIndex(char ch);
bool parseCoordinate(const string& input, int& x, int& y);

//БАЗОВІ ФУНКЦІЇ
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void resetColor() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}

void clearScreen() {
    system("cls");
}

//ФУНКЦІЇ АНІМАЦІЇ ТА ПІДСВІТКИ
void animateSeaBattleTitle() {
    const string title = "  <<<  М О Р С Ь К И Й   Б І Й  >>>  ";
    const int colors[] = { 9, 11, 10, 14, 12, 13, 11, 9 };
    const int numColors = 8;

    for (int frame = 0; frame < 3; frame++) {
        clearScreen();
        for (int i = 0; i < numColors; i++) {
            setColor(colors[i]);
            int shift = (frame * 2 + i) % numColors;
            cout << "\n\n";
            setColor(colors[shift]);
            cout << "    ";
            for (size_t k = 0; k < title.length() + 4; k++) cout << "~";
            cout << "\n    ~";
            for (size_t k = 0; k < title.length(); k++) {
                setColor(colors[(shift + (int)k) % numColors]);
                cout << title[k];
            }
            setColor(colors[shift]);
            cout << "~\n    ";
            for (size_t k = 0; k < title.length() + 4; k++) cout << "~";
            resetColor();
            cout << "\n\n";
            Sleep(200);
            clearScreen();
        }
    }
    clearScreen();
    setColor(11);
    cout << "\n\n    ";
    for (size_t k = 0; k < title.length() + 4; k++) cout << "=";
    cout << "\n    =";
    setColor(14);
    for (char c : title) {
        Sleep(30);
        cout << c;
    }
    setColor(11);
    cout << "=\n    ";
    for (size_t k = 0; k < title.length() + 4; k++) cout << "=";
    resetColor();
    cout << "\n\n";
    Sleep(500);
}
void printPersistentTitle() {
    setColor(11);
    cout << "======================================================================\n";
    setColor(14);
    cout << "           <<<  М О Р С Ь К И Й   Б І Й  >>>           [><]   \n";
    setColor(11);
    cout << "======================================================================\n";
    resetColor();
}

// Анімація хвильової лінії
void printWaveLine(int length, int color1, int color2) {
    for (int i = 0; i < length; i++) {
        if (i % 2 == 0) setColor(color1);
        else setColor(color2);
        cout << "~";
    }
    resetColor();
}

void printRowWithSunk(const char field[FD_SIZE][FD_SIZE], int row, bool isMyField, bool showMyShips,
    const vector<Ship>& ships) {
    cout << "  " << LETT[row] << " |";
    for (int col = 0; col < FD_SIZE; col++) {
        char cell = field[row][col];

        bool isSunkPart = false;
        for (const auto& ship : ships) {
            if (ship.sunk) {
                for (int i = 0; i < ship.size; i++) {
                    int cx = ship.x + (ship.horizontal ? 0 : i);
                    int cy = ship.y + (ship.horizontal ? i : 0);
                    if (cx == row && cy == col) {
                        isSunkPart = true;
                        break;
                    }
                }
            }
        }

        if (isSunkPart && cell == HIT) {
            setColor(13); 
            cout << "#"; 
        }
        else if (cell == SHIP) {
            if (isMyField && showMyShips) setColor(10);
            cout << EMPTY;
        }
        else if (cell == HIT) {
            setColor(12); cout << cell;
        }
        else if (cell == MISS) {
            setColor(9); cout << cell;
        }
        else {
            setColor(7); cout << cell;
        }
        cout << " ";
        resetColor();
    }
    cout << "|";
}
void printBothFieldsWithSunk(const char field1[FD_SIZE][FD_SIZE], const char field2[FD_SIZE][FD_SIZE],
    bool showMyShips, const vector<Ship>& ships1, const vector<Ship>& ships2) {
    printHeader();
    printColumnNumbers();

    for (int row = 0; row < FD_SIZE; row++) {
        printRowWithSunk(field1, row, true, showMyShips, ships1);
        cout << " ";
        printRowWithSunk(field2, row, false, false, ships2);
        cout << "\n";
    }
    cout << "\n";
}

void animateExplosion(int row, int col, bool isHit) {
    setColor(14);
    cout << "\n    [";
    for (int i = 0; i < 6; i++) {
        setColor(14 + i % 2);
        cout << "*";
        Sleep(60);
    }
    setColor(isHit ? 12 : 9);
    cout << (isHit ? " БУМ! " : " буль-буль... ");
    setColor(14);
    cout << "]\n";
    resetColor();
    Sleep(100);
}

void animateSinking(const Ship& ship) {
    setColor(13);
    cout << "\n    >>> КОРАБЕЛЬ ПОТОПЛЕНО! <<<\n";
    cout << "    ";
    for (int i = 0; i < ship.size; i++) {
        setColor(12 + i % 2);
        cout << "# ";
        Sleep(150);
    }
    cout << "\n";
    resetColor();
    Sleep(300);
}
void animateShipAppear(char field[FD_SIZE][FD_SIZE], int x, int y, int size, bool horizontal) {
    for (int step = 0; step < size; step++) {
        clearScreen();
        printPersistentTitle();
        cout << "\n";
        cout << "    ";
        for (int i = 1; i <= FD_SIZE; i++) cout << i << " ";
        cout << "\n";
        for (int i = 0; i < FD_SIZE; i++) {
            cout << " " << LETT[i] << "| ";
            for (int j = 0; j < FD_SIZE; j++) {
                bool isNewShip = false;
                for (int s = 0; s <= step; s++) {
                    int cx = x + (horizontal ? 0 : s);
                    int cy = y + (horizontal ? s : 0);
                    if (i == cx && j == cy) isNewShip = true;
                }
                if (isNewShip) {
                    setColor(10 + step % 6);
                    cout << "# ";
                }
                else if (field[i][j] == SHIP) {
                    setColor(10);
                    cout << "# ";
                }
                else {
                    setColor(7);
                    cout << field[i][j] << " ";
                }
                resetColor();
            }
            cout << "|\n";
        }
        Sleep(120);
    }
    clearScreen();
    printPersistentTitle();
    cout << "\n";
    cout << "    ";
    for (int i = 1; i <= FD_SIZE; i++) cout << i << " ";
    cout << "\n";
    for (int i = 0; i < FD_SIZE; i++) {
        cout << " " << LETT[i] << "| ";
        for (int j = 0; j < FD_SIZE; j++) {
            if (field[i][j] == SHIP) {
                setColor(15);
                cout << "# ";
            }
            else {
                setColor(7);
                cout << field[i][j] << " ";
            }
            resetColor();
        }
        cout << "|\n";
    }
    Sleep(200);
}

void animateShipPlacement(char field[FD_SIZE][FD_SIZE], int x, int y, int size, bool horizontal, bool success) {
    setColor(success ? 10 : 12);
    cout << "\n    [";
    for (int i = 0; i < size; i++) {
        cout << (success ? "+" : "x");
        Sleep(80);
    }
    cout << "] " << (success ? "ГОТОВО" : "НЕМОЖЛИВО") << "\n";
    resetColor();
    Sleep(500); 
}

void animateVictory(int winner) {
    const string victory = "*** ПЕРЕМОГА ГРАВЦЯ " + to_string(winner) + "! ***";
    const int colors[] = { 14, 12, 10, 11, 13, 14 };

    for (int frame = 0; frame < 10; frame++) {
        clearScreen();
        cout << "\n\n";
        for (int i = 0; i < 5; i++) {
            setColor(colors[(frame + i) % 6]);
            for (int s = 0; s < i * 4; s++) cout << " ";
            if (i == 2) {
                cout << victory;
            }
            else {
                for (size_t k = 0; k < victory.length(); k++) cout << "* ";
            }
            cout << "\n";
        }
        Sleep(200);
    }

    clearScreen();
    setColor(14);
    cout << "\n\n";
    cout << "    ===========================================\n";
    cout << "    |  ";
    setColor(12);
    cout << "* * * ";
    setColor(14);
    cout << "ПЕРЕМОГА ГРАВЦЯ " << winner;
    setColor(12);
    cout << " * * *";
    setColor(14);
    cout << "  |\n";
    cout << "    ===========================================\n\n";
    setColor(11);
    printWaveLine(50, 11, 9);
    resetColor();
    cout << "\n";
}



//ДОПОМІЖНІ ФУНКЦІЇ
int letterToIndex(char ch) {
    char upper = toupper(static_cast<char>(ch));
    if (upper >= 'A' && upper <= 'J') return upper - 'A';
    return -1;
}

bool parseCoordinate(const string& input, int& x, int& y) {
    if (input.empty()) return false;
    string s = input;
    s.erase(remove_if(s.begin(), s.end(), [](char c) { return isspace(static_cast<unsigned char>(c)); }), s.end());
    if (s.length() < 2) return false;
    char letter = s[0];
    string numStr = s.substr(1);
    x = letterToIndex(letter);
    try { y = stoi(numStr) - 1; }
    catch (...) { return false; }
    return x != -1 && y >= 0 && y < FD_SIZE;
}

//РОЗМІЩЕННЯ
bool canPlaceShip(char field[FD_SIZE][FD_SIZE], int x, int y, int size, bool horizontal) {
    for (int i = 0; i < size; ++i) {
        int cx = x + (horizontal ? 0 : i);
        int cy = y + (horizontal ? i : 0);
        if (cx < 0 || cx >= FD_SIZE || cy < 0 || cy >= FD_SIZE) return false;
        for (int dx = -1; dx <= 1; ++dx)
            for (int dy = -1; dy <= 1; ++dy) {
                int nx = cx + dx, ny = cy + dy;
                if (nx >= 0 && nx < FD_SIZE && ny >= 0 && ny < FD_SIZE)
                    if (field[nx][ny] == SHIP) return false;
            }
    }
    return true;
}

bool placeShip(char field[FD_SIZE][FD_SIZE], vector<Ship>& ships, int x, int y, int size, bool horizontal) {
    if (!canPlaceShip(field, x, y, size, horizontal)) return false;
    for (int i = 0; i < size; ++i) {
        int cx = x + (horizontal ? 0 : i);
        int cy = y + (horizontal ? i : 0);
        field[cx][cy] = SHIP;
    }
    ships.push_back({ x, y, size, horizontal, 0, false });
    return true;
}

void placeShipsRandom(char field[FD_SIZE][FD_SIZE], vector<Ship>& ships) {
    for (int i = 0; i < FD_SIZE; i++)
        for (int j = 0; j < FD_SIZE; j++) field[i][j] = EMPTY;
    ships.clear();
    int shipSizes[] = { 4,3,2,1 };
    int shipCounts[] = { 1,2,3,4 };
    for (int type = 0; type < 4; type++) {
        for (int k = 0; k < shipCounts[type]; k++) {
            bool placed = false; int attempts = 0;
            while (!placed && attempts < 1000) {
                attempts++;
                int x = rand() % FD_SIZE;
                int y = rand() % FD_SIZE;
                bool horizontal = rand() % 2;
                placed = placeShip(field, ships, x, y, shipSizes[type], horizontal);
            }
        }
    }
}

//ВИВІД 
void printHeader() {
    cout << "    ГРАВЕЦЬ 1                  ГРАВЕЦЬ 2\n\n";
}

void printColumnNumbers() {
    cout << "     ";
    for (int i = 1; i <= 10; i++) cout << i << " ";
    cout << "      ";
    for (int i = 1; i <= 10; i++) cout << i << " ";
    cout << "\n";
}

void printRow(const char field[FD_SIZE][FD_SIZE], int row, bool isMyField, bool showMyShips) {
    cout << "  " << LETT[row] << " |";
    for (int col = 0; col < FD_SIZE; col++) {
        char cell = field[row][col];
        if (cell == SHIP) {
            if (isMyField && showMyShips) setColor(10);
            cout << EMPTY;
        }
        else if (cell == HIT) {
            setColor(12); cout << cell;
        }
        else if (cell == MISS) {
            setColor(9); cout << cell;
        }
        else {
            setColor(7); cout << cell;
        }
        cout << " ";
        resetColor();
    }
    cout << "|";
}

void printBothFields(const char field1[FD_SIZE][FD_SIZE], const char field2[FD_SIZE][FD_SIZE], bool showMyShips) {
    printHeader();
    printColumnNumbers();
    for (int row = 0; row < FD_SIZE; row++) {
        printRow(field1, row, true, showMyShips);
        cout << " ";
        printRow(field2, row, false, false);
        cout << "\n";
    }
    cout << "\n";
}

//ЛОГІКА ГРИ
bool makeShot(char field[FD_SIZE][FD_SIZE], int x, int y) {
    if (field[x][y] == HIT || field[x][y] == MISS) return false;
    field[x][y] = (field[x][y] == SHIP ? HIT : MISS);
    return field[x][y] == HIT;
}

bool allShipsSunk(const char field[FD_SIZE][FD_SIZE]) {
    for (int i = 0; i < FD_SIZE; i++)
        for (int j = 0; j < FD_SIZE; j++)
            if (field[i][j] == SHIP) return false;
    return true;
}

bool checkIfSunk(char field[FD_SIZE][FD_SIZE], vector<Ship>& ships, int hitX, int hitY) {
    for (auto& ship : ships) {
        if (ship.sunk) continue;
        bool isThisShip = false;
        int shipHits = 0;
        for (int i = 0; i < ship.size; ++i) {
            int cx = ship.x + (ship.horizontal ? 0 : i);
            int cy = ship.y + (ship.horizontal ? i : 0);
            if (cx == hitX && cy == hitY) isThisShip = true;
            if (field[cx][cy] == HIT) shipHits++;
        }
        if (isThisShip) {
            ship.hits = shipHits;
            if (shipHits == ship.size) {
                ship.sunk = true;
                animateSinking(ship);
                return true;
            }
            break;
        }
    }
    return false;
}

//ГОЛОВНЕ МЕНЮ
void showRules() {
    clearScreen();
    printPersistentTitle();
    cout << "\n";
    setColor(11);
    cout << "==================== ПРАВИЛА ГРИ ====================\n\n";
    resetColor();
    cout << "1. Морський бій — класична гра для двох гравців.\n";
    cout << "2. Кожен гравець має флот:\n";
    cout << " -> 1 чотирипалубний корабель\n";
    cout << " -> 2 трипалубних кораблі\n";
    cout << " -> 3 двопалубних кораблі\n";
    cout << " -> 4 однопалубних кораблі\n";
    cout << "3. Кораблі не можуть торкатися один одного (навіть діагонально).\n";
    cout << "4. Гравці по черзі стріляють, вказуючи координати (наприклад B5).\n";
    cout << "5. Перемагає той, хто першим потопить усі кораблі суперника.\n\n";
    cout << "Натисніть Enter, щоб повернутися в меню...";
    cin.ignore();
    cin.get();
}

void playGame() {
    char player1[FD_SIZE][FD_SIZE];
    char player2[FD_SIZE][FD_SIZE];
    int mode;
    clearScreen();
    animateSeaBattleTitle();
    cout << "\n";
    cout << "1. Автоматичне розміщення\n";
    cout << "2. Ручне розміщення\n";
    cout << "Виберіть режим: ";
    cin >> mode;
    if (mode == 1) {
        clearScreen();
        printPersistentTitle();
        setColor(11);
        cout << "\n\n    Розміщення кораблів Гравця 1...\n";
        printWaveLine(40, 11, 9);
        resetColor();
        Sleep(800);
        placeShipsRandom(player1, player1Ships);

        clearScreen();
        printPersistentTitle();
        setColor(11);
        cout << "\n\n    Розміщення кораблів Гравця 2...\n";
        printWaveLine(40, 11, 9);
        resetColor();
        Sleep(800);
        placeShipsRandom(player2, player2Ships);
    }
    else {
        cout << "\nГравець 1 розміщує кораблі...\n";
        manualPlaceShips(player1, player1Ships);
        cout << "\nГравець 2 розміщує кораблі...\n";
        manualPlaceShips(player2, player2Ships);
    }
    int currentPlayer = 1;
    bool gameOver = false;
    while (!gameOver) {
        clearScreen();
        printPersistentTitle();
        cout << "\n";
        printBothFieldsWithSunk(player1, player2, false, player1Ships, player2Ships);

        setColor(14);
        cout << "Гравець " << currentPlayer << " — ваш хід!\n";
        resetColor();
        cout << "Введіть координати пострілу (наприклад B5): ";
        string coord;
        cin >> coord;
        int x, y;
        if (!parseCoordinate(coord, x, y)) {
            setColor(12);
            cout << "--> Невірні координати!\n";
            resetColor();
            Sleep(1200);
            continue;
        }
        char(&targetField)[FD_SIZE][FD_SIZE] = (currentPlayer == 1 ? player2 : player1);
        vector<Ship>& targetShips = (currentPlayer == 1 ? player2Ships : player1Ships);
        if (targetField[x][y] == HIT || targetField[x][y] == MISS) {
            setColor(12);
            cout << "--> Ви вже стріляли сюди!\n";
            resetColor();
            Sleep(1200);
            continue;
        }
        bool hit = makeShot(targetField, x, y);
        animateExplosion(x, y, hit);

        if (hit) {
            setColor(12); cout << "\n>>> ВЛУЧАННЯ! <<<\n"; resetColor();
            bool justSunk = checkIfSunk(targetField, targetShips, x, y);
            if (justSunk) {
                setColor(14); cout << ">>> КОРАБЕЛЬ ПОТОПЛЕНО! <<<\n"; resetColor();
                Sleep(500);
            }
            if (allShipsSunk(targetField)) {
                clearScreen();
                animateVictory(currentPlayer);
                gameOver = true;
            }
        }
        else {
            setColor(9); cout << "\n>>> ПРОМАХ <<<\n"; resetColor();
            Sleep(800);
            currentPlayer = (currentPlayer == 1 ? 2 : 1);
        }
        if (!gameOver) {
            cout << "\nНатисніть Enter для продовження...";
            cin.ignore();
            cin.get();
        }
    }
    cout << "\nДякуємо за гру!\n";
    system("pause");
}

void showMainMenu() {
    while (true) {
        clearScreen();
        static bool firstRun = true;
        if (firstRun) {
            animateSeaBattleTitle();
            firstRun = false;
        }
        
        setColor(10);
        cout << "  1. ";
        resetColor();
        cout << "Правила гри\n";
        setColor(12);
        cout << "  2. ";
        resetColor();
        cout << "Розпочати гру\n";
        setColor(9);
        cout << "  3. ";
        resetColor();
        cout << "Вийти з програми\n\n";

        setColor(14);
        cout << "Виберіть опцію: ";
        resetColor();
        int choice;
        cin >> choice;
        if (choice == 1) {
            showRules();
        }
        else if (choice == 2) {
            playGame();
        }
        else if (choice == 3) {
            clearScreen();
            printPersistentTitle();
            setColor(11);
            cout << "\n\n    Дякуємо за гру! До побачення!\n";
            printWaveLine(30, 11, 9);
            resetColor();
            Sleep(1500);
            exit(0);
        }
        else {
            setColor(12);
            cout << "Невірний вибір! Спробуйте ще раз.\n";
            resetColor();
            Sleep(1200);
        }
    }
}

//РУЧНЕ РОЗМІЩЕННЯ
void manualPlaceShips(char field[FD_SIZE][FD_SIZE], vector<Ship>& ships) {
    ships.clear();
    int shipSizes[] = { 4,3,2,1 };
    int shipCounts[] = { 1,2,3,4 };
    for (int type = 0; type < 4; type++) {
        int size = shipSizes[type];
        for (int k = 0; k < shipCounts[type]; k++) {
            bool placed = false;
            while (!placed) {
                clearScreen();
                printPersistentTitle();
                cout << "\n";
                setColor(11);
                cout << "<<-- РОЗМІЩЕННЯ КОРАБЛІВ -->>\n";
                setColor(14);
                cout << "  Корабель: " << size << "-палубний (" << (k + 1) << "/" << shipCounts[type] << ")\n\n";
                resetColor();
                cout << "    ";
                for (int i = 1; i <= FD_SIZE; i++) cout << i << " ";
                cout << "  \n";
                for (int i = 0; i < FD_SIZE; i++) {
                    cout << " " << LETT[i] << "| ";
                    for (int j = 0; j < FD_SIZE; j++) {
                        if (field[i][j] == SHIP) setColor(10);
                        else setColor(7);
                        cout << field[i][j] << " ";
                        resetColor();
                    }
                    cout << "|\n";
                }
                cout << "\n";
                string coord;
                cout << "Введіть початкову координату (наприклад B5): ";
                cin >> coord;
                int x, y;
                if (!parseCoordinate(coord, x, y)) {
                    setColor(12);
                    cout << "Невірний формат координат!\n";
                    resetColor();
                    Sleep(1500);
                    continue;
                }
                cout << "Орієнтація (H - горизонтально, V - вертикально): ";
                string orient;
                cin >> orient;

                // Перевірка на порожній рядок
                if (orient.empty()) {
                    setColor(12);
                    cout << "Невірна орієнтація!\n";
                    resetColor();
                    Sleep(1500);
                    continue;
                }

                bool horizontal = (toupper(orient[0]) == 'H' || toupper(orient[0]) == 'Г');

                bool canPlace = canPlaceShip(field, x, y, size, horizontal);
                animateShipPlacement(field, x, y, size, horizontal, canPlace);

                placed = placeShip(field, ships, x, y, size, horizontal);
                if (placed) {
                    animateShipAppear(field, x, y, size, horizontal);
                    setColor(10);
                    cout << "\n<<-- Корабель успішно розміщено! -->>\n";
                }
                else {
                    setColor(12);
                    cout << "\n<<-- Неможливо розмістити тут! -->>\n";
                }
                resetColor();
                Sleep(1200);
            }
        }
    }
    clearScreen();
    printPersistentTitle();
    cout << "\n";
    setColor(10);
    cout << "<<-- ВСІ КОРАБЛІ УСПІШНО РОЗМІЩЕНО! -->>\n\n";
    resetColor();
    cout << " ";
    for (int i = 1; i <= FD_SIZE; i++) cout << i << " ";
    cout << "\n";
    for (int i = 0; i < FD_SIZE; i++) {
        cout << LETT[i] << " |";
        for (int j = 0; j < FD_SIZE; j++) {
            setColor(field[i][j] == SHIP ? 10 : 7);
            cout << field[i][j] << " ";
            resetColor();
        }
        cout << "|\n";
    }
    cout << "\nНатисніть Enter для продовження...";
    cin.ignore();
    cin.get();
}

int main() {
    SetConsoleOutputCP(1251); 
    setlocale(LC_ALL, "Russian"); 
    srand(static_cast<unsigned>(time(0)));
    showMainMenu();
    return 0;
}
