#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

void displayBudget();
void expenseDistribution();
void sortEntries();
void addEntry();
void modifyEntry();
void filterByMonth();
void undoLastAction();

#define MAX_ENTRIES 1000
#define MAX_STR 101  // 100 chars + null terminator

typedef struct {
    int id;
    char date[MAX_STR];
    char type[MAX_STR];
    char subtype[MAX_STR];
    char description[MAX_STR];
    float amount;
} FinanceEntry;

int readFile(FinanceEntry entries[], const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file.\n");
        exit(1);
    }

    int count = 0;
    char line[512]; // safe default size
    while (fgets(line, sizeof(line), file)) {
        // Remove newline if exists
        line[strcspn(line, "\n")] = 0; // This replaces the newline with a null terminator, effectively truncating the string at that point.

        char *token = strtok(line, "|"); // splits a string into tokens using a delimiter (here, `"|"`).
        char *fields[6];
        int i = 0;
        while (token != NULL && i < 6) {
            fields[i++] = token;
            token = strtok(NULL, "|");
        }
        if (i != 6) {
            fprintf(stderr, "Error: Invalid file format. Each line must have exactly six fields.\n");
            exit(1);
        }

        entries[count].id = atoi(fields[0]);
        strncpy(entries[count].date, fields[1], MAX_STR);
        strncpy(entries[count].type, fields[2], MAX_STR);
        strncpy(entries[count].subtype, fields[3], MAX_STR);
        strncpy(entries[count].description, fields[4], MAX_STR);
        entries[count].amount = atof(fields[5]);

        count++;
    }

    fclose(file);
    return count;
}

FinanceEntry entries[MAX_ENTRIES];
int count;
FinanceEntry lastEntryBackup;
int lastActionType = 0; // 0 = none, 1 = add, 2 = modify
int lastModifiedIndex = -1;

int main() {
    count = readFile(entries, "finances.txt");

    int choice;
    bool running = true;
    while (running) {
        system("cls");
        printf("Budget Tracking System\n"
        "========================\n"
        "1. Display Budget\n"
        "2. Expense Distribution\n"
        "3. Sort Entries\n"
        "4. Add Income/Expense Entry\n"
        "5. Modify Entry\n"
        "6. Filter by Month\n"
        "7. Undo Last Action\n"
        "8. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);
        if (choice < 1 || choice > 8) {
            printf("Invalid choice. Please try again.\n");
            continue;
        }
        switch (choice) {
        case 1:
            displayBudget();
            break;
        case 2:
            expenseDistribution();
            break;
        case 3:
            sortEntries();
            break;
        case 4:
            addEntry();
            break;
        case 5:
            modifyEntry();
            break;
        case 6:
            filterByMonth();
            break;
        case 7:
            undoLastAction();
            break;
        case 8:
            printf("Exiting...\n");
            return 0;
        default:
            printf("Invalid choice. Please try again.\n");
        }
    }
    return 0;
}

void displayBudget() {
    printf("Finaces Summary\n"
           "========================\n"
           "ID | Date       | Type   | Subtype | Description | Amount\n"
           "---------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("%d | %s | %s | %s | %s | %.2f\n",
            entries[i].id,
            entries[i].date,
            entries[i].type,
            entries[i].subtype,
            entries[i].description,
            entries[i].amount
        );
    }
    printf("\nPress ENTER to return to menu...\n");
    getchar();  // absorb leftover '\n'
    getchar();  // wait for real ENTER
}

void expenseDistribution() {
    float totIncome = 0, totExpenses = 0;
    float needs = 0, wants = 0;

    printf("===== Expense Distribution Report =====\n");

    for (int i = 0; i < count; i++) {
        if (strcmp(entries[i].type, "income") == 0) {
            totIncome += entries[i].amount;
        } else if (strcmp(entries[i].type, "expense") == 0) {
            totExpenses += entries[i].amount;

            if (strcmp(entries[i].subtype, "needs") == 0) {
                needs += entries[i].amount;
            } else if (strcmp(entries[i].subtype, "wants") == 0) {
                wants += entries[i].amount;
            }
        }
    }
    float netBalance = totIncome - totExpenses;
    float needsPercentOfExpenses = (totExpenses > 0) ? (needs / totExpenses * 100) : 0;
    float wantsPercentOfExpenses = (totExpenses > 0) ? (wants / totExpenses * 100) : 0;
    float needsPercentOfIncome = (totIncome > 0) ? (needs / totIncome * 100) : 0;
    float wantsPercentOfIncome = (totIncome > 0) ? (wants / totIncome * 100) : 0;

    printf("Total Income: $%.2f\n", totIncome);
    printf("Total Expenses: $%.2f\n", totExpenses);
    printf("Needs: $%.2f (%.2f%% of expenses, %.2f%% of income)\n",
           needs, needsPercentOfExpenses, needsPercentOfIncome);
    printf("Wants: $%.2f (%.2f%% of expenses, %.2f%% of income)\n",
           wants, wantsPercentOfExpenses, wantsPercentOfIncome);
    printf("Net Balance: $%.2f\n", netBalance);
    printf("=======================================\n");
    printf("\nPress ENTER to return to menu...\n");
    getchar();  // absorb leftover '\n'
    getchar();  // wait for real ENTER
}

