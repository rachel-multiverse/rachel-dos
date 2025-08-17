/*
 * RACHEL FOR DOS - CORRECT IMPLEMENTATION
 * 
 * This time following the ACTUAL rules from GAME_RULES.md
 * No made-up rules, no shortcuts, the REAL game.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Card structure */
typedef struct {
    int rank;  /* 2-14 (Ace = 14) */
    int suit;  /* 0=Hearts, 1=Diamonds, 2=Clubs, 3=Spades */
} Card;

/* Game state */
typedef struct {
    Card deck[52];
    int deck_count;
    
    Card player_hand[52];  /* Can exceed starting size */
    int player_count;
    
    Card cpu_hand[52];
    int cpu_count;
    
    Card discard[52];
    int discard_count;
    
    Card top_card;
    int nominated_suit;     /* -1 if none, 0-3 if Ace played */
    
    int current_player;     /* 0=human, 1=CPU */
    int direction;          /* 1=clockwise, -1=counter */
    int draw_penalty;       /* Accumulated from 2s and Jacks */
    int skip_count;         /* Accumulated from 7s */
    
    int game_over;
} Game;

Game g;

/* Function prototypes */
void init_game(void);
void shuffle_deck(void);
void deal_cards(void);
void clear_screen(void);
void show_game(void);
void print_card(Card c);
void print_suit(int suit);
int can_play_card(Card c);
int can_counter_attack(Card c);
void play_cards(Card *hand, int *count, int *indices, int num_cards);
void draw_cards(Card *hand, int *count, int num);
void apply_special_effects(Card *cards, int num_cards);
void player_turn(void);
void cpu_turn(void);
void check_mandatory_play(Card *hand, int count, int *must_play);
int find_valid_plays(Card *hand, int count, int *valid_indices);
void next_player(void);

/* Clear screen */
void clear_screen(void) {
    system("cls 2>NUL || clear");
}

/* Initialize game */
void init_game(void) {
    int i, s, r, idx = 0;
    
    /* Create deck - ranks 2-14, suits 0-3 */
    for (s = 0; s < 4; s++) {
        for (r = 2; r <= 14; r++) {
            g.deck[idx].suit = s;
            g.deck[idx].rank = r;
            idx++;
        }
    }
    g.deck_count = 52;
    
    /* Initialize state */
    g.current_player = 0;
    g.direction = 1;  /* Clockwise */
    g.draw_penalty = 0;
    g.skip_count = 0;
    g.nominated_suit = -1;
    g.game_over = 0;
    
    /* Shuffle and deal */
    shuffle_deck();
    deal_cards();
    
    /* First card to discard (no effect) */
    g.top_card = g.deck[--g.deck_count];
    g.discard[0] = g.top_card;
    g.discard_count = 1;
}

/* Shuffle deck */
void shuffle_deck(void) {
    int i, j;
    Card temp;
    
    srand((unsigned)time(NULL));
    
    for (i = g.deck_count - 1; i > 0; i--) {
        j = rand() % (i + 1);
        temp = g.deck[i];
        g.deck[i] = g.deck[j];
        g.deck[j] = temp;
    }
}

/* Deal 7 cards to each player (for 2 players) */
void deal_cards(void) {
    int i;
    
    g.player_count = 0;
    g.cpu_count = 0;
    
    for (i = 0; i < 7; i++) {
        g.player_hand[g.player_count++] = g.deck[--g.deck_count];
        g.cpu_hand[g.cpu_count++] = g.deck[--g.deck_count];
    }
}

/* Print a card */
void print_card(Card c) {
    /* Rank display */
    switch(c.rank) {
        case 14: printf("A"); break;
        case 13: printf("K"); break;
        case 12: printf("Q"); break;
        case 11: printf("J"); break;
        case 10: printf("10"); break;
        default: printf("%d", c.rank); break;
    }
    
    /* Suit display */
    print_suit(c.suit);
}

/* Print suit */
void print_suit(int suit) {
    switch(suit) {
        case 0: printf("H"); break;  /* Hearts */
        case 1: printf("D"); break;  /* Diamonds */
        case 2: printf("C"); break;  /* Clubs */
        case 3: printf("S"); break;  /* Spades */
    }
}

/* Check if card can be played */
int can_play_card(Card c) {
    /* If there's a pending attack, special rules apply */
    if (g.draw_penalty > 0) {
        return can_counter_attack(c);
    }
    
    /* If suit was nominated by Ace */
    if (g.nominated_suit >= 0) {
        return (c.suit == g.nominated_suit || c.rank == 14); /* Match suit or play another Ace */
    }
    
    /* Normal play: match rank or suit */
    return (c.rank == g.top_card.rank || c.suit == g.top_card.suit);
}

