// PacMan
// 2025 M. Gerloff


#include "pacman.hpp"
#include "assets.hpp"

#define CENTER 40

using namespace blit;

struct GAME
{
    short state;
    short map[30][30];
    float ghost_dir[30][30];
    short ani;
    short dots;
    short level;
};

struct PLAYER
{
    short sprite;
    short new_sprite;
    Point delta_pos;
    Point grid_pos;
    short dir;
    short life;
    int score;
    short died_ani;
};

struct GHOST
{
    Vec2 delta_pos;
    Point grid_pos;
    short dir;
    short state;
};

GAME game;
PLAYER p;
GHOST ghost[4];
TileMap* map;

Timer ani_timer;
Timer fear_timer;
Timer end_fear_timer;
Timer died_timer;
Tween gate_tween;
Tween fruit_tween;

void start();

Mat3 callback(uint8_t){
    Mat3 transform = Mat3::translation(Vec2(0, 0) - Vec2(CENTER, 0));
    return transform;
};

void UpdateAni(Timer &t){
    game.ani++;
    if(game.ani == 4)
        game.ani = 0;
}

void UpdateFear(Timer &t){
    for(short g=0; g<4; g++){
        if(ghost[g].state == 1)
            ghost[g].state = 2;
        end_fear_timer.start();  
    }          
}

void UpdateEndFear(Timer &t){
    for(short g=0; g<4; g++){
        if(ghost[g].state == 2)
            ghost[g].state = 0;
    }          
}

void UpdateDied(Timer &t){
    p.died_ani++;
    if(p.died_ani == 10){
        p.died_ani = 0;
        died_timer.stop();
        p.life--;
        if(p.life == 0)
            game.state = 0;
        else{
            p.sprite = 1;
            p.grid_pos = Point(14, 22);
            p.delta_pos = Point(0, 0);
            p.dir = 4; 

            for (short g=0; g<4; g++){
                ghost[g].dir = 1;
                ghost[g].grid_pos = Point(13, 13);
                ghost[g].delta_pos = Vec2(g * 6, 0);
                ghost[g].state = 4;
            }
            game.state = 1;
        }
    }
}

bool collision(float x, float y){
    float dx = (p.grid_pos.x * 8 + p.delta_pos.x) - x;
    float dy = (p.grid_pos.y * 8 + p.delta_pos.y) - y;
    float hyp = sqrt((dx * dx) + (dy * dy));
    if (hyp < 14)
        return true;
    return false;
}

void UpdateControl(){
    Point dir[5] = {Point(0, -1), Point(1, 0), Point(0, 1), Point(-1, 0), Point(0, 0)};

    if(buttons & Button::DPAD_UP)
        p.new_sprite = 0;
    else if(buttons & Button::DPAD_RIGHT)
        p.new_sprite = 1;
    else if(buttons & Button::DPAD_DOWN)
        p.new_sprite = 2;
    else if(buttons & Button::DPAD_LEFT)
        p.new_sprite = 3;

    p.delta_pos += dir[p.dir];
    if(p.delta_pos == (dir[p.dir] * 8)){
        p.delta_pos = Point(0, 0);
        p.grid_pos += dir[p.dir];
        if(p.grid_pos.x > 28)
            p.grid_pos.x = 0;
        else if(p.grid_pos.x < 1)
            p.grid_pos.x = 29;
        else if(game.map[p.grid_pos.x][p.grid_pos.y] == 1){ // dots
            game.map[p.grid_pos.x][p.grid_pos.y] = 0;
            game.dots--;
            if(game.dots == 100)
                fruit_tween.start();
            else if(game.dots == 0){
                game.level++;
                start();
            }
        }
        else if(game.map[p.grid_pos.x][p.grid_pos.y] == 2){ // pill
            game.map[p.grid_pos.x][p.grid_pos.y] = 0;
            for(short g=0; g<4; g++){
                if(ghost[g].state == 0 || ghost[g].state == 2)
                    ghost[g].state = 1;
                fear_timer.start();  
            }          
        }
        if(game.map[p.grid_pos.x + dir[p.new_sprite].x][p.grid_pos.y + dir[p.new_sprite].y] < 3){
            p.sprite = p.new_sprite;
            p.dir = p.new_sprite;
        }
        else if(game.map[p.grid_pos.x + dir[p.sprite].x][p.grid_pos.y + dir[p.dir].y] > 2)
            p.dir = 4;

        for(short y=1; y<30; y+=3){
            for(short x=1; x<30; x+=3){
                game.ghost_dir[x][y] = sqrt(((p.grid_pos.x - x) * (p.grid_pos.x - x)) + ((p.grid_pos.y - y) * (p.grid_pos.y - y)));
            }
        }
    }
}

