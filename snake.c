#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <time.h>
#include <string.h>

#define MAX_SCORE 256
#define FRAME_TIME 200000

typedef struct {
    int x;
    int y;
} vec2;

typedef struct Node {
    vec2 position;
    char data[256];
    struct Node *prev;
    struct Node *next;
} Node;

int  score = 0;
char score_message[16];

bool skip = false;
bool is_running = true;

int screen_width = 25;
int screen_height = 20;

// initialize screen
WINDOW *win;
WINDOW *input_win;

// User input
char user_input[256];
char last_input[256];

// snake
vec2 head = { 0, 0 };
vec2 dir = { 1, 0 };

// Node snake_body;
Node *Head;
Node *tail;

// berry
vec2 berry;

void init_input_window() {
    input_win = newwin(1, screen_width * 2 + 1, screen_height + 2, 0); // 1 row, full width
    wrefresh(input_win);
}

// Function to get user input
void get_user_input() {
    // Clear previous input
    memset(user_input, 0, sizeof(user_input));
    mvwprintw(input_win, 0, 1, "Enter your Data: ");
    wrefresh(input_win);
    
    // Get user input
    wgetnstr(input_win, user_input, sizeof(user_input) - 1);
    wclear(input_win); // Clear input window after getting input

    // Store the latest input
    strncpy(last_input, user_input, sizeof(last_input) - 1);
    last_input[sizeof(last_input) - 1] = '\0'; // Ensure null-termination

}

void init_snake_body() {
    Head = (Node*)malloc(sizeof(Node));
    Head->position = head;
    Head->next = NULL;
    Head->prev = NULL;  
    tail = Head;  
}

Node *create_node(char data[256])
{
    Node * new_node = (Node*)malloc(sizeof(Node));
    strncpy(new_node->data, data, sizeof(new_node->data) - 1);
    new_node->data[sizeof(new_node->data) - 1] = '\0';
    new_node->next = NULL;
    return new_node;
}

void insertNode(char data[256]) {
    Node* newNode = create_node(data);
    if (Head == NULL) {
        Head = newNode;
        tail = newNode;
    }
    tail->next = newNode;
    newNode->prev = tail;
    tail = newNode;
}

bool collide(vec2 a, vec2 b) {
    if (a.x == b.x && a.y == b.y) {
        return true;
    }
    else return false;
}

bool collide_snake_body(vec2 point) {
    Node *temp = Head->next;
    while (temp!=NULL)
    {
        if (collide(point, temp->position)) {
            return true;
        }
        temp = temp->next;
    }
    return false;
}

vec2 spawn_berry() {
    // spawn a new berry with 1 pixel padding from edges and not inside of the snake
    vec2 berry = { 1 + rand() % (screen_width - 2), 1 + rand() % (screen_height - 2) };
    while (collide(head, berry) || collide_snake_body(berry)) {
        berry.x = 1 + rand() % (screen_width - 2);
        berry.y = 1 + rand() % (screen_height - 2);
    }
    return berry;
}

void draw_border(int y, int x, int width, int height) {
    // top row
    mvaddch(y, x, ACS_ULCORNER);
    mvaddch(y, x + width * 2 + 1, ACS_URCORNER);
    for (int i = 1; i < width * 2 + 1; i++) {
        mvaddch(y, x + i, ACS_HLINE);
    }
    // vertical lines
    for (int i = 1; i < height + 1; i++) {
        mvaddch(y + i, x, ACS_VLINE);
        mvaddch(y + i, x + width * 2 + 1, ACS_VLINE);
    }
    // bottom row
    mvaddch(y + height + 1, x, ACS_LLCORNER);
    mvaddch(y + height + 1, x + width * 2 + 1, ACS_LRCORNER);
    for (int i = 1; i < width * 2 + 1; i++) {
        mvaddch(y + height + 1, x + i, ACS_HLINE);
    }
}

void free_snake() {
    Node *current = Head;
    while (current != NULL) {
        Node *next = current->next;
        free(current);
        current = next;
    }
}

void quit_game() {
    // exit cleanly from application
    endwin();
    // clear screen, place cursor on top, and un-hide cursor
    printf("\e[1;1H\e[2J");
    printf("\e[?25h");
    free_snake();
    exit(0);
}

void restart_game() {
    Head->position.x = 0;
    Head->position.y = 0;
    dir.x = 1;
    dir.y = 0;
    score = 0;
    sprintf(score_message, "[ Score: %d ]", score);
    is_running = true;
}

