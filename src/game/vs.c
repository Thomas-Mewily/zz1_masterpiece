#include "base.h"

#define player vs_players[idx]
#define gplayer (gstate->players_graphic[idx])

void get_vision(game_arg arg, int idx, vs_vision* v);

bool vs_inside_grid(int x, int y)
{
    return x >= 0 && x < vs_nb_colonne_x && y >= 0 &&  y < vs_nb_ligne_y;
}

vs_entity_dir dir_right(vs_entity_dir dir)
{
    switch (dir)
    {
        case vs_entity_dir_right: return vs_entity_dir_down;
        case vs_entity_dir_left:  return vs_entity_dir_up;
        case vs_entity_dir_up:    return vs_entity_dir_right;
        case vs_entity_dir_down:  return vs_entity_dir_left;    
        default: break;
    }
    return vs_entity_dir_right;
}  


vs_entity_dir dir_left(vs_entity_dir dir)
{
    repeat(i, 3)
    {
        dir = dir_right(dir);
    }
    return dir;
}


int damage_create(int right, int left, int up, int down, int top)
{
    return ((right & 1) << 0) | ((left & 1) << 1) | ((up & 1) << 2) | ((down & 1) << 3) | ((top & 1) << 4);
}

int damage_later(int damage)
{
    return (damage << 5) & 0b1111100000;
}

int damage_update(int damage)
{
    return (damage >> 5);
}

float get_player_score(game_arg arg, int idx)
{
    get_game_state(vs);
    if(idx == 0)
    {
        return current_entity->score;
    }  
    return -1;
}

void set_player_score(game_arg arg, int idx, float val)
{
    get_game_state(vs);
    if(idx == 0)
    {
        current_entity->score = val;
    }
}


void grid_set_damage(game_arg arg, int x, int y, int damage)
{
    get_game_state(vs);
    if(vs_inside_grid(x, y) == false) return;

    vs_grid_damage[x][y] = vs_grid_damage[x][y] | damage;
}

int damage_right(int d) { return (d & 0b1); }
int damage_left (int d) { return (d & 0b10); }
int damage_up   (int d) { return (d & 0b100); }
int damage_down (int d) { return (d & 0b1000); }
int damage_top  (int d) { return (d & 0b10000); }

bool vs_will_damage(game_arg arg, int x, int y)
{
    get_game_state(vs);
    return vs_inside_grid(x,y) ? (((vs_grid_damage[x][y] >> 5) & 0b11111) != 0) : false;
}

bool vs_can_damage(game_arg arg, int x, int y)
{
    get_game_state(vs);
    return vs_inside_grid(x,y) ? ((vs_grid_damage[x][y] & 0b11111) != 0) : false;
}

int vs_grid_wall_get(game_arg arg, int x, int y)
{
    get_game_state(vs);
    return vs_inside_grid(x,y) ? (vs_grid_wall[x][y]) : VS_INPUT_SOL;
}

void vs_init_grid(game_arg arg)
{
    get_game_state(vs);

    repeat(x, vs_nb_colonne_x)
    {
        repeat(y, vs_nb_ligne_y)
        {
            vs_grid_wall[x][y] = vs_wall_sol;
        }
    }
}