void UpdateGhost(){
    Point dir[4] = {Point(0, -1), Point(1, 0), Point(0, 1), Point(-1, 0)};
    float speed = (4 + game.level) * .1f;
    if(speed > .8f)
        speed = .8f;

    for(short g=0; g<4; g++){ 
       ghost[g].delta_pos += Vec2(dir[ghost[g].dir].x * speed, dir[ghost[g].dir].y * speed);
        if(ghost[g].delta_pos == Vec2(dir[ghost[g].dir].x * 24, dir[ghost[g].dir].y * 24)){
            ghost[g].delta_pos = Vec2(0, 0);
            ghost[g].grid_pos += (dir[ghost[g].dir] * 3);

            short x = ghost[g].grid_pos.x;
            short y = ghost[g].grid_pos.y;

            if(x < 1)
                ghost[g].grid_pos.x = 28;
            else if(x > 28)
                ghost[g].grid_pos.x = 1;
            else if(ghost[g].state == 3 && x == 10 && y == 13)
                ghost[g].dir = 1;
            else if(ghost[g].state == 3 && x == 13 && y == 13){
                ghost[g].state = 4;
                gate_tween.start();
            }
            else if(ghost[g].state == 4){
                if(gate_tween.value > 1000 && fear_timer.is_running() == false && end_fear_timer.is_running() == false){
                    gate_tween.start();
                    ghost[g].state = 0;
                }
                else{
                    ghost[g].dir += 2;
                    if(ghost[g].dir > 3)
                        ghost[g].dir -= 4;
                }
            }            
            else{
                float value = 999;
                short newdir = ghost[g].dir;

                for(short i=0; i<4; i++){
                    if(dir[ghost[g].dir] + dir[i] == Point(0, 0) || game.map[x + dir[i].x][y + dir[i].y] > 2)
                        continue;
                    else if(ghost[g].state == 3){
                        float hyp = sqrt(((3 * dir[i].x + x - 10) * (3 * dir[i].x + x - 10)) + ((3 * dir[i].y + y - 13) * (3 * dir[i].y + y - 13)));
                        if(hyp < value){
                            value = hyp;
                            newdir = i;
                        }
                    }
                    else if(ghost[g].state == 0){
                        if(game.ghost_dir[x + (dir[i].x * 3)][y + (dir[i].y * 3)] < value){
                            value = game.ghost_dir[x + (dir[i].x * 3)][y + (dir[i].y * 3)];
                            newdir = i;
                        }
                    }
                    else{
                        if(value == 999 || game.ghost_dir[x + (dir[i].x * 3)][y + (dir[i].y * 3)] > value){
                            value = game.ghost_dir[x + (dir[i].x * 3)][y + (dir[i].y * 3)];
                            newdir = i;
                         }
                    }
                }
                ghost[g].dir = newdir;
            }
        }
    }
}

void start(){
    game.dots = 0;

// Load the map data from the map memory
    TMX *tmx = (TMX *)asset_tilemap;
    for(short y = 0; y < 30; y++){
        for(short x = 0; x < 30; x++){
            short tile = tmx->data[y * 32 + x - 16];
            if(tile == 105){ // video waffel
                game.map[x][y] = 1;
                game.dots++;
            }
            else if(tile == 106) // pill
                game.map[x][y] = 2;
            else if(tile == 107) // blockade
                game.map[x][y] = 3;
            else if(tile < 32) // wall
                game.map[x][y] = 4;
            else // empty
                game.map[x][y] = 0;
        }
    }            

    p.sprite = 1;
    p.grid_pos = Point(14, 22);
    p.delta_pos = Point(0, 0);
    p.dir = 4;

    for (short g=0; g<4; g++){
        ghost[g].dir = 1;
        ghost[g].grid_pos = Point(13, 13);
        ghost[g].delta_pos = Vec2(g * 6, 0);
        ghost[g].state = 4;
    }
}

// init
void init(){
    blit::set_screen_mode(ScreenMode::hires);
    screen.sprites = Surface::load(spritesheet);

    map = new TileMap((uint8_t*)asset_tilemap, nullptr, Size(32, 32), screen.sprites);

    ani_timer.init(UpdateAni, 75, -1);
    ani_timer.start();

    fear_timer.init(UpdateFear, 4000, 1);

    end_fear_timer.init(UpdateEndFear, 1000, 1);

    died_timer.init(UpdateDied, 150, -1);

    gate_tween.init(tween_linear, 0, 2000, 2000, -1);
    gate_tween.start();

    fruit_tween.init(tween_linear, 0, 1, 6000, 1);
}