/* Check if card can counter current attack */
int can_counter_attack(Card c) {
    /* Pending draw from 2s */
    if (g.top_card.rank == 2) {
        return c.rank == 2;  /* Only 2 can counter 2 */
    }
    
    /* Pending draw from Black Jack */
    if (g.top_card.rank == 11 && g.top_card.suit >= 2) {  /* Black Jack */
        /* Can play another Black Jack or Red Jack to reduce */
        return c.rank == 11;
    }
    
    return 0;
}

/* Play cards (handles stacking) */
void play_cards(Card *hand, int *count, int *indices, int num_cards) {
    int i, j;
    Card played[13];
    
    /* Collect cards being played */
    for (i = 0; i < num_cards; i++) {
        played[i] = hand[indices[i]];
    }
    
    /* Remove from hand (work backwards to preserve indices) */
    for (i = num_cards - 1; i >= 0; i--) {
        for (j = indices[i]; j < *count - 1; j++) {
            hand[j] = hand[j + 1];
        }
        (*count)--;
    }
    
    /* Place on discard pile */
    for (i = 0; i < num_cards; i++) {
        g.discard[g.discard_count++] = played[i];
    }
    g.top_card = played[num_cards - 1];
    
    /* Apply special effects */
    apply_special_effects(played, num_cards);
}

/* Apply special card effects */
void apply_special_effects(Card *cards, int num_cards) {
    int i;
    int black_jacks = 0, red_jacks = 0;
    int twos = 0, sevens = 0, queens = 0, aces = 0;
    
    /* Count special cards */
    for (i = 0; i < num_cards; i++) {
        switch(cards[i].rank) {
            case 2:  twos++; break;
            case 7:  sevens++; break;
            case 11: /* Jacks */
                if (cards[i].suit >= 2) black_jacks++;  /* Clubs/Spades */
                else red_jacks++;  /* Hearts/Diamonds */
                break;
            case 12: queens++; break;
            case 14: aces++; break;
        }
    }
    
    /* Clear previous nomination */
    g.nominated_suit = -1;
    
    /* Apply effects */
    
    /* 2s - Draw two (stackable) */
    if (twos > 0) {
        g.draw_penalty += twos * 2;
    }
    
    /* 7s - Skip turn (stackable) */
    if (sevens > 0) {
        g.skip_count += sevens;
    }
    
    /* Jacks - Draw 5 or reduce */
    if (black_jacks > 0) {
        g.draw_penalty += black_jacks * 5;
    }
    if (red_jacks > 0 && g.draw_penalty > 0) {
        g.draw_penalty -= red_jacks * 5;
        if (g.draw_penalty < 0) g.draw_penalty = 0;
    }
    
    /* Queens - Reverse direction */
    if (queens > 0) {
        /* Each Queen reverses */
        for (i = 0; i < queens; i++) {
            g.direction = -g.direction;
        }
    }
    
    /* Aces - Nominate suit (only one nomination even if multiple aces) */
    if (aces > 0) {
        printf("\nChoose suit (H=0, D=1, C=2, S=3): ");
        scanf("%d", &g.nominated_suit);
        while (g.nominated_suit < 0 || g.nominated_suit > 3) {
            printf("Invalid! Choose 0-3: ");
            scanf("%d", &g.nominated_suit);
        }
        printf("Next play must be ");
        print_suit(g.nominated_suit);
        printf(" or an Ace\n");
    }
}

/* Draw cards */
void draw_cards(Card *hand, int *count, int num) {
    int i;
    
    for (i = 0; i < num; i++) {
        /* Reshuffle if needed */
        if (g.deck_count == 0) {
            int j;
            Card saved = g.top_card;
            
            /* Move discard to deck (except top card) */
            for (j = 0; j < g.discard_count - 1; j++) {
                g.deck[j] = g.discard[j];
            }
            g.deck_count = g.discard_count - 1;
            
            /* Reset discard */
            g.discard[0] = saved;
            g.discard_count = 1;
            
            shuffle_deck();
        }
        
        /* Draw if possible */
        if (g.deck_count > 0) {
            hand[(*count)++] = g.deck[--g.deck_count];
        }
    }
}

