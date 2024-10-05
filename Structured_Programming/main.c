#include <stdio.h>
#define _WIN32_WINNT 0x0500 //Ορίζει ποια έκδοση του windows.h θα χρησιμοποιηθεί - Χρήση για το "κλείδωμα" του παραθύρου (βλ main)
#include <windows.h>
#include <string.h>
#include <conio.h>
#include <stdbool.h>
#include <time.h>
//Τα παρακάτω δύο headers είναι για την _setmode - την εμφάνιση χαρακτήρων unicode στην κονσόλα των windows
#include <io.h>
#include <fcntl.h>

//Ορισμός κωδικών χρωμάτων για ευκολότερη χρήση μέσα σε εντολές αλλαγής χρώματος της κονσόλας
#define COLOR_RED         4
#define COLOR_GREEN       2
#define COLOR_YELLOW      6
#define COLOR_BLUE        1
#define COLOR_MAGENTA     13
#define COLOR_CYAN        11
#define COLOR_LIGHTBLUE   3
#define COLOR_RESET       7

//Ορισμός κωδικών που λαμβάνει η getch όταν πατιούνται συγκεκριμένα πλήκτρα
#define LEFT_ARROW      75
#define RIGHT_ARROW     77
#define UP_ARROW        72
#define DOWN_ARROW      80
#define ESC             27
#define Q               113

//Ορισμός κωδικών για χρήση στην changeui - Αντιστοιχούν στο χ στο οποίο πρέπει να μεταβέι ο κέρσορας ώστε να επανεμφανίσει τα αντίστοιχα δεδομένα
#define WIND            10
#define ROUND           42  
#define PLAYER          72

//Ορισμοί κωδικών για χρήση στην lake - Τα τρία τελευταία ψηφία ορίζουν έναν κωδικό χαρακτήρα ascii ενώ τα υπόλοιπα το χρώμα
#define SEA             3177
#define LAND            14219
#define PLAYER1         2219
#define PLAYER2         4219
#define PLAYER3         6219
#define PLAYER4         5219
#define PLAYER1PATH     2178
#define PLAYER2PATH     4178
#define PLAYER3PATH     6178
#define PLAYER4PATH     5178
#define WRECKAGE        4158

//Ορισμός κωδικών ASCII για ευκολότερη χρήση
char d_top_right_corner    = 187;
char s_top_right_corner    = 191;
char d_top_left_corner     = 201;
char s_top_left_corner     = 218;
char d_bottom_right_corner = 188;
char s_bottom_right_corner = 217;
char d_bottom_left_corner  = 200;
char s_bottom_left_corner  = 192;
char d_horizontal_line     = 205;
char s_horizontal_line     = 196;
char d_vertical_line       = 186;
char s_vertical_line       = 179;
char crossed_circle        = 157;
char sea                   = 177;
char land                  = 176;
char movement              = 178;

//Ορισμός των ASCII ART ως δισδιάστατο πίνακα για να μπορέσει να εμφανιστεί χωρίς προβλήματα
char ship1[7][30]={    "    __|__ |___| |\\        ", 
                       "    |o__| |___| | \\       ", 
                       "    |___| |___| |o \\      ", 
                       "   _|___| |___| |__o\\     ", 
                       "  /...\\_____|___|____\\_/ ", 
                       "  \\   o * o * * o o  /    ", 
                       "~~~~~~~~~~~~~~~~~~~~~~~~~~~"},
     
    ship2[7][30]={     "          /__| )           ", 
                       "        /____| ))          ", 
                       "      /______| )))         ", 
                       "    /________|  )))        ", 
                       "            _|____)        ", 
                       "     \\======| o o /       ", 
                       "~~~~~~~~~~~~~~~~~~~~~~~~~~~"},

    ship3[7][30]={     "      |    |    |            ", 
                       "     )_)  )_)  )_)           ", 
                       "    )___))___))___)\\        ", 
                       "   )____)____)_____)\\\\     ", 
                       " _____|____|____|____\\\\\\__", 
                       "  \\                   /     ", 
                       "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"};
                  
//Δηλώσεις global μεταβλητών
int lake[25][64]; //Αποθηκεύει κάθε έναν χαρακτήρα που βρίσκεται στην λίμνη
int prevlake[25][64]; //Προσωρινή αποθήκευση της lake ώστε το πρόγραμμα να εντοπίζει πότε αλλάζει κάποιο στοιχείο της
int ships[4];//Αποθηκεύει στην αντίστοιχη θέση το πλοίο που επέλεξε ο κάθε παίκτης
int names[4][10];//Αποθηκεύει στην αντ΄΄ιστοιχη θέση το όνομα που επέλεξε ο κάθε παίκτης
int player_turn=1;//Περιέχει τον αριθμό που αντιστοιχεί στον παίκτη από τον οποίο αναμένει είσοδο το παιχνίδι
int cur_round = 0; //Μετράει σε ποιόν γύρο βρίσκεται το τρέχον παιχνίδι
int wind_dir = 1; //Η διεύθυνση του ανέμου | 1 Πάνω - 2 Δεξία - 3 Κάτω - 4 Δεξιά |
int wind_spd = 0; //Η ένταση του ανέμου
int playermovements[4][8]={0};//Αποθηκεύει τις κινήσεις που όρισαν οι παίκτες | 1 Πάνω - 2 Δεξία - 3 Κάτω - 4 Δεξιά |
const int playercolors[5]={0,2,4,6,5};// 2 Πράσινο, 4 Κόκκινο, 6 Κίτρινο, 5 Μωβ
bool alive[4]; //Αποθηκέυει την κατάσταση του κάθε πλοίου - true αν μπορεί να πλεύσει | false αν έχει βουλιάξει
bool donemoving[4]; //Αποθηκεύει αν οι παίκτες στις αντίστοιχες θέσεις έχουν τελειώσει τις κινήσεις τους
int lives[4];//Μετράει πόσες συγκρούσεις μπορεί να κάνει ακόμη ο παίκτης
int savedpos[4][2]={0};//Αποθηκεύει την θέση του πλοίου όταν χρειαστεί να α΄΄λλάξει χρώμα (και άρα δεν μπορεί να το βρει η findship)
int wreckship[4][2];//Αποθηκεύει τις τοποθεσίες κατεστραμένων πλοίων ώστε να τα διαγράψει μετά απο 3 γύρους (Αποθηκεύει τα x και y των ΔΕΞΙΩΝ στοιχειων)
int wreckcounter[4];//Αποθηκεύει τους υπολοιπόμενους γύρους μέχρι την διαγραφή των κατεστραμένων πλοίων
bool winners[4];//Αποθηκέυει τους παίκτες που νίκησαν
/*
Η επιλογή κάποιες απο τις παραπάνω μεταβλητές (π.χ. wreckship) να αποθηκεύουν το δεξι στοιχείο έγινε για να μην υπάρχουν προβλήματα με
πλοία-ναυάγια κλπ που βρίσκονται στην πάνω αριστερή γωνία. Εκέι το αριστερό στοιχείο εχει συντεταγμένες 0,0 οι οποίες
θα μπέρδευαν τους ελέγχους που λαμβάνουν το 0 ως "κενό στοιχείο"
*/

//Δηλώσεις Συναρτήσεων - Οι περιγραφές τους πάνω απο την κάθε μια πιο κάτω στον κώδικα
int main_menu();
int print_ship(char art[7][30]);
int nspaces(int num);
int wspaces(char *word);
void printmenu(int selection);
int printbutton_unselected(char *word,bool left_arrow,bool right_arrow);
int printbutton_selected(char *word, int color,bool left_arrow,bool right_arrow);
int sailing();
int printdiff(int selection);
int diff_select();
int players_select();
int printplayers(int selection);
int ship_select();
int printshipselect(int selection);
int name_select(bool empty);
void printnameselect(bool empty);
void set_lake_line(int line,int start,int end);
int printlake();
void lake_reset();
int findship(int player);
int play_turn();
void declaremove();
bool crashcheck(int player,int x, int y);
bool wincheck();
int MoveConsoleCursorToPosition (int x, int y);
int changelaketext(char *word, int color);
int winscreen();
int defeatscreen();
int changeui(int object);
int windchange(int difficulty);
int set_players(int players);
int setmovements(int cursor[]);
int unsetmovements(int cursor[], int backup[25][64]);
int printchange();
int moveto(int direction, int player);
void setfinishline();
int wreck(int target);
int windcheck(int player);
int moveplayers(int players);
int setpreview();
int resetsavedpos();

