#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"
#include "ball_object.h"
#include "particle_generator.h"
#include "post_processor.h"
#include "text_renderer.h"

#include "sound_manager.h"

#include <iostream>

// Game-related State data
SpriteRenderer    *Renderer;
GameObject        *Player;
BallObject        *Ball;
ParticleGenerator *Particles;
PostProcessor     *Effects;
TextRenderer      *Text;
SoundManager sound;

float ShakeTime = 0.0f;
unsigned int FONT_SIZE = 24;

void ActivatePowerUp(PowerUp&);

// const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);
// const float PLAYER_VELOCITY(500.0f);
// GameObject *Player;

// const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
// const float BALL_RADIUS = 12.5f;
// BallObject *Ball;

Game::Game(unsigned int width, unsigned int height) 
    : State(GAME_MENU), Keys(), KeysProcessed(), Width(width), Height(height), Level(0), Lives(3){}


Game::~Game()
{
    delete Renderer;
    delete Player;
    delete Ball;
    delete Particles;
    delete Effects;
    delete Text;
}

void Game::Init(){
    //loading shaders
    ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.fs", nullptr, "sprite");
    ResourceManager::LoadShader("shaders/particle.vs", "shaders/particle.fs", nullptr, "particle");
    ResourceManager::LoadShader("shaders/post_processing.vs", "shaders/post_processing.fs", nullptr, "postprocessing");

    //shader config
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);

    ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("particle").SetMatrix4("projection", projection);

    Shader tmp = ResourceManager::GetShader("sprite");
    Renderer = new SpriteRenderer(tmp);

    //load textures
    ResourceManager::LoadTexture("textures/background.jpg", false, "background");
    ResourceManager::LoadTexture("textures/awesomeface.png", true, "face");
    ResourceManager::LoadTexture("textures/block.png", false, "block");
    ResourceManager::LoadTexture("textures/block_solid.png", false, "block_solid");
    ResourceManager::LoadTexture("textures/paddle.png", true, "paddle");
    ResourceManager::LoadTexture("textures/particle.png", true, "particle");
    ResourceManager::LoadTexture("textures/kanyewest.png", true, "kanye");    
    Texture2D tex_sticky = ResourceManager::LoadTexture("textures/powerup_sticky.png", true, "texsticky");
    Texture2D tex_speed = ResourceManager::LoadTexture("textures/powerup_speed.png", true, "texspeed");
    Texture2D tex_passthrough = ResourceManager::LoadTexture("textures/powerup_passthrough.png", true, "texpassthrough");
    Texture2D tex_increase = ResourceManager::LoadTexture("textures/powerup_increase.png", true, "texincrease");
    Texture2D tex_confuse = ResourceManager::LoadTexture("textures/powerup_confuse.png", true, "texconfuse");
    Texture2D tex_chaos = ResourceManager::LoadTexture("textures/powerup_chaos.png", true, "texchaos");

    //load levels
    GameLevel one; one.Load("levels/one.lvl", Width, Height/2);
    GameLevel two; two.Load("levels/two.lvl", Width, Height / 2);
    GameLevel three; three.Load("levels/three.lvl", Width, Height / 2);
    GameLevel four; four.Load("levels/four.lvl", Width, Height / 2);
    Levels.push_back(one);
    Levels.push_back(two);
    Levels.push_back(three);
    Levels.push_back(four);
    Level = 0;

    //load player
    glm::vec2 playerPos = glm::vec2(Width / 2.0f - PLAYER_SIZE.x / 2.0f, Height - PLAYER_SIZE.y);
    Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));

    //load ball
    glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x/2.0f - BALL_RADIUS, -BALL_RADIUS*2.0f);
    Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
    
    //Particle gen
    Particles = new ParticleGenerator (ResourceManager::GetShader("particle"),
        ResourceManager::GetTexture("particle"), 1500
    );

    Effects = new PostProcessor(ResourceManager::GetShader("postprocessing"), this->Width, this->Height);

    //load sounds
    sound.load("bleep1", "sounds/bleep.mp3");
    sound.load("bleep2", "sounds/bleep.wav");
    sound.load("background", "sounds/breakout.mp3");
    sound.load("powerup", "sounds/powerup.wav");
    sound.load("solid", "sounds/solid.wav");

    Text = new TextRenderer(Width, Height);
    Text->Load("fonts/ocraext.ttf", FONT_SIZE);

    sound.play("background", true);
    this->State = GAME_MENU;
    std::cout << "GAME INITIALISED\n";

}

