/* NILA */
/* BAHMAN 1403 */

#include <SDL2/SDL.h> 
#include <SDL2/SDL_mixer.h> 
#include <dirent.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <stdarg.h> 
#include <stdbool.h>
#include <locale.h>
#include <pthread.h>

#define WIDTH  85
#define HEIGHT 40

bool map_status[HEIGHT][WIDTH] = {false};
bool temp_map_status[HEIGHT][WIDTH];
bool temp_map_status2[HEIGHT][WIDTH] = {false};
bool trap_check[HEIGHT][WIDTH] = {false};
bool treasure_roomm[HEIGHT][WIDTH] = {false};
bool nightmare_room[HEIGHT][WIDTH] = {false};
bool spell_room[HEIGHT][WIDTH] = {false};
char selected_music[256] = {0};
bool change_music = true;
bool change_music2 = true;




// Definitions for user management
char* NAME_FILE = "player_users.txt";
#define all_games "all_games.txt"
#define MAX_PLAYERS 100
#define MAX_PLAYER 6
#define MAX_WEAPONS 5
#define MAX_MUSIC_FILES 100 
#define MUSIC_DIR "musics" 
#define EASY_SCORE 25 
#define MEDIUM_SCORE 20 
#define HARD_SCORE 15
#define MAX_FOOD_ITEMS 5
#define FOOD_NORMAL      0
#define FOOD_SUPERIOR    1
#define FOOD_MAGIC       2
#define FOOD_ROTTEN      3
#define MAX_HIDDEN_DOORS 10
#define MAX_SPELLS 3
#define MAX_MONSTERS 5
#define ENEMY 'E'

char *music_files[MAX_MUSIC_FILES]; 
int num_music_files = 0;
int current_floor = 1;
int health_multiplier = 1;
int speed_multiplier = 1;
int speed_multiplier2 = 1;
int magic_act = 1;
int damage_spells = 1;
int move_count = 0;
int move_count_magic = 0;
int move_count_damage = 0;
int move_count_food = 0;
int player_color = 22;
int difficulty = 0; // 0: Easy, 1: Medium, 2: Hard
int time_limit; 
int last_dx = 0;
int last_dy = 0;
int trap_seen = 0;
bool has_last_direction = false;
bool treasure_room_entered = false;
bool last_near_window = false;
int last_window_x, last_window_y;
int print_d = 0;
int print_s = 0;
int print_g = 0;
int print_u = 0;
int print_f = 0;
int healthy_up = 0;
int attacking = 0;
int s_attack = 0;
int u_attack = 0;
int d_attack = 0;
int g_attack = 0;
int f_attack = 0;
bool auto_moving = false;
int auto_dx = 0;
int auto_dy = 0;
int hidden_door_count = 0;
char door_pass[5];

#define NUMCOLS 85
#define NUMLINES 35
#define MAXROOMS 6

#define PASSAGE     '#'
#define DOOR        '+'
#define FLOOR       '.'
#define WALL_HORZ   '_'
#define WALL_VERT   '|'
#define PLAYER      '@'
#define STAIRS      '<'
#define TRAP        '^'
#define GOLD_NORMAL '*'
#define GOLD_ADVANCED '~'
#define UNKHOWN_DOOR      '?'
#define MAGIC       '$'
#define FOOD        ':'
#define SUPERIOR    '9'
#define ROTTEN      '/'
#define WINDOW      '='

typedef struct {
    int x, y;
} coord;

struct room {
    coord r_pos;
    coord r_max;
};

struct room rooms[MAXROOMS];
coord player_pos; 
coord treasure_room;

struct Gamer {
    char username[100];
    char password[100];
    char email[50];
    int score;
    int gold;
    int finished_games;
    time_t time_first;
};

struct Gamer g = {
    .username = "",
    .password = "",
    .email = "",
    .score = 0,
    .gold = 0,
    .finished_games = 0,
    .time_first = 0
};


typedef struct {
    int hunger; 
    int health;
    int food_count;
    int food_type[MAX_FOOD_ITEMS];
    struct {
        char name[20];
        int hunger_reduction;
        int health_reduction;
    } food_inventory[MAX_FOOD_ITEMS];
} Player;

Player player = {
    .hunger = 100,
    .health = 200,
    .food_count = 0
};

const char* food_types[] = {
    "Normal",
    "Superior",
    "Magic",
    "Rotten"
};

const int food_hunger_reduction[] = {
    20, // Normal
    30, // Superior
    50, // Magic
    -10 // Rotten
};

const int food_health_reduction[] = {
    5,  // Normal
    10, // Superior
    25, // Magic
    -20 // Rotten (decreases health)
};

coord hidden_doors[MAX_HIDDEN_DOORS];

typedef struct {
    char symbol;
    char name[20];
    int damage;
    bool is_ranged;
    int range;
    int count;
    bool thrown;
} Weapon;

Weapon weapons[MAX_WEAPONS] = {
    { 'M', "Mace", 5, false, 1, 1 , false},
    { 'K', "Dagger", 12, true, 5, 10 , false},
    { 'A', "Magic Wand", 15, true, 10, 10 , false},
    { 'N', "Normal Arrow", 5, true, 5, 10, false},
    { '!', "Sword", 10, false, 1, 10 , false}
};

Weapon *player_weapon = NULL;

typedef struct {
    char symbol;
    char name[20];
    int count;
} Spell;

Spell spells[MAX_SPELLS] = {
    { 'h', "Health", 2 },
    { 's', "Speed", 1 },
    { 'd', "Damage", 1 }
};

typedef struct {
    coord pos; 
    int health;    
    bool alive;   
    bool stunned;
} Daemon;

typedef struct {
    coord pos;      
    int health;     
    bool alive;     
    bool stunned; 
} FireBreathingMonster;

typedef struct {
    coord pos;      
    int health;     
    bool alive;     
    bool stunned; 
    int chase_count;
    bool check_move
} Giant;

typedef struct {
    coord pos;     
    int health;    
    bool alive;     
    bool stunned; 
} Snake;

typedef struct {
    coord pos;      
    int health;     
    bool alive;   
    bool stunned; 
    int chase_count;
    bool check_move;
} Undeed;

Daemon daemons[MAXROOMS][MAX_MONSTERS];
FireBreathingMonster fbms[MAXROOMS][MAX_MONSTERS];
Giant giant[MAXROOMS][MAX_MONSTERS];
Snake snake[MAXROOMS][MAX_MONSTERS];
Undeed undeed[MAXROOMS][MAX_MONSTERS];
int move_giant[MAXROOMS][MAX_MONSTERS] = {0};
int move_undeed[MAXROOMS][MAX_MONSTERS] = {0};

void ask_for_password();
void generate_random_password2(char* password, int length);
void display_password_for_30_seconds();
void update_corner_of_room_with_locked_door();
void lock_door_and_update_room();
void stop_music();
void* music_thread(void* arg);
void auto_move_player();
void auto_move_player_one_step();
void initialize_monsters();
void exit_game ();
void draw_menu_border();
int show_main_menu();
int show_pre_game_menu();
void generate_random_password();
bool is_valid_email(const char *email);
void create_new_user();
int check_user_email();
void update_user_password();
void forgot_password();
int post_login_menu();
void login_user();
int count_gamers(const char *filename);
void read_gamers(char *filename, struct Gamer *gamers, int num_gamers);
int compare_gamers(const void *a, const void *b);
void sort_gamers_by_rank(struct Gamer *gamers, int num_gamers);
void show_scoreboard(char *filename, char *current_username);
void set_difficulty();
void choose_hero_color();
void load_music_files();
const char* select_music();
void play_music(const char *file);
void show_settings();
void show_profile();
void get_player_info(struct Gamer *g);
int check_name_exists(const char *username);
void save_username(const char *username);
int validate_user_login(const char *username, const char *password);
bool is_near_window();
void display_treasure_room();
void reveal_two_steps_around(int x, int y);
void create_spell_room();
bool check_spell_room();
void create_nightmare_room();
bool check_nightmare_room(int x, int y) ;
void display_nightmare_room();
void hide_room_through_window();
void reveal_room_through_window(int window_x, int window_y);
void kill_monster(int y, int x) ;
void attack_with_dagger();
void attack_with_normal();
void attack_with_sword();
void attack_with_mace();
void initialize_previous_floor();
void save_current_floor();
void show_all_map();
void reveal_three_steps_ahead(int x, int y, int dx, int dy, int steps) ;
void reveal_room(int x, int y);
void initialize_map_status();
void update_map_status();
void display_updated_map();
void end_game2();
void display_status();
void check_and_convert_doors();
void draw_room(struct room *rp);
void vert(struct room *rp, int startx);
void horiz(struct room *rp, int starty);
void initialize_map();
void display_map();
int rnd(int range);
void place_door();
void draw_passage(coord start, coord end);
void add_random_passages();
bool has_weapon(char weapon_symbol);
void handle_weapon_pickup(int x, int y, char weapon_symbol);
void place_random_monsters();
void move_monsters() ;
void attack_with_weapon();
void attack_enemy(int x, int y, int damage);
void place_random_spells();
bool ask_player_to_take_spell(const char *spell_name) ;
bool ask_player_to_take_weapon(const char *weapon_name);
void show_spells();
void show_inventory();
void place_random_weapons();
bool ask_player_to_take_food();
void show_message_top_right(const char* format, ...);
void show_message_top_right2(const char* format, ...);
void show_message_top_right3(const char* format, ...);
void show_message_top_right4(const char* format, ...);
void show_message_top_right5(const char* format, ...);
void show_message_top_right6(const char* format, ...);
void show_message_top_right7(const char* format, ...);
void show_message_top_right8(const char* format, ...);
void show_message_top_right9(const char* format, ...);
void collect_food(int food_type);
void show_food_menu();
void consume_food(int index);
void place_random_windows();
void check_treasure_room();
void update_food_inventory();
void treas_room();
void move_until_blocked(int dx, int dy);
void move_through_objects(int dx, int dy);
void place_random_gold();
void place_random_gold2();
void place_random_stairs(int *stairs_x, int *stairs_y);
void place_random_traps();
void place_player();
void move_player(int dx, int dy);
void initialize_new_floor();
void update_timer(time_t start_time, int time_limit);
bool show_exit_menu();
void save_game(time_t start_time, int time_limit);
void handle_input(int ch ,time_t start_time, int time_limit);
void start_timer();
void update_user_info(const char* username, struct Gamer* updated_gamer);
void end_game();
void start_new_game();
void continue_game();
void start_timer_with_remaining_time(int remaining_time);


char chat[NUMLINES][NUMCOLS]; // Map array



// Main function

int main() {
    setlocale(LC_ALL, "");
    srand(time(0));
    initscr();
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK); 
    init_pair(2, COLOR_GREEN, COLOR_WHITE);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_pair(4, COLOR_BLACK, COLOR_YELLOW);
    init_pair(5, COLOR_YELLOW, COLOR_BLACK); 
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(7, COLOR_BLACK, COLOR_RED); 
    init_pair(8, COLOR_BLACK, COLOR_BLUE);
    init_pair(9, COLOR_WHITE, COLOR_BLACK);
    init_pair(10, COLOR_BLUE, COLOR_BLACK); 
    init_pair(11, COLOR_CYAN, COLOR_BLACK);
    init_pair(12, COLOR_YELLOW, COLOR_RED); 
    init_pair(16, COLOR_RED, COLOR_YELLOW); 
    init_pair(17, COLOR_BLACK, COLOR_GREEN); 
    init_pair(18, COLOR_WHITE, COLOR_CYAN); 
    init_pair(19,COLOR_BLACK , COLOR_MAGENTA);
    init_pair(20, COLOR_BLACK, COLOR_WHITE);
    init_pair(21,COLOR_YELLOW,COLOR_CYAN);
    init_pair(22, COLOR_WHITE, COLOR_BLACK); 
    init_pair(23, COLOR_RED, COLOR_BLUE); 
    init_pair(24, COLOR_MAGENTA, COLOR_BLACK); 
    init_pair(25, COLOR_RED, COLOR_BLACK); 




    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);

    while (1) {
        clear();
        draw_menu_border();
        int choice = show_main_menu();
        switch (choice) {
            case 1: create_new_user(); break;
            case 2: login_user(); break;
            case 3: show_scoreboard(NAME_FILE , g.username); break;
            case 4: show_settings(); break;
            case 5: exit_game(); break;
            default:
                mvprintw(LINES - 2, 2, "Invalid choice! Please try again.");
                refresh();
                usleep(2000000);
        }
    }

    endwin();
    return 0;
}

void exit_game (){
    endwin();
    printf("Thank you for playing!\n");
    exit(0);
}


// Draw menu borders
void draw_menu_border() {
    for (int i = 0; i < COLS; i++) {
        mvprintw(0, i, "*");
        mvprintw(LINES - 1, i, "*");
    }
    for (int i = 0; i < LINES; i++) {
        mvprintw(i, 0, "*");
        mvprintw(i, COLS - 1, "*");
    }
}

// Main menu
int show_main_menu() {
    const char *options[] = {
        "1. Create New User",
        "2. Login",
        "3. View Scoreboard",
        "4. Settings",
        "5. Exit"
    };
    int selected = 0;
    int num_options = sizeof(options) / sizeof(options[0]);

    while (1) {
        clear();
        draw_menu_border();
        attron(COLOR_PAIR(1));
        mvprintw(2, COLS / 2 - 10, "Main Menu");
        attroff(COLOR_PAIR(1));
        for (int i = 0; i < num_options; i++) {
            if (i == selected) attron(A_REVERSE);
            mvprintw(4 + i, COLS / 2 - 15, "%s", options[i]);
            if (i == selected) attroff(A_REVERSE);
        }

        int key = getch();
        switch (key) {
            case KEY_UP: selected = (selected - 1 + num_options) % num_options; break;
            case KEY_DOWN: selected = (selected + 1) % num_options; break;
            case 10: 
                switch (selected + 1) {
                    case 1: create_new_user(); break;
                    case 2: login_user(); break;
                    case 3: show_scoreboard(NAME_FILE , g.username); break;
                    case 4: show_settings(); break;
                    case 5: exit_game(); break;
                    default:
                        mvprintw(LINES - 2, 2, "Invalid choice! Please try again.");
                        refresh();
                        usleep(2000000);
                }
                break;
        }
    }
}

// Pre-game menu
int show_pre_game_menu() {
    const char *options[] = {
        "1. Start New Game",
        "2. Continue Game",
        "3. Back to Last Menu"
    };
    int selected = 0;
    int num_options = sizeof(options) / sizeof(options[0]);

    while (1) {
        clear();
        draw_menu_border();
        attron(COLOR_PAIR(1));
        mvprintw(2, COLS / 2 - 10, "Pre-Game Menu");
        attroff(COLOR_PAIR(1));
        for (int i = 0; i < num_options; i++) {
            if (i == selected) attron(A_REVERSE);
            mvprintw(4 + i, COLS / 2 - 15, "%s", options[i]);
            if (i == selected) attroff(A_REVERSE);
        }

        int key = getch();
        switch (key) {
            case KEY_UP: selected = (selected - 1 + num_options) % num_options; break;
            case KEY_DOWN: selected = (selected + 1) % num_options; break;
            case 10:
                if (selected == 0) {
                    start_new_game();
                } else if (selected == 1) {
                    continue_game();
                } else if (selected == 2) {
                    post_login_menu(g.username);
                }
                break;
        }
    }
}

void update_timer(time_t start_time, int time_limit) {
    int remaining_time = time_limit - difftime(time(NULL), start_time);
    mvprintw(1, COLS / 2 - 10, "Time left: %d seconds", remaining_time);
    draw_menu_border();
    attron(COLOR_PAIR(3));
    for (int i = 1; i <= LINES/2 ; i++) {
        mvprintw(i, COLS - 52, "|");
    }
    for (int i = 1; i <= LINES/2 ; i++) {
        mvprintw(i, COLS - 2, "|");
    }
    for (int i = 0; i < 51; i++) {
        mvprintw(1, COLS - 52 + i, "_");
    }
    for (int i = 0; i < 51; i++) {
        mvprintw(LINES/2, COLS - 52 + i, "_");
    }
    attroff(COLOR_PAIR(3));
    attron(COLOR_PAIR(1));
    mvprintw(1, COLS - 52, "♥"); 
    mvprintw(1, COLS - 2, "♥");
    attroff(COLOR_PAIR(1)); 
    attron(COLOR_PAIR(11));
    for (int i = LINES/2 + 1; i <= LINES - 2 ; i++) {
        mvprintw(i, COLS - 52, "|");
    }
    for (int i = LINES/2 + 1; i <= LINES - 2 ; i++) {
        mvprintw(i, COLS - 2, "|");
    }
    for (int i = 0; i < 51; i++) {
        mvprintw(LINES/2 + 1, COLS - 52 + i, "_");
    }
    for (int i = 0; i < 51; i++) {
        mvprintw(LINES - 2, COLS - 52 + i, "_");
    }
    mvprintw(LINES/2 + 1, COLS - 52, "😎");
    mvprintw(LINES/2 + 1, COLS - 2, "😎");
    attroff(COLOR_PAIR(11));
    attron(COLOR_PAIR(7)); 
    mvprintw(2, COLS - 35, "Game Messages");
    mvprintw(LINES/2 + 2 , COLS - 35, "Fighting the Enemy");
    attroff(COLOR_PAIR(7)); 

    refresh();
}

bool show_exit_menu() {
    int selected = 0;
    const char *options[] = { "Yes", "No" };
    int num_options = sizeof(options) / sizeof(options[0]);

    while (1) {
        clear();
        mvprintw(LINES / 2 - 2, COLS / 2 - 10, "DO YOU WANT TO STOP YOUR GAME?");
        for (int i = 0; i < num_options; i++) {
            if (i == selected) attron(A_REVERSE);
            mvprintw(LINES / 2 + i, COLS / 2 - 10, "%s", options[i]);
            if (i == selected) attroff(A_REVERSE);
        }

        int key = getch();
        switch (key) {
            case KEY_UP: selected = (selected - 1 + num_options) % num_options; break;
            case KEY_DOWN: selected = (selected + 1) % num_options; break;
            case 10: 
                return selected == 0;
        }
    }
}

void display_score() {
    mvprintw(0, COLS - 20, "Score: %d", g.score);
}

void place_random_food() {
    int food_count = rnd(5) + 5;
    int food_count2 = (rnd(5) + 5)/2;
    int food_index = 0;

    for (int i = 0; i < food_count; i++) {
        int x, y;
        do {
            x = rnd(NUMCOLS);
            y = rnd(NUMLINES);
        } while (chat[y][x] != FLOOR); 
        chat[y][x] = FOOD;
        player.food_type[food_index] = FOOD_NORMAL;
        food_index++;
        attron(COLOR_PAIR(11)); 
        mvaddch(y, x, FOOD);
        attroff(COLOR_PAIR(11));
    }
    for (int i = 0; i < food_count2; i++) {
        int x, y;
        do {
            x = rnd(NUMCOLS);
            y = rnd(NUMLINES);
        } while (chat[y][x] != FLOOR); 
        chat[y][x] = MAGIC;
        player.food_type[food_index] = FOOD_MAGIC;
        food_index++;
        attron(COLOR_PAIR(11)); 
        mvaddch(y, x, MAGIC);
        attroff(COLOR_PAIR(11));
    }
    for (int i = 0; i < food_count2; i++) {
        int x, y;
        do {
            x = rnd(NUMCOLS);
            y = rnd(NUMLINES);
        } while (chat[y][x] != FLOOR); 
        chat[y][x] = SUPERIOR;
        player.food_type[food_index] = FOOD_SUPERIOR;
        food_index++;
        attron(COLOR_PAIR(11)); 
        mvaddch(y, x, SUPERIOR);
        attroff(COLOR_PAIR(11));
    }
    for (int i = 0; i < food_count2; i++) {
        int x, y;
        do {
            x = rnd(NUMCOLS);
            y = rnd(NUMLINES);
        } while (chat[y][x] != FLOOR); 
        chat[y][x] = ROTTEN;
        player.food_type[food_index] = FOOD_ROTTEN;
        food_index++;
        attron(COLOR_PAIR(11)); 
        mvaddch(y, x, ROTTEN);
        attroff(COLOR_PAIR(11));
    }
    int pillar_num = 1;
    for (int i = 0; i < pillar_num; i++) {
        int x, y;
        do {
            x = rnd(NUMCOLS);
            y = rnd(NUMLINES);
        } while (chat[y][x] != FLOOR); 
        chat[y][x] = 'O';
        mvaddch(y, x, 'O');
    }

}




