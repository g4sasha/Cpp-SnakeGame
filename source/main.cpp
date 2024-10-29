#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>

const int SIZE = 20;
const int WIDTH = 800 / SIZE;
const int HEIGHT = 600 / SIZE;
const int APPLE_COUNT = 5;

enum Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

class Apple
{
public:
    Apple()
    {
        shape.setRadius(SIZE / 2);
        shape.setFillColor(sf::Color::Red);
    }

    void respawn(const std::vector<sf::Vector2i> &snakeSegments)
    {
        do
        {
            position = {std::rand() % WIDTH, std::rand() % HEIGHT};
        } while (std::find(snakeSegments.begin(), snakeSegments.end(), position) != snakeSegments.end());

        shape.setPosition(position.x * SIZE, position.y * SIZE);
    }

    sf::Vector2i getPosition() const { return position; }
    sf::CircleShape shape;

private:
    sf::Vector2i position;
};

class Snake
{
public:
    Snake() : direction(RIGHT), nextDirection(RIGHT), grow(false)
    {
        segments.emplace_back(5, 5);
    }

    void handleInput()
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && direction != DOWN)
            nextDirection = UP;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && direction != UP)
            nextDirection = DOWN;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && direction != RIGHT)
            nextDirection = LEFT;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && direction != LEFT)
            nextDirection = RIGHT;
    }

    void update(std::vector<Apple> &apples)
    {
        direction = nextDirection;
        move();

        auto headPos = segments[0];
        for (auto &apple : apples)
        {
            if (headPos == apple.getPosition())
            {
                grow = true;
                apple.respawn(getSegmentPositions());
            }
        }

        if (checkSelfCollision())
            isDead = true;
    }

    void render(sf::RenderWindow &window)
    {
        sf::RectangleShape segmentShape(sf::Vector2f(SIZE, SIZE));
        segmentShape.setFillColor(sf::Color::Green);
        for (const auto &segment : segments)
        {
            segmentShape.setPosition(segment.x * SIZE, segment.y * SIZE);
            window.draw(segmentShape);
        }
    }

    bool isDead = false;

    std::vector<sf::Vector2i> getSegmentPositions() const
    {
        std::vector<sf::Vector2i> positions;
        for (const auto &segment : segments)
            positions.push_back(sf::Vector2i(segment.x, segment.y));
        return positions;
    }

    void reset()
    {
        segments = {Segment(5, 5)};
        direction = RIGHT;
        nextDirection = RIGHT;
        grow = false;
        isDead = false;
    }

private:
    struct Segment
    {
        int x, y;
        Segment(int x, int y) : x(x), y(y) {}
        bool operator==(const sf::Vector2i &pos) const { return x == pos.x && y == pos.y; }
    };

    void move()
    {
        if (grow)
        {
            segments.emplace_back(0, 0);
            grow = false;
        }

        for (int i = segments.size() - 1; i > 0; --i)
            segments[i] = segments[i - 1];

        switch (direction)
        {
        case UP:
            segments[0].y -= 1;
            break;
        case DOWN:
            segments[0].y += 1;
            break;
        case LEFT:
            segments[0].x -= 1;
            break;
        case RIGHT:
            segments[0].x += 1;
            break;
        }

        if (segments[0].x < 0)
            segments[0].x = WIDTH - 1;
        else if (segments[0].x >= WIDTH)
            segments[0].x = 0;
        if (segments[0].y < 0)
            segments[0].y = HEIGHT - 1;
        else if (segments[0].y >= HEIGHT)
            segments[0].y = 0;
    }

    bool checkSelfCollision()
    {
        for (int i = 1; i < segments.size(); ++i)
            if (segments[0] == sf::Vector2i(segments[i].x, segments[i].y))
                return true;
        return false;
    }

    Direction direction, nextDirection;
    std::vector<Segment> segments;
    bool grow;
};

class SnakeGame
{
public:
    SnakeGame() : window(sf::VideoMode(WIDTH * SIZE, HEIGHT * SIZE), "Snake Game"), gameOver(false), restartDelay(2.0f)
    {
        font.loadFromFile("arial.ttf");
        gameOverText.setFont(font);
        gameOverText.setString("Game Over");
        gameOverText.setCharacterSize(82);
        gameOverText.setFillColor(sf::Color::White);
        gameOverText.setPosition(WIDTH * SIZE / 2 - gameOverText.getLocalBounds().width / 2, HEIGHT * SIZE / 2 - gameOverText.getLocalBounds().height / 2);

        apples.resize(APPLE_COUNT);
        for (auto &apple : apples)
            apple.respawn(snake.getSegmentPositions());
    }

    void run()
    {
        sf::Clock clock;
        while (window.isOpen())
        {
            handleEvents();
            if (!gameOver)
                snake.handleInput();

            if (!gameOver && clock.getElapsedTime().asMilliseconds() > 150)
            {
                snake.update(apples);
                if (snake.isDead)
                {
                    gameOver = true;
                    gameOverClock.restart();
                }
                clock.restart();
            }

            if (gameOver && gameOverClock.getElapsedTime().asSeconds() >= restartDelay)
                resetGame();

            render();
        }
    }

private:
    sf::RenderWindow window;
    sf::Font font;
    sf::Text gameOverText;
    Snake snake;
    std::vector<Apple> apples;
    bool gameOver;
    sf::Clock gameOverClock;
    const float restartDelay;

    void handleEvents()
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) // Закрываем игру при нажатии Escape
                window.close();
        }
    }

    void render()
    {
        window.clear();
        for (auto &apple : apples)
            window.draw(apple.shape);
        snake.render(window);

        if (gameOver)
            window.draw(gameOverText);

        window.display();
    }

    void resetGame()
    {
        snake.reset();
        for (auto &apple : apples)
            apple.respawn(snake.getSegmentPositions());
        gameOver = false;
    }
};

int main()
{
    SnakeGame game;
    game.run();
    return 0;
}
