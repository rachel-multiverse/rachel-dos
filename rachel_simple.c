/*
 * RACHEL SIMPLE TEST VERSION
 * Minimal implementation to verify game logic
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Simple card game without the complex rules.h */

typedef struct {
    int rank;  /* 2-14 (Ace high) */
    int suit;  /* 0-3 */
} SimpleCard;

typedef struct {
    SimpleCard deck[52];
    int deck_count;
    SimpleCard player_hand[13];
    int player_count;
    SimpleCard cpu_hand[13];
    int cpu_count;
    SimpleCard top_card;
    int current_player;
} SimpleGame;

SimpleGame g;

void init_deck() {
    int i, s, r, idx = 0;
    for (s = 0; s < 4; s++) {
        for (r = 2; r <= 14; r++) {
            g.deck[idx].suit = s;
            g.deck[idx].rank = r;
            idx++;
        }
    }
    g.deck_count = 52;
}

void shuffle_deck() {
    int i, j;
    SimpleCard temp;
    srand(time(NULL));
    
    for (i = g.deck_count - 1; i > 0; i--) {
        j = rand() % (i + 1);
        temp = g.deck[i];
        g.deck[i] = g.deck[j];
        g.deck[j] = temp;
    }
}

void deal_cards() {
    int i;
    g.player_count = 0;
    g.cpu_count = 0;
    
    for (i = 0; i < 7; i++) {
        g.player_hand[g.player_count++] = g.deck[--g.deck_count];
        g.cpu_hand[g.cpu_count++] = g.deck[--g.deck_count];
    }
    
    g.top_card = g.deck[--g.deck_count];
}

void print_card(SimpleCard c) {
    char ranks[] = " 23456789TJQKA";
    char suits[] = "HDCS";
    printf("%c%c", ranks[c.rank - 1], suits[c.suit]);
}

void show_game() {
    int i;
    
    printf("\n=== RACHEL CARD GAME ===\n\n");
    printf("CPU: %d cards\n", g.cpu_count);
    printf("Top card: ");
    print_card(g.top_card);
    printf("\n");
    printf("Deck: %d cards\n\n", g.deck_count);
    
    printf("Your hand:\n");
    for (i = 0; i < g.player_count; i++) {
        printf("%d. ", i + 1);
        print_card(g.player_hand[i]);
        printf("  ");
    }
    printf("\n\n");
}

int can_play(SimpleCard card) {
    return (card.rank == g.top_card.rank || 
            card.suit == g.top_card.suit ||
            card.rank == 8);  /* 8s are wild */
}

void play_card(SimpleCard *hand, int *count, int index) {
    int i;
    g.top_card = hand[index];
    
    for (i = index; i < *count - 1; i++) {
        hand[i] = hand[i + 1];
    }
    (*count)--;
}

void draw_card(SimpleCard *hand, int *count) {
    if (g.deck_count > 0) {
        hand[(*count)++] = g.deck[--g.deck_count];
    }
}

int main() {
    int choice;
    char input[10];
    
    printf("RACHEL - Simple Test Version\n");
    printf("Press Enter to start...");
    fgets(input, sizeof(input), stdin);
    
    init_deck();
    shuffle_deck();
    deal_cards();
    g.current_player = 0;
    
    while (g.player_count > 0 && g.cpu_count > 0) {
        show_game();
        
        if (g.current_player == 0) {
            printf("Your turn (1-%d to play, 0 to draw): ", g.player_count);
            fgets(input, sizeof(input), stdin);
            choice = atoi(input);
            
            if (choice == 0) {
                draw_card(g.player_hand, &g.player_count);
                g.current_player = 1;
            } else if (choice >= 1 && choice <= g.player_count) {
                if (can_play(g.player_hand[choice - 1])) {
                    play_card(g.player_hand, &g.player_count, choice - 1);
                    g.current_player = 1;
                } else {
                    printf("Can't play that card!\n");
                }
            }
        } else {
            /* Simple CPU */
            int i, played = 0;
            for (i = 0; i < g.cpu_count && !played; i++) {
                if (can_play(g.cpu_hand[i])) {
                    printf("CPU plays: ");
                    print_card(g.cpu_hand[i]);
                    printf("\n");
                    play_card(g.cpu_hand, &g.cpu_count, i);
                    played = 1;
                }
            }
            if (!played) {
                printf("CPU draws a card\n");
                draw_card(g.cpu_hand, &g.cpu_count);
            }
            g.current_player = 0;
            printf("Press Enter to continue...");
            fgets(input, sizeof(input), stdin);
        }
    }
    
    if (g.player_count == 0) {
        printf("\nYOU WIN!\n");
    } else {
        printf("\nCPU WINS!\n");
    }
    
    return 0;
}