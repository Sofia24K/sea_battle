#include <iostream>
#include <vector>
#include <string>
#include <windows.h>

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
//ДОПОМІЖНІ
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}
void resetColor() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}
void clearScreen() {
    system("cls");
}

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
    cout <<"  "<< LETT[row] << " |";
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
                return true;
            }
            break;
        }
    }
    return false;
}
//РУЧНЕ РОЗМІЩЕННЯ
void manualPlaceShips(char field[FD_SIZE][FD_SIZE], vector<Ship>& ships);
//ГОЛОВНЕ МЕНЮ
void showRules() {
    clearScreen();
    cout << "==================== ПРАВИЛА ГРИ ====================\n\n";
    cout << "1.Морський бій — класична гра для двох гравців.\n";
    cout << "2.Кожен гравець має флот:\n";
    cout << " -> 1 чотирипалубний корабель\n";
    cout << " -> 2 трипалубних кораблі\n";
    cout << " -> 3 двопалубних кораблі\n";
    cout << " -> 4 однопалубних кораблі\n";
    cout << "3.Кораблі не можуть торкатися один одного (навіть діагонально).\n";
    cout << "4.Гравці по черзі стріляють, вказуючи координати (наприклад B5).\n";
    cout << "5.Перемагає той, хто першим потопить усі кораблі суперника.\n\n";
    cout << "Натисніть Enter, щоб повернутися в меню...";
    cin.ignore();
    cin.get();
}
void playGame() {
    char player1[FD_SIZE][FD_SIZE];
    char player2[FD_SIZE][FD_SIZE];
    int mode;
    clearScreen();
    cout << "<<< МОРСЬКИЙ БІЙ >>>\n\n";
    cout << "1. Автоматичне розміщення\n";
    cout << "2. Ручне розміщення\n";
    cout << "Виберіть режим: ";
    cin >> mode;
    if (mode == 1) {
        placeShipsRandom(player1, player1Ships);
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
        printBothFields(player1, player2, false);
        cout << "Гравець " << currentPlayer << " — ваш хід!\n";
        cout << "Введіть координати пострілу (наприклад B5): ";
        string coord;
        cin >> coord;
        int x, y;
        if (!parseCoordinate(coord, x, y)) {
            cout << "--> Невірні координати!\n";
            Sleep(1200);
            continue;
        }
        char(&targetField)[FD_SIZE][FD_SIZE] = (currentPlayer == 1 ? player2 : player1);
        vector<Ship>& targetShips = (currentPlayer == 1 ? player2Ships : player1Ships);
        if (targetField[x][y] == HIT || targetField[x][y] == MISS) {
            cout << "--> Ви вже стріляли сюди!\n";
            Sleep(1200);
            continue;
        }
        bool hit = makeShot(targetField, x, y);
        if (hit) {
            setColor(12); cout << "\n>>> ВЛУЧАННЯ! <<<\n"; resetColor();
            bool justSunk = checkIfSunk(targetField, targetShips, x, y);
            if (justSunk) {
                setColor(14); cout << ">>> КОРАБЕЛЬ ПОТОПЛЕНО! <<<\n"; resetColor();
            }
            if (allShipsSunk(targetField)) {
                clearScreen();
                cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
                cout << " ПЕРЕМОГА ГРАВЦЯ " << currentPlayer << "!!! \n";
                cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
                gameOver = true;
            }
        }
        else {
            setColor(9); cout << "\n>>> ПРОМАХ <<<\n"; resetColor();
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
        cout << "========================================\n";
        cout << "          <<< МОРСЬКИЙ БІЙ >>>\n";
        cout << "========================================\n\n";
        cout << "1. Правила гри\n";
        cout << "2. Розпочати гру\n";
        cout << "3. Вийти з програми\n\n";
        cout << "Виберіть опцію: ";
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
            cout << "Дякуємо за гру! До побачення!\n";
            Sleep(1500);
            exit(0);
        }
        else {
            cout << "Невірний вибір! Спробуйте ще раз.\n";
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
                cout << "<<-- РОЗМІЩЕННЯ КОРАБЛІВ -->>\n";
                cout << "  Корабель: " << size << "-палубний (" << (k + 1) << "/" << shipCounts[type] << ")\n\n";
                cout << "    ";
                for (int i = 1; i <= FD_SIZE; i++) cout << i << " ";
                cout << "  \n";
                for (int i = 0; i < FD_SIZE; i++) {
                    cout << " "<< LETT[i] << "| ";
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
                    cout << "Невірний формат координат!\n";
                    Sleep(1500);
                    continue;
                }
                cout << "Орієнтація (H - горизонтально, V - вертикально): ";
                string orient;
                cin >> orient;
                bool horizontal = (toupper(orient[0]) == 'H' || toupper(orient[0]) == 'Г');
                placed = placeShip(field, ships, x, y, size, horizontal);
                if (placed) {
                    setColor(10); cout << "<<-- Корабель успішно розміщено! -->>\n";
                }
                else {
                    setColor(12); cout << "<<-- Неможливо розмістити тут! -->>\n";
                }
                resetColor();
                Sleep(1200);
            }
        }
    }
    clearScreen();
    cout << "<<-- ВСІ КОРАБЛІ УСПІШНО РОЗМІЩЕНО! -->>\n\n";
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
//MAIN
int main() {
    setlocale(LC_ALL, "");
    srand(static_cast<unsigned>(time(0)));
    showMainMenu(); 
    return 0;
} 