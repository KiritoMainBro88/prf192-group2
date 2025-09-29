#include <stdio.h>
#include <stdlib.h>

// Main function
int main() {
    int choice;

    printf("=========================================\n");
    printf(" Welcome! This is BOOK MANAGEMENT SYSTEM \n");
    printf("              From Team 2                \n");
    printf("=========================================\n");
    printf("Press Enter to continue...");
    getchar();

    do {
        system("cls");  // clear screen (Windows)
        printf("\n=============== MAIN MENU ===============\n");
        printf("1. Add new book\n");
        printf("2. Delete a book\n");
        printf("3. Edit book information\n");
        printf("4. Search books\n");
        printf("5. Show all books\n");
        printf("6. Sort books by Title/Author/Year\n");
        printf("7. Save books to file\n");
        printf("8. Load books from file\n");
        printf("9. User Login / Register\n");
        printf("0. Exit program\n");
        printf("=========================================\n");
        printf("Please enter your choice: ");

        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("\n>>> Add new book feature...\n");
                while(getchar() != '\n'); getchar();
                break;
            case 2:
                printf("\n>>> Delete book feature...\n");
                while(getchar() != '\n'); getchar();
                break;
            case 3:
                printf("\n>>> Edit book information feature...\n");
                while(getchar() != '\n'); getchar();
                break;
            case 4:
                printf("\n>>> Search book feature...\n");
                while(getchar() != '\n'); getchar();
                break;
            case 5:
                printf("\n>>> Show all books feature...\n");
                while(getchar() != '\n'); getchar();
                break;
            case 6:
                printf("\n>>> Sort books feature...\n");
                while(getchar() != '\n'); getchar();
                break;
            case 7:
                printf("\n>>> Save to file feature...\n");
                while(getchar() != '\n'); getchar();
                break;
            case 8:
                printf("\n>>> Load from file feature...\n");
                while(getchar() != '\n'); getchar();
                break;
            case 9:
                printf("\n>>> User Login/Register feature...\n");
                while(getchar() != '\n'); getchar();
                break;
            case 0:
                printf("\nExiting program. Goodbye!\n");
                break;
            default:
                printf("\nError: Invalid choice. Try again!\n");
                while(getchar() != '\n'); getchar();
                break;
        }

    } while (choice != 0);

    return 0;
}
