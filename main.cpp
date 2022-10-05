#include <simplecpp>
#include "shooter.h"
#include "bubble.h"
#include<cmath>
#include<sstream>

/* Simulation Vars */
const double STEP_TIME = 0.02;

/* Game Vars */
const int PLAY_Y_HEIGHT = 450;
const int LEFT_MARGIN = 70;
const int TOP_MARGIN = 20;
const int BOTTOM_MARGIN = (PLAY_Y_HEIGHT+TOP_MARGIN);
const int max_Level = 3;
const int max_health = 3;
const double max_level1_time = 30.00;

//  string stream is used to make strings containing variables
string convert2Str(double d){
    ostringstream os;

    os << (int)d;
    return os.str();
}

// a struct to easily pass values for different levels
struct lvldets {
    int curr_Level ;
    double get_numBubbles() {
        if (curr_Level == 1) {return 2;}
        if (curr_Level == 2) {return 3;}
        if (curr_Level == 3) {return 4;}
    }
    double get_min_rad() {
        if (curr_Level == 1) {return BUBBLE_DEFAULT_RADIUS;}
        if (curr_Level == 2) {return BUBBLE_DEFAULT_RADIUS;}
        if (curr_Level == 3) {return ((3*BUBBLE_DEFAULT_RADIUS/4)+1);}
    }
    Color get_color() {
        if (curr_Level == 1) {return COLOR(255,105,180);}
        if (curr_Level == 2) {return COLOR(255,100,0);}
        if (curr_Level == 3) {return COLOR(100,255,0);}
    }
    double get_time() {
        if (curr_Level == 1) {return 30;}
        if (curr_Level == 2) {return 40;}
        if (curr_Level == 3) {return 50;}
    }

} ;

// intersection between bubbles and bullets
bool intersectionBB(vector<Bubble> &bubbles , Bullet b , int& score, int& curr_Level) {
    int x1, x2, y1, y2, r;

    x1= b.get_center_x();
    y1=b.get_center_y();

    lvldets lvls = {curr_Level} ;

    // checking for every bubble with a bullet
    for (int j = 0; j < bubbles.size(); j++){
        Bubble bubble = bubbles.at(j);
        r = bubble.get_radius();

        x2 = bubble.get_center_x();
        y2 = bubble.get_center_y();

        // we use edge interaction conditions using the "and" operator for intersection
        if (abs(x1-x2)<abs(b.get_width()/2 + r) && abs(y1-y2)<abs(b.get_height()/2 + r)) {
                bubbles.erase(bubbles.begin()+j);
                if (r > lvls.get_min_rad()){
                    bubbles.push_back(Bubble(x1, y1, r/2, BUBBLE_DEFAULT_VX, BUBBLE_DEFAULT_VY, lvls.get_color()));
                    bubbles.push_back(Bubble(x1, y1, r/2, -BUBBLE_DEFAULT_VX, BUBBLE_DEFAULT_VY, lvls.get_color()));
                }
                score++ ;
                return true ;
        }
    }

    return false;
}

// intersection between shooter and bubbles - similarity with the above intersectionBB
bool intersectionSB(Shooter &shooter , vector<Bubble> &bubbles, bool isBubbleContacted, int& remainingHealth) {
    int xc, xr, yc, yr, rc ;

    xc= shooter.get_head_center_x();
    yc= shooter.get_head_center_y();
    xr= shooter.get_body_center_x();
    yr= shooter.get_body_center_y();
    rc= shooter.get_head_radius();

    for (unsigned int i=0 ; i < bubbles.size() ; i++) {
        Bubble bubble = bubbles.at(i);
        int xB , yB , rB ;
        xB= bubble.get_center_x();
        yB= bubble.get_center_y();
        rB= bubble.get_radius();

        // check intersection by dividing the shooter into two parts - the head and the body - and applying the "or" operator
        if (((abs(xr-xB)<abs(shooter.get_body_width()/2 + rB)) &&
            (abs(yr-yB)<abs(shooter.get_body_height()/2 + rB))) ||
            ((abs(xB-xc)<abs(rB+rc)) &&
            (abs(yB-yc)<abs(rB+rc)))) {
                if (!isBubbleContacted){
                    shooter.setColor(COLOR(0,0,255));
                    remainingHealth--;
                }
                return true ;
        }
    }

    shooter.setColor(COLOR(0,255,0));
    return false;
}

void move_bullets(vector<Bubble> &bubbles, vector<Bullet> &bullets, int& score, int& curr_Level){
    for(unsigned int i=0; i<bullets.size();){
        if(!bullets[i].nextStep(STEP_TIME)){
            bullets.erase(bullets.begin()+i);
        }else{
            if (intersectionBB(bubbles, bullets[i], score, curr_Level)){ //check interaction for all bullets and ease them if found true
                bullets.erase(bullets.begin()+i);
            }else{
                i++;
            }
        }
    }
}

void move_bubbles(vector<Bubble> &bubbles){
    for (unsigned int i=0; i < bubbles.size(); i++)
    {
        bubbles[i].nextStep(STEP_TIME);
    }
}

vector<Bubble> create_bubbles(int& curr_Level){
    vector<Bubble> bubbles;
    lvldets lvl = {curr_Level};
    int x = lvl.get_numBubbles() ;
    for (int i = 0; i < x ; i++){
        bubbles.push_back(Bubble((i+1)*WINDOW_X/(x+1), BUBBLE_START_Y, (BUBBLE_DEFAULT_RADIUS)*(x-1), pow((-1),i)*BUBBLE_DEFAULT_VX, BUBBLE_DEFAULT_VY, lvl.get_color()));
    }
    return bubbles;
}