void vs_load(game_arg arg)
{
    get_game_state(vs);

    imstate->target_ups = 1.25f;

    vs_players[0].direction = vs_entity_dir_down;
    vs_players[0].x = vs_nb_colonne_x /2;
    vs_players[0].y = vs_nb_ligne_y-1;

    vs_players[1].direction = vs_entity_dir_up;
    vs_players[1].x = vs_nb_colonne_x /2;
    vs_players[1].y = 0;

    vs_players[2].direction = vs_entity_dir_left;
    vs_players[2].x = 0;
    vs_players[2].y = vs_nb_ligne_y /2;

    vs_players[3].direction = vs_entity_dir_right;
    vs_players[3].x = vs_nb_colonne_x-1;
    vs_players[3].y = vs_nb_ligne_y /2;

    repeat(x, vs_nb_colonne_x)
    {
        repeat(y, vs_nb_ligne_y)
        {
            vs_grid_damage[x][y] = vs_damage_none;
        }
    }

    repeat(idx, vs_max_player)
    {
        player.state = vs_entity_state_normal;

        if(gstate != null)
        {
            gplayer.x = player.x;
            gplayer.y = player.y;
        } 
    }

    if(dstate != null)
    {
        gstate->situations = vec_empty(vs_situation);
    }

    if(need_reset)
    {

    }else
    {
        game_ordi_configure(the_game, VS_INPUT_SIZE, VS_INPUT_MAX_RANGE, 1, VS_OUTPUT_MOVE_RANGE, 10);
        dstate->grass   = texture_create(c,"asset/fond.png");
        dstate->dance   = texture_create(c,"asset/tapis_dance.png");
        dstate->archere = texture_create(c,"asset/archere.png");
        dstate->damage  = texture_create(c,"asset/damage.png");
        vs_init_grid(arg);
    }
}

// décharge tout sauf le mutable state
void vs_unload(game_arg arg)
{
    get_game_state(vs);
    texture_free(dstate->grass);
    texture_free(dstate->dance);
    texture_free(dstate->archere);
    texture_free(dstate->damage);
    vec_free_lazy(gstate->situations);
}

void vs_unload_mutable(game_arg arg)
{
    get_game_state(vs);
}

void* vs_clone_mutable(game_arg arg)
{
    get_game_state(vs);
    vs_mutable_state* copy = create(vs_mutable_state);
    memccpy(copy, mstate, 1, sizeof(vs_mutable_state));
    return (void*)copy;
}

void vs_set_default_input(game_arg arg)
{
    get_game_state(vs);
    output_single_value(VS_OUTPUT_DO_NOTHINGS);
}

void vs_get_player_input(game_arg arg, int idx)
{
    get_game_state(vs);

    output_single_value(VS_OUTPUT_DO_NOTHINGS);

    if(input_up(c, idx))
    {
        output_single_value(VS_OUTPUT_MOVE_UP); return;
    }
    if(input_down(c, idx))
    {
        output_single_value(VS_OUTPUT_MOVE_DOWN); return;
    }
    if(input_right(c, idx))
    {
        output_single_value(VS_OUTPUT_MOVE_RIGHT); return;
    }
    if(input_left(c, idx))
    {
        output_single_value(VS_OUTPUT_MOVE_LEFT); return;
    }
    if(input_special0(c, idx))
    {
        output_single_value(VS_OUTPUT_MOVE_BOW); return;
    }
    if(input_special1(c, idx))
    {
        output_single_value(VS_OUTPUT_MOVE_SWORD); return;
    }
}

void coordinate_move(int* x, int* y, vs_entity_dir dir)
{
    switch (dir)
    {
        case vs_entity_dir_right:*x = x[0]+1; break;
        case vs_entity_dir_left: *x = x[0]-1; break;
        case vs_entity_dir_up:   *y = y[0]-1; break; // :'(
        case vs_entity_dir_down: *y = y[0]+1; break; // :'(        
        default: break;
    }
}




void vs_player_get_input(game_arg arg, int idx)
{
    get_game_state(vs);
    vs_set_default_input(arg);

    if(idx == 0) // player
    {
        game_get_input(c,the_game, current_entity);
    }else if(current_entity->type == ENTITY_TYPE_PLAYER)
    {
        vs_get_player_input(arg, idx);
        //vs_player_get_input(arg, idx, 1);
    }
}

