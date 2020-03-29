
//
// Disclaimer:
// ----------
//
// This code will work only if you selected window, graphics and audio.
//
// In order to load the resources like cute_image.png, you have to set up
// your target scheme:
//
// - Select "Edit Scheme…" in the "Product" menu;
// - Check the box "use custom working directory";
// - Fill the text field with the folder path containing your resources;
//        (e.g. your project folder)
// - Click OK.
//

#include <iostream>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

class Window {
private:
   sf::RenderWindow m_window;
   sf::Vector2u m_windowSize;
   std::string m_windowTitle;
   bool m_isDone;
   bool m_isFullscreen;
   void setup(const std::string &title, const sf::Vector2u &size);
   void destroy() {m_window.close();}
   void create();

public:
   Window();
   Window(const std::string &title, const sf::Vector2u &size) {setup(title, size);}
   ~Window() {destroy();}
   void beginDraw() {m_window.clear(sf::Color::Black);}
   void endDraw() {m_window.display();}
   void Update();
   bool getIsDone() {return m_isDone;}
   bool getIsFullscreen() {return m_isFullscreen;}
   sf::Vector2u getWindowSize() {return m_windowSize;}
    sf::RenderWindow* getWindow() {return &m_window;}
   void toggleFullscreen();
   void draw(sf::Drawable &drawable) {m_window.draw(drawable);}
};

void Window::setup(const std::string &title, const sf::Vector2u &size) {
   m_windowTitle = title;
   m_windowSize = size;
   m_isFullscreen = false;
   m_isDone = false;
   create();
}

void Window::create() {
   auto style = (m_isFullscreen ? sf::Style::Fullscreen : sf::Style::Default);
   m_window.create({m_windowSize.x, m_windowSize.y, 32}, m_windowTitle, style);
}

void Window::Update() {
   sf::Event e;
   while (m_window.pollEvent(e)) {
      if (e.type == sf::Event::Closed) m_isDone = true;
      else if (e.type == sf::Event::KeyPressed &&
               e.key.code == sf::Keyboard::F5) toggleFullscreen();
   }
}

void Window::toggleFullscreen() {
   m_isFullscreen = !m_isFullscreen;
   destroy();
   create();
}

struct SnakeSegment {
   sf::Vector2i position;
   SnakeSegment(int x, int y) : position(x, y){}
};

using SnakeContainer = std::vector<SnakeSegment>;
enum class Direction{None, Up, Down, Left, Right};

class Snake {
private:
   SnakeContainer snakeBody;
   int size;
   Direction dir;
   int speed;
   int lives;
   int score;
   bool lost;
   sf::RectangleShape body;
   void collision();

public:
   Snake(int blockSize);
   ~Snake(){}
   void setDirection(Direction dir) {this->dir = dir;}
   Direction getDirection() {return dir;}
   int getSpeed() {return speed;}
   sf::Vector2i getPos();
   int getLives() {return lives;}
   int getScore() {return score;}
   void increaseScore() {score += 10;}
   bool hasLost() {return lost;}
   void lose() {lost = true;}
   void toggleLost() {lost = !lost;}
   void extend();
   void reset();
   void move();
   void tick();
   void cut(int segments);
   void render(sf::RenderWindow &window);
};

void Snake::collision() {
   if (snakeBody.size() < 5) return;
   SnakeSegment &head = snakeBody.front();
   for (auto itr = snakeBody.begin() + 1; itr != snakeBody.end(); itr++) {
      if (itr->position == head.position) {
         int segments = snakeBody.end() - itr;
         cut(segments);
         break;
      }
   }
}

Snake::Snake(int blockSize) {
   size = blockSize;
   body.setSize(sf::Vector2f(size - 1, size - 1));
   reset();
}

void Snake::extend() {
   if (snakeBody.empty()) return;
   SnakeSegment &tail_head = snakeBody[snakeBody.size() - 1];
   if (snakeBody.size() > 1) {
      SnakeSegment &tail_bone = snakeBody[snakeBody.size() - 2];
      if (tail_head.position.x == tail_bone.position.x) {
         if (tail_head.position.y > tail_bone.position.y)
            snakeBody.push_back(SnakeSegment(tail_head.position.x,
                                             tail_head.position.y + 1));
         else snakeBody.push_back(SnakeSegment(tail_head.position.x,
                                            tail_head.position.y - 1));
      } else if (tail_head.position.y == tail_bone.position.y) {
         if (tail_head.position.x > tail_bone.position.x)
            snakeBody.push_back(SnakeSegment(tail_head.position.x + 1,
                                             tail_head.position.y));
         else snakeBody.push_back(SnakeSegment(tail_head.position.x - 1,
                                               tail_head.position.y));
      }
   } else {
      if (dir == Direction::Up) snakeBody.push_back
         (SnakeSegment(tail_head.position.x, tail_head.position.y + 1));
      else if (dir == Direction::Down) snakeBody.push_back
         (SnakeSegment(tail_head.position.x, tail_head.position.y - 1));
      else if (dir == Direction::Left) snakeBody.push_back
         (SnakeSegment(tail_head.position.x + 1, tail_head.position.y));
      else if (dir == Direction::Right) snakeBody.push_back
         (SnakeSegment(tail_head.position.x - 1, tail_head.position.y));
   }
}

void Snake::reset() {
   snakeBody.clear();
   snakeBody.push_back(SnakeSegment(5, 7));
   snakeBody.push_back(SnakeSegment(5, 6));
   snakeBody.push_back(SnakeSegment(5, 5));
   setDirection(Direction::None);
   speed = 15;
   lives = 3;
   score = 0;
   lost = false;
}