//Αρχή του προγράμματος
int main()
{
    srand(time(NULL));//Ορίζει το seed για την παραγωγή τυχαίων αριθμών
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_RESET); //Ορίζει το χρώμα των γραμμάτων και του background της κονσόλας σε ασπρο-μαύρο αντίστοιχα
    SetConsoleOutputCP(850);//Ορίζει την κωδικοσελίδα σε 850 ώστε να μην υπάρχει πρόβλημα σε υπολογιστές με άλλες κωδικοσελίδες - να εμφανίζονται ορισμένα σύμβολα σωστά
    system("MODE 85,37"); //Ορίζει το μέγεθος του παραθύρου σε 85 χαρακτήρες οριζόντια και 25 χαρακτήρες κάθετα
    printf("\e[?25l");//Κρύβει τον κέρσορα της κονσόλας

    //Οι παρακάτω δύο γραμμές "κλειδώνουν" το μέγεθος του παραθύρου ώστε να μην μπορεί να το αλλάξει ο χρήστης. Πηγή στο έγγραφο word
    HWND consoleWindow = GetConsoleWindow(); 
    SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);   

    return main_menu();
}

/*Εκτυπώνει το ASCII ART των πλοίων
Η παράμετρος αντιστοιχεί σε έναν απο τους πίνακες που ορίστηκαν πιο πανω*/
int print_ship(char art[7][30])
{
    for (int i=0;i<7;i++)
    {
        if (i==6)
        {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_LIGHTBLUE);    
        }
        nspaces(30);
        printf("%s\n",art[i]);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_RESET);
    }
}

/*Ελέγχει το σύστημα επιλογής των μενού

Δέχεται ως παραμετρους:
previous : τον κωδικό του πλήκτρου που μεταβαίνει στην προηγούμενη επιλογή
next : τον κωδικό του πλήκτρου που μεταβαίνει στην προηγούμενη επιλογή
min: την ελάχιστη επιλογή
max: την μέγιστη επιλογή
menuprint: συνάρτηση που εμφανίζει το κατάλληλο μενού
Επιστρέφει -1 αν πατηθεί το ESC ή τον αριθμό της τρέχουσας επιλογής αν πατηθηεί το Enter*/
int menu_manager(const int previous,const int next, int min, int max, void(*menuprint)(int))
{
    int input = 0; 
    int selection = min; 

    do
    {
    if (input == previous)
    {
        if (selection!=min)//Εαν δεν είμαστε στην πρώτη επιλογη - δηλαδή υπάρχει προηγούμενη επιλογή
            {
                 selection --;//Η τρέχουσα επιλογή πηγαίνει μια θέση πίσω
            }     
    }else if (input == next)
    {
        if (selection!=max)//Εαν δεν είμαστε στην τελευταία επιλογή - δηλαδή υπάρχει επόμενη επιλογή
            {
                 selection ++;//Η τρέχουσα επιλογή πηγαίνει μια θέση μπροστά
            }  
    }else if (input == ESC)
    {
        return -1;
    }

    menuprint(selection);//Εμφανίζει το μενού στην κονσόλα με την κατάληλη συνάρτηση που ορίζει ο χρήστης ως menuprint

    fflush(stdin);
    input = getch();//Το πλήκτρο που θα πατήσει ο χρήστης αποθηκεύεται ως int στην input
    } while (input != 13); //Όσο δεν πατάμε το enter 
    return selection;
}

/*Αφήνει κενά ίσα με αριθμό που ορίζουμε

num: Ο αριθμός των κενών που θέλουμε να αφήσουμε*/
int nspaces(int num)
{
    for (int i=0;i<num;i++)
    {
        printf(" ");
    }
}

/*Αφήνει κενά ώστε να κεντράρει την λέξη που βάλαμε ως παράμετρο

word: Σταθερή συμβολοσειρά την οποία θέλουμε να κεντράρουμε*/
int wspaces(char *word)
{
    int wordlength = strlen(word);
    int num = (85 - wordlength)/2;

    nspaces(num);
}

/*Εμφανίζει ένα κουμπί με άσπρο περίγραμμα μιας γραμμής

Δέχεται ως παραμέτρους:
word: Σταθερή συμβολοσειρά που περιέχει το κείμενο του κουμπιού
left_arrow,right_arrow: μεταβλητές bool που περιέχουν πληροφορία για το εάν η συνάρτηση πρέπει να εμφανίσει βελάκια δίπλα από τα κουμπιά*/
int printbutton_unselected(char *word,bool left_arrow,bool right_arrow)
{
    int wordlength = strlen(word);

//Εμφανίζει την πρώτη γραμμή του κουμπιού
    nspaces((85 - wordlength)/2 );
    printf("%c",s_top_left_corner);
    for(int i=0;i<wordlength;i++)
    {
        printf("%c",s_horizontal_line);       
    }
    printf("%c\n",s_top_right_corner);

    //Εμφανίζει την δεύτερη γραμμή του κουμπιού
    nspaces((85 - wordlength)/2 -1);//1 λιγότερο σε περίπτωση που υπάρχει το ΄βέλος
    
    //Εμφανίζει αριστερό βέλος
    if (left_arrow == true)   
    {
        _setmode(_fileno(stdout), _O_U16TEXT);//Ορίζει την μορφή output της κονσόλας σε Unicode ώστε να εμφανίσει τέτοιους χαρακτήρες
        wprintf(L"←");
        _setmode(_fileno(stdout), _O_TEXT);//Επαναφέρει την μορφή output σε Ascii Text      
    }else nspaces(1);
    //Εμφανίζει το κυρίως κουμπί
    printf("%c",s_vertical_line);
    for (int i=0;i<wordlength;i++)
    {
        printf("%c",word[i]);
    }
    printf("%c",s_vertical_line);
    //Εμφανίζει δεξί βέλος
    if (right_arrow == true)
    {
        _setmode(_fileno(stdout), _O_U16TEXT);
        wprintf(L"→");
        _setmode(_fileno(stdout), _O_TEXT);      
    }
    printf("\n");

    //Εμφανίζει την τρίτη γραμμή του κουμπιού
    nspaces((85 - wordlength)/2 );
    printf("%c",s_bottom_left_corner);
    for(int i=0;i<wordlength;i++)
    {
        printf("%c",s_horizontal_line);       
    }
    printf("%c\n",s_bottom_right_corner);
}

/*Εμφανίζει ένα κουμπί με έγχρωμο περίγραμμα διπλής γραμμής

Δέχεται ως παραμέτρους:
word: Σταθερή συμβολοσειρά που περιέχει το κείμενο του κουμπιού
color: Κωδικός του χρώματος για το περίγραμμα του κουμπιού
left_arrow,right_arrow: μεταβλητές bool που περιέχουν πληροφορία για το εάν η συνάρτηση πρέπει να εμφανίσει βελάκια δίπλα από τα κουμπιά*/
int printbutton_selected(char *word, int color,bool left_arrow,bool right_arrow)
{
    int wordlength = strlen(word);

    //Εμφανίζει την πρώτη γραμμή του κουμπιού
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);//Αλλάζει το χρώμα
    nspaces((85 - wordlength)/2 );
    printf("%c",d_top_left_corner);
    for(int i=0;i<wordlength;i++)
    {
        printf("%c",d_horizontal_line);       
    }
    printf("%c\n",d_top_right_corner);

    //Εμφανίζει την δεύτερη γραμμή του κουμπιού
    nspaces((85 - wordlength)/2 -1);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_RESET);
    //Εμφανίζει αριστερό βέλος
    if (left_arrow == true)
    {
        _setmode(_fileno(stdout), _O_U16TEXT);
        wprintf(L"←");
        _setmode(_fileno(stdout), _O_TEXT);      
    }else nspaces(1);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);

    printf("%c",d_vertical_line);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_RESET);
    for (int i=0;i<wordlength;i++)
    {
        printf("%c",word[i]);
    }
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
    printf("%c",d_vertical_line);

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_RESET);
    //Εμφανίζει δεξί βέλος
    if (right_arrow == true)
    {
        _setmode(_fileno(stdout), _O_U16TEXT);//Εξηγούνται πιο πάνω
        wprintf(L"→");
        _setmode(_fileno(stdout), _O_TEXT);      
    }
    printf("\n");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);

    //Εμφανίζει την τρίτη γραμμή του κουμπιού
    nspaces((85 - wordlength)/2 );
    printf("%c",d_bottom_left_corner);
    for(int i=0;i<wordlength;i++)
    {
        printf("%c",d_horizontal_line);       
    }
    printf("%c\n",d_bottom_right_corner);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_RESET);
}

/*Ελέγχει το αρχικό μενού χρησιμοποιώντας την συνάρτηση menu_manager

Αν ο χρήστης επιλέξει play καλεί την συνάρτηση sailing() [η sailing τρέχει το παιχνίδι και επιστρέφει 0 αν όλοι οι παίκτες χάσουν
ή 1 αν κάποιος νικήσει]
Αν ο χρήστης επιλέξει exit κλείνει το παιχνίδι*/
int main_menu()
{
    int selection = menu_manager(UP_ARROW,DOWN_ARROW,0,1,printmenu);
    if (selection == 0)//Αν η επιλογή ήταν το 0 (δηλαδή το κουμπι play)
    {
        if(sailing()==0) return defeatscreen(); //Τρέχει την συνάρτηση sailing() η οποία περιέχει όλο το παιχνίδι. Εαν επιστρέψει 0 σημαίνει πως όλοι οι παίκτες έχασαν.
        else return winscreen(); //Αν δεν έχει επιστρέψει 0, θα έχει επιστρέψει 1 που σημαίνει πως κάποιος νίκησε.
        
    }else if (selection == 1)//Αν η επιλογή ήταν 1 (δηλαδή το κουμπι exit)
    {
        exit(0);//Τερματίζεται η εφαρμογή
    }else if (selection == -1) return main_menu(); //Αν πατηθεί το esc μας επιστρέφει στην αρχή της συνάρτησης αφού δεν υπάρχει προηγόυμενο βήμα
}