#if 0
void vs_player_get_input(game_arg arg, int idx)
{
    get_game_state(vs);
    vs_set_default_input(arg);

    if(idx == 0) // player
    {
        game_get_input(c,the_game, current_entity);
    }else if(current_entity->type == ENTITY_TYPE_PLAYER)
    {

        if(the_game->internal_mutable_state->is_training_ia)
        {

        }else
        {
            
            vs_get_player_input(arg, idx);
            //vs_output player_input = tab_first_value(entity_input);
            /*
            vs_situation s;
            s.output = player_input;
            get_vision(arg, idx, &s.vision);
            vec_add(gstate->situations, vs_situation, s);*/
        }
        // p2

        //vs_player_get_input(arg, idx, 1);
    }
}
#endif

void player_move_dir(game_arg arg, int idx, vs_output dir)
{
    get_game_state(vs);
    int x = player.x;
    int y = player.y;
    coordinate_move(&x,&y, dir);
    if(vs_inside_grid(x,y))
    {
        player.x = x;
        player.y = y;
    }
    player.direction = dir;
}



void bow_attack(game_arg arg, int x, int y, int dx, int dy)
{
    get_game_state(vs);

    while(vs_inside_grid(x,y))
    {
        grid_set_damage(arg, x, y, damage_create(dx>0,dx<0,dy<0,dy>0,0));
        x += dx;
        y += dy;
    }
}

void remove_damage(game_arg arg)
{
    get_game_state(vs);

    repeat(y, vs_nb_ligne_y)
    {
        repeat(x, vs_nb_colonne_x)
        {
            vs_grid_damage[x][y] = damage_update(vs_grid_damage[x][y]);
        }
    }
}

void vs_update_player(game_arg arg, int idx)
{
    get_game_state(vs);


    if(player.state == vs_entity_state_dead) return;

    vs_player_get_input(arg, idx);
    vs_output player_input = tab_first_value(entity_input);

    set_player_score(arg, idx, get_player_score(arg, idx) + 1);

    if(player.state == vs_entity_state_load_bow) 
    {
        player.var_0--;
        if(player.var_0 == vs_bow_loading_time-1)
        {
            switch (player_input)
            {
                case VS_OUTPUT_MOVE_RIGHT:
                case VS_OUTPUT_MOVE_LEFT:
                case VS_OUTPUT_MOVE_UP:
                case VS_OUTPUT_MOVE_DOWN:
                    player.direction = player_input;
                break;
                default:break;
            }

            int dx = 0;
            int dy = 0;
            coordinate_move(&dx, &dy, player.direction);
            bow_attack(arg, player.x+dx, player.y+dy, dx, dy);

            return;
        }else if(player.var_0 <= 0)
        {
            player.state = vs_entity_state_normal;
        }
        //return;
    }

    if(player.state == vs_entity_state_attack_sword) 
    {
        int dx;
        int dy;

        dx = 0;
        dy = 0;
        coordinate_move(&dx, &dy, player.direction);
        grid_set_damage(arg, player.x+dx, player.y+dy, damage_create(0,0,0,0,1));
        
        dx = 0;
        dy = 0;
        coordinate_move(&dx, &dy, dir_right(player.direction));
        grid_set_damage(arg, player.x+dx, player.y+dy, damage_create(0,0,0,0,1));
        
        dx = 0;
        dy = 0;
        coordinate_move(&dx, &dy, dir_left(player.direction));
        grid_set_damage(arg, player.x+dx, player.y+dy, damage_create(0,0,0,0,1));
        

        
        player.state = vs_entity_state_normal;
        return;
    }

    switch (player_input)
    {
        case VS_OUTPUT_DO_NOTHINGS: break;
        case VS_OUTPUT_MOVE_RIGHT:
        case VS_OUTPUT_MOVE_LEFT:
        case VS_OUTPUT_MOVE_UP:
        case VS_OUTPUT_MOVE_DOWN:
            player_move_dir(arg, idx, player_input);
            break;
        case VS_OUTPUT_MOVE_SWORD:
            player.state = vs_entity_state_attack_sword;
            break;
        case VS_OUTPUT_MOVE_BOW:
            player.var_0 = vs_bow_loading_time;
            player.state = vs_entity_state_load_bow;
            break;
            default:
        break;
    }
}

