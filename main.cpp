#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <random>

using namespace std;

// ======================== Game Constants ========================
const vector<string> ZONES = {"Bucharest", "Danube Delta", "Black Sea"};
const vector<string> RARITIES = {"Common", "Rare", "Epic", "Legendary"};
const vector<double> RARITY_PROBS = {0.5, 0.3, 0.15, 0.05};
const vector<double> RARITY_MULTIPLIERS = {1.0, 1.5, 2.5, 5.0};
const int UPGRADES_PER_ZONE = 4;
const vector<int> TRAVEL_COSTS = {0, 1000, 5000};

// Better random number generation
mt19937 createRandomEngine() {
    random_device rd;
    return mt19937(rd());
}

// ======================== Fish Class ========================
class Fish {
private:
    string name;
    string rarity;
    double baseValue;
    int zone;

public:
    // Constructor
    Fish(const string& n, const string& r, double v, int z)
        : name(n), rarity(r), baseValue(v), zone(z) {}

    // Copy Constructor
    Fish(const Fish& other)
        : name(other.name), rarity(other.rarity), baseValue(other.baseValue), zone(other.zone) {}

    // Operator=
    Fish& operator=(const Fish& other) {
        if (this != &other) { // protect against self-assignment
            name = other.name;
            rarity = other.rarity;
            baseValue = other.baseValue;
            zone = other.zone;
        }
        return *this;
    }

    // Destructor
    ~Fish() = default; // No dynamic memory, default is fine

    double getValue(double baitMultiplier) const {
        auto it = find(RARITIES.begin(), RARITIES.end(), rarity);
        int rarityIndex = distance(RARITIES.begin(), it);
        return baseValue * baitMultiplier * (zone + 1) * RARITY_MULTIPLIERS[rarityIndex];
    }

    friend ostream& operator<<(ostream& os, const Fish& fish) {
        os << fish.rarity << " " << fish.name << " (Zone " << fish.zone + 1 << ")";
        return os;
    }

    const string& getRarity() const { return rarity; }
    int getZone() const { return zone; }
    const string& getName() const { return name; }
};

// ======================== Player Equipment ========================
class Equipment {
private:
    double failChance;
    double baitMultiplier;
    vector<vector<int>> zoneUpgrades; // [zone][0=rod, 1=bait]

public:
    Equipment()
        : failChance(0.6),
          baitMultiplier(1.0),
          zoneUpgrades(3, std::vector<int>(2, 0)) {
        // Constructor body can remain empty or contain other logic if needed
    }

    friend ostream& operator<<(ostream& os, const Equipment& eq) {
        os << "\n=== Equipment Status ==="
           << "\nFail Chance: " << fixed << setprecision(0) << (eq.failChance * 100) << "%"
           << "\nBait Multiplier: " << fixed << setprecision(1) << eq.baitMultiplier << "x"
           << "\nZone Upgrades:";

        for (int z = 0; z < 3; z++) {
            os << "\n  " << ZONES[z] << ": Rod " << eq.zoneUpgrades[z][0] << "/4, Bait "
               << eq.zoneUpgrades[z][1] << "/4";
        }
        return os;
    }

    bool attemptCatch() const {
        uniform_real_distribution<double> dist(0.0, 1.0);
        auto rng = createRandomEngine();
        return dist(rng) > failChance;
    }

    double getBaitMultiplier() const { return baitMultiplier; }

    void upgradeRod(int zone) {
        if (zone < 0 || zone >= 3) return;

        if (zoneUpgrades[zone][0] < UPGRADES_PER_ZONE) {
            failChance = max(0.0, failChance - 0.05); // Prevent negative chance
            zoneUpgrades[zone][0]++;
            cout << "Rod upgraded for " << ZONES[zone] << "! New fail chance: "
                 << fixed << setprecision(0) << (failChance * 100) << "%\n";
        } else {
            cout << "Max rod upgrades reached for " << ZONES[zone] << "!\n";
        }
    }

    void upgradeBait(int zone) {
        if (zone < 0 || zone >= 3) return;

        if (zoneUpgrades[zone][1] < UPGRADES_PER_ZONE) {
            baitMultiplier += 0.5;
            zoneUpgrades[zone][1]++;
            cout << "Bait upgraded for " << ZONES[zone] << "! New multiplier: "
                 << fixed << setprecision(1) << baitMultiplier << "x\n";
        } else {
            cout << "Max bait upgrades reached for " << ZONES[zone] << "!\n";
        }
    }

