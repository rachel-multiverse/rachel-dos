/*
 * RACHEL PORTABLE VERSION
 * 
 * A version that compiles on modern systems for testing
 * Before we build the real DOS version
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "rules.h"

#ifdef _WIN32
    #include <conio.h>
#else
    /* Unix/Mac compatibility */
    #include <termios.h>
    #include <unistd.h>
    
    int getch(void) {
        struct termios oldattr, newattr;
        int ch;
        tcgetattr(STDIN_FILENO, &oldattr);
        newattr = oldattr;
        newattr.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
        return ch;
    }
    
    void clrscr(void) {
        printf("\033[2J\033[H");
    }
#endif

/* Game state */
typedef struct {
    Card deck[52];
    int deck_count;
    Card player_hand[MAX_HAND_SIZE];
    int player_count;
    Card cpu_hand[MAX_HAND_SIZE];
    int cpu_count;
    Card discard_pile[52];
    int discard_count;
    Card top_card;
    int current_player; /* 0 = human, 1 = CPU */
    int game_over;
} GameState;

GameState game;

/* Function prototypes */
void init_game(void);
void shuffle_deck(void);
void deal_cards(void);
void draw_screen(void);
void draw_hand(Card *hand, int count, int y, int show_cards);
void player_turn(void);
void cpu_turn(void);
int play_card(Card *hand, int *count, int index);
void draw_card(Card *hand, int *count);
void print_card(Card c);
char* get_rank_string(Rank r);
char* get_suit_string(Suit s);

void init_game(void) {
    int i;
    
    /* Initialize standard deck */
    game.deck_count = 0;
    for (i = 0; i < 52; i++) {
        game.deck[game.deck_count].rank = (i % 13) + 1;
        game.deck[game.deck_count].suit = i / 13;
        game.deck_count++;
    }
    
    /* Shuffle */
    shuffle_deck();
    
    /* Deal cards */
    deal_cards();
    
    /* Set up discard pile with top card */
    game.top_card = game.deck[--game.deck_count];
    game.discard_pile[0] = game.top_card;
    game.discard_count = 1;
    
    game.current_player = 0;
    game.game_over = 0;
}

void shuffle_deck(void) {
    int i, j;
    Card temp;
    
    srand(time(NULL));
    
    for (i = game.deck_count - 1; i > 0; i--) {
        j = rand() % (i + 1);
        temp = game.deck[i];
        game.deck[i] = game.deck[j];
        game.deck[j] = temp;
    }
}

void deal_cards(void) {
    int i;
    
    game.player_count = 0;
    game.cpu_count = 0;
    
    for (i = 0; i < STARTING_HAND_SIZE; i++) {
        game.player_hand[game.player_count++] = game.deck[--game.deck_count];
        game.cpu_hand[game.cpu_count++] = game.deck[--game.deck_count];
    }
}

void draw_screen(void) {
    clrscr();
    
    printf("=================================\n");
    printf("         RACHEL CARD GAME        \n");
    printf("=================================\n\n");
    
    /* CPU hand (hidden) */
    printf("CPU: %d cards\n", game.cpu_count);
    draw_hand(game.cpu_hand, game.cpu_count, 5, 0);
    
    /* Discard pile */
    printf("\n\nTop Card: ");
    print_card(game.top_card);
    printf("\n");
    printf("Deck: %d cards remaining\n\n", game.deck_count);
    
    /* Player hand */
    printf("Your Hand:\n");
    draw_hand(game.player_hand, game.player_count, 15, 1);
    
    if (game.current_player == 0) {
        printf("\nYour turn! (1-%d to play, D to draw, Q to quit): ", game.player_count);
    } else {
        printf("\nCPU is thinking...\n");
    }
}

void draw_hand(Card *hand, int count, int y, int show_cards) {
    int i;
    
    for (i = 0; i < count; i++) {
        if (show_cards) {
            printf("%d. ", i + 1);
            print_card(hand[i]);
            printf("  ");
        } else {
            printf("[?] ");
        }
    }
    printf("\n");
}