/*Εμφανίζει το αρχικό μενού

Ως παράμετρο δέχεται την τρέχουσα επιλογή του χρήστη ώστε να εμφανίζει το αντίστοιχο κουμπί ως επιλεγμένο*/
void printmenu(int selection)
{
    system("cls");
    printf("\n\n\n\n\n");
    wspaces("Sailing Game");//Περισσότερα στην wspaces
    printf("Sailing Game\n\n\n");
    
    //Εμφανίζει το ASCII ART του μενου
    print_ship(ship1);
    printf("\n");

    if (selection == 0) //Επιλεγμένο το Play
    {
        printf("\n\n");
        printbutton_selected("Play",COLOR_GREEN,false,false);
        printf("\n");
        printbutton_unselected("Exit",false,false);
    }else //Επιλεγμένο το Exit
    {
        printf("\n\n");
        printbutton_unselected("Play",false,false);
        printf("\n");
        printbutton_selected("Exit",COLOR_RED,false,false);
    }
}

/*Ελέγχει το μενού επιλογής πλοίων

Δέχεται ως παράμετρο τον αριθμό των παικτών που συμμετέχουν στο παιχνίδι
Ο κάθε παίκτης επιλέγει το πλοίο του και ορίζει ένα όνομα με την βοήθεια της name_select
Αν όλα πανε καλά επιστρέφει 0
Αν ο χρήστης πατήσει ESC επιστρέφει κάνοντας κλήση στην sailing
Πλοίο 0: Αντίσταση στον άνεμο
Πλοίο 1: 2 επιπλέον κινήσεις
Πλοίο 2: 1 επιπλέον ζωή
Οι ιδιότητες του κάθε πλοίου περιγράφονται αναλυτικά στο έγγραφο word*/
int ship_select(int players)
{
    for (int i=0;i<players;i++)
    {
        //Επιλογή πλοίου
        ships[i]=menu_manager(LEFT_ARROW,RIGHT_ARROW,0,2,printshipselect);
        if ( ships[i]==-1) return sailing();
        
        //Επιλογή ονόματος
        bool empty = false; //Μεταβλητή που ελέγχει αν ο χρήστης πάτησε enter με κενό όνομα
        name_select(empty);

        //Επόμενος παίκτης
        player_turn++;
    }
    return 0;
}

/*Ελέγχει την επιλογή ονόματος

Δέχεται ως παράμετρο μια μεταβλητή empty τύπου bool που περιγράφει αν ο χρήστης έβαλε κενό όνομα ώστε να εμφανίσει το ανάλογο
warning message. Προφανώς η empty έχει νόημα μόνο αν η name_select δεν τρέχει για πρώτη φορά
Επιστρέφει κλήση στην sailing αν πατηθεί ESC.
Αν ο χρήστης πατήσει enter χωρίς να βάλει όνομα επιστρέφει κλήση στον εαυτό την με την bool empty να είναι true
Αν ο χρήστης εισάγει επιτυχώς ένα όνομα επιστρέφει 0*/
int name_select(bool empty)
{
    
    int input = 0;
    int position = 0;//Η θέση του χαρακτήρα στην λέξη

    do
    {
    if ((input>=65 && input<=90) || (input>=97 && input<=122))//Αν ο χρήστης πάτησε κεφαλαίο ή πεζό γράμμα
    {
        if (position!=10)//Αν επιτρέπεται η είσοδος άλλου χαρακτήρα (Δεν είναι συμπληρωμένες και οι 10 θέσεις)
        {
            names[player_turn-1][position]=input;
            position++;
        }
    }else if (input == 8)//Αν ο χρήστης πάτησε backspace
    {
        if (position!=0)//Αν υπάρχει χαρακτήρας για να σβηστεί
        {
            position--;
            names[player_turn-1][position]=0;
        }
    }else if (input == ESC)
    {
        return sailing();
    }
    
    printnameselect(empty);
    fflush(stdin);
    input = getch();
    } while (input != 13);
    if (position==0) //Αν το πρόγραμμα δεν έχει δεκτεί κανέναν χαρακτήρα, δηλαδή δεν έχει εισαχθεί όνομα
    {
        empty = true;
        return name_select(empty);
    }else 
    {
        empty = false;  
        return 0;
    }
    
}

/*Εμφανίζει την οθόνη επιλογής ονόματος

Δέχεται ως παράμετρο την bool empty, η χρήση της οποίας περιγράφεται παραπάνω στην name_select*/
void printnameselect(bool empty)
{
    system("cls");
    printf("\n\n\n\n\n\n\n");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), playercolors[player_turn]);
    wspaces("PLAYER 1");
    printf("PLAYER %d\n",player_turn);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_RESET);
    wspaces("Enter a name for your ship (10 characters max)");
    printf("Enter a name for your ship (10 characters max)\n");
    
    //Γραμμή 1 του κουτιού εισαγωγής ονόματος
    nspaces(36);
    printf("%c",s_top_left_corner);
    for (int i=0;i<10;i++) printf("%c",s_horizontal_line);
    printf("%c\n",s_top_right_corner);
    //Γραμμή 2
    nspaces(36);
    printf("%c",s_vertical_line);
    for(int i=0;i<10;i++)
    {
        if (names[player_turn-1][i] == 0)
        {
            printf(" ");
        }else printf("%c",names[player_turn-1][i]);
    }
    printf("%c\n",s_vertical_line);
    //Γραμμή 3
    nspaces(36);
    printf("%c",s_bottom_left_corner);
    for (int i=0;i<10;i++) printf("%c",s_horizontal_line);
    printf("%c\n",s_bottom_right_corner);

    if (empty == true)
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_RED);
        printf("\n\n");
        wspaces("You must enter at least 1 character!");
        printf("You must enter at least 1 character!");
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_RESET);
    }
}

/*Η κύρια συνάρτηση του παιχνιδιού

Ορίζει όλες τις μεταβλητές παιχνιδιού στις αρχικές τους τιμές ώστε να είναι έτοιμες για το παιχνίδι
Καλεί τις κατάλληλες συναρτήσεις ώστε να ο χρήστης να επιλέξει δυσκολία και αριθμό παικτών
Καλεί τις κατάλληλες συναρτήσεις ώστε οι παίκτες να επιλέξουν πλοία και ονόματα
Καλεί την συνάρτηση play_turn μέχρι είτε να χάσουν όλοι οι παίκτες είτε να νικήσει έστω και ένας
Ενστωματώνει και τον έλεγχο της συνθήκης ήττας - του αριθμού παικτών που έχουν χάσει
Επιστρέφει 0 αν χάσουν όλοι ή 1 αν νικήσει έστω και ενας*/
int sailing()
{   
    int deadnum=0; //Μεταβλητή που αποθηκεύει πόσοι παίκτες έχουν πεθάνει. Χρησιμοποιείται για να ξέρουμε πότε έχουν πεθάνει όλοι

    for(int i=0;i<4;i++) alive[i]=true; 
    for(int i=0;i<4;i++) lives[i]=1; 
    for(int i=0;i<4;i++) {wreckship[i][0]=0; wreckship[i][1]=0;}
    for(int i=0;i<4;i++) wreckcounter[i]=0;
    
    player_turn = 1;

    //Παρακάτω εμφανίζονται δυο μενού ίδιας λογικής με το main_menu
    int difficulty = menu_manager(UP_ARROW,DOWN_ARROW,0,2,printdiff); //Αποθηκεύεται το επίπεδο δυσκολίας στην defficulty
    if (difficulty == -1) return main_menu(); //Αν στο μενού πατήθηκε το esc μας επιστρέφει στο αρχικό μενού
    int players = menu_manager(DOWN_ARROW,UP_ARROW,1,4,printplayers); //Αποθηκεύεται ο αριθμός των παικτών στην players
    if (players == -1) return sailing(); //Αν στο μενού πατήθηκε το esc μας επιστρέφει στην αρχή αυτής της συνάρτησης και άρα στο μενού επιλογής δυσκολίας
    
    ship_select(players); 

    lake_reset();
    set_players(players); 
    printlake();

    for(int i=0;i<4;i++) if(ships[i]==2) lives[i]++;//Όσοι παίκτες έχουν το πλοίο με αριθμό 2 έχουν μια έξτρα ζωή
    
    while(deadnum!=players && play_turn(players,difficulty)!=1)//Όσο οι νεκροί παίκτες δεν είναι ίσοι με τους συνολικούς (δηλαδή τουλ. ενας ζει) και όσο η play_turn δεν επιστρέφει 1 (δηλαδή δεν έχει νικήσει κανείς)
    {
        deadnum = 0; 
        cur_round++; //Σε κάθε επανάληψη αυξάνουμε τον γύρο στον οποίο βρισκόμαστε
        for (int i=0;i<players;i++) if(alive[i]==false) deadnum++; //Ελέγχει κάθε θέση της alive για παίκτες που δεν ζουν. Για κάθε έναν αυξάνει το deadnum            
        fflush(stdin);
    }
    if (deadnum==players) return 0;//Αν έχουν πεθάνει όλοι επιστρέφει 0
    else return 1;//Αλλιώς έχουν νικήσει όλοι (αφου έχει τελειώσει το loop) και άρα επιστρέφει 1
}

