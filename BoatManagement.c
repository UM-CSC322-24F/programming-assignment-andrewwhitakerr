//-------------------------------------------------------------------------------------------------
// Include necessary headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//-------------------------------------------------------------------------------------------------
// Define constants as macros
#define MAX_BOATS 120
#define MAX_NAME_LENGTH 127
#define MAX_LENGTH 100
#define SLIP_RATE 12.50
#define LAND_RATE 14.00
#define TRAILOR_RATE 25.00
#define STORAGE_RATE 11.20
//-------------------------------------------------------------------------------------------------
// Type definitions
typedef enum {
    slip,
    land,
    trailor, 
    storage,
    no_place
} PlaceType;

typedef union {
    int slipNumber;        // 1-85
    char bayLetter;        // A-Z
    char trailorTag[10];   // License tag
    int storageNumber;     // 1-50
} LocationType;

typedef struct {
    char name[MAX_NAME_LENGTH + 1];
    float length;
    PlaceType place;
    LocationType location;
    float amountOwed;
} BoatType;

typedef BoatType* BoatPointer;
//-------------------------------------------------------------------------------------------------

// Function prototypes
void welcomeMessage(void);
void exitMessage(void);
char getMenuChoice(void);
void loadBoats(BoatPointer boats[], int* numBoats, const char* filename);
void saveBoats(BoatPointer boats[], int numBoats, const char* filename);
void printInventory(BoatPointer boats[], int numBoats);
void addBoat(BoatPointer boats[], int* numBoats);
void removeBoat(BoatPointer boats[], int* numBoats);
void makePayment(BoatPointer boats[], int numBoats);
void updateMonth(BoatPointer boats[], int numBoats);
int compareBoats(const void* a, const void* b);
PlaceType stringToPlaceType(const char* placeString);
const char* placeTypeToString(PlaceType place);
void cleanupMemory(BoatPointer boats[], int numBoats);
int findBoat(BoatPointer boats[], int numBoats, const char* name);
//-------------------------------------------------------------------------------------------------