    void printUpgrades() const {
        cout << "\nCurrent Upgrades:\n";
        for (int z = 0; z < 3; z++) {
            cout << ZONES[z] << ": Rod " << zoneUpgrades[z][0] << "/" << UPGRADES_PER_ZONE
                 << ", Bait " << zoneUpgrades[z][1] << "/" << UPGRADES_PER_ZONE << "\n";
        }
    }

    int getUpgradeCount(int zone, char type) const {
        if (zone < 0 || zone >= 3) return 0;
        return zoneUpgrades[zone][type == 'r' ? 0 : 1]; //'r'=Rod
    }
};

// ======================== Player Class ========================
class Player {
private:
    string name;
    double money;
    int currentZone;
    Equipment equipment;
    map<string, int> fishCollection;
    vector<bool> zonesUnlocked;
    vector<map<string, bool>> zoneFishCaught;

public:
    explicit Player(const std::string& n)
        : name(n),
          money(100.0),
          currentZone(0),
          zonesUnlocked(3, false),       // initialize directly
          zoneFishCaught(3)              // also initialize directly
    {
        zonesUnlocked[0] = true;         // set the first zone as unlocked
    }

    friend ostream& operator<<(ostream& os, const Player& player) {
        os << "\n=== Player Status ==="
           << "\nName: " << player.name
           << "\nMoney: $" << fixed << setprecision(2) << player.money
           << "\nCurrent Zone: " << ZONES[player.currentZone]
           << "\nFish Caught: " << player.fishCollection.size() << " species";

        // Display zone completion status
        os << "\n\nZone Progress:";
        for (size_t z = 0; z < ZONES.size(); z++) {
            int caught = 0;
            if (z < player.zoneFishCaught.size()) {
                for (const auto& fish : player.zoneFishCaught[z]) {
                    if (fish.second) caught++;
                }
            }
            os << "\n  " << ZONES[z] << ": " << caught << "/4 fish";
        }

        return os;
    }

    void catchFish(const Fish& fish) {
        double value = fish.getValue(equipment.getBaitMultiplier());
        money += value;
        fishCollection[fish.getName()]++;
        zoneFishCaught[fish.getZone()][fish.getName()] = true;

        cout << "Caught a " << fish.getRarity() << " " << fish << " for $"
             << fixed << setprecision(2) << value << "\n";

        checkZoneCompletion(fish.getZone());
    }

    bool attemptUpgrade(char type) {
        int cost = calculateUpgradeCost(type);

        if (money >= cost) {
            money -= cost;
            if (type == 'r') //'r'=Rod
            {
                equipment.upgradeRod(currentZone);
            } else {
                equipment.upgradeBait(currentZone);
            }
            return true;
        }
        cout << "Not enough money! Need $" << cost << "\n";
        return false;
    }

    void travelToZone(int zone) {
        if (zone == currentZone) return;

        if (!canTravelToZone(zone)) {
            cout << "\nYou must fully upgrade both rod and bait in all previous zones before traveling to "
                 << ZONES[zone] << "!\n";
            return;
        }

        if (money >= TRAVEL_COSTS[zone]) {
            money -= TRAVEL_COSTS[zone];
            currentZone = zone;
            zonesUnlocked[zone] = true;
            cout << "\nTraveled to " << ZONES[zone] << " for $" << TRAVEL_COSTS[zone] << "\n";
        } else {
            cout << "\nNot enough money to travel to " << ZONES[zone]
                 << "! Need $" << TRAVEL_COSTS[zone] << "\n";
        }
    }

    void printStatus() const {
        cout << "\n=== " << name << " ===\n"
             << "Zone: " << ZONES[currentZone] << "\n"
             << "Money: $" << fixed << setprecision(2) << money << "\n"
             << "Fish caught: " << fishCollection.size() << " different species\n";
        equipment.printUpgrades();
    }

    void showCollection() const {
        cout << "\n=== Fish Collection ===\n";
        if (fishCollection.empty()) {
            cout << "You haven't caught any fish yet!\n";
            return;
        }

        for (const auto& entry : fishCollection) {
            cout << entry.first << ": " << entry.second << " caught\n";
        }
    }

    // Helper methods
    int calculateUpgradeCost(char type) const {
        int upgrades = equipment.getUpgradeCount(currentZone, type);
        return 100 * (currentZone + 1) * (upgrades + 1);
    }

    bool canTravelToZone(int zone) const {
        if (zone == currentZone || zone < 0 || zone >= 3) return false;

        if (zone > currentZone) {
            for (int z = 0; z < zone; z++) {
                if (equipment.getUpgradeCount(z, 'r') < UPGRADES_PER_ZONE ||  //'r'=Rod
                    equipment.getUpgradeCount(z, 'b') < UPGRADES_PER_ZONE) {  //'b'=Bait
                    return false;
                }
            }
        }
        return true;
    }