void vs_update(game_arg arg)
{
    get_game_state(vs);
    if(current_game_state != GAME_STATE_RUNNING) return;

    remove_damage(arg);


    repeat(idx, vs_nb_player)
    {
        vs_update_player(arg, idx);
    }
    int nb_player_alive = vs_nb_player;
    repeat(idx, vs_nb_player)
    {
        if(player.state != vs_entity_state_dead)
        {
            if(vs_can_damage(arg, player.x, player.y))
            {
                player.state = vs_entity_state_dead;
            }
        }else
        {
            nb_player_alive--;
        }
    }

    if(nb_player_alive <= 1)
    {
        repeat(idx, vs_nb_player)
        {
            if(player.state != vs_entity_state_dead)
            {
                player.state = vs_entity_state_win; 
                set_player_score(arg, idx, get_player_score(arg, idx) + 1000000.f / nb_update);
            }
        }
        current_game_state = GAME_STATE_GAME_OVER;
    }
}

#define vs_lerp lerpf

void vs_draw_player(game_arg arg, int idx)
{
    get_game_state(vs);
    vs_entity* p = &(player);

    int direction_y_offset = 0;
    switch (p->direction)
    {
        case vs_entity_dir_down: direction_y_offset = 0; break;
        case vs_entity_dir_up:   direction_y_offset = 4; break;
        case vs_entity_dir_right:direction_y_offset = 2; break;
        case vs_entity_dir_left: direction_y_offset = 6; break;
        default: break;
    }

    int line = 0;
    int nb_frame = 1;
    int fps = 1;
    int frame = -1;
    switch (p->state)
    {
        case vs_entity_state_dead: fps = 3; nb_frame = 2; line = 5; direction_y_offset = 0; break;
        case vs_entity_state_win: fps = 3; nb_frame = 2; line = 5; direction_y_offset = 1; break;
        case vs_entity_state_load_bow:
        {
            fps = -1;
            nb_frame = 5;
            line = 4;
            /*
            if(player.var_0 == vs_bow_loading_time)
            {
                fps = 3;
                nb_frame = 2;
                line = 1;
            }else */
            if(player.var_0 < vs_bow_loading_time)
            {
                frame = 0;
            }
        }break;
        case vs_entity_state_attack_sword: fps = 1; nb_frame = 1; line = 0; break;
        case vs_entity_state_normal: 
        {
            if(length(gplayer.x,gplayer.y,player.x,player.y) <= 0.1)
            {
                fps = 7; nb_frame = 2; line = 0;     
            }else
            {
                fps = 7; nb_frame = 6; line = 2;     
            }
        }break;
        default: break;
    }

    if(frame == -1)
    {
        frame = (fps > 0) ? ((int)((c->timer / frequence_s(fps)) % nb_frame)) : ((int)(draw_coef*nb_frame));
    }

    float coef = 0.01;
    gplayer.x = vs_lerp(gplayer.x, player.x, coef);
    gplayer.y = vs_lerp(gplayer.y, player.y, coef);

    rect src = rectangle(32*frame+32*7*idx,32*(line*8+direction_y_offset),32,32);
    rectf dest = rectanglef(gplayer.x-0.5, gplayer.y-1,2,2);

    pen_texture(c, dstate->archere, src, dest);
}