void save_game(time_t start_time, int time_limit) {
    char game_name[50];
    int map;
    clear();
    mvprintw(LINES / 2, COLS / 2 - 10, "NAME YOUR GAME:");
    echo();
    getstr(game_name);
    noecho();

    struct stat st = {0};
    if (stat("all_games", &st) == -1) {
        mkdir("all_games", 0700);
    }


    int remaining_time = time_limit - difftime(time(NULL), start_time);

    char user_file_path[100];
    snprintf(user_file_path, sizeof(user_file_path), "all_games/%s", g.username);

    FILE *user_file_ptr = fopen(user_file_path, "w");
    if (user_file_ptr == NULL) {
        mvprintw(LINES / 2 + 2, COLS / 2 - 10, "Error saving game.");
        return;
    }

    fprintf(user_file_ptr, "Username: %s\n", g.username);
    fprintf(user_file_ptr, "Game Name: %s\n", game_name);
    fprintf(user_file_ptr, "Current Floor: %d\n", current_floor);
    fprintf(user_file_ptr, "Remaining Time: %d\n", remaining_time);
    fprintf(user_file_ptr, "Player Position: %d %d\n", player_pos.x, player_pos.y); 
    fprintf(user_file_ptr, "Player Color: %d\n", player_color); 
    fprintf(user_file_ptr, "Score: %d\n", g.score); 
    fprintf(user_file_ptr, "Gold: %d\n", g.gold); 
    fprintf(user_file_ptr, "Finished Games: %d\n", g.finished_games);

    fprintf(user_file_ptr, "Daemons:\n");
    for (int i = 0; i < MAXROOMS; i++) {
        for (int j = 0; j < MAX_MONSTERS; j++) {
            if (daemons[i][j].alive) {
                if(daemons[i][j].stunned){
                    fprintf(user_file_ptr, "%d %d %d %d %d %d\n", i, j, daemons[i][j].pos.x, daemons[i][j].pos.y, daemons[i][j].health , 1);
                }else if(!daemons[i][j].stunned){
                    fprintf(user_file_ptr, "%d %d %d %d %d %d\n", i, j, daemons[i][j].pos.x, daemons[i][j].pos.y, daemons[i][j].health , 0);
                }
            }
        }
    }

    fprintf(user_file_ptr, "Fbms:\n");
    for (int i = 0; i < MAXROOMS; i++) {
        for (int j = 0; j < MAX_MONSTERS; j++) {
            if (fbms[i][j].alive) {
                if(fbms[i][j].stunned){
                    fprintf(user_file_ptr, "%d %d %d %d %d %d\n", i, j, fbms[i][j].pos.x, fbms[i][j].pos.y, fbms[i][j].health , 1);
                }else if(!fbms[i][j].stunned){
                    fprintf(user_file_ptr, "%d %d %d %d %d %d\n", i, j, fbms[i][j].pos.x, fbms[i][j].pos.y, fbms[i][j].health , 0);
                }
            }
        }
    }

    fprintf(user_file_ptr, "Giant:\n");
    for (int i = 0; i < MAXROOMS; i++) {
        for (int j = 0; j < MAX_MONSTERS; j++) {
            if (giant[i][j].alive) {
                if(giant[i][j].stunned){
                    fprintf(user_file_ptr, "%d %d %d %d %d %d\n", i, j, giant[i][j].pos.x, giant[i][j].pos.y, giant[i][j].health , 1);
                }else if(!giant[i][j].stunned){
                    fprintf(user_file_ptr, "%d %d %d %d %d %d\n", i, j, giant[i][j].pos.x, giant[i][j].pos.y, giant[i][j].health , 0);
                }
            }
        }
    }

    fprintf(user_file_ptr, "Snake:\n");
    for (int i = 0; i < MAXROOMS; i++) {
        for (int j = 0; j < MAX_MONSTERS; j++) {
            if (snake[i][j].alive) {
                if(snake[i][j].stunned){
                    fprintf(user_file_ptr, "%d %d %d %d %d %d\n", i, j, snake[i][j].pos.x, snake[i][j].pos.y, snake[i][j].health , 1);
                } else if(!snake[i][j].stunned){
                    fprintf(user_file_ptr, "%d %d %d %d %d %d\n", i, j, snake[i][j].pos.x, snake[i][j].pos.y, snake[i][j].health , 0);
                }
            }
        }
    }

    fprintf(user_file_ptr, "Undeed:\n");
    for (int i = 0; i < MAXROOMS; i++) {
        for (int j = 0; j < MAX_MONSTERS; j++) {
            if (undeed[i][j].alive) {
                if(undeed[i][j].stunned){
                    fprintf(user_file_ptr, "%d %d %d %d %d %d\n", i, j, undeed[i][j].pos.x, undeed[i][j].pos.y, undeed[i][j].health , 1);
                }else if(!undeed[i][j].stunned){
                    fprintf(user_file_ptr, "%d %d %d %d %d %d\n", i, j, undeed[i][j].pos.x, undeed[i][j].pos.y, undeed[i][j].health , 0);
                }
                
            }
        }
    }

    fprintf(user_file_ptr, "Spells:\n");
    for (int i = 0; i < MAX_SPELLS; i++) {
        fprintf(user_file_ptr, "%c %s %d\n", spells[i].symbol, spells[i].name, spells[i].count);
    }

    fprintf(user_file_ptr, "Weapons:\n");
    for (int i = 0; i < MAX_WEAPONS; i++) {
        if(weapons[i].thrown){
            fprintf(user_file_ptr, "%c %s %d %d %d\n", weapons[i].symbol, weapons[i].name, weapons[i].count,weapons[i].damage , 1);
        }else if(!weapons[i].thrown){
            fprintf(user_file_ptr, "%c %s %d %d %d\n", weapons[i].symbol, weapons[i].name, weapons[i].count,weapons[i].damage , 0);
        }
    }

    fprintf(user_file_ptr, "Foods:\n");
    for (int i = 0; i < sizeof(food_types)/sizeof(food_types[0]); i++) {
        fprintf(user_file_ptr, "%s\n", food_types[i]);
    }


    fprintf(user_file_ptr, "Food Locations:\n");
    for (int y = 0; y < NUMLINES; y++) {
        for (int x = 0; x < NUMCOLS; x++) {
            if (chat[y][x] == FOOD) {
                fprintf(user_file_ptr, "%d %d\n", x, y);
            }
        }
    }

    fprintf(user_file_ptr, "Hidden Doors: %d\n", hidden_door_count); 
    for (int i = 0; i < hidden_door_count; i++) {
        fprintf(user_file_ptr, "Hidden Door %d: %d %d\n", i, hidden_doors[i].x, hidden_doors[i].y);
    }

    fprintf(user_file_ptr, "Treasure Room:\n");
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            fprintf(user_file_ptr, "%d ", treasure_roomm[y][x] ? 1 : 0);
        }
        fprintf(user_file_ptr, "\n");
    }

    fprintf(user_file_ptr, "Nightmare Room:\n");
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            fprintf(user_file_ptr, "%d ", nightmare_room[y][x] ? 1 : 0);
        }
        fprintf(user_file_ptr, "\n");
    }

    fprintf(user_file_ptr, "Spell Room:\n");
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            fprintf(user_file_ptr, "%d ", spell_room[y][x] ? 1 : 0);
        }
        fprintf(user_file_ptr, "\n");
    }

    fprintf(user_file_ptr, "Map:\n");
    for (int y = 0; y < NUMLINES; y++) {
        for (int x = 0; x < NUMCOLS; x++) {
            fputc(chat[y][x], user_file_ptr);
        }
        fputc('\n', user_file_ptr);
    }
    fprintf(user_file_ptr, "Map Status:\n");
    for (int y = 0; y < NUMLINES; y++) {
        for (int x = 0; x < NUMCOLS; x++) {
            if(map_status[y][x])map = 1;
            else if(!map_status[y][x])map = 0;
            // else if(!map_status[y][x])map = 0;
            fprintf(user_file_ptr, "%d ", map);
        }
        fprintf(user_file_ptr, "\n");
    }

    fprintf(user_file_ptr, "End of Game\n\n");
    fclose(user_file_ptr);

    mvprintw(LINES / 2 + 2, COLS / 2 - 10, "Game saved successfully.");
}

void show_spells() {
    clear();
    mvprintw(2, COLS / 2 - 10, "Spells Inventory");
    for (int i = 0; i < MAX_SPELLS; i++) {
        mvprintw(4 + i, COLS / 2 - 20, "%s: %d", spells[i].name, spells[i].count);
    }
    mvprintw(LINES - 2, COLS / 2 - 20, "Press x to exit or the first letter of the spell type to use it:");
    refresh();
    
    int ch = getch();
    if (ch == 'x' || ch == 'X') {
        return;
    }

    bool found_spell = false;
    for (int i = 0; i < MAX_SPELLS; i++) {
        if (tolower(ch) == tolower(spells[i].name[0])) {
            if (spells[i].count > 0) {
                spells[i].count--;
                mvprintw(LINES - 3, COLS / 2 - 20, "You used a %s spell!", spells[i].name);
                found_spell = true;
                if (tolower(ch) == 'h') { 
                    health_multiplier = 2;
                }else if (tolower(ch) == 's') { 
                    speed_multiplier = 2; 
                }else if(tolower(ch) == 'd') {
                    damage_spells = 2;
                }
                break;
            } else {
                mvprintw(LINES - 3, COLS / 2 - 20, "You don't have any %s spell!", spells[i].name);
                found_spell = true;
                break;
            }
        }
    }
    if (!found_spell) {
        mvprintw(LINES - 3, COLS / 2 - 20, "Invalid spell type selected!");
    }
    refresh();
    getch(); 
}



void handle_input(int ch ,time_t start_time, int time_limit) {
    if (auto_moving) {
        return; 
    }

    if(ch == ' '){
        if (player_weapon == NULL) {
        show_message_top_right5("No weapon in hand!");
        return;
        }
        if(player_weapon == &weapons[0]){
            attack_with_mace(); 
        } else if(player_weapon == &weapons[1]){
            has_last_direction = false;
            attack_with_dagger();
        } else if(player_weapon == &weapons[2]){
            has_last_direction = false;
            attack_with_wand();
        }else if(player_weapon == &weapons[3]){
            has_last_direction = false;
            attack_with_normal(); 
        }else if(player_weapon == &weapons[4]){
            attack_with_sword(); 
        }
        
    } else if (ch == 'a') {
        if (has_last_direction) {
            if(player_weapon == &weapons[1]) attack_with_dagger();
            if(player_weapon == &weapons[2]) attack_with_wand();
            if(player_weapon == &weapons[3]) attack_with_normal();
        } else {
            show_message_top_right5("No previous direction!");
        }
    }else {
        switch (ch) {
        case '8': move_player(0, -1); break;  // Up
        case '2': move_player(0, 1); break;   // Down
        case '6': move_player(1, 0); break;   // Right
        case '4': move_player(-1, 0); break;  // Left
        case '9': move_player(1, -1); break;  // Up-Right
        case '7': move_player(-1, -1); break; // Up-Left
        case '1': move_player(-1, 1); break;  // Down-Left
        case '3': move_player(1, 1); break;   // Down-Right
        case 'e': 
        case 'E': show_food_menu(); break;
        case 'f':
        case 'F':
            auto_moving = true;
            mvprintw(3, COLS / 2 - 10, "Select a direction (1-9): ");
            refresh();
            int direction_key = getch();
            switch (direction_key) {
                case '7':
                    auto_dx = -1;
                    auto_dy = -1;
                    break;
                case '8':
                    auto_dx = 0;
                    auto_dy = -1;
                    break;
                case '9': 
                    auto_dx = 1;
                    auto_dy = -1;
                    break;
                case '4':
                    auto_dx = -1;
                    auto_dy = 0;
                    break;
                case '6': 
                    auto_dx = 1;
                    auto_dy = 0;
                    break;
                case '1': 
                    auto_dx = -1;
                    auto_dy = 1;
                    break;
                case '2': 
                    auto_dx = 0;
                    auto_dy = 1;
                    break;
                case '3': 
                    auto_dx = 1;
                    auto_dy = 1;
                    break;
                default:
                    auto_moving = false;
                    return;
            }
            auto_move_player();
            break;
        case 'g':
        case 'G':
            auto_moving = true;
            mvprintw(3, COLS / 2 - 10, "Select a direction (1-9): ");
            refresh();
            direction_key = getch();
            switch (direction_key) {
                case '7': 
                    auto_dx = -1;
                    auto_dy = -1;
                    break;
                case '8': 
                    auto_dx = 0;
                    auto_dy = -1;
                    break;
                case '9': 
                    auto_dx = 1;
                    auto_dy = -1;
                    break;
                case '4': 
                    auto_dx = -1;
                    auto_dy = 0;
                    break;
                case '6': 
                    auto_dx = 1;
                    auto_dy = 0;
                    break;
                case '1': 
                    auto_dx = -1;
                    auto_dy = 1;
                    break;
                case '2': 
                    auto_dx = 0;
                    auto_dy = 1;
                    break;
                case '3': 
                    auto_dx = 1;
                    auto_dy = 1;
                    break;
                default:
                    auto_moving = false;
                    return;
            }
            auto_move_player_one_step();
            break;
        case 'i':
        case 'I':show_inventory(); break;
        case 't':
        case 'T':show_spells(); break;
        case 'm':
        case 'M':show_all_map(); break;
        case 'q': 
            if (show_exit_menu()) { 
                save_game(start_time, time_limit); 
                post_login_menu(&g);
            } 
            break;
        }
        update_map_status();
        display_updated_map();
    }
    move_monsters(); 
    check_treasure_room();
}


void start_timer() {
    time_t start_time = time(NULL);
    while (difftime(time(NULL), start_time) < time_limit) {
        int remaining_time = time_limit - difftime(time(NULL), start_time);
        update_timer(start_time , time_limit);
        display_map();
        int key = getch();
        handle_input(key, start_time, remaining_time);
    }
    end_game();
}

void update_user_info(const char* username, struct Gamer* updated_gamer) {
    if(strcmp(g.username , "guest") == 0)
    {
        return;
    }
    int num_gamers = count_gamers(NAME_FILE);
    char *temp = malloc(sizeof(char) * 100);
    if (num_gamers <= 0) {
        mvprintw(2, COLS / 2 - 10, "No gamers found.");
        refresh();
        getch();
        return;
    }

    struct Gamer *gamers = (struct Gamer*)malloc(num_gamers * sizeof(struct Gamer));
    if (gamers == NULL) {
        perror("Error allocating memory");
        return;
    }


    read_gamers(NAME_FILE, gamers, num_gamers);
    
    FILE *file = fopen(NAME_FILE, "w+");
    if (!file)
        return;
    for(int i = 0 ; i < num_gamers ; i++)
    {
        
        if(strcmp(g.username ,gamers[i].username) == 0){
            gamers[i].gold += g.gold;
            gamers[i].score += g.score;
            gamers[i].finished_games++;
        }
        if (file) {
            fprintf(file, "%s,%s,%s,%d,%d,%d,%ld\n", gamers[i].username, gamers[i].password,
             gamers[i].email,gamers[i].score,gamers[i].gold,gamers[i].finished_games ,gamers[i].time_first);   
        }
    }

    fclose(file);
}
void stop_music() {
    Mix_HaltMusic();
    Mix_CloseAudio();
    SDL_Quit();
}


void end_game() {
    clear();
    stop_music();
    memset(selected_music, 0, sizeof(selected_music));  
    mvprintw(LINES / 2, COLS / 2 - 10, "Game Over! Time's up!");
    mvprintw(LINES / 2 + 1, COLS / 2 - 10, "Your score: %d" , g.score);
    int start_y = LINES / 2 - 1;
    int start_x = COLS / 2 - 12;
    int end_y = LINES / 2 + 2;
    int end_x = COLS / 2 + 12;

    // Draw the top and bottom edges of the rectangle
    for (int x = start_x; x <= end_x; x++) {
        attron(COLOR_PAIR(6));
        mvprintw(start_y, x, "-");
        mvprintw(end_y, x, "-");
        attroff(COLOR_PAIR(6));
    }

    // Draw the left and right edges of the rectangle
    for (int y = start_y; y <= end_y; y++) {
        attron(COLOR_PAIR(6));
        mvprintw(y, start_x, "|");
        mvprintw(y, end_x, "|");
        attroff(COLOR_PAIR(6));
    }

    // Draw the corners of the rectangle
    attron(COLOR_PAIR(6));
    mvprintw(start_y, start_x, "+");
    mvprintw(start_y, end_x, "+");
    mvprintw(end_y, start_x, "+");
    mvprintw(end_y, end_x, "+");
    attroff(COLOR_PAIR(6));
    refresh();
    getch();

    update_user_info(g.username, &g);
    post_login_menu(&g);
}
void end_game2() {
    clear();
    stop_music();
    memset(selected_music, 0, sizeof(selected_music)); 
    mvprintw(LINES / 2, COLS / 2 - 10, "Game Over! You died!");
    mvprintw(LINES / 2 + 1, COLS / 2 - 10, "Your score: %d" , g.score);
    int start_y = LINES / 2 - 1;
    int start_x = COLS / 2 - 12;
    int end_y = LINES / 2 + 2;
    int end_x = COLS / 2 + 12;

    // Draw the top and bottom edges of the rectangle
    for (int x = start_x; x <= end_x; x++) {
        attron(COLOR_PAIR(6));
        mvprintw(start_y, x, "-");
        mvprintw(end_y, x, "-");
        attroff(COLOR_PAIR(6));
    }

    // Draw the left and right edges of the rectangle
    for (int y = start_y; y <= end_y; y++) {
        attron(COLOR_PAIR(6));
        mvprintw(y, start_x, "|");
        mvprintw(y, end_x, "|");
        attroff(COLOR_PAIR(6));
    }

    // Draw the corners of the rectangle
    attron(COLOR_PAIR(6));
    mvprintw(start_y, start_x, "+");
    mvprintw(start_y, end_x, "+");
    mvprintw(end_y, start_x, "+");
    mvprintw(end_y, end_x, "+");
    attroff(COLOR_PAIR(6));
    refresh();
    getch();

    update_user_info(g.username, &g);
    post_login_menu(&g);
}

void* music_thread(void* arg) {
    const char* file = (const char*)arg;
    play_music(file);
    return NULL;
}


void start_new_game()
{   
    player_weapon = &weapons[0];
    int stairs_x , stairs_y;
    pthread_t music_tid;
    initialize_map();
    random_generate_room();
    add_random_passages();
    place_random_stairs(&stairs_x, &stairs_y);
    place_random_traps();
    place_random_food();
    place_random_gold();
    place_random_gold2();
    check_and_convert_doors();
    lock_door_and_update_room();
    update_corner_of_room_with_locked_door();
    place_random_weapons();
    place_random_spells();
    place_random_windows();
    place_player();
    place_random_monsters();
    initialize_map_status();
    if (strlen(selected_music) > 0) {
        pthread_create(&music_tid, NULL, music_thread, (void*)selected_music);
    }
    time_t start_time = time(NULL);
    while (difftime(time(NULL), start_time) < time_limit) {
        clear();
        draw_menu_border();
        update_map_status();
        display_updated_map();
        display_map();
        mvprintw(3, COLS - 45, "Score: %d | Gold: %d | Healthy: %d", g.score,g.gold ,player.health);
        update_timer(start_time, time_limit);
        if(print_d == 1){
            mvprintw(4, COLS - 50, "Kill Daemon quickly;Your health decreased by 1");
        }
        if(print_s == 1){
            mvprintw(5, COLS - 50, "Kill Snake quickly;Your health decreased by 4");
        }
        if(print_g == 1){
            mvprintw(6, COLS - 50, "Kill Giant quickly;Your health decreased by 3");
        }
        if(print_u == 1){
            mvprintw(7, COLS - 50, "Kill Undead quickly;Your health decreased by 5");
        }
        if(print_f == 1){
            mvprintw(8, COLS - 50, "Kill Fbms quickly;Your health decreased by 2");
        }
        if(healthy_up == 1){
            mvprintw(9, COLS - 50, "YOU GOT %d MORE HEALTH.NOW: %d" ,health_multiplier*5, player.health);
            healthy_up = 0;
        }
        if(attacking == 1){
            mvprintw(LINES/2 + 3,COLS - 50,"You are attacking with Monsters ;)");
            attacking = 0;
        }
        if(s_attack == 1){
            mvprintw(LINES/2 + 4,COLS - 50,"You killed Snake :)");
            s_attack = 0;
        }
        if(u_attack == 1){
            mvprintw(LINES/2 + 5,COLS - 50,"You killed Undead :)");
            u_attack = 0;
        }
        if(d_attack == 1){
            mvprintw(LINES/2 + 6,COLS - 50,"You killed Daemon :)");
            d_attack = 0;
        }
        if(g_attack == 1){
            mvprintw(LINES/2 + 7,COLS - 50,"You killed Giant :)");
            g_attack = 0;
        }
        if(f_attack == 1){
            mvprintw(LINES/2 + 8,COLS - 50,"You killed Fbms :)");
            f_attack = 0;
        }

        int key = getch();
        handle_input(key, start_time, time_limit);
    }
    end_game();
    getch();
    display_map();

}