    void checkZoneCompletion(int zone) {
        const vector<string> zoneFishNames = {
            "Carp", "Pike", "Catfish", "Sturgeon",
            "Bream", "Zander", "Pike-Perch", "Beluga",
            "Mackerel", "Sea Bass", "Tuna", "Bluefin Tuna"
        };

        int zoneStartIdx = zone * 4;
        bool allCaught = true;

        for (int i = 0; i < 4; i++) {
            string fishName = zoneFishNames[zoneStartIdx + i];
            if (!zoneFishCaught[zone][fishName]) {
                allCaught = false;
                break;
            }
        }

        if (allCaught) {
            cout << "\n************************************\n";
            cout << "*  CONGRATULATIONS!                *\n";
            cout << "*  You've caught all fish species  *\n";
            cout << "*  in " << setw(12) << left << ZONES[zone] << "!      *\n";
            cout << "************************************\n\n";

            double bonus = 500.0 * (zone + 1);
            money += bonus;
            cout << "Received zone completion bonus: $" << bonus << "!\n";
        }
    }

    bool hasCaughtFish(const string& fishName) const {
        return fishCollection.count(fishName) > 0;
    }

    double getMoney() const { return money; }
    int getCurrentZone() const { return currentZone; }
    const Equipment& getEquipment() const { return equipment; }
};

// ======================== Game World ========================
class FishingWorld {
private:
    vector<vector<Fish>> zoneFish;
    vector<vector<string>> zoneFishNames;

public:
    FishingWorld() {
        // Initialize fish for each zone
        zoneFish.resize(3);

        // Zone 1 - Bucharest
        zoneFish[0] = {
            Fish("Carp", "Common", 5.0, 0),
            Fish("Pike", "Rare", 15.0, 0),
            Fish("Catfish", "Epic", 40.0, 0),
            Fish("Sturgeon", "Legendary", 100.0, 0)
        };

        // Zone 2 - Danube Delta
        zoneFish[1] = {
            Fish("Bream", "Common", 10.0, 1),
            Fish("Zander", "Rare", 30.0, 1),
            Fish("Pike-Perch", "Epic", 75.0, 1),
            Fish("Beluga", "Legendary", 200.0, 1)
        };

        // Zone 3 - Black Sea
        zoneFish[2] = {
            Fish("Mackerel", "Common", 20.0, 2),
            Fish("Sea Bass", "Rare", 50.0, 2),
            Fish("Tuna", "Epic", 125.0, 2),
            Fish("Bluefin Tuna", "Legendary", 300.0, 2)
        };
        zoneFishNames = {
            {"Carp", "Pike", "Catfish", "Sturgeon"},
            {"Bream", "Zander", "Pike-Perch", "Beluga"},
            {"Mackerel", "Sea Bass", "Tuna", "Bluefin Tuna"}
        };
    }

    const vector<string>& getFishNamesForZone(int zone) const {
        return zoneFishNames[zone];
    }
    friend ostream& operator<<(ostream& os, const FishingWorld& world) {
        os << "\n=== Fishing World ==="
           << "\nAvailable Fish by Zone:";

        for (size_t z = 0; z < world.zoneFish.size(); z++) {
            os << "\n\n" << ZONES[z] << ":";
            for (const auto& fish : world.zoneFish[z]) {
                os << "\n  " << fish; // Using Fish's operator<<
            }
        }
        return os;
    }

    Fish generateFish(int zone) const {
        uniform_real_distribution<double> dist(0.0, 1.0);
        auto rng = createRandomEngine();
        double randVal = dist(rng);
        double cumulative = 0.0;

        for (size_t i = 0; i < RARITY_PROBS.size(); i++) {
            cumulative += RARITY_PROBS[i];
            if (randVal <= cumulative) {
                return zoneFish[zone][i];
            }
        }
        return zoneFish[zone][0]; // Default to common
    }
};

// ======================== Game Interface ========================
int getValidatedInput(int min, int max) {
    int choice;
    while (true) {
        if (cin >> choice) {
            if (choice >= min && choice <= max) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return choice;
            }
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Please enter a number between "
             << min << " and " << max << ": ";
    }
}

string generateRandomWord(int length) {
    string word;
    uniform_int_distribution<int> dist('a', 'z');
    auto rng = createRandomEngine();
    for (int i = 0; i < length; i++) {
        word += dist(rng);
    }
    return word;
}