void vs_draw_damage(game_arg arg, int x, int y)
{
    get_game_state(vs);

    int damage = vs_grid_damage[x][y];
    rect src = rectangle(0, 0, 16, 16);
    rectf dest = rectanglef(x, y,1,1);

    int nb_frame = 12;
    
#define damage_frame (int)(nb_frame*draw_coef)

    if(damage_right(damage))
    {
        src = rectangle(damage_frame*16, 16*0, 16, 16);
        pen_texture(c, dstate->damage, src, dest);
    }
    if(damage_left(damage))
    {
        src = rectangle(damage_frame*16, 16*1, 16, 16);
        pen_texture(c, dstate->damage, src, dest);
    }
    if(damage_up(damage))
    {
        src = rectangle(damage_frame*16, 16*2, 16, 16);
        pen_texture(c, dstate->damage, src, dest);
    }
    if(damage_down(damage))
    {
        src = rectangle(damage_frame*16, 16*3, 16, 16);
        pen_texture(c, dstate->damage, src, dest);
    }
    if(damage_top(damage))
    {
        src = rectangle(damage_frame*16, 16*4, 16, 16);
        pen_texture(c, dstate->damage, src, dest);
    }
}

void vs_draw_cadence(game_arg arg)
{
    get_game_state(vs);
    float y = window_height(c)*0.9f;
    float tickness = window_height(c)*0.025f;
    float pulse_tickness_y = window_height(c)*0.1f;
    float pulse_tickness_x = window_width(c)*0.01f;
    pen_color(c, rgba(128, 128, 128, 127));
    pen_rect(c, rectanglef(0, y-tickness/2, window_width(c), tickness));
    int max_pulse_display = 4;

    pen_color(c, rgb(255, 255, 255));

    repeat(i, max_pulse_display+1)
    {
        float j = i+1-(draw_coef+0.2);
        if(j < 0) { continue; }
        float x = ((j/(float)max_pulse_display))*window_width(c)/2;
        pen_rect(c, rectanglef(window_width(c)/2+x-pulse_tickness_x/2, y-pulse_tickness_y/2, pulse_tickness_x, pulse_tickness_y));
        pen_rect(c, rectanglef(window_width(c)/2-x-pulse_tickness_x/2, y-pulse_tickness_y/2, pulse_tickness_x, pulse_tickness_y));
    }
}

void vs_draw(game_arg arg)
{
    get_game_state(vs);

    float coef = draw_coef;
    unused(coef);

    float offset = 1;
    rectf area = rectanglef(-offset,-offset, vs_nb_colonne_x+2*offset, vs_nb_ligne_y+2*offset+1);
    area = camera_push_focus_fullscreen(c, area);
    
    for(int x = floor(area.x); (float)x <= area.x+area.w; x++)
    {
        for(int y = floor(area.y);  (float)y <= area.y+area.h; y++)
        {
            rect fond_rect = texture_rect(dstate->grass);
            fond_rect.w /= 2;
            fond_rect.h /= 2;
            int parite = (x+y+10000) % 2;
            fond_rect.x = fond_rect.w *parite;
            bool inside = vs_inside_grid(x,y);
            fond_rect.y = fond_rect.h * (inside ? 0 : 1);
            pen_texture(c,dstate->grass,fond_rect,rectanglef(x,y,1,1));
            if(inside)
            {
                pen_texture(c,dstate->dance,rectangle(16*((int)(imstate->nb_update+(coef+0.2) +x+y)%2) ,0,16,16),rectanglef(x,y,1,1));
            }
        }
    }

    repeat(y, vs_nb_ligne_y)
    {
        repeat(x, vs_nb_colonne_x)
        {
            repeat(i, vs_nb_player)
            {
                if(vs_players[i].y == y)
                {
                    vs_draw_player(arg, i);
                }

                if(vs_can_damage(arg, x, y))
                {
                    vs_draw_damage(arg, x, y);
                }
            }
        }
    }

    camera_pop(c);

    vs_draw_cadence(arg);

    pen_formatted_text_at_center(c, window_width(c)/2, 0, FONT_SIZE_NORMAL, 0.5, 0, "p1 %.0f p2 %.0f", get_player_score(arg, 0), get_player_score(arg, 1));

    if(current_game_state == GAME_STATE_GAME_OVER){
         pen_text_at_center(c, "Fin de Partie", window_width(c)/2, window_height(c), FONT_SIZE_NORMAL, 0.5, 1);
    }
}