/* Show game state */
void show_game(void) {
    int i;
    
    clear_screen();
    
    printf("==============================\n");
    printf("    RACHEL - CORRECT RULES    \n");
    printf("==============================\n\n");
    
    /* Game info */
    printf("Direction: %s\n", g.direction > 0 ? "Clockwise" : "Counter-clockwise");
    if (g.draw_penalty > 0) {
        printf("PENDING: Draw %d cards!\n", g.draw_penalty);
    }
    if (g.skip_count > 0) {
        printf("PENDING: Skip %d turn(s)!\n", g.skip_count);
    }
    if (g.nominated_suit >= 0) {
        printf("Must play: ");
        print_suit(g.nominated_suit);
        printf(" or Ace\n");
    }
    printf("\n");
    
    /* CPU status */
    printf("CPU: %d cards\n", g.cpu_count);
    
    /* Top card */
    printf("Top: ");
    print_card(g.top_card);
    printf("\n");
    
    /* Deck */
    printf("Deck: %d cards\n\n", g.deck_count);
    
    /* Player's hand */
    printf("Your hand (%d cards):\n", g.player_count);
    for (i = 0; i < g.player_count; i++) {
        printf("%d.", i + 1);
        print_card(g.player_hand[i]);
        
        /* Mark special cards */
        switch(g.player_hand[i].rank) {
            case 2:  printf("(+2)"); break;
            case 7:  printf("(Skip)"); break;
            case 11: 
                if (g.player_hand[i].suit >= 2) printf("(+5)");
                else printf("(-5)");
                break;
            case 12: printf("(Rev)"); break;
            case 14: printf("(Suit)"); break;
        }
        printf(" ");
        
        if ((i + 1) % 5 == 0) printf("\n");
    }
    printf("\n\n");
}

/* Check mandatory play rule */
void check_mandatory_play(Card *hand, int count, int *must_play) {
    int i;
    *must_play = 0;
    
    for (i = 0; i < count; i++) {
        if (can_play_card(hand[i])) {
            *must_play = 1;
            return;
        }
    }
}

/* Find all valid plays */
int find_valid_plays(Card *hand, int count, int *valid_indices) {
    int i, num_valid = 0;
    
    for (i = 0; i < count; i++) {
        if (can_play_card(hand[i])) {
            valid_indices[num_valid++] = i;
        }
    }
    
    return num_valid;
}

/* Move to next player */
void next_player(void) {
    /* Handle skips */
    if (g.skip_count > 0) {
        g.skip_count--;
        /* In 2-player, skip just returns to same player */
        return;
    }
    
    /* Normal turn change */
    g.current_player = 1 - g.current_player;
}

/* Player's turn */
void player_turn(void) {
    int must_play, num_valid, valid_indices[52];
    int choice, i, num_to_play;
    int cards_to_play[13], same_rank;
    char input[100];
    
    /* Handle pending draw penalty */
    if (g.draw_penalty > 0) {
        /* Check if can counter */
        num_valid = 0;
        for (i = 0; i < g.player_count; i++) {
            if (can_counter_attack(g.player_hand[i])) {
                valid_indices[num_valid++] = i;
            }
        }
        
        if (num_valid > 0) {
            printf("Counter attack? (card # or 0 to accept penalty): ");
            scanf("%d", &choice);
            
            if (choice > 0 && choice <= g.player_count) {
                if (can_counter_attack(g.player_hand[choice - 1])) {
                    cards_to_play[0] = choice - 1;
                    play_cards(g.player_hand, &g.player_count, cards_to_play, 1);
                    return;
                }
            }
        }
        
        /* Accept penalty */
        printf("Drawing %d cards...\n", g.draw_penalty);
        draw_cards(g.player_hand, &g.player_count, g.draw_penalty);
        g.draw_penalty = 0;
        getchar(); /* Wait */
        /* Continue turn after drawing! */
    }
    
    /* Check for mandatory play */
    check_mandatory_play(g.player_hand, g.player_count, &must_play);
    
    if (must_play) {
        printf("You MUST play (mandatory rule). Choose card: ");
        
        /* Find valid plays */
        num_valid = find_valid_plays(g.player_hand, g.player_count, valid_indices);
        
        /* Show valid options */
        printf("\nValid plays: ");
        for (i = 0; i < num_valid; i++) {
            printf("%d ", valid_indices[i] + 1);
        }
        printf("\n");
        
        /* Get choice */
        scanf("%d", &choice);
        choice--;  /* Convert to 0-based */
        
        /* Check for stacking */
        same_rank = g.player_hand[choice].rank;
        num_to_play = 0;
        cards_to_play[num_to_play++] = choice;
        
        printf("Stack same rank? (y/n): ");
        scanf("%s", input);
        
        if (input[0] == 'y' || input[0] == 'Y') {
            for (i = 0; i < g.player_count; i++) {
                if (i != choice && g.player_hand[i].rank == same_rank) {
                    printf("Also play card %d (", i + 1);
                    print_card(g.player_hand[i]);
                    printf(")? (y/n): ");
                    scanf("%s", input);
                    if (input[0] == 'y' || input[0] == 'Y') {
                        cards_to_play[num_to_play++] = i;
                    }
                }
            }
        }
        
        /* Play the cards */
        play_cards(g.player_hand, &g.player_count, cards_to_play, num_to_play);
        
    } else {
        /* Must draw */
        printf("No valid plays. Drawing card...\n");
        draw_cards(g.player_hand, &g.player_count, 1);
        getchar(); /* Wait */
        
        /* Check if new card can be played */
        if (can_play_card(g.player_hand[g.player_count - 1])) {
            printf("New card can be played! Play it? (y/n): ");
            scanf("%s", input);
            if (input[0] == 'y' || input[0] == 'Y') {
                cards_to_play[0] = g.player_count - 1;
                play_cards(g.player_hand, &g.player_count, cards_to_play, 1);
            }
        }
    }
    
    /* Check for win */
    if (g.player_count == 0) {
        printf("\nYOU WIN!\n");
        g.game_over = 1;
    }
}

