/*
 * RACHEL DOS EDITION - PLATFORM #002
 * 
 * CGA graphics in all their 4-color glory!
 * Tested on DOS 3.3 through FreeDOS
 * 
 * "640K ought to be enough for anybody to play Rachel"
 */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include <string.h>
#include "rules.h"

/* CGA video memory */
#define CGA_MEMORY 0xB8000000L
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

/* CGA colors */
#define COLOR_BLACK     0x00
#define COLOR_BLUE      0x01
#define COLOR_GREEN     0x02
#define COLOR_CYAN      0x03
#define COLOR_RED       0x04
#define COLOR_MAGENTA   0x05
#define COLOR_BROWN     0x06
#define COLOR_WHITE     0x07
#define COLOR_BRIGHT    0x08
#define COLOR_BLINK     0x80

/* Card colors */
#define CARD_RED        (COLOR_RED | COLOR_BRIGHT)
#define CARD_BLACK      COLOR_WHITE
#define CARD_BACK       COLOR_CYAN

/* DOS-specific functions */
void set_video_mode(int mode);
void clear_screen(void);
void gotoxy(int x, int y);
void textcolor(int color);
void draw_box(int x, int y, int width, int height);
void draw_card(int x, int y, Card card);
void draw_card_back(int x, int y);
void draw_title(void);
void wait_key(void);
int get_key(void);

/* Game state */
static Game game;
static int current_selection = 0;

/* Clear screen using BIOS */
void clear_screen(void) {
    union REGS regs;
    regs.h.ah = 0x06;  /* Scroll up */
    regs.h.al = 0x00;  /* Clear entire screen */
    regs.h.bh = 0x07;  /* White on black */
    regs.h.ch = 0x00;  /* Top row */
    regs.h.cl = 0x00;  /* Left column */
    regs.h.dh = 24;    /* Bottom row */
    regs.h.dl = 79;    /* Right column */
    int86(0x10, &regs, &regs);
    gotoxy(1, 1);
}

/* Set cursor position */
void gotoxy(int x, int y) {
    union REGS regs;
    regs.h.ah = 0x02;
    regs.h.bh = 0x00;
    regs.h.dh = y - 1;
    regs.h.dl = x - 1;
    int86(0x10, &regs, &regs);
}

/* Set text color */
void textcolor(int color) {
    textattr(color);
}

/* Draw a box using DOS box drawing characters */
void draw_box(int x, int y, int width, int height) {
    int i;
    
    gotoxy(x, y);
    putch(218);  /* Top-left corner */
    for (i = 0; i < width - 2; i++) putch(196);  /* Horizontal line */
    putch(191);  /* Top-right corner */
    
    for (i = 1; i < height - 1; i++) {
        gotoxy(x, y + i);
        putch(179);  /* Vertical line */
        gotoxy(x + width - 1, y + i);
        putch(179);  /* Vertical line */
    }
    
    gotoxy(x, y + height - 1);
    putch(192);  /* Bottom-left corner */
    for (i = 0; i < width - 2; i++) putch(196);  /* Horizontal line */
    putch(217);  /* Bottom-right corner */
}

/* Draw ASCII art card */
void draw_card(int x, int y, Card card) {
    uint8_t suit = GET_SUIT(card.encoded);
    uint8_t rank = GET_RANK(card.encoded);
    char rank_char;
    char suit_char;
    int color;
    
    /* Determine rank character */
    if (rank <= 9) {
        rank_char = '0' + rank;
    } else {
        switch(rank) {
            case 10: rank_char = 'T'; break;
            case RANK_JACK: rank_char = 'J'; break;
            case RANK_QUEEN: rank_char = 'Q'; break;
            case RANK_KING: rank_char = 'K'; break;
            case RANK_ACE: rank_char = 'A'; break;
            default: rank_char = '?'; break;
        }
    }
    
    /* Determine suit and color */
    switch(suit) {
        case SUIT_HEARTS:
            suit_char = 3;  /* ASCII heart */
            color = CARD_RED;
            break;
        case SUIT_DIAMONDS:
            suit_char = 4;  /* ASCII diamond */
            color = CARD_RED;
            break;
        case SUIT_CLUBS:
            suit_char = 5;  /* ASCII club */
            color = CARD_BLACK;
            break;
        case SUIT_SPADES:
            suit_char = 6;  /* ASCII spade */
            color = CARD_BLACK;
            break;
        default:
            suit_char = '?';
            color = COLOR_WHITE;
    }
    
    /* Draw the card */
    textcolor(COLOR_WHITE);
    draw_box(x, y, 7, 5);
    
    textcolor(color);
    gotoxy(x + 2, y + 1);
    if (rank == 10) {
        printf("10");
    } else {
        putch(rank_char);
        putch(' ');
    }
    
    gotoxy(x + 3, y + 2);
    putch(suit_char);
    
    gotoxy(x + 2, y + 3);
    if (rank == 10) {
        printf("10");
    } else {
        putch(' ');
        putch(rank_char);
    }
}

