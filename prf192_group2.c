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
    char  id[ID_MAX];
    char  title[STR_MAX];
    char  author[STR_MAX];
    int   year;
    int   priceVND;
} Book;

static Book books[MAX];
static int  nBooks = 0;

// ==============================
// TIỆN ÍCH CHUỖI & NHẬP LIỆU
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

static int icontains(const char *haystack, const char *needle){
    char a[STR_MAX*2], b[STR_MAX*2];
    toLowerStr(haystack, a, sizeof(a));
    toLowerStr(needle,   b, sizeof(b));
    return strstr(a,b)!=NULL;
}

static void spaces_to_underscores(const char *in, char *out, size_t outsz){
    size_t i=0;
    for(; in[i] && i+1<outsz; ++i) out[i] = (in[i]==' ') ? '_' : in[i];
    if(outsz) out[i]='\0';
}
static void underscores_to_spaces(const char *in, char *out, size_t outsz){
    size_t i=0;
    for(; in[i] && i+1<outsz; ++i) out[i] = (in[i]=='_') ? ' ' : in[i];
    if(outsz) out[i]='\0';
}

static void formatVND(int v, char *out, size_t outsz){
    char buf[64]; snprintf(buf, sizeof(buf), "%d", v);
    int len=(int)strlen(buf);
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
    int c; while ((c=getchar())!='\n' && c!=EOF) {}
}

static int readIntRange(const char *label, int minv, int maxv){
    char buf[128];
    while(1){
        printf("%s", label);
        fflush(stdout);
        if(!fgets(buf, sizeof(buf), stdin)){ clearerr(stdin); continue; }
        trim(buf);
        if(buf[0]=='\0'){ puts("Gia tri khong duoc rong!"); continue; }
        int ok=1;
        for(size_t i=0;i<strlen(buf);++i)
            if(!(i==0 && (buf[i]=='-'||buf[i]=='+')) && !isdigit((unsigned char)buf[i])){ ok=0; break; }
        if(!ok){ puts("Hay nhap so nguyen!"); continue; }
        long v = strtol(buf,NULL,10);
        if(v<minv || v>maxv){ printf("Chi chap nhan tu %d den %d\n", minv, maxv); continue; }
        return (int)v;
    }
}

static void readLine(const char *label, char *out, int outsz, int allowEmpty){
    while(1){
        printf("%s", label);
        fflush(stdout);
        if(!fgets(out, outsz, stdin)){ clearerr(stdin); continue; }
        trim(out);
        if(!allowEmpty && out[0]=='\0'){
            puts("Gia tri khong duoc rong!");
            continue;
        }
        return;
    }
}

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
// UI BOX '=' & '||' (canh tuyệt đối)
// ==============================
#define UI_WIDTH 116            // bề ngang cố định (mỗi dòng in đúng 116 ký tự)
#define VBAR     "||"           // viền dọc
#define MARGIN   1              // khoảng trắng trong viền: 1 trái + 1 phải

static void ui_hline(void){
    for(int i=0;i<UI_WIDTH;i++) putchar('=');
    putchar('\n');
}

// In 1 dòng hộp với text căn trái, luôn đủ UI_WIDTH ký tự
static void ui_text(const char *text){
    const int inner = UI_WIDTH - 4;           // bỏ 2*'||'
    const int content = inner - 2*MARGIN;     // bỏ margin trái/phải
    int len = (int)strlen(text);
    int off = 0;

    while(off < len){
        int take = (len - off < content) ? (len - off) : content;
        // in: || + ' ' + text(take) + pad + ' ' + ||
        printf("%s%*s%.*s%*s%s\n",
               VBAR,
               MARGIN, "",
               take, text + off,
               MARGIN + (content - take), "",  // pad còn thiếu + margin phải
               VBAR);
        off += take;
    }
    if(len==0){
        // dòng rỗng
        printf("%s%*s%*s%s\n", VBAR, MARGIN, "", content, "", VBAR);
    }
}

