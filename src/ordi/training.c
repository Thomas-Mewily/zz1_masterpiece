#include "base.h"



#define gtype  (g->type)
// internal mutable state
#undef  mstate
#define mstate (g->internal_mutable_state)
#define arg game_arg_create(c, g)
#define cur_entity (g->internal_mutable_state->current_entity)
#define entity_per_gen (gtype->nb_behavior)
#define best_entity (mstate->best_ordi)
#define gen (mstate->generation)
#define gen_length (gen->length)

#define gen_idx_training          (mstate->generation_idx_training)
#define gen_current_idx_nb_update (mstate->generation_current_idx_nb_update)
#define gen_max_update_per_entity     (mstate->generation_update_per_entity)
#define gen_current_entity (gen_idx_training < gen_length ? gen_get(gen_idx_training) : null)

#define gen_delta_score (mstate->gen_delta_score)


#define game_state (mstate->state)
#define gen_get(idx) gen_get_at(g, idx)


entity* gen_get_at(game* g, int idx)
{
    return vec_get(gen, entity*, idx);
}

void gen_set(game* g, int idx, entity* e_will_be_copied)
{
    entity* old = gen_get_at(g, idx);
    entity* copy = entity_clone(e_will_be_copied);
    vec_set(gen, entity*, idx, copy);
    entity_free(old);
}

bool replace_best_entity_if_needed(context* c, game* g, entity* e)
{
    //return false;
    unused(c);
    if(e->type != ENTITY_TYPE_ORDI) return false;

    if(((best_entity == null) || (e->score > best_entity->score)) && (e != best_entity))
    {

        if(best_entity != null)
        {
            float old_score = best_entity->score;
            entity_free(best_entity);
            gen_delta_score += e->score - old_score;
        }
        best_entity = entity_clone(e);
        
        entity_behavior_set(cur_entity, entity_behavior(e));
        
        //float score = cur_entity->score;
        //entity_free(cur_entity);
        //cur_entity = entity_clone(best_entity);
        //cur_entity->score = score;

        return true;
    }
    return false;
}

void game_glouton(context* c, game* g, vec* next_gen, entity* best, int nb_modif)
{
    unused(nb_modif);
    unused(c);
    behavior* b = best->behavior;

    /*
    nb_modif %= 3;
    nb_modif--;

    if(nb_modif < 0)
    {
        return;
    }*/

    //if(nb_modif == 0)
    {
        repeat(r_idx, behavior_nb_rule(b))
        {   
            rule* r = behavior_get_rule(b, r_idx);

            // input
            repeat(i, r->input->length)
            {   
                repeat(k, gtype->condition_input_max_range)
                {
                    entity* e = entity_clone(best);
                    rule* r_replace = behavior_get_rule(e->behavior, r_idx);
                    tab_set(r_replace->input, i, k);
                    vec_add(next_gen, entity*, e);
                }
            }

            // ouput
            repeat(i, r->output->length)
            {   
                repeat(k, gtype->condition_output_max_range)
                {
                    entity* e = entity_clone(best);
                    rule* r_replace = behavior_get_rule(e->behavior, r_idx);
                    tab_set(r_replace->output, i, k);
                    vec_add(next_gen, entity*, e);
                } 
            }
        }
        return;
    }


    //entity* e = game_glouton(c, g, next_gen, best,)
}

void game_choose_next_generation(context* c, game* g)
{
    g->internal_mutable_state->nb_generation++;
    gen_idx_training = 0;
    gen_current_idx_nb_update = 0;
    //current_entity = null;
    //return;

    repeat(i, gen_length)
    {
        if(replace_best_entity_if_needed(c, g, gen_get(i)))
        {
            //entity_printf(g, gen_get(i));
        }
    }

    vec* next_gen = vec_empty(entity*);
    {
        // keep the best
        repeat(i, 40)
        {
            entity* e = entity_clone(best_entity);
            vec_add(next_gen, entity*, e);
        }
        // 100 random from the last gen
        repeat(i, 40)
        {
            int j = rand()%gen->length;
            entity* e = entity_clone(gen_get(j));
            vec_add(next_gen, entity*, e);
        }

        // 100 pure random
        repeat(i, 40)
        {
            entity* rng = entity_create_ordi_random(g, rand()%best_entity->behavior->rules->length+1+rand()%2);
            vec_add(next_gen, entity*, rng);
        }
    }
    
    repeat(i, next_gen->length)
    {
        entity* e = vec_get(next_gen, entity*, i);
        behavior* b = e->behavior;

        // mutation
        repeat(r_idx, behavior_nb_rule(b))
        {
            rule* r = behavior_get_rule(b, r_idx);

            int nb_mut;

            nb_mut = gtype->condition_input_size;
            nb_mut = (rand() % nb_mut)+1;
            //nb_mut = (rand() % nb_mut);

            repeat(i, nb_mut)
            {   
                int j = rand() % gtype->condition_input_size;
                tab_set(r->input, j, rand()%gtype->condition_input_max_range);
            }

            nb_mut = gtype->condition_output_size;
            nb_mut = (rand() % nb_mut)+1;
            //nb_mut = (rand() % nb_mut);

            repeat(i, nb_mut)
            {   
                int j = rand() % gtype->condition_output_size;
                tab_set(r->output, j, rand()%gtype->condition_output_max_range);
            }
        }

        // add rule
        if(rand() % 100 <= 50)
        {
            rule* r = rule_create(g);
            rule_randomize(g, r);
            behavior_add_rule(b, r);
            rule_free(r);
        }else if(rand() % 100 <= 5 && b->rules->length > 1)
        {
            int j = rand()%b->rules->length;
            behavior_remove_rule(b, j);
        }
        //swap rule
        if(rand() % 100 <= 10 && b->rules->length >= 2)
        {
            int j = rand()%b->rules->length;
            int k = rand()%b->rules->length;

            rule* rj = behavior_get_rule(b, j);
            rule* rk = behavior_get_rule(b, k);
            vec_set(b->rules, rule*, j, rk);
            vec_set(b->rules, rule*, k, rj);
        }
    }

    if(mstate->nb_generation % 50 == 0)
    {
        game_glouton(c, g, next_gen, best_entity, (mstate->nb_generation / 20));
    }
    
    // full random
    repeat(i, gen_length)
    {
        entity_free(gen_get(i));
        //entity_init_random(g, gen_get(i));
    }
    vec_free_lazy(gen);
    gen = next_gen;

    /*
    gen_delta_score *= 0.95;
    if(gen_delta_score < 1)
    {
        repeat(i, gen_length)
        {
            if(rand()%100 <= 50)
            {
                entity* e = gen_get(i);
                rule* r = rule_create(g);
                behavior_add_rule(entity_behavior(e), r);
                rule_free(r);
            }
        }
        gen_delta_score = 100;
    }*/
    /*
    entity* first = gen_get(0);
    vec_remove_at(gen, 0);
    entity_free(first);*/
    //entity* first = gen_get(0);
    //entity_free(first);
    //vec_set(gen, entity*, 0, entity_clone(best_entity));

    //return;
    gen_set(g, 0, best_entity);
}