void Game::Render()
{

    if(State == GAME_ACTIVE || State == GAME_MENU || State == GAME_PAUSE){
        
        Effects->BeginRender();

        Texture2D bg = ResourceManager::GetTexture("background");
        Renderer->DrawSprite(bg, glm::vec2(0.0f, 0.0f), glm::vec2(Width, Height), 0.0f);
        Levels[Level].Draw(*Renderer);
        Player->Draw(*Renderer);
        Particles->Draw();
        Ball->Draw(*Renderer);

        Effects->EndRender(); Effects->Render(glfwGetTime());
        for (PowerUp &p : PowerUps) if (!p.Destroyed) p.Draw(*Renderer);

        std::stringstream ss; ss << Lives;
        Text->RenderText("Lives:"+ss.str(), 5.0f, 5.0f, 1.0f);
    }
    if (this->State == GAME_MENU) {
        Text->RenderText("Press ENTER to start", 250.0f, this->Height / 2.0f, 1.0f);
        Text->RenderText("Press W or S to select level", 245.0f, this->Height / 2.0f + 20.0f, 0.75f);
        std::string levelText = "Selected Level: " + std::to_string(this->Level + 1);
        float textWidth = Text->GetTextWidth(levelText, 1.0f);
        Text->RenderText(levelText, 
            (Width/2.0f) - (textWidth/2.0f), this->Height / 2.0f - 50.0f, 1.0f, 
            glm::vec3(0.0f, 1.0f, 0.0f));
    }
    if (State == GAME_WIN) {
        Text->RenderText("You WON!!!", 320.0, Height / 2 - 20.0,
            1.0, glm::vec3(0.0, 1.0, 0.0));
        Text->RenderText("Press ENTER to retry or ESC to quit", 130.0,
            Height / 2, 1.0, glm::vec3(1.0, 1.0, 0.0));
    }
    if (State == GAME_PAUSE){
        std::string txt = "PAUSED. Press P to resume";
        float scale = 1.0f;
        float textWidth = Text->GetTextWidth(txt, scale);
        float xPos = (this->Width/2.0f) - (textWidth/2.0f); 
        float yPos = (this->Height / 2.0f) - ((float)FONT_SIZE* scale / 2.0f);

        Text->RenderText(txt, xPos, yPos, scale);
    }

}

