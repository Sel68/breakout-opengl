#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"
#include "ball_object.h"
#include "particle_generator.h"
#include "post_processor.h"
#include "text_renderer.h"

#include <iostream>

// Game-related State data
SpriteRenderer    *Renderer;
GameObject        *Player;
BallObject        *Ball;
ParticleGenerator *Particles;
PostProcessor     *Effects;
TextRenderer      *Text;

float ShakeTime = 0.0f;

// const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);
// const float PLAYER_VELOCITY(500.0f);
// GameObject *Player;

// const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
// const float BALL_RADIUS = 12.5f;
// BallObject *Ball;

Game::Game(unsigned int width, unsigned int height) 
    : State(GAME_MENU), Keys(), KeysProcessed(), Width(width), Height(height), Level(0), Lives(3)
{ 

}

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

    //shader config
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);

    Shader tmp = ResourceManager::GetShader("sprite");
    Renderer = new SpriteRenderer(tmp);

    //load textures
    ResourceManager::LoadTexture("textures/background.jpg", false, "background");
    ResourceManager::LoadTexture("textures/awesomeface.png", true, "face");
    ResourceManager::LoadTexture("textures/block.png", false, "block");
    ResourceManager::LoadTexture("textures/block_solid.png", false, "block_solid");
    ResourceManager::LoadTexture("textures/paddle.png", true, "paddle");

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
    std::cout << "GAME INITIALISED\n";
    this->State = GAME_ACTIVE;
}

void Game::Render()
{
    if(State == GAME_ACTIVE){
        Texture2D bg = ResourceManager::GetTexture("background");
        Renderer->DrawSprite(bg, glm::vec2(0.0f, 0.0f), glm::vec2(Width, Height), 0.0f);
        
        Levels[Level].Draw(*Renderer);
        Player->Draw(*Renderer);
        Ball->Draw(*Renderer);
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
    }
}

void Game::Update(float dt){
    Ball->Move(dt, Width);
    DoCollisions();
    if (Ball->Position.y >= Height){
        ResetLevel(); ResetPlayer();
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


void Game::DoCollisions(){
    for (GameObject &box:Levels[Level].Bricks){
        if (!box.Destroyed){
            Collision collision = CheckCollision(*Ball, box);
            if (std::get<0>(collision)){
                if (!box.IsSolid) box.Destroyed = true;

                Direction dir = std::get<1>(collision);
                glm::vec2 diff_vector = std::get<2>(collision);
                if (dir == LEFT || dir == RIGHT){
                    Ball->Velocity.x *=-1;
                    float penetration = Ball->Radius - std::abs(diff_vector.x);
                    if (dir == LEFT) Ball->Position.x += penetration;
                    else Ball->Position.x -= penetration;
                }
                else {
                    Ball->Velocity.y *= -1;
                    float penetration = Ball->Radius - std::abs(diff_vector.y);
                    if (dir==UP) Ball->Position.y -= penetration;
                    else Ball->Position.y += penetration;
                }
            }
        }
    }

    Collision result = CheckCollision(*Ball, *Player);
    if (!Ball->Stuck && std::get<0>(result)){
        float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
        float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
        float percentage = distance / (Player->Size.x / 2.0f);
        // then move accordingly
        float strength = 2.0f;
        glm::vec2 oldVelocity = Ball->Velocity;
        Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
        Ball->Velocity.y = -1* abs(Ball->Velocity.y);
        Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
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
