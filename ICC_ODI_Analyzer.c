#include <stdio.h>
#include <stdlib.h>
#include "Players_data.h"

#define TEAM_COUNT 10
#define MAX_PLAYERS_PER_TEAM 50
#define NAME_LENGTH 51
#define ROLE_COUNT 3

typedef struct PlayerNode {
    int playerId;
    char name[NAME_LENGTH];
    char teamName[NAME_LENGTH];
    int role;
    int totalRuns;
    float battingAverage;
    float strikeRate;
    int wickets;
    float economyRate;
    float performanceIndex;
    struct PlayerNode *next;
} PlayerNode;

typedef struct {
    int teamId;
    char name[NAME_LENGTH];
    int totalPlayers;
    float averageBattingStrikeRate;
    PlayerNode *head;
    PlayerNode *tail;
    PlayerNode *rolePlayers[ROLE_COUNT][MAX_PLAYERS_PER_TEAM];
    int roleCount[ROLE_COUNT];
} Team;

Team *teamList = NULL;

int stringLength(const char text[]);
void stringCopy(char destination[], const char source[]);
int stringCompare(const char first[], const char second[]);
void clearInputBuffer(void);
int readInteger(const char prompt[], int minimum, int maximum);
float readFloat(const char prompt[], float minimum, float maximum);
void readName(const char prompt[], char name[]);
const char *getRoleName(int role);
int getRoleNumber(const char role[]);
float calculatePerformanceIndex(PlayerNode *player);

Team *searchTeam(int teamId);
Team *searchTeamByName(const char name[]);
int isPlayerIdPresent(int playerId);
PlayerNode *createPlayerNode(void);
void insertIntoRoleArray(Team *team, PlayerNode *player);
void appendPlayer(Team *team, PlayerNode *player);
void updateAverageStrikeRate(Team *team);

void initializeTeams(void);
void loadInitialPlayers(void);
void displayPlayerHeader(int showTeam);
void displayPlayer(PlayerNode *player, int showTeam);
void addNewPlayer(void);
void displayTeamPlayers(void);
void displayTeamsByStrikeRate(void);
void displayTopKPlayers(void);
void displayAllPlayersOfRole(void);
void displayMenu(void);

int teamShouldComeFirst(Team *first, Team *second);
void mergeTeams(Team *array[], Team *temp[], int left, int mid, int right);
void mergeSortTeams(Team *array[], Team *temp[], int left, int right);
void mergePlayerArrays(PlayerNode *array[], PlayerNode *temp[], int left, int mid, int right);
void mergeTeamRoleLists(int roleIndex, int leftTeam, int rightTeam, PlayerNode *result[], PlayerNode *temp[], int *resultCount);

void freeAllMemory(void);

int stringLength(const char text[]) {
    int i = 0;
    while (text[i] != '\0') {
        i++;
    }
    return i;
}

void stringCopy(char destination[], const char source[]) {
    int i = 0;
    while (source[i] != '\0') {
        destination[i] = source[i];
        i++;
    }
    destination[i] = '\0';
}

int stringCompare(const char first[], const char second[]) {
    int i = 0;

    while (first[i] != '\0' && second[i] != '\0') {
        if (first[i] != second[i]) {
            return (unsigned char)first[i] - (unsigned char)second[i];
        }
        i++;
    }

    return (unsigned char)first[i] - (unsigned char)second[i];
}

void clearInputBuffer() {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
    }
}

int readInteger(const char prompt[], int minimum, int maximum) {
    char input[100];
    int i, sign, value, digit, valid;

    while (1) {
        printf("%s", prompt);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\nInput ended.\n");
            return -1;
        }

        i = 0;
        while (input[i] != '\0' && input[i] != '\n') {
            i++;
        }

        if (input[i] == '\0' && i == 99) {
            clearInputBuffer();
            printf("Invalid input. Enter a value between %d and %d.\n", minimum, maximum);
            continue;
        }

        input[i] = '\0';

        i = 0;
        while (input[i] == ' ' || input[i] == '\t') {
            i++;
        }

        sign = 1;

        if (input[i] == '-') {
            sign = -1;
            i++;
        } else if (input[i] == '+') {
            i++;
        }

        value = 0;
        valid = 1;

        if (input[i] == '\0') {
            valid = 0;
        }

        while (input[i] != '\0') {
            if (input[i] < '0' || input[i] > '9') {
                valid = 0;
                break;
            }

            digit = input[i] - '0';

            if (value > (maximum + 1L) / 10 + 1) {
                valid = 0;
                break;
            }

            value = value * 10 + digit;
            i++;
        }

        if (valid) {
            while (input[i] == ' ' || input[i] == '\t') {
                i++;
            }

            if (input[i] != '\0') {
                valid = 0;
            }
        }

        if (valid) {
            value = value * sign;

            if (value >= minimum && value <= maximum) {
                return value;
            }
        }

        printf("Invalid input. Enter a value between %d and %d.\n", minimum, maximum);
    }
}

