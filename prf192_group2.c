#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
  #define CLEAR "cls"
#else
  #define CLEAR "clear"
#endif

// ==============================
// Cấu hình & mô hình dữ liệu
// ==============================
#define MAX     1000
#define ID_MAX  16
#define STR_MAX 64

typedef struct {
    char  id[ID_MAX];      // chỉ chấp nhận toàn chữ số
    char  title[STR_MAX];  // cho phép khoảng trắng (lưu file sẽ đổi ' ' -> '_')
    char  author[STR_MAX]; // rỗng nếu bỏ qua
    int   year;            // >= 0
    int   priceVND;        // >= 0
} Book;

static Book books[MAX];
static int  nBooks = 0;

// ==============================
// Tiện ích chuỗi & nhập liệu
// ==============================
static void trim(char *s){
    if(!s) return;
    int len = (int)strlen(s);
    while(len>0 && (s[len-1]=='\n' || s[len-1]=='\r' || isspace((unsigned char)s[len-1]))) s[--len]='\0';
    int i=0; while(s[i] && isspace((unsigned char)s[i])) i++;
    if(i>0) memmove(s, s+i, strlen(s+i)+1);
}

static void toLowerStr(const char *src, char *dst, size_t dstsz){
    size_t i=0;
    for(; src[i] && i+1<dstsz; ++i) dst[i]=(char)tolower((unsigned char)src[i]);
    if(dstsz) dst[i]='\0';
}

// contains không phân biệt hoa/thường
static int icontains(const char *haystack, const char *needle){
    char a[STR_MAX*2], b[STR_MAX*2];
    toLowerStr(haystack, a, sizeof(a));
    toLowerStr(needle,   b, sizeof(b));
    return strstr(a,b)!=NULL;
}

// Thay ' ' -> '_' khi ghi file "1 dòng 5 cột tách bằng khoảng"
static void spaces_to_underscores(const char *in, char *out, size_t outsz){
    size_t i=0;
    for(; in[i] && i+1<outsz; ++i) out[i] = (in[i]==' ') ? '_' : in[i];
    if(outsz) out[i]='\0';
}
// Ngược lại khi hiển thị/đưa vào bộ nhớ
static void underscores_to_spaces(const char *in, char *out, size_t outsz){
    size_t i=0;
    for(; in[i] && i+1<outsz; ++i) out[i] = (in[i]=='_') ? ' ' : in[i];
    if(outsz) out[i]='\0';
}

// Định dạng VND có dấu phân tách nghìn (1,234,567)
static void formatVND(int v, char *out, size_t outsz){
    char buf[64]; snprintf(buf, sizeof(buf), "%d", v);
    int len=(int)strlen(buf);
    // đếm dấu ','
    int commas = (len>0) ? (len-1)/3 : 0;
    int newlen = len + commas;
    if((size_t)(newlen+1) > outsz) { snprintf(out, outsz, "%d", v); return; }

    out[newlen] = '\0';
    int i=len-1, j=newlen-1, k=0;
    while(i>=0){
        out[j--]=buf[i--];
        if(++k==3 && i>=0){ out[j--]=','; k=0; }
    }
}

static void waitEnter(void){
    printf("\nNhan Enter de tiep tuc...");
    fflush(stdout);
    // đọc đến hết dòng
    int c;
    while ((c=getchar())!='\n' && c!=EOF) {}
}

// đọc 1 dòng có prompt, bắt buộc không rỗng (trừ khi allowEmpty=1)
static void readLine(const char *label, char *out, int outsz, int allowEmpty){
    while(1){
        printf("%s", label);
        fflush(stdout);
        if(!fgets(out, outsz, stdin)){
            clearerr(stdin);
            continue;
        }
        trim(out);
        if(!allowEmpty && out[0]=='\0'){
            puts("Gia tri khong duoc rong!");
            continue;
        }
        return;
    }
}

