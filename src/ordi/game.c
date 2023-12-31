#include "base.h"

void game_ordi_configure(game* g,
    int condition_input_size,
    int condition_input_max_range,
    int condition_output_size,
    int condition_output_max_range,
    int nb_behavior)
{
    game_type* t = g->type;
    game_mutable* m = g->internal_mutable_state;

    if(t->condition_input_max_range != -1)
    {
        debug;
        printf("Jeu déjà configurer\n");
        return;
    }

    check(nb_behavior >= 1);
    check(condition_input_size >= 1);
    check(condition_input_max_range >= 1);
    check(condition_output_size >= 1);
    check(condition_output_max_range >= 1);


    t->condition_input_size = condition_input_size;
    t->condition_input_max_range = condition_input_max_range;
    t->condition_output_size = condition_output_size;
    t->condition_output_max_range = condition_output_max_range;
    t->nb_behavior = nb_behavior;

    m->input = tab_create(condition_output_size, 0);
    m->current_entity = entity_create_ordi_random(g, 1);
    m->best_ordi = entity_create_ordi_random(g, 1);
}


game* game_create_arg(
    context* c,
    char name[game_name_string_size],
    size_t sizeof_immutable_state,
    size_t sizeof_mutable_state,
    size_t sizeof_draw_state,
    game_fn update,
    game_fn draw,
    game_draw_rule_fn draw_rule,
    game_clone_fn clone_mutable,
    game_fn unload_mutable,
    game_fn load,
    game_fn unload,
    game_get_input_fn player_input,
    game_fn ordi_input_init,
    game_rule_match_fn rule_match,
    things_to_char_fn input_to_char,
    things_to_char_fn output_to_char,
    game_fn _printf
    )
{
    game* g = create(game);
    {
        g->type = create(game_type);
        g->internal_mutable_state = create(game_mutable);
        g->mutable_state   = calloc(sizeof_mutable_state,1);
        g->immutable_state = calloc(sizeof_immutable_state,1);
        g->draw_state      = calloc(sizeof_draw_state,1);
    }

    game_type* t = g->type;
    game_mutable* m = g->internal_mutable_state;

    {
        t->load = load;
        t->unload = unload;

        t->update = update;
        t->draw   = draw;
        t->draw_rule = draw_rule;

        t->clone_mutable = clone_mutable;
        t->unload_mutable = unload_mutable;

        t->player_input = player_input;
        t->ordi_input_init = ordi_input_init;
        t->rule_match = rule_match;

        t->input_to_char  = input_to_char;
        t->output_to_char = output_to_char;
        t->printf = _printf;

        t->maximize_score = true;
        t->is_loaded = false;

        t->condition_input_size = -1;
        t->condition_input_max_range = -1;
        t->condition_output_size = -1;
        t->condition_output_max_range = -1;
        t->nb_behavior = -1;

        repeat(i, game_name_string_size)
        {
            t->name[i] = '\0';
        }
        repeat(i, game_name_string_size)
        {
            t->name[i] = name[i];
            if(name[i] == '\0') break;
        }
        printf("game name %s\n", t->name);
    }

    {
        m->draw_dest = window_rectf(c);
        m->state = GAME_STATE_RUNNING;        
        m->nb_update = 0;

        m->best_score_player = 0; 
        m->best_score_ordi   = 0; 

        m->nb_generation = 0;
        m->target_ups = 3;
        m->current_entity   = null; 
        m->best_ordi        = null;
        m->generation       = null;
        m->input            = null;
        m->is_training_ia   = false;
    }
    return g;
}

game_arg game_arg_create(context* c, game* g)
{
    game_arg arg;
    arg.c = c;
    arg.g = g;
    return arg;
}

//#define g (arg.g)
#define gtype (g->type)
#define arg game_arg_create(c, g)

void game_update(context* c, game* g, float coef_ups)
{
    game_train_best_ordi(c, g);

    if(g->internal_mutable_state->state != GAME_STATE_RUNNING) return;

    g->internal_mutable_state->draw_coef += g->internal_mutable_state->target_ups*coef_ups /update_per_second; //1.0f/ups;
    
    
    if(g->internal_mutable_state->draw_coef >= 1)
    {
        g->internal_mutable_state->draw_coef = 0;
        g->internal_mutable_state->is_training_ia = false;
        game_update_fixed(c, g);
    }
}

void game_update_fixed(context* c, game* g)
{
    check(gtype->is_loaded == true);
    if(g->internal_mutable_state->state != GAME_STATE_GAME_OVER)
    {
        g->internal_mutable_state->nb_update++;
    }


    gtype->update(arg);

    entity* e = g->internal_mutable_state->current_entity;
    float score = e->score;
    if(score >= g->internal_mutable_state->best_score_player && e->type == ENTITY_TYPE_PLAYER)
    {
        g->internal_mutable_state->best_score_player = score;
    }else if(score >= g->internal_mutable_state->best_score_ordi && e->type == ENTITY_TYPE_ORDI)
    {
        g->internal_mutable_state->best_score_ordi = score;
    }
}