float readFloat(const char prompt[], float minimum, float maximum) {
    char input[100];
    int i, sign, valid, decimalFound, digitsFound;
    float value, divisor;

    while (1) {
        printf("%s", prompt);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\nInput ended.\n");
            return -1.0f;
        }

        i = 0;
        while (input[i] != '\0' && input[i] != '\n') {
            i++;
        }

        if (input[i] == '\0' && i == 99) {
            clearInputBuffer();
            printf("Invalid input. Enter a value between %.2f and %.2f.\n",
                   minimum, maximum);
            continue;
        }

        input[i] = '\0';

        i = 0;
        while (input[i] == ' ' || input[i] == '\t') {
            i++;
        }

        sign = 1;

        if (input[i] == '-') {
            sign = -1;
            i++;
        } else if (input[i] == '+') {
            i++;
        }

        value = 0.0f;
        divisor = 1.0f;
        valid = 1;
        decimalFound = 0;
        digitsFound = 0;

        while (input[i] != '\0') {
            if (input[i] == '.') {
                if (decimalFound) {
                    valid = 0;
                    break;
                }

                decimalFound = 1;
            } else if (input[i] >= '0' && input[i] <= '9') {
                digitsFound = 1;

                if (!decimalFound) {
                    value = value * 10.0f + (input[i] - '0');
                } else {
                    divisor *= 10.0f;
                    value += (input[i] - '0') / divisor;
                }
            } else if (input[i] != ' ' && input[i] != '\t') {
                valid = 0;
                break;
            }

            if (input[i] == ' ' || input[i] == '\t') {
                break;
            }

            i++;
        }

        if (!digitsFound) {
            valid = 0;
        }

        while (input[i] == ' ' || input[i] == '\t') {
            i++;
        }

        if (input[i] != '\0') {
            valid = 0;
        }

        value *= sign;

        if (valid && value >= minimum && value <= maximum) {
            return value;
        }

        printf("Invalid input. Enter a value between %.2f and %.2f.\n", minimum, maximum);
    }
}


void readName(const char prompt[], char name[]) {
    int length;
    int ch;

    while (1) {
        printf("%s", prompt);

        if (fgets(name, NAME_LENGTH, stdin) == NULL) {
            name[0] = '\0';
            return;
        }

        length = stringLength(name);

        if (length > 0 && name[length - 1] == '\n') {
            name[length - 1] = '\0';
            length--;
        } else {
            while ((ch = getchar()) != '\n' && ch != EOF) {
            }
        }

        if (length >= 1 && length <= 50) {
            return;
        }

        printf("Name must contain 1 to 50 characters.\n");
    }
}

const char *getRoleName(int role) {
    if (role == 1) return "Batsman";
    if (role == 2) return "Bowler";
    if (role == 3) return "All-rounder";
    return "Unknown";
}

int getRoleNumber(const char role[]) {
    if (stringCompare(role, "Batsman") == 0) 
    return 1;
    if (stringCompare(role, "Bowler") == 0) 
    return 2;
    if (stringCompare(role, "All-rounder") == 0) 
    return 3;
    return 0;
}

float calculatePerformanceIndex(PlayerNode *player) {
    float battingScore;

    battingScore = (player->battingAverage * player->strikeRate) / 100.0f;

    if (player->role == 1) {
        return battingScore;
    }

    if (player->role == 2) {
        return (player->wickets * 2.0f) + (100.0f - player->economyRate);
    }

    return battingScore + (player->wickets * 2.0f);
}