char vs_rule_output_to_char(int output)
{
    return " ^v><BA"[output];
}
char vs_rule_input_to_char(int input)
{
    return "X.#!?EFFF"[input];
}

void vs_draw_rule(game_arg arg, entity* e, rule* r, int idx)
{
    get_game_state(vs);
    unused(idx);
    unused(e);
    unused(r);
}



void vs_player_input(game_arg arg, entity* e)
{
    unused(e);
    vs_get_player_input(arg,0);
}



void get_coord_relative_to_player(game_arg arg, int idx, int forward, int side, int* rx, int* ry)
{
    get_game_state(vs);
    int X = player.x;
    int Y = player.y;
    switch (player.direction)
    {
        case vs_entity_dir_up:
        X += side;
        Y += forward;
        break;

        case vs_entity_dir_down:
        X -= side;
        Y -= forward;
        break;

        case vs_entity_dir_right:
        X+=forward;
        Y-=side;
        break;

        case vs_entity_dir_left:
        X-=forward;
        Y+=side;
        break;
    
        default: break;
    }
    *rx = X;
    *ry = Y;
}

int vision_idx(int x, int y)
{
    return x+half_vision_size+(y+half_vision_size)*vision_size;
}

void get_vision(game_arg arg, int idx, vs_vision* v)
{
    get_game_state(vs);

    //int idx = e->id;

    repeat(i, VS_INPUT_SIZE)
    {
        v->grid_input[i] = VS_INPUT_SOL;
    }

    int dx = 0;
    int dy = 0;
    coordinate_move(&dx, &dy, player.direction);

    for(int x = -half_vision_size; x <= half_vision_size; x++)
    {
        for(int y = -half_vision_size; y <= half_vision_size; y++)
        {
            if(x == 0 && y == 0) continue;
            int rx = 0;
            int ry = 0;
            get_coord_relative_to_player(arg, idx, x, y, &rx, &ry);

            if(vs_inside_grid(rx, ry) == false)
            {
                v->grid_input[vision_idx(x,y)] = VS_INPUT_OUT_OF_ARENA;
            }else
            {
                int current_input = VS_INPUT_SOL;
                /*
                if(vs_can_damage(arg, rx, ry) && rx == )
                {
                    current_input = VS_INPUT_DAMAGE;
                }else */
                if(vs_will_damage(arg, rx, ry))
                {
                    current_input = VS_INPUT_FUTUR_DAMAGE;
                }else 
                {
                    repeat(i, vs_nb_player)
                    {
                        if(i == idx) continue;
                        if(vs_players[i].x == rx && vs_players[i].y == ry)
                        {
                            current_input = VS_INPUT_ENEMY_NORMAL;
                            break;
                        }
                    }
                }
                v->grid_input[vision_idx(x,y)] = current_input;
            }
        }
    }
}

bool vs_rule_match(game_arg arg, entity* e, rule* r)
{
    /**/
    get_game_state(vs);

    int idx = e->id;
    check(idx == 0);

    tab_as_array(r->input, rule_in);
    unused(rule_in_size);

    bool rule_match = true;
            
    repeat(i, rule_in_size)
    {
        //if(rule_in[i] <= VS_INPUT_OSEF) continue;
        if(rule_in[i] != mstate->ordi_vision.grid_input[i] && rule_in[i] != VS_INPUT_OSEF)
        {
            rule_match = false;
        }else
        {
            // Osef an other go here
            input_symbol_useful_at(i);
        }
    }

    // match
    if(rule_match)
    {
        output_single_value(tab_first_value(r->output));
    }
    
    unused(e);
    unused(r);
    return rule_match;
}

void vs_printf(game_arg arg)
{
    unused(arg);
    printf("VerSUS game\n");
}

void vs_ordi_input_init(game_arg arg)
{
    get_game_state(vs);
    get_vision(arg, current_entity->id, &mstate->ordi_vision);
}