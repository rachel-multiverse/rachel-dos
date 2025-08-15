/*
 * RACHEL CANONICAL RULES HEADER
 * 
 * This is the sacred definition. All implementations must follow these rules.
 * This code must compile on any C compiler since C89.
 * No external dependencies. No assumptions. Just cards and logic.
 * 
 * "The rules are immutable. The cards are eternal."
 */

#ifndef RACHEL_RULES_H
#define RACHEL_RULES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Standard integer types for maximum compatibility */
#ifndef RACHEL_TYPES_DEFINED
#define RACHEL_TYPES_DEFINED
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef signed char    int8_t;
typedef int            bool_t;
#define TRUE  1
#define FALSE 0
#endif

/* Constants */
#define MAX_PLAYERS        8
#define MAX_HAND_SIZE     52    /* Theoretical maximum */
#define STANDARD_DECK     52
#define ULTIMATE_DECK     56    /* With 4 jokers */
#define MAX_DECK_SIZE     104   /* Two decks for ultimate chaos */

/* Suits - Using bit patterns for efficiency */
#define SUIT_HEARTS   0x00
#define SUIT_DIAMONDS 0x01
#define SUIT_CLUBS    0x02
#define SUIT_SPADES   0x03
#define SUIT_NONE     0xFF    /* For jokers */

/* Ranks */
#define RANK_2        2
#define RANK_3        3
#define RANK_4        4
#define RANK_5        5
#define RANK_6        6
#define RANK_7        7
#define RANK_8        8
#define RANK_9        9
#define RANK_10       10
#define RANK_JACK     11
#define RANK_QUEEN    12
#define RANK_KING     13
#define RANK_ACE      14
#define RANK_JOKER    15    /* Ultimate variant */

/* Card encoding: single byte per card */
#define MAKE_CARD(suit, rank) (((suit) << 6) | (rank))
#define GET_SUIT(card)        ((card) >> 6)
#define GET_RANK(card)        ((card) & 0x3F)
#define NO_CARD               0x00
#define UNKNOWN_CARD          0xFF

/* Special card detection macros */
#define IS_TWO(card)          (GET_RANK(card) == RANK_2)
#define IS_SEVEN(card)        (GET_RANK(card) == RANK_7)
#define IS_JACK(card)         (GET_RANK(card) == RANK_JACK)
#define IS_QUEEN(card)        (GET_RANK(card) == RANK_QUEEN)
#define IS_ACE(card)          (GET_RANK(card) == RANK_ACE)
#define IS_JOKER(card)        (GET_RANK(card) == RANK_JOKER)
#define IS_BLACK_JACK(card)   (IS_JACK(card) && (GET_SUIT(card) >= SUIT_CLUBS))
#define IS_RED_JACK(card)     (IS_JACK(card) && (GET_SUIT(card) <= SUIT_DIAMONDS))

/* Game states */
typedef enum {
    STATE_WAITING,     /* Waiting for players */
    STATE_PLAYING,     /* Game in progress */
    STATE_FINISHED     /* Game over */
} GameState;

/* Play direction */
typedef enum {
    DIR_CLOCKWISE = 0,
    DIR_COUNTER_CLOCKWISE = 1
} Direction;

/* Card structure */
typedef struct {
    uint8_t encoded;   /* Single byte encoding */
} Card;

/* Player structure */
typedef struct {
    uint8_t  id;
    char     name[32];
    Card     hand[MAX_HAND_SIZE];
    uint8_t  hand_count;
    bool_t   is_out;
    bool_t   is_ai;
    uint8_t  finish_position;
} Player;

/* Pending effect structure */
typedef struct {
    uint8_t  type;         /* RANK_2, RANK_7, RANK_JACK */
    uint8_t  count;        /* How many stacked */
    uint8_t  source_player;
} PendingEffect;

/* Game structure - The complete game state */
typedef struct {
    /* Players */
    Player   players[MAX_PLAYERS];
    uint8_t  player_count;
    uint8_t  current_player_index;
    
    /* Cards */
    Card     deck[MAX_DECK_SIZE];
    uint8_t  deck_count;
    Card     discard_pile[MAX_DECK_SIZE];
    uint8_t  discard_count;
    
    /* Game flow */
    GameState state;
    Direction direction;
    uint8_t   nominated_suit;     /* After ace played, 0xFF if none */
    
    /* Pending effects */
    PendingEffect pending_effect;
    
    /* Statistics */
    uint32_t turn_count;
    uint8_t  winner_count;        /* How many have gone out */
    
    /* Configuration */
    bool_t   ultimate_mode;       /* Jokers enabled */
    uint8_t  starting_hand_size;  /* Varies by player count */
} Game;

/* Core rule functions - These are the LAW */

/* Initialize a new game */
void rachel_init_game(Game* game, uint8_t player_count);

/* Add a player to the game */
bool_t rachel_add_player(Game* game, const char* name, bool_t is_ai);

/* Start the game (shuffle and deal) */
void rachel_start_game(Game* game);

/* Check if a card can be played */
bool_t rachel_can_play_card(const Game* game, Card card);

/* Check if player must play (cannot choose to draw) */
bool_t rachel_must_play(const Game* game, uint8_t player_id);

/* Play one or more cards */
bool_t rachel_play_cards(Game* game, uint8_t player_id, 
                        const Card* cards, uint8_t count,
                        uint8_t nominated_suit);

/* Draw cards (when cannot play or due to attack) */
bool_t rachel_draw_cards(Game* game, uint8_t player_id, uint8_t count);

/* Process pending effects for current player */
void rachel_process_effects(Game* game);

/* Advance to next player */
void rachel_next_turn(Game* game);

/* Check if game is over */
bool_t rachel_is_game_over(const Game* game);

/* Get valid plays for current player */
uint8_t rachel_get_valid_plays(const Game* game, Card* valid_cards);

/* Utility functions */

/* Shuffle deck using seed for reproducibility */
void rachel_shuffle(Card* cards, uint8_t count, uint32_t seed);

/* Create standard 52-card deck */
void rachel_create_deck(Card* deck, bool_t include_jokers);

/* Check if two cards match by suit or rank */
bool_t rachel_cards_match(Card c1, Card c2);

/* Check if card is a special card */
bool_t rachel_is_special(Card card);

/* Get attack value of a card (0 if not attack) */
uint8_t rachel_get_attack_value(Card card);

/* Calculate hand size based on player count */
uint8_t rachel_calculate_hand_size(uint8_t player_count);

/* Encode/decode cards for network protocol */
uint8_t rachel_encode_card(Card card);
Card rachel_decode_card(uint8_t encoded);

/* Debug helpers (can be excluded in production) */
#ifdef RACHEL_DEBUG
void rachel_print_card(Card card);
void rachel_print_hand(const Card* hand, uint8_t count);
void rachel_print_game_state(const Game* game);
const char* rachel_card_to_string(Card card);
#endif

/* Version information */
#define RACHEL_RULES_VERSION_MAJOR 1
#define RACHEL_RULES_VERSION_MINOR 0
#define RACHEL_RULES_VERSION_PATCH 0

/* Get version string */
const char* rachel_version(void);

/* Validate implementation against test suite */
bool_t rachel_self_test(void);

#ifdef __cplusplus
}
#endif

#endif /* RACHEL_RULES_H */

/*
 * End of sacred header.
 * 
 * "You must play if you can.
 *  You must implement if you can.
 *  The protocol is 64 bytes.
 *  The rules are eternal."
 */