void continue_game() {
    char user_file_path[100];
    snprintf(user_file_path, sizeof(user_file_path), "all_games/%s", g.username);

    FILE *user_file_ptr = fopen(user_file_path, "r");
    if (user_file_ptr == NULL) {
        mvprintw(LINES / 2 + 2, COLS / 2 - 10, "No saved game found.");
        return;
    }

    int remaining_time = 0;
    int i, j, x, y, health; 
    int s;

    // Read the saved game file
    char line[100];
    while (fgets(line, sizeof(line), user_file_ptr)) {
        if (strncmp(line, "Current Floor:", 14) == 0) {
            sscanf(line + 14, "%d", &current_floor);
        } else if (strncmp(line, "Remaining Time:", 15) == 0) {
            sscanf(line + 15, "%d", &remaining_time);
        } else if (strncmp(line, "Player Position:", 16) == 0) {
            sscanf(line + 16, "%d %d", &player_pos.x, &player_pos.y);
        } else if (strncmp(line, "Player Color:", 13) == 0) {
            sscanf(line + 13, "%d", &player_color);
        }else if (strncmp(line, "Score:", 6) == 0) { 
            sscanf(line + 6, "%d", &g.score); 
        }else if (strncmp(line, "Gold:", 5) == 0) { 
            sscanf(line + 5, "%d", &g.gold);
        } else if (strncmp(line, "Finished Games:", 15) == 0) {
            sscanf(line + 15, "%d", &g.finished_games);
        }else if (strncmp(line, "Daemons:", 8) == 0) {
            while (fgets(line, sizeof(line), user_file_ptr)) {
                if (sscanf(line, "%d %d %d %d %d", &i, &j, &x, &y, &health , &s) == 6) {
                    daemons[i][j].pos.x = x;
                    daemons[i][j].pos.y = y;
                    daemons[i][j].health = health;
                    daemons[i][j].alive = true;
                    if(s == 1)daemons[i][j].stunned = true;
                    else if(s == 0) daemons[i][j].stunned = (s == 1);
                    // daemons[i][j].stunned = false;
                    chat[y][x] = 'D';
                    attron(COLOR_PAIR(7));
                    mvaddch(y, x, 'D');
                    attroff(COLOR_PAIR(7));
                } else {
                    break;
                }
            }
        }else if (strncmp(line, "Fbms:", 5) == 0) {
            while (fgets(line, sizeof(line), user_file_ptr)) {
                if (sscanf(line, "%d %d %d %d %d", &i, &j, &x, &y, &health , &s) == 6) {
                    fbms[i][j].pos.x = x;
                    fbms[i][j].pos.y = y;
                    fbms[i][j].health = health;
                    fbms[i][j].alive = true;
                    if(s == 1)fbms[i][j].stunned = true;
                    else if(s == 0) fbms[i][j].stunned = (s == 1);
                    // fbms[i][j].stunned = false;
                    chat[y][x] = 'F';
                    attron(COLOR_PAIR(8));
                    mvaddch(y, x, 'F');
                    attroff(COLOR_PAIR(8));
                } else {
                    break;
                }
            }
        } else if (strncmp(line, "Giant:", 6) == 0) {
            while (fgets(line, sizeof(line), user_file_ptr)) {
                if (sscanf(line, "%d %d %d %d %d", &i, &j, &x, &y, &health, &s) == 6) {
                    giant[i][j].pos.x = x;
                    giant[i][j].pos.y = y;
                    giant[i][j].health = health;
                    giant[i][j].alive = true;
                    if(s == 1)giant[i][j].stunned = true;
                    else if(s == 0) giant[i][j].stunned = (s == 1);
                    chat[y][x] = 'G';
                    attron(COLOR_PAIR(17));
                    mvaddch(y, x, 'G');
                    attroff(COLOR_PAIR(17));
                } else {
                    break;
                }
            }
        } else if (strncmp(line, "Snake:", 6) == 0) {
            while (fgets(line, sizeof(line), user_file_ptr)) {
                if (sscanf(line, "%d %d %d %d %d", &i, &j, &x, &y, &health , &s) == 6) {
                    snake[i][j].pos.x = x;
                    snake[i][j].pos.y = y;
                    snake[i][j].health = health;
                    snake[i][j].alive = true;
                    if(s == 1)snake[i][j].stunned = true;
                    else if(s == 0) snake[i][j].stunned = (s == 1);
                    chat[y][x] = 'S';
                    attron(COLOR_PAIR(16));
                    mvaddch(y, x, 'S');
                    attroff(COLOR_PAIR(16));
                } else {
                    break;
                }
            }
        } else if (strncmp(line, "Undeed:", 7) == 0) {
            while (fgets(line, sizeof(line), user_file_ptr)) {
                if (sscanf(line, "%d %d %d %d %d", &i, &j, &x, &y, &health , &s) == 6) {
                    undeed[i][j].pos.x = x;
                    undeed[i][j].pos.y = y;
                    undeed[i][j].health = health;
                    undeed[i][j].alive = true;
                    if(s == 1)undeed[i][j].stunned = true;
                    else if(s == 0) undeed[i][j].stunned = (s == 1);
                    chat[y][x] = 'U';
                    attron(COLOR_PAIR(18));
                    mvaddch(y, x, 'U');
                    attroff(COLOR_PAIR(18));
                } else {
                    break;
                }
            }
        } else if (strncmp(line, "Spells:", 7) == 0) {
            for (int i = 0; i < MAX_SPELLS; i++) {
                fgets(line, sizeof(line), user_file_ptr);
                sscanf(line, "%c %s %d", &spells[i].symbol, spells[i].name, &spells[i].count);
            }
        } else if (strncmp(line, "Weapons:", 8) == 0) {
            for (int i = 0; i < MAX_WEAPONS; i++) {
                fgets(line, sizeof(line), user_file_ptr);
                sscanf(line, "%c %s %d %d %d", &weapons[i].symbol, weapons[i].name, &weapons[i].count, &weapons[i].damage ,&s);
                if(s == 1) weapons[i].thrown = true;
                else if(s == 0) weapons[i].thrown = false;
            }
        }else if (strncmp(line, "Foods:", 6) == 0) {
            for (int i = 0; i < sizeof(food_types) / sizeof(food_types[0]); i++) {
                fgets(line, sizeof(line), user_file_ptr);
                
            }
        }else if (strncmp(line, "Food Locations:", 15) == 0) {
            while (fgets(line, sizeof(line), user_file_ptr)) {
                int x, y;
                if (sscanf(line, "%d %d", &x, &y) == 2) {
                    chat[y][x] = FOOD;
                } else {
                    break;
                }
            }
        }else if (strncmp(line, "Hidden Doors:", 13) == 0) {
            sscanf(line + 13, "%d", &hidden_door_count);
            for (int i = 0; i < hidden_door_count; i++) {
                fgets(line, sizeof(line), user_file_ptr);
                sscanf(line, "Hidden Door %d: %d %d", &i, &hidden_doors[i].x, &hidden_doors[i].y);
            }
        } else if (strncmp(line, "Treasure Room:", 14) == 0) {
            for (int y = 0; y < HEIGHT; y++) {
                for (int x = 0; x < WIDTH; x++) {
                    int treasure_value;
                    fscanf(user_file_ptr, "%d", &treasure_value);
                    treasure_roomm[y][x] = (treasure_value == 1);
                }
            }
        } else if (strncmp(line, "Nightmare Room:", 15) == 0) {
            for (int y = 0; y < HEIGHT; y++) {
                for (int x = 0; x < WIDTH; x++) {
                    int nightmare_value;
                    fscanf(user_file_ptr, "%d", &nightmare_value);
                    nightmare_room[y][x] = (nightmare_value == 1);
                }
            }
        } else if (strncmp(line, "Spell Room:", 11) == 0) {
            for (int y = 0; y < HEIGHT; y++) {
                for (int x = 0; x < WIDTH; x++) {
                    int spell_value;
                    fscanf(user_file_ptr, "%d", &spell_value);
                    spell_room[y][x] = (spell_value == 1);
                }
            }
        }else if (strncmp(line, "Map:", 4) == 0) {
            for (int y = 0; y < NUMLINES; y++) {
                fgets(line, sizeof(line), user_file_ptr);
                for (int x = 0; x < NUMCOLS; x++) {
                    chat[y][x] = line[x];
                }
            }
        }else if (strncmp(line, "Map Status:", 11) == 0) {
            for (int y = 0; y < NUMLINES; y++) {
                for (int x = 0; x < NUMCOLS; x++) {
                    int map_value;
                    fscanf(user_file_ptr, "%d", &map_value);
                    if(map_value == 1) map_status[y][x] = true;
                    else if(map_value == 0)  map_status[y][x] = false;
                }
            }
        }
    }
    fclose(user_file_ptr);
    print_d = 0;
    print_s = 0;
    print_g = 0;
    print_u = 0;
    print_f = 0;
    attacking = 0;
    d_attack = 0;
    g_attack = 0;
    f_attack = 0;
    s_attack = 0;
    u_attack = 0;
    damage_spells = 1;
    move_monsters();
    check_and_convert_doors();
    clear();
    mvprintw(0, COLS - 20, "Score: %d", g.score);
    mvprintw(1, COLS - 20, "Gold: %d", g.gold);
    mvprintw(2, COLS - 20, "Time: %d", remaining_time); 

    mvprintw(LINES / 2 + 2, COLS / 2 - 10, "Game loaded successfully.");
    refresh();
    getch();
    time_t start_time = time(NULL);
    while (remaining_time > 0) {
        clear();
        move_monsters();
        update_map_status();
        display_updated_map();
        draw_menu_border();
        display_map();
        move_monsters();
        refresh();
        check_and_convert_doors();
        draw_menu_border();
        mvprintw(3, COLS - 43, "Score: %d | Gold: %d | Healthy: %d", g.score,g.gold ,player.health);
        update_timer(start_time, remaining_time);
        attron(COLOR_PAIR(3));
    for (int i = 1; i <= LINES/2 ; i++) {
        mvprintw(i, COLS - 52, "|");
    }
    for (int i = 1; i <= LINES/2 ; i++) {
        mvprintw(i, COLS - 2, "|");
    }
    for (int i = 0; i < 51; i++) {
        mvprintw(1, COLS - 52 + i, "_");
    }
    for (int i = 0; i < 51; i++) {
        mvprintw(LINES/2, COLS - 52 + i, "_");
    }
    attroff(COLOR_PAIR(3));
    attron(COLOR_PAIR(11));
    for (int i = LINES/2 + 1; i <= LINES - 5 ; i++) {
        mvprintw(i, COLS - 52, "|");
    }
    for (int i = LINES/2 + 1; i <= LINES - 5 ; i++) {
        mvprintw(i, COLS - 2, "|");
    }
    for (int i = 0; i < 51; i++) {
        mvprintw(LINES/2 + 1, COLS - 52 + i, "_");
    }
    for (int i = 0; i < 51; i++) {
        mvprintw(LINES - 5, COLS - 52 + i, "_");
    }
    attroff(COLOR_PAIR(11));
    attron(COLOR_PAIR(1));
    mvprintw(1, COLS - 52, "♥"); 
    mvprintw(1, COLS - 2, "♥");
    attroff(COLOR_PAIR(1)); 
    mvprintw(LINES/2 + 1, COLS - 52, "😎"); 
    mvprintw(LINES/2 + 1, COLS - 2, "😎");
    attron(COLOR_PAIR(7));  
    mvprintw(2, COLS - 35, "Game Messages");
    mvprintw(LINES/2 + 2 , COLS - 35, "Fighting the Enemy");
    attroff(COLOR_PAIR(7)); 
    if(print_d == 1){
            mvprintw(4, COLS - 50, "Kill Daemon quickly;Your health decreased by 1");
        }
        if(print_s == 1){
            mvprintw(5, COLS - 50, "Kill Snake quickly;Your health decreased by 2");
        }
        if(print_g == 1){
            mvprintw(6, COLS - 50, "Kill Giant quickly;Your health decreased by 1");
        }
        if(print_u == 1){
            mvprintw(7, COLS - 50, "Kill Undead quickly;Your health decreased by 2");
        }
        if(print_f == 1){
            mvprintw(8, COLS - 50, "Kill Fbms quickly;Your health decreased by 4");
        }
        if(healthy_up == 1){
            mvprintw(9, COLS - 50, "YOU GOT %d MORE HEALTH.NOW: %d" ,health_multiplier*5, player.health);
        }
        if(attacking == 1){
            mvprintw(LINES/2 + 3,COLS - 50,"You are attacking with Monsters ;)");
            attacking = 0;
        }
        if(s_attack == 1){
            mvprintw(LINES/2 + 4,COLS - 50,"You killed Snake :)");
            s_attack = 0;
        }
        if(u_attack == 1){
            mvprintw(LINES/2 + 5,COLS - 50,"You killed Undead :)");
            u_attack = 0;
        }
        if(d_attack == 1){
            mvprintw(LINES/2 + 6,COLS - 50,"You killed Daemon :)");
            d_attack = 0;
        }
        if(g_attack == 1){
            mvprintw(LINES/2 + 7,COLS - 50,"You killed Giant :)");
            g_attack = 0;
        }
        if(f_attack == 1){
            mvprintw(LINES/2 + 8,COLS - 50,"You killed Fbms :)");
            f_attack = 0;
        }
        int key = getch();
        handle_input(key, start_time, remaining_time);
        move_monsters();
        remaining_time -= difftime(time(NULL), start_time);
        start_time = time(NULL);
        mvprintw(2, COLS/2, "Time: %d", remaining_time);
        refresh();

    }
    end_game();
    getch();
    display_map();
}



int count_gamers(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    int count = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        count++;
    }

    fclose(file);
    return count ;
}

void read_gamers(char *filename, struct Gamer *gamers, int num_gamers) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }
    char line[256]; 
    for (int i = 0; i < num_gamers; i++) { 
        fgets(line, sizeof(line), file);  
        sscanf(line, "%[^,],%[^,],%[^,],%d,%d,%d,%d", 
                            gamers[i].username, 
                            gamers[i].password, 
                            gamers[i].email, 
                            &gamers[i].score,
                            &gamers[i].gold,
                            &gamers[i].finished_games,
                            &gamers[i].time_first);
    }
    fclose(file);
}

int compare_gamers(const void *a, const void *b) { 
    struct Gamer *gamerA = (struct Gamer *)a; 
    struct Gamer *gamerB = (struct Gamer *)b; 
    return gamerB->score - gamerA->score; 
} 

void sort_gamers_by_rank(struct Gamer *gamers, int num_gamers) { 
    qsort(gamers, num_gamers, sizeof(struct Gamer), compare_gamers);
}

void show_scoreboard(char *filename, char *current_username) {
    int num_gamers = count_gamers(filename);
    char *temp = malloc(sizeof(char) * 100);
    if (num_gamers <= 0) {
        mvprintw(2, COLS / 2 - 10, "No gamers found.");
        refresh();
        getch();
        return;
    }

    struct Gamer *gamers = (struct Gamer*)malloc(num_gamers * sizeof(struct Gamer));
    if (gamers == NULL) {
        perror("Error allocating memory");
        return;
    }


    read_gamers(filename, gamers, num_gamers);

    sort_gamers_by_rank(gamers, num_gamers);

    int page = 0;
    int total_pages = (num_gamers + MAX_PLAYER - 1) / MAX_PLAYER;
    int ch;

    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);

    while (1) {
        clear();
        draw_menu_border();
        mvprintw(2, COLS / 2 - 15, "Scoreboard - Page %d of %d", page + 1, total_pages);

        int start = page * MAX_PLAYER;
        int end = start + MAX_PLAYER;
        if (end > num_gamers) end = num_gamers;

        mvprintw(4, COLS / 2 - 25, "Rank | Username         | Score  | Gold   | Finished Games | Time Since First Game");
        mvprintw(5, COLS / 2 - 25, "-----|------------------|--------|--------|----------------|-----------------------");

        time_t current_time = time(NULL);
        for (int i = start; i < end; i++) {
            int hours_since_first = (current_time - gamers[i].time_first) / (60 * 60);
            if (hours_since_first < 0) {
                    hours_since_first = 0; 
            }
            if (i < 3) {
                if (strcmp(gamers[i].username, current_username) == 0) {
                    attron(A_BOLD);
                }
                attron(COLOR_PAIR(1)); 
                mvprintw(6 + (i - start), COLS / 2 - 25, "%4d | %-16s | %6d | %6d | %14d | %20d [GOAT]",
                         (i+1), gamers[i].username, gamers[i].score, gamers[i].gold,
                         gamers[i].finished_games, hours_since_first);
                attroff(COLOR_PAIR(1));
                 if (strcmp(gamers[i].username, current_username) == 0) {
                    attroff(A_BOLD);
                }         
                
            } else if (strcmp(gamers[i].username, current_username) == 0) {
                attron(A_BOLD);
                mvprintw(6 + (i - start), COLS / 2 - 25, "%4d | %-16s | %6d | %6d | %14d | %20d",
                         (i+1), gamers[i].username, gamers[i].score, gamers[i].gold,
                         gamers[i].finished_games, hours_since_first);
                attroff(A_BOLD);
            } else {
                mvprintw(6 + (i - start), COLS / 2 - 25, "%4d | %-16s | %6d | %6d | %14d | %20d",
                         (i+1), gamers[i].username, gamers[i].score, gamers[i].gold,
                         gamers[i].finished_games, hours_since_first);
            }
        }

        mvprintw(LINES - 2, COLS / 2 - 10, "n: Next Page, p: Previous Page, q: Quit");
        refresh();

        ch = getch();
        switch (ch) {
            case 'n':
                if (page < total_pages - 1) page++;
                break;
            case 'p':
                if (page > 0) page--;
                break;
            case 'q':
                free(gamers);
                return;
        }
    }
}


void show_settings() {
    const char *options[] = {
        "1. Set Difficulty",
        "2. Choose Hero Color",
        "3. Select Music",
        "4. Back to Main Menu"
    };
    int selected = 0;
    int num_options = sizeof(options) / sizeof(options[0]);

    while (1) {
        clear();
        draw_menu_border();
        attron(COLOR_PAIR(1));
        mvprintw(2, COLS / 2 - 10, "Settings");
        attroff(COLOR_PAIR(1));
        for (int i = 0; i < num_options; i++) {
            if (i == selected) attron(A_REVERSE);
            mvprintw(4 + i, COLS / 2 - 15, "%s", options[i]);
            if (i == selected) attroff(A_REVERSE);
        }

        int key = getch();
        switch (key) {
            case KEY_UP: selected = (selected - 1 + num_options) % num_options; break;
            case KEY_DOWN: selected = (selected + 1) % num_options; break;
            case 10:
                switch (selected) {
                    case 0: set_difficulty(); break;
                    case 1: choose_hero_color(); break;
                    case 2: 
                        strncpy(selected_music, select_music(), sizeof(selected_music) - 1);
                        break;
                    case 3: return;
                        break;
                }
        }
    }
}

void set_difficulty() {
    const char *difficulties[] = { "Easy", "Medium", "Hard" };
    int selected = 0;
    int num_difficulties = sizeof(difficulties) / sizeof(difficulties[0]);

    while (1) {
        clear();
        draw_menu_border();
        attron(COLOR_PAIR(1));
        mvprintw(2, COLS / 2 - 10, "Select Difficulty");
        attroff(COLOR_PAIR(1));
        for (int i = 0; i < num_difficulties; i++) {
            if (i == selected) attron(A_REVERSE);
            mvprintw(4 + i, COLS / 2 - 10, "%s", difficulties[i]);
            if (i == selected) attroff(A_REVERSE);
        }

        int key = getch();
        switch (key) {
            case KEY_UP: selected = (selected - 1 + num_difficulties) % num_difficulties; break;
            case KEY_DOWN: selected = (selected + 1) % num_difficulties; break;
            case 10: // Enter key
                difficulty = selected; 
                if (selected == 0) { 
                    time_limit = 2000; 
                    g.score = 25;
                    } 
                else if (selected == 1) { 
                    time_limit = 300; 
                    g.score = 20;
                    } 
                else if (selected == 2) { 
                    time_limit = 250;
                    g.score = 15;
                }
                return;
        }
    }
}

void choose_hero_color() {
    const char *colors[] = { "Red", "Green", "Blue", "Yellow" };
    int selected = 0;
    int num_colors = sizeof(colors) / sizeof(colors[0]);

    while (1) {
        clear();
        draw_menu_border();
        attron(COLOR_PAIR(1));
        mvprintw(2, COLS / 2 - 10, "Choose Hero Color");
        attroff(COLOR_PAIR(1));
        for (int i = 0; i < num_colors; i++) {
            if (i == selected) attron(A_REVERSE);
            mvprintw(4 + i, COLS / 2 - 10, "%s", colors[i]);
            if (i == selected) attroff(A_REVERSE);
        }

        int key = getch();
        switch (key) {
            case KEY_UP: selected = (selected - 1 + num_colors) % num_colors; break;
            case KEY_DOWN: selected = (selected + 1) % num_colors; break;
            case 10: // Enter key
                player_color = selected + 1;
                return;
        }
    }
}

void load_music_files() {
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(MUSIC_DIR)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_REG) {
                music_files[num_music_files] = strdup(ent->d_name);
                num_music_files++;
            }
        }
        closedir(dir);
    } else {
        perror("Could not open music directory");
    }
}

const char* select_music() {
    load_music_files();
    int selected = 0;

    while (1) {
        clear();
        draw_menu_border();
        attron(COLOR_PAIR(1));
        mvprintw(2, COLS / 2 - 10, "Select Music");
        attron(COLOR_PAIR(1));
        for (int i = 0; i < num_music_files; ++i) {
            if (i == selected) attron(A_REVERSE);
            mvprintw(4 + i, COLS / 2 - 10, "%s", music_files[i]);
            if (i == selected) attroff(A_REVERSE);
        }

        int key = getch();
        switch (key) {
            case KEY_UP: selected = (selected - 1 + num_music_files) % num_music_files; break;
            case KEY_DOWN: selected = (selected + 1) % num_music_files; break;
            case 10: // Enter key
                return music_files[selected];
        }
    }
}
void play_music(const char *file) {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "SDL_mixer could not initialize! Mix_Error: %s\n", Mix_GetError());
        return;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", MUSIC_DIR, file);

    Mix_Music *music = Mix_LoadMUS(filepath);
    if (music == NULL) {
        fprintf(stderr, "Failed to load music file! Mix_Error: %s\n", Mix_GetError());
        return;
    }

    Mix_PlayMusic(music, -1);

    // Wait until music stops playing
    while (Mix_PlayingMusic()) {
        SDL_Delay(100);
    }

    Mix_FreeMusic(music);
    Mix_CloseAudio();
    SDL_Quit();
}

void show_profile() {
    clear();
    draw_menu_border();

    attron(COLOR_PAIR(1));
    mvprintw(2, COLS / 2 - 10, "User Profile");
    attroff(COLOR_PAIR(1));
    attron(A_BOLD);
    attron(COLOR_PAIR(5));
    for (int i = 10; i < COLS - 10; i++) {
        mvprintw(8, i, "*");
        mvprintw(12, i, "*");
    }
    for (int i = 8; i <= 12; i++) {
        mvprintw(i, 10, "*");
        mvprintw(i, COLS - 11, "*");
    }
    attroff(COLOR_PAIR(5));

    attron(COLOR_PAIR(11));
    mvprintw(9, 12, "Username: %s", g.username);

    char formatted_email[100];
    sscanf(g.email, "%[^,]", formatted_email);
    mvprintw(10, 12, "Email: %s", formatted_email);

    mvprintw(11, 12, "Password: %s", g.password);
    attroff(COLOR_PAIR(11));
    attroff(A_BOLD);
    attron(COLOR_PAIR(5));
    mvprintw(LINES - 2, 2, "Press any key to return to the menu...");
    attroff(COLOR_PAIR(5));
    refresh();
    getch();
}



// Function to generate a random password 
void generate_random_password(char* password, int length) { 
    char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()"; 
    int charset_size = strlen(charset); 
    int has_upper = 0, has_lower = 0, has_digit = 0, has_special = 0; 
    srand(time(NULL)); 
    while (!has_upper || !has_lower || !has_digit || !has_special) { 
        for (int i = 0; i < length; i++) { 
            password[i] = charset[rand() % charset_size]; 
            if (password[i] >= 'A' && password[i] <= 'Z') has_upper = 1; 
            else if (password[i] >= 'a' && password[i] <= 'z') has_lower = 1; 
            else if (password[i] >= '0' && password[i] <= '9') has_digit = 1; 
            else has_special = 1; 
        } 
    } 
    password[length] = '\0';
}

bool is_valid_email(const char *email) {
    const char *at_sign = strchr(email, '@');
    const char *dot_sign = strrchr(email, '.');
    if (at_sign == NULL || dot_sign == NULL) {
        return false;
    }

    if (at_sign > dot_sign || dot_sign - at_sign < 2) {
        return false;
    }

    if (at_sign - email < 1) {
        return false;
    }

    if (dot_sign - at_sign < 2) {
        return false;
    }

    if (strlen(dot_sign) < 2) {
        return false;
    }

    return true;
}

// User creation and management
void create_new_user() {
    struct Gamer g;
    int valid = 0;

    while (!valid) {
        clear();
        draw_menu_border();
        attron(COLOR_PAIR(3));
        mvprintw(LINES / 2 - 2, COLS / 2 - 15, "Enter username: ");
        attroff(COLOR_PAIR(3));
        echo();
        getnstr(g.username, 100);
        noecho();

        if (check_name_exists(g.username)) {
            mvprintw(LINES / 2 + 2, COLS / 2 - 18, "ERROR! Username already exists!");
            refresh();
            usleep(2000000);
        } else {
            valid = 1;
        }
    }

    // Password
    valid = 0;
    while (!valid) {
        clear();
        draw_menu_border();
        attron(COLOR_PAIR(11));
        mvprintw(LINES / 2 - 2, COLS / 2 - 15, "Enter password: ");
        mvprintw(LINES / 2 - 1, COLS / 2 - 15, "If you want random password press ENTER!");
        attroff(COLOR_PAIR(11));
        echo();
        getnstr(g.password, 100);
        noecho();
        int has_upper = 0, has_lower = 0, has_digit = 0, has_special = 0;
        if (strlen(g.password) == 0)
        { 
            generate_random_password(g.password, 12); 
            mvprintw(LINES / 2 + 2, COLS / 2 - 15, "Generated Password: %s", g.password); 
            refresh(); 
            usleep(5000000);
        }
        else{
            has_upper = 0, has_lower = 0, has_digit = 0, has_special = 0;
            for (int i = 0; i < strlen(g.password); i++) {
            if (g.password[i] >= 'A' && g.password[i] <= 'Z')
                has_upper = 1;
            else if (g.password[i] >= 'a' && g.password[i] <= 'z')
                has_lower = 1;
            else if (g.password[i] >= '0' && g.password[i] <= '9')
                has_digit = 1;
            else
                has_special = 1;
            }
        }
        

        if (!has_upper || !has_lower || !has_digit || !has_special || strlen(g.password) < 7) {
            mvprintw(LINES / 2 + 2, COLS / 2 - 30, "ERROR! Password must have upper, lower, digit, special, and 7+ chars.");
            refresh();
            usleep(3000000);
        } else {
            valid = 1;
        }
    }

    // Email
    valid = 0;
    while (!valid) {
        clear();
        draw_menu_border();
        attron(COLOR_PAIR(3));
        mvprintw(LINES / 2 - 2, COLS / 2 - 15, "Enter your email: ");
        attroff(COLOR_PAIR(3));
        echo();
        getnstr(g.email, 50);
        noecho();

        if (!is_valid_email(g.email)) {
            attron(COLOR_PAIR(11));
            mvprintw(LINES / 2 + 2, COLS / 2 - 15, "ERROR! Invalid email format.");
            attroff(COLOR_PAIR(11));
            refresh();
            usleep(3000000);
        } else {
            valid = 1;
        }
    }
    g.score = 0;
    g.gold = 0;
    g.finished_games = 0;
    g.time_first = time(NULL);
    FILE *file = fopen(NAME_FILE, "a");
    if (file) {
        fprintf(file, "%s,%s,%s,%d,%d,%d,%ld\n", g.username, g.password, g.email,g.score,g.gold,g.finished_games,g.time_first);
        fclose(file);
    }

    mvprintw(LINES / 2, COLS / 2 - 15, "User successfully registered!");
    refresh();
    usleep(3000000);
}
// Function to check if the username and email match
int check_user_email(const char* username, const char* email) {
    char stored_username[100], stored_password[100], stored_email[100];
    FILE *file = fopen(NAME_FILE, "r");
    if (!file) {
        return 0;
    }
    char line[101];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';
        sscanf(line, "%[^,],%[^,],%[^,]", stored_username, stored_password, stored_email);
        if (strcmp(g.username, stored_username) == 0 && strcmp(g.email, stored_email) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}
// Function to update the user password
void update_user_password(const char* username, const char* new_password) {
    FILE *file = fopen(NAME_FILE, "r+");
    char line[256], stored_username[100], stored_password[100], stored_email[100];
    
    if (!file) {
        return;
    }

    // Read through the file to find the username
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%[^,],%[^,],%[^,]", stored_username, stored_password, stored_email);
        if (strcmp(g.username, stored_username) == 0) {
            // Move the file pointer back to the beginning of this line
            fseek(file, -strlen(line), SEEK_CUR);
            // Write the updated information to the file
            fprintf(file, "%s,%s,%s\n", stored_username, new_password, stored_email);
            break;
        }
    }

    fclose(file);
}