void import_best_entity(context* c, game* g)
{
    unused(c);
    file* f = game_get_save_file(g, "r");

    if(f == null) 
    { 
        printf("import failed, no file\n");
        return;
    }

    entity* e = game_import_one_entity(g, f);
    if(e != null)
    {
        printf("import succeed\n");
        vec_add(gen, entity*, e);
        if (!g->internal_mutable_state->best_ordi) {entity_free(g->internal_mutable_state->best_ordi);}
        g->internal_mutable_state->best_ordi = entity_clone(e);
    }else
    {
        printf("import failed, entity null\n");
    }
    fclose(f);
}

void game_init_training_if_needed(context* c, game* g)
{
    unused(c);
    
    check(entity_per_gen >= 2);

    // call game_ordi_configure before init training
    check(mstate->input != null);

    if(gen != null) return;

    gen = vec_empty(entity*);

    //import_best_entity(c,g);
    repeat(i, entity_per_gen)
    {
        vec_add(gen, entity*, entity_create_ordi_random(g, 1));
    }

    //if(mstate->input == null)
    //a

    gen_idx_training = 0;
    gen_current_idx_nb_update = 0;
    // hardcoder
    gen_max_update_per_entity = 128;
    gen_delta_score = 4;
    gtype->total_entities_generated = 0;
}


void game_unload_copy(context* c, game* g)
{
    unused(c);

    gtype->unload_mutable(arg);
    free(g->mutable_state);
    //current_entity = null;
    game_internal_mutable_free(g->internal_mutable_state);
    free(g);
}


void update_current_entity(context* c, game* g)
{
    unused(c);

    //current_entity = entity_clone(gen_current_entity);
    //current_entity->score = 0;
    game* copy = game_clone(c, g);

    copy->internal_mutable_state->current_entity = gen_current_entity;
    game_reset(c, copy);

    gen_current_idx_nb_update = 0;
    while(gen_current_idx_nb_update < gen_max_update_per_entity && copy->internal_mutable_state->state == GAME_STATE_RUNNING)
    {
        tab_clear(copy->internal_mutable_state->input, 0);
        game_update_fixed(c, copy);
        gen_current_idx_nb_update++;
    }
    copy->internal_mutable_state->current_entity = null;
    game_unload_copy(c, copy);
    gen_idx_training++;
    gtype->total_entities_generated++;
}


// train the generation, and choose the best
void game_train_best_ordi(context* c, game* g)
{
    //return;
    game_init_training_if_needed(c, g);

    repeat(i, 100)
    {
        update_current_entity(c, g);
        if(gen_current_entity == null)
        {
            game_choose_next_generation(c, g);
        }
    }

    unused(c);
    unused(g);
}

void game_trainning_printf(context* c, game* g)
{
    unused(c);
    printf("\n");
    if(mstate->nb_generation == 0)
    {
        printf("generation 0\n");
    }else
    {
        printf("generation %i (%i/%i entities evaluated = %.0f %%), total %i entities\n", mstate->nb_generation, gen_idx_training, gen_length, gen_idx_training/(float)gen_length*100, gtype->total_entities_generated);
        //printf("generation %i (%i/%i entities evaluated = %.0f %%), total %i entities\n", mstate->nb_generation, gen_idx_training, entity_per_gen, gen_idx_training/(float)entity_per_gen, mstate->nb_generation*entity_per_gen+gen_idx_training);

    }
    printf("best entity :\n");
    entity_printf(g, best_entity);
    printf("\n");
}