// Binary search: O(log t). teamList is maintained by increasing Team ID.
Team *searchTeam(int teamId) {
    int low = 0;
    int high = TEAM_COUNT - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;

        if (teamList[mid].teamId == teamId) {
            return &teamList[mid];
        }

        if (teamList[mid].teamId < teamId) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    return NULL;
}

Team *searchTeamByName(const char name[]) {
    int i;

    for (i = 0; i < TEAM_COUNT; i++) {
        if (stringCompare(teamList[i].name, name) == 0) {
            return &teamList[i];
        }
    }

    return NULL;
}

int isPlayerIdPresent(int playerId) {
    int i;
    PlayerNode *current;

    for (i = 0; i < TEAM_COUNT; i++) {
        current = teamList[i].head;

        while (current != NULL) {
            if (current->playerId == playerId) {
                return 1;
            }
            current = current->next;
        }
    }

    return 0;
}

PlayerNode *createPlayerNode(void) {
    PlayerNode *player = (PlayerNode *)malloc(sizeof(PlayerNode));

    if (player == NULL) {
        return NULL;
    }

    player->next = NULL;
    return player;
}

void insertIntoRoleArray(Team *team, PlayerNode *player) {
    int roleIndex = player->role - 1;
    int position = team->roleCount[roleIndex];
    while (position > 0 && team->rolePlayers[roleIndex][position - 1]->performanceIndex < player->performanceIndex) {
        team->rolePlayers[roleIndex][position] = team->rolePlayers[roleIndex][position - 1];
        position--;
    }

    team->rolePlayers[roleIndex][position] = player;
    team->roleCount[roleIndex]++;
}

void appendPlayer(Team *team, PlayerNode *player) {
    if (team->head == NULL) {
        team->head = player;
        team->tail = player;
    } else {
        team->tail->next = player;
        team->tail = player;
    }

    team->totalPlayers++;
    insertIntoRoleArray(team, player);
}

void updateAverageStrikeRate(Team *team) {
    PlayerNode *current = team->head;
    float totalStrikeRate = 0.0f;
    int count = 0;

    while (current != NULL) {
        if (current->role == 1 || current->role == 3) {
            totalStrikeRate += current->strikeRate;
            count++;
        }

        current = current->next;
    }

    if (count == 0) {
        team->averageBattingStrikeRate = 0.0f;
    } else {
        team->averageBattingStrikeRate = totalStrikeRate / count;
    }
}

void initializeTeams() {
    int i;

    teamList = (Team *)calloc(TEAM_COUNT, sizeof(Team));
    if (teamList == NULL) {
        return;
    }

    for (i = 0; i < TEAM_COUNT; i++) {
        teamList[i].teamId = i + 1;
        stringCopy(teamList[i].name, teams[i]);

        teamList[i].totalPlayers = 0;
        teamList[i].averageBattingStrikeRate = 0.0f;
        teamList[i].head = NULL;
        teamList[i].tail = NULL;

        teamList[i].roleCount[0] = 0;
        teamList[i].roleCount[1] = 0;
        teamList[i].roleCount[2] = 0;
    }
}

void loadInitialPlayers() {
    int i;
    Team *team;
    PlayerNode *player;

    for (i = 0; i < playerCount; i++) {
        team = searchTeamByName(players[i].team);

        if (team == NULL) {
            continue;
        }

        player = createPlayerNode();

        if (player == NULL) {
            printf("Player storage capacity exceeded.\n");
            return;
        }

        player->playerId = players[i].id;
        stringCopy(player->name, players[i].name);
        stringCopy(player->teamName, players[i].team);

        player->role = getRoleNumber(players[i].role);
        player->totalRuns = players[i].totalRuns;
        player->battingAverage = players[i].battingAverage;
        player->strikeRate = players[i].strikeRate;
        player->wickets = players[i].wickets;
        player->economyRate = players[i].economyRate;

        if (player->role == 0) {
            free(player);
            continue;
        }

        player->performanceIndex = calculatePerformanceIndex(player);
        player->next = NULL;

        appendPlayer(team, player);
    }

    for (i = 0; i < TEAM_COUNT; i++) {
        updateAverageStrikeRate(&teamList[i]);
    }
}