// In dòng căn giữa, luôn đủ UI_WIDTH
static void ui_center(const char *text){
    const int inner = UI_WIDTH - 4;
    const int content = inner - 2*MARGIN;
    int len = (int)strlen(text);
    if(len > content) len = content;
    int left  = (content - len) / 2;
    int right = content - len - left;

    printf("%s%*s%*s%.*s%*s%*s%s\n",
           VBAR,
           MARGIN, "",          // margin trái
           left, "",            // pad trái
           len, text,           // nội dung (cắt nếu dài)
           right, "",           // pad phải
           MARGIN, "",          // margin phải
           VBAR);
}

static void ui_blank(void){
    const int inner = UI_WIDTH - 4;
    const int content = inner - 2*MARGIN;
    printf("%s%*s%*s%*s%s\n", VBAR, MARGIN, "", content, "", MARGIN, "", VBAR);
}

static void ui_head(const char *title){
    ui_hline();
    ui_center(title);
    ui_hline();
}
static void ui_foot(void){
    ui_hline();
}

// ==============================
// DATA & BẢNG
// ==============================
static int findById(const char *id){
    for(int i=0;i<nBooks;i++){
        if(strcmp(books[i].id,id)==0) return i;
    }
    return -1;
}

// Kích thước cột bảng (tổng luôn <= content, ui_text sẽ pad cho đủ)
#define W_STT  4
#define W_ID   16
#define W_TIT  30
#define W_AUT  30
#define W_YEAR 6
#define W_PRC  14

static void table_header(void){
    ui_head("DANH SACH SACH");
    char buf[256];
    snprintf(buf, sizeof(buf), "%-*s | %-*s | %-*s | %-*s | %-*s | %-*s",
             W_STT, "STT", W_ID, "ID", W_TIT, "Title", W_AUT, "Author", W_YEAR, "Year", W_PRC, "Price (VND)");
    ui_text(buf);
    ui_hline();
}
static void table_row(int idx, const Book *b){
    char price[64]; formatVND(b->priceVND, price, sizeof(price));
    char tline[256];
    snprintf(tline, sizeof(tline), "%*d | %-*s | %-*s | %-*s | %*d | %*s",
             W_STT, idx+1,
             W_ID,  b->id,
             W_TIT, (b->title[0]?b->title:"(no_title)"),
             W_AUT, (b->author[0]?b->author:"(no_author)"),
             W_YEAR, b->year,
             W_PRC, price);
    ui_text(tline);
}
static void table_footer(void){
    ui_foot();
}

// ==============================
// CHỨC NĂNG CORE
// ==============================
static void addBook(void){
    if(nBooks>=MAX){ ui_head("THONG BAO"); ui_text("Kho sach day!"); ui_foot(); return; }
    Book b; memset(&b,0,sizeof(b));

    ui_head("THEM SACH MOI");
    readId(b.id);
    if(findById(b.id)!=-1){ ui_text("ID da ton tai!"); ui_foot(); return; }

    readLine("Title: ",  b.title,  STR_MAX, 0);
    readLine("Author (go '-' de bo qua): ", b.author, STR_MAX, 1);
    if(strcmp(b.author,"-")==0) b.author[0]='\0';
    b.year     = readIntRange("Year (>=0): ", 0, 3000);
    b.priceVND = readIntRange("Price (VND, >=0): ", 0, 2000000000);
    ui_foot();

    books[nBooks++] = b;
    ui_head("THONG BAO"); ui_text(">> Added!"); ui_foot();
}