void print_card(Card c) {
    printf("%s%s", get_rank_string(c.rank), get_suit_string(c.suit));
}

char* get_rank_string(Rank r) {
    static char buf[3];
    switch(r) {
        case ACE: return "A";
        case JACK: return "J";
        case QUEEN: return "Q";
        case KING: return "K";
        default:
            sprintf(buf, "%d", r);
            return buf;
    }
}

char* get_suit_string(Suit s) {
    switch(s) {
        case HEARTS: return "♥";
        case DIAMONDS: return "♦";
        case CLUBS: return "♣";
        case SPADES: return "♠";
        default: return "?";
    }
}

int can_play_card(Card card) {
    /* Check if card matches rank or suit of top card */
    return (card.rank == game.top_card.rank || 
            card.suit == game.top_card.suit ||
            card.rank == EIGHT); /* 8s are wild */
}

void player_turn(void) {
    char input;
    int choice;
    int valid = 0;
    
    while (!valid) {
        input = getch();
        
        if (input == 'q' || input == 'Q') {
            game.game_over = 1;
            return;
        }
        
        if (input == 'd' || input == 'D') {
            draw_card(game.player_hand, &game.player_count);
            game.current_player = 1;
            return;
        }
        
        if (input >= '1' && input <= '9') {
            choice = input - '1';
            if (choice < game.player_count) {
                if (can_play_card(game.player_hand[choice])) {
                    if (play_card(game.player_hand, &game.player_count, choice)) {
                        if (game.player_count == 0) {
                            printf("\nYOU WIN!\n");
                            game.game_over = 1;
                        } else {
                            game.current_player = 1;
                        }
                        return;
                    }
                } else {
                    printf("\nCan't play that card! Try again: ");
                }
            }
        }
    }
}

void cpu_turn(void) {
    int i;
    
    /* Simple AI: play first valid card */
    for (i = 0; i < game.cpu_count; i++) {
        if (can_play_card(game.cpu_hand[i])) {
            play_card(game.cpu_hand, &game.cpu_count, i);
            if (game.cpu_count == 0) {
                printf("\nCPU WINS!\n");
                game.game_over = 1;
            } else {
                game.current_player = 0;
            }
            return;
        }
    }
    
    /* No valid card, must draw */
    draw_card(game.cpu_hand, &game.cpu_count);
    game.current_player = 0;
}

int play_card(Card *hand, int *count, int index) {
    int i;
    
    /* Place card on discard pile */
    game.top_card = hand[index];
    game.discard_pile[game.discard_count++] = game.top_card;
    
    /* Remove card from hand */
    for (i = index; i < *count - 1; i++) {
        hand[i] = hand[i + 1];
    }
    (*count)--;
    
    /* Handle special cards */
    if (game.top_card.rank == TWO) {
        /* Next player draws 2 */
        /* For simplicity, just switch turns */
    }
    
    return 1;
}

void draw_card(Card *hand, int *count) {
    if (game.deck_count == 0) {
        /* Reshuffle discard pile into deck */
        int i;
        Card top = game.top_card;
        
        for (i = 0; i < game.discard_count - 1; i++) {
            game.deck[i] = game.discard_pile[i];
        }
        game.deck_count = game.discard_count - 1;
        game.discard_count = 1;
        game.discard_pile[0] = top;
        
        shuffle_deck();
    }
    
    if (game.deck_count > 0 && *count < MAX_HAND_SIZE) {
        hand[(*count)++] = game.deck[--game.deck_count];
    }
}

int main(void) {
    printf("RACHEL - DOS Edition (Portable Test Build)\n");
    printf("=========================================\n\n");
    printf("Press any key to start...\n");
    getch();
    
    init_game();
    
    while (!game.game_over) {
        draw_screen();
        
        if (game.current_player == 0) {
            player_turn();
        } else {
            cpu_turn();
            printf("Press any key to continue...");
            getch();
        }
    }
    
    printf("\nGame Over! Press any key to exit...\n");
    getch();
    
    return 0;
}