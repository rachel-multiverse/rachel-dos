/*
 * RACHEL.C - DOS VERSION
 * Compatible with Turbo C 2.0, Borland C++ 3.1, DJGPP
 * 
 * Simplified for maximum compatibility
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __TURBOC__
    #include <conio.h>
    #include <dos.h>
#endif

/* Card structure */
typedef struct {
    int rank;  /* 2-14 */
    int suit;  /* 0-3 */
} Card;

/* Game state */
Card deck[52];
int deck_count;
Card player_hand[13];
int player_count;
Card cpu_hand[13];
int cpu_count;
Card top_card;
Card discard[52];
int discard_count;

/* Function prototypes */
void init_game(void);
void shuffle_deck(void);
void deal_cards(void);
void show_game(void);
void print_card(Card c);
int can_play(Card c);
void play_card(Card *hand, int *count, int idx);
void draw_card(Card *hand, int *count);
void player_turn(void);
void cpu_turn(void);
void clear_screen(void);
char get_key(void);

/* Clear screen - works on DOS and Unix */
void clear_screen(void) {
#ifdef __TURBOC__
    clrscr();
#else
    system("cls 2>NUL || clear");
#endif
}

/* Get single key - works on DOS and Unix */
char get_key(void) {
#ifdef __TURBOC__
    return getch();
#else
    return getchar();
#endif
}

/* Initialize game */
void init_game(void) {
    int i, s, r, idx = 0;
    
    /* Create deck */
    for (s = 0; s < 4; s++) {
        for (r = 2; r <= 14; r++) {
            deck[idx].suit = s;
            deck[idx].rank = r;
            idx++;
        }
    }
    deck_count = 52;
    
    /* Shuffle and deal */
    shuffle_deck();
    deal_cards();
    
    /* First card to discard */
    top_card = deck[--deck_count];
    discard[0] = top_card;
    discard_count = 1;
}

/* Shuffle deck */
void shuffle_deck(void) {
    int i, j;
    Card temp;
    
    srand((unsigned)time(NULL));
    
    for (i = deck_count - 1; i > 0; i--) {
        j = rand() % (i + 1);
        temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}

/* Deal 7 cards to each player */
void deal_cards(void) {
    int i;
    
    player_count = 0;
    cpu_count = 0;
    
    for (i = 0; i < 7; i++) {
        player_hand[player_count++] = deck[--deck_count];
        cpu_hand[cpu_count++] = deck[--deck_count];
    }
}

/* Print a card */
void print_card(Card c) {
    char *ranks = " 23456789TJQKA";
    char *suits = "HDCS";
    
    printf("%c%c", ranks[c.rank - 1], suits[c.suit]);
}

/* Check if card can be played */
int can_play(Card c) {
    /* Match rank, suit, or 8 is wild */
    return (c.rank == top_card.rank || 
            c.suit == top_card.suit ||
            c.rank == 8);
}

/* Play a card */
void play_card(Card *hand, int *count, int idx) {
    int i;
    
    /* Put on discard pile */
    top_card = hand[idx];
    discard[discard_count++] = top_card;
    
    /* Remove from hand */
    for (i = idx; i < *count - 1; i++) {
        hand[i] = hand[i + 1];
    }
    (*count)--;
}

/* Draw a card */
void draw_card(Card *hand, int *count) {
    /* Reshuffle if needed */
    if (deck_count == 0) {
        int i;
        Card saved = top_card;
        
        /* Move discard to deck */
        for (i = 0; i < discard_count - 1; i++) {
            deck[i] = discard[i];
        }
        deck_count = discard_count - 1;
        
        /* Reset discard */
        discard[0] = saved;
        discard_count = 1;
        
        shuffle_deck();
    }
    
    /* Draw if possible */
    if (deck_count > 0 && *count < 13) {
        hand[(*count)++] = deck[--deck_count];
    }
}

/* Show game state */
void show_game(void) {
    int i;
    
    clear_screen();
    
    printf("=============================\n");
    printf("    RACHEL - DOS EDITION\n");
    printf("=============================\n\n");
    
    printf("CPU: %d cards\n", cpu_count);
    printf("Top: ");
    print_card(top_card);
    printf("\n");
    printf("Deck: %d cards\n\n", deck_count);
    
    printf("Your hand:\n");
    for (i = 0; i < player_count; i++) {
        printf("%d.", i + 1);
        print_card(player_hand[i]);
        printf(" ");
        if ((i + 1) % 6 == 0) printf("\n");
    }
    printf("\n\n");
}

/* Player's turn */
void player_turn(void) {
    char key;
    int choice;
    
    printf("Your turn (1-%d play, D draw, Q quit): ", player_count);
    
    key = get_key();
    
    if (key == 'q' || key == 'Q') {
        printf("\nThanks for playing!\n");
        exit(0);
    }
    
    if (key == 'd' || key == 'D') {
        draw_card(player_hand, &player_count);
        return;
    }
    
    if (key >= '1' && key <= '9') {
        choice = key - '0';
        if (choice <= player_count) {
            if (can_play(player_hand[choice - 1])) {
                play_card(player_hand, &player_count, choice - 1);
            } else {
                printf("\nCan't play that card!");
                get_key();
            }
        }
    }
}

/* CPU's turn */
void cpu_turn(void) {
    int i;
    
    printf("CPU thinking...\n");
    
    /* Try to play a card */
    for (i = 0; i < cpu_count; i++) {
        if (can_play(cpu_hand[i])) {
            printf("CPU plays ");
            print_card(cpu_hand[i]);
            printf("\n");
            play_card(cpu_hand, &cpu_count, i);
            printf("Press any key...");
            get_key();
            return;
        }
    }
    
    /* Must draw */
    printf("CPU draws a card\n");
    draw_card(cpu_hand, &cpu_count);
    printf("Press any key...");
    get_key();
}

/* Main game */
int main(void) {
    int turn = 0;
    
    clear_screen();
    printf("RACHEL - The Card Game\n");
    printf("DOS Edition - Platform #004\n\n");
    printf("Match rank or suit to play\n");
    printf("8s are wild!\n\n");
    printf("Press any key to start...");
    get_key();
    
    init_game();
    
    /* Game loop */
    while (player_count > 0 && cpu_count > 0) {
        show_game();
        
        if (turn == 0) {
            player_turn();
        } else {
            cpu_turn();
        }
        
        /* Switch turns */
        turn = 1 - turn;
    }
    
    /* Game over */
    clear_screen();
    if (player_count == 0) {
        printf("\n\n    YOU WIN!\n\n");
    } else {
        printf("\n\n    CPU WINS!\n\n");
    }
    printf("Press any key to exit...");
    get_key();
    
    return 0;
}