#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
  #define CLEAR "cls"
#else
  #define CLEAR "clear"
#endif

// ==========================
// Data model (struct + store)
// ==========================
#define MAX 1000
#define ID_MAX 16
#define STR_MAX 64

typedef struct {
    char  id[ID_MAX];      // chỉ chấp nhận toàn chữ số
    char  title[STR_MAX];
    char  author[STR_MAX]; // có thể trống (dùng "-" để bỏ qua khi nhập/sửa)
    int   year;
    int   priceVND;        // tiền theo VND (đồng), số nguyên
} Book;

Book books[MAX];
int nBooks = 0;

// ==============
// Helper input
// ==============
void clearInputBuffer(void){
    int c;
    while((c = getchar()) != '\n' && c != EOF) {}
}

void pauseScreen(void){
    printf("\nNhan Enter de tiep tuc...");
    clearInputBuffer();
    getchar();
}

// kiểm tra chuỗi toàn chữ số (>=1 ký tự)
int isDigits(const char *s){
    if(s==NULL || *s=='\0') return 0;
    for(int i=0; s[i]; i++){
        if(!isdigit((unsigned char)s[i])) return 0;
    }
    return 1;
}

// đọc ID: bắt buộc toàn chữ số, độ dài <= ID_MAX-1
void readId(char *out){
    char buf[ID_MAX*2];
    while(1){
        printf("ID (chi nhap so): ");
        if(scanf("%15s", buf) != 1){ puts("Nhap loi."); clearInputBuffer(); continue; }
        if(!isDigits(buf)){ puts("Hay nhap so, khong nhap chu!"); continue; }
        if(strlen(buf) >= ID_MAX){ puts("ID qua dai!"); continue; }
        strcpy(out, buf);
        break;
    }
}

// đọc số nguyên không âm (VND, Year >= 0 cho đơn giản)
int readIntNonNegative(const char *label){
    char buf[64]; long v; char *end;
    while(1){
        printf("%s", label);
        if(scanf("%63s", buf) != 1){ puts("Nhap loi."); clearInputBuffer(); continue; }
        // chỉ chấp nhận nếu toàn chữ số
        if(!isDigits(buf)){ puts("Hay nhap so, khong nhap chu!"); continue; }
        v = strtol(buf, &end, 10);
        if(*end=='\0' && v >= 0) return (int)v;
        puts("Gia tri khong hop le!");
    }
}

// đọc chuỗi “không khoảng trắng” đơn giản; nếu cho phép bỏ trống → dùng "-"
void readToken(const char *label, char *out, int outsz, int allowEmptyDash){
    printf("%s", label);
    if(scanf("%63s", out) != 1){ puts("Nhap loi."); clearInputBuffer(); out[0]='\0'; return; }
    if(allowEmptyDash && strcmp(out, "-")==0){ out[0]='\0'; return; }
}

// ==================
// Core search helper
// ==================
int findById(const char *id){
    for(int i=0;i<nBooks;i++){
        if(strcmp(books[i].id,id)==0) return i;
    }
    return -1;
}

// =======
// Add
// =======
void addBook(){
    if(nBooks>=MAX){ puts("Full!"); return; }
    Book b;

    readId(b.id);
    if(findById(b.id)!=-1){ puts("ID exists!"); return; }

    readToken("Title: ",  b.title,  STR_MAX, 0);
    readToken("Author (go '-' de bo qua): ", b.author, STR_MAX, 1);
    b.year     = readIntNonNegative("Year (>=0): ");
    b.priceVND = readIntNonNegative("Price (VND, >=0): ");

    books[nBooks++] = b;
    puts("Added!");
}

// =======
// Delete
// =======
void delBook(){
    char id[ID_MAX];
    readId(id);
    int k = findById(id);
    if(k==-1){ puts("Not found"); return; }
    for(int i=k;i<nBooks-1;i++) books[i]=books[i+1];
    nBooks--; puts("Deleted!");
}

// =======
// Edit
// =======
void editBook(){
    char id[ID_MAX];
    readId(id);
    int k = findById(id);
    if(k==-1){ puts("Not found"); return; }

    // cho phép sửa từng trường; nếu muốn bỏ trống author, gõ "-"
    readToken("New Title: ",  books[k].title,  STR_MAX, 0);
    readToken("New Author (go '-' de bo qua): ", books[k].author, STR_MAX, 1);
    books[k].year     = readIntNonNegative("New Year (>=0): ");
    books[k].priceVND = readIntNonNegative("New Price (VND, >=0): ");

    puts("Updated!");
}

// =======
// Search
// =======
void searchBook(){
    int mode; 
    printf("Tim theo: 1-ID  2-Title : ");
    if(scanf("%d",&mode)!=1){ puts("Nhap loi."); clearInputBuffer(); return; }

    if(mode==1){
        char id[ID_MAX];
        readId(id);
        int i=findById(id);
        if(i==-1) puts("Not found");
        else{
            printf("%s %s %s %d %d VND\n",
                books[i].id, books[i].title,
                (books[i].author[0]?books[i].author:"(no_author)"),
                books[i].year, books[i].priceVND);
        }
    }else{
        char t[STR_MAX]; int ok=0;
        readToken("Title (chinh xac): ", t, STR_MAX, 0);
        for(int i=0;i<nBooks;i++){
            if(strcmp(books[i].title,t)==0){
                printf("%s %s %s %d %d VND\n",
                    books[i].id, books[i].title,
                    (books[i].author[0]?books[i].author:"(no_author)"),
                    books[i].year, books[i].priceVND);
                ok=1;
            }
        }
        if(!ok) puts("Not found");
    }
}