void forgot_password() { 
    char username[100], email[100], new_password[12 + 1]; 
    int valid = 0; 
    while (!valid) { 
        clear(); 
        draw_menu_border(); 
        attron(COLOR_PAIR(3));
        mvprintw(LINES / 2 - 2, COLS / 2 - 15, "Enter your username: "); 
        attroff(COLOR_PAIR(3));
        echo(); 
        getnstr(g.username, 100); 
        noecho(); 
        attron(COLOR_PAIR(3));
        mvprintw(LINES / 2 - 1, COLS / 2 - 15, "Enter your email: "); 
        attroff(COLOR_PAIR(3));
        echo(); 
        getnstr(g.email, 100); 
        noecho(); 
        if (check_user_email(g.username, g.email)) { 
            attron(COLOR_PAIR(3));
            mvprintw(LINES / 2 - 2, COLS / 2 - 15, "Enter your new password: ");
            attroff(COLOR_PAIR(3));
            echo();
            getnstr(new_password, 12);
            update_user_password(g.username, new_password); 
            clear();
            draw_menu_border(); 
            mvprintw(LINES / 2, COLS / 2 - 20, "New password generated: %s", new_password); 
            refresh(); 
            usleep(3000000); 
            valid = 1; 
        } 
        else { 
            mvprintw(LINES / 2 + 2, COLS / 2 - 15, "ERROR! Username and email do not match."); 
            refresh(); 
            usleep(3000000); 
        } 
    } 
}

int post_login_menu(const char *username){
    const char *options[] = {
        "1. Pre-Game Menu",
        "2. Scoreboard",
        "3. Setting",
        "4. Profile",
        "5. Back to Main Menu",
    };
    int selected = 0;
    int num_options = sizeof(options) / sizeof(options[0]);

    while (1) {
        clear();
        draw_menu_border();
        attron(COLOR_PAIR(12));
        mvprintw(2, COLS / 2 - 10, "Welcome %s!", username);
        attroff(COLOR_PAIR(12));
        for (int i = 0; i < num_options; i++) {
            if (i == selected) attron(A_REVERSE);
            mvprintw(4 + i, COLS / 2 - 15, "%s", options[i]);
            if (i == selected) attroff(A_REVERSE);
        }

        int key = getch();
        switch (key) {
            case KEY_UP: selected = (selected - 1 + num_options) % num_options; break;
            case KEY_DOWN: selected = (selected + 1) % num_options; break;
            case 10:
                switch (selected + 1) {
                    case 1: show_pre_game_menu(g.username); break;
                    case 2: show_scoreboard(NAME_FILE , g.username); break;
                    case 3: show_settings(); break;
                    case 4: show_profile(); break;
                    case 5: show_main_menu(); break;
                }
        }
    }
}

void login_user()
{
    char username[100], password[100];
    char *gus[] = {"guest"};

    int attempts = 3;

    while (attempts > 0)
    {
        clear();
        draw_menu_border();
        attron(COLOR_PAIR(1));
        mvprintw(LINES / 2 - 3, COLS / 2 - 15, "Login Menu");
        attroff(COLOR_PAIR(1));
        mvprintw(LINES / 2 - 2, COLS / 2 - 15, "Enter your username(or type'guest'): ");
        echo();
        getnstr(g.username, 100);

        if (strcmp(g.username, "guest") == 0)
        {
            clear();
            draw_menu_border();
            mvprintw(LINES / 2, COLS / 2 - 15, "Welcome, Guest! Limited access granted.");
            refresh();
            usleep(2000000);
            post_login_menu(*gus);
            return;
        }

        mvprintw(LINES / 2 - 1, COLS / 2 - 15, "Enter your password: ");
        getnstr(g.password, 100);
        noecho();

        if (validate_user_login(g.username, g.password))
        {
            clear();
            draw_menu_border();
            attron(COLOR_PAIR(12));
            mvprintw(LINES / 2, COLS / 2 - 15, "Login successful! Welcome %s", g.username);
            attron(COLOR_PAIR(12));
            refresh();
            usleep(2000000);
            post_login_menu(g.username);
            return;
        }
        else
        {
            attempts--;
            attron(COLOR_PAIR(5));
            mvprintw(LINES / 2 + 2, COLS / 2 - 20, "Invalid username or password. Attempts left: %d", attempts);
            mvprintw(LINES / 2 + 4, COLS / 2 - 20, "Forgot password? Press 'F' to reset.");
            attroff(COLOR_PAIR(5));
            refresh();
            usleep(2000000);

            int ch = getch(); 
            if (ch == 'F' || ch == 'f') { 
                forgot_password(); 
                return;
            }
        }
    }

    clear();
    draw_menu_border();
    mvprintw(LINES / 2, COLS / 2 - 15, "Login failed. Returning to main menu.");
    refresh();
    usleep(2000000);
}