int compareMode = 1; // 1: Date, 2: Type, 3: Amount, 4: Description
int universalComparator(const void *a, const void *b) {
    FinanceEntry *e1 = (FinanceEntry *)a;
    FinanceEntry *e2 = (FinanceEntry *)b;

    switch (compareMode) {
        case 1:
            return strcmp(e1->date, e2->date);
        case 2:
            return strcmp(e1->type, e2->type);
        case 3:
            if (e1->amount < e2->amount) return -1;
        if (e1->amount > e2->amount) return 1;
        return 0;
        case 4:
            return strcmp(e1->description, e2->description);
        default:
            return 0;
    }
}
void sortEntries() {
    int choice;
    printf("Sort Menu\n"
           "1. Sort by ID\n"
           "2. Sort by Date\n"
           "3. Sort by Amount\n"
           "4. Sort by Description\n"
           "Choice: ");
    scanf("%d", &choice);

    int (*comparator)(const void *, const void *) = NULL;

    switch (choice) {
        case 1:
            compareMode = 1;
            break;
        case 2:
            compareMode = 2;
            break;
        case 3:
            compareMode = 3;
            break;
        case 4:
            compareMode = 4;
            break;
        default:
            printf("Invalid choice.\n");
        return;
    }

    qsort(entries, count, sizeof(FinanceEntry), universalComparator);
}

void addEntry() {
    FinanceEntry newEntry;
    char useToday;

    // Generate the next ID
    newEntry.id = (count == 0) ? 1 : entries[count - 1].id + 1;

    // Ask if user wants to use today's date
    printf("Use today's date? (y/n): ");
    scanf(" %c", &useToday);

    if (useToday == 'y' || useToday == 'Y') {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        strftime(newEntry.date, sizeof(newEntry.date), "%Y-%m-%d", tm_info);
    } 
    else {
        printf("Enter date (YYYY-MM-DD): ");
        scanf("%s", newEntry.date);
    }

    // Get type (income or expense)
    printf("Type (income/expense): ");
    scanf("%s", newEntry.type);

    // Get category (Needs/Wants/etc.)
    printf("Category: ");
    scanf("%s", newEntry.subtype);

    // Get description
    printf("Description: ");
    getchar();  // Clear leftover newline
    fgets(newEntry.description, sizeof(newEntry.description), stdin);
    newEntry.description[strcspn(newEntry.description, "\n")] = '\0'; // remove trailing newline

    // Get amount
    printf("Amount:$");
    scanf("%f", &newEntry.amount);

    // Add to entries list
    if (count < MAX_ENTRIES) {
        lastEntryBackup = newEntry;
        lastActionType = 1; // means it was an add
        entries[count++] = newEntry;
        printf("Entry added successfully with ID: %d\n", newEntry.id);
    } else {
        printf("Error: Maximum entry limit reached.\n");
    }

    // Optional: Wait for user to continue
    printf("Press ENTER to return to menu...\n");
    getchar();  // absorb leftover newline
    getchar();  // wait for real ENTER
}

void modifyEntry() {
    int id, found = -1;
    printf("Enter ID of entry to modify: ");
    scanf("%d", &id);

    // Search for entry by ID
    for (int i = 0; i < count; i++) {
        if (entries[i].id == id) {
            found = i;
            break;
        }
    }

    if (found == -1) {
        printf("Entry with ID %d not found.\n", id);
        printf("Press ENTER to return to menu...\n");
        getchar(); getchar();
        return;
    }

    lastEntryBackup = entries[found];
    lastActionType = 2; // means modify
    lastModifiedIndex = found;

    // Show current entry
    FinanceEntry *e = &entries[found];
    printf("\nCurrent Details:\n");
    printf("ID: %d\n", e->id);
    printf("Date: %s\n", e->date);
    printf("Type: %s\n", e->type);
    printf("Category: %s\n", e->subtype);
    printf("Description: %s\n", e->description);
    printf("Amount: $%.2f\n", e->amount);

    // Ask what to modify
    int choice;
    printf("\nWhat would you like to modify?\n");
    printf("1. Date\n");
    printf("2. Amount\n");
    printf("Choice: ");
    scanf("%d", &choice);

    if (choice == 1) {
        printf("Enter new date (YYYY-MM-DD): ");
        scanf("%s", e->date);
        printf("Date updated successfully.\n");
    } else if (choice == 2) {
        printf("Enter new amount: $");
        scanf("%f", &e->amount);
        printf("Amount updated successfully.\n");
    } else {
        printf("Invalid choice.\n");
    }

    printf("Press ENTER to return to menu...\n");
    getchar();  // absorb leftover newline
    getchar();  // wait for ENTER
}