int main()
{
    int curr_Level=1;
    int remainingHealth = 3;
    int score = 0;
    int counter = 0 ;
    double time = 0.00 ;
    bool level_over = false;
    bool game_over = false;
    bool isBubbleContacted = false;

    // making of canvas
    initCanvas("Bubble Trouble", WINDOW_X, WINDOW_Y);

    // displays the initial level1 message
    int some = 0 ;
    while (some == 0) {
        Text initLevel(WINDOW_X/2 , WINDOW_Y/2 , "Level: 1!") ;
        wait (2) ;
        some++ ;
    }

    Line b1(0, PLAY_Y_HEIGHT, WINDOW_X, PLAY_Y_HEIGHT);
    b1.setColor(COLOR(0, 0, 255));

    // series of strings to show the messages on canvas
    string msg_cmd("Cmd: _");
    Text charPressed(LEFT_MARGIN, BOTTOM_MARGIN, msg_cmd);

    string game_over_str("");
    Text gameOver(WINDOW_X/2, WINDOW_Y/2, game_over_str);

    string levelMaxTimeStr = convert2Str(max_level1_time);
    string time_m("Time: ");
    string timer_str(time_m + "0 / " + levelMaxTimeStr) ;
    Text Timer(50 , 20 , timer_str) ;

    string levelMaxHealthStr = convert2Str(max_health);
    string health_m("Health: ") ;
    string health_str(health_m + "3 / " + levelMaxHealthStr) ;
    Text Health(450 , 20 , health_str) ;

    string score_m("Score ") ;
    string score_str(score_m + "0") ;
    Text Score(450 , BOTTOM_MARGIN , score_str) ;

    string levelNum = convert2Str(max_Level);
    string level_m("Level: ");
    string level_str(level_m + "1 / " + levelNum) ;
    Text Level(WINDOW_X/2 , BOTTOM_MARGIN , level_str) ;
    //

    // Intialize the shooter
    Shooter shooter(SHOOTER_START_X, SHOOTER_START_Y, SHOOTER_VX);

    // Initialize the bubbles
    vector<Bubble> bubbles = create_bubbles(curr_Level);

    // Initialize the bullets (empty)
    vector<Bullet> bullets;

    XEvent event;

    // Main game loop
    while (true)
    {
        bool pendingEvent = checkEvent(event);
        if (pendingEvent)
        {
            // Get the key pressed
            char c = charFromEvent(event);
            msg_cmd[msg_cmd.length() - 1] = c;
            charPressed.setMessage(msg_cmd);

            // Update the shooter
            if (!game_over){
                if(c == 'a')
                    shooter.move(STEP_TIME, true);
                else if(c == 'd')
                    shooter.move(STEP_TIME, false);
                else if(c == 'w')
                    bullets.push_back(shooter.shoot());
                else if(c == 'q')
                    return 0;

            }else if(c == 'q')
                    return 0;
        }

        // steps for time on timer display
        lvldets for_time = {curr_Level} ;
        if(!game_over) {
            time = (curr_Level)*STEP_TIME * counter ;
            timer_str = time_m + convert2Str(time) + " / " + convert2Str(for_time.get_time());
            Timer.setMessage(timer_str) ;
        }
        else {
            // after the game is over we dont want the timer to continue running so we give it a preset value
            timer_str = convert2Str(for_time.get_time()) + " / " + convert2Str(for_time.get_time());
            Timer.setMessage(timer_str) ;
        }

        counter++ ;

        // steps for health on health count display
        health_str = health_m + convert2Str(remainingHealth) + " / " + levelMaxHealthStr;
        Health.setMessage(health_str) ;

        // steps for total score display
        score_str = score_m + convert2Str(score);
        Score.setMessage(score_str) ;

        // steps to show level number display
        level_str = level_m + convert2Str(curr_Level) + " / " + levelNum;
        Level.setMessage(level_str) ;

        // game over bcs of time limit or total levels finished
        if (time > for_time.get_time() || curr_Level == 4){
            bubbles.clear();
            bullets.clear();
            game_over = true;
            gameOver.setMessage("Game Over!");
            wait(STEP_TIME);
            continue;
        }

        // Update the bubbles
        move_bubbles(bubbles);

        // Update the bullets
        move_bullets(bubbles, bullets, score, curr_Level);

        // this bool depicts if the bubble and the shooter are in contact so that the decreament of health can be right
        isBubbleContacted = intersectionSB(shooter, bubbles, isBubbleContacted, remainingHealth) ;

        // game over bcs outof health
        if (isBubbleContacted && remainingHealth == 0){
            bubbles.clear();
            bullets.clear();
            game_over = true;
            gameOver.setMessage("Game Over!");
        }

        // winner is displayed if game is completed by getting the maximum score of 39.
        if (score == 39) {
            game_over = true;
            gameOver.setMessage("Winner!");
        }

        // change of level and reset of time and health
        if (!level_over && bubbles.empty() && curr_Level < 4 && !game_over) {
            bubbles.clear();
            bullets.clear();
            level_over = true;
            curr_Level++;
            if (curr_Level < 4) {
                string nlvl = convert2Str(curr_Level);
                string level("level ");
                string nlevel_num(level + nlvl + "!");
                Text nlevelNum(WINDOW_X/2, WINDOW_Y/2, nlevel_num);
                wait(2) ;
                bubbles = create_bubbles(curr_Level);
                counter = 0 ;
                remainingHealth = 3;
            }
        }
        level_over = false;
        //wait(STEP_TIME/4);
    }
}