int check_name_exists(const char *username) {
    FILE *file = fopen(NAME_FILE, "r");
    if (!file) {
        return 0;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char file_username[100];
        sscanf(line, "%[^,],", file_username); 
        if (strcmp(file_username, username) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0; 
}


void save_username(const char *username) {
    FILE *file = fopen(NAME_FILE, "a");
    if (file) {
        fprintf(file, "%s\n", g.username);
        fclose(file);
    }
}

int validate_user_login(const char *username, const char *password) {
    FILE *file = fopen(NAME_FILE, "r");
    if (!file) return 0;

    char line[256], file_username[100], file_password[100], file_email[50];
    int file_score , file_gold , file_finished_games ;
    time_t time_first;
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%99[^,],%99[^,],%49[^\n],%d,%d,%d,%ld", file_username, file_password, file_email ,
                                         &file_score , &file_gold , &file_finished_games , &time_first);
        if (strcmp(g.username, file_username) == 0 && strcmp(g.password, file_password) == 0) {
            strcpy(g.email , file_email);
            g.score = g.gold = 0;
            g.finished_games = file_finished_games;
            g.time_first = time_first;
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

int rnd(int range) {
    return rand() % range;
}

void initialize_map() {
    for (int i = 0; i < NUMLINES; i++) {
        for (int j = 0; j < NUMCOLS; j++) {
            chat[i][j] = ' ';
        }
    }
}

void display_map() {
    clear();
    for (int y = 0; y < NUMLINES; y++) {
        for (int x = 0; x < NUMCOLS; x++) {
            if (map_status[y][x]) {
            char ch = chat[y][x]; 
            if (ch == TRAP) { 
                ch = FLOOR;
                if(trap_check[y][x]) ch = TRAP; 
            }
            if (ch == FOOD) {
                attron(COLOR_PAIR(11));
                mvaddch(y, x, ch);
                attroff(COLOR_PAIR(11));
            } else if (ch == MAGIC) {
                attron(COLOR_PAIR(11)); 
                mvaddch(y, x, ch);
                attroff(COLOR_PAIR(11));
            } else if (ch == SUPERIOR) {
                attron(COLOR_PAIR(11)); 
                mvaddch(y, x, ch);
                attroff(COLOR_PAIR(11));
            } 
            else if (ch == ROTTEN) {
                ch == FOOD;
                attron(COLOR_PAIR(11)); 
                mvaddch(y, x, ch);
                attroff(COLOR_PAIR(11));
            }
             else if (ch == STAIRS) {
                attron(COLOR_PAIR(12)); 
                mvaddch(y, x, ch);
                attroff(COLOR_PAIR(12));
            }else if (ch == GOLD_NORMAL) {
                attron(COLOR_PAIR(5));
                mvaddch(y, x, ch);
                attroff(COLOR_PAIR(5));
            }else if (ch == GOLD_ADVANCED) {
                attron(COLOR_PAIR(1)); 
                mvaddch(y, x, ch);
                attroff(COLOR_PAIR(1));
            }else if (ch == 'D') {
                attron(COLOR_PAIR(7)); 
                mvaddch(y, x, ch);
                attroff(COLOR_PAIR(7));
            }else if (ch == '8') {
                attron(COLOR_PAIR(1)); 
                mvaddch(y, x, ch);
                attroff(COLOR_PAIR(1));
            }else if (ch == 'F') {
                attron(COLOR_PAIR(8)); 
                mvaddch(y, x, ch);
                attroff(COLOR_PAIR(8));
            }else if (ch == 'G') {
                attron(COLOR_PAIR(17)); 
                mvaddch(y, x, ch);
                attroff(COLOR_PAIR(17));
            }else if (ch == 'S') {
                attron(COLOR_PAIR(16)); 
                mvaddch(y, x, ch);
                attroff(COLOR_PAIR(16));
            }else if (ch == 'U') {
                attron(COLOR_PAIR(18)); 
                mvaddch(y, x, ch);
                attroff(COLOR_PAIR(18));
            }else if (ch == 'h') {
                attron(COLOR_PAIR(20)); 
                mvaddch(y, x, ch);
                attroff(COLOR_PAIR(20));
            }else if (ch == 's') {
                attron(COLOR_PAIR(20)); 
                mvaddch(y, x, ch);
                attroff(COLOR_PAIR(20));
            }else if (ch == 'd') {
                attron(COLOR_PAIR(20)); 
                mvaddch(y, x, ch);
                attroff(COLOR_PAIR(20));
            }else if (ch == 'M') {
                attron(COLOR_PAIR(19)); 
                mvaddch(y, x, ch);
                attroff(COLOR_PAIR(19));
            }else if (ch == 'K') {
                attron(COLOR_PAIR(19)); 
                mvaddch(y, x, ch);
                attroff(COLOR_PAIR(19));
            }else if (ch == 'A') {
                attron(COLOR_PAIR(19)); 
                mvaddch(y, x, ch);
                attroff(COLOR_PAIR(19));
            }else if (ch == 'N') {
                attron(COLOR_PAIR(19)); 
                mvaddch(y, x, ch);
                attroff(COLOR_PAIR(19));
            }else if (ch == '!') {
                attron(COLOR_PAIR(19)); 
                mvaddch(y, x, ch);
                attroff(COLOR_PAIR(19));
            }else if(treasure_roomm[y][x]){
                if(ch == '|' || ch == '_' || ch == '+' || ch == '?'){
                    attron(COLOR_PAIR(2)); 
                    mvaddch(y, x, ch);
                    attroff(COLOR_PAIR(2));
                }
            } else if(nightmare_room[y][x]){
                if(ch == '|' || ch == '_' || ch == '+' || ch == '?' || ch == '=' ){
                    attron(COLOR_PAIR(21)); 
                    mvaddch(y, x, ch);
                    attroff(COLOR_PAIR(21));
                }
                if( ch == '.' || ch == 'O'){
                    attron(COLOR_PAIR(22)); 
                    mvaddch(y, x, ch);
                    attroff(COLOR_PAIR(22));
                }
            }else if(spell_room[y][x]){
                if(ch == '|' || ch == '_' || ch == '+' || ch == '?' || ch == '=' ){
                    attron(COLOR_PAIR(23)); 
                    mvaddch(y, x, ch);
                    attroff(COLOR_PAIR(23));
                }
                if( ch == '.' || ch == 'O'){
                    attron(COLOR_PAIR(22)); 
                    mvaddch(y, x, ch);
                    attroff(COLOR_PAIR(22));
                }
            }
            else {
                mvaddch(y, x, ch);
            }
            }
        }
    }
    attron(COLOR_PAIR(player_color)); 
    mvaddch(player_pos.y, player_pos.x, PLAYER); 
    attroff(COLOR_PAIR(player_color));
    refresh();
}


void draw_room(struct room *rp) {
    int y, x;

    vert(rp, rp->r_pos.x);                      // Draw left side
    vert(rp, rp->r_pos.x + rp->r_max.x - 1);    // Draw right side
    horiz(rp, rp->r_pos.y);                     // Draw top
    horiz(rp, rp->r_pos.y + rp->r_max.y - 1);   // Draw bottom

    // Put the floor down
    for (y = rp->r_pos.y + 1; y < rp->r_pos.y + rp->r_max.y - 1; y++) {
        for (x = rp->r_pos.x + 1; x < rp->r_pos.x + rp->r_max.x - 1; x++) {
            chat[y][x] = FLOOR;
        }
    }
}

void vert(struct room *rp, int startx) {
    for (int y = rp->r_pos.y + 1; y <= rp->r_max.y + rp->r_pos.y - 1; y++) {
        chat[y][startx] = WALL_VERT;
    }
}

void horiz(struct room *rp, int starty) {
    for (int x = rp->r_pos.x; x <= rp->r_pos.x + rp->r_max.x - 1; x++) {
        chat[starty][x] = WALL_HORZ;
    }
}

bool is_overlap_or_too_close(struct room *r1, struct room *r2) {
    return !(r1->r_pos.x + r1->r_max.x + 5 <= r2->r_pos.x ||
             r2->r_pos.x + r2->r_max.x + 5 <= r1->r_pos.x ||
             r1->r_pos.y + r1->r_max.y + 4 <= r2->r_pos.y ||
             r2->r_pos.y + r2->r_max.y + 4 <= r1->r_pos.y);
}

void random_generate_room() {
    int i, attempts = 0;
    bool overlap_or_too_close;

    for (i = 0; i < MAXROOMS; i++) {
        do {
            overlap_or_too_close = false;
            int width = rnd(NUMCOLS / 8) + 8; 
            width = (width < 8) ? 8 : width;
            int height = rnd(NUMLINES / 8) + 8;
            height = (height < 8) ? 8 : height; 
            int x_pos = rnd(NUMCOLS - width - 1) + 1;
            int y_pos = rnd(NUMLINES - height - 1) + 1;

            rooms[i].r_pos.x = x_pos;
            rooms[i].r_pos.y = y_pos;
            rooms[i].r_max.x = width;
            rooms[i].r_max.y = height;

            for (int j = 0; j < i; j++) {
                if (is_overlap_or_too_close(&rooms[i], &rooms[j])) {
                    overlap_or_too_close = true;
                    break;
                }
            }

            attempts++;
            if (attempts > 1000) {
                break;
            }
        } while (overlap_or_too_close);
        
        if (attempts <= 1000) {
            draw_room(&rooms[i]);
        }
    }
}

// Updated connectivity matrix
struct rdes {
    bool conn[MAXROOMS];
    bool isconn[MAXROOMS];
    bool ingraph; 
} rdes[MAXROOMS] = {
    { { 0, 1, 0, 1, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, 0 },
    { { 1, 0, 1, 0, 1, 0 }, { 0, 0, 0, 0, 0, 0 }, 0 },
    { { 0, 1, 0, 0, 0, 1 }, { 0, 0, 0, 0, 0, 0 }, 0 },
    { { 1, 0, 0, 0, 1, 0 }, { 0, 0, 0, 0, 0, 0 }, 0 },
    { { 0, 1, 0, 1, 0, 1 }, { 0, 0, 0, 0, 0, 0 }, 0 },
    { { 0, 0, 1, 0, 1, 0 }, { 0, 0, 0, 0, 0, 0 }, 0 },
};


void place_door() {
    for (int y = 1; y < NUMLINES - 1; y++) {
        for (int x = 1; x < NUMCOLS - 1; x++) {
            if(chat[y][x] == '_')
            {
                if((chat[y][x - 1] == '|') && (chat[y][x + 1] == '#'))
                {
                    if(chat[y - 1][x] == '_')
                    {
                        chat[y][x] = 's';
                        chat[y - 1][x + 1] = '#';
                    }
                    else if(chat[y + 1][x] == '_')
                    {
                        chat[y][x] = 's';
                        chat[y + 1][x + 1] = '#';
                    }
                }
                else if((chat[y][x + 1] == '|') && (chat[y][x - 1] == '#'))
                {
                    if(chat[y - 1][x] == '_')
                    {
                        chat[y][x] = 's';
                        chat[y - 1][x - 1] = '#';
                    }
                    else if(chat[y + 1][x] == '_')
                    {
                        chat[y][x] = 's';
                        chat[y + 1][x - 1] = '#';
                    }
                }
            }
            // Check horizontal walls
            if (chat[y][x] == WALL_HORZ && 
                (chat[y - 1][x] == '#' || chat[y + 1][x] == '#' || 
                 chat[y][x - 1] == '#' || chat[y][x + 1] == '#')) {
                chat[y][x] = DOOR;
            }

            // Check vertical walls
            if (chat[y][x] == WALL_VERT && 
                (chat[y - 1][x] == '#' || chat[y + 1][x] == '#' || 
                 chat[y][x - 1] == '#' || chat[y][x + 1] == '#')) {
                chat[y][x] = DOOR;
            }

        }
    }
    for (int y = 1; y < NUMLINES - 1; y++) {
        for (int x = 1; x < NUMCOLS - 1; x++) {
            if(chat[y][x] == 's')
            chat[y][x] = '_';
        }
    }

}

void draw_passage(coord start, coord end) {
    coord current = start;
    bool turn_made = false;
    coord turn_point;

    while (current.x != end.x || current.y != end.y) {
        if (!turn_made && rnd(2) == 0) { // Decide to make a turn
            turn_point = current;
            turn_made = true;
        }

        if (turn_made && current.x != turn_point.x) {
            current.x += (end.x > current.x) ? 1 : -1;
        } else if (current.y != end.y) {
            current.y += (end.y > current.y) ? 1 : -1;
        } else {
            current.x += (end.x > current.x) ? 1 : -1;
        }

        if (chat[current.y][current.x] == ' ') {
            chat[current.y][current.x] = PASSAGE;
        }
    }
}
void add_random_passages() {
    for (int i = 0; i < MAXROOMS; i++) {
        for (int j = 0; j < MAXROOMS; j++) {
            if (rdes[i].conn[j] && !rdes[i].isconn[j]) {
                coord start = {rooms[i].r_pos.x + rooms[i].r_max.x / 2, rooms[i].r_pos.y + rooms[i].r_max.y / 2};
                coord end = {rooms[j].r_pos.x + rooms[j].r_max.x / 2, rooms[j].r_pos.y + rooms[j].r_max.y / 2};
                draw_passage(start, end);
                place_door();
                rdes[i].isconn[j] = true;
                rdes[j].isconn[i] = true;
            }
        }
    }
}



void place_random_stairs(int *stairs_x, int *stairs_y) {
    int floor_count = 0;
    for (int y = 0; y < NUMLINES; y++) {
        for (int x = 0; x < NUMCOLS; x++) {
            if (chat[y][x] == FLOOR) {
                floor_count++;
            }
        }
    }
    if (floor_count == 0) {
        return;
    }

    int target_floor = rnd(floor_count);
    floor_count = 0;
    for (int y = 0; y < NUMLINES; y++) {
        for (int x = 0; x < NUMCOLS; x++) {
            if (chat[y][x] == FLOOR && spell_room[y][x]) {
                if (floor_count == target_floor) {
                    chat[y][x] = STAIRS;
                    attron(COLOR_PAIR(12)); 
                    mvaddch(y, x, STAIRS);
                    attroff(COLOR_PAIR(12));
                    *stairs_x = x;
                    *stairs_y = y;
                    return;
                }
                floor_count++;
            }
            else if (chat[y][x] == FLOOR && !spell_room[y][x]) {
                if (floor_count == target_floor) {
                    chat[y][x] = STAIRS;
                    attron(COLOR_PAIR(12)); 
                    mvaddch(y, x, STAIRS);
                    attroff(COLOR_PAIR(12));
                    *stairs_x = x;
                    *stairs_y = y;
                    return;
                }
                floor_count++;
            }
        }
    }
}




void place_random_traps() {
    int trap_count = rnd(2) + 2;

    for (int i = 0; i < trap_count; i++) {
        int x, y;
        do {
            x = rnd(NUMCOLS);
            y = rnd(NUMLINES);
        } while (chat[y][x] != FLOOR); 
        chat[y][x] = TRAP;
    }
}

void place_player() {
    int x, y;
    do {
        x = rnd(NUMCOLS);
        y = rnd(NUMLINES);
    } while (chat[y][x] != FLOOR);

    player_pos.x = x;
    player_pos.y = y;
}


bool ask_player_to_take_weapon(const char *weapon_name) {
    clear();
    mvprintw(2, COLS / 2 - 10, "You found a %s!", weapon_name);
    mvprintw(3, COLS / 2 - 15, "Do you want to take it? (y/n): ");
    refresh();

    int ch = getch();
    return (ch == 'y' || ch == 'Y');
}
bool ask_player_to_take_spell(const char *spell_name) {
    clear();
    mvprintw(2, COLS / 2 - 10, "You found a %s spell!", spell_name);
    mvprintw(3, COLS / 2 - 15, "Do you want to take it? (y/n): ");
    refresh();

    int ch = getch();
    return (ch == 'y' || ch == 'Y');
}

void play_next_music() {
    stop_music();

    static int current_music_index = 0;
    current_music_index = (current_music_index + 1) % num_music_files;
    const char* next_music = music_files[current_music_index];

    if (strlen(next_music) > 0) {
        pthread_t music_tid;
        pthread_create(&music_tid, NULL, music_thread, (void*)next_music);
    }
}


void check_treasure_room() {
    if (current_floor == 4 &&
        player_pos.x >= treasure_room.x && player_pos.x < treasure_room.x + rooms[0].r_max.x &&
        player_pos.y >= treasure_room.y && player_pos.y < treasure_room.y + rooms[0].r_max.y) {
        if (treasure_room_entered == false) {
            display_map();
            show_message_top_right4("Welcome to the Treasure Room! Defeat all monsters to win!");
            treasure_room_entered = true;

            if (Mix_PlayingMusic()) {
                play_next_music();
            }
        }
        display_map();
        
        bool monsters_remaining = false;
        for (int y = treasure_room.y; y < treasure_room.y + rooms[0].r_max.y; y++) {
            for (int x = treasure_room.x; x < treasure_room.x + rooms[0].r_max.x; x++) {
                if (chat[y][x] == 'S' || chat[y][x] == 'G' || chat[y][x] == 'U' || chat[y][x] == 'F' || chat[y][x] == 'D') {
                    monsters_remaining = true;
                    // break;
                }
            }
            if (monsters_remaining) break;
        }
        if (!monsters_remaining) {
            g.score += 100;
            update_user_info(g.username , &g);
            clear();
            stop_music();
            memset(selected_music, 0, sizeof(selected_music));
            mvprintw(LINES / 2, COLS / 2 - 10, "YOU WON!");
            mvprintw(LINES / 2 + 1, COLS / 2 - 10, "Score: %d", g.score);
            int start_y = LINES / 2 - 1;
            int start_x = COLS / 2 - 12;
            int end_y = LINES / 2 + 2;
            int end_x = COLS / 2 + 10;
            for (int x = start_x; x <= end_x; x++) {
                attron(COLOR_PAIR(6));
                mvprintw(start_y, x, "-");
                mvprintw(end_y, x, "-");
                attroff(COLOR_PAIR(6));
            }
            for (int y = start_y; y <= end_y; y++) {
                attron(COLOR_PAIR(6));
                mvprintw(y, start_x, "|");
                mvprintw(y, end_x, "|");
                attroff(COLOR_PAIR(6));
            }

            attron(COLOR_PAIR(6));
            mvprintw(start_y, start_x, "+");
            mvprintw(start_y, end_x, "+");
            mvprintw(end_y, start_x, "+");
            mvprintw(end_y, end_x, "+");
            attroff(COLOR_PAIR(6));


            refresh();
            getch();
            post_login_menu(&g);
        }
    }
    
}


void update_food_inventory() {
    for (int i = 0; i < player.food_count; i++) {
        if (strcmp(player.food_inventory[i].name, "Normal") == 0) {
            strcpy(player.food_inventory[i].name, "Rotten");
            player.food_inventory[i].hunger_reduction = 10;
            player.food_inventory[i].health_reduction = -20;
        } else if (strcmp(player.food_inventory[i].name, "Superior") == 0 ||
                   strcmp(player.food_inventory[i].name, "Magic") == 0) {
            strcpy(player.food_inventory[i].name, "Normal");
            player.food_inventory[i].hunger_reduction = 20;
            player.food_inventory[i].health_reduction = 5;
        }
    }
}

bool has_weapon(char weapon_symbol) {
    for (int i = 0; i < MAX_WEAPONS; i++) {
        if (weapons[i].symbol == weapon_symbol) {
            return weapons[i].count > 0;
        }
    }
    return false;
}

void handle_weapon_pickup(int x, int y, char weapon_symbol) {
    for (int i = 0; i < MAX_WEAPONS; i++) {
        if (weapons[i].symbol == weapon_symbol) {
            if (ask_player_to_take_weapon(weapons[i].name)) {
                if (weapons[i].thrown && chat[y][x] == weapon_symbol) {
                    weapons[i].count += 1;
                } else {
                    switch (weapon_symbol) {
                        case 'K': 
                            weapons[i].count += 10;
                            break;
                        case 'M': 
                            weapons[i].count += 1;
                            break;
                        case 'A':
                            weapons[i].count += 8;
                            break;
                        case 'N':
                            weapons[i].count += 20;
                            break;
                        case '!':
                            weapons[i].count = 100; 
                            break;
                    }
                }
                weapons[i].thrown = false; 
                show_message_top_right("You collected a %s!", weapons[i].name);
                chat[y][x] = FLOOR;
                mvaddch(y, x, FLOOR);
                refresh();
            }
            break;
        }
    }
}


void move_player(int dx, int dy) {
    print_d = 0;
    print_g = 0;
    print_f = 0;
    print_u = 0;
    print_s = 0;
    healthy_up = 0;
    int new_x = player_pos.x + dx * speed_multiplier;
    int new_y = player_pos.y + dy * speed_multiplier;
    if(magic_act == 2){
        new_x = player_pos.x + dx * magic_act;
        new_y = player_pos.y + dy * magic_act;
    }
    if(player.health == 0)
    {
        end_game2();
    }
    if (speed_multiplier == 2) {

        if (dx != 0) {
            if ((new_x - dx * 2 >= 0 && new_x - dx * 2 < NUMCOLS && chat[player_pos.y][new_x - dx * 2] == '#') &&
                (new_x + dx < NUMCOLS && new_x + dx >= 0 && chat[player_pos.y][new_x + dx] != '#')) {
                new_x = player_pos.x + dx;
            }
        }

        if (dy != 0) {
            if ((new_y - dy * 2 >= 0 && new_y - dy * 2 < NUMLINES && chat[new_y - dy * 2][player_pos.x] == '#') &&
                (new_y + dy < NUMLINES && new_y + dy >= 0 && chat[new_y + dy][player_pos.x] != '#')) {
                new_y = player_pos.y + dy;
            }
        }


        if (dx != 0) {
            if ((new_x + dx * 2 < NUMCOLS && new_x + dx * 2 >= 0 && chat[player_pos.y][new_x + dx * 2] == '#') &&
                (new_x - dx < NUMCOLS && new_x - dx >= 0 && chat[player_pos.y][new_x - dx] != '#')) {
                new_x = player_pos.x + dx;
            }
        }
        if (dy != 0) {
            if ((new_y + dy * 2 < NUMLINES && new_y + dy * 2 >= 0 && chat[new_y + dy * 2][player_pos.x] == '#') &&
                (new_y - dy < NUMLINES && new_y - dy >= 0 && chat[new_y - dy][player_pos.x] != '#')) {
                new_y = player_pos.y + dy;
            }
        }
    }

    if (new_x >= 0 && new_x < NUMCOLS && new_y >= 0 && new_y < NUMLINES) {
        char next_tile = chat[new_y][new_x];
        if (next_tile == FLOOR || next_tile == PASSAGE || next_tile == DOOR || next_tile == UNKHOWN_DOOR ) {
            player_pos.x = new_x;
            player_pos.y = new_y;
            update_map_status();
            display_updated_map();
            check_treasure_room();
            display_map();
            bool near_window = is_near_window();
            if (near_window) {
                last_near_window = true;
                reveal_room_through_window(player_pos.x, player_pos.y);
            } else if (last_near_window) {
                last_near_window = false;
                hide_room_through_window();
            }

            move_count++;
            if(magic_act == 2) move_count_magic++;
            if(damage_spells == 2 || speed_multiplier == 2 || health_multiplier == 2) move_count_damage++;
            if(damage_spells == 3) move_count_food++;
            move_monsters();
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    if (i == 0 && j == 0) continue;
                    int check_x = player_pos.x + i;
                    int check_y = player_pos.y + j;
                    if (check_x >= 0 && check_x < NUMCOLS && check_y >= 0 && check_y < NUMLINES) {
                        char tile = chat[check_y][check_x];
                        if (tile == 'D') {
                            player.health -= 1;
                            if (player.health < 0) player.health = 0;
                            print_d = 1;
                        }
                        if(tile == 'F'){
                            player.health -= 2;
                            if(player.health < 0) player.health = 0;
                            print_f = 1;
                        }
                        if (tile == 'S') {
                            player.health -= 4;
                            if (player.health < 0) player.health = 0;
                            print_s = 1;
                        }
                        if (tile == 'G') {
                            player.health -= 3;
                            if (player.health < 0) player.health = 0;
                            print_g = 1;
                        }
                        if (tile == 'U') {
                            player.health -= 5;
                            if (player.health < 0) player.health = 0;
                            print_u = 1;
                        }
                    }
                }
            }

            if(speed_multiplier == 2 || health_multiplier == 2 || damage_spells == 2){
                if(move_count_damage % 10 == 0){
                    speed_multiplier = 1;
                    health_multiplier = 1;
                    damage_spells = 1;
                    show_message_top_right("The effect of the spells has faded!");
                }
            }
            if(damage_spells == 3){
                if(move_count_food % 10 == 0){
                    damage_spells = 1;
                    show_message_top_right("The effect of the superior food has faded!");
                }
            }
            if(magic_act == 2){
                if(move_count_magic % 40 == 0){
                    magic_act = 1;
                }
            }
            if (move_count % 100 == 0) {
                update_food_inventory();
            }
            //increasing hunger
            if (move_count % 5 == 0) {
                player.hunger += 3;
            }
            //decreasing healthy
            if (player.hunger > 200 && move_count % 3 == 0) {
                player.health -= 2;
            }
            //warning
            if (player.health == 40 || player.health == 35 || player.health == 30 || player.health < 20) {
                show_message_top_right("Warning: You need to eat food!");
            }
            //check the variables
            if (player.hunger < 0) player.hunger = 0;
            if (player.hunger > 300) player.hunger = 300;//max value of hungry
            if (player.health < 0) player.health = 0;
            if (player.health > 200) player.health = 200;//base of healty
            if (move_count % 2 == 0) {
                if(player.hunger < 101){
                    player.health += (health_multiplier*5);
                    if (player.health >200) player.health = 200;
                    healthy_up = 1;
                }
            }
        } else if (next_tile == STAIRS) {
            clear();
            mvprintw(LINES / 2, COLS / 2 - 10, "Go up (u) or down (d)?");
            refresh();
            int ch = getch();
            if (ch == 'u') {
                save_current_floor();
                current_floor++;
                if(current_floor == 4)
                {
                    for(int i = 0 ; i < HEIGHT ; i++){
                        for(int j = 0 ; j < WIDTH ; j++){
                            nightmare_room[i][j] = false;
                        }
                    }
                    for(int i = 0 ; i < HEIGHT ; i++){
                        for(int j = 0 ; j < WIDTH ; j++){
                            spell_room[i][j] = false;
                        }
                    }
                }
                initialize_new_floor();
            } else if (ch == 'd') {
                if (current_floor > 1) {
                    save_current_floor();
                    current_floor--;
                    if(current_floor == 1)
                {
                    for(int i = 0 ; i < HEIGHT ; i++){
                        for(int j = 0 ; j < WIDTH ; j++){
                            nightmare_room[i][j] = false;
                        }
                    }
                    for(int i = 0 ; i < HEIGHT ; i++){
                        for(int j = 0 ; j < WIDTH ; j++){
                            spell_room[i][j] = false;
                        }
                    }
                }
                    initialize_previous_floor();
                }
            }
        } else if (next_tile == TRAP) {
            player.health -= 2;
            if (player.health < 0) player.health = 0;
            trap_check[new_y][new_x] = true;
            chat[new_y][new_x] = TRAP; 
            show_message_top_right("Oops!TRAP! Your lost %d healthy", 2);
            chat[new_y][new_x] = '^';
        }  else if (next_tile == '&') {
            display_password_for_30_seconds();
        }  else if (next_tile == '8') {
            ask_for_password();
            chat[new_y][new_x] = '+';
        } else if (next_tile == FOOD) {
            if (ask_player_to_take_food()) {
                collect_food(FOOD_NORMAL); 
                chat[new_y][new_x] = FLOOR;
            }else if (!ask_player_to_take_food()) {
                chat[new_y][new_x] = FLOOR;
            } 
        } else if (next_tile == MAGIC) {
            if (ask_player_to_take_food()) {
                collect_food(FOOD_MAGIC); 
                chat[new_y][new_x] = FLOOR;
            }else if (!ask_player_to_take_food()) {
                chat[new_y][new_x] = FLOOR;
            }
        }else if (next_tile == SUPERIOR) {
            if (ask_player_to_take_food()) {
                collect_food(FOOD_SUPERIOR); 
                chat[new_y][new_x] = FLOOR;
            }else if (!ask_player_to_take_food()) {
                chat[new_y][new_x] = FLOOR;
            }
        }else if (next_tile == ROTTEN) {
            if (ask_player_to_take_food()) {
                collect_food(FOOD_ROTTEN); 
                chat[new_y][new_x] = FLOOR;
            } else if (!ask_player_to_take_food()) {
                chat[new_y][new_x] = FLOOR;
            } 
        }else if (next_tile == GOLD_NORMAL) {
            if (check_nightmare_room(player_pos.x, player_pos.y)) {
            show_message_top_right("This gold is fake!");
            chat[new_y][new_x] = FLOOR;
            }else{
                g.gold++;
                g.score += 10; 
                show_message_top_right("Normal gold! your got %d score", 10);
                chat[new_y][new_x] = FLOOR; 
            }
        } else if (next_tile == GOLD_ADVANCED) {
            if (check_nightmare_room(player_pos.x, player_pos.y)) {
            show_message_top_right("This gold is fake!");
            chat[new_y][new_x] = FLOOR;
            }else{
                g.gold++;
                g.score += 20; 
                show_message_top_right("Normal gold! your got %d score", 20);
                chat[new_y][new_x] = FLOOR; 
            }
        } else if (chat[new_y][new_x] == 'K' || chat[new_y][new_x] == '!' || chat[new_y][new_x] == 'M' || 
                    chat[new_y][new_x] == 'A'|| chat[new_y][new_x] == 'N') {
            handle_weapon_pickup(new_x, new_y, chat[new_y][new_x]);
        } else{
            for (int i = 0; i < MAX_SPELLS; i++) {
            if(check_spell_room(player_pos.x, player_pos.y)){
                show_message_top_right("This spell is fake!");
                show_message_top_right("Your health is in danger!GO OUT");
                player.health -= 3;
            }else if(!check_spell_room(player_pos.x, player_pos.y)){
                if (next_tile == spells[i].symbol) {
                    if (ask_player_to_take_spell(spells[i].name)) {
                        spells[i].count++;
                        show_message_top_right("You collected a %s spell!", spells[i].name);
                        chat[new_y][new_x] = FLOOR; 
                    }
                    break;
                }
            }
            }
        }
        if(check_spell_room(player_pos.x, player_pos.y) && change_music){
                if (Mix_PlayingMusic()) {
                play_next_music();
                }
                change_music = false;
            }
            if(check_nightmare_room(player_pos.x, player_pos.y) && change_music2){
                if (Mix_PlayingMusic()) {
                play_next_music();
                }
                change_music2 = false;
            }
        for (int i = 0; i < hidden_door_count; i++) {
            int hx = hidden_doors[i].x;
            int hy = hidden_doors[i].y;
            if ((new_x == hx && abs(new_y - hy) == 1) || (new_y == hy && abs(new_x - hx) == 1)) {
                chat[hy][hx] = '?';
                mvaddch(hy, hx, '?');
                refresh();
            }
        }
    }
}

void initialize_rdes() {
    for (int i = 0; i < MAXROOMS; i++) {
        for (int j = 0; j < MAXROOMS; j++) {
            rdes[i].conn[j] = false;
            rdes[i].isconn[j] = false;
        }
        rdes[i].ingraph = false;
    }

    rdes[0].conn[1] = true;
    rdes[0].conn[3] = true;
    rdes[1].conn[0] = true;
    rdes[1].conn[2] = true;
    rdes[1].conn[4] = true;
    rdes[2].conn[1] = true;
    rdes[2].conn[5] = true;
    rdes[3].conn[0] = true;
    rdes[3].conn[4] = true;
    rdes[4].conn[1] = true;
    rdes[4].conn[3] = true;
    rdes[4].conn[5] = true;
    rdes[5].conn[2] = true;
    rdes[5].conn[4] = true;
}

void treas_room() {
    int treasure_room_index = rnd(MAXROOMS);
    struct room *rp = &rooms[treasure_room_index];
    treasure_room = rp->r_pos;


    for (int y = rp->r_pos.y; y < rp->r_pos.y + rp->r_max.y; y++) {
        for (int x = rp->r_pos.x; x < rp->r_pos.x + rp->r_max.x; x++) {
            if (chat[y][x] == WALL_HORZ || chat[y][x] == WALL_VERT || chat[y][x] == '+' || chat[y][x] == '?') {
                treasure_roomm[y][x] = true;
            }
        }
    }
    int trap_count = rnd(8) + 5;

    for (int i = 0; i < trap_count; i++) {
        int x, y;
        do {
            x = rnd(rp->r_max.x) + rp->r_pos.x;
            y = rnd(rp->r_max.y) + rp->r_pos.y;
        } while (chat[y][x] != FLOOR);
        chat[y][x] = TRAP;
    }


    int num_items = rnd(8) + 5;
    for (int i = 0; i < num_items; i++) {
        int x, y;
        do {
            x = rnd(rp->r_max.x) + rp->r_pos.x;
            y = rnd(rp->r_max.y) + rp->r_pos.y;
        } while (chat[y][x] != FLOOR);
        chat[y][x] = GOLD_NORMAL;
    }

    int daemons_count = rnd(3) + 1;
    for (int i = 0; i < daemons_count; i++) {
        int x, y;
        do {
            x = rnd(rp->r_max.x) + rp->r_pos.x;
            y = rnd(rp->r_max.y) + rp->r_pos.y;
        } while (chat[y][x] != FLOOR);

        chat[y][x] = 'D';
        attron(COLOR_PAIR(9)); 
        mvaddch(y, x, 'D');
        attroff(COLOR_PAIR(9));
        for (int j = 0; j < MAX_MONSTERS; j++) {
            if (!daemons[treasure_room_index][j].alive) {
                daemons[treasure_room_index][j].pos.x = x;
                daemons[treasure_room_index][j].pos.y = y;
                daemons[treasure_room_index][j].health = 5; 
                daemons[treasure_room_index][j].alive = true;
                daemons[treasure_room_index][j].stunned = false;

                break;
            }
        }
    }
    int giant_count = rnd(3) + 1;
    for (int i = 0; i < giant_count; i++) {
        int x, y;
        do {
            x = rnd(rp->r_max.x) + rp->r_pos.x;
            y = rnd(rp->r_max.y) + rp->r_pos.y;
        } while (chat[y][x] != FLOOR);

        chat[y][x] = 'G'; 
        attron(COLOR_PAIR(9));
        mvaddch(y, x, 'G');
        attroff(COLOR_PAIR(9));
        for (int j = 0; j < MAX_MONSTERS; j++) {
            if (!giant[treasure_room_index][j].alive) {
                giant[treasure_room_index][j].pos.x = x;
                giant[treasure_room_index][j].pos.y = y;
                giant[treasure_room_index][j].health = 15; 
                giant[treasure_room_index][j].alive = true;
                giant[treasure_room_index][j].stunned = false;
                giant[treasure_room_index][j].check_move = false;

                break;
            }
        }
    }
    int snake_count = rnd(3) + 1;
    for (int i = 0; i < snake_count; i++) {
        int x, y;
        do {
            x = rnd(rp->r_max.x) + rp->r_pos.x;
            y = rnd(rp->r_max.y) + rp->r_pos.y;
        } while (chat[y][x] != FLOOR);

        chat[y][x] = 'S'; 
        attron(COLOR_PAIR(9));
        mvaddch(y, x, 'S');
        attroff(COLOR_PAIR(9));
        for (int j = 0; j < MAX_MONSTERS; j++) {
            if (!snake[treasure_room_index][j].alive) {
                snake[treasure_room_index][j].pos.x = x;
                snake[treasure_room_index][j].pos.y = y;
                snake[treasure_room_index][j].health = 20; 
                snake[treasure_room_index][j].alive = true;
                snake[treasure_room_index][j].stunned = false;

                break;
            }
        }
    }
    int undeed_count = rnd(4) + 1;
    for (int i = 0; i < undeed_count; i++) {
        int x, y;
        do {
            x = rnd(rp->r_max.x) + rp->r_pos.x;
            y = rnd(rp->r_max.y) + rp->r_pos.y;
        } while (chat[y][x] != FLOOR);

        chat[y][x] = 'U';
        attron(COLOR_PAIR(9));
        mvaddch(y, x, 'U');
        attroff(COLOR_PAIR(9));
        for (int j = 0; j < MAX_MONSTERS; j++) {
            if (!undeed[treasure_room_index][j].alive) {
                undeed[treasure_room_index][j].pos.x = x;
                undeed[treasure_room_index][j].pos.y = y;
                undeed[treasure_room_index][j].health = 30; 
                undeed[treasure_room_index][j].alive = true;
                undeed[treasure_room_index][j].stunned = false;
                undeed[treasure_room_index][j].check_move = false;

                break;
            }
        }
    }
    int fbms_count = rnd(4) + 1;
    for (int i = 0; i < fbms_count; i++) {
        int x, y;
        do {
            x = rnd(rp->r_max.x) + rp->r_pos.x;
            y = rnd(rp->r_max.y) + rp->r_pos.y;
        } while (chat[y][x] != FLOOR);

        chat[y][x] = 'F';
        attron(COLOR_PAIR(9));
        mvaddch(y, x, 'F');
        attroff(COLOR_PAIR(9));
        for (int j = 0; j < MAX_MONSTERS; j++) {
            if (!fbms[treasure_room_index][j].alive) {
                fbms[treasure_room_index][j].pos.x = x;
                fbms[treasure_room_index][j].pos.y = y;
                fbms[treasure_room_index][j].health = 10; 
                fbms[treasure_room_index][j].alive = true;
                fbms[treasure_room_index][j].stunned = false;

                break;
            }
        }
    }
}

void place_player_near_stairs(int stairs_x, int stairs_y) {
    int x, y;
    do {
        x = stairs_x + (rnd(3) - 1);
        y = stairs_y + (rnd(3) - 1);
    } while (x < 0 || x >= NUMCOLS || y < 0 || y >= NUMLINES || chat[y][x] != FLOOR);

    player_pos.x = x;
    player_pos.y = y;
}

void initialize_new_floor() {
    int stairs_x , stairs_y;
    for(int i = 0 ; i < HEIGHT ; i++){
        for(int j = 0 ; j < WIDTH ; j++){
            nightmare_room[i][j] = false;
        }
    }
    for(int i = 0 ; i < HEIGHT ; i++){
        for(int j = 0 ; j < WIDTH ; j++){
            spell_room[i][j] = false;
        }
    }
    initialize_rdes();
    initialize_map();
    random_generate_room();
    add_random_passages();
    if(current_floor == 3 || current_floor == 2){
        create_spell_room();
        create_nightmare_room();
    }
    place_random_stairs(&stairs_x, &stairs_y);
    place_random_traps();
    place_random_food();
    place_random_gold();
    place_random_gold2();
    check_and_convert_doors();
    lock_door_and_update_room();
    update_corner_of_room_with_locked_door();
    place_random_weapons();
    place_random_spells();
    place_random_windows();

    place_player_near_stairs(stairs_x, stairs_y);

    
    if (current_floor == 4) {
        treas_room();
    }
    place_random_monsters();
    initialize_map_status();
    display_map();
    update_map_status();
    display_updated_map();

}
void show_message_top_right(const char* format, ...) {
    va_list args;
    va_start(args, format);
    move(0, COLS - 40);
    vw_printw(stdscr, format, args);
    va_end(args);
    refresh();
    getch();
}
void show_message_top_right2(const char* format, ...) {
    va_list args;
    va_start(args, format);
    move(1, COLS - 50);
    vw_printw(stdscr, format, args);
    va_end(args);
    refresh();
    getch();
}
void show_message_top_right3(const char* format, ...) {
    va_list args;
    va_start(args, format);
    move(2, COLS - 50);
    vw_printw(stdscr, format, args);
    va_end(args);
    refresh();
    getch();
}
void show_message_top_right4(const char* format, ...) {
    va_list args;
    va_start(args, format);
    move(3, COLS - 60);
    vw_printw(stdscr, format, args);
    va_end(args);
    refresh();
    getch();
}
void show_message_top_right5(const char* format, ...) {
    va_list args;
    va_start(args, format);
    move(LINES - 3, COLS - 50);
    vw_printw(stdscr, format, args);
    va_end(args);
    refresh();
    getch();
}
void show_message_top_right6(const char* format, ...) {
    va_list args;
    va_start(args, format);
    move(5, COLS - 60);
    vw_printw(stdscr, format, args);
    va_end(args);
    refresh();
    getch();
}
void show_message_top_right7(const char* format, ...) {
    va_list args;
    va_start(args, format);
    move(6, COLS - 60);
    vw_printw(stdscr, format, args);
    va_end(args);
    refresh();
    getch();
}
void show_message_top_right8(const char* format, ...) {
    va_list args;
    va_start(args, format);
    move(7, COLS - 60);
    vw_printw(stdscr, format, args);
    va_end(args);
    refresh();
    getch();
}
void show_message_top_right9(const char* format, ...) {
    va_list args;
    va_start(args, format);
    move(8, COLS - 40);
    vw_printw(stdscr, format, args);
    va_end(args);
    refresh();
    getch();
}

void collect_food(int food_type) {
    if (player.food_count < MAX_FOOD_ITEMS) {
        strcpy(player.food_inventory[player.food_count].name, food_types[food_type]);
        player.food_inventory[player.food_count].hunger_reduction = food_hunger_reduction[food_type];
        player.food_inventory[player.food_count].health_reduction = food_health_reduction[food_type];
        player.food_count++;
        show_message_top_right("You collected %s food.", food_types[food_type]);
    } else {
        show_message_top_right("Your inventory is full. Can't carry more food.");
    }
}

void show_food_menu() {
    clear();
    mvprintw(2, COLS / 2 - 10, "Food Menu");
    mvprintw(3, COLS / 2 - 15, "Hunger: %d, Health: %d", player.hunger,player.health);
    for (int i = 0; i < player.food_count; i++) {
        char* food_name = player.food_type[i] == FOOD_ROTTEN ? "Rotten" : player.food_inventory[i].name;
        mvprintw(4 + i, COLS / 2 - 20, "%d. %s (Hunger Reduction: %d, Health Impact: %d)",
                 i + 1, food_name,
                 player.food_inventory[i].hunger_reduction,
                 player.food_inventory[i].health_reduction);
    }
    mvprintw(LINES - 2, COLS / 2 - 20, "Press number to consume food, Q to exit");
    refresh();

    int ch = getch();
    if (ch >= '1' && ch <= '5') {
        int food_index = ch - '1';
        if (food_index < player.food_count) {
            consume_food(food_index);
        }
    } else if (ch == 'Q' || ch == 'q') {
        return;
    }
}
void consume_food(int index) {
    if (index >= 0 && index < player.food_count) {
        player.hunger -= player.food_inventory[index].hunger_reduction;
        player.health += player.food_inventory[index].health_reduction;

        if (player.hunger < 0) player.hunger = 0;
        if (player.hunger > 300) player.hunger = 300;
        if (player.health > 200) player.health = 200;

        show_message_top_right("You consumed %s food.", player.food_inventory[index].name);

        if (player.food_type[index] == FOOD_ROTTEN) {
            player.health += food_health_reduction[FOOD_ROTTEN];
            show_message_top_right2("The food was Rotten! You lost %d health.", food_health_reduction[FOOD_ROTTEN]);
        }

        for (int i = index; i < player.food_count - 1; i++) {
            player.food_inventory[i] = player.food_inventory[i + 1];
        }
        player.food_count--;

        if(strcmp(player.food_inventory[index].name , "Magic")== 0){
            magic_act = 2; 
        }
        if(strcmp(player.food_inventory[index].name , "Superior")== 0){
            damage_spells = 3;
        }
    }
}
void place_random_gold() {
    int normal_gold_count = rnd(5) + 10; 
    for (int i = 0; i < normal_gold_count; i++) {
        int x, y;
        do {
            x = rnd(NUMCOLS);
            y = rnd(NUMLINES);
        } while (chat[y][x] != FLOOR);
        chat[y][x] = GOLD_NORMAL;
        attron(COLOR_PAIR(5));
        mvaddch(y, x, GOLD_NORMAL);
        attroff(COLOR_PAIR(5));
    }
}

void place_random_gold2() {
    int advanced_gold_count = (rnd(5) + 10)/4;
    
    for (int i = 0; i < advanced_gold_count; i++) {
        int x, y;
        do {
            x = rnd(NUMCOLS);
            y = rnd(NUMLINES);
        } while (chat[y][x] != FLOOR);
        chat[y][x] = GOLD_ADVANCED;
        attron(COLOR_PAIR(1));
        mvaddch(y, x, GOLD_ADVANCED);
        attroff(COLOR_PAIR(1));
    }
}

bool ask_player_to_take_food() {
    clear();
    mvprintw(2, COLS / 2 - 10, "You found some food!");
    mvprintw(3, COLS / 2 - 15, "Do you want to take it? (y/n): ");
    refresh();

    int ch = getch();
    if(ch == 'y' || ch == 'Y');
    {
        if (check_nightmare_room(player_pos.x, player_pos.y)) {
            show_message_top_right("This food is useless!");
            return false;
        }
        return true;
    }
    return false;
}




void check_and_convert_doors() {
    for (int i = 0; i < MAXROOMS; i++) {
        int window_count = 0;
        coord windows[3];

        for (int y = rooms[i].r_pos.y; y < rooms[i].r_pos.y + rooms[i].r_max.y; y++) {
            for (int x = rooms[i].r_pos.x; x < rooms[i].r_pos.x + rooms[i].r_max.x; x++) {
                if (chat[y][x] == '=') {
                    if (window_count < 4) {
                        windows[window_count].x = x;
                        windows[window_count].y = y;
                    }
                    window_count++;
                }
            }
        }

    }
    for (int i = 0; i < MAXROOMS; i++) {
        int door_count = 0;
        coord doors[4];

        for (int y = rooms[i].r_pos.y; y < rooms[i].r_pos.y + rooms[i].r_max.y; y++) {
            for (int x = rooms[i].r_pos.x; x < rooms[i].r_pos.x + rooms[i].r_max.x; x++) {
                if (chat[y][x] == DOOR) {
                    if (door_count < 4) {
                        doors[door_count].x = x;
                        doors[door_count].y = y;
                    }
                    door_count++;
                }
            }
        }

        if (door_count == 2 && hidden_door_count < MAX_HIDDEN_DOORS) {
            if (chat[doors[1].y][doors[1].x - 1] == WALL_HORZ || chat[doors[1].y][doors[1].x + 1] == WALL_HORZ) {
                chat[doors[1].y][doors[1].x] = WALL_HORZ;
            } else if (chat[doors[1].y - 1][doors[1].x] == WALL_VERT || chat[doors[1].y + 1][doors[1].x] == WALL_VERT) {
                chat[doors[1].y][doors[1].x] = WALL_VERT;
            }
            hidden_doors[hidden_door_count++] = doors[1];
        }
    }
}

void display_status() {
    mvprintw(1, COLS - 20, "Score: %d", g.score);
    mvprintw(2, COLS - 20, "Gold: %d", g.gold);
    refresh();
}

void place_random_weapons() {
    int weapon_counts[MAX_WEAPONS] = { 0, rnd(5), rnd(5), rnd(5), rnd(5) };
    if(difficulty == 0){
        weapon_counts[MAX_WEAPONS] = rnd(5); 
    }
    else if(difficulty == 1){
        weapon_counts[MAX_WEAPONS] = rnd(3);
    }
    else if(difficulty == 2){
        weapon_counts[MAX_WEAPONS] = rnd(2);
    }

    for (int i = 0; i < MAX_WEAPONS; i++) {
        if (weapons[i].symbol == 'M') {
            continue;
        }
        for (int j = 0; j < weapon_counts[i]; j++) {
            int x, y;
            do {
                x = rnd(NUMCOLS);
                y = rnd(NUMLINES);
            } while (chat[y][x] != FLOOR);

            chat[y][x] = weapons[i].symbol;
        }
    }
}

void show_inventory() {
    clear();
    int line = 2;

    mvprintw(line++, COLS / 2 - 10, "Inventory");

    mvprintw(line++, COLS / 2 - 20, "Melee Weapons:");
    for (int i = 0; i < MAX_WEAPONS; i++) {
        if (!weapons[i].is_ranged) {
            mvprintw(line++, COLS / 2 - 20, "%c: %s (%d) - Damage: %d, Range: %d %s",
                     weapons[i].symbol, weapons[i].name, weapons[i].count,
                     weapons[i].damage, weapons[i].range,
                     player_weapon == &weapons[i] ? "*" : "");
        }
    }

    line++;

    mvprintw(line++, COLS / 2 - 20, "Ranged Weapons:");
    for (int i = 0; i < MAX_WEAPONS; i++) {
        if (weapons[i].is_ranged) {
            mvprintw(line++, COLS / 2 - 20, "%c: %s (%d) - Damage: %d, Range: %d %s",
                     weapons[i].symbol, weapons[i].name, weapons[i].count,
                     weapons[i].damage, weapons[i].range,
                     player_weapon == &weapons[i] ? "*" : "");
        }
    }

    mvprintw(line++, COLS / 2 - 25, "Press 'w' to empty your hand, or press the weapon's first letter to change weapon.");
    mvprintw(LINES - 2, COLS / 2 - 20, "Press any other key to exit");

    int ch = getch();
    if (ch == 'w') {
        clear();
        player_weapon = NULL;
        mvprintw(LINES / 2, COLS/2 - 20, "You have emptied your hand");
        refresh();
        getch();
        show_inventory();
    } else if(player_weapon == NULL){
        clear();
        if(ch == 'M' || ch == 'm')
        {
            if(weapons[0].count > 0){
                player_weapon = &weapons[0];
                mvprintw(LINES / 2, COLS / 2 - 20, "You have %s in your hand!", weapons[0].name);
            }else if(weapons[0].count == 0){
                mvprintw(LINES / 2, COLS / 2 - 20, "Oh! You don't have any %s!", weapons[0].name);
                getch();
                return;
            }
        }
        else if(ch == 'k' || ch == 'K')
        {
            if(weapons[1].count > 0){
                player_weapon = &weapons[1];
                mvprintw(LINES / 2, COLS / 2 - 20, "You have %s in your hand!", weapons[1].name);
            }else if(weapons[1].count == 0){
                mvprintw(LINES / 2, COLS / 2 - 20, "Oh! You don't have any %s!", weapons[1].name);
                getch();
                return;
            }
        }
        else if(ch == 'A' || ch == 'a')
        {
            if(weapons[2].count > 0){
                player_weapon = &weapons[2];
                mvprintw(LINES / 2, COLS / 2 - 20, "You have %s in your hand!", weapons[2].name);
            }else if(weapons[2].count == 0){
                mvprintw(LINES / 2, COLS / 2 - 20, "Oh! You don't have any %s!", weapons[2].name);
                getch();
                return;
            }
        }
        else if(ch == 'N' || ch == 'n')
        {
            if(weapons[3].count > 0){
                player_weapon = &weapons[3];
                mvprintw(LINES / 2, COLS / 2 - 20, "You have %s in your hand!", weapons[3].name);
            }else if(weapons[4].count == 0){
                mvprintw(LINES / 2, COLS / 2 - 20, "Oh! You don't have any %s!", weapons[3].name);
                getch();
                return;
            }
        }
        else if(ch == 'S' || ch == 's' )
        {
            if(weapons[4].count > 0){
                player_weapon = &weapons[4];
                mvprintw(LINES / 2, COLS / 2 - 20, "You have %s in your hand!", weapons[4].name);
            }else if(weapons[5].count == 0){
                mvprintw(LINES / 2, COLS / 2 - 20, "Oh! You don't have any %s!", weapons[4].name);
                getch();
                return;
            }
        }else {
            clear();
            return;
        }
        refresh();
        getch();
        show_inventory();
    }else if(player_weapon != NULL){
        if(ch == 's' || ch == 'n' || ch == 'a' || ch == 'k' || ch == 'm'){
            clear();
            mvprintw(LINES / 2 , COLS / 2 - 20, "You have some weapons in your hand!");
            refresh();
            getch();
        }   
    }else if(ch == ' ') {
        clear();
        return;
    }else{
        clear();
        mvprintw(LINES / 2 , COLS / 2 - 20, "The weapon with that name doesn't find!");
        refresh();
        getch();
    }
}


void place_random_spells() {
    int spell_counts[MAX_SPELLS] = { rnd(3), rnd(3), rnd(3) };

    for (int i = 0; i < MAX_SPELLS; i++) {
        for (int j = 0; j < spell_counts[i]; j++) {
            int x, y;
            do {
                x = rnd(NUMCOLS);
                y = rnd(NUMLINES);
            } while (chat[y][x] != FLOOR);

            chat[y][x] = spells[i].symbol;
        }
    }
}

void attack_with_weapon() {
    int dx = 0, dy = 0;
    if (player_weapon->is_ranged) {

        for (int i = 1; i <= player_weapon->range; i++) {
            int target_x = player_pos.x + i * dx;
            int target_y = player_pos.y + i * dy;
            char tile = chat[target_y][target_x];

            if (tile == WALL_HORZ || tile == WALL_VERT || tile == DOOR ) {
                break;
            }

            if (tile == ENEMY) {
                attack_enemy(target_x, target_y, player_weapon->damage);
                break;
            }
        }
    } else {

        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue;

                int target_x = player_pos.x + dx;
                int target_y = player_pos.y + dy;
                char tile = chat[target_y][target_x];

                if (tile == ENEMY) {
                    attack_enemy(target_x, target_y, player_weapon->damage);
                }
            }
        }
    }
}