/*Εμφανίζει το μενού επιλογής δυσκολίας
Ως παράμετρο δέχεται την τρέχουσα επιλογή του χρήστη ώστε να εμφανίζει το αντίστοιχο κουμπί ως επιλεγμένο*/
int printdiff(int selection)
{
    system("cls");
    printf("\n\n\n\n\n\n");
    wspaces("Select a difficulty:");
    printf("Select a difficulty:\n\n\n");
    printf("\n");

    if (selection == 0) //Χαμηλή Δυσκολία
    {
        printf("\n");
        printbutton_selected("Easy",COLOR_GREEN,false,false);
        printf("\n");
        printbutton_unselected("Normal",false,false);
        printf("\n");
        printbutton_unselected("Hard",false,false); 
        printf("\n\n");
        wspaces("Max wind speed010B        Max wind change0000");
        printf("Max wind speed%c10B        Max wind change%c90%c",179,179,248);      
    }else if (selection == 1) //Μέτρια δυσκολία
    {
        printf("\n");
        printbutton_unselected("Easy",false,false);
        printf("\n");
        printbutton_selected("Normal",COLOR_YELLOW,false,false); 
        printf("\n"); 
        printbutton_unselected("Hard",false,false);
        printf("\n\n");
        wspaces("Max wind speed010B        Max wind change0000");
        printf("Max wind speed%c15B        Max wind change%c90%c",179,179,248);  
    }else //Υψηλή δυσκολία
    {
        printf("\n");    
        printbutton_unselected("Easy",false,false);
        printf("\n");
        printbutton_unselected("Normal",false,false);  
        printf("\n");
        printbutton_selected("Hard",COLOR_RED,false,false);
        printf("\n\n"); 
        wspaces("Max wind speed010B        Max wind change0000");
        printf("Max wind speed%c20B        Max wind change%c180%c",179,179,248);  
    }
    
}

/*Εμφανίζει το μενού επιλογής αριθμού παικτών
Ως παράμετρο δέχεται την τρέχουσα επιλογή του χρήστη*/
int printplayers(int selection)
{
    system("cls");
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n");
    wspaces("Select a number of players:");
    printf("Select a number of players:\n\n\n");
   
    if (selection != 4)//Παραλείπετε η εμφάνιση του πάνω βέλους αν ο αριθμός των παικτών δεν μπορεί να αυξηθέι περεταίρω
    {
        wspaces(".");
        _setmode(_fileno(stdout), _O_U16TEXT);//Ορίζει την λειτουργία εξόδου της κονσόλας σε Unicode ώστε να εκτυπωθεί το ↑
        wprintf(L"↑");//Εμφανίζει το ↑ με χρήση wide print
        _setmode(_fileno(stdout), _O_TEXT);//Ορίζει την λειτουργία εξόδου της κονσόλας σε κείμενο ASCII ώστε να λειτουργεί η printf
          
    }
    
    printf("\n");
    wspaces("AAAAA");
    printf("%c%c%c%c%c\n",d_top_left_corner,d_horizontal_line,d_horizontal_line,d_horizontal_line,d_top_right_corner);
    wspaces("AAAAA");
    printf("%c %d %c\n",d_vertical_line,selection,d_vertical_line);
    wspaces("AAAAA");
    printf("%c%c%c%c%c\n",d_bottom_left_corner,d_horizontal_line,d_horizontal_line,d_horizontal_line,d_bottom_right_corner);
    
    if (selection != 1)//Παραλείπετε η εμφάνιση του κατω βέλους αν ο αριθμός των παικτών δεν μπορεί να μειωθεί περεταίρω
    {
        wspaces(".");
        _setmode(_fileno(stdout), _O_U16TEXT);//Εξηγούνται πιο πάνω
        wprintf(L"↓");
        _setmode(_fileno(stdout), _O_TEXT);       
    }
}

/*Εμφανίζει το μενού επιλογής πλοίου
Ως παράμετρο δέχεται την τρέχουσα επιλογή του χρήστη*/
int printshipselect(int selection)
{
    
    system("cls");
    printf("\n\n");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), playercolors[player_turn]);
    wspaces("PLAYER 1");
    printf("PLAYER %d\n",player_turn);
     SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_RESET);
    wspaces("Select a ship:");
    printf("Select a ship:\n\n\n"); 

    if (selection == 0)
    {
        //Eμφανίζει το πλοίο και το όνομά του
        printbutton_selected("Ship 1", 14,false,true);  
        printf("\n");
        print_ship(ship1);
        printf("\n");

        //Εμφανίζει τα χαρακτηριστικά του πλοίου
        nspaces(33);
        printf("Movement       %c%c \n",s_vertical_line,crossed_circle);
        nspaces(33);
        printf("Wind Resistance%c%c%c\n",s_vertical_line,crossed_circle,crossed_circle);
        nspaces(33);
        printf("Armor          %c%c \n",s_vertical_line,crossed_circle); 
        
        //Εμφανίζει την περιγραφή του πλοίου
        printf("\n");
        wspaces("DESCRIPTION");
        printf("DESCRIPTION\n");
        wspaces("Immunne to winds of 1-5B. 50%% less chance to be affected by winds of 5-15 B.");
        printf("Immunne to winds of 1-5B. 50%% less chance to be affected by winds of 5-15 B.");  
        
    }else if (selection == 1)
    {
        printbutton_selected("Ship 2", 14,true,true);  
        printf("\n");
        print_ship(ship2);
        printf("\n");

        nspaces(33);
        printf("Movement       %c%c%c\n",s_vertical_line,crossed_circle,crossed_circle);
        nspaces(33);
        printf("Wind Resistance%c%c \n",s_vertical_line,crossed_circle);
        nspaces(33);
        printf("Armor          %c%c \n",s_vertical_line,crossed_circle);   

        printf("\n");
        wspaces("DESCRIPTION");
        printf("DESCRIPTION\n");
        wspaces("Can move 2 extra spaces per round.");
        printf("Can move 2 extra spaces per round.");
    }else if (selection == 2)
    {
        printbutton_selected("Ship 3", 14,true,false);  
        printf("\n");
        print_ship(ship3);
        printf("\n");

        nspaces(33);
        printf("Movement       %c%c \n",s_vertical_line,crossed_circle);
        nspaces(33);
        printf("Wind Resistance%c%c \n",s_vertical_line,crossed_circle);
        nspaces(33);
        printf("Armor          %c%c%c\n",s_vertical_line,crossed_circle,crossed_circle);   

        printf("\n");
        wspaces("DESCRIPTION");
        printf("DESCRIPTION\n");
        wspaces("Can hit once into another boat or terrain without being destroyed.");
        printf("Can hit once into another boat or terrain without being destroyed."); 
    } 
}

/*Ορίζει τους κατάλληλους χαρακτήρες μιας γραμμής της lake σε SEA και LAND

Δέχεται ως παραμέτρους:
line: ο αριθμός της γραμμής την οποία θέλουμε να ορίσουμε (Η μέτρηση αρχ΄΄ιζει από το 0)
start: η θέση x στην οποία ξεκινά η στεριά
end: η θέση x στην οποία τελειώνει η στεριά*/
void set_lake_line(int line,int start,int end)
{
    for (int i=0;i<64;i++) lake[line][i]=0;
    for (int i=start;i<end;i++) lake[line][i]=LAND;
    for (int i=0;i<64;i++) if(lake[line][i]==0) lake[line][i]=SEA;
}

//Ορίζει την γραμμή τερματισμού στην lake
void setfinishline()
{
    for(int i=0;i<12;i++)
    {
        if(i%2==0)
        {
            lake[6][i]=11177;
        }else lake[6][i]=11177;
    } 
}

//Ορίζει όλη την lake στην μορφή που θέλουμε με την βοήθεια της set_lake_line
void lake_reset()
{
    set_lake_line(0,0,0);
    set_lake_line(1,0,0);
    
    set_lake_line(2,17,40);
    set_lake_line(3,15,46);
    set_lake_line(4,14,47);
    set_lake_line(5,13,48);
    set_lake_line(6,12,55);
    set_lake_line(7,5,58);
    set_lake_line(8,1,59);
    set_lake_line(9,8,59);
    set_lake_line(10,15,60);
    set_lake_line(11,16,60);
    set_lake_line(12,17,60);
    set_lake_line(13,17,57);
    set_lake_line(14,18,50);
    set_lake_line(15,18,49);
    set_lake_line(16,19,39);
    set_lake_line(17,24,37);
   
    set_lake_line(18,0,0);
    set_lake_line(19,0,0);
    set_lake_line(20,0,0);
    set_lake_line(21,0,0);
    set_lake_line(22,0,0);
    set_lake_line(23,0,0);
    set_lake_line(24,0,0);

    setfinishline();
}