void init() {
    init_snake_body();
    srand(time(NULL));
    // initialize window
    win = initscr();
    init_input_window();
    // take player input and hide cursor
    keypad(win, true);
    noecho();
    nodelay(win, true);
    curs_set(0);

    // initialize color
    if (has_colors() == FALSE) {
        endwin();
        fprintf(stderr, "Your terminal does not support color\n");
        exit(1);
    }
    start_color();
    use_default_colors();
    init_pair(1, COLOR_RED, -1);
    init_pair(2, COLOR_GREEN, -1);
    init_pair(3, COLOR_YELLOW, -1);

    berry.x = rand() % screen_width;
    berry.y = rand() % screen_height;

    // update score message
    sprintf(score_message, "[ Score: %d ]", score);
    // get starting value
    get_user_input();

}

void process_input() {
    int pressed = wgetch(win);
    if (pressed == KEY_LEFT) {
        if (dir.x == 1) {
            return;
            skip = true;
        }
        dir.x = -1;
        dir.y = 0;
    }
    if (pressed == KEY_RIGHT) {
        if (dir.x == -1) {
            return;
            skip = true;
        }
        dir.x = 1;
        dir.y = 0;
    }
    if (pressed == KEY_UP) {
        if (dir.y == 1) {
            return;
            skip = true;
        }
        dir.x = 0;
        dir.y = -1;
    }
    if (pressed == KEY_DOWN) {
        if (dir.y == -1) {
            return;
            skip = true;
        }
        dir.x = 0;
        dir.y = 1;
    }
    if (pressed == ' ') {
        if (!is_running)
            restart_game();
    }
    if (pressed == '\e') {
        is_running = false;
        quit_game();
    }
}

void game_over() {
    while (is_running == false) {
        process_input();

        mvaddstr(screen_height / 2, screen_width - 16, "              Game Over          ");
        mvaddstr(screen_height / 2 + 1, screen_width - 16, "[SPACE] to restart, [ESC] to quit ");
        attron(COLOR_PAIR(3));
        draw_border(screen_height / 2 - 1, screen_width - 17, 17, 2);
        attroff(COLOR_PAIR(3));

        usleep(FRAME_TIME);
    }
}

void update() {

    // collide with body
    if (collide_snake_body(Head->position)) {
        is_running = false;
        game_over();
    }

    // eating a berry
    if (collide(Head->position, berry)) {
        if (score < MAX_SCORE) {
            score += 1;
            sprintf(score_message, "[ Score: %d ]", score);
            insertNode(last_input);
            get_user_input();

        }
        else {
            // WIN!
            printf("You Win!");
        }
        berry = spawn_berry();
    }

    // move snake
    Node *temp = tail;
    while (temp->prev != NULL) {
        temp->position = temp->prev->position;
        temp = temp->prev;
    }
    
    Head->position.x += dir.x;
    Head->position.y += dir.y;

    // Wrap-around logic for walls
    if (Head->position.x < 0) {
        Head->position.x = screen_width - 1;
    } else if (Head->position.x >= screen_width) {
        Head->position.x = 0;
    }

    if (Head->position.y < 0) {
        Head->position.y = screen_height - 1;
    } else if (Head->position.y >= screen_height) {
        Head->position.y = 0;
    }

    usleep(FRAME_TIME);
}

void draw() {
    erase();

    attron(COLOR_PAIR(1));
    mvaddch(berry.y+1, berry.x * 2+1, user_input[0]);
    attroff(COLOR_PAIR(1));

    // draw snake
    attron(COLOR_PAIR(2));
    Node *temp = Head->next; // no need to draw head
    while (temp!=NULL)
    {
        mvaddch(temp->position.y+1, temp->position.x * 2 + 1, temp->data[0]);
        temp = temp->next;
    }
    
    mvaddch(Head->position.y+1, Head->position.x * 2+1, 'O'); // keep snake head same 
    attroff(COLOR_PAIR(2));

    attron(COLOR_PAIR(3));
    draw_border(0, 0, screen_width, screen_height);
    attroff(COLOR_PAIR(3));
    mvaddstr(0, screen_width - 5, score_message);
}

int main(int argc, char *argv[]) {

    init();
    while(is_running) {
        process_input();
        if (skip == true) {
            skip = false;
            continue;
        }

        update();
        draw();
    }
    quit_game();
    return 0;
}