void Game::ProcessInput(float dt){

    if (State == GAME_ACTIVE){
        float velocity = PLAYER_VELOCITY * dt;
        if (Keys[GLFW_KEY_A]){
            if (Player->Position.x >= 0.0f) {
                Player->Position.x -= velocity;
                if (Ball->Stuck) Ball->Position.x -= velocity;
            }
            
        }
        if (Keys[GLFW_KEY_D]){
            if (Player->Position.x <= Width-Player->Size.x) {
                Player->Position.x += velocity;
                if (Ball->Stuck) Ball->Position.x += velocity;
            }
        }
        if (Keys[GLFW_KEY_SPACE]) Ball->Stuck = false;

        if (Keys[GLFW_KEY_P] && !KeysProcessed[GLFW_KEY_P]) {
            State = GAME_PAUSE; KeysProcessed[GLFW_KEY_P] = true;
        }
    }

    if (State == GAME_MENU){
        if (Keys[GLFW_KEY_ENTER] && !KeysProcessed[GLFW_KEY_ENTER]){
            State = GAME_ACTIVE; KeysProcessed[GLFW_KEY_ENTER] = true;}
        if (Keys[GLFW_KEY_W] && !KeysProcessed[GLFW_KEY_W]){
            Level = (Level + 1) % 4; 
            ResetLevel(); ResetPlayer();
            KeysProcessed[GLFW_KEY_W] = true;
        }
        if (Keys[GLFW_KEY_S] && !KeysProcessed[GLFW_KEY_S]) {
            Level = (Level +3)%4;
            ResetLevel(); ResetPlayer();
            KeysProcessed[GLFW_KEY_S] = true;
        }

        if (Keys[GLFW_KEY_Q] && !KeysProcessed[GLFW_KEY_Q]){
            Texture2D texKanye = ResourceManager::GetTexture("kanye");
            Texture2D texFace = ResourceManager::GetTexture("face");
            if (Ball->Sprite.ID == texFace.ID) Ball->Sprite = texKanye;
            else Ball->Sprite = texFace;
        }
    }

    if (State == GAME_WIN){
        if (Keys[GLFW_KEY_ENTER]){
            KeysProcessed[GLFW_KEY_ENTER] = true;
            Effects->Chaos = false; State = GAME_MENU;
        }
    }

    if (State == GAME_PAUSE){
        if (Keys[GLFW_KEY_P] &&!KeysProcessed[GLFW_KEY_P]){
            KeysProcessed[GLFW_KEY_P] = true;
            State = GAME_ACTIVE;
        }
    }

    CheatTimer += dt;
    if (CheatTimer > 0.8f) CheatBuffer = "";

    for (int k = GLFW_KEY_A; k<= GLFW_KEY_Z; k++){
        if (Keys[k] && !KeysProcessed[k]){
            CheatBuffer += (char)(k-GLFW_KEY_A) + 'a';
            CheatTimer = 0.0f; KeysProcessed[k] = true;
        }
    }
    for (int k = GLFW_KEY_0; k <= GLFW_KEY_9; k++) {
        if (Keys[k] && !KeysProcessed[k]) {
            CheatBuffer += (char)(k - GLFW_KEY_0 + '0');
            CheatTimer = 0.0f;
            KeysProcessed[k] = true;
        }
    }

    if (CheatBuffer.find("livelong") != std::string::npos) {
        this->Lives = 999;
        sound.play("powerup"); // Audio feedback
        CheatBuffer = "";
    }
    
    if (CheatBuffer.find("bigbigbig") != std::string::npos) {
        Player->Size.x = (float)this->Width;
        Player->Position.x = 0.0f;
        sound.play("powerup");
        CheatBuffer = "";
    }

}

void Game::Update(float dt){

    if (State == GAME_ACTIVE){
        Ball->Move(dt, Width);
        DoCollisions();
        this->UpdatePowerUps(dt);
    
        if (State == GAME_ACTIVE && Levels[Level].IsCompleted()){
            ResetLevel(); ResetPlayer(); Effects->Chaos = true; State = GAME_WIN;
        }

        if (Ball->Position.y >= Height){
            if (--Lives == 0){ResetLevel(); State = GAME_MENU;}
            ResetPlayer();
        }

        Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius/2.0f));
        if (ShakeTime>0.0f){
            ShakeTime-=dt; if (ShakeTime<=0.0f) Effects->Shake = false;
        }
    }

}

Direction VectorDirection(glm::vec2 target){
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f), // up
        glm::vec2(1.0f, 0.0f), // right
        glm::vec2(0.0f, -1.0f), // down
        glm::vec2(-1.0f, 0.0f) // left
    };
    float max = 0.0f;
    unsigned int best_match = -1;
    for (unsigned int i = 0; i<4; i++){
        float dot_product = glm::dot(glm::normalize(target), compass[i]);
        if (dot_product>max){ max = dot_product; best_match = i;}
    }

    return (Direction)best_match;
}

// AABB - Circle collision logic
Collision CheckCollision(BallObject &one, GameObject &two) {
    // get center point circle first
    glm::vec2 center(one.Position + one.Radius);
    // calculate AABB info (center, half-extents)
    glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
    glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x,
    two.Position.y + aabb_half_extents.y);
    // get difference vector between both centers
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents,
    aabb_half_extents);
    // add clamped value to AABB_center and get the value closest to circle
    glm::vec2 closest = aabb_center + clamped;
    // vector between center circle and closest point AABB
    difference = closest - center;
    
    if (glm::length(difference) <= one.Radius)
        return std::make_tuple(true, VectorDirection(difference), difference);
    else
        return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}