// đọc số nguyên trong [min,max]
static int readIntRange(const char *label, int minv, int maxv){
    char buf[128];
    while(1){
        printf("%s", label);
        fflush(stdout);
        if(!fgets(buf, sizeof(buf), stdin)){ clearerr(stdin); continue; }
        trim(buf);
        if(buf[0]=='\0'){ puts("Gia tri khong duoc rong!"); continue; }
        int ok=1;
        for(size_t i=0;i<strlen(buf);++i) if(!(i==0 && (buf[i]=='-'||buf[i]=='+')) && !isdigit((unsigned char)buf[i])){ ok=0; break; }
        if(!ok){ puts("Hay nhap so nguyen!"); continue; }
        long v = strtol(buf,NULL,10);
        if(v<minv || v>maxv){ printf("Chi chap nhan tu %d den %d\n", minv, maxv); continue; }
        return (int)v;
    }
}

// đọc ID (toàn chữ số, dài <= ID_MAX-1)
static void readId(char *out){
    char buf[ID_MAX*2];
    while(1){
        readLine("ID (chi nhap so): ", buf, sizeof(buf), 0);
        int ok = buf[0]!='\0';
        for(int i=0; buf[i]; ++i) if(!isdigit((unsigned char)buf[i])){ ok=0; break; }
        if(!ok){ puts("ID phai toan chu so!"); continue; }
        if((int)strlen(buf) >= ID_MAX){ puts("ID qua dai!"); continue; }
        strcpy(out, buf);
        return;
    }
}

// ==============================
// Tìm kiếm & thao tác dữ liệu
// ==============================
static int findById(const char *id){
    for(int i=0;i<nBooks;i++){
        if(strcmp(books[i].id,id)==0) return i;
    }
    return -1;
}

// In bảng đẹp
static void printTableHeader(void){
    puts("+----+----------------+------------------------------+------------------------------+------+--------------+");
    puts("| STT| ID             | Title                        | Author                       | Year | Price (VND)  |");
    puts("+----+----------------+------------------------------+------------------------------+------+--------------+");
}
static void printTableRow(int idx, const Book *b){
    char price[64]; formatVND(b->priceVND, price, sizeof(price));
    // khi hiển thị, dùng đúng chuỗi có khoảng trắng (đã lưu trong struct)
    printf("|%4d|%-16s|%-30s|%-30s|%6d|%14s|\n",
        idx+1,
        b->id,
        (b->title[0]? b->title : "(no_title)"),
        (b->author[0]? b->author: "(no_author)"),
        b->year,
        price
    );
}
static void printTableFooter(void){
    puts("+----+----------------+------------------------------+------------------------------+------+--------------+");
}

// ==============================
// Các chức năng
// ==============================
static void addBook(void){
    if(nBooks>=MAX){ puts("Kho sach day!"); return; }
    Book b; memset(&b,0,sizeof(b));

    readId(b.id);
    if(findById(b.id)!=-1){ puts("ID da ton tai!"); return; }

    readLine("Title: ",  b.title,  STR_MAX, 0);
    readLine("Author (go '-' de bo qua): ", b.author, STR_MAX, 1);
    if(strcmp(b.author,"-")==0) b.author[0]='\0';
    b.year     = readIntRange("Year (>=0): ", 0, 3000);
    b.priceVND = readIntRange("Price (VND, >=0): ", 0, 2000000000);

    books[nBooks++] = b;
    puts(">> Added!");
}

static void delBook(void){
    if(nBooks==0){ puts("Danh sach rong."); return; }
    char id[ID_MAX];
    readId(id);
    int k = findById(id);
    if(k==-1){ puts("Khong tim thay."); return; }

    char ans[8];
    printf("Ban co chac chan muon xoa ID %s? (Y/N): ", id);
    if(!fgets(ans,sizeof(ans),stdin)){ clearerr(stdin); return; }
    if(tolower((unsigned char)ans[0])!='y'){ puts(">> Huy xoa."); return; }

    for(int i=k;i<nBooks-1;i++) books[i]=books[i+1];
    nBooks--;
    puts(">> Deleted!");
}

