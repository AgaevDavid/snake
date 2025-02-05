
#include <SFML/Graphics.hpp>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <cmath>
#include <fstream>
#include <algorithm> 

enum class GameState { Menu, Game, GameOver };

// Функция для генерации случайных координат внутри сетки
sf::Vector2f getRandomGridPosition(float gridSize, float gridWidth, float gridHeight, float gridOffsetX, float gridOffsetY) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distX(0, gridWidth - 1);
    std::uniform_int_distribution<> distY(0, gridHeight - 1);

    return sf::Vector2f(distX(gen) * gridSize + gridOffsetX, distY(gen) * gridSize + gridOffsetY);
}

int loadBestScore(const std::string& filename) {
    std::ifstream file(filename);
    int bestScore = 0;
    if (file.is_open()) {
        file >> bestScore;
        file.close();
    }
    return bestScore;
}

void saveBestScore(const std::string& filename, int bestScore) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << bestScore;
        file.close();
    }
} 

int main()
{
    bool gameOver = false; // Флаг, указывающий на окончание игры

    sf::RenderWindow window(sf::VideoMode(400, 400), "Snake!");

    sf::Vector2f snakeDirection = { 1.f, 0.f }; // Начальное направление: вправо

    sf::Color grayColor(128, 128, 128);

    std::vector<sf::RectangleShape> squares;
    for (int x = 40; x < window.getSize().x - 40; x += 10) 
    {
        for (int y = 100; y < window.getSize().y - 40; y += 10) 
        {
            sf::RectangleShape gridSquare(sf::Vector2f(10, 10));
            gridSquare.setFillColor(sf::Color::Black);
            gridSquare.setOutlineColor(grayColor);
            gridSquare.setOutlineThickness(1);
            gridSquare.setPosition(x, y);
            squares.push_back(gridSquare);
        }
    }

    int attachedCount = 0;

    float gridSize = 10.f; // Размер клетки сетки
    float snakeSpeed = gridSize; // Змея движется на одну клетку за шаг

    float dx = 0.f;   // Изменение координаты x (в клетках)
    float dy = 0.f;   // Изменение координаты y (в клетках)

    // Представление змейки
    std::vector<sf::RectangleShape> snake;
    snake.push_back(sf::RectangleShape(sf::Vector2f(gridSize, gridSize)));
    snake[0].setFillColor(sf::Color::Green);
    snake[0].setPosition(200, 200);

    sf::Vector2f headPos = snake[0].getPosition(); // Получаем позицию из ГОЛОВЫ змейки

    std::string bestScoreFilename = "best_score.txt"; //Имя файла для хранения лучшего счёта
    int bestScore = loadBestScore(bestScoreFilename); // Загрузка лучшего счёта из файла

    sf::Font font;
    if (!font.loadFromFile("resources/EpilepsySans.ttf")) {
        return EXIT_FAILURE;
    }
    // Элементы меню
    sf::Text menuTitle("Snake!", font, 48);
    menuTitle.setFillColor(sf::Color::White);
    menuTitle.setPosition(20, 200);

    sf::Text startButtonText("Start", font, 32);
    startButtonText.setPosition(20, 270);

    sf::Text exitButtonText("Exit", font, 32);
    exitButtonText.setPosition(20, 310);

    sf::Text highscoreText("", font, 24);
    highscoreText.setFillColor(sf::Color::White);
    highscoreText.setPosition(20, 50);

    sf::Text bestscoreText("", font, 30);
    bestscoreText.setFillColor(sf::Color::White);
    bestscoreText.setPosition(20, 20);

    bool isMoving = false; // Флаг, указывающий, движется ли змея
    sf::Clock clock;      // Часы для контроля скорости движения

    float gridWidth = (window.getSize().x - 80.f) / gridSize;
    float gridHeight = (window.getSize().y - 140.f) / gridSize;
    float gridOffsetX = 40.f;
    float gridOffsetY = 100.f;

    sf::RectangleShape yellowSquare(sf::Vector2f(gridSize, gridSize));
    yellowSquare.setFillColor(sf::Color::Yellow);
    sf::Vector2f yellowSquarePosition = getRandomGridPosition(gridSize, gridWidth, gridHeight, gridOffsetX, gridOffsetY);
    yellowSquare.setPosition(yellowSquarePosition);

    GameState gameState = GameState::Menu;
    sf::Event event;

    while (window.isOpen()) 
    {
        switch (gameState)
        {
            case GameState::Menu: 
            {
                window.clear();
                window.draw(menuTitle);
                window.draw(startButtonText);
                window.draw(exitButtonText);
                window.display();

                while (window.pollEvent(event)) {
                    if (event.type == sf::Event::Closed) window.close();
                    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                        if (startButtonText.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                            gameState = GameState::Game;
                        }
                        else if (exitButtonText.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                            window.close();
                        }
                    }
                }
                break;
            }
            case GameState::Game:
            {
                sf::Event event;
                while (window.pollEvent(event))
                {
                    window.clear();

                    if (event.type == sf::Event::KeyPressed)
                    {
                        if (event.key.code == sf::Keyboard::Left && snakeDirection.x != 1.f)
                        {
                            snakeDirection = { -1.f, 0.f };
                        }
                        else if (event.key.code == sf::Keyboard::Right && snakeDirection.x != -1.f)
                        {
                            snakeDirection = { 1.f, 0.f };
                        }
                        else if (event.key.code == sf::Keyboard::Up && snakeDirection.y != 1.f)
                        {
                            snakeDirection = { 0.f, -1.f };
                        }
                        else if (event.key.code == sf::Keyboard::Down && snakeDirection.y != -1.f)
                        {
                            snakeDirection = { 0.f, 1.f };
                        }
                    }
                }

                if (clock.getElapsedTime().asSeconds() >= 0.2f)
                {
                    sf::Vector2f headPos = snake[0].getPosition();
                    sf::Vector2f newHeadPos = { headPos.x + snakeDirection.x * gridSize, headPos.y + snakeDirection.y * gridSize };

                    // Обертывание (с учётом смещения сетки)
                    if (newHeadPos.x < gridOffsetX) newHeadPos.x += gridWidth * gridSize;
                    if (newHeadPos.x >= gridOffsetX + gridWidth * gridSize) newHeadPos.x -= gridWidth * gridSize;
                    if (newHeadPos.y < gridOffsetY) newHeadPos.y += gridHeight * gridSize;
                    if (newHeadPos.y >= gridOffsetY + gridHeight * gridSize) newHeadPos.y -= gridHeight * gridSize;

                    // Обнаружение самостолкновения
                    for (size_t i = 1; i < snake.size(); ++i)
                    {
                        if (snake[0].getGlobalBounds().intersects(snake[i].getGlobalBounds()))
                        {
                            gameOver = true;
                            break; // Выход из цикла после обнаружения столкновения
                        }
                    }

                    if (!gameOver)
                    { // Добавление сегмента только если нет столкновения
                        snake.insert(snake.begin(), sf::RectangleShape(sf::Vector2f(gridSize, gridSize)));
                        snake[0].setFillColor(sf::Color::Green);
                        snake[0].setPosition(newHeadPos);
                    }

                    // Проверка на столкновение
                    if (snake[0].getGlobalBounds().intersects(yellowSquare.getGlobalBounds()))
                    {
                        yellowSquarePosition = getRandomGridPosition(gridSize, gridWidth, gridHeight, gridOffsetX, gridOffsetY);
                        yellowSquare.setPosition(yellowSquarePosition);

                        attachedCount++;

                        bestScore = std::max(bestScore, attachedCount);
                    }
                    else
                    {
                        snake.pop_back();
                    }
                    clock.restart();

                    if (gameOver) {
                        gameState = GameState::GameOver;
                        clock.restart();
                    }
                    break;
                }
                
                highscoreText.setString("Highscore: " + std::to_string(attachedCount));
                bestscoreText.setString("Best Score: " + std::to_string(bestScore));

                window.clear();
                for (const auto& sq : squares) window.draw(sq);
                for (const auto& segment : snake) window.draw(segment); // Рисуем все сегменты змейки
                window.draw(yellowSquare);
                window.draw(highscoreText);
                window.draw(bestscoreText);
                window.display();
            }
            case GameState::GameOver: 
            {
                if (gameOver) 
                {
                    window.clear();
                    sf::Text gameOverText("Game Over", font, 36);
                    gameOverText.setFillColor(sf::Color::Red);
                    gameOverText.setPosition(120, 180);
                    window.draw(gameOverText);
                    window.display();
                    snake[0].setPosition(200, 200);

                    if (clock.getElapsedTime().asSeconds() >= 3.f) 
                    {
                        gameOver = false; // Сброс gameOver только после задержки
                        gameState = GameState::Menu;
                        attachedCount = 0;
                        clock.restart();

                        std::vector<sf::RectangleShape> newSnake;
                        if (!snake.empty()) 
                        {
                            newSnake.push_back(snake[0]);
                        }
                        snake = newSnake; // Заменяем старый вектор новым
                    }
                    break;
                }   
            }
        }
    }
    saveBestScore(bestScoreFilename, bestScore);
    return 0;
}