// render
void render(uint32_t time_ms){
    screen.alpha = 255;
    screen.pen = Pen(0, 0, 0);
    screen.clear();

    screen.alpha = 255;
    screen.mask = nullptr;

    if(game.state == 0){
        screen.sprite(Rect(0, 13, 16, 3), Point(56 + CENTER, 108));
        screen.pen = Pen(255, 255, 255);
        screen.text("PRESS X TO START", minimal_font, Point(120 + CENTER, 200), true, TextAlign::top_center);
    }
    else{
        map->draw(&screen, Rect(CENTER, 0, 240, 240), callback); // map

        short pill_ani[4]{110, 111, 110, 109};
        for(short y = 1; y < 29; y++){
            for(short x = 1; x < 29; x++){
                if(game.map[x][y] == 1)
                    screen.sprite(108, Point(x * 8 + CENTER, y * 8));
                if(game.map[x][y] == 2)
                    screen.sprite(pill_ani[game.ani], Point(x * 8 + CENTER, y * 8));
            }
        }

        if(fruit_tween.is_running())
            screen.sprite(Rect(game.level * 2, 11, 2, 2), Point(112 + CENTER, 124));

        short gs[6] = {0, 2, 0, 2, 4, 6};
        for(short g=0; g<4; g++){
            if(ghost[g].state == 1){ // feared ghost
                screen.sprite(Rect(gs[game.ani], 9, 2, 2), 
                              Point(ghost[g].grid_pos.x * 8 + ghost[g].delta_pos.x - 4 + CENTER, ghost[g].grid_pos.y * 8 + ghost[g].delta_pos.y - 4));
            }
            else if(ghost[g].state == 2){ // end feared ghost
                screen.sprite(Rect(gs[game.ani + 2], 9, 2, 2), 
                              Point(ghost[g].grid_pos.x * 8 + ghost[g].delta_pos.x - 4 + CENTER, ghost[g].grid_pos.y * 8 + ghost[g].delta_pos.y - 4));
            }
            else{
                if(ghost[g].state == 0 || ghost[g].state == 4){ // normal
                    screen.sprite(Rect((g * 4) + gs[game.ani], 7, 2, 2), 
                                  Point(ghost[g].grid_pos.x * 8 + ghost[g].delta_pos.x - 4 + CENTER, ghost[g].grid_pos.y * 8 + ghost[g].delta_pos.y - 4));
                }
                screen.sprite(Rect(ghost[g].dir * 2, 6, 2, 1), 
                              Point(ghost[g].grid_pos.x * 8 + ghost[g].delta_pos.x - 4 + CENTER, ghost[g].grid_pos.y * 8 + ghost[g].delta_pos.y - 2));
            }
        }

        if(game.state == 1){
            Point sprite[16] = {Point(0, 2), Point(2, 2), Point(0, 4), Point(0, 2),
                                Point(4, 2), Point(6, 2), Point(0, 4), Point(4, 2),
                                Point(8, 2), Point(10, 2), Point(0, 4), Point(8, 2),
                                Point(12, 2), Point(14, 2), Point(0, 4), Point(12, 2)};
            screen.sprite(Rect(sprite[p.sprite * 4 + game.ani].x, sprite[p.sprite * 4 + game.ani].y, 2, 2), 
                          Point(p.grid_pos.x * 8 + p.delta_pos.x - 4 + CENTER, p.grid_pos.y * 8 + p.delta_pos.y - 4));
        }
        else if(game.state == 2){
            Point died[10] = {Point(0, 4), Point(2, 2), Point(0, 2), Point(2, 4),
                                Point(4, 4), Point(6, 4), Point(8, 4), Point(10, 4),
                                Point(12, 4), Point(14, 4)};
            screen.sprite(Rect(died[p.died_ani].x, died[p.died_ani].y, 2, 2), 
                          Point(p.grid_pos.x * 8 + p.delta_pos.x - 4 + CENTER, p.grid_pos.y * 8 + p.delta_pos.y - 4));
        }

        for(short i=0; i<p.life; i++) // life
            screen.sprite(107, Point(8 + (i * 4) + CENTER, 232));

        screen.pen = Pen(255, 255, 255);
        screen.text(std::to_string(game.dots), minimal_font, Point(232 + CENTER, 232), true, TextAlign::top_right);


/*

        if(game.state == 4){ // game over
            screen.pen = Pen(255, 255, 255);
            screen.text("GAME OVER", font, Point(80, 56), true, TextAlign::center_center);
        }
*/
    }
    
//    screen.pen = Pen(255, 255, 255);
//    screen.text(std::to_string(p.start), font, Point(134, 114), true, TextAlign::top_right); // object.size()
//    screen.text(std::to_string(game.map[p.grid_pos.x][p.grid_pos.y]), minimal_font, Point(280, 114), true, TextAlign::top_right);
//    screen.watermark();  
}

// update
void update(uint32_t time){
    if(game.state == 0){ // title
        if(buttons.released & Button::A){
            p.life = 3;
            p.score = 0;
            game.level = 0;
            start();
            game.state = 1;
        }
    }
    else if(game.state == 1){ // game
        UpdateGhost();
        UpdateControl();

        for(short g=0; g<4; g++){
            if(collision(ghost[g].grid_pos.x * 8 + ghost[g].delta_pos.x, ghost[g].grid_pos.y * 8 + ghost[g].delta_pos.y)){
                if(ghost[g].state == 0){
                    died_timer.start();
                    game.state = 2;
                }
                if(ghost[g].state == 1 || ghost[g].state == 2){
                    ghost[g].state = 3;
                }
            }
        }

        if(fruit_tween.is_running() && collision(116, 128)){
            fruit_tween.stop();
        }
        
    }
    else if(game.state == 3){ // game over
        if(buttons.released & Button::A){
            game.state = 0;
        }
    }
}
