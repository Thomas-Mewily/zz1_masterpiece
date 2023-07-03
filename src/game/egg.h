#ifndef GAME_EGG
#define GAME_EGG
#include "base.h"

typedef int egg_output;
#define EGG_OUTPUT_DO_NOTHINGS 0
#define EGG_OUTPUT_MOVE_UP     1
#define EGG_OUTPUT_MOVE_DOWN   2
#define EGG_OUTPUT_MOVE_RANGE  3

typedef int egg_input;
#define EGG_INPUT_OSEF 0

typedef struct
{
    int nb_colonne;
    vec* /* of vec* of EGG_obstacle */ grid;
} egg_immutable_state;

#define egg_nb_colonne (istate->nb_colonne)
#define egg_nb_ligne 5

#define egg_grid (istate->grid)

typedef int8 obstacle;
#define EGG_OBSTACLE_NONE  0
#define EGG_OBSTACLE_ARROW 1

typedef struct
{
    // is_wall[nb_colonne][nb_ligne] aka [x][y]
    //bool** is_walls;
    int nb_tour;
    int player_y;
    int player_x;
} egg_mutable_state;

typedef struct
{
    int dummy;
    texture* fond;
    texture* fleche;

    
} egg_draw_state;

void egg_load(game_arg arg);
void egg_unload(game_arg arg);

void  egg_unload_mutable(game_arg arg);
void* egg_clone_mutable(game_arg arg);

void egg_update(game_arg arg);
void egg_draw(game_arg arg);
void egg_draw_rule(game_arg arg, rule* r);

void egg_player_input(game_arg arg, entity* e);
bool egg_rule_match(game_arg arg, entity* e, rule* r);


void grid_set(game_arg arg, int ligne, int colonne, obstacle val);
obstacle grid_get(game_arg arg, int ligne, int colonne);
void grid_push_colonne(game_arg arg, vec* v);

#endif