// Main function
int main(int argc, char* argv[]) {
    BoatPointer boats[MAX_BOATS] = {NULL};
    int numBoats = 0;
    char choice;

    // Check command line arguments
    if (argc != 2) {
        printf("Usage: %s <data_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Load data and display welcome
    loadBoats(boats, &numBoats, argv[1]);
    welcomeMessage();

    // Main menu loop
    while (1) {
        choice = getMenuChoice();
        switch (tolower(choice)) {
            case 'i':
                printInventory(boats, numBoats);
                break;
            case 'a':
                addBoat(boats, &numBoats);
                break;
            case 'r':
                removeBoat(boats, &numBoats);
                break;
            case 'p':
                makePayment(boats, numBoats);
                break;
            case 'm':
                updateMonth(boats, numBoats);
                break;
            case 'x':
                exitMessage();
                saveBoats(boats, numBoats, argv[1]);
                cleanupMemory(boats, numBoats);
                return EXIT_SUCCESS;
            default:
                printf("Invalid option %c\n\n", choice);
                break;
        }
    }
}
//-------------------------------------------------------------------------------------------------

// Display welcome message
void welcomeMessage(void) {
    printf("\nWelcome to the Boat Management System\n");
    printf("-------------------------------------\n\n");
}
//-------------------------------------------------------------------------------------------------

// Display exit message
void exitMessage(void) {
    printf("\nExiting the Boat Management System\n\n");
}
//-------------------------------------------------------------------------------------------------

// Get menu choice from user
char getMenuChoice(void) {
    char choice;
    printf("(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
    scanf(" %c", &choice);
    return choice;
}
//-------------------------------------------------------------------------------------------------

// Load boats from CSV file
void loadBoats(BoatPointer boats[], int* numBoats, const char* filename) {
    FILE* file = fopen(filename, "r");
    char line[256];
    char place[20];
    char location[20];
    float amount;

    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    while (fgets(line, sizeof(line), file) && *numBoats < MAX_BOATS) {
        boats[*numBoats] = (BoatType*)malloc(sizeof(BoatType));
        if (!boats[*numBoats]) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }

        // Parse CSV line
        if (sscanf(line, "%[^,],%f,%[^,],%[^,],%f",
                   boats[*numBoats]->name,
                   &boats[*numBoats]->length,
                   place,
                   location,
                   &boats[*numBoats]->amountOwed) == 5) {
            
            boats[*numBoats]->place = stringToPlaceType(place);
            
            // Handle location based on place type
            switch (boats[*numBoats]->place) {
                case slip:
                    boats[*numBoats]->location.slipNumber = atoi(location);
                    break;
                case land:
                    boats[*numBoats]->location.bayLetter = location[0];
                    break;
                case trailor:
                    strncpy(boats[*numBoats]->location.trailorTag, location, 9);
                    boats[*numBoats]->location.trailorTag[9] = '\0';
                    break;
                case storage:
                    boats[*numBoats]->location.storageNumber = atoi(location);
                    break;
                default:
                    break;
            }
            (*numBoats)++;
        }
    }
    
    fclose(file);
    qsort(boats, *numBoats, sizeof(BoatPointer), compareBoats);
}
//-------------------------------------------------------------------------------------------------

// Save boats to CSV file
void saveBoats(BoatPointer boats[], int numBoats, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file for writing");
        return;
    }

    for (int i = 0; i < numBoats; i++) {
        fprintf(file, "%s,%.0f,%s,", 
                boats[i]->name,
                boats[i]->length,
                placeTypeToString(boats[i]->place));

        // Output location based on place type
        switch (boats[i]->place) {
            case slip:
                fprintf(file, "%d", boats[i]->location.slipNumber);
                break;
            case land:
                fprintf(file, "%c", boats[i]->location.bayLetter);
                break;
            case trailor:
                fprintf(file, "%s", boats[i]->location.trailorTag);
                break;
            case storage:
                fprintf(file, "%d", boats[i]->location.storageNumber);
                break;
            default:
                break;
        }
        
        fprintf(file, ",%.2f\n", boats[i]->amountOwed);
    }

    fclose(file);
}
//-------------------------------------------------------------------------------------------------

// Print boat inventory
void printInventory(BoatPointer boats[], int numBoats) {
    for (int i = 0; i < numBoats; i++) {
        printf("%-20s %3.0f' ", boats[i]->name, boats[i]->length);
        
        switch (boats[i]->place) {
            case slip:
                printf("%7s   # %2d", placeTypeToString(boats[i]->place),
                       boats[i]->location.slipNumber);
                break;
            case land:
                printf("%7s      %c", placeTypeToString(boats[i]->place),
                       boats[i]->location.bayLetter);
                break;
            case trailor:
                printf("%7s %6s", placeTypeToString(boats[i]->place),
                       boats[i]->location.trailorTag);
                break;
            case storage:
                printf("%7s   # %2d", placeTypeToString(boats[i]->place),
                       boats[i]->location.storageNumber);
                break;
            default:
                break;
        }
        printf("   Owes $%7.2f\n", boats[i]->amountOwed);
    }
    printf("\n");
}
//-------------------------------------------------------------------------------------------------

// Add a new boat
void addBoat(BoatPointer boats[], int* numBoats) {
    if (*numBoats >= MAX_BOATS) {
        printf("Marina is full\n\n");
        return;
    }

    char boatData[256];
    printf("Please enter the boat data in CSV format                 : ");
    scanf(" %[^\n]", boatData);

    boats[*numBoats] = (BoatType*)malloc(sizeof(BoatType));
    if (!boats[*numBoats]) {
        perror("Memory allocation failed");
        return;
    }

    char place[20];
    char location[20];
    if (sscanf(boatData, "%[^,],%f,%[^,],%[^,],%f",
               boats[*numBoats]->name,
               &boats[*numBoats]->length,
               place,
               location,
               &boats[*numBoats]->amountOwed) == 5) {
        
        boats[*numBoats]->place = stringToPlaceType(place);
        
        switch (boats[*numBoats]->place) {
            case slip:
                boats[*numBoats]->location.slipNumber = atoi(location);
                break;
            case land:
                boats[*numBoats]->location.bayLetter = location[0];
                break;
            case trailor:
                strncpy(boats[*numBoats]->location.trailorTag, location, 9);
                boats[*numBoats]->location.trailorTag[9] = '\0';
                break;
            case storage:
                boats[*numBoats]->location.storageNumber = atoi(location);
                break;
            default:
                break;
        }
        
        (*numBoats)++;
        qsort(boats, *numBoats, sizeof(BoatPointer), compareBoats);
    }
    printf("\n");
}
//-------------------------------------------------------------------------------------------------

// Remove a boat
void removeBoat(BoatPointer boats[], int* numBoats) {
    char name[MAX_NAME_LENGTH + 1];
    printf("Please enter the boat name                               : ");
    scanf(" %[^\n]", name);

    int index = findBoat(boats, *numBoats, name);
    if (index == -1) {
        printf("No boat with that name\n\n");
        return;
    }

    free(boats[index]);
    for (int i = index; i < *numBoats - 1; i++) {
        boats[i] = boats[i + 1];
    }
    (*numBoats)--;
    printf("\n");
}
//-------------------------------------------------------------------------------------------------

// Make a payment for a boat
void makePayment(BoatPointer boats[], int numBoats) {
    char name[MAX_NAME_LENGTH + 1];
    float payment;

    printf("Please enter the boat name                               : ");
    scanf(" %[^\n]", name);

    int index = findBoat(boats, numBoats, name);
    if (index == -1) {
        printf("No boat with that name\n\n");
        return;
    }

    printf("Please enter the amount to be paid                       : ");
    scanf("%f", &payment);

    if (payment > boats[index]->amountOwed) {
        printf("That is more than the amount owed, $%.2f\n\n", 
               boats[index]->amountOwed);
        return;
    }

    boats[index]->amountOwed -= payment;
    printf("\n");
}
//-------------------------------------------------------------------------------------------------

// Update monthly charges
void updateMonth(BoatPointer boats[], int numBoats) {
    for (int i = 0; i < numBoats; i++) {
        switch (boats[i]->place) {
            case slip:
                boats[i]->amountOwed += boats[i]->length * SLIP_RATE;
                break;
            case land:
                boats[i]->amountOwed += boats[i]->length * LAND_RATE;
                break;
            case trailor:
                boats[i]->amountOwed += boats[i]->length * TRAILOR_RATE;
                break;
            case storage:
                boats[i]->amountOwed += boats[i]->length * STORAGE_RATE;
                break;
            default:
                break;
        }
    }
    printf("\n");
}
//-------------------------------------------------------------------------------------------------

// Compare boats for sorting
int compareBoats(const void* a, const void* b) {
    BoatPointer boat1 = *(BoatPointer*)a;
    BoatPointer boat2 = *(BoatPointer*)b;
    return strcasecmp(boat1->name, boat2->name);
}
//-------------------------------------------------------------------------------------------------

// Convert string to PlaceType
PlaceType stringToPlaceType(const char* placeString) {
    if (strcasecmp(placeString, "slip") == 0) return slip;
    if (strcasecmp(placeString, "land") == 0) return land;
    if (strcasecmp(placeString, "trailor") == 0) return trailor;
    if (strcasecmp(placeString, "storage") == 0) return storage;
    return no_place;
}
//-------------------------------------------------------------------------------------------------

// Convert PlaceType to string
const char* placeTypeToString(PlaceType place) {
    switch (place) {
        case slip: return "slip";
        case land: return "land";
        case trailor: return "trailor";
        case storage: return "storage";
        default: return "no_place";
    }
}
//-------------------------------------------------------------------------------------------------

// Find boat by name (case insensitive)
int findBoat(BoatPointer boats[], int numBoats, const char* name) {
    for (int i = 0; i < numBoats; i++) {
        if (strcasecmp(boats[i]->name, name) == 0) {
            return i;
        }
    }
    return -1;
}
//-------------------------------------------------------------------------------------------------

// Free allocated memory
void cleanupMemory(BoatPointer boats[], int numBoats) {
    for (int i = 0; i < numBoats; i++) {
        free(boats[i]);
    }
}
//-------------------------------------------------------------------------------------------------