bool CheckCollision(GameObject &one, GameObject &two)
{
    // Collision x-axis?
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
        two.Position.x + two.Size.x >= one.Position.x;
    // Collision y-axis?
    bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
        two.Position.y + two.Size.y >= one.Position.y;              
    // Collision only if on both axes
    return collisionX && collisionY;
}

void Game::DoCollisions()
{
    for (GameObject &box : this->Levels[this->Level].Bricks)
    {
        if (!box.Destroyed)
        {
            Collision collision = CheckCollision(*Ball, box);
            if (std::get<0>(collision))
            {
                if (!box.IsSolid)
                {
                    box.Destroyed = true; this->SpawnPowerUps(box);
                    sound.play("bleep1");
                }
                else
                {   // if block is solid, enable shake effect
                    ShakeTime = 0.05f; Effects->Shake = true;
                    sound.play("solid");
                }
                // collision resolution
                Direction dir = std::get<1>(collision);
                glm::vec2 diff_vector = std::get<2>(collision);
                if (!(Ball->PassThrough && !box.IsSolid))
                {
                    if (dir == LEFT || dir == RIGHT) // horizontal collision
                    {
                        Ball->Velocity.x = -Ball->Velocity.x; // reverse horizontal velocity
                        // relocate
                        float penetration = Ball->Radius - std::abs(diff_vector.x);
                        if (dir == LEFT)
                            Ball->Position.x += penetration; // move ball to right
                        else
                            Ball->Position.x -= penetration; // move ball to left;
                    }
                    else // vertical collision
                    {
                        Ball->Velocity.y = -Ball->Velocity.y; // reverse vertical velocity
                        // relocate
                        float penetration = Ball->Radius - std::abs(diff_vector.y);
                        if (dir == UP)
                            Ball->Position.y -= penetration; // move ball bback up
                        else
                            Ball->Position.y += penetration; // move ball back down
                    }
                }
            }
        }    
    }

    // also check collisions on PowerUps and if so, activate them
    for (PowerUp &powerUp : this->PowerUps)
    {
        if (!powerUp.Destroyed)
        {
            // first check if powerup passed bottom edge, if so: keep as inactive and destroy
            if (powerUp.Position.y >= this->Height)
                powerUp.Destroyed = true;

            if (CheckCollision(*Player, powerUp))
            {	// collided with player, now activate powerup
                sound.play("powerup");
                ActivatePowerUp(powerUp);
                powerUp.Destroyed = true;
                powerUp.Activated = true;
            }
        }
    }

    // and finally check collisions for player pad (unless stuck)
    Collision result = CheckCollision(*Ball, *Player);
    if (!Ball->Stuck && std::get<0>(result)) {
        sound.play("bleep2");
        // check where it hit the board, and change velocity based on where it hit the board
        float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
        float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
        float percentage = distance / (Player->Size.x / 2.0f);
        // then move accordingly
        float strength = 2.0f;
        glm::vec2 oldVelocity = Ball->Velocity;
        Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength; 
        //Ball->Velocity.y = -Ball->Velocity.y;
        Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity); // keep speed consistent over both axes (multiply by length of old velocity, so total strength is not changed)
        // fix sticky paddle
        Ball->Velocity.y = -1.0f * abs(Ball->Velocity.y);

        // if Sticky powerup is activated, also stick ball to paddle once new velocity vectors were calculated
        Ball->Stuck = Ball->Sticky;
    }
}


void Game::ResetLevel()
{
    if (this->Level == 0)
        this->Levels[0].Load("levels/one.lvl", this->Width, this->Height / 2);
    else if (this->Level == 1)
        this->Levels[1].Load("levels/two.lvl", this->Width, this->Height / 2);
    else if (this->Level == 2)
        this->Levels[2].Load("levels/three.lvl", this->Width, this->Height / 2);
    else if (this->Level == 3)
        this->Levels[3].Load("levels/four.lvl", this->Width, this->Height / 2);

    this->Lives = 3;
}

void Game::ResetPlayer()
{
    // Reset paddle state
    Player->Size = PLAYER_SIZE;
    Player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
    // Reset ball state
    Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
}

bool ShouldSpawn(unsigned int chance){
    unsigned int random = rand() % chance;
    return random == 0;
}

const int freq = 20;