/*Τοποθετεί τα πλοία των παικτών σε προκαθορισμένη θέση στην lake- με 2 κενά ανάμεσά τους
Δέχεται ως παράμετρο των αριθμό των παικτών*/
int set_players(int players)
{
    int cursor = 0;
    for (int i=0;i<players;i++)
    {
        lake[11][cursor]=playercolors[i+1]*1000+219;
        lake[11][cursor+1]=playercolors[i+1]*1000+219;
        cursor += 4;
    }
}

/*Εμφανίζει την οθόνη του παιχνιδιού */
int printlake()
{
    system("cls");
    printf("\n\n");
    wspaces("WIND                           ROUND                         PLAYER");
    printf("WIND                           ROUND                         PLAYER\n");
    wspaces("WIND                           ROUND                         PLAYER");
    for (int i=0;i<2;i++)
    {
        printf("%c%c%c%c%c                          ",d_top_left_corner,d_horizontal_line,d_horizontal_line,d_horizontal_line,d_top_right_corner);
    } 
    
    printf("%c%c%c%c%c\n",d_top_left_corner,d_horizontal_line,d_horizontal_line,d_horizontal_line,d_top_right_corner);

    wspaces("WIND                           ROUND                         PLAYER");
    printf("%c",d_vertical_line);
    _setmode(_fileno(stdout), _O_U16TEXT);
    switch(wind_dir)
    {
        case 1:
            wprintf(L" ↑ ");
            break;
        case 2:
            wprintf(L" → ");
            break;
        case 3:
            wprintf(L" ↓ ");
            break;
        case 4:
            wprintf(L" ← ");
            break;
        default:

            break;
    }
     
    _setmode(_fileno(stdout), _O_TEXT);
    printf("%c",d_vertical_line);
    printf("                          %c %d %c                          %c",d_vertical_line,cur_round,d_vertical_line,d_vertical_line);
   SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), playercolors[player_turn]);
    printf(" %d ", player_turn);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_RESET); 
    printf("%c\n",d_vertical_line);

    wspaces("WIND                           TURN                          PLAYER");
    for (int i=0;i<2;i++)
    {
        printf("%c%c%c%c%c                          ",d_bottom_left_corner,d_horizontal_line,d_horizontal_line,d_horizontal_line,d_bottom_right_corner);
    } 
    
    printf("%c%c%c%c%c\n",d_bottom_left_corner,d_horizontal_line,d_horizontal_line,d_horizontal_line,d_bottom_right_corner);
    
    wspaces("WIND                           TURN                          PLAYER");
    if (wind_spd <10)
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_GREEN);
    }else if(wind_spd <15)
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_YELLOW);
    }else if(wind_spd <=20)
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_RED);
    }
    printf(" %cB\n",wind_spd);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_RESET);
    
    nspaces(10);
    printf("%c",d_top_left_corner);
    for(int i=0;i<64;i++) printf("%c",d_horizontal_line);
    printf("%c\n",d_top_right_corner);
    
    for(int i=0;i<25;i++)
    {
        nspaces(10);
        printf("%c",d_vertical_line);
        for (int x=0;x<64;x++)
        {
            
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), lake[i][x]/1000);
            printf("%c",lake[i][x]%1000);
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_RESET);

        }
        printf("%c",d_vertical_line);
        printf("\n");
    }

    nspaces(10);
    printf("%c",d_bottom_left_corner);
    for(int i=0;i<64;i++) printf("%c",d_horizontal_line);
    printf("%c\n",d_bottom_right_corner);
}

/*Παίζει έναν γύρο παιχνιδιού - Ολόκληρο το παιχνίδι αποτελείται απο επαναλήψεις αυτής της συνάρτησης
Δέχεται ως παραμέτρους:
players: o αριθμός των παικτών
difficulty: η δυσκολία του παιχνιδιού
Επιστρέφει 1 αν κάποιος πάικτης νικήσει - αλλιώς επιστρέφει 0*/
int play_turn(int players,int difficulty)
{ 
    player_turn = 1;
    changeui(ROUND);
    windchange(difficulty);
    changeui(WIND);
    for(player_turn;player_turn<players+1;player_turn++)
    {
        if(alive[player_turn-1]!=0)//Αν ο παίκτης που έχει σειρά ζει
        {
            changelaketext("Awaiting player input", playercolors[player_turn]);
            declaremove();
        }
    }
    for (int i=0;i<4;i++) donemoving[i]=false;//Επαναφέρεται η μεταβλητή που ελέγχει αν ο κάθε παίκτης έχει σταματήσει να κινείται
    if(moveplayers(players)==1) return 1;//Κινεί του παίκτες- Αν επιστρέψει 1 σημαίνει πως κάποιος νίκησε, άρα η συνάρτηση τερματίζεται και επιστρέφει 1
    
    for (int i=0;i<4;i++) 
    {
        if (wreckcounter[i]!=0) wreckcounter[i]--; //Κάθε γύρο μειώνονται κατα 1 τα μη μηδενικά στοιχεία (αντίστροφη μέτρηση)
        if (wreckcounter[i]==0 && (wreckship[i][0]!=0 || wreckship[i][0]!=0)) //Αν το στοιχείο του χρονόμετρου είναι 0 και υπάρχει κατερστραμένο πλοίο που αντιστοιχεί σε αυτό το χρονόμετρο
        {
            setpreview();
            //Αντικαθιστά τα σημεία-ναυάγια με θάλασσα
            lake[wreckship[i][0]][wreckship[i][1]]=SEA;
            lake[wreckship[i][0]][wreckship[i][1]-1]=SEA;
            printchange();
            wreckship[i][0]=0;//Οι αποθηκευμένες συντεταγμένες του ναυαγίου διαγράφονται
            wreckship[i][1]=0;
        }
    } 
    return 0;   
}

