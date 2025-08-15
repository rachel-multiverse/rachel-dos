/*
 * RACHEL CANONICAL RULES IMPLEMENTATION
 * 
 * This is the sacred implementation. Every platform must behave identically.
 * Pure C89. No dependencies. Compiles everywhere from PDP-11 to modern systems.
 * 
 * "From 1 MHz to 1 THz, the rules remain the same."
 */

#include "rules.h"

/* String functions we implement ourselves for portability */
static void rachel_strcpy(char* dest, const char* src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

/* Random number generator - simple but portable */
static uint32_t rachel_rand_seed = 12345;

static uint32_t rachel_rand(void) {
    rachel_rand_seed = rachel_rand_seed * 1103515245 + 12345;
    return (rachel_rand_seed / 65536) % 32768;
}

static void rachel_srand(uint32_t seed) {
    rachel_rand_seed = seed;
}

/* Initialize a new game */
void rachel_init_game(Game* game, uint8_t player_count) {
    int i;
    
    /* Clear everything */
    for (i = 0; i < sizeof(Game); i++) {
        ((uint8_t*)game)[i] = 0;
    }
    
    /* Set defaults */
    game->state = STATE_WAITING;
    game->direction = DIR_CLOCKWISE;
    game->nominated_suit = 0xFF;  /* No nomination */
    game->player_count = 0;
    game->current_player_index = 0;
    game->turn_count = 0;
    game->winner_count = 0;
    
    /* Calculate starting hand size */
    game->starting_hand_size = rachel_calculate_hand_size(player_count);
    
    /* Initialize pending effects */
    game->pending_effect.type = 0;
    game->pending_effect.count = 0;
    game->pending_effect.source_player = 0xFF;
}

/* Add a player */
bool_t rachel_add_player(Game* game, const char* name, bool_t is_ai) {
    Player* player;
    
    if (game->player_count >= MAX_PLAYERS) {
        return FALSE;
    }
    
    player = &game->players[game->player_count];
    player->id = game->player_count;
    rachel_strcpy(player->name, name);
    player->hand_count = 0;
    player->is_out = FALSE;
    player->is_ai = is_ai;
    player->finish_position = 0;
    
    game->player_count++;
    return TRUE;
}

/* Create a standard deck */
void rachel_create_deck(Card* deck, bool_t include_jokers) {
    uint8_t suit, rank;
    int index = 0;
    
    /* Add standard 52 cards */
    for (suit = SUIT_HEARTS; suit <= SUIT_SPADES; suit++) {
        for (rank = RANK_2; rank <= RANK_ACE; rank++) {
            deck[index].encoded = MAKE_CARD(suit, rank);
            index++;
        }
    }
    
    /* Add jokers for ultimate mode */
    if (include_jokers) {
        for (rank = 0; rank < 4; rank++) {
            deck[index].encoded = RANK_JOKER;  /* Jokers don't need suit encoding */
            index++;
        }
    }
}

/* Shuffle deck */
void rachel_shuffle(Card* cards, uint8_t count, uint32_t seed) {
    int i, j;
    Card temp;
    
    rachel_srand(seed);
    
    /* Fisher-Yates shuffle */
    for (i = count - 1; i > 0; i--) {
        j = rachel_rand() % (i + 1);
        temp = cards[i];
        cards[i] = cards[j];
        cards[j] = temp;
    }
}

/* Start the game */
void rachel_start_game(Game* game) {
    int i, j, card_index = 0;
    
    /* Create and shuffle deck */
    rachel_create_deck(game->deck, game->ultimate_mode);
    game->deck_count = game->ultimate_mode ? ULTIMATE_DECK : STANDARD_DECK;
    rachel_shuffle(game->deck, game->deck_count, rachel_rand_seed);
    
    /* Deal cards to players */
    for (i = 0; i < game->starting_hand_size; i++) {
        for (j = 0; j < game->player_count; j++) {
            game->players[j].hand[i] = game->deck[card_index++];
            game->players[j].hand_count++;
        }
    }
    
    /* Place one card in discard pile */
    game->discard_pile[0] = game->deck[card_index++];
    game->discard_count = 1;
    
    /* Move remaining cards to deck */
    for (i = 0; card_index < game->deck_count; i++, card_index++) {
        game->deck[i] = game->deck[card_index];
    }
    game->deck_count = i;
    
    /* Start playing */
    game->state = STATE_PLAYING;
    game->current_player_index = 0;
}

/* Check if cards match */
bool_t rachel_cards_match(Card c1, Card c2) {
    uint8_t suit1 = GET_SUIT(c1.encoded);
    uint8_t suit2 = GET_SUIT(c2.encoded);
    uint8_t rank1 = GET_RANK(c1.encoded);
    uint8_t rank2 = GET_RANK(c2.encoded);
    
    /* Jokers are wild in ultimate mode */
    if (rank1 == RANK_JOKER || rank2 == RANK_JOKER) {
        return TRUE;
    }
    
    /* Match by suit or rank */
    return (suit1 == suit2) || (rank1 == rank2);
}

/* Check if player can play a card */
bool_t rachel_can_play_card(const Game* game, Card card) {
    Card top_card;
    uint8_t required_suit;
    
    if (game->discard_count == 0) {
        return FALSE;
    }
    
    top_card = game->discard_pile[game->discard_count - 1];
    
    /* Handle pending effects - can only play certain cards */
    if (game->pending_effect.count > 0) {
        uint8_t card_rank = GET_RANK(card.encoded);
        
        /* Must play same attack type or counter */
        if (game->pending_effect.type == RANK_2) {
            return card_rank == RANK_2;
        }
        else if (game->pending_effect.type == RANK_7) {
            return card_rank == RANK_7;
        }
        else if (game->pending_effect.type == RANK_JACK) {
            /* Can play black jack to stack or red jack to counter */
            if (IS_BLACK_JACK(top_card.encoded)) {
                return IS_JACK(card.encoded);
            }
        }
    }
    
    /* Normal play rules */
    required_suit = game->nominated_suit;
    if (required_suit == 0xFF) {
        required_suit = GET_SUIT(top_card.encoded);
    }
    
    /* Jokers can always be played */
    if (IS_JOKER(card.encoded)) {
        return TRUE;
    }
    
    /* Check suit or rank match */
    return (GET_SUIT(card.encoded) == required_suit) || 
           (GET_RANK(card.encoded) == GET_RANK(top_card.encoded));
}

/* Check if player must play */
bool_t rachel_must_play(const Game* game, uint8_t player_id) {
    const Player* player;
    int i;
    
    if (player_id >= game->player_count) {
        return FALSE;
    }
    
    player = &game->players[player_id];
    
    /* Check each card in hand */
    for (i = 0; i < player->hand_count; i++) {
        if (rachel_can_play_card(game, player->hand[i])) {
            return TRUE;
        }
    }
    
    return FALSE;
}

/* Play cards */
bool_t rachel_play_cards(Game* game, uint8_t player_id, 
                        const Card* cards, uint8_t count,
                        uint8_t nominated_suit) {
    Player* player;
    uint8_t first_rank, i, j;
    bool_t found;
    
    if (player_id >= game->player_count || count == 0) {
        return FALSE;
    }
    
    player = &game->players[player_id];
    
    /* Verify first card is playable */
    if (!rachel_can_play_card(game, cards[0])) {
        return FALSE;
    }
    
    first_rank = GET_RANK(cards[0].encoded);
    
    /* Verify all cards have same rank (for stacking) */
    for (i = 1; i < count; i++) {
        if (GET_RANK(cards[i].encoded) != first_rank) {
            return FALSE;
        }
    }
    
    /* Verify player has all these cards */
    for (i = 0; i < count; i++) {
        found = FALSE;
        for (j = 0; j < player->hand_count; j++) {
            if (player->hand[j].encoded == cards[i].encoded) {
                found = TRUE;
                break;
            }
        }
        if (!found) {
            return FALSE;
        }
    }
    
    /* Remove cards from hand and add to discard */
    for (i = 0; i < count; i++) {
        /* Add to discard pile */
        game->discard_pile[game->discard_count++] = cards[i];
        
        /* Remove from hand */
        for (j = 0; j < player->hand_count; j++) {
            if (player->hand[j].encoded == cards[i].encoded) {
                /* Shift remaining cards */
                for (; j < player->hand_count - 1; j++) {
                    player->hand[j] = player->hand[j + 1];
                }
                player->hand_count--;
                break;
            }
        }
    }
    
    /* Handle special card effects */
    if (IS_TWO(cards[0].encoded)) {
        game->pending_effect.type = RANK_2;
        game->pending_effect.count += count * 2;
        game->pending_effect.source_player = player_id;
    }
    else if (IS_SEVEN(cards[0].encoded)) {
        game->pending_effect.type = RANK_7;
        game->pending_effect.count += count;
        game->pending_effect.source_player = player_id;
    }
    else if (IS_BLACK_JACK(cards[0].encoded)) {
        game->pending_effect.type = RANK_JACK;
        game->pending_effect.count += count * 5;
        game->pending_effect.source_player = player_id;
    }
    else if (IS_RED_JACK(cards[0].encoded) && game->pending_effect.type == RANK_JACK) {
        /* Red jack reduces black jack penalty */
        if (game->pending_effect.count >= 5 * count) {
            game->pending_effect.count -= 5 * count;
        } else {
            game->pending_effect.count = 0;
        }
        if (game->pending_effect.count == 0) {
            game->pending_effect.type = 0;
        }
    }
    else if (IS_QUEEN(cards[0].encoded)) {
        /* Reverse direction for each queen */
        for (i = 0; i < count; i++) {
            game->direction = !game->direction;
        }
    }
    else if (IS_ACE(cards[0].encoded) || IS_JOKER(cards[0].encoded)) {
        /* Nominate suit - only one nomination even if multiple aces played */
        game->nominated_suit = nominated_suit;
    }
    else {
        /* Non-special card clears nomination */
        game->nominated_suit = 0xFF;
    }
    
    /* Check if player went out */
    if (player->hand_count == 0) {
        player->is_out = TRUE;
        player->finish_position = ++game->winner_count;
    }
    
    return TRUE;
}

/* Draw cards */
bool_t rachel_draw_cards(Game* game, uint8_t player_id, uint8_t count) {
    Player* player;
    uint8_t cards_to_draw;
    int i;
    
    if (player_id >= game->player_count) {
        return FALSE;
    }
    
    player = &game->players[player_id];
    cards_to_draw = count;
    
    /* Draw from deck */
    while (cards_to_draw > 0 && game->deck_count > 0) {
        player->hand[player->hand_count++] = game->deck[--game->deck_count];
        cards_to_draw--;
    }
    
    /* If deck empty, shuffle discard pile (keeping top card) */
    if (cards_to_draw > 0 && game->discard_count > 1) {
        Card top_card = game->discard_pile[game->discard_count - 1];
        
        /* Move discard to deck (except top card) */
        for (i = 0; i < game->discard_count - 1; i++) {
            game->deck[i] = game->discard_pile[i];
        }
        game->deck_count = game->discard_count - 1;
        
        /* Shuffle */
        rachel_shuffle(game->deck, game->deck_count, rachel_rand());
        
        /* Keep only top card in discard */
        game->discard_pile[0] = top_card;
        game->discard_count = 1;
        
        /* Continue drawing */
        while (cards_to_draw > 0 && game->deck_count > 0) {
            player->hand[player->hand_count++] = game->deck[--game->deck_count];
            cards_to_draw--;
        }
    }
    
    return TRUE;
}

/* Process pending effects */
void rachel_process_effects(Game* game) {
    uint8_t current_player = game->current_player_index;
    
    if (game->pending_effect.count == 0) {
        return;
    }
    
    /* Apply effect based on type */
    if (game->pending_effect.type == RANK_2) {
        /* Draw 2s */
        rachel_draw_cards(game, current_player, game->pending_effect.count);
    }
    else if (game->pending_effect.type == RANK_JACK) {
        /* Draw for black jacks */
        rachel_draw_cards(game, current_player, game->pending_effect.count);
    }
    else if (game->pending_effect.type == RANK_7) {
        /* Skip turns */
        uint8_t skips = game->pending_effect.count;
        while (skips > 0) {
            rachel_next_turn(game);
            skips--;
        }
    }
    
    /* Clear effect */
    game->pending_effect.type = 0;
    game->pending_effect.count = 0;
    game->pending_effect.source_player = 0xFF;
}

/* Advance to next player */
void rachel_next_turn(Game* game) {
    int attempts = 0;
    
    do {
        if (game->direction == DIR_CLOCKWISE) {
            game->current_player_index = (game->current_player_index + 1) % game->player_count;
        } else {
            if (game->current_player_index == 0) {
                game->current_player_index = game->player_count - 1;
            } else {
                game->current_player_index--;
            }
        }
        attempts++;
    } while (game->players[game->current_player_index].is_out && attempts < game->player_count);
    
    game->turn_count++;
}

/* Check if game is over */
bool_t rachel_is_game_over(const Game* game) {
    uint8_t active_players = 0;
    int i;
    
    for (i = 0; i < game->player_count; i++) {
        if (!game->players[i].is_out) {
            active_players++;
        }
    }
    
    return active_players <= 1;
}

/* Get valid plays */
uint8_t rachel_get_valid_plays(const Game* game, Card* valid_cards) {
    const Player* player;
    uint8_t count = 0;
    int i;
    
    if (game->current_player_index >= game->player_count) {
        return 0;
    }
    
    player = &game->players[game->current_player_index];
    
    for (i = 0; i < player->hand_count; i++) {
        if (rachel_can_play_card(game, player->hand[i])) {
            if (valid_cards != 0) {
                valid_cards[count] = player->hand[i];
            }
            count++;
        }
    }
    
    return count;
}

/* Calculate hand size based on player count */
uint8_t rachel_calculate_hand_size(uint8_t player_count) {
    /* Maintain ~11 cards in deck after dealing */
    switch (player_count) {
        case 2:
        case 3:
        case 4:
        case 5:
            return 7;
        case 6:
        case 7:
            return 6;
        case 8:
            return 5;
        default:
            return 7;
    }
}

/* Get attack value */
uint8_t rachel_get_attack_value(Card card) {
    uint8_t rank = GET_RANK(card.encoded);
    
    if (rank == RANK_2) return 2;
    if (IS_BLACK_JACK(card.encoded)) return 5;
    return 0;
}

/* Check if special */
bool_t rachel_is_special(Card card) {
    uint8_t rank = GET_RANK(card.encoded);
    return (rank == RANK_2 || rank == RANK_7 || 
            rank == RANK_JACK || rank == RANK_QUEEN || 
            rank == RANK_ACE || rank == RANK_JOKER);
}

/* Version string */
const char* rachel_version(void) {
    return "1.0.0";
}

/* Self test */
bool_t rachel_self_test(void) {
    Game game;
    Card test_card;
    
    /* Test card encoding */
    test_card.encoded = MAKE_CARD(SUIT_HEARTS, RANK_ACE);
    if (GET_SUIT(test_card.encoded) != SUIT_HEARTS) return FALSE;
    if (GET_RANK(test_card.encoded) != RANK_ACE) return FALSE;
    
    /* Test black jack detection */
    test_card.encoded = MAKE_CARD(SUIT_SPADES, RANK_JACK);
    if (!IS_BLACK_JACK(test_card.encoded)) return FALSE;
    
    test_card.encoded = MAKE_CARD(SUIT_HEARTS, RANK_JACK);
    if (!IS_RED_JACK(test_card.encoded)) return FALSE;
    
    /* Test game initialization */
    rachel_init_game(&game, 4);
    if (game.state != STATE_WAITING) return FALSE;
    
    /* More tests would go here */
    
    return TRUE;
}

/*
 * End of sacred implementation.
 * 
 * This code has been blessed to run on:
 * - 8-bit processors from the 1970s
 * - Modern 64-bit multicore systems  
 * - Everything in between
 * - Probably some things that shouldn't exist
 * 
 * "The cards are eternal. The rules are immutable."
 */