void Game::SpawnPowerUps(GameObject &block) {
    Texture2D tex_speed = ResourceManager::GetTexture("texspeed");
    Texture2D tex_sticky = ResourceManager::GetTexture("texsticky");
    Texture2D tex_passthrough = ResourceManager::GetTexture("texpassthrough");
    Texture2D tex_increase = ResourceManager::GetTexture("texincrease");
    Texture2D tex_confuse = ResourceManager::GetTexture("texconfuse");
    Texture2D tex_chaos = ResourceManager::GetTexture("texchaos");

    if (ShouldSpawn(freq)) // 1 in 75 chance
        PowerUps.push_back(PowerUp("speed", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, tex_speed));
    if (ShouldSpawn(freq))
        PowerUps.push_back(PowerUp("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.Position, tex_sticky));
    if (ShouldSpawn(freq))
        PowerUps.push_back(PowerUp("pass-through", glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, block.Position, tex_passthrough));
    if (ShouldSpawn(freq))
        PowerUps.push_back(PowerUp("pad-size-increase", glm::vec3(1.0f, 0.6f, 0.4), 0.0f, block.Position, tex_increase));
    if (ShouldSpawn(freq/2)) // negative powerups should spawn more often
        PowerUps.push_back(PowerUp("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, block.Position, tex_confuse));
    if (ShouldSpawn(freq/2))
        PowerUps.push_back(PowerUp("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, block.Position, tex_chaos));
}


void ActivatePowerUp(PowerUp &powerUp){
    if (powerUp.Type == "speed") {
        Ball->Velocity *= 1.2;
    }
    else if (powerUp.Type == "sticky") {
        Ball->Sticky = true;
        Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
    }
    else if (powerUp.Type == "pass-through") {
        Ball->PassThrough = true;
        Ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
    }
    else if (powerUp.Type == "pad-size-increase") {
        Player->Size.x += 50;   
    }
    else if (powerUp.Type == "confuse") {
        if (!Effects->Chaos) Effects->Confuse = true;
    }
    else if (powerUp.Type == "chaos") {
        if (!Effects->Confuse) Effects->Chaos = true;
    }
}

bool IsOtherPowerUpActive(std::vector<PowerUp> &powerUps, std::string type) {
    for (const PowerUp &powerUp : powerUps) {
        if (powerUp.Activated && powerUp.Type == type) return true;
    }
    return false;
}

void Game::UpdatePowerUps(float dt) {
    for (PowerUp &powerUp : this->PowerUps) {
        powerUp.Position += powerUp.Velocity * dt;
        if (powerUp.Activated) {
            powerUp.Duration -= dt;

            if (powerUp.Duration <= 0.0f) {
                // remove powerup from list (will later be removed)
                powerUp.Activated = false;
                // deactivate effects
                if (powerUp.Type == "sticky") {
                    if (!IsOtherPowerUpActive(this->PowerUps, "sticky"))
                    {	// only reset if no other PowerUp of type sticky is active
                        Ball->Sticky = false;
                        Player->Color = glm::vec3(1.0f);
                    }
                }
                else if (powerUp.Type == "pass-through") {
                    if (!IsOtherPowerUpActive(this->PowerUps, "pass-through")) {
                        // only reset if no other PowerUp of type pass-through is active
                        Ball->PassThrough = false;
                        Ball->Color = glm::vec3(1.0f);
                    }
                }
                else if (powerUp.Type == "confuse") {
                    if (!IsOtherPowerUpActive(this->PowerUps, "confuse")){
                        // only reset if no other PowerUp of type confuse is active
                        Effects->Confuse = false;
                    }
                }
                else if (powerUp.Type == "chaos") {
                    if (!IsOtherPowerUpActive(this->PowerUps, "chaos"))
                    {	// only reset if no other PowerUp of type chaos is active
                        Effects->Chaos = false;
                    }
                }
            }
        }
    }
    // Remove all PowerUps from vector that are destroyed AND !activated (thus either off the map or finished)
    // Note we use a lambda expression to remove each PowerUp which is destroyed and not activated
    this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(),
        [](const PowerUp &powerUp) { return powerUp.Destroyed && !powerUp.Activated; }
    ), this->PowerUps.end());
}