void displayPlayerHeader(int showTeam) {
    printf("----------------------------------------------------------------------------------------------------------------\n");

    if (showTeam) {
        printf("%-6s %-24s %-16s %-13s %-8s %-8s %-8s %-8s %-9s %-10s\n",
               "ID", "Name", "Team", "Role", "Runs", "Average",
               "SR", "Wickets", "Economy", "Perf.Index");
    } else {
        printf("%-6s %-24s %-13s %-8s %-8s %-8s %-8s %-9s %-10s\n",
               "ID", "Name", "Role", "Runs", "Average",
               "SR", "Wickets", "Economy", "Perf.Index");
    }

    printf("----------------------------------------------------------------------------------------------------------------\n");
}

void displayPlayer(PlayerNode *player, int showTeam) {
    if (showTeam) {
        printf("%-6d %-24s %-16s %-13s %-8d %-8.1f %-8.1f %-8d %-9.1f %-10.2f\n",
               player->playerId, player->name, player->teamName,
               getRoleName(player->role), player->totalRuns,
               player->battingAverage, player->strikeRate,
               player->wickets, player->economyRate,
               player->performanceIndex);
    } else {
        printf("%-6d %-24s %-13s %-8d %-8.1f %-8.1f %-8d %-9.1f %-10.2f\n",
               player->playerId, player->name,
               getRoleName(player->role), player->totalRuns,
               player->battingAverage, player->strikeRate,
               player->wickets, player->economyRate,
               player->performanceIndex);
    }
}

void addNewPlayer(void) {
    int teamId;
    int role;
    Team *team;
    PlayerNode *player;

    teamId = readInteger("Enter Team ID (1-10): ", 1, 10);
    if (teamId == -1) return;

    team = searchTeam(teamId);

    if (team == NULL) {
        printf("Team not found.\n");
        return;
    }

    if (team->totalPlayers >= MAX_PLAYERS_PER_TEAM) {
        printf("This team already has the maximum of 50 players.\n");
        return;
    }

    player = createPlayerNode();

    if (player == NULL) {
        printf("Unable to create player.\n");
        return;
    }

    player->playerId = readInteger("Player ID (1-1000): ", 1, 1000);
    if (player->playerId == -1) return;

    if (isPlayerIdPresent(player->playerId)) {
        printf("This Player ID already exists.\n");
        return;
    }

    readName("Player Name: ", player->name);
    if (player->name[0] == '\0') return;

    role = readInteger("Role (1-Batsman, 2-Bowler, 3-All-rounder): ", 1, 3);
    if (role == -1) 
    return;

    player->role = role;
    player->totalRuns = readInteger("Total Runs: ", 0, 1000000);
    if (player->totalRuns == -1) 
    return;

    player->battingAverage = readFloat("Batting Average: ", 0.0f, 1000.0f);
    if (player->battingAverage < 0.0f) 
    return;

    player->strikeRate = readFloat("Strike Rate: ", 0.0f, 1000.0f);
    if (player->strikeRate < 0.0f) 
    return;

    player->wickets = readInteger("Wickets: ", 0, 10000);
    if (player->wickets == -1) 
    return;

    player->economyRate = readFloat("Economy Rate: ", 0.0f, 1000.0f);
    if (player->economyRate < 0.0f) 
    return;

    stringCopy(player->teamName, team->name);
    player->performanceIndex = calculatePerformanceIndex(player);
    player->next = NULL;

    appendPlayer(team, player);
    updateAverageStrikeRate(team);

    printf("Player added successfully to Team %s!\n", team->name);
    printf("Performance Index: %.2f\n", player->performanceIndex);
}

void displayTeamPlayers(void) {
    int teamId;
    Team *team;
    PlayerNode *current;

    teamId = readInteger("Enter Team ID (1-10): ", 1, 10);
    if (teamId == -1) return;

    team = searchTeam(teamId);

    if (team == NULL) {
        printf("Team not found.\n");
        return;
    }

    printf("\nPlayers of Team %s:\n", team->name);
    displayPlayerHeader(0);

    current = team->head;

    while (current != NULL) {
        displayPlayer(current, 0);
        current = current->next;
    }

    printf("----------------------------------------------------------------------------------------------------------------\n");
    printf("Total Players: %d\n", team->totalPlayers);
    printf("Average Batting Strike Rate: %.2f\n",
           team->averageBattingStrikeRate);
}

