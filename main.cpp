#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
using namespace std;

// Struct pentru reprezentarea unui peste
struct Fish {
    string rarity;
    int baseValue;
};

// Clasa Player
class Player {
private:
    double money;
    int rodLevel;    // influenteaza sansele de succes (reduce sansele de fail)
    int baitLevel;   // influenteaza multiplicatorul castigului
public:
    Player() : money(50.0), rodLevel(0), baitLevel(0) {}

    double getMoney() const { return money; }
    int getRodLevel() const { return rodLevel; }
    int getBaitLevel() const { return baitLevel; }

    double getFailureChance() const {
        return 60.0 - rodLevel * 5; // scade 5% la fiecare upgrade
    }

    double getBaitMultiplier() const {
        return 1.0 + baitLevel * 0.5; // fiecare upgrade creste cu 0.5x
    }

    void addMoney(double amount) {
        money += amount;
    }

    bool buyRodUpgrade() {
        double cost = 50 + rodLevel * 50;
        if (money >= cost && rodLevel < 4) {
            money -= cost;
            rodLevel++;
            cout << "Undita imbunatatita la nivelul " << rodLevel << "!\n";
            return true;
        } else {
            cout << "Nu ai suficienti bani sau ai atins nivelul maxim pentru undita.\n";
            return false;
        }
    }

    bool buyBaitUpgrade() {
        double cost = 50 + baitLevel * 50;
        if (money >= cost && baitLevel < 4) {
            money -= cost;
            baitLevel++;
            cout << "Momeala imbunatatita la nivelul " << baitLevel << "!\n";
            return true;
        } else {
            cout << "Nu ai suficienti bani sau ai atins nivelul maxim pentru momeala.\n";
            return false;
        }
    }
};

// Functie care returneaza un peste pe baza zonei si sanselor
Fish catchFish(int zone) {
    int chance = rand() % 100 + 1;
    Fish f;

    // Stabilim valorile in functie de raritate
    if (chance <= 50) {
        f.rarity = "Common";
        f.baseValue = 10 * zone;
    } else if (chance <= 80) {
        f.rarity = "Rare";
        f.baseValue = 20 * zone;
    } else if (chance <= 95) {
        f.rarity = "Epic";
        f.baseValue = 40 * zone;
    } else {
        f.rarity = "Legendary";
        f.baseValue = 100 * zone;
    }
    return f;
}

void showMenu() {
    cout << "\n=== Hooked: Tycoon Fishing Game ===\n";
    cout << "1. Pescuieste\n";
    cout << "2. Cumpara upgrade undita\n";
    cout << "3. Cumpara upgrade momeala\n";
    cout << "4. Schimba zona\n";
    cout << "5. Arata status\n";
    cout << "0. Iesire\n";
    cout << "Alege o optiune: ";
}

int main() {
    srand(time(0));
    Player player;
    int zone = 1; // Incepem in zona Bucuresti
    bool running = true;

    cout << "Bine ai venit in Hooked! Incepi jocul cu $" << player.getMoney() << ".\n";
    cout << "Esti in zona 1: Bucuresti.\n";

    while (running) {
        showMenu();
        int choice;
        cin >> choice;

        switch (choice) {
        case 1: { // Pescuieste
            cout << "Incearca sa prinzi un peste...\n";
            double failChance = player.getFailureChance();
            int roll = rand() % 100 + 1;
            if (roll <= failChance) {
                cout << "Ghinion! Pestele a scapat!\n";
            } else {
                Fish f = catchFish(zone);
                double profit = f.baseValue * player.getBaitMultiplier();
                cout << "Ai prins un peste de tip " << f.rarity << " care valoreaza $" << profit << "!\n";
                player.addMoney(profit);
            }
            break;
        }

        case 2:
            player.buyRodUpgrade();
            break;

        case 3:
            player.buyBaitUpgrade();
            break;

        case 4:
            cout << "Alege zona (1: Bucuresti, 2: Delta Dunarii, 3: Marea Neagra): ";
            cin >> zone;
            if (zone < 1 || zone > 3) {
                cout << "Zona invalida. Ramine zona curenta.\n";
                zone = 1;
            } else {
                cout << "Te-ai mutat in zona " << zone << ".\n";
            }
            break;

        case 5:
            cout << "\n--- Status ---\n";
            cout << "Bani: $" << player.getMoney() << endl;
            cout << "Zona curenta: " << zone << endl;
            cout << "Nivel undita: " << player.getRodLevel() << " (Sansa de fail: " << player.getFailureChance() << "%)\n";
            cout << "Nivel momeala: " << player.getBaitLevel() << " (Multiplicator: x" << player.getBaitMultiplier() << ")\n";
            break;

        case 0:
            running = false;
            cout << "Multumim ca ai jucat Hooked! Spor la pescuit!\n";
            break;

        default:
            cout << "Optiune invalida.\n";
        }
    }

    return 0;
}