void game_load(context* c, game* g)
{
    //check(gtype->is_loaded == false);
    gtype->load(arg);
    g->internal_mutable_state->draw_coef = 0;
    g->internal_mutable_state->state = GAME_STATE_RUNNING;
    g->internal_mutable_state->nb_update = 0;
    gtype->is_loaded = true;
    
}

void game_internal_mutable_free(game* g, game_mutable* mut)
{
    tab_free(mut->input);
    if(mut->best_ordi != null)
    {
        entity_free(g, mut->best_ordi);
    }
    if(mut->current_entity != null)
    {
        entity_free(g, mut->current_entity);
    }
    if(mut->generation != null)
    {
        repeat(i, mut->generation->length)
        {
            entity_free(g, vec_get(mut->generation, entity*, i));
        }
        vec_free_lazy(mut->generation);
    }
    free(mut);
}


void game_type_unload(context* c, game* g)
{
    unused(c);
    free(gtype);
    g->type = null;
}



void export_best_entity(context* c, game* g)
{
    unused(c);
    if(g->internal_mutable_state->best_ordi == null) return;

    file* f = game_get_save_file(g, "w+");

    if(f == null) 
    { 
        printf("export failed, no file\n");
        return;
    }
    
    game_export_one_entity(g, f, g->internal_mutable_state->best_ordi);
    fclose(f);
}

void game_unload(context* c, game* g)
{
    if(g == null) return;
    check(gtype->is_loaded == true);
    export_best_entity(c, g);
    gtype->is_loaded = false;

    gtype->unload_mutable(arg);
    free(g->mutable_state);

    gtype->unload(arg);
    free(g->draw_state);
    free(g->immutable_state);

    game_internal_mutable_free(g, g->internal_mutable_state);
    game_type_unload(c, g);
    
    free(g);
    //todo;
}

game_mutable* game_internal_mutable_clone(game_mutable* mut)
{
    game_mutable* copy = create(game_mutable);
    copy->nb_update = mut->nb_update;
    copy->draw_dest = mut->draw_dest;
    copy->input = tab_clone(mut->input);
    copy->state = mut->state;
    copy->generation_idx_training = mut->generation_idx_training;
    copy->generation_current_idx_nb_update = mut->generation_current_idx_nb_update;
    copy->generation_update_per_entity = mut->generation_update_per_entity;
    copy->nb_generation = mut->nb_generation;

    // not clonned
    copy->current_entity = null;//mut->current_entity;
    copy->best_ordi = null;
    copy->generation = null;
    copy->draw_coef = NAN;
    return copy;
}

#define current_entity (g->internal_mutable_state->current_entity)

void game_printf(context* c, game* g)
{
    printf("\n\n");
    gtype->printf(arg);
    printf("current entity %i : score %.1f, is ordi %i\n", current_entity->id, current_entity->score, current_entity->type == ENTITY_TYPE_ORDI);
    game_trainning_printf(c, g);
}

void game_draw(context* c, game* g)
{
    check(gtype->is_loaded == true);
    gtype->draw(arg);

    if(current_entity->type == ENTITY_TYPE_ORDI)
    {
        behavior* b = entity_behavior(current_entity);
        repeat(i, behavior_nb_rule(b))
        {
            gtype->draw_rule(arg, current_entity, behavior_get_rule(b, i), i);
        }
    }
}

game* game_clone(context* c, game* g)
{
    check(gtype->is_loaded == true);
    game* copy = create(game);
    copy->type = gtype;
    copy->immutable_state = g->immutable_state;
    copy->draw_state = null; // don't care
    copy->internal_mutable_state = game_internal_mutable_clone(g->internal_mutable_state);
    copy->mutable_state = gtype->clone_mutable(arg);
    return copy;
}

void game_get_player_input(context* c, game* g, entity* e)
{
    check(gtype->is_loaded == true && e->type == ENTITY_TYPE_PLAYER);
    gtype->player_input(arg, e);
}

bool game_rule_match(context* c, game* g, entity* e, rule* r)
{
    check(gtype->is_loaded == true);
    return gtype->rule_match(arg, e, r);
}


void game_get_input(context* c, game* g, entity* e)
{
    if(e->type == ENTITY_TYPE_PLAYER)
    {
        game_get_player_input(c,g,e);
        return;
    }
    // ordi
    behavior* b = entity_behavior(e);
    
    tab_clear(g->internal_mutable_state->input, 0);

    /*
    //repeat(i, behavior_nb_rule(b))
    for(int i = behavior_nb_rule(b)-1; i>=0;i--)
    {
        rule* r = behavior_get_rule(b, i);

        if(gtype->rule_match(arg, e, r)) 
        {
            r->nb_match++;
            //return;
        }
    }
    */
    repeat(i, behavior_nb_rule(b))
    {
        rule* r = behavior_get_rule(b, i);

        if(gtype->rule_match(arg, e, r)) 
        {
            r->nb_match++;
            return;
        }
    }
}

void game_set_entity_type(game* g, entity_type type)
{
    if(g == null) return;
    current_entity->type = type;
}

void game_reset(context* c, game* g)
{
    game_load(c, g);
    current_entity->score = 0;
}