void attack_enemy(int x, int y, int damage) {

    mvprintw(0, 0, "You attacked the enemy for %d damage", damage);
    refresh();
}

void place_random_monsters() {
    for (int i = 0; i < MAXROOMS; i++) {
        int daemon_count = rnd(2); 
        for (int j = 0; j < daemon_count; j++) {
            int x, y;
            do {
                x = rnd(rooms[i].r_max.x) + rooms[i].r_pos.x;
                y = rnd(rooms[i].r_max.y) + rooms[i].r_pos.y;
            } while (chat[y][x] != FLOOR);
            if(!spell_room[y][x]){
            daemons[i][j].pos.x = x;
            daemons[i][j].pos.y = y;
            daemons[i][j].health = 5; 
            daemons[i][j].alive = true;
            chat[y][x] = 'D'; 
            attron(COLOR_PAIR(7)); 
            mvaddch(y, x, 'D');
            attroff(COLOR_PAIR(7));
            }
        }

        int fbm_count = rnd(2); 
        for (int j = 0; j < fbm_count; j++) {
            int x, y;
            do {
                x = rnd(rooms[i].r_max.x) + rooms[i].r_pos.x;
                y = rnd(rooms[i].r_max.y) + rooms[i].r_pos.y;
            } while (chat[y][x] != FLOOR);
            if(!spell_room[y][x]){
            fbms[i][j].pos.x = x;
            fbms[i][j].pos.y = y;
            fbms[i][j].health = 10; 
            fbms[i][j].alive = true;
            chat[y][x] = 'F';
            attron(COLOR_PAIR(8));
            mvaddch(y, x, 'F');
            attroff(COLOR_PAIR(8));
            }
        }
        int giant_count = rnd(2); 
        for (int j = 0; j < giant_count; j++) {
            int x, y;
            do {
                x = rnd(rooms[i].r_max.x) + rooms[i].r_pos.x;
                y = rnd(rooms[i].r_max.y) + rooms[i].r_pos.y;
            } while (chat[y][x] != FLOOR);
            if(!spell_room[y][x]){
            giant[i][j].pos.x = x;
            giant[i][j].pos.y = y;
            giant[i][j].health = 15;
            giant[i][j].alive = true;
            chat[y][x] = 'G'; 
            attron(COLOR_PAIR(17));
            mvaddch(y, x, 'G');
            attroff(COLOR_PAIR(17));
            }
        }
        int snake_count = rnd(2);
        for (int j = 0; j < snake_count; j++) {
            int x, y;
            do {
                x = rnd(rooms[i].r_max.x) + rooms[i].r_pos.x;
                y = rnd(rooms[i].r_max.y) + rooms[i].r_pos.y;
            } while (chat[y][x] != FLOOR);
            if(!spell_room[y][x]){
            snake[i][j].pos.x = x;
            snake[i][j].pos.y = y;
            snake[i][j].health = 20;
            snake[i][j].alive = true;
            chat[y][x] = 'S'; 
            attron(COLOR_PAIR(16)); 
            mvaddch(y, x, 'S');
            attroff(COLOR_PAIR(16));
            }
        }
        int undeed_count = rnd(2); 
        for (int j = 0; j < undeed_count; j++) {
            int x, y;
            do {
                x = rnd(rooms[i].r_max.x) + rooms[i].r_pos.x;
                y = rnd(rooms[i].r_max.y) + rooms[i].r_pos.y;
            } while (chat[y][x] != FLOOR);
            if(!spell_room[y][x]){
            undeed[i][j].pos.x = x;
            undeed[i][j].pos.y = y;
            undeed[i][j].health = 30; 
            undeed[i][j].alive = true;
            chat[y][x] = 'U'; 
            attron(COLOR_PAIR(18)); 
            mvaddch(y, x, 'U');
            attroff(COLOR_PAIR(18));
            }
        }
    }
}


void move_monsters() {
    for (int i = 0; i < MAXROOMS; i++) {
        for (int j = 0; j < MAX_MONSTERS; j++) {
            if (daemons[i][j].alive && !daemons[i][j].stunned) {
                int dx = 0, dy = 0;
                int distance_x = abs(daemons[i][j].pos.x - player_pos.x);
                int distance_y = abs(daemons[i][j].pos.y - player_pos.y);

                if (distance_x <= 3 && distance_y <= 3) {
                    if (daemons[i][j].pos.x < player_pos.x) dx = 1;
                    else if (daemons[i][j].pos.x > player_pos.x) dx = -1;

                    if (daemons[i][j].pos.y < player_pos.y) dy = 1;
                    else if (daemons[i][j].pos.y > player_pos.y) dy = -1;

                    if (distance_x <= 1 && distance_y <= 1) {
                        if (daemons[i][j].pos.x != player_pos.x) dx = 0;
                        if (daemons[i][j].pos.y != player_pos.y) dy = 0;
                    }

                    int new_x = daemons[i][j].pos.x + dx;
                    int new_y = daemons[i][j].pos.y + dy;

                    if (chat[new_y][new_x] == FLOOR || chat[new_y][new_x] == PLAYER) {
                        chat[daemons[i][j].pos.y][daemons[i][j].pos.x] = FLOOR;
                        daemons[i][j].pos.x = new_x;
                        daemons[i][j].pos.y = new_y;
                        chat[new_y][new_x] = 'D';
                        attron(COLOR_PAIR(7)); 
                        mvaddch(new_y, new_x, 'D');
                        attroff(COLOR_PAIR(7));
                    }
                    else if(chat[new_y][new_x] == TRAP){
                        chat[daemons[i][j].pos.y][daemons[i][j].pos.x] = TRAP;
                        daemons[i][j].pos.x = new_x;
                        daemons[i][j].pos.y = new_y;
                        chat[new_y][new_x] = 'D';
                        attron(COLOR_PAIR(7)); 
                        mvaddch(new_y, new_x, 'D');
                        attroff(COLOR_PAIR(7));
                    }
                }
            }
            if (fbms[i][j].alive && !fbms[i][j].stunned) {
                int dx = 0, dy = 0;
                int distance_x = abs(fbms[i][j].pos.x - player_pos.x);
                int distance_y = abs(fbms[i][j].pos.y - player_pos.y);

                if (distance_x <= 4 && distance_y <= 4) {
                    if (fbms[i][j].pos.x < player_pos.x) dx = 1;
                    else if (fbms[i][j].pos.x > player_pos.x) dx = -1;

                    if (fbms[i][j].pos.y < player_pos.y) dy = 1;
                    else if (fbms[i][j].pos.y > player_pos.y) dy = -1;

                    if (distance_x <= 1 && distance_y <= 1) {
                        if (fbms[i][j].pos.x != player_pos.x) dx = 0;
                        if (fbms[i][j].pos.y != player_pos.y) dy = 0;
                    }

                    int new_x = fbms[i][j].pos.x + dx;
                    int new_y = fbms[i][j].pos.y + dy;

                    if (chat[new_y][new_x] == FLOOR || chat[new_y][new_x] == PLAYER ) {
                        chat[fbms[i][j].pos.y][fbms[i][j].pos.x] = FLOOR;
                        fbms[i][j].pos.x = new_x;
                        fbms[i][j].pos.y = new_y;
                        chat[new_y][new_x] = 'F';
                        attron(COLOR_PAIR(8)); 
                        mvaddch(new_y, new_x, 'F');
                        attroff(COLOR_PAIR(8));
                    }
                    else if(chat[new_y][new_x] == TRAP){
                        chat[fbms[i][j].pos.y][fbms[i][j].pos.x] = TRAP;
                        fbms[i][j].pos.x = new_x;
                        fbms[i][j].pos.y = new_y;
                        chat[new_y][new_x] = 'F';
                        attron(COLOR_PAIR(7)); 
                        mvaddch(new_y, new_x, 'F');
                        attroff(COLOR_PAIR(7));
                    }
                }
            }
            if (undeed[i][j].alive && !undeed[i][j].stunned && !undeed[i][j].check_move) {

                int dx = 0, dy = 0;
                int distance_x = abs(undeed[i][j].pos.x - player_pos.x);
                int distance_y = abs(undeed[i][j].pos.y - player_pos.y);

                if (distance_x <= 6 && distance_y <= 6) {
                    if (undeed[i][j].pos.x < player_pos.x) dx = 1;
                    else if (undeed[i][j].pos.x > player_pos.x) dx = -1;

                    if (undeed[i][j].pos.y < player_pos.y) dy = 1;
                    else if (undeed[i][j].pos.y > player_pos.y) dy = -1;

                    if (distance_x <= 1 && distance_y <= 1) {
                        if (undeed[i][j].pos.x != player_pos.x) dx = 0;
                        if (undeed[i][j].pos.y != player_pos.y) dy = 0;

                        if (undeed[i][j].chase_count < 7) {
                            undeed[i][j].chase_count++;
                        } else {
                            undeed[i][j].check_move = true;
                            undeed[i][j].chase_count = 0;
                        }
                    }

                    int new_x = undeed[i][j].pos.x + dx;
                    int new_y = undeed[i][j].pos.y + dy;

                    if (chat[new_y][new_x] == FLOOR || chat[new_y][new_x] == PLAYER) {
                        chat[undeed[i][j].pos.y][undeed[i][j].pos.x] = FLOOR;
                        undeed[i][j].pos.x = new_x;
                        undeed[i][j].pos.y = new_y;
                        chat[new_y][new_x] = 'U';
                        attron(COLOR_PAIR(18)); 
                        mvaddch(new_y, new_x, 'U');
                        attroff(COLOR_PAIR(18));
                    }else if(chat[new_y][new_x] == TRAP){
                        chat[undeed[i][j].pos.y][undeed[i][j].pos.x] = TRAP;
                        undeed[i][j].pos.x = new_x;
                        undeed[i][j].pos.y = new_y;
                        chat[new_y][new_x] = 'U';
                        attron(COLOR_PAIR(18)); 
                        mvaddch(new_y, new_x, 'U');
                        attroff(COLOR_PAIR(18));
                    }
                }
                else{
                    undeed[i][j].chase_count = 0;
                }
            } else if(undeed[i][j].check_move){
                move_undeed[i][j] += 1;
                if(move_undeed[i][j] > 20) {
                    undeed[i][j].check_move = false;
                    // undeed[i][j].chase_count = 0;
                    move_undeed[i][j] = 0;
                }
            }
            if (snake[i][j].alive && !snake[i][j].stunned) {
                int dx = 0, dy = 0;
                int distance_x = abs(snake[i][j].pos.x - player_pos.x);
                int distance_y = abs(snake[i][j].pos.y - player_pos.y);

                if (distance_x <= 4 && distance_y <= 4) {
                    if (snake[i][j].pos.x < player_pos.x) dx = 1;
                    else if (snake[i][j].pos.x > player_pos.x) dx = -1;

                    if (snake[i][j].pos.y < player_pos.y) dy = 1;
                    else if (snake[i][j].pos.y > player_pos.y) dy = -1;

                    if (distance_x <= 1 && distance_y <= 1) {
                        if (snake[i][j].pos.x != player_pos.x) dx = 0;
                        if (snake[i][j].pos.y != player_pos.y) dy = 0;
                    }

                    int new_x = snake[i][j].pos.x + dx;
                    int new_y = snake[i][j].pos.y + dy;

                    if (chat[new_y][new_x] == FLOOR || chat[new_y][new_x] == PLAYER ) {
                        chat[snake[i][j].pos.y][snake[i][j].pos.x] = FLOOR;
                        snake[i][j].pos.x = new_x;
                        snake[i][j].pos.y = new_y;
                        chat[new_y][new_x] = 'S';
                        attron(COLOR_PAIR(16)); 
                        mvaddch(new_y, new_x, 'S');
                        attroff(COLOR_PAIR(16));
                    }
                    if(chat[new_y][new_x] == TRAP){
                        chat[snake[i][j].pos.y][snake[i][j].pos.x] = TRAP;
                        snake[i][j].pos.x = new_x;
                        snake[i][j].pos.y = new_y;
                        chat[new_y][new_x] = 'S';
                        attron(COLOR_PAIR(16)); 
                        mvaddch(new_y, new_x, 'S');
                        attroff(COLOR_PAIR(16));
                    }
                    if (chat[new_y][new_x] == DOOR) {
                        chat[snake[i][j].pos.y][snake[i][j].pos.x] = '+';
                        snake[i][j].pos.x = new_x;
                        snake[i][j].pos.y = new_y;
                        chat[new_y][new_x] = 'S';
                        attron(COLOR_PAIR(16)); 
                        mvaddch(new_y, new_x, 'S');
                        attroff(COLOR_PAIR(16));
                    }
                    if (chat[new_y][new_x] == '?') {
                        chat[snake[i][j].pos.y][snake[i][j].pos.x] = '?';
                        snake[i][j].pos.x = new_x;
                        snake[i][j].pos.y = new_y;
                        chat[new_y][new_x] = 'S';
                        attron(COLOR_PAIR(16));
                        mvaddch(new_y, new_x, 'S');
                        attroff(COLOR_PAIR(16));
                    }
                    if (chat[new_y][new_x] == '#') {
                        chat[snake[i][j].pos.y][snake[i][j].pos.x] = '#';
                        snake[i][j].pos.x = new_x;
                        snake[i][j].pos.y = new_y;
                        chat[new_y][new_x] = 'S';
                        attron(COLOR_PAIR(16));
                        mvaddch(new_y, new_x, 'S');
                        attroff(COLOR_PAIR(16));
                    }
                    
                }
            }
            
            if (giant[i][j].alive && !giant[i][j].stunned && !giant[i][j].check_move) {

                int dx = 0, dy = 0;
                int distance_x = abs(giant[i][j].pos.x - player_pos.x);
                int distance_y = abs(giant[i][j].pos.y - player_pos.y);

                if (distance_x <= 5 && distance_y <= 5) {
                    if (giant[i][j].pos.x < player_pos.x) dx = 1;
                    
                    else if (giant[i][j].pos.x > player_pos.x) dx = -1;
                    
                    if (giant[i][j].pos.y < player_pos.y) dy = 1;
                    
                    else if (giant[i][j].pos.y > player_pos.y) dy = -1;


                    if (distance_x <= 1 && distance_y <= 1) {
                        if (giant[i][j].pos.x != player_pos.x) dx = 0;
                        if (giant[i][j].pos.y != player_pos.y) dy = 0;

                        if (giant[i][j].chase_count < 7) {
                            giant[i][j].chase_count++;
                        } else {
                            giant[i][j].check_move = true;
                            giant[i][j].chase_count = 0;  
                        }
                    }
                    int new_x = giant[i][j].pos.x + dx;
                    int new_y = giant[i][j].pos.y + dy;

                    if (chat[new_y][new_x] == FLOOR || chat[new_y][new_x] == PLAYER ) {
                        chat[giant[i][j].pos.y][giant[i][j].pos.x] = FLOOR;
                        giant[i][j].pos.x = new_x;
                        giant[i][j].pos.y = new_y;
                        chat[new_y][new_x] = 'G';
                        attron(COLOR_PAIR(17)); 
                        mvaddch(new_y, new_x, 'G');
                        attroff(COLOR_PAIR(17));
                    }
                    if(chat[new_y][new_x] == TRAP){
                        chat[giant[i][j].pos.y][giant[i][j].pos.x] = TRAP;
                        giant[i][j].pos.x = new_x;
                        giant[i][j].pos.y = new_y;
                        chat[new_y][new_x] = 'G';
                        attron(COLOR_PAIR(17)); 
                        mvaddch(new_y, new_x, 'G');
                        attroff(COLOR_PAIR(17));
                    }
                }
                else {
                    giant[i][j].chase_count = 0; 
                }
            }else if(giant[i][j].check_move){
                move_giant[i][j] += 1;
                if(move_giant[i][j] > 20){
                    giant[i][j].check_move = false;
                    // giant[i][j].chase_count = 0;
                    move_giant[i][j] = 0;
                }

            }
        }
    }
}