bool minigame() {
    string targetWord = generateRandomWord(5);
    cout << "\nQUICK! Type this word within 3 seconds: " << targetWord << "\n> ";

    auto start = chrono::steady_clock::now();
    string input;
    cin >> input;
    auto end = chrono::steady_clock::now();

    double elapsed = chrono::duration_cast<chrono::milliseconds>(end - start).count() / 1000.0;

    if (input == targetWord && elapsed <= 3.0) {
        cout << "Success! (" << fixed << setprecision(1) << elapsed << "s)\n";
        return true;
    } else {
        cout << "Too slow or wrong word! (" << elapsed << "s)\n";
        return false;
    }
}

void displayMainMenu() {
    cout << "\n=== Hooked ===\n"
         << "1. Go Fishing\n"
         << "2. Upgrade Equipment\n"
         << "3. Travel to New Zone\n"
         << "4. View Status\n"
         << "5. View Fish Collection\n"
         << "0. Quit Game\n"
         << "Choose an option (0-5): ";
}

void displayUpgradeMenu(const Player& player) {
    int rodCost = player.calculateUpgradeCost('r');//'r'=Rod
    int baitCost = player.calculateUpgradeCost('b');//'b'=Bait

    cout << "\n=== Upgrade Equipment ===\n"
         << "Current Money: $" << fixed << setprecision(2) << player.getMoney() << "\n\n"
         << "1. Upgrade Rod (-5% fail chance) - $" << rodCost << "\n"
         << "2. Upgrade Bait (+0.5x multiplier) - $" << baitCost << "\n"
         << "0. Back to Main Menu\n"
         << "Choose an option (0-2): ";
}

void displayZoneMenu(const Player& player) {
    cout << "\n=== Travel to New Zone ===\n";
    for (size_t i = 0; i < ZONES.size(); i++) {
        cout << i+1 << ". " << ZONES[i];
        if (static_cast<int>(i) == player.getCurrentZone()) {  // Fixed comparison
            cout << " (Current)";
        } else if (!player.canTravelToZone(static_cast<int>(i))) {  // Also fixed here
            cout << " [Locked]";
        } else {
            cout << " - Travel Cost: $" << TRAVEL_COSTS[i];
        }
        cout << "\n";
    }
    cout << "0. Back to Main Menu\n"
         << "Choose a zone (0-" << ZONES.size() << "): ";
}

// ======================== Main Game Loop ========================
int main() {
    cout << "=== Welcome to Hooked! ===\n";
    cout << "Enter your name: ";
    string playerName;
    getline(cin, playerName);

    Player player(playerName);
    FishingWorld world;

    while (true) {
        displayMainMenu();
        int choice = getValidatedInput(0, 5);

        switch (choice) {
            case 1: { // Fishing
                if (player.getEquipment().attemptCatch()) {
                    if (minigame()) {
                        Fish caught = world.generateFish(player.getCurrentZone());
                        player.catchFish(caught);
                    } else {
                        cout << "\nThe fish got away!\n";
                    }
                } else {
                    cout << "\nThe fish got away!\n";
                }
                break;
            }
            case 2: { // Upgrades
                while (true) {
                    displayUpgradeMenu(player);
                    int upgradeChoice = getValidatedInput(0, 2);

                    if (upgradeChoice == 0) break;

                    if (upgradeChoice == 1 || upgradeChoice == 2) {
                        player.attemptUpgrade(upgradeChoice == 1 ? 'r' : 'b');//'r'=Rod
                                                                              //'b'=Bait
                    }
                }
                break;
            }
            case 3: { // Travel
                displayZoneMenu(player);
                int zoneChoice = getValidatedInput(0, ZONES.size());

                if (zoneChoice > 0) {
                    player.travelToZone(zoneChoice - 1);
                }
                break;
            }
            case 4: { // Status
                player.printStatus();

                // Show zone completion progress
                int currentZone = player.getCurrentZone();
                int caughtCount = 0;
                auto zoneFish = world.getFishNamesForZone(currentZone);

                for (const auto& fishName : zoneFish) {
                    if (player.hasCaughtFish(fishName)) {
                        caughtCount++;
                    }
                }

                cout << "\nZone Progress: " << caughtCount << "/" << zoneFish.size()
                     << " fish species caught in " << ZONES[currentZone] << "\n";
                break;
            }
            case 5: // Fish Collection
                player.showCollection();
                break;
            case 0: // Quit
                cout << "\nThanks for playing Hooked, " << playerName << "!\n";
                return 0;
        }
    }
}