/* Draw card back */
void draw_card_back(int x, int y) {
    textcolor(CARD_BACK);
    draw_box(x, y, 7, 5);
    gotoxy(x + 2, y + 2);
    printf("###");
}

/* Draw title screen */
void draw_title(void) {
    clear_screen();
    textcolor(COLOR_WHITE | COLOR_BRIGHT);
    
    gotoxy(20, 5);
    printf("╔═══════════════════════════════════════╗");
    gotoxy(20, 6);
    printf("║            R A C H E L                ║");
    gotoxy(20, 7);
    printf("║         DOS Edition v1.0              ║");
    gotoxy(20, 8);
    printf("║         Platform #002 of ∞            ║");
    gotoxy(20, 9);
    printf("╚═══════════════════════════════════════╝");
    
    textcolor(COLOR_CYAN);
    gotoxy(25, 12);
    printf("CGA Graphics in 4-color glory!");
    
    textcolor(COLOR_WHITE);
    gotoxy(22, 15);
    printf("1. Play against AI");
    gotoxy(22, 16);
    printf("2. Two player game");
    gotoxy(22, 17);
    printf("3. View rules");
    gotoxy(22, 18);
    printf("ESC. Exit to DOS");
    
    textcolor(COLOR_YELLOW);
    gotoxy(20, 22);
    printf("Press a key to select...");
}

/* Main game display */
void draw_game_state(void) {
    int i, x, y;
    Player* current;
    Card top_card;
    
    clear_screen();
    
    /* Draw header */
    textcolor(COLOR_WHITE | COLOR_BRIGHT);
    draw_box(1, 1, 79, 3);
    gotoxy(30, 2);
    printf("RACHEL - Turn %d", game.turn_count);
    
    /* Draw opponents */
    y = 5;
    for (i = 0; i < game.player_count; i++) {
        if (i != game.current_player_index) {
            gotoxy(3, y);
            if (game.players[i].is_out) {
                textcolor(COLOR_RED);
                printf("%s: OUT", game.players[i].name);
            } else {
                textcolor(COLOR_WHITE);
                printf("%s: %d cards", game.players[i].name, 
                       game.players[i].hand_count);
                
                /* Draw card backs */
                x = 30;
                for (int j = 0; j < game.players[i].hand_count && j < 5; j++) {
                    draw_card_back(x, y - 1);
                    x += 8;
                }
            }
            y += 6;
        }
    }
    
    /* Draw discard pile */
    textcolor(COLOR_WHITE);
    gotoxy(35, 12);
    printf("DISCARD");
    if (game.discard_count > 0) {
        top_card = game.discard_pile[game.discard_count - 1];
        draw_card(36, 13, top_card);
    }
    
    /* Draw deck */
    textcolor(COLOR_WHITE);
    gotoxy(50, 12);
    printf("DECK: %d", game.deck_count);
    draw_card_back(50, 13);
    
    /* Draw current player's hand */
    current = &game.players[game.current_player_index];
    textcolor(COLOR_YELLOW);
    gotoxy(3, 19);
    printf("Your hand (%s):", current->name);
    
    x = 3;
    for (i = 0; i < current->hand_count && i < 9; i++) {
        if (i == current_selection) {
            textcolor(COLOR_YELLOW | COLOR_BRIGHT);
            gotoxy(x, 24);
            putch('^');
        }
        draw_card(x, 20, current->hand[i]);
        x += 8;
    }
    
    /* Status line */
    textcolor(COLOR_WHITE);
    gotoxy(1, 25);
    printf("←→:Select  ENTER:Play  D:Draw  ESC:Menu");
}

/* Get keyboard input */
int get_key(void) {
    int ch = getch();
    if (ch == 0 || ch == 224) {  /* Extended key */
        ch = getch();
        return ch + 256;
    }
    return ch;
}

/* Wait for any key */
void wait_key(void) {
    while (!kbhit());
    getch();
}