/*Επιτρέπει στον χρήστη να ορίσει την κίνησή του εμφανίζοντας και μια προεπισκόπισή της
Ο παίκτης δεν μπορεί να ορίσει κίνηση πάνω σε ξηρά ή σε ναυάγια*/
void declaremove()
{
    changeui(PLAYER);
    int input=0;
    int backup[25][64];//Αποθηκεύει αντίγραφο την λίμνης όπως ήταν πριν ο χρήστης ορίσει την κίνησή του
    int moves=0; //Οι κινήσεις που έχει χρησιμοποιήσει ο παίκτης
    int maxmoves = 6; 
    int cursor[2] = {findship(player_turn)/100,findship(player_turn)%100}; // Ορίζει την θέση που βρίσκεται το ΑΡΙΣΤΕΡΟ κουτάκι του πλοίου του παίκτη (βλ findship)

    if (ships[player_turn-1] == 1) maxmoves+=2; //Αν ο παίκτης έχει επιλέξει το πλοίο 1, έχει 2 επιπλέον κινήσεις


    for(int i=0;i<25;i++) for(int x=0;x<64;x++) backup[i][x]=lake[i][x];
    
    do
    {   
    setpreview();

    switch(input)
    {
        case UP_ARROW:
            if (cursor[0]!=0)//Αν το πλοίο δεν βρίσκεται στην πρώτη γραμμή-δηλαδή μπορεί να πάει πιο πάνω
            {
                if (moves!=0)//Αν δεν είναι η πρώτη κίνηση
                {
                    if (playermovements[player_turn-1][moves-1]==3)//Αν η προηγούμενη κίνηση που δηλώθηκε ήταν προς τα κάτω, σημαίνει πως ο χρήστης αναιρεί την κίνηση που όρισε πριν
                    {
                        moves--;//Αφου αναιρεί μια κίνηση μειώνεται κατα 1 το moves
                        playermovements[player_turn-1][moves]=0;//Η προηγούμενη κινησή του είναι ΄΄ιση με 0 - διαγράφεται
                        unsetmovements(cursor,backup);
                        cursor[0]--;//Ο κέρσορας μετακινείται ένα κουτάκι πάνω
                        break;
                    }
                }
                //Αν δεν ίσχυε ο προηγούμενος έλεγχος - δεν ήθελε να αναιρέσει κάποια κίνησή του
                if (moves<maxmoves)
                {
                    /*
                    Ο παρακάτω έλεγχος τσεκάρει αν τα στοιχεία που βρίσκονται πάνω απο το πλοίο (ακριβώς πανω και πάνω δεξιά απο τον κέρσορα) 
                    είναι ξηρά ή ναυάγιο και αν είναι δεν επιτρέπει τον ορισμό κίνησης προς εκείνη την κατεύθηνση
                    */
                    if(lake[cursor[0]-1][cursor[1]] != LAND && lake[cursor[0]-1][cursor[1]+1] != LAND && lake[cursor[0]-1][cursor[1]] != WRECKAGE && lake[cursor[0]-1][cursor[1]+1] != WRECKAGE)
                    {
                        cursor[0]--;//Μετακινεί τον κέρσορα πάνω
                        setmovements(cursor);
                        playermovements[player_turn-1][moves] = 1;
                        moves++;//Προσθέτει 1 στην moves
                    }
                }
            }
            break;
        //ΟΙ ΑΛΛΕΣ ΠΕΡΙΠΤΩΣΕΙΣ ΕΙΝΑΙ ΑΝΑΛΟΓΕΣ ΤΗΣ ΠΡΩΤΗΣ
        case DOWN_ARROW:

                if (cursor[0]!=24)
                {
                    if (moves!=0)
                    {
                        if (playermovements[player_turn-1][moves-1]==1)
                        {
                            moves--;
                            playermovements[player_turn-1][moves]=0;
                            
                            unsetmovements(cursor,backup);
                            cursor[0]++;
                            break;

                        }
                    }
                    if (moves<maxmoves)
                    {
                        if(lake[cursor[0]+1][cursor[1]] != LAND && lake[cursor[0]+1][cursor[1]+1]!= LAND && lake[cursor[0]+1][cursor[1]] != WRECKAGE && lake[cursor[0]+1][cursor[1]+1]!= WRECKAGE)
                        {
                            cursor[0]++;
                            setmovements(cursor);
                            playermovements[player_turn-1][moves] = 3;
                            moves++;
                        }
                    }
                }
                

            break;
        case LEFT_ARROW:

                if(cursor[1]>1)
                {

                    if (moves!=0)
                    {
                        if (playermovements[player_turn-1][moves-1]==2)
                        {
                            moves--;
                            playermovements[player_turn-1][moves]=0;
                            unsetmovements(cursor,backup);
                            cursor[1]-=2;
                            break;

                        }
                    }
                    if (moves<maxmoves)
                    {
                        if(lake[cursor[0]][cursor[1]-2] != LAND && lake[cursor[0]][cursor[1]-1] != LAND && lake[cursor[0]][cursor[1]-2] != WRECKAGE && lake[cursor[0]][cursor[1]-1] != WRECKAGE)
                        {
                            cursor[1]-=2;
                            setmovements(cursor);
                            playermovements[player_turn-1][moves] = 4;
                            moves++;
                        }
                    }
                }
            break;
        case RIGHT_ARROW:

            if(cursor[1]<61)
            {
                    if (moves!=0)
                    {
                        if (playermovements[player_turn-1][moves-1]==4)
                        {
                            moves--;
                            playermovements[player_turn-1][moves]=0;
                            unsetmovements(cursor,backup);
                            cursor[1]+=2;
                            break;

                        }
                    }
                    if (moves<maxmoves)
                    {
                        if(lake[cursor[0]][cursor[1]+2] != LAND && lake[cursor[0]][cursor[1]+3] != LAND && lake[cursor[0]][cursor[1]+2] != WRECKAGE && lake[cursor[0]][cursor[1]+3] != WRECKAGE)
                        {
                            cursor[1]+=2;
                            setmovements(cursor);
                            playermovements[player_turn-1][moves] = 2;   
                            moves++;
                        }
                    }
            }
            break;
        case Q: //Το Q καθαρίζει τις επιλογές του χρήστη ώστε να αρχίσει από την αρχή τον ορισμό της κίνησής του
            moves=0;//Επαναφέρει την μεταβλητή moves
            cursor[0] = findship(player_turn)/100;
            cursor[1] =findship(player_turn)%100;
            for(int i=0;i<25;i++)
            {
                for(int x=0;x<64;x++)
                {
                    lake[i][x]=backup[i][x];
                }
            }

            for (int i=0;i<8;i++) playermovements[player_turn-1][i]=0;
            break;
    }
    
    printchange();//Εμφανι΄ζει τις αλλαγές που έγιναν στα στοιχεία του πίνακα lake
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));//Αδειάζει το buffer για την getch

    input = getch();
    } while (input != 13); //Όσο δεν πατάμε το enter 
}

/*Κινεί όλους τους παίκτες σύμφωνα με τις κινήσεις που όρισαν - Ελέγχει και για κινήσεις ανέμων με την βοήθεια της windcheck
Δέχεται ως παραμετρο τον συνολικό αριθμό των παικτών στο τρέχον παιχνίδι*/
int moveplayers(int players)
{   
    for(int i=0;i<4;i++) for(int x=0;x<2;x++) savedpos[i][x]=0; //Επαναφέρουμε της savedpos σε 0
    setpreview();
    for(int i=0;i<25;i++) 
    {
        for(int x=0;x<64;x++)
        {
            if(lake[i][x]%1000==178) 
            {
                lake[i][x]=SEA; //Όλες οι θέσεις με προεπισκόπηση κίνησης στην lake επαναφέρονται σε θάλασσα
            }
        } 
    }
    setfinishline();
    printchange();

    for(int i=0;i<8;i++)
    {
        if(wincheck()==true) return 1; //Υπάρχει νικητής
        char indicator[41];
        //Χρησιμοποιούμε της snprintf ώστε να δημιουργήσουμε ένα string από ένωση σταθερού string με τιμή μεταβλητής
        snprintf(indicator,sizeof(indicator),"Moving ships (phase %d/8). Please wait...",i+1); 
        changelaketext(indicator, 11);
        bool affectedbywind[4]={false};
        setpreview();
        for(int x=0;x<players;x++)
        {
            if(alive[x]==true)
            {
                bool doublemovement=false;
                if(windcheck(x+1)==true)//Αν το παρασύρει ο άνεμος
                {
                    //Αν ο άνεμος έχει κατεύθυνση ίδια με την κίνηση του παίκτη πρέπει να κινηθεί 2 κουτάκια αντί για 1
                    if(wind_dir==playermovements[x][i])
                    {
                        doublemovement=true;
                    }
                    playermovements[x][i]=wind_dir;
                    affectedbywind[x]=true;
                }
                moveto(playermovements[x][i],x+1);
                if (doublemovement==true) moveto(wind_dir,x+1);
                /*Αν το πλοίο σταματήσει να έχει ορισμένες κινήσεις πριν εκτελεστούν και οι 8 γύροι
                αποθηκεύεται στην donemoving το νούμερο του παίκτη. Αυτό συμβαίνει για να μην υπάρχουν 
                προβλήματα με την crashcheck
                */
                if (i!=7) if(playermovements[x][i+1]==0) donemoving[x]=true; 
                playermovements[x][i]=0;   
            }      
        }
        for (int i=0;i<4;i++)
        {
            /*Αποθηκέυεται η τοποθεσία πλοίων που κινήθηκαν απο τον άνεμο
            διοτι πλέον δεν μπορεί να τα βρει η findhsip λόγω της αλλαγής του χρώματός τους*/
            if(affectedbywind[i]==true && alive[i]==true)
            {
                savedpos[i][0] = findship(i+1)/100;
                savedpos[i][1] = findship(i+1)%100+1;
            } 
        }
        printchange();

        setpreview();
        for (int i=0;i<4;i++)
        {
            if (savedpos[i][0]!=0 || savedpos[i][1]!=0)
            {
                if(alive[i]==true)
                {
                    lake[savedpos[i][0]][savedpos[i][1]]=7219;
                    lake[savedpos[i][0]][savedpos[i][1]-1]=7219;
                }  
            }
        }
        //H χρήση της sleep γίνεται ώστε ο χρήστης να μπορεί να δει τον κάθε γύρο κίνησης
        printchange();              
        Sleep(300);  
        resetsavedpos();

       Sleep(300); 
    }
    return 0;
}