/* CPU turn (simple AI) */
void cpu_turn(void) {
    int i, must_play, played = 0;
    int cards_to_play[13], num_to_play;
    
    printf("CPU thinking...\n");
    
    /* Handle pending draw penalty */
    if (g.draw_penalty > 0) {
        /* Try to counter */
        for (i = 0; i < g.cpu_count; i++) {
            if (can_counter_attack(g.cpu_hand[i])) {
                printf("CPU counters with ");
                print_card(g.cpu_hand[i]);
                printf("\n");
                cards_to_play[0] = i;
                play_cards(g.cpu_hand, &g.cpu_count, cards_to_play, 1);
                played = 1;
                break;
            }
        }
        
        if (!played) {
            /* Accept penalty */
            printf("CPU draws %d cards\n", g.draw_penalty);
            draw_cards(g.cpu_hand, &g.cpu_count, g.draw_penalty);
            g.draw_penalty = 0;
            /* Continue turn! */
        } else {
            return;
        }
    }
    
    /* Check mandatory play */
    check_mandatory_play(g.cpu_hand, g.cpu_count, &must_play);
    
    if (must_play) {
        /* Find first valid play (simple AI) */
        for (i = 0; i < g.cpu_count; i++) {
            if (can_play_card(g.cpu_hand[i])) {
                printf("CPU plays ");
                print_card(g.cpu_hand[i]);
                
                /* Simple stacking - play all of same rank */
                num_to_play = 0;
                cards_to_play[num_to_play++] = i;
                
                /* Look for same rank to stack */
                int rank = g.cpu_hand[i].rank;
                int j;
                for (j = i + 1; j < g.cpu_count; j++) {
                    if (g.cpu_hand[j].rank == rank) {
                        printf(" + ");
                        print_card(g.cpu_hand[j]);
                        cards_to_play[num_to_play++] = j;
                    }
                }
                
                printf("\n");
                play_cards(g.cpu_hand, &g.cpu_count, cards_to_play, num_to_play);
                
                /* Handle Ace nomination */
                if (rank == 14) {
                    g.nominated_suit = rand() % 4;
                    printf("CPU nominates ");
                    print_suit(g.nominated_suit);
                    printf("\n");
                }
                
                played = 1;
                break;
            }
        }
    } else {
        /* Must draw */
        printf("CPU draws a card\n");
        draw_cards(g.cpu_hand, &g.cpu_count, 1);
        
        /* Try to play new card */
        if (can_play_card(g.cpu_hand[g.cpu_count - 1])) {
            printf("CPU plays drawn card: ");
            print_card(g.cpu_hand[g.cpu_count - 1]);
            printf("\n");
            cards_to_play[0] = g.cpu_count - 1;
            play_cards(g.cpu_hand, &g.cpu_count, cards_to_play, 1);
            
            /* Handle Ace */
            if (g.cpu_hand[g.cpu_count - 1].rank == 14) {
                g.nominated_suit = rand() % 4;
                printf("CPU nominates ");
                print_suit(g.nominated_suit);
                printf("\n");
            }
        }
    }
    
    /* Check for win */
    if (g.cpu_count == 0) {
        printf("\nCPU WINS!\n");
        g.game_over = 1;
    }
    
    printf("Press Enter to continue...");
    getchar();
    getchar();
}

/* Main game */
int main(void) {
    clear_screen();
    printf("RACHEL - The REAL Card Game\n");
    printf("DOS Edition with CORRECT RULES\n\n");
    printf("Special Cards:\n");
    printf("  2s: Draw 2 (stackable)\n");
    printf("  7s: Skip turn (stackable)\n");
    printf("  Black Jacks: Draw 5 (stackable)\n");
    printf("  Red Jacks: Reduce penalty by 5\n");
    printf("  Queens: Reverse direction\n");
    printf("  Aces: Choose next suit\n\n");
    printf("MANDATORY PLAY: If you can play, you MUST!\n\n");
    printf("Press Enter to start...");
    getchar();
    
    init_game();
    
    /* Game loop */
    while (!g.game_over) {
        show_game();
        
        if (g.current_player == 0) {
            player_turn();
        } else {
            cpu_turn();
        }
        
        if (!g.game_over) {
            next_player();
        }
    }
    
    printf("Press Enter to exit...");
    getchar();
    
    return 0;
}