#include "Game.h"
#include <GameObject.h>
#include <SDL.h>
#include <math.h>
#include <SDL_image.h>
#include <SDL_opengl.h>
#include <InputManager.h>
#include <Graphics/GraphicsOpenGL.h>
#include <stdlib.h>     
#include <time.h>
#include "Cube.h"
#include <Cameras/Camera.h>
#include <Cameras/PerspectiveCamera.h>
#include <Cameras/OrthographicCamera.h>

#include "Player.h"
#include "Fruit.h"

// Initializing our static member pointer.
GameEngine* GameEngine::_instance = nullptr;

GameEngine* GameEngine::CreateInstance()
{
  if (_instance == nullptr)
  {
    _instance = new Game();
  }
  return _instance;
}

Game::Game() : GameEngine()
{

}

Game::~Game()
{
  // Clean up our pointers.
	
	delete _gameCamera; // Delete game camera.
	_gameCamera = nullptr;

	_player->~Player(); // Deleting body nodes
		_player = nullptr; 

	 delete _fruit; // Detete fruit
	_fruit = nullptr;

	//_score[1000] = 0;
}

SDL_Renderer *_renderer;
SDL_Texture *_texture;

void Game::InitializeImpl()
{
	/* initialize random seed for fruit: */
	srand(time(NULL));
	currentScore = 0;
	sprintf_s(_score, "Snake___Player Score %d",currentScore);
	SDL_SetWindowTitle(_window, _score);

  float nearPlane = 0.01f;
  float farPlane = 100.0f;

  Vector4 position(0.0f, 0.0f, 2.5f, 0.0f);
  Vector4 lookAt = Vector4::Normalize(Vector4::Difference(Vector4(0.0f, 0.0f, 0.0f, 0.0f), position));
  Vector4 up(0.0f, 1.0f, 0.0f, 0.0f);

  _gameCamera = new OrthographicCamera(-10.0f, 10.0f, 10.0f, -10.0f, nearPlane, farPlane, position, lookAt, up);

  // Create the player.
  _player = new Player();
  _objects.push_back(_player);

  // Create the fruit.
  _fruit = new Fruit;
  _objects.push_back(_fruit);
  _fruit->GetTransform().position.y -= 2.0f;

  currentScore = 0.0f;

  for (auto itr = _objects.begin(); itr != _objects.end(); itr++)
  {
    (*itr)->Initialize(_graphicsObject);
  }
}

void Game::UpdateImpl(float dt)
{
  InputManager::GetInstance()->Update(dt);

  // Check controls.
  if (InputManager::GetInstance()->IsKeyDown(SDLK_UP) == true)
  {
    _player->SetHeadDirection(BodyNode::UP);
  }
  else if (InputManager::GetInstance()->IsKeyDown(SDLK_DOWN) == true)
  {
    _player->SetHeadDirection(BodyNode::DOWN);
  }
  else if (InputManager::GetInstance()->IsKeyDown(SDLK_LEFT) == true)
  {
    _player->SetHeadDirection(BodyNode::LEFT);
  }
  else if (InputManager::GetInstance()->IsKeyDown(SDLK_RIGHT) == true)
  {
    _player->SetHeadDirection(BodyNode::RIGHT);
  }


  if ((_player->GetHeadPosition().x >= _fruit->GetTransform().position.x + 0.75) && (_player->GetHeadPosition().y >= _fruit->GetTransform().position.y - 0.75))
  {
	  _player->AddBodyPiece(_graphicsObject);	  
	  Vector4 position(rand() % 18 + (-8), rand() % 18 + (-8), rand() % 18 + (-8), rand() % 18 + (-8));
	  _fruit->SetFruitPosition(position);
	  currentScore += 10;
	  sprintf_s(_score, "Snake___Player Score %d", currentScore);
	  SDL_SetWindowTitle(_window, _score);
  }
 // printf("Player-X:%f,Y:%f\n", _player->GetHeadPosition().x, _player->GetHeadPosition().y);
 // printf("Fruit-X:%f,Y:%f\n", _fruit->GetTransform().position.x, _fruit->GetTransform().position.y);

  // Do bounds checking.
  if (_player->GetHeadPosition().x <= -8.75 )//left wall
  {
	  Reset();
  }
  if (_player->GetHeadPosition().y <= -8.75)//bottom wall
  {
	  Reset();
  }
  if (_player->GetHeadPosition().x >= 8.75)//right wall
  {
	  Reset();
  }
  if (_player->GetHeadPosition().y >= 8.75)//top wall
  {
	  Reset();
  }
  for (auto itr = _objects.begin(); itr != _objects.end(); itr++)
  {
	  (*itr)->Update(dt);
  }

}

void Game::DrawImpl(Graphics *graphics, float dt)
{
  std::vector<GameObject *> renderOrder = _objects;

  // Draw scenery on top.
  glPushMatrix();
  {
    glClear(GL_DEPTH_BUFFER_BIT);
    CalculateCameraViewpoint(_gameCamera);

    for (auto itr = renderOrder.begin(); itr != renderOrder.end(); itr++)
    {
      (*itr)->Draw(graphics, _gameCamera->GetProjectionMatrix(), dt);
    }
  }
  glPopMatrix();
}

void Game::CalculateCameraViewpoint(Camera *camera)
{
  camera->Apply();

  Vector4 xAxis(1.0f, 0.0f, 0.0f, 0.0f);
  Vector4 yAxis(0.0f, 1.0f, 0.0f, 0.0f);
  Vector4 zAxis(0.0f, 0.0f, 1.0f, 0.0f);

  Vector3 cameraVector(camera->GetLookAtVector().x, camera->GetLookAtVector().y, camera->GetLookAtVector().z);
  Vector3 lookAtVector(0.0f, 0.0f, -1.0f);

  Vector3 cross = Vector3::Normalize(Vector3::Cross(cameraVector, lookAtVector));
  float dot = MathUtils::ToDegrees(Vector3::Dot(lookAtVector, cameraVector));

  glRotatef(cross.x * dot, 1.0f, 0.0f, 0.0f);
  glRotatef(cross.y * dot, 0.0f, 1.0f, 0.0f);
  glRotatef(cross.z * dot, 0.0f, 0.0f, 1.0f);

  glTranslatef(-camera->GetPosition().x, -camera->GetPosition().y, -camera->GetPosition().z);
}

void Game::Reset()
{

	printf("Game over\n");
	
	Vector4 _respawn(0.0f, 0.0f, 2.5f, 0.0f);
	_player->SetHeadPosition(_respawn);
	currentScore = 0;
}