/* Simple AI turn */
void ai_turn(void) {
    Player* player = &game.players[game.current_player_index];
    Card valid_cards[MAX_HAND_SIZE];
    uint8_t valid_count;
    Card play_card;
    
    gotoxy(20, 12);
    textcolor(COLOR_CYAN);
    printf("%s is thinking...", player->name);
    delay(1000);  /* DOS delay function */
    
    valid_count = rachel_get_valid_plays(&game, valid_cards);
    
    if (valid_count == 0) {
        rachel_draw_cards(&game, player->id, 1);
    } else {
        play_card = valid_cards[0];
        rachel_play_cards(&game, player->id, &play_card, 1, SUIT_HEARTS);
    }
    
    rachel_next_turn(&game);
}

/* Main game loop */
void play_game(int num_ai) {
    char name[32];
    int key;
    Player* current;
    Card selected_card;
    
    /* Initialize game */
    rachel_init_game(&game, 1 + num_ai);
    
    /* Get player name */
    clear_screen();
    gotoxy(20, 10);
    printf("Enter your name: ");
    scanf("%31s", name);
    rachel_add_player(&game, name, FALSE);
    
    /* Add AI players */
    for (int i = 0; i < num_ai; i++) {
        sprintf(name, "DOS_AI_%d", i + 1);
        rachel_add_player(&game, name, TRUE);
    }
    
    /* Start game */
    rachel_start_game(&game);
    
    /* Game loop */
    while (!rachel_is_game_over(&game)) {
        draw_game_state();
        
        current = &game.players[game.current_player_index];
        
        if (current->is_ai) {
            ai_turn();
        } else {
            /* Human player */
            while (1) {
                key = get_key();
                
                if (key == 27) {  /* ESC */
                    return;
                } else if (key == 331) {  /* Left arrow */
                    if (current_selection > 0) {
                        current_selection--;
                        draw_game_state();
                    }
                } else if (key == 333) {  /* Right arrow */
                    if (current_selection < current->hand_count - 1) {
                        current_selection++;
                        draw_game_state();
                    }
                } else if (key == 13) {  /* Enter - play card */
                    selected_card = current->hand[current_selection];
                    if (rachel_play_cards(&game, current->id, 
                                         &selected_card, 1, SUIT_HEARTS)) {
                        rachel_next_turn(&game);
                        current_selection = 0;
                        break;
                    }
                } else if (key == 'd' || key == 'D') {  /* Draw */
                    if (!rachel_must_play(&game, current->id)) {
                        rachel_draw_cards(&game, current->id, 1);
                        rachel_next_turn(&game);
                        break;
                    }
                }
            }
        }
    }
    
    /* Game over */
    clear_screen();
    textcolor(COLOR_YELLOW | COLOR_BRIGHT);
    gotoxy(30, 10);
    printf("GAME OVER!");
    gotoxy(25, 15);
    printf("Press any key to continue...");
    wait_key();
}

/* Main function */
int main(void) {
    int choice;
    
    /* Test for CGA/EGA/VGA */
    if (!rachel_self_test()) {
        printf("Self test failed! The cards refuse to be dealt.\n");
        return 1;
    }
    
    while (1) {
        draw_title();
        choice = get_key();
        
        switch(choice) {
            case '1':
                play_game(1);  /* 1 AI opponent */
                break;
            case '2':
                play_game(0);  /* 2 player hotseat */
                break;
            case '3':
                clear_screen();
                printf("RACHEL RULES:\n\n");
                printf("1. You must play if you can\n");
                printf("2. Match suit or rank\n");
                printf("3. 2s make next player draw 2\n");
                printf("4. 7s skip next player\n");
                printf("5. Queens reverse direction\n");
                printf("6. Aces let you pick suit\n");
                printf("7. Black Jacks = draw 5\n");
                printf("8. Red Jacks counter Black Jacks\n");
                printf("\nPress any key...");
                wait_key();
                break;
            case 27:  /* ESC */
                clear_screen();
                textcolor(COLOR_WHITE);
                printf("Thanks for playing Rachel!\n");
                printf("Platform #002 complete.\n");
                printf("198 platforms to go...\n");
                return 0;
        }
    }
    
    return 0;
}

/*
 * End of DOS Rachel.
 * 
 * CGA graphics have never looked so good.
 * 640K is definitely enough for anybody.
 * The cards persist in 16-bit real mode.
 * 
 * "Platform 2 of ∞ complete."
 */