void Snake::move() {
   for (int i = snakeBody.size() - 1; i > 0; --i)
      snakeBody[i].position = snakeBody[i - 1].position;
   if (dir == Direction::Left) --snakeBody[0].position.x;
   else if (dir == Direction::Right) ++snakeBody[0].position.x;
   else if (dir == Direction::Up) --snakeBody[0].position.y;
   else if (dir == Direction::Down) ++snakeBody[0].position.y;
}

void Snake::tick() {
   if (snakeBody.empty()) return;
   if (dir == Direction::None) return;
   move();
   collision();
}

void Snake::cut(int segments) {
   for (int i = 0; i < segments; ++i) snakeBody.pop_back();
   --lives;
   if (!lives) {lose(); return;}
}

void Snake::render(sf::RenderWindow &window) {
   if (snakeBody.empty()) return;
   auto head = snakeBody.begin();
   body.setFillColor(sf::Color::Yellow);
   body.setPosition(head->position.x * size, head->position.y * size);
   window.draw(body);

   body.setFillColor(sf::Color::Green);
   for (auto itr = snakeBody.begin() + 1; itr != snakeBody.end(); ++itr) {
      body.setPosition(itr->position.x * size, itr->position.y * size);
      window.draw(body);
   }
}

sf::Vector2i Snake::getPos() {
   return (!snakeBody.empty() ? snakeBody.front().position : sf::Vector2i(1, 1));
}

class Board {
private:
   sf::Vector2u windowSize;
   sf::Vector2i item;
   int blockSize;
   sf::CircleShape apple;
   sf::RectangleShape bounds[4];

public:
   Board(sf::Vector2u winSize);
   ~Board(){}
   int getBlockSize() {return blockSize;}
   void respawnApple();
   void Update(Snake &player);
   void render(sf::RenderWindow &window);
};

Board::Board(sf::Vector2u winSize) {
   blockSize = 16;
   windowSize = winSize;
   respawnApple();
   apple.setFillColor(sf::Color::Red);
   apple.setRadius(blockSize / 2);

   for (int i = 0; i < 4; ++i) {
      bounds[i].setFillColor(sf::Color(150, 0, 0));
      if (!((i+1) % 2)) bounds[i].setSize(sf::Vector2f(windowSize.x, blockSize));
      else bounds[i].setSize(sf::Vector2f(blockSize, windowSize.y));
      if (i < 2) bounds[i].setPosition(0, 0);
      else {
         bounds[i].setOrigin(bounds[i].getSize());
         bounds[i].setPosition(sf::Vector2f(windowSize));
      }
   }
}

void Board::respawnApple() {
   int maxX = (windowSize.x / blockSize) - 2;
   int maxY = (windowSize.y / blockSize) - 2;
   item = sf::Vector2i(rand() % maxX + 1, rand() % maxY + 1);
   apple.setPosition(item.x * blockSize, item.y * blockSize);
}

void Board::Update(Snake &player) {
   if (player.getPos() == item) {
      player.extend();
      player.increaseScore();
      respawnApple();
   }
   int gridSizeX = windowSize.x / blockSize;
   int gridSizeY = windowSize.y / blockSize;

   if (player.getPos().x <= 0 ||
       player.getPos().y <= 0 ||
       player.getPos().x >= gridSizeX - 1 ||
       player.getPos().y >= gridSizeY - 1) player.lose();
}

void Board::render(sf::RenderWindow &window) {
   for (int i = 0; i < 4; ++i) window.draw(bounds[i]);
   window.draw(apple);
}

class Game {
private:
   Window m_window;
   sf::Clock m_clock;
   float m_elapsed;
   Board board;
   Snake snake;

public:
   Game();
   ~Game(){}
   void handleInput();
   void Update();
   void render();
   Window* getWindow() {return &m_window;}
   sf::Time getElapsed() {return m_clock.getElapsedTime();}
   void restartClock() {m_elapsed += m_clock.restart().asSeconds();}
};

Game::Game() : m_window("Snake", sf::Vector2u(1200, 800)),
snake(board.getBlockSize()), board(sf::Vector2u(1200, 800)){
   m_clock.restart();
   srand(time(nullptr));
   m_elapsed = 0.0f;
}

void Game::handleInput() {
   if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) &&
      snake.getDirection() != Direction::Down)
      snake.setDirection(Direction::Up);
   else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) &&
   snake.getDirection() != Direction::Up)
   snake.setDirection(Direction::Down);
   else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) &&
   snake.getDirection() != Direction::Right)
   snake.setDirection(Direction::Left);
   else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) &&
   snake.getDirection() != Direction::Left)
   snake.setDirection(Direction::Right);
}

void Game::Update() {
   m_window.Update();
   float timestep = 1.0f / snake.getSpeed();
   if (m_elapsed >= timestep) {
      snake.tick();
      board.Update(snake);
      m_elapsed -= timestep;
      if (snake.hasLost()) snake.reset();
   }
}

void Game::render() {
   m_window.beginDraw();
   board.render(*m_window.getWindow());
   snake.render(*m_window.getWindow());
   m_window.endDraw();
}
int main(int argc, char const** argv) {
   Game game;
   while(!game.getWindow()->getIsDone()) {
      game.handleInput();
      game.Update();
      game.render();
      game.restartClock();
   }
   return 0;
}