static void delBook(void){
    if(nBooks==0){ ui_head("THONG BAO"); ui_text("Danh sach rong."); ui_foot(); return; }
    char id[ID_MAX];
    ui_head("XOA SACH");
    readId(id);
    int k = findById(id);
    if(k==-1){ ui_text("Khong tim thay."); ui_foot(); return; }

    char ans[8];
    char confirmMsg[96]; snprintf(confirmMsg, sizeof(confirmMsg), "Ban co chac chan muon xoa ID %s? (Y/N): ", id);
    ui_text(confirmMsg); printf("> ");
    if(!fgets(ans,sizeof(ans),stdin)){ clearerr(stdin); ui_foot(); return; }
    if(tolower((unsigned char)ans[0])!='y'){ ui_text(">> Huy xoa."); ui_foot(); return; }

    for(int i=k;i<nBooks-1;i++) books[i]=books[i+1];
    nBooks--;
    ui_text(">> Deleted!"); ui_foot();
}

static void editBook(void){
    if(nBooks==0){ ui_head("THONG BAO"); ui_text("Danh sach rong."); ui_foot(); return; }
    char id[ID_MAX];
    ui_head("SUA THONG TIN SACH");
    readId(id);
    int k = findById(id);
    if(k==-1){ ui_text("Khong tim thay."); ui_foot(); return; }

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

    int setYear  = readIntRange("New Year (>=0, nhap so hien tai de giu): ", 0, 3000);
    int setPrice = readIntRange("New Price (>=0, nhap so hien tai de giu): ", 0, 2000000000);
    bk->year = setYear; bk->priceVND = setPrice;

    ui_foot();
    ui_head("THONG BAO"); ui_text(">> Updated!"); ui_foot();
}

static void swapB(Book *a, Book *b){ Book t=*a; *a=*b; *b=t; }

static void sortBooks(void){
    if(nBooks<=1){ ui_head("THONG BAO"); ui_text("Khong can sap xep."); ui_foot(); return; }
    ui_head("SAP XEP");
    ui_text("Sort by: 1-Title  2-Author  3-Year  4-Price");
    int m   = readIntRange("Chon (1..4): ", 1, 4);
    int asc = readIntRange("Thu tu: 1-Asc  2-Desc: ", 1, 2);
    int isAsc = (asc==1);
    ui_foot();

    for(int i=0;i<nBooks-1;i++){
        for(int j=0;j<nBooks-1-i;j++){
            int greater=0;
            if(m==1)      greater = strcmp(books[j].title,  books[j+1].title)  > 0;
            else if(m==2) greater = strcmp(books[j].author, books[j+1].author) > 0;
            else if(m==3) greater = books[j].year     > books[j+1].year;
            else          greater = books[j].priceVND > books[j+1].priceVND;

            if((isAsc && greater) || (!isAsc && !greater)){
                swapB(&books[j], &books[j+1]);
            }
        }
    }
    ui_head("THONG BAO"); ui_text(">> Sorted!"); ui_foot();
}

static void table_print_all(void){
    table_header();
    for(int i=0;i<nBooks;i++) table_row(i, &books[i]);
    table_footer();
}

static void searchBook(void){
    if(nBooks==0){ ui_head("THONG BAO"); ui_text("Danh sach rong."); ui_foot(); return; }
    ui_head("TIM KIEM");
    ui_text("Tim theo: 1-ID  2-Title (substring, khong phan biet hoa/thuong)");
    int mode = readIntRange("Chon (1/2): ", 1, 2);
    ui_foot();

    if(mode==1){
        char id[ID_MAX];
        ui_head("TIM THEO ID");
        readId(id);
        int i=findById(id);
        if(i==-1){ ui_text("Not found."); ui_foot(); }
        else{
            table_header();
            table_row(i, &books[i]);
            table_footer();
        }
    }else{
        char key[STR_MAX];
        ui_head("TIM THEO TITLE");
        readLine("Nhap mot phan hoac toan bo Title: ", key, STR_MAX, 0);
        ui_foot();

        int cnt=0;
        table_header();
        for(int i=0;i<nBooks;i++){
            if(icontains(books[i].title, key)){
                table_row(i, &books[i]); cnt++;
            }
        }
        table_footer();
        ui_head("KET QUA");
        char msg[64]; snprintf(msg, sizeof(msg), (cnt==0)?"Not found.":" >> Found %d result(s).", cnt);
        ui_text(msg); ui_foot();
    }
}