/*Μετακινεί τον παίκτη προς την δοσμένη κατεύθυνση
Δέχεται ως παραμέτρους:
int direction: την κατεύθυνση της κίνησης
int player: το νούμερο που αντιστοιχεί στον παίκτη που θέλουμε να μετακινήσουμε (1-4)*/
int moveto(int direction, int player)
{
    //Βρίσκει το πλοίο του παίκτης που θέλουμε να μετακινήσουμε και αποθηκεύει τις συντεταγμένες του
    int shipY=findship(player)/100;
    int shipX=findship(player)%100;
   
    if (alive[player-1]==true)
    {
        switch(direction)
        {
            case 1://Επάνω
                    if(findship(player)/100==0) lives[player-1]--; //Αν ο παίκτης βρίσκεται στο πιο πάνω κουτάκι χάνει μια ζωη (δεν παέι πιο πάνω)
                    if(crashcheck(player,shipX,shipY-1) == true && findship(player)/100>0)
                    {
                        lake[shipY][shipX]=SEA;
                        lake[shipY][shipX+1]=SEA;
                        lake[shipY-1][shipX]=playercolors[player]*1000+219;
                        lake[shipY-1][shipX+1]=playercolors[player]*1000+219;
                    }else if (alive[player-1]== false)//Αν ο παίκτης δεν ζει εμφανίζει την εικόνα του ναυάγιου
                    { 
                        wreck(player);
                    }

                break;
            case 2://Δεξιά
                    if(findship(player)%100>61) lives[player-1]--; 

                    if(crashcheck(player,shipX+2,shipY) == true && findship(player)%100<62)
                    {
                        lake[shipY][shipX]=SEA;
                        lake[shipY][shipX+1]=SEA;
                        lake[shipY][shipX+2]=playercolors[player]*1000+219;
                        lake[shipY][shipX+3]=playercolors[player]*1000+219;
                    }else if (alive[player-1]== false)
                    { 
                        wreck(player);
                    }

                
                break;
            case 3://Κάτω
                    if(findship(player)/100==24) lives[player-1]--; 

                    if(crashcheck(player,shipX,shipY+1) == true && findship(player)/100<24)
                    {
                        lake[shipY][shipX]=SEA;
                        lake[shipY][shipX+1]=SEA;
                        lake[shipY+1][shipX]=playercolors[player]*1000+219;
                        lake[shipY+1][shipX+1]=playercolors[player]*1000+219;
                    }else if (alive[player-1]== false)
                    { 
                        wreck(player);
                    }

                
                break;
            case 4://Αριστερά
                    if(findship(player)%100<2) lives[player-1]--; 
                    if(crashcheck(player,shipX-1,shipY) == true && findship(player)%100>1)
                    {
                        lake[shipY][shipX]=SEA;
                        lake[shipY][shipX+1]=SEA;
                        lake[shipY][shipX-1]=playercolors[player]*1000+219;
                        lake[shipY][shipX-2]=playercolors[player]*1000+219;
                    }else if (alive[player-1]== false)
                    { 
                        wreck(player);
                    }

                break;
            default:

                break;
        }
    }
    return 0;
}

/*Ελέγχει αν το πλοίο πέφτει πάνω σε κάτι
Δέχεται ως παραμέτρους:
int player: τον αριθμό του παίκτη
int x: την τετμημένη της θέσης στην οποία θέλουμε να μετακινηθεί το πλοίο (Το αριστερό κουτάκι απο τα 2)
int y: την τεταγμένη της θέσης στην οποία θέλουμε να μετακινηθεί το πλοίο
Επιστρέφει true αν ο παίκτης μπορεί να μετακινηθεί στην θέση x,y ή flase σε περίπτωση που συγκρουστεί*/
bool crashcheck(int player,int x, int y)
{
    int target = lake[y][x];
    int sectarget = lake[y][x+1];//Μια θέση πιο δεξια (γιατί το πλοίο καταλαμβάνει 2 θέσεις)
    int targetplayer = 5;
    bool didcollide = false;
    int targetship=0;

    if(target == PLAYER1) targetplayer =1; 
    if(target == PLAYER2) targetplayer =2; 
    if(target == PLAYER3) targetplayer =3; 
    if(target == PLAYER4) targetplayer =4; 

    if (target==LAND || target==WRECKAGE || sectarget==LAND || sectarget==WRECKAGE) 
    {
        lives[player-1]--; 
        didcollide=true;  
    }
    /* Ελέγχει αν το κουτάκι που θα μετακινηθεί ο παίκτης έχει άλλο πλοίο
    Επειδη θέλουμε οι παίκτες να εκτελούν κάθε κίνηση ταυτόχρονα, ελέγχουμε αν το πλοίο που βρίσκεται
    στην θέση στην οποία θέλουμε να μετακινηθούμε ανήκει σε παίκτη με αριθμό μικρότερο απο του τρέχον παίκτη.
    Αυτό συμβαίνει γιατί στην πραγματικότητα οι παίκτες δεν κινούνται ταυτόχρονα αλλά με σειρά (πρώτα ο 1, μετα ο 2 κ.ο.κ.)
    Άρα αν το targetplayer είναι μεγαλύτερο απο το player σημαίνει ότι ο targetplayer δεν έχει κινηθεί ακόμη.
    Αν όμως έχει τελειώσει τις κινήσεις του, τότε δεν θα κινηθεί άλλο και πρέπει να υπάρξει σύγκρουση. Για αυτό
    έχουμε την donemoving
    */
    if (targetplayer<player || donemoving[targetplayer-1]==true)
    {
        lives[player-1]--;
        lives[targetplayer-1]--;
        if (lives[targetplayer-1] <= 0) 
        {
            alive[targetplayer-1]=false;
            wreck(targetplayer);
        }
        didcollide=true;  
    }
    
    if (lives[player-1] <= 0) alive[player-1]=false;
    return !didcollide;
}

/*Εντοπίζει το πλοίο του παίκτη 
Ως παράμετρο δέχεται τον αριθμό που αντιστοιχεί στον παίκτη του οποίου το πλοίο αναζητούμε (1-4)
Επιστρέφει 0 αν δεν βρεθεί το ζητούμενο πλοίο ή
επιστρέφει αριθμο της μορφης yyxx οπου yy η γραμμή και xx η στήλη στην οποία βρέθηκε*/
int findship(int player)
{
    for(int i=0;i<25;i++)
    {
        for(int x=0;x<64;x++)
        {
           if (lake[i][x]==playercolors[player]*1000+219)
           {
               return i*100+x;
           }
        }
    }
    return 0;
}


/*Εμφανίζει την προεπισκόπιση κίνησης (εκτος αν η θέση του κέρσορα αντιστοιχεί σε θέση πλοίου παίκτη)
Ως παράμετρο δέχεται την θέση του κέρσορα*/
int setmovements(int cursor[])
{
    if (lake[cursor[0]][cursor[1]]%1000 != 219)
    {   
        lake[cursor[0]][cursor[1]] = playercolors[player_turn]*1000+178;
        lake[cursor[0]][cursor[1]+1] = playercolors[player_turn]*1000+178;
    }
    return 0;
}

/*Διαγράφει την προεπισκόπιση κίνησης
Ως παραμέτρους δέχεται:
int cursor[]: πίνακας με το x και y του κέρσορα
int backup[25][64]: το backup της λιμνης με τα στοιχεια πριν την εμφανιση οποιασδήποτε προεπισκόπησης*/
int unsetmovements(int cursor[], int backup[25][64])
{
    
    lake[cursor[0]][cursor[1]] = backup[cursor[0]][cursor[1]];
    lake[cursor[0]][cursor[1]+1] = backup[cursor[0]][cursor[1]+1];
}

/*Ελέγχει αν κάποιος παίκτης έφτασε στον τερματισμό
Επιστρέφει true αν υπάρχει νικητής
επιστρέφει flase αν δεν υπάρχει νικητής*/
bool wincheck()
{
    for(int i=0;i<4;i++)
    {
        if(findship(i+1)/100==6 && findship(i+1)%100>=0 && findship(i+1)%100<=11)
        {
            winners[i]=true;
        }
    }
    for(int i=0;i<4;i++) if(winners[i]==true) return true;
    return false;
}

//Ορίζει την prevlake να είναι ίση με την lake ώστε να λειτουργήσει η printchange
int setpreview()
{
     for(int i=0;i<25;i++) for(int x=0;x<64;x++) prevlake[i][x]=lake[i][x];
}

/*Εντοπίζει να στοιχεία του πίνακα lake που έχουν αλλάξει, μετακινεί τον κέρσορα σε εκείνο το σημείο και εμφανίζει "απο πάνω" το αλλαγμένο στοιχείο
χωρίς να διαγράφει και να ξαναεμφανίζει ολόκληρη την οθόνη.*/
int printchange()
{
     for(int i=0;i<25;i++)
    {
        for(int x=0;x<64;x++)
        {
           if (lake[i][x]!=prevlake[i][x])
           {
                MoveConsoleCursorToPosition (x+11, i+8);
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), lake[i][x]/1000);
                printf("%c",lake[i][x]%1000);
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_RESET);
           }
        }
    }
}

/*Αλλάζει την ένταση και την διέυθυνση του ανέμου σύμφωνα με την δυσκολία
Ως παράμετρο δέχεται την δυσκολία του παιχνιδιού*/
int windchange(int difficulty)
{
    int random = rand() / (RAND_MAX / 100 + 1);//Λαμβάνουμε έναν τυχαίο αριθμό με max το 100
    
    switch(difficulty)
    {
        case 0:
            wind_spd= rand() / (RAND_MAX / 10 + 1);
            if (random>20 && random<60)
            {
                wind_dir++; //Αλλάζει 90 μοίρες δεξιόστροφα (χρειάζεται και ο έλεγχος στο τέλος)

            }else if (random>=60) 
            {
                wind_dir--; //Αλλάζει 90 μοίρες αριστερόστροφα (χρειάζεται και ο έλεγχος στο τέλος)
            }
            break;
        case 1:
            wind_spd= rand() / (RAND_MAX / 15 + 1); 
            if (random>20 && random<60)
            {
                wind_dir++; //Αλλάζει 90 μοίρες δεξιόστροφα (χρειάζεται και ο έλεγχος στο τέλος)

            }else if (random>=60) 
            {
                wind_dir--; //Αλλάζει 90 μοίρες αριστερόστροφα (χρειάζεται και ο έλεγχος στο τέλος)
            }
            break;
        case 2:
            wind_dir=rand() / (RAND_MAX / 4 + 1);

            if (rand() / (RAND_MAX / 100 + 1)<5)
            {
                wind_spd= rand() / (RAND_MAX / 5 + 1) + 15;
            }else wind_spd= rand() / (RAND_MAX / 15 + 1);
            break;
    }

    if (wind_dir==0) wind_dir=4;
    if (wind_dir==5) wind_dir=1;
    if(cur_round==0) wind_spd=0;
}