// =======
// Sort
// =======
void swapB(Book *a, Book *b){ Book t=*a; *a=*b; *b=t; }

void sortBooks(){
    int m; 
    printf("Sort by 1-Title 2-Author 3-Year: ");
    if(scanf("%d",&m)!=1){ puts("Nhap loi."); clearInputBuffer(); return; }

    for(int i=0;i<nBooks-1;i++){
        for(int j=0;j<nBooks-1-i;j++){
            int greater=0;
            if(m==1) greater = strcmp(books[j].title, books[j+1].title) > 0;
            if(m==2) greater = strcmp(books[j].author,books[j+1].author)> 0;
            if(m==3) greater = books[j].year > books[j+1].year;
            if(greater) swapB(&books[j], &books[j+1]);
        }
    }
    puts("Sorted!");
}

// =======
// Show
// =======
void showAll(){
    if(nBooks==0){ puts("Empty."); return; }
    printf("STT ID Title Author Year Price(VND)\n");
    for(int i=0;i<nBooks;i++)
        printf("%d %s %s %s %d %d VND\n",
            i+1, books[i].id, books[i].title,
            (books[i].author[0]?books[i].author:"(no_author)"),
            books[i].year, books[i].priceVND);
}

// =======
// Files
// =======
void saveFile(){
    FILE *f = fopen("books.txt","w");
    if(!f){ puts("Open fail"); return; }
    for(int i=0;i<nBooks;i++)
        // lưu text đơn giản, không khoảng trắng
        fprintf(f,"%s %s %s %d %d\n",
            books[i].id, books[i].title,
            (books[i].author[0]?books[i].author:"-"),
            books[i].year, books[i].priceVND);
    fclose(f); puts("Saved!");
}

void loadFile(){
    FILE *f = fopen("books.txt","r");
    if(!f){ puts("No file"); return; }
    nBooks = 0;
    while(nBooks<MAX &&
          fscanf(f,"%15s %63s %63s %d %d",
                books[nBooks].id, books[nBooks].title, books[nBooks].author,
                &books[nBooks].year, &books[nBooks].priceVND) == 5){
        // chuyển "-" thành rỗng cho author
        if(strcmp(books[nBooks].author,"-")==0) books[nBooks].author[0]='\0';
        nBooks++;
    }
    fclose(f); puts("Loaded!");
}

// =======
// Login
// =======
int userExists(const char *u){
    FILE *f=fopen("users.txt","r"); if(!f) return 0;
    char U[64],P[64]; int ok=0;
    while(fscanf(f,"%63s %63s",U,P)==2){
        if(strcmp(U,u)==0){ ok=1; break; }
    }
    fclose(f); return ok;
}
int checkLogin(const char *u,const char *p){
    FILE *f=fopen("users.txt","r"); if(!f) return 0;
    char U[64],P[64]; int ok=0;
    while(fscanf(f,"%63s %63s",U,P)==2){
        if(strcmp(U,u)==0 && strcmp(P,p)==0){ ok=1; break; }
    }
    fclose(f); return ok;
}
void loginRegister(){
    int c; char u[64],p[64];
    printf("1-Login 2-Register: ");
    if(scanf("%d",&c)!=1){ puts("Nhap loi."); clearInputBuffer(); return; }

    if(c==1){
        printf("User: "); scanf("%63s",u);
        printf("Pass: "); scanf("%63s",p);
        puts(checkLogin(u,p)?"Login OK":"Wrong info");
    }else{
        printf("New user: "); scanf("%63s",u);
        if(userExists(u)){ puts("Exists"); return; }
        printf("New pass: "); scanf("%63s",p);
        FILE *f=fopen("users.txt","a"); if(!f){ puts("Open fail"); return; }
        fprintf(f,"%s %s\n",u,p); fclose(f); puts("Registered");
    }
}

// ============
// Main
// ============
int main() {
    int choice;

    printf("=========================================\n");
    printf(" Welcome! This is BOOK MANAGEMENT SYSTEM \n");
    printf("              From Team 2                \n");
    printf("=========================================\n");
    printf("Press Enter to continue...");
    getchar();

    do {
        system(CLEAR);  // clear screen (Windows/Linux/Mac)
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

        if(scanf("%d", &choice)!=1){ puts("Nhap loi!"); clearInputBuffer(); choice = -1; }

        switch (choice) {
            case 1:
                addBook();        pauseScreen(); break;
            case 2:
                delBook();        pauseScreen(); break;
            case 3:
                editBook();       pauseScreen(); break;
            case 4:
                searchBook();     pauseScreen(); break;
            case 5:
                showAll();        pauseScreen(); break;
            case 6:
                sortBooks();      pauseScreen(); break;
            case 7:
                saveFile();       pauseScreen(); break;
            case 8:
                loadFile();       pauseScreen(); break;
            case 9:
                loginRegister();  pauseScreen(); break;
            case 0:
                printf("\nExiting program. Goodbye!\n");
                break;
            default:
                printf("\nError: Invalid choice. Try again!\n");
                pauseScreen();
                break;
        }

    } while (choice != 0);

    return 0;
}