static void showAll(void){
    if(nBooks==0){ ui_head("THONG BAO"); ui_text("Danh sach rong."); ui_foot(); return; }
    table_print_all();
    char msg[64]; snprintf(msg, sizeof(msg), "Tong so sach: %d", nBooks);
    ui_head("THONG KE"); ui_text(msg); ui_foot();
}

// ==============================
// LƯU/ĐỌC FILE (tương thích cũ)
// ==============================
static void saveFile(void){
    FILE *f = fopen("books.txt","w");
    if(!f){ ui_head("THONG BAO"); ui_text("Open fail"); ui_foot(); return; }
    for(int i=0;i<nBooks;i++){
        char t[STR_MAX], a[STR_MAX];
        spaces_to_underscores(books[i].title,  t, sizeof(t));
        spaces_to_underscores(books[i].author, a, sizeof(a));
        if(a[0]=='\0') strcpy(a,"-");
        fprintf(f,"%s %s %s %d %d\n",
            books[i].id, t, a, books[i].year, books[i].priceVND);
    }
    fclose(f);
    ui_head("THONG BAO"); ui_text(">> Saved to books.txt"); ui_foot();
}

static void loadFile(void){
    FILE *f = fopen("books.txt","r");
    if(!f){ ui_head("THONG BAO"); ui_text("No file"); ui_foot(); return; }
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
    ui_head("THONG BAO"); ui_text(">> Loaded!"); ui_foot();
}

// ==============================
// LOGIN / REGISTER
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
    ui_head("LOGIN / REGISTER");
    ui_text("1-Login  2-Register");
    int c = readIntRange("Chon (1/2): ", 1, 2);

    char u[64], p[64];
    readLine("User: ", u, sizeof(u), 0);
    readLine("Pass: ", p, sizeof(p), 0);

    if(c==1){
        ui_text(checkLogin(u,p) ? ">> Login OK" : ">> Wrong info");
    }else{
        if(userExists(u)){ ui_text(">> Username da ton tai"); ui_foot(); return; }
        FILE *f=fopen("users.txt","a"); if(!f){ ui_text("Open fail"); ui_foot(); return; }
        fprintf(f,"%s %s\n",u,p);
        fclose(f);
        ui_text(">> Registered");
    }
    ui_foot();
}

// ==============================
// MAIN MENU
// ==============================
static void ui_mainMenu(void){
    ui_head("BOOK MANAGEMENT SYSTEM - Team 2 (UI box ver.)");
    ui_blank();
    ui_center("MAIN MENU");
    ui_blank();
    ui_text("1. Add new book");
    ui_text("2. Delete a book");
    ui_text("3. Edit book information");
    ui_text("4. Search books (ID / Title substring)");
    ui_text("5. Show all books");
    ui_text("6. Sort books (Title/Author/Year/Price, Asc/Desc)");
    ui_text("7. Save books to file");
    ui_text("8. Load books from file");
    ui_text("9. User Login / Register");
    ui_text("0. Exit program");
    ui_blank();
    ui_text("Please enter your choice (0-9):");
    ui_foot();
}

int main(void){
    int choice;
    system(CLEAR);
    ui_head("WELCOME"); ui_center("Press Enter to continue..."); ui_foot();
    waitEnter();

    do{
        system(CLEAR);
        ui_mainMenu();
        choice = readIntRange("> ", 0, 9);
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
            case 0: ui_head("GOODBYE"); ui_text("Exiting program. Goodbye!"); ui_foot(); break;
        }
        if(choice!=0) waitEnter();
    }while(choice!=0);

    return 0;
}