static void editBook(void){
    if(nBooks==0){ puts("Danh sach rong."); return; }
    char id[ID_MAX];
    readId(id);
    int k = findById(id);
    if(k==-1){ puts("Khong tim thay."); return; }

    Book *bk = &books[k];
    char buf[STR_MAX];

    printf("New Title (bo trong de giu nguyen): ");
    if(fgets(buf,sizeof(buf),stdin)){ trim(buf); if(buf[0]!='\0') strncpy(bk->title, buf, STR_MAX-1); }

    printf("New Author (go '-' de bo qua, bo trong de giu nguyen): ");
    if(fgets(buf,sizeof(buf),stdin)){
        trim(buf);
        if(strcmp(buf,"-")==0) bk->author[0]='\0';
        else if(buf[0]!='\0') strncpy(bk->author, buf, STR_MAX-1);
    }

    int setYear = readIntRange("New Year (>=0, nhap so hien tai de giu): ", 0, 3000);
    bk->year = setYear;

    int setPrice = readIntRange("New Price (>=0, nhap so hien tai de giu): ", 0, 2000000000);
    bk->priceVND = setPrice;

    puts(">> Updated!");
}

static void searchBook(void){
    if(nBooks==0){ puts("Danh sach rong."); return; }
    puts("Tim theo: 1-ID  2-Title (substring, khong phan biet hoa/thuong)");
    int mode = readIntRange("Chon (1/2): ", 1, 2);

    if(mode==1){
        char id[ID_MAX];
        readId(id);
        int i=findById(id);
        if(i==-1) puts("Not found.");
        else{
            printTableHeader();
            printTableRow(i, &books[i]);
            printTableFooter();
        }
    }else{
        char key[STR_MAX];
        readLine("Nhap mot phan hoac toan bo Title: ", key, STR_MAX, 0);
        int cnt=0;
        printTableHeader();
        for(int i=0;i<nBooks;i++){
            if(icontains(books[i].title, key)){
                printTableRow(i, &books[i]);
                cnt++;
            }
        }
        printTableFooter();
        if(cnt==0) puts("Not found.");
        else printf(">> Found %d result(s).\n", cnt);
    }
}

static void swapB(Book *a, Book *b){ Book t=*a; *a=*b; *b=t; }

static void sortBooks(void){
    if(nBooks<=1){ puts("Khong can sap xep."); return; }
    puts("Sort by: 1-Title  2-Author  3-Year  4-Price");
    int m = readIntRange("Chon (1..4): ", 1, 4);
    int asc = readIntRange("Thu tu: 1-Asc  2-Desc: ", 1, 2);
    int isAsc = (asc==1);

    for(int i=0;i<nBooks-1;i++){
        for(int j=0;j<nBooks-1-i;j++){
            int greater=0; // j > j+1 ?
            if(m==1){
                greater = strcmp(books[j].title, books[j+1].title) > 0;
            }else if(m==2){
                greater = strcmp(books[j].author, books[j+1].author) > 0;
            }else if(m==3){
                greater = books[j].year > books[j+1].year;
            }else{
                greater = books[j].priceVND > books[j+1].priceVND;
            }
            if((isAsc && greater) || (!isAsc && !greater)){
                swapB(&books[j], &books[j+1]);
            }
        }
    }
    puts(">> Sorted!");
}

static void showAll(void){
    if(nBooks==0){ puts("Danh sach rong."); return; }
    printTableHeader();
    for(int i=0;i<nBooks;i++) printTableRow(i, &books[i]);
    printTableFooter();
    printf("Tong so sach: %d\n", nBooks);
}

// ==============================
// Lưu/đọc file (tương thích cũ)
// ==============================
static void saveFile(void){
    FILE *f = fopen("books.txt","w");
    if(!f){ puts("Open fail"); return; }
    for(int i=0;i<nBooks;i++){
        char t[STR_MAX], a[STR_MAX];
        // đổi ' ' -> '_' để không vỡ định dạng tách bằng khoảng
        spaces_to_underscores(books[i].title,  t, sizeof(t));
        spaces_to_underscores(books[i].author, a, sizeof(a));
        if(a[0]=='\0') strcpy(a,"-");
        fprintf(f,"%s %s %s %d %d\n",
            books[i].id, t, a, books[i].year, books[i].priceVND);
    }
    fclose(f);
    puts(">> Saved to books.txt");
}