/*Ελέγχει αν ο άνεμος θα παρασύρει το πλοίο του παίκτη player
Ως παράμετρο δέχεται τον αριθμό του παίκτη player (1-4)
Επιστρέφει false αν ο παίκτης δεν επηρεαστέι απο τον άνεμο ή true στην αντίθετη περίπτωση*/
int windcheck(int player)
{
    int random = rand() / (RAND_MAX / 100 + 1); //Επιλέγει έναν τυχαίο αριθμό απο το 0 μέχρι το 100
    int chances;
        
    if(ships[player-1]==0)//Αν ο παίκτης έχει το πλοίο που αντιστέκεται στον άνεμο
    {
        if(wind_spd<=5) return false; //Αν ο άνεμος είναι μικρότερος ή ισος με 5 δεν επηρεάζεται
        else if (wind_spd<=15) chances = wind_spd*2.5; //Αν ο άνεμος είναι μέχρι 15 έχει μισές πιθανότητες
        else chances = wind_spd*5; //Αλλιώς έχει ολόκληρες τις πιθανότητες

    }else chances = wind_spd*5;

    if (chances>=10) chances-=10;//Για ισοστάθμιση δυσκολίας, ώστε να μην υπάρχει ποτέ 100% πιθανότητα
    if (random>=chances) return false;
    else return true;
}

//Επαναφέρει τα πλοία που βρίσκονται αποθηκευμένα στην savedpos στο κανονικό τους χρώμα και ορίζει όλα της τα στοιχεία σε 0
int resetsavedpos()
{
    setpreview();
    for(int i=0;i<4;i++) 
    {
        if(savedpos[i][0]!=0 || savedpos[i][1]!=0) 
        {
            lake[savedpos[i][0]][savedpos[i][1]]=playercolors[i+1]*1000+219; 
            lake[savedpos[i][0]][savedpos[i][1]-1]=playercolors[i+1]*1000+219;
            savedpos[i][0]=0; 
            savedpos[i][1]=0; 
        }
    }
    printchange();
    return 0;
}

/*Εμφανίζει τις αλλαγές σε στοιχεία user interface κατα την διάρκεια του παιχνιδιού (Ανεμος, γύρος και σειρά)
Ως παράμετρο δέχεται το αναγνωριστικό του UI που πρέπει να αλλάξει (βλ αρχη του προγράμματος)*/
int changeui(int object)
{
    MoveConsoleCursorToPosition (object,4);
    if (object==PLAYER) 
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), playercolors[player_turn]);
        printf(" %d ",player_turn);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_RESET);
    }
    else if (object==WIND) 
    {
    _setmode(_fileno(stdout), _O_U16TEXT);
    switch(wind_dir)
    {
        case 1:
            wprintf(L" ↑ ");
            break;
        case 2:
            wprintf(L" → ");
            break;
        case 3:
            wprintf(L" ↓ ");
            break;
        case 4:
            wprintf(L" ← ");
            break;
        default:

            break;
    }
    _setmode(_fileno(stdout), _O_TEXT);
    //Καθαρίζει την θέση 12,6 οι οποία περιέχει χαρακτήρες μόνο για ανέμους διψήφιας έντασης
    MoveConsoleCursorToPosition (12,6);
    printf(" ");
    MoveConsoleCursorToPosition (10,6);

    if (wind_spd <10)
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_GREEN);
    }else if(wind_spd <15)
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_YELLOW);
    }else if(wind_spd <=20)
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_RED);
    }
    printf("%dB\n",wind_spd);

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_RESET); 
    }else if(object==ROUND)
    {
       printf("%d",cur_round+1); 
    }
    return 0;
}

/*Εμφανίζει τα σύμβολα του κατεστραμένου πλοίου στην θέση του πλοίου του target, αποθηκεύει την θέση του στην wreckship και ορίζει
το αντίστοιχο wreckcounter σε 3 ώστε να αρχίσει η αντίστροφη μέτρηση
Δέχεται ως παράμετρο τον αριθμό του παίκτη στον οποίο αντιστοιχεί το πλοίο*/
int wreck(int target)
{
    int targetship = findship(target);
    if(target==0 && (savedpos[target][0]!=0 || savedpos[target][0]!=0))
    {
        targetship = savedpos[target][0]*100+(savedpos[target][1]-1);
    }
    lake[targetship/100][targetship%100]=WRECKAGE;
    lake[targetship/100][targetship%100+1]=WRECKAGE;
    wreckship[target-1][0]=targetship/100;
    wreckship[target-1][1]=targetship%100+1;
    wreckcounter[target-1]=3;
    savedpos[target-1][0]=0;
    savedpos[target-1][1]=0;
    return 0;
}

/*Εμφανίζει την νίκη
Επιστρέφει κλήση στην main ώστε να επιστρέψει ο χρήστης στο αρχικό μενού*/
int winscreen()
{
    int win_num=0;//Ο αριθμός των νικητ΄ων
    for(int i=0;i<4;i++) if(winners[i]==true) win_num++;
    int x=37;
    int y=16;

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 2);
    MoveConsoleCursorToPosition(x,y);
    printf("%c",d_top_left_corner);
    for(int i=0;i<10;i++) printf("%c",d_horizontal_line);
    printf("%c",d_top_right_corner);
    
    MoveConsoleCursorToPosition(x,y+1);
    printf("%c",d_vertical_line);
    printf("  WINNER");
    if(win_num>1) printf("S "); 
    else printf("  "); 
    printf("%c",d_vertical_line);

    MoveConsoleCursorToPosition(x,y+2);
    printf("%c ",d_vertical_line);
    for(int i=0;i<8;i++) printf("%c",s_horizontal_line);
    printf(" %c",d_vertical_line);

    for(int i=0;i<4;i++)
    {   
        MoveConsoleCursorToPosition(x,y+3+i);
        printf("%c",d_vertical_line);
        if(winners[i]==true)
        {
            for(int x=0;x<10;x++) {printf("%c",names[i][x]); if(names[i][x]==0) printf(" ");}
        }else printf("          ");
        printf("%c",d_vertical_line);
    }

    MoveConsoleCursorToPosition(x,y+7);
    printf("%c",d_bottom_left_corner);
    for(int i=0;i<10;i++) printf("%c",d_horizontal_line);
    printf("%c",d_bottom_right_corner);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_RESET);

    changelaketext("Press any key to return to the menu", COLOR_GREEN);
    getchar();
    return main();

}

/*Εμφανίζει την ήττα
Επιστρέφει κλήση στην main ώστε να επιστρέψει ο χρήστης στο αρχικό μενού*/
int defeatscreen()
{
    int x=35;
    int y=19;  

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_RED);
    MoveConsoleCursorToPosition(x,y);
    printf("%c",d_top_left_corner);
    for(int i=0;i<13;i++) printf("%c",d_horizontal_line);
    printf("%c",d_top_right_corner);
    MoveConsoleCursorToPosition(x,y+1);
    printf("%c",d_vertical_line);
    printf("EVERYONE LOST");
    printf("%c",d_vertical_line);
    MoveConsoleCursorToPosition(x,y+2);
    printf("%c",d_bottom_left_corner);
    for(int i=0;i<13;i++) printf("%c",d_horizontal_line);
    printf("%c",d_bottom_right_corner);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_RESET);
    
    changelaketext("Press any key to return to the menu", COLOR_RED);
    getchar();
    return main();
}

/*Αλλάζει το κείμενο κάτω απο την λίμη
Δέχεται ως παραμέτρους:
char *word: την συμβολοσειρά που θέλουμε να εμφανήσουμε
int color: το χρώμα που θέλουμε να έχει το κείμενο*/
int changelaketext(char *word, int color)
{
    MoveConsoleCursorToPosition(11,35);
    for(int i=0;i<64;i++) printf(" ");//Εμφανίζει κενά κα΄τω από όλη την λίμνη ώστε να διαγραφούν προηγούμενες εμφανίσεις
    MoveConsoleCursorToPosition(11,35);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
    printf("%s",word);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_RESET);
}

//Συνάρτηση που κουνάει τον κέρσορα της κονσόλας σε τοποθεσία που ορίζουμε (x,y)
int MoveConsoleCursorToPosition (int x, int y)
{
    HANDLE Screen;
    Screen = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD Position={x, y};

    SetConsoleCursorPosition(Screen, Position);
}