int teamShouldComeFirst(Team *first, Team *second) {
    if (first->averageBattingStrikeRate > second->averageBattingStrikeRate) {
        return 1;
    }

    if (first->averageBattingStrikeRate < second->averageBattingStrikeRate) {
        return 0;
    }

    return first->teamId < second->teamId;
}

void mergeTeams(Team *array[], Team *temp[], int left, int mid, int right) {
    int i = left;
    int j = mid + 1;
    int k = left;

    while (i <= mid && j <= right) {
        if (teamShouldComeFirst(array[i], array[j])) {
            temp[k] = array[i];
            i++;
        } else {
            temp[k] = array[j];
            j++;
        }
        k++;
    }

    while (i <= mid) {
        temp[k] = array[i];
        i++;
        k++;
    }

    while (j <= right) {
        temp[k] = array[j];
        j++;
        k++;
    }

    for (i = left; i <= right; i++) {
        array[i] = temp[i];
    }
}

void mergeSortTeams(Team *array[], Team *temp[], int left, int right) {
    int mid;

    if (left >= right) {
        return;
    }

    mid = left + (right - left) / 2;

    mergeSortTeams(array, temp, left, mid);
    mergeSortTeams(array, temp, mid + 1, right);
    mergeTeams(array, temp, left, mid, right);
}

void displayTeamsByStrikeRate(void) {
    Team *sorted[TEAM_COUNT];
    Team *temp[TEAM_COUNT];
    int i;

    for (i = 0; i < TEAM_COUNT; i++) {
        sorted[i] = &teamList[i];
    }

    mergeSortTeams(sorted, temp, 0, TEAM_COUNT - 1);

    printf("\nTeams Sorted by Average Batting Strike Rate\n");
    printf("================================================================\n");
    printf("%-8s %-22s %-18s %-15s\n",
           "Team ID", "Team Name", "Average Bat SR", "Total Players");
    printf("================================================================\n");

    for (i = 0; i < TEAM_COUNT; i++) {
        printf("%-8d %-22s %-18.2f %-15d\n",
               sorted[i]->teamId,
               sorted[i]->name,
               sorted[i]->averageBattingStrikeRate,
               sorted[i]->totalPlayers);
    }

    printf("================================================================\n");
}

/* ---------------- Requirement 4: Top K players ---------------- */

void displayTopKPlayers(void) {
    int teamId;
    int role;
    int k;
    int i;
    int roleIndex;
    Team *team;

    teamId = readInteger("Enter Team ID (1-10): ", 1, 10);
    if (teamId == -1) return;

    team = searchTeam(teamId);

    if (team == NULL) {
        printf("Team not found.\n");
        return;
    }

    role = readInteger("Role (1-Batsman, 2-Bowler, 3-All-rounder): ", 1, 3);
    if (role == -1) return;

    roleIndex = role - 1;

    if (team->roleCount[roleIndex] == 0) {
        printf("No %s found in Team %s.\n",
               getRoleName(role), team->name);
        return;
    }

    k = readInteger("Enter number of players (K): ", 1, 50);
    if (k == -1) 
    return;

    if (k > team->roleCount[roleIndex]) {
        k = team->roleCount[roleIndex];
        printf("Only %d players of this role are available.\n", k);
    }

    printf("\nTop %d %s(s) of Team %s:\n", k, getRoleName(role), team->name);

    displayPlayerHeader(0);

    for (i = 0; i < k; i++) {
        displayPlayer(team->rolePlayers[roleIndex][i], 0);
    }

    printf("================================================================\n");
}

void mergePlayerArrays(PlayerNode *array[], PlayerNode *temp[], int left, int mid, int right) {
    int i = left;
    int j = mid + 1;
    int k = left;

    while (i <= mid && j <= right) {
        if (array[i]->performanceIndex >= array[j]->performanceIndex) {
            temp[k] = array[i];
            i++;
        } else {
            temp[k] = array[j];
            j++;
        }
        k++;
    }

    while (i <= mid) {
        temp[k] = array[i];
        i++;
        k++;
    }

    while (j <= right) {
        temp[k] = array[j];
        j++;
        k++;
    }

    for (i = left; i <= right; i++) {
        array[i] = temp[i];
    }
}