void initialize_map_status() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            map_status[y][x] = (chat[y][x] == ' ' || chat[y][x] == PLAYER);
        }
    }
    
    map_status[player_pos.y][player_pos.x] = true;
}

void reveal_three_steps_ahead(int x, int y, int dx, int dy, int steps) {
    if (steps == 0 || x < 0 || x >= NUMCOLS || y < 0 || y >= NUMLINES) {
        return;
    }

    char ch = chat[y][x];
    if (ch == '#' || ch == '+' || ch == '?' ) {
        map_status[y][x] = true;
    } else {
        return;
    }

    if (ch == '+') {
        return;
    }
    reveal_three_steps_ahead(x + dx, y + dy, dx, dy, steps - 1);
}


void reveal_room(int x, int y) {

    if (x < 0 || x >= NUMCOLS || y < 0 || y >= NUMLINES) {
        return;
    }
    
    if (map_status[y][x]) {
        return;
    }
    char ch = chat[y][x];
    if (ch == ' ' || ch == '|' || ch == '_' || ch == '.' || 
        ch == '+' || ch == '?' || ch == FOOD || ch == MAGIC || 
        ch == SUPERIOR || ch == ROTTEN || ch == GOLD_NORMAL || 
        ch == GOLD_ADVANCED || ch == 'D' || ch == 'F' || 
        ch == 'G' || ch == 'S' || ch == 'U' || ch == DOOR ||
        ch == ROTTEN || ch == '9' || ch == '$' || ch == STAIRS || ch == 'h' || ch == 's' || 
        ch == 'd' || ch == 'M' || ch == 'K' || ch == 'A' || ch == 'N' || ch == '!' ||
         ch == 'O' || ch == TRAP || ch == WINDOW || ch == '8' || ch == '&') {
        map_status[y][x] = true;
    } else {
        return;
    }

    reveal_room(x + 1, y);
    reveal_room(x - 1, y); 
    reveal_room(x, y + 1);
    reveal_room(x, y - 1); 
    reveal_room(x + 1, y + 1); 
    reveal_room(x - 1, y + 1);
    reveal_room(x + 1, y - 1);
    reveal_room(x - 1, y - 1);
}

void update_map_status() {
    if (check_nightmare_room(player_pos.x, player_pos.y)) {
        reveal_two_steps_around(player_pos.x, player_pos.y);
        display_nightmare_room();
    } else{
    reveal_room(player_pos.x, player_pos.y);
    if (chat[player_pos.y][player_pos.x + 1] == '#') {
        reveal_three_steps_ahead(player_pos.x + 1, player_pos.y, 1, 0, 3);
    }
    if (chat[player_pos.y][player_pos.x - 1] == '#') {
        reveal_three_steps_ahead(player_pos.x - 1, player_pos.y, -1, 0, 3);
    }
    if (chat[player_pos.y + 1][player_pos.x] == '#') {
        reveal_three_steps_ahead(player_pos.x, player_pos.y + 1, 0, 1, 3);
    }
    if (chat[player_pos.y - 1][player_pos.x] == '#') {
        reveal_three_steps_ahead(player_pos.x, player_pos.y - 1, 0, -1, 3);
    }
    if (treasure_roomm[player_pos.y][player_pos.x]) {
            display_treasure_room();
    }
}
}
void display_treasure_room() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (treasure_roomm[y][x]) {
                mvaddch(y, x, chat[y][x]);
            }
        }
    }
    refresh();
}


void display_updated_map() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (map_status[y][x]) {
                mvaddch(y, x, chat[y][x]);
            }
        }
    }
    refresh();
}

void show_all_map() {
    bool temp_map_status[NUMLINES][NUMCOLS];
    for (int y = 0; y < NUMLINES; y++) {
        for (int x = 0; x < NUMCOLS; x++) {
            temp_map_status[y][x] = map_status[y][x];
            map_status[y][x] = true;
        }
    }

    display_map();
    display_updated_map();
    refresh();
    display_map();
    int ch;
    ch = getch();
    if(ch == 'm'|| ch == 'M'){
    for (int y = 0; y < NUMLINES; y++) {
        for (int x = 0; x < NUMCOLS; x++) {
            map_status[y][x] = temp_map_status[y][x];
        }
    }

    display_updated_map();
    display_map();
    refresh();
    }
}


void save_current_floor() {
    char filename[100];
    snprintf(filename, sizeof(filename), "floor_%d.txt", current_floor);

    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening floor file");
        return;
    }

    for (int y = 0; y < NUMLINES; y++) {
        for (int x = 0; x < NUMCOLS; x++) {
            fputc(chat[y][x], file);
        }
        fputc('\n', file);
    }

    for (int y = 0; y < NUMLINES; y++) {
        for (int x = 0; x < NUMCOLS; x++) {
            fprintf(file, "%d ", map_status[y][x] ? 1 : 0);
        }
        fputc('\n', file);
    }
    fprintf(file, "%d %d\n", player_pos.x, player_pos.y);

    // Save nightmare room state
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            fprintf(file, "%d ", nightmare_room[y][x] ? 1 : 0);
        }
        fputc('\n', file);
    }

    // Save spell room state
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            fprintf(file, "%d ", spell_room[y][x] ? 1 : 0);
        }
        fputc('\n', file);
    }

    fprintf(file, "%d %d\n", player_pos.x, player_pos.y);

    fclose(file);
}

void initialize_previous_floor() {
    char filename[100];
    snprintf(filename, sizeof(filename), "floor_%d.txt", current_floor);

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening floor file");
        return;
    }
    for (int y = 0; y < NUMLINES; y++) {
        for (int x = 0; x < NUMCOLS; x++) {
            map_status[y][x] = false;
            chat[y][x] = ' ';
        }
    }
    for (int y = 0; y < NUMLINES; y++) {
        for (int x = 0; x < NUMCOLS; x++) {
            fscanf(file, "%c", &chat[y][x]);
        }
        fgetc(file);
    }

    for (int y = 0; y < NUMLINES; y++) {
        for (int x = 0; x < NUMCOLS; x++) {
            int status;
            fscanf(file, "%d", &status);
            map_status[y][x] = (status == 1);
        }
    }

    fscanf(file, "%d %d", &player_pos.x, &player_pos.y);

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int status;
            fscanf(file, "%d", &status);
            nightmare_room[y][x] = (status == 1);
        }
    }

    // Restore spell room state
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int status;
            fscanf(file, "%d", &status);
            spell_room[y][x] = (status == 1);
        }
    }

    fclose(file);

    display_updated_map();
    refresh();

    mvprintw(LINES / 2 + 2, COLS / 2 - 10, "Moved to previous floor.");
}