void filterByMonth() {
    int year, month;
    printf("Enter year (YYYY): ");
    scanf("%d", &year);

    printf("Enter month (1-12): ");
    scanf("%d", &month);

    if (month < 1 || month > 12) {
        printf("Invalid month.\n");
        printf("Press ENTER to return to menu...\n");
        getchar(); getchar();
        return;
    }

    printf("\nEntries for %04d-%02d:\n", year, month);
    printf("ID | Date       | Type    | Category | Description | Amount\n");
    printf("------------------------------------------------------------\n");

    char target[8]; // format: YYYY-MM
    snprintf(target, sizeof(target), "%04d-%02d", year, month);

    int found = 0;
    for (int i = 0; i < count; i++) {
        if (strncmp(entries[i].date, target, 7) == 0) {
            printf("%d | %s | %s | %s | %s | $%.2f\n",
                entries[i].id,
                entries[i].date,
                entries[i].type,
                entries[i].subtype,
                entries[i].description,
                entries[i].amount
            );
            found++;
        }
    }

    if (found == 0) {
        printf("No entries found for %04d-%02d.\n", year, month);
    }

    printf("\nPress ENTER to return to menu...\n");
    getchar(); getchar();
}

// EXTRAS
void printBar(float value, float total) {
    int stars = (int)((value / total) * 50);  // Max 50 stars
    for (int i = 0; i < stars; i++) printf("*");
    printf(" (%.2f%%)\n", (value / total) * 100);
}

void visualExpenseBreakdown() {
    float needs = 0, wants = 0, others = 0, total = 0;

    // Calculate totals
    for (int i = 0; i < count; i++) {
        if (strcmp(entries[i].type, "expense") == 0) {
            total += entries[i].amount;

            if (strcasecmp(entries[i].subtype, "needs") == 0)
                needs += entries[i].amount;
            else if (strcasecmp(entries[i].subtype, "wants") == 0)
                wants += entries[i].amount;
            else
                others += entries[i].amount;
        }
    }

    if (total == 0) {
        printf("No expense entries to display.\n");
        return;
    }

    printf("\nVisual Expense Breakdown:\n");
    printf("Needs   : ");
    printBar(needs, total);
    printf("Wants   : ");
    printBar(wants, total);
    printf("Others  : ");
    printBar(others, total);

    printf("\nPress ENTER to return to menu...\n");
    getchar(); getchar();
}

void searchTransactionsByAmount() {
    float min, max;
    printf("Enter minimum amount: $");
    scanf("%f", &min);
    printf("Enter maximum amount: $");
    scanf("%f", &max);

    printf("\nTransactions between $%.2f and $%.2f:\n", min, max);
    printf("ID | Date       | Type    | Category | Description | Amount\n");
    printf("------------------------------------------------------------\n");

    int found = 0;
    for (int i = 0; i < count; i++) {
        if (entries[i].amount >= min && entries[i].amount <= max) {
            printf("%d | %s | %s | %s | %s | $%.2f\n",
                entries[i].id,
                entries[i].date,
                entries[i].type,
                entries[i].subtype,
                entries[i].description,
                entries[i].amount);
            found++;
        }
    }

    if (found == 0)
        printf("No transactions found in that range.\n");

    printf("\nPress ENTER to return to menu...\n");
    getchar(); getchar();
}

void undoLastAction() {
    if (lastActionType == 0) {
        printf("No action to undo.\n");
    }
    else if (lastActionType == 1) {
        // Undo Add: remove last added entry
        count--;
        printf("Last added entry has been removed.\n");
    }
    else if (lastActionType == 2 && lastModifiedIndex >= 0) {
        // Undo Modify: restore original entry
        entries[lastModifiedIndex] = lastEntryBackup;
        printf("Last modified entry has been restored.\n");
    }

    // Reset undo state
    lastActionType = 0;
    lastModifiedIndex = -1;

    printf("Press ENTER to return to menu...\n");
    getchar(); getchar();
}