void mergeTeamRoleLists(int roleIndex, int leftTeam, int rightTeam, PlayerNode *result[], PlayerNode *temp[], int *resultCount) {
    int mid;
    int leftCount = 0;
    int rightCount = 0;
    int i;
    int offset;

    if (leftTeam == rightTeam) {
        for (i = 0; i < teamList[leftTeam].roleCount[roleIndex]; i++) {
            result[*resultCount] = teamList[leftTeam].rolePlayers[roleIndex][i];
            (*resultCount)++;
        }
        return;
    }

    mid = leftTeam + (rightTeam - leftTeam) / 2;

    offset = *resultCount;

    mergeTeamRoleLists(roleIndex, leftTeam, mid,
                       result, temp, resultCount);
    leftCount = *resultCount - offset;

    mergeTeamRoleLists(roleIndex, mid + 1, rightTeam,
                       result, temp, resultCount);
    rightCount = *resultCount - offset - leftCount;

    if (leftCount > 0 && rightCount > 0) {
        mergePlayerArrays(result, temp, offset, offset + leftCount - 1, offset + leftCount + rightCount - 1);
    }
}

void displayAllPlayersOfRole() {
    int role;
    int roleIndex;
    int total = 0;
    int i;
    int resultCount = 0;
    PlayerNode **sortedPlayers;
    PlayerNode **temp;

    role = readInteger("Role (1-Batsman, 2-Bowler, 3-All-rounder): ", 1, 3);
    if (role == -1) return;

    roleIndex = role - 1;

    for (i = 0; i < TEAM_COUNT; i++) {
        total += teamList[i].roleCount[roleIndex];
    }

    if (total == 0) {
        printf("No %s records found.\n", getRoleName(role));
        return;
    }

    sortedPlayers = (PlayerNode **)malloc((size_t)total * sizeof(PlayerNode *));
    temp = (PlayerNode **)malloc((size_t)total * sizeof(PlayerNode *));

    if (sortedPlayers == NULL || temp == NULL) {
        printf("Unable to allocate memory for sorting players.\n");
        free(sortedPlayers);
        free(temp);
        return;
    }

    mergeTeamRoleLists(roleIndex, 0, TEAM_COUNT - 1, sortedPlayers, temp, &resultCount);

    printf("\nAll %s players across all teams:\n", getRoleName(role));
    displayPlayerHeader(1);

    for (i = 0; i < resultCount; i++) {
        displayPlayer(sortedPlayers[i], 1);
    }

    printf("================================================================================\n");
    printf("Total %s players: %d\n", getRoleName(role), resultCount);

    free(sortedPlayers);
    free(temp);
}

void displayMenu() {
    printf("\n============================================================\n");
    printf("          ICC ODI PLAYER PERFORMANCE ANALYZER\n");
    printf("============================================================\n");
    printf("1. Add Player to Team\n");
    printf("2. Display Players of a Specific Team\n");
    printf("3. Display Teams by Average Batting Strike Rate\n");
    printf("4. Display Top K Players of a Specific Team by Role\n");
    printf("5. Display All Players of a Specific Role Across All Teams\n");
    printf("6. Exit\n");
    printf("============================================================\n");
}

void freeAllMemory(void) {
    int i;
    PlayerNode *current;
    PlayerNode *next;

    if (teamList == NULL) {
        return;
    }

    for (i = 0; i < TEAM_COUNT; i++) {
        current = teamList[i].head;
        while (current != NULL) {
            next = current->next;
            free(current);
            current = next;
        }
    }

    free(teamList);
    teamList = NULL;
}

int main() {
    int choice;

    initializeTeams();
    if (teamList == NULL) {
        return 1;
    }
    loadInitialPlayers();

    printf("==============================================================\n");
    printf("ICC ODI Player Performance Analyzer initialized.\n");
    printf("==============================================================\n");

    while (1) {
        displayMenu();

        choice = readInteger("Enter your choice: ", 1, 6);
        if (choice == -1) {
            break;
        }

        if (choice == 1) {
            addNewPlayer();
        } else if (choice == 2) {
            displayTeamPlayers();
        } else if (choice == 3) {
            displayTeamsByStrikeRate();
        } else if (choice == 4) {
            displayTopKPlayers();
        } else if (choice == 5) {
            displayAllPlayersOfRole();
        } else if (choice == 6) {
            break;
        }
    }

    freeAllMemory();
    return 0;
}