static void loadFile(void){
    FILE *f = fopen("books.txt","r");
    if(!f){ puts("No file"); return; }
    nBooks = 0;
    char id[ID_MAX], t[STR_MAX], a[STR_MAX];
    int year, price;
    while(nBooks<MAX && fscanf(f,"%15s %63s %63s %d %d", id, t, a, &year, &price)==5){
        Book b; memset(&b,0,sizeof(b));
        strcpy(b.id, id);
        underscores_to_spaces(t, b.title,  STR_MAX);
        if(strcmp(a,"-")==0) b.author[0]='\0';
        else underscores_to_spaces(a, b.author, STR_MAX);
        b.year = year; b.priceVND = price;
        books[nBooks++] = b;
    }
    fclose(f);
    puts(">> Loaded!");
}

// ==============================
// Login / Register (đơn giản)
// ==============================
static int userExists(const char *u){
    FILE *f=fopen("users.txt","r"); if(!f) return 0;
    char U[64],P[64]; int ok=0;
    while(fscanf(f,"%63s %63s",U,P)==2){
        if(strcmp(U,u)==0){ ok=1; break; }
    }
    fclose(f); return ok;
}
static int checkLogin(const char *u,const char *p){
    FILE *f=fopen("users.txt","r"); if(!f) return 0;
    char U[64],P[64]; int ok=0;
    while(fscanf(f,"%63s %63s",U,P)==2){
        if(strcmp(U,u)==0 && strcmp(P,p)==0){ ok=1; break; }
    }
    fclose(f); return ok;
}
static void loginRegister(void){
    puts("1-Login  2-Register");
    int c = readIntRange("Chon (1/2): ", 1, 2);

    char u[64], p[64];
    readLine("User: ", u, sizeof(u), 0);
    readLine("Pass: ", p, sizeof(p), 0);

    if(c==1){
        puts(checkLogin(u,p) ? ">> Login OK" : ">> Wrong info");
    }else{
        if(userExists(u)){ puts(">> Username da ton tai"); return; }
        FILE *f=fopen("users.txt","a"); if(!f){ puts("Open fail"); return; }
        fprintf(f,"%s %s\n",u,p);
        fclose(f);
        puts(">> Registered");
    }
}

// ==============================
// Main menu (UI thân thiện)
// ==============================
int main(void){
    int choice;
    printf("================================================\n");
    printf("  BOOK MANAGEMENT SYSTEM  -  Team 2 (UI+ ver.)  \n");
    printf("================================================\n");
    printf("Press Enter to continue...");
    waitEnter();

    do{
        system(CLEAR);
        printf("\n==================== MAIN MENU ====================\n");
        printf(" 1. Add new book\n");
        printf(" 2. Delete a book\n");
        printf(" 3. Edit book information\n");
        printf(" 4. Search books (ID / Title substring)\n");
        printf(" 5. Show all books\n");
        printf(" 6. Sort books (Title/Author/Year/Price, Asc/Desc)\n");
        printf(" 7. Save books to file\n");
        printf(" 8. Load books from file\n");
        printf(" 9. User Login / Register\n");
        printf(" 0. Exit program\n");
        printf("====================================================\n");
        choice = readIntRange("Please enter your choice: ", 0, 9);

        system(CLEAR);
        switch(choice){
            case 1: addBook(); break;
            case 2: delBook(); break;
            case 3: editBook(); break;
            case 4: searchBook(); break;
            case 5: showAll(); break;
            case 6: sortBooks(); break;
            case 7: saveFile(); break;
            case 8: loadFile(); break;
            case 9: loginRegister(); break;
            case 0: puts("\nExiting program. Goodbye!"); break;
        }
        if(choice!=0) waitEnter();
    }while(choice!=0);

    return 0;
}