void attack_with_dagger(){
    attacking = 0;
    s_attack = 0;
    u_attack = 0;
    d_attack = 0;
    g_attack = 0;
    f_attack = 0;
    if(player_weapon->count == 0){
        show_message_top_right5("Your weapon finished");
        return;
    }else{
        if (!has_last_direction) {
            int direction = getch();
            switch (direction) {
                case '8': last_dy = -1; last_dx = 0; break;  // Up
                case '2': last_dy = 1; last_dx = 0; break;   // Down
                case '6': last_dx = 1; last_dy = 0; break;   // Right
                case '4': last_dx = -1; last_dy = 0; break;  // Left
                case '9': last_dx = 1; last_dy = -1; break;  // Up-Right
                case '7': last_dx = -1; last_dy = -1; break; // Up-Left
                case '1': last_dx = -1; last_dy = 1; break;  // Down-Left
                case '3': last_dx = 1; last_dy = 1; break;   // Down-Right
                default:
                    show_message_top_right5("Invalid direction!");
                    return;
            }
            has_last_direction = true;
        }
        bool hit_target = false;
        int prev_x = player_pos.x;
        int prev_y = player_pos.y;


        for (int i = 1; i <= player_weapon->range; i++) {
            int new_x = player_pos.x + i * last_dx;
            int new_y = player_pos.y + i * last_dy;

            if (new_x < 0 || new_x >= NUMCOLS || new_y < 0 || new_y >= NUMLINES || 
                chat[new_y][new_x] == WALL_HORZ || chat[new_y][new_x] == WALL_VERT ||
                 chat[new_y][new_x] == DOOR || chat[new_y][new_x] == UNKHOWN_DOOR  || chat[new_y][new_x] == 'O' ) {
                    chat[prev_y][prev_x] = 'K'; 
                    mvaddch(prev_y, prev_x, 'K');
                    hit_target = true;
                    player_weapon->thrown = true;
                    break;
            }

            if (chat[new_y][new_x] == FLOOR || chat[new_y][new_x] == PASSAGE) {
                prev_x = new_x;
                prev_y = new_y;
                chat[new_y][new_x] = 'K';
                mvaddch(new_y, new_x, 'K');
                refresh();
                usleep(50000); 
                chat[new_y][new_x] = FLOOR;
                mvaddch(new_y, new_x, FLOOR);
            }

            if (chat[new_y][new_x] == 'G' || chat[new_y][new_x] == 'S' || chat[new_y][new_x] == 'U' ||
                 chat[new_y][new_x] == 'F' || chat[new_y][new_x] == 'D')  {
                for (int room = 0; room < MAXROOMS; room++) {
                    for (int j = 0; j < MAX_MONSTERS; j++) {
                        if (daemons[room][j].pos.x == new_x && daemons[room][j].pos.y == new_y && daemons[room][j].alive) {
                            daemons[room][j].health -= damage_spells * player_weapon->damage;
                            attacking = 1;
                            if (daemons[room][j].health <= 0) daemons[room][j].health = 0;
                            show_message_top_right5("Health of the Daemons: %d", daemons[room][j].health);
                            if (daemons[room][j].health <= 0) {
                                daemons[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                d_attack = 1;
                            }
                        }
                        if (fbms[room][j].pos.x == new_x && fbms[room][j].pos.y == new_y && fbms[room][j].alive) {
                            fbms[room][j].health -= damage_spells * player_weapon->damage;
                            attacking = 1;
                            if (fbms[room][j].health <= 0) fbms[room][j].health = 0;
                            show_message_top_right5("Health of the Fbms: %d", fbms[room][j].health);
                            if (fbms[room][j].health <= 0) {
                                fbms[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                f_attack = 1;
                            }
                        }
                        if (giant[room][j].pos.x == new_x && giant[room][j].pos.y == new_y && giant[room][j].alive) {
                            giant[room][j].health -= damage_spells * player_weapon->damage;
                            attacking = 1;
                            if (giant[room][j].health <= 0) giant[room][j].health = 0;
                            show_message_top_right5("Health of the Giant: %d", giant[room][j].health);
                            if (giant[room][j].health <= 0) {
                                giant[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                g_attack = 1;
                            }
                        }
                        if (snake[room][j].pos.x == new_x && snake[room][j].pos.y == new_y && snake[room][j].alive) {
                            snake[room][j].health -= damage_spells * player_weapon->damage;
                            attacking = 1;
                            if (snake[room][j].health <= 0) snake[room][j].health = 0;
                            show_message_top_right5("Health of the Snake: %d", snake[room][j].health);
                            if (snake[room][j].health <= 0) {
                                snake[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                s_attack = 1;
                            }
                        }
                        if (undeed[room][j].pos.x == new_x && undeed[room][j].pos.y == new_y && undeed[room][j].alive) {
                            undeed[room][j].health -= damage_spells * player_weapon->damage;
                            attacking = 1;
                            if (undeed[room][j].health <= 0) undeed[room][j].health = 0 ;
                            show_message_top_right5("Health of the Undeed: %d", undeed[room][j].health);
                            if (undeed[room][j].health <= 0) {
                                undeed[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                u_attack = 1;
                            }
                        }
                    }
                }
                hit_target = true;
                break; 
            }
        }


        if (!hit_target) {
            chat[prev_y][prev_x] = 'K';
            mvaddch(prev_y, prev_x, 'K');
            player_weapon->thrown = true;
        }
        player_weapon->count--;
        display_updated_map();
        display_map();
        refresh();
    }
}

void attack_with_wand(){
    if(player_weapon->count == 0){
        show_message_top_right5("Your weapon finished");
        return;
    }else{
        if (!has_last_direction) {
            int direction = getch();
            switch (direction) {
                case '8': last_dy = -1; last_dx = 0; break;  // Up
                case '2': last_dy = 1; last_dx = 0; break;   // Down
                case '6': last_dx = 1; last_dy = 0; break;   // Right
                case '4': last_dx = -1; last_dy = 0; break;  // Left
                case '9': last_dx = 1; last_dy = -1; break;  // Up-Right
                case '7': last_dx = -1; last_dy = -1; break; // Up-Left
                case '1': last_dx = -1; last_dy = 1; break;  // Down-Left
                case '3': last_dx = 1; last_dy = 1; break;   // Down-Right
                default:
                    show_message_top_right5("Invalid direction!");
                    return;
            }
            has_last_direction = true;
        }
        bool hit_target = false;
        int prev_x = player_pos.x;
        int prev_y = player_pos.y;


        for (int i = 1; i <= player_weapon->range; i++) {
            int new_x = player_pos.x + i * last_dx;
            int new_y = player_pos.y + i * last_dy;

            if (new_x < 0 || new_x >= NUMCOLS || new_y < 0 || new_y >= NUMLINES || 
                chat[new_y][new_x] == WALL_HORZ || chat[new_y][new_x] == WALL_VERT || 
                chat[new_y][new_x] == DOOR || chat[new_y][new_x] == UNKHOWN_DOOR  || chat[new_y][new_x] == 'O' ) {
                    chat[prev_y][prev_x] = 'A'; 
                    mvaddch(prev_y, prev_x, 'A');
                    hit_target = true;
                    player_weapon->thrown = true;
                    break;
            }
            if (chat[new_y][new_x] == FLOOR || chat[new_y][new_x] == PASSAGE) {
                prev_x = new_x;
                prev_y = new_y;
                chat[new_y][new_x] = 'A';
                mvaddch(new_y, new_x, 'A');
                refresh();
                usleep(50000);
                chat[new_y][new_x] = FLOOR;
                mvaddch(new_y, new_x, FLOOR);
            }

            if (chat[new_y][new_x] == 'G' || chat[new_y][new_x] == 'S' || chat[new_y][new_x] == 'U' ||
                     chat[new_y][new_x] == 'F' || chat[new_y][new_x] == 'D')  {
                for (int room = 0; room < MAXROOMS; room++) {
                    for (int j = 0; j < MAX_MONSTERS; j++) {
                        if (daemons[room][j].pos.x == new_x && daemons[room][j].pos.y == new_y && daemons[room][j].alive) {
                            daemons[room][j].health -= damage_spells * player_weapon->damage;
                            attacking = 1;
                            if (daemons[room][j].health <= 0) daemons[room][j].health = 0;
                            show_message_top_right5("Health of the Daemons: %d", daemons[room][j].health);
                            if (daemons[room][j].health <= 0) {
                                daemons[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                d_attack = 1;
                            }else {
                                daemons[room][j].stunned = true; 
                            }
                        }
                        if (fbms[room][j].pos.x == new_x && fbms[room][j].pos.y == new_y && fbms[room][j].alive) {
                            fbms[room][j].health -= damage_spells * player_weapon->damage;
                            attacking = 1;
                            if (fbms[room][j].health <= 0) fbms[room][j].health = 0;
                            show_message_top_right5("Health of the Fbms: %d", fbms[room][j].health);
                            if (fbms[room][j].health <= 0) {
                                fbms[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                f_attack = 1;
                            }else {
                                fbms[room][j].stunned = true;
                            }
                        }
                        if (giant[room][j].pos.x == new_x && giant[room][j].pos.y == new_y && giant[room][j].alive) {
                            giant[room][j].health -= damage_spells * player_weapon->damage;
                            attacking = 1;
                            if (giant[room][j].health <= 0) giant[room][j].health = 0;
                            show_message_top_right5("Health of the Giant: %d", giant[room][j].health);
                            if (giant[room][j].health <= 0) {
                                giant[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                g_attack = 1;
                            }else {
                                giant[room][j].stunned = true; 
                            }
                        }
                        if (snake[room][j].pos.x == new_x && snake[room][j].pos.y == new_y && snake[room][j].alive) {
                            snake[room][j].health -= damage_spells * player_weapon->damage;
                            attacking = 1 ;
                            if (snake[room][j].health <= 0) snake[room][j].health = 0;
                            show_message_top_right5("Health of the Snake: %d", snake[room][j].health);
                            if (snake[room][j].health <= 0) {
                                snake[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                s_attack = 1;
                            }else {
                                snake[room][j].stunned = true; 
                            }
                        }
                        if (undeed[room][j].pos.x == new_x && undeed[room][j].pos.y == new_y && undeed[room][j].alive) {
                            undeed[room][j].health -= damage_spells * player_weapon->damage;
                            attacking = 1;
                            if (undeed[room][j].health <= 0) undeed[room][j].health = 0 ;
                            show_message_top_right5("Health of the Undeed: %d", undeed[room][j].health);
                            if (undeed[room][j].health <= 0) {
                                undeed[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                u_attack = 1;
                            }else {
                                undeed[room][j].stunned = true; 
                            }
                        }
                    }
                }
                hit_target = true;
                break;
            }
        }


        if (!hit_target) {
            chat[prev_y][prev_x] = 'S';
            mvaddch(prev_y, prev_x, 'S');
            player_weapon->thrown = true;
        }
        player_weapon->count--;
        display_updated_map();
        display_map();
        refresh();
    }
}

void attack_with_mace() {
    if (weapons[0].count == 0){
        show_message_top_right5("No weapon in hand!");
        return;
    }
    int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1}, {1, 0}, {1, 1}
    };

    if (weapons[0].count > 0) {
        weapons[0].count--; 

        for (int i = 0; i < 8; i++) {
            int new_y = player_pos.y + directions[i][0];
            int new_x = player_pos.x + directions[i][1];

            if (new_y >= 0 && new_y < NUMLINES && new_x >= 0 && new_x < NUMCOLS) {

                for (int room = 0; room < MAXROOMS; room++) {
                    for (int j = 0; j < MAX_MONSTERS; j++) {
                        if (daemons[room][j].pos.x == new_x && daemons[room][j].pos.y == new_y && daemons[room][j].alive) {
                            daemons[room][j].health -= damage_spells * weapons[0].damage;
                            attacking = 1;
                            // show_message_top_right4("You are attacking with Monsters ;)");
                            if (daemons[room][j].health <= 0) daemons[room][j].health = 0;
                            show_message_top_right5("Health of the Daemons: %d", daemons[room][j].health);
                            if (daemons[room][j].health <= 0) {
                                daemons[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                d_attack = 1;
                            }
                        }
                        if (fbms[room][j].pos.x == new_x && fbms[room][j].pos.y == new_y && fbms[room][j].alive) {
                            fbms[room][j].health -= damage_spells * weapons[0].damage;
                            attacking = 1;
                            if (fbms[room][j].health <= 0) fbms[room][j].health = 0;
                            show_message_top_right5("Health of the Fbms: %d", fbms[room][j].health);
                            if (fbms[room][j].health <= 0) {
                                fbms[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                f_attack = 1;
                            }
                        }
                        if (giant[room][j].pos.x == new_x && giant[room][j].pos.y == new_y && giant[room][j].alive) {
                            giant[room][j].health -= damage_spells * weapons[0].damage;
                            attacking= 1;
                            if (giant[room][j].health <= 0) giant[room][j].health = 0;
                            show_message_top_right5("Health of the Giant: %d", giant[room][j].health);
                            if (giant[room][j].health <= 0) {
                                giant[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                g_attack = 1;
                            }
                        }
                        if (snake[room][j].pos.x == new_x && snake[room][j].pos.y == new_y && snake[room][j].alive) {
                            snake[room][j].health -= damage_spells * weapons[0].damage;
                            attacking = 1;
                            if (snake[room][j].health <= 0)snake[room][j].health = 0;
                            show_message_top_right5("Health of the Snake: %d", snake[room][j].health);
                            if (snake[room][j].health <= 0) {
                                snake[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                s_attack = 1;
                            }
                        }
                        if (undeed[room][j].pos.x == new_x && undeed[room][j].pos.y == new_y && undeed[room][j].alive) {
                            undeed[room][j].health -= damage_spells * weapons[0].damage;
                            attacking = 1;
                            if (undeed[room][j].health <= 0) undeed[room][j].health = 0;
                            show_message_top_right5("Health of the Undeed: %d", undeed[room][j].health);
                            if (undeed[room][j].health <= 0) {
                                undeed[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                u_attack = 1;
                            }
                        }
                    }
                }
            }
        }

        display_updated_map();
        display_map();
        refresh();
    } else {
        show_message_top_right9("No maces left!"); 
    }
}

void attack_with_sword() {
    if (weapons[4].count == 0){
        show_message_top_right5("No weapon in hand!");
        return;
    }
    int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1}, {1, 0}, {1, 1}
    };

    if (weapons[4].count > 0) {
        weapons[4].count--; 

        for (int i = 0; i < 8; i++) {
            int new_y = player_pos.y + directions[i][0];
            int new_x = player_pos.x + directions[i][1];

            if (new_y >= 0 && new_y < NUMLINES && new_x >= 0 && new_x < NUMCOLS) {

                for (int room = 0; room < MAXROOMS; room++) {
                    for (int j = 0; j < MAX_MONSTERS; j++) {
                        if (daemons[room][j].pos.x == new_x && daemons[room][j].pos.y == new_y && daemons[room][j].alive) {
                            daemons[room][j].health -= damage_spells * weapons[4].damage;
                            attacking = 1;
                            if (daemons[room][j].health <= 0) daemons[room][j].health = 0;
                            show_message_top_right5("Health of the Daemons: %d", daemons[room][j].health);
                            if (daemons[room][j].health <= 0) {
                                daemons[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                d_attack = 1;
                            }
                        }
                        if (fbms[room][j].pos.x == new_x && fbms[room][j].pos.y == new_y && fbms[room][j].alive) {
                            fbms[room][j].health -= damage_spells * weapons[4].damage;
                            attacking = 1;
                            if (fbms[room][j].health <= 0) fbms[room][j].health = 0;
                            show_message_top_right5("Health of the Fbms: %d", fbms[room][j].health);
                            if (fbms[room][j].health <= 0) {
                                fbms[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                f_attack = 1;
                            }
                        }
                        if (giant[room][j].pos.x == new_x && giant[room][j].pos.y == new_y && giant[room][j].alive) {
                            giant[room][j].health -= damage_spells * weapons[4].damage;
                            attacking = 1;
                            if (giant[room][j].health <= 0) giant[room][j].health = 0;
                            show_message_top_right5("Health of the Giant: %d", giant[room][j].health);
                            if (giant[room][j].health <= 0) {
                                giant[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                g_attack = 1;
                            }
                        }
                        if (snake[room][j].pos.x == new_x && snake[room][j].pos.y == new_y && snake[room][j].alive) {
                            snake[room][j].health -= damage_spells * weapons[4].damage;
                            attacking = 1;
                            if (snake[room][j].health <= 0)snake[room][j].health = 0;
                            show_message_top_right5("Health of the Snake: %d", snake[room][j].health);
                            if (snake[room][j].health <= 0) {
                                snake[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                s_attack = 1;
                            }
                        }
                        if (undeed[room][j].pos.x == new_x && undeed[room][j].pos.y == new_y && undeed[room][j].alive) {
                            undeed[room][j].health -= damage_spells * weapons[4].damage;
                            attacking = 1;
                            if (undeed[room][j].health <= 0) undeed[room][j].health = 0;
                            show_message_top_right5("Health of the Undeed: %d", undeed[room][j].health);
                            if (undeed[room][j].health <= 0) {
                                undeed[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                u_attack = 1;
                            }
                        }
                    }
                }
            }
        }

        display_updated_map();
        display_map();
        refresh();
    } else {
        show_message_top_right5("No swords left!"); 
    }
}

void attack_with_normal(){
    if(player_weapon->count == 0){
        show_message_top_right5("Your weapon finished");
        return;
    }else{
        if (!has_last_direction) {
            int direction = getch();
            switch (direction) {
                case '8': last_dy = -1; last_dx = 0; break;  // Up
                case '2': last_dy = 1; last_dx = 0; break;   // Down
                case '6': last_dx = 1; last_dy = 0; break;   // Right
                case '4': last_dx = -1; last_dy = 0; break;  // Left
                case '9': last_dx = 1; last_dy = -1; break;  // Up-Right
                case '7': last_dx = -1; last_dy = -1; break; // Up-Left
                case '1': last_dx = -1; last_dy = 1; break;  // Down-Left
                case '3': last_dx = 1; last_dy = 1; break;   // Down-Right
                default:
                    show_message_top_right5("Invalid direction!");
                    return;
            }
            has_last_direction = true;
        }
        bool hit_target = false;
        int prev_x = player_pos.x;
        int prev_y = player_pos.y;


        for (int i = 1; i <= player_weapon->range; i++) {
            int new_x = player_pos.x + i * last_dx;
            int new_y = player_pos.y + i * last_dy;

            if (new_x < 0 || new_x >= NUMCOLS || new_y < 0 || new_y >= NUMLINES || 
                chat[new_y][new_x] == WALL_HORZ || chat[new_y][new_x] == WALL_VERT ||
                 chat[new_y][new_x] == DOOR || chat[new_y][new_x] == UNKHOWN_DOOR  || chat[new_y][new_x] == 'O' ) {
                    chat[prev_y][prev_x] = 'N';
                    mvaddch(prev_y, prev_x, 'N');
                    hit_target = true;
                    player_weapon->thrown = true;
                    break;
            }


            if (chat[new_y][new_x] == FLOOR || chat[new_y][new_x] == PASSAGE) {
                prev_x = new_x;
                prev_y = new_y;
                chat[new_y][new_x] = 'N';
                mvaddch(new_y, new_x, 'N');
                refresh();
                usleep(50000); 
                chat[new_y][new_x] = FLOOR;
                mvaddch(new_y, new_x, FLOOR);
            }

            if (chat[new_y][new_x] == 'G' || chat[new_y][new_x] == 'S' ||
             chat[new_y][new_x] == 'U' || chat[new_y][new_x] == 'F' || chat[new_y][new_x] == 'D')  {
                for (int room = 0; room < MAXROOMS; room++) {
                    for (int j = 0; j < MAX_MONSTERS; j++) {
                        if (daemons[room][j].pos.x == new_x && daemons[room][j].pos.y == new_y && daemons[room][j].alive) {
                            daemons[room][j].health -= damage_spells * player_weapon->damage;
                            attacking = 1;
                            if (daemons[room][j].health <= 0) daemons[room][j].health = 0;
                            show_message_top_right5("Health of the Daemons: %d", daemons[room][j].health);
                            if (daemons[room][j].health <= 0) {
                                daemons[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                d_attack = 1;
                            }
                        }
                        if (fbms[room][j].pos.x == new_x && fbms[room][j].pos.y == new_y && fbms[room][j].alive) {
                            fbms[room][j].health -= damage_spells * player_weapon->damage;
                            attacking = 1;
                            if (fbms[room][j].health <= 0) fbms[room][j].health = 0;
                            show_message_top_right5("Health of the Fbms: %d", fbms[room][j].health);
                            if (fbms[room][j].health <= 0) {
                                fbms[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                f_attack = 1;
                            }
                        }
                        if (giant[room][j].pos.x == new_x && giant[room][j].pos.y == new_y && giant[room][j].alive) {
                            giant[room][j].health -= damage_spells * player_weapon->damage;
                            attacking = 1;
                            if (giant[room][j].health <= 0) giant[room][j].health = 0;
                            show_message_top_right5("Health of the Giant: %d", giant[room][j].health);
                            if (giant[room][j].health <= 0) {
                                giant[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                g_attack = 1;
                            }
                        }
                        if (snake[room][j].pos.x == new_x && snake[room][j].pos.y == new_y && snake[room][j].alive) {
                            snake[room][j].health -= damage_spells * player_weapon->damage;
                            attacking = 1;
                            if (snake[room][j].health <= 0) snake[room][j].health = 0;
                            show_message_top_right5("Health of the Snake: %d", snake[room][j].health);
                            if (snake[room][j].health <= 0) {
                                snake[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                s_attack = 1;
                            }
                        }
                        if (undeed[room][j].pos.x == new_x && undeed[room][j].pos.y == new_y && undeed[room][j].alive) {
                            undeed[room][j].health -= damage_spells * player_weapon->damage;
                            attacking = 1;
                            if (undeed[room][j].health <= 0) undeed[room][j].health = 0 ;
                            show_message_top_right5("Health of the Undeed: %d", undeed[room][j].health);
                            if (undeed[room][j].health <= 0) {
                                undeed[room][j].alive = false;
                                kill_monster(new_y, new_x);
                                u_attack = 1;
                            }
                        }
                    }
                }
                hit_target = true;
                break; 
            }
        }


        if (!hit_target) {
            chat[prev_y][prev_x] = 'N';
            mvaddch(prev_y, prev_x, 'N');
            player_weapon->thrown = true;
        }
        player_weapon->count--;
        display_updated_map();
        display_map();
        refresh();
    }
}

void kill_monster(int y, int x) {
    chat[y][x] = FLOOR; 
    mvaddch(y, x, chat[y][x]); 
    refresh();
}

void initialize_monsters() {
    for (int room = 0; room < MAXROOMS; room++) {
        for (int i = 0; i < MAX_MONSTERS; i++) {

            daemons[room][i].health = 5;
            daemons[room][i].alive = true;

            fbms[room][i].health = 10;
            fbms[room][i].alive = true;

            giant[room][i].health = 15;
            giant[room][i].alive = true;

            snake[room][i].health = 20;
            snake[room][i].alive = true;

            undeed[room][i].health = 30;
            undeed[room][i].alive = true;
        }
    }
}

void place_random_windows() {
    for (int i = 0; i < MAXROOMS; i++) {
        int num_windows = rnd(2) + 1; 
        for (int j = 0; j < num_windows; j++) {
            int wall = rnd(2); 
            int x, y;
            struct room *rp = &rooms[i];
            switch (wall) {
                case 0: 
                    x = rp->r_pos.x;
                    y = rp->r_pos.y + rnd(rp->r_max.y - 2) + 1;
                    break;
                case 1: 
                    x = rp->r_pos.x + rp->r_max.x - 1;
                    y = rp->r_pos.y + rnd(rp->r_max.y - 2) + 1;
                    break;
            }

            if (chat[y][x] == '|') {
                chat[y][x] = WINDOW;
            }
        }
    }
}

bool is_near_window() {
    int directions[4][2] = { {0, 1}, {1, 0}, {0, -1}, {-1, 0} };
    for (int i = 0; i < 4; i++) {
        int new_x = player_pos.x + directions[i][0];
        int new_y = player_pos.y + directions[i][1];
        if (chat[new_y][new_x] == WINDOW) {
            return true;
        }
    }
    return false;
}

void reveal_room_through_window(int window_x, int window_y) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            temp_map_status2[y][x] = map_status[y][x];
        }
    }

    for (int x = 0; x < NUMCOLS; x++) {
        if (chat[window_y][x] == WALL_VERT || chat[window_y][x] == WALL_HORZ ||
         chat[window_y][x] == DOOR || chat[window_y][x] == UNKHOWN_DOOR) {
            reveal_room(x, window_y);
        }
    }


    display_updated_map();
    display_map();
    refresh();

    usleep(2000000);
    hide_room_through_window();
}

void hide_room_through_window() {

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            map_status[y][x] = temp_map_status2[y][x];
        }
    }
    display_map();
    display_updated_map();
    refresh();
}

void create_nightmare_room() {
    if (current_floor == 2 || current_floor == 3) {
        int nightmare_room_index = rnd(MAXROOMS);
        struct room *rp = &rooms[nightmare_room_index];

        for (int y = rp->r_pos.y; y < rp->r_pos.y + rp->r_max.y; y++) {
            for (int x = rp->r_pos.x; x < rp->r_pos.x + rp->r_max.x; x++) {
                if(chat[y][x] == '|' || chat[y][x] == '.' ||  chat[y][x] == '_' ||
                 chat[y][x] == '+' || chat[y][x] == '_' || chat[y][x] == ':' || chat[y][x] == '?' ||
                   chat[y][x] == 's' || chat[y][x] == '9' || chat[y][x] == '/' ||
                    chat[y][x] == '$' || chat[y][x] == '*' || chat[y][x] == '~' || chat[y][x] == 'h' ||
                   chat[y][x] == 'h' || chat[y][x] == '=' || chat[y][x] == '8' || chat[y][x] == '&'){
                        nightmare_room[y][x] = true;
                   }
            }
        }
    }
}
bool check_nightmare_room(int x, int y) {
    return nightmare_room[y][x];
}
void display_nightmare_room() {
    if (check_nightmare_room(player_pos.x, player_pos.y)) {
        for (int y = player_pos.y - 2; y <= player_pos.y + 2; y++) {
            for (int x = player_pos.x - 2; x <= player_pos.x + 2; x++) {
                if (y >= 0 && y < HEIGHT && x >= 0 && x < WIDTH) {
                    mvaddch(y, x, chat[y][x]);
                }
            }
        }
    } else {
        display_updated_map();
    }
    refresh();
}
void reveal_two_steps_around(int x, int y) {
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < WIDTH && ny >= 0 && ny < HEIGHT) {
                if (abs(dx) <= 2 && abs(dy) <= 2) {
                    map_status[ny][nx] = true;
                }
            }
        }
    }
}
void auto_move_player() {
    int new_x = player_pos.x + auto_dx;
    int new_y = player_pos.y + auto_dy;

    while (new_x >= 0 && new_x < NUMCOLS && new_y >= 0 && new_y < NUMLINES &&
           chat[new_y][new_x] != WALL_HORZ && chat[new_y][new_x] != WALL_VERT && 
           chat[new_y][new_x] != 'O' &&
           chat[new_y][new_x] != TRAP && chat[new_y][new_x] != FOOD && chat[new_y][new_x] != '*' && chat[new_y][new_x] != '/' &&
           chat[new_y][new_x] != '$' && chat[new_y][new_x] != '9' && chat[new_y][new_x] != 'K' && chat[new_y][new_x] != '!'
           && chat[new_y][new_x] != 'N' && chat[new_y][new_x] != 'M' && chat[new_y][new_x] != 'A' && chat[new_y][new_x] != 's'
           &&chat[new_y][new_x] != 'd' && chat[new_y][new_x] != 'h' && chat[new_y][new_x] != 'G' && chat[new_y][new_x] != 'F'
           &&chat[new_y][new_x] != 'D' && chat[new_y][new_x] != 'S' && chat[new_y][new_x] != 'U' && chat[new_y][new_x] != '~' 
           && chat[new_y][new_x] != '<' && chat[new_y][new_x] != ' ') {

        player_pos.x = new_x;
        player_pos.y = new_y;
        update_map_status();
        display_updated_map();
        display_map();

        new_x = player_pos.x + auto_dx;
        new_y = player_pos.y + auto_dy;

        usleep(50000); 
    }

    auto_moving = false;
}
void auto_move_player_one_step() {
    int new_x = player_pos.x + auto_dx;
    int new_y = player_pos.y + auto_dy;

    if (new_x >= 0 && new_x < NUMCOLS && new_y >= 0 && new_y < NUMLINES &&
        chat[new_y][new_x] != WALL_HORZ && chat[new_y][new_x] != WALL_VERT) {

        player_pos.x = new_x;
        player_pos.y = new_y;
        update_map_status();
        display_updated_map();
        display_map();
    }

    auto_moving = false;
}
void create_spell_room() {
    int spell_room_index = 1;
    
    
    if (spell_room_index == -1) return;

    struct room *rp = &rooms[spell_room_index];

    for (int y = rp->r_pos.y; y < rp->r_pos.y + rp->r_max.y; y++) {
            for (int x = rp->r_pos.x; x < rp->r_pos.x + rp->r_max.x; x++) {
                if(chat[y][x] == '|' || chat[y][x] == '.' ||  chat[y][x] == '_' || chat[y][x] == '+' ||
                 chat[y][x] == '_' || chat[y][x] == ':' || chat[y][x] == '?' ||chat[y][x] == 's' || 
                 chat[y][x] == '9' || chat[y][x] == '/' || chat[y][x] == '$' || chat[y][x] == '*' || 
                 chat[y][x] == '~' || chat[y][x] == 'h' ||chat[y][x] == 'h' || chat[y][x] == '=' ||
                 chat[y][x] == '8' || chat[y][x] == '&'){
                        spell_room[y][x] = true;
                        if (chat[y][x] != '@' && chat[y][x] != '<' && chat[y][x] != '|' &&
                         chat[y][x] != '_' && chat[y][x] != '+' && chat[y][x] != '?' && chat[y][x] != '=' &&
                        chat[y][x] == 'U' && chat[y][x] == 'D' && chat[y][x] == 'S' && chat[y][x] == 'F' && chat[y][x] == 'G') {
                            chat[y][x] = '.';
                        }
                   }
            }
        }
    int stairs_x, stairs_y;
    do {
        stairs_x = rnd(rp->r_max.x) + rp->r_pos.x;
        stairs_y = rnd(rp->r_max.y) + rp->r_pos.y;
    } while (chat[stairs_y][stairs_x] != '.');

    chat[stairs_y][stairs_x] = '<'; 
    

    // Place spells randomly within the room
    for (int i = 0; i < MAX_SPELLS; i++) {
        for (int j = 0; j < spells[i].count; j++) {
            int x, y;
            do {
                x = rnd(rp->r_max.x) + rp->r_pos.x;
                y = rnd(rp->r_max.y) + rp->r_pos.y;
            } while (chat[y][x] != '.');
            chat[y][x] = spells[i].symbol;
        }
    }
}
bool check_spell_room(int x, int y) {
    return spell_room[y][x];
}
void lock_door_and_update_room() {
    for (int y = 1; y < NUMLINES - 1; y++) {
        for (int x = 1; x < NUMCOLS - 1; x++) {
            if (chat[y][x] == '+') {
                chat[y][x] = '8';
                
                
                return; 
            }
        }
    }
}

void update_corner_of_room_with_locked_door() {

    int door_x = -1;
    int door_y = -1;
    for (int y = 1; y < NUMLINES - 1; y++) {
        for (int x = 1; x < NUMCOLS - 1; x++) {
            if (chat[y][x] == '8') {
                door_x = x;
                door_y = y;
                break;
            }
        }
        if (door_x != -1 && door_y != -1) break;
    }

    if (door_x == -1 || door_y == -1) {
        return; 
    }

    struct room *locked_room = NULL;
    for (int i = 0; i < MAXROOMS; i++) {
        if (door_x >= rooms[i].r_pos.x && door_x < rooms[i].r_pos.x + rooms[i].r_max.x &&
            door_y >= rooms[i].r_pos.y && door_y < rooms[i].r_pos.y + rooms[i].r_max.y) {
            locked_room = &rooms[i];
            break;
        }
    }

    if (locked_room == NULL) {
        return;
    }

    for (int y = locked_room->r_pos.y; y < locked_room->r_pos.y + locked_room->r_max.y; y++) {
        for (int x = locked_room->r_pos.x; x < locked_room->r_pos.x + locked_room->r_max.x; x++) {
            if (chat[y][x] == FLOOR) {
                if ((chat[y][x-1] == WALL_VERT || chat[y][x+1] == WALL_VERT) &&
                    (chat[y-1][x] == WALL_HORZ || chat[y+1][x] == WALL_HORZ)) {
                    chat[y][x] = '&';
                    mvaddch(y, x, '&'); 
                    return; 
                }
            }
        }
    }
}
void generate_random_password2(char* password, int length) {
    char charset[] = "0123456789"; 
    int charset_size = strlen(charset);
    srand(time(NULL)); 

    for (int i = 0; i < length; i++) {
        password[i] = charset[rand() % charset_size];
    }
    password[length] = '\0'; 
}

void display_password_for_30_seconds() {
    generate_random_password2(door_pass, 4);
    mvprintw(1, 1, "pass: %s", door_pass); 
    refresh(); 
    sleep(10); 
    mvprintw(1, 1, "                    "); 
    refresh(); 
}


void ask_for_password() {
    int attempts = 0;
    char input[5];

    while (1) {
        clear();
        mvprintw(0, 0, "Enter the password: ");
        refresh();

        echo();
        curs_set(TRUE);
        mvgetnstr(1, 0, input, 4);
        noecho();
        curs_set(FALSE);

        if (strcmp(input, door_pass) == 0) {
            mvprintw(2, 0, "Password is correct!");
            refresh();
            sleep(2);
            break;
        } else {
            attempts++;
            if (attempts == 1) {
                attron(COLOR_PAIR(5)); 
                mvprintw(2, 0, "Incorrect password! Please try again.");
                attroff(COLOR_PAIR(5));
            } else if (attempts == 2) {
                attron(COLOR_PAIR(24));
                mvprintw(2, 0, "Incorrect password again! Please be careful.");
                attroff(COLOR_PAIR(24));
            } else if (attempts == 3) {
                attron(COLOR_PAIR(25)); 
                mvprintw(2, 0, "Incorrect password three times! Please wait for 10 seconds.");
                attroff(COLOR_PAIR(25));
                refresh();
                sleep(10);
                attempts = 0;
            }
            refresh();
            sleep(2);
        }
    }
}
