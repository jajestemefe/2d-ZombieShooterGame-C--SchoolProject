#include <SFML/Graphics.hpp>
#include <fmt/format.h>
#include <vector>
#include <cmath>
#include <random>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>
#include <iostream>

//SOURCES
//https://opengameart.org/content/animated-top-down-survivor-player
//https://opengameart.org/content/animated-top-down-zombie
//https://opengameart.org/content/30-grass-textures-tilable
//https://www.vecteezy.com/vector-art/23875308-emergency-first-aid-kit-game-pixel-art-vector-illustration
//https://www.transparentpng.com/details/bullets-free-download_22781.html
//https://www.dafont.com/minecraft.font

using namespace sf;

Vector2f normalize(Vector2f vec) {
    float length = std::sqrt(vec.x * vec.x + vec.y * vec.y);
    if (length != 0) return vec / length;
    return {0, 0};
}

struct Bullet {
    //CircleShape shape;
    RectangleShape shape;
    Vector2f velocity;
    float bulletSpeed = 100;

    Bullet(Vector2f position, Vector2f target) {
        //this->shape = CircleShape(5);
        this->shape = RectangleShape({18,4});
        this->shape.setOrigin({9,2});
        //shape.setFillColor(Color::Red);
        shape.setPosition(position);

        Vector2f direction = normalize(target - position);
        velocity = direction;
    }
};

struct Enemy {
    CircleShape shape;
    float health;
    Vector2f velocity;
    float enemySpeed;

    Enemy(Vector2f position, float health, Vector2f target){
        this->shape = CircleShape(35);
        shape.setOrigin({35,35});
        shape.setPosition(position);
        //shape.setFillColor(Color::Red);
        this->health = health;
        this->enemySpeed = 10;

        Vector2f direction = normalize(target - position);
        velocity = direction;
    }
};

struct SpecialEnemy{
    CircleShape shape;
    float health;
    Vector2f velocity;
    float enemySpeed;

    SpecialEnemy(Vector2f position, float health, Vector2f target) {
        this->shape = CircleShape(40);
        shape.setOrigin({40,40});
        shape.setPosition(position);
        //shape.setFillColor(Color::Blue);
        this->health = health;
        this->enemySpeed = 20;

        Vector2f direction = normalize(target - position);
        velocity = direction;
    }
};

struct AidKit {
    RectangleShape shape;

    AidKit() {
        this->shape = RectangleShape({26,40});
        shape.setOrigin({13,20});
        //shape.setFillColor(Color::Red);
    }
};

auto main()-> int {
    using namespace sf;

    //Building the window.
    auto window = RenderWindow(
            VideoMode({1280,720}),
            "My Semester Project",
            Style::Default,
            State::Windowed,
            ContextSettings{.antiAliasingLevel = 8}
        );

    //window.setFramerateLimit(240); I use delta time instead

    //Finding the middle point of the window
    auto middleWindow = Vector2f((float)window.getSize().x /2, (float)window.getSize().y /2);

    //Building the player
    auto player = CircleShape(50);
    auto playerTexture1 = Texture("../Sprites/Player_1.png");
    auto playerTexture2 = Texture("../Sprites/Player_2.png");
    player.setTexture(&playerTexture2);
    player.setOrigin({50,50});
    player.setPosition({middleWindow});
    auto playerVelocity = Vector2f();
    auto hitPlayer = false;
    auto damageClock = Clock();
    auto isDead = false;

    //is game saved
    auto isGameSaved = false;

    //is restart game
    auto restartGame = false;

    //score
    auto score = 0;
    auto highestScore = 0;

    //Building crosshair
    auto crosshair = RectangleShape(Vector2f(20,20));
    crosshair.setOrigin({10,10});
    auto crosshairTexture = Texture("../Sprites/Crosshair.png");
    crosshair.setTexture(&crosshairTexture);

    //Player movements
    auto playerMoveRight = false;
    auto playerMoveLeft = false;
    auto playerMoveUp = false;
    auto playerMoveDown = false;
    auto playerSprint = false;
    auto mouseClicked = false;

    //Stamina
    auto font1 = Font("../Fonts/Minecraft.ttf");
    auto text1 = Text(font1, "", 24);
    text1.setPosition({20,10});
    text1.setFillColor(Color::Blue);
    text1.setOutlineColor(Color::Black);
    text1.setOutlineThickness(3);
    auto playerStamina = 100.f;

    //is load game
    auto loadGame = false;
    auto chooseLoadFile = false;
    auto iteratorRight = false;
    auto iteratorLeft = false;
    auto saveText = Text(font1, "", 36);
    saveText.setFillColor(Color::White);
    saveText.setOutlineColor(Color::Black);
    saveText.setOutlineThickness(3);
    saveText.setPosition({400,550});
    auto loadText = Text(font1, "", 36);
    loadText.setFillColor(Color::Cyan);
    loadText.setOutlineColor(Color::Black);
    loadText.setOutlineThickness(3);
    loadText.setPosition({575,505});
    auto loadFilePath = std::string();

    //Pause the game on escape
    auto isGamePaused = false;
    auto pausedText = Text(font1, "", 72);
    pausedText.setPosition({520,300});
    pausedText.setFillColor(Color::Red);
    pausedText.setOutlineColor(Color::Black);
    pausedText.setOutlineThickness(3);

    //Health
    auto text2 = Text(font1, "", 24);
    text2.setPosition({20,40});
    text2.setFillColor(Color::Red);
    text2.setOutlineColor(Color::Black);
    text2.setOutlineThickness(3);
    auto playerHealth = 100.f;

    //Enemy
    auto enemies = std::vector<Enemy>();
    auto spawnEnemy = true;
    auto enemySpawnLocation = Vector2f();
    auto enemyClock = Clock();
    auto enemySpawnTime = 2.f;
    auto enemyTexture = Texture("../Sprites/Zombie.png");

    //Special enemies
    auto specialEnemies = std::vector<SpecialEnemy>();
    auto spawnSpecialEnemy = true;
    auto specialEnemySpawnLocation = Vector2f();
    auto specialEnemyClock = Clock();
    auto specialEnemySpawnTime = 6.f;
    auto specialEnemyTexture = Texture("../Sprites/Special_Enemy.png");
    auto specialEnemyCounter = 0;

    //First aid kit
    auto aidKits = std::vector<AidKit>();
    auto aidKitTexture = Texture("../Sprites/AidKit.png");
    auto aidKitRate = 5;

    //Kill count
    auto killCount = 0.f;
    auto text6 = Text(font1, "", 24);
    text6.setPosition({(float)window.getSize().x-90 ,40});
    text6.setFillColor(Color::Cyan);
    text6.setOutlineColor(Color::Black);
    text6.setOutlineThickness(3);

    //Miss count
    auto missCount = 0.f;
    auto text7 = Text(font1, "", 24);
    text7.setOutlineColor(Color::Black);
    text7.setOutlineThickness(3);
    text7.setPosition({(float)window.getSize().x-120 ,10});
    text7.setFillColor(Color::Yellow);

    //End Text
    auto endText = Text(font1, "", 48);
    endText.setPosition({500,90});
    endText.setFillColor(Color::Red);
    endText.setOutlineColor(Color::Black);
    endText.setOutlineThickness(3);

    //Restart Text
    auto restartText = Text(font1, "", 36);
    restartText.setPosition({545,405});
    restartText.setFillColor(Color::Green);
    restartText.setOutlineColor(Color::Black);
    restartText.setOutlineThickness(3);

    //Saving Text
    auto savingText = Text(font1, "", 36);
    savingText.setPosition({565,455});
    savingText.setFillColor(Color::Blue);
    savingText.setOutlineColor(Color::Black);
    savingText.setOutlineThickness(3);

    //Background
    auto backgroundTexture = Texture("../Sprites/Background.png");
    auto background = RectangleShape({(float)window.getSize().x, (float)window.getSize().y});
    background.setTexture(&backgroundTexture);

    //Magazine
    auto bullets = std::vector<Bullet>();
    auto enoughAmmo = true;
    auto text3 = Text(font1, "", 24);
    text3.setPosition({20,70});
    text3.setFillColor(Color::Green);
    text3.setOutlineColor(Color::Black);
    text3.setOutlineThickness(3);
    auto maxAmmo = 12;
    auto playerAmmo = maxAmmo;
    auto bulletTexture = Texture("../Sprites/Bullet.png");

    //Reloading text
    auto text4 = Text(font1, "", 18);
    text4.setOrigin({80,15});
    text4.setPosition(Vector2f((float)window.getSize().x/2, (float)window.getSize().y - 10));
    text4.setFillColor(Color::Red);
    text4.setOutlineColor(Color::Black);
    text4.setOutlineThickness(3);
    auto isReloading = false;
    auto reloadClock = Clock();
    auto reloadTime = 5.0f;

    //Reloading countdown
    auto text5 = Text(font1, "", 18);
    text5.setOrigin({100,15});
    text5.setPosition(Vector2f((float)window.getSize().x/2, (float)window.getSize().y - 30));
    text5.setFillColor(Color::Red);
    text5.setOutlineColor(Color::Black);
    text5.setOutlineThickness(3);

    //Clock and delta time
    auto gameClock = Clock();
    auto dt = 0.f;

    while (window.isOpen()) {
        while (auto const event = window.pollEvent()) {
            if (event->is<Event::Closed>()) window.close();
            //
            if (auto const e = event->getIf<Event::KeyPressed>()) {
                if (e->code == Keyboard::Key::W) playerMoveUp = true;
            }
            //
            if (auto const e = event->getIf<Event::KeyPressed>()) {
                if (e->code == Keyboard::Key::S) playerMoveDown = true;
            }
            //
            if (auto const e = event->getIf<Event::KeyPressed>()) {
                if (e->code == Keyboard::Key::D) playerMoveRight = true;
            }
            //
            if (auto const e = event->getIf<Event::KeyPressed>()) {
                if (e->code == Keyboard::Key::A) playerMoveLeft = true;
            }
            //
            if (auto const e = event->getIf<Event::KeyPressed>()) {
                if (e->code == Keyboard::Key::A && chooseLoadFile) {
                    iteratorLeft = true;
                }
            }
            if (auto const e = event->getIf<Event::KeyPressed>()) {
                if (e->code == Keyboard::Key::D && chooseLoadFile) {
                    iteratorRight = true;
                }
            }
            //
            if (auto const e = event->getIf<Event::KeyPressed>()) {
                if (e->code == Keyboard::Key::LShift) playerSprint = true;
            }
            //
            if (auto const e = event->getIf<Event::KeyPressed>()) {
                if (e->code == Keyboard::Key::S && isGamePaused && !isGameSaved) {
                    isGameSaved = true;
                }
            }
            //
            if (auto const e = event->getIf<Event::KeyPressed>()) {
                if (e->code == Keyboard::Key::L && (isGamePaused || isDead)) {
                    if (!chooseLoadFile) {
                        chooseLoadFile = true;
                    }else chooseLoadFile = false;
                }
            }
            //
            if (auto const e = event->getIf<Event::KeyPressed>()) {
                if (e->code == Keyboard::Key::Enter && chooseLoadFile) {
                    loadGame = true;
                }
            }
            //
            if (auto const e = event->getIf<Event::KeyPressed>()) {
                if (e->code == Keyboard::Key::R && (isGamePaused || isDead)) {
                    restartGame = true;
                }
            }
            //
            if (auto const e = event->getIf<Event::KeyPressed>()) {
                if (e->code == Keyboard::Key::Escape) {
                    if (!isDead && isGamePaused) {
                        isGamePaused = false;
                    }else
                        {isGamePaused = true;}
                }
            }
            //
            if (auto const e = event->getIf<Event::KeyReleased>()) {
                if (e->code == Keyboard::Key::W) playerMoveUp = false;
            }
            //
            if (auto const e = event->getIf<Event::KeyReleased>()) {
                if (e->code == Keyboard::Key::S) playerMoveDown = false;
            }
            //
            if (auto const e = event->getIf<Event::KeyReleased>()) {
                if (e->code == Keyboard::Key::D) playerMoveRight = false;
            }
            //
            if (auto const e = event->getIf<Event::KeyReleased>()) {
                if (e->code == Keyboard::Key::A) playerMoveLeft = false;
            }
            //
            if (auto const e = event->getIf<Event::KeyReleased>()) {
                if (e->code == Keyboard::Key::LShift) playerSprint = false;
            }
            //
            //if mouse left  button is clicked
            if (auto const e = event->getIf<Event::MouseButtonPressed>()) {
                if (e->button == Mouse::Button::Left && !isReloading) mouseClicked = true;
            }
            if (auto const e = event->getIf<Event::MouseButtonReleased>()) {
                if (e->button == Mouse::Button::Left) mouseClicked = false;
            }
            //

            if (auto const e = event->getIf<Event::KeyPressed>()) {
                if (e->code == Keyboard::Key::R && !isGamePaused && !isReloading) {
                    isReloading = true;
                    reloadClock.restart();
                }
            }
        }

        //Find mouse position
        auto mousePosition = Vector2f(Mouse::getPosition(window));
        crosshair.setPosition(mousePosition);

        //calculate the delta time
        dt = gameClock.restart().asSeconds() * 10;

        //Move the player and keep it in borders
        if (!isGamePaused && playerMoveUp && player.getPosition().y > 0) {
            player.move({0, -playerVelocity.y * dt});
        }
        if (!isGamePaused && playerMoveDown && player.getPosition().y < (float)window.getSize().y) {
            player.move({0, playerVelocity.y * dt});
        }
        if (!isGamePaused && playerMoveRight && player.getPosition().x < (float)window.getSize().x) {
            player.move({playerVelocity.x * dt, 0});
        }
        if (!isGamePaused && playerMoveLeft && player.getPosition().x > 0) {
            player.move({-playerVelocity.x * dt, 0});
        }

        //player sprint
        if (!isGamePaused && playerSprint && playerStamina > 0) {
            playerVelocity = {30,30};
            playerStamina -= dt*5;
        }
        else if (!playerSprint || playerStamina <= 0) {
            playerVelocity = {15,15};
        }

        //Refill the stamina
        if (!isGamePaused && playerStamina < 100 && !playerSprint) {
            playerStamina += dt;
        }

        //check if the player has ammo
        if (playerAmmo <= 0) {
            enoughAmmo = false;
        } else enoughAmmo = true;

        //Player rotation through mouse
        if (!isGamePaused) {
            auto angleX = mousePosition.x - player.getPosition().x;
            auto angleY = mousePosition.y -  player.getPosition().y;
            auto angleRadians = std::atan2(angleY, angleX) * 180 / std::numbers::pi;
            player.setRotation(Angle(degrees((float)angleRadians)));
        }

        //adding bullets to the vector
        if (!isGamePaused && !isDead && mouseClicked && enoughAmmo) {
            auto bullet = Bullet(player.getPosition(), mousePosition);
            bullet.shape.setTexture(&bulletTexture);

            auto bulletX = bullet.shape.getPosition().x - mousePosition.x;
            auto bulletY = bullet.shape.getPosition().y - mousePosition.y;
            auto bulletRadians = std::atan2(bulletY, bulletX) * 180 / std::numbers::pi;
            bullet.shape.setRotation(Angle(degrees((float)bulletRadians)));

            bullets.push_back(bullet);
            playerAmmo -= 1;
            mouseClicked = false; // to prevent holding left button
        }

        //shooting the bullets in vector
        for (auto& bullet : bullets) {
            if (!isGamePaused) {
                bullet.shape.move(bullet.velocity * (dt * bullet.bulletSpeed));
            }
        }

        //Check if the game is paused before moving enemies
        if (!isGamePaused){
        //getting the enemy spawn location
        auto randomDevice = std::random_device();
        auto gen = std::mt19937(randomDevice());
        auto sideDecider = std::uniform_int_distribution<int>(0,1);

        auto spawnSide = false;
        spawnSide = sideDecider(gen);
        auto spawnXLeft = std::uniform_real_distribution<float>(-300.f,-50.f);
        auto spawnXRight = std::uniform_real_distribution<float>((float)window.getSize().x + 50, (float)window.getSize().x + 300);
        auto spawnY = std::uniform_real_distribution<float>(-200.f, (float)window.getSize().y + 200);
        if (spawnSide) {
            enemySpawnLocation = Vector2f({spawnXLeft(gen), spawnY(gen)});
        }else {
            enemySpawnLocation = Vector2f({spawnXRight(gen), spawnY(gen)});
        }

        //getting the special enemy spawn location
        spawnSide = sideDecider(gen);
        spawnXLeft = std::uniform_real_distribution<float>(-300.f,-50.f);
        spawnXRight = std::uniform_real_distribution<float>((float)window.getSize().x + 50, (float)window.getSize().x + 300);
        if (spawnSide) {
            specialEnemySpawnLocation = Vector2f({spawnXLeft(gen), spawnY(gen)});
        }else {
            specialEnemySpawnLocation = Vector2f({spawnXRight(gen), spawnY(gen)});
        }

        //Enemy
        if (!isDead && spawnEnemy && enemyClock.getElapsedTime().asSeconds() >= enemySpawnTime) {
            auto enemy = Enemy(enemySpawnLocation, 40.f, player.getPosition());
            enemy.shape.setTexture(&enemyTexture);
            enemies.push_back(enemy);
            enemyClock.restart();
        }

        //Special enemy
        if (!isDead && spawnSpecialEnemy && specialEnemyClock.getElapsedTime().asSeconds() >= specialEnemySpawnTime) {
            auto specialEnemy = SpecialEnemy(specialEnemySpawnLocation, 100.f, player.getPosition());
            specialEnemy.shape.setTexture(&specialEnemyTexture);
            specialEnemies.push_back(specialEnemy);
            specialEnemyClock.restart();
        }

        //Moving and rotating the enemies
        for (auto& enemy : enemies) {
            Vector2f direction = normalize(player.getPosition() - enemy.shape.getPosition());
            auto enemyX = player.getPosition().x - enemy.shape.getPosition().x;
            auto enemyY = player.getPosition().y - enemy.shape.getPosition().y;
            auto enemyRadians = std::atan2(enemyY, enemyX) * 180 / std::numbers::pi;
            enemy.shape.setRotation(Angle(degrees((float)enemyRadians)));
            enemy.velocity = direction;
            enemy.shape.move(enemy.velocity * (dt * enemy.enemySpeed));
        }

        //Moving and rotating the special enemies
        for (auto& sEnemy : specialEnemies) {
            Vector2f direction = normalize(player.getPosition() - sEnemy.shape.getPosition());
            auto enemyX = player.getPosition().x - sEnemy.shape.getPosition().x;
            auto enemyY = player.getPosition().y - sEnemy.shape.getPosition().y;
            auto enemyRadians = std::atan2(enemyY, enemyX) * 180 / std::numbers::pi;
            sEnemy.shape.setRotation((Angle(degrees((float)enemyRadians))));
            sEnemy.velocity = direction;
            sEnemy.shape.move(sEnemy.velocity * (dt * sEnemy.enemySpeed));
            }
        }

        //Miss counts
        for (auto b: bullets) {
            if (b.shape.getPosition().x < 0 || b.shape.getPosition().y < 0 ||
                b.shape.getPosition().x > (float)window.getSize().x || b.shape.getPosition().y > (float)window.getSize().y) {
                missCount++;
            }
        }

        //Removing outbounds bullets
        bullets.erase(
            std::remove_if(bullets.begin(), bullets.end(), [&](Bullet const& b) {
                Vector2f pos = b.shape.getPosition();
                return pos.x < 0 || pos.x > (float)window.getSize().x || pos.y < 0 || pos.y > (float)window.getSize().y;
            }),
            bullets.end()
        );

        //Reloading
        if (!isGamePaused && isReloading && reloadClock.getElapsedTime().asSeconds() >= reloadTime) {
            playerAmmo = maxAmmo;
            isReloading = false;
            enoughAmmo = true;
        }

        //killing enemies on collusion
        for (auto b = bullets.begin(); b != bullets.end();) {
            auto bulletRemoved = false;

            for (auto e = enemies.begin(); e!= enemies.end();) {
                if (b->shape.getGlobalBounds().findIntersection(e->shape.getGlobalBounds())) {
                    e->health -= 40;
                    if (e->health <=0) {
                        e = enemies.erase(e);
                        killCount++;
                    }
                    b = bullets.erase(b);
                    bulletRemoved = true;
                    break;
                }
                else ++e;
            }
            if (!bulletRemoved) ++b;
        }

        //killing special enemies on collusion
        for (auto b = bullets.begin(); b != bullets.end();) {
            auto bulletRemoved = false;

            for (auto se = specialEnemies.begin(); se!= specialEnemies.end();) {
                if (b->shape.getGlobalBounds().findIntersection(se->shape.getGlobalBounds())) {
                    se->health -= 40;
                    if (se->health <= 0) {
                        //Dropping first aid kit
                        if (specialEnemyCounter != 0 && specialEnemyCounter % aidKitRate == 0 && aidKits.empty()) {
                            auto aidKit = AidKit();
                            aidKit.shape.setTexture(&aidKitTexture);
                            aidKit.shape.setPosition(se->shape.getPosition());
                            aidKits.push_back(aidKit);
                        }
                        se = specialEnemies.erase(se);
                        killCount++;
                        specialEnemyCounter++;
                    }
                    b = bullets.erase(b);
                    bulletRemoved = true;
                    break;
                }
                else ++se;
            }
            if (!bulletRemoved) ++b;
        }

        //check if the player gets aidkit
        for (auto aid = aidKits.begin(); aid!= aidKits.end();) {
            if (player.getGlobalBounds().findIntersection(aid->shape.getGlobalBounds())) {
                aid = aidKits.erase(aid);
                playerHealth += 20;
                if (playerHealth > 100) {
                    playerHealth = 100;
                }
            }else {
                ++aid;
            }
        }

        //check if the player is harmed by enemies
        for (const auto& enemy: enemies) {
            if (!isGamePaused && player.getGlobalBounds().findIntersection(enemy.shape.getGlobalBounds())) {
                hitPlayer = true;
                break;
            } else hitPlayer = false;
        }

        //check if the player is harmed by special enemies
        for (const auto& sEnemy : specialEnemies) {
            if (!isGamePaused && player.getGlobalBounds().findIntersection(sEnemy.shape.getGlobalBounds())) {
                hitPlayer = true;
                break;
            } else hitPlayer = false;
        }
        //check if no one hits


        //hitting the player
        if (!isDead && !isGamePaused && hitPlayer && damageClock.getElapsedTime().asSeconds() >= 1) {
            damageClock.restart();
            playerHealth -= 10.f;
            hitPlayer = false;
        }

        //check if the player is dead
        if (playerHealth <= 0) {
            isDead = true;
        }

        //Saving the current stats to load back
        if (isGamePaused && isGameSaved) {
            //Create a new save file
            //getting the current time
            auto now = std::chrono::system_clock::now();
            std::time_t now_time = std::chrono::system_clock::to_time_t(now);
            std::tm* local_tm = std::localtime(&now_time);
            //formatting it
            std::string filename = fmt::format("../Saves/save_{:04}{:02}{:02}_{:02}{:02}{:02}.txt",
                    local_tm->tm_year + 1900,
                    local_tm->tm_mon +1,
                    local_tm->tm_mday,
                    local_tm->tm_hour,
                    local_tm->tm_min,
                    local_tm->tm_sec
                );
            //writing to new file
            std::ofstream outFile(filename);
            if (outFile.is_open()) {
                outFile << fmt::format("Created at {}",std::ctime(&now_time));
                outFile << "Stamina:" << playerStamina << "\n";
                outFile << "Health:" << playerHealth << "\n";
                outFile << "Ammo:" << playerAmmo << "\n";
                outFile << "Misses:" << missCount << "\n";
                outFile << "Kills:" << killCount << "\n";
                outFile << "sKills:" << specialEnemyCounter << "\n";
                outFile.close();
                isGameSaved = false;
            }
        }

        //Restart game
        if (restartGame) {
            player.setPosition(middleWindow);
            playerStamina = 100;
            playerHealth = 100;
            playerAmmo = maxAmmo;
            missCount = 0;
            killCount = 0;
            specialEnemyCounter = 0;
            isReloading = false;
            enoughAmmo = true;
            enemies.clear();
            specialEnemies.clear();
            aidKits.clear();
            isGamePaused = false;
            isDead = false;
            restartGame = false;
            bullets.clear();
        }
        if (isDead) {
            if (missCount == 0) {
                score = (int)killCount;
            }else if (killCount == 0) {
                score = 0;
            }else {
                score = (int)(killCount/missCount);
            }
        }

        //check if the player wants to load from a file
        namespace fs = std::filesystem;
        if (chooseLoadFile) {
            std::vector<std::string> saves;
            //
            for (const auto& entry : fs::directory_iterator("../Saves/")) {
                if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                    saves.push_back(entry.path().filename().string());
                }
            }
            //
            if (!saves.empty()) {
                static int currentIndex = 0;

                if (iteratorRight && currentIndex + 1 < saves.size()) {
                    currentIndex++;
                    iteratorRight = false;
                } else if (iteratorLeft && currentIndex > 0) {
                    currentIndex--;
                    iteratorLeft = false;
                }

                saveText.setString(saves[currentIndex]);
                loadFilePath = saves[currentIndex];
            }
            //
        }

        //Load game
        if (loadGame) {
            auto file = std::ifstream("../Saves/" + loadFilePath);
            auto line = std::string();

            if (!file.is_open()) {
                std::cerr << "Failed to open file";
                return 1;
            }

            while (std::getline(file, line)) {
                auto lineStream = std::istringstream(line);
                auto word = std::string();

                while (lineStream >> word) {
                    if (word.rfind("Stamina:", 0) == 0 && word.length() > 8) {
                        auto value = word.substr(8);
                        playerStamina = std::stof(value);
                    }
                    else if (word.rfind("Health:", 0) == 0 && word.length() > 7) {
                        auto value = word.substr(7);
                        playerHealth = std::stof(value);
                    }
                    else if (word.rfind("Ammo:", 0) == 0 && word.length() > 5) {
                        auto value = word.substr(5);
                        playerAmmo = std::stoi(value);
                    }
                    else if (word.rfind("Misses:", 0) == 0 && word.length() > 7) {
                        auto value = word.substr(7);
                        missCount = std::stof(value);
                    }
                    else if (word.rfind("Kills:", 0) == 0 && word.length() > 6) {
                        auto value = word.substr(6);
                        killCount = std::stof(value);
                    }
                    else if (word.rfind("sKills:", 0) == 0 && word.length() > 7) {
                        auto value = word.substr(7);
                        specialEnemyCounter = std::stoi(value);
                    }
                }
            }
            file.close();
            player.setPosition(middleWindow);
            enemies.clear();
            specialEnemies.clear();
            aidKits.clear();
            loadGame = false;
            isGamePaused = false;
            chooseLoadFile = false;
            isDead = false;
        }

        //Reading the highest score from the record
        if (isDead) {
            auto file = std::ifstream("../HighestScore.txt");
            auto line = std::string();

            if (!file.is_open()) {
                std::cerr << "Failed to open file";
                return 1;
            }

            while (std::getline(file, line)) {
                auto lineStream = std::istringstream(line);
                auto word = std::string();

                while (lineStream >> word) {
                    if (word.rfind("HighestScore:", 0) == 0 && word.length() > 13) {
                        auto value = word.substr(13);
                        highestScore = std::stoi(value);
                    }
                }
            }
            file.close();
            //if the highest score is less then the current score just replace them
            if (highestScore < score) {
                auto file = std::ofstream("../HighestScore.txt");
                file << "HighestScore:" << score;
                highestScore = score;
                file.close();
            }
        }

        //Formatting the texts
        text1.setString(fmt::format("Stamina: {}", (int) playerStamina));
        text2.setString(fmt::format("Health: {}", (int)playerHealth));
        text3.setString(fmt::format("Ammo: {}", (int)playerAmmo));
        text4.setString(fmt::format("press R to reload"));
        if (!isGamePaused) {
            reloadClock.start();
            text5.setString(fmt::format("Reloading in {} seconds", reloadTime - (int)reloadClock.getElapsedTime().asSeconds()));
        } else {
            reloadClock.stop();
        }
        text6.setString(fmt::format("{} Kills", killCount));
        text7.setString(fmt::format("{} Misses", missCount));
        endText.setString(fmt::format("Game Over!\nMax kill: {}\nSpecial kills: {}\nTotal misses: {}\nScore: {}\nHigh Score: {}", killCount, specialEnemyCounter, missCount, score, highestScore));
        pausedText.setString("Paused");
        restartText.setString("R to restart");
        savingText.setString("S to save");
        loadText.setString("L to load");
        //

        //Displaying components
        window.clear();
        if (!isDead) {
            window.draw(background);
            for (auto& aidkit: aidKits)
                window.draw(aidkit.shape);
            for (auto& enemy : enemies)
                window.draw(enemy.shape);
            for (auto& sEnemy: specialEnemies)
                window.draw(sEnemy.shape);
            window.draw(player);
            for (auto& bullet : bullets)
                window.draw(bullet.shape);
            window.draw(crosshair);
            window.draw(text1);
            window.draw(text2);
            window.draw(text3);
            if (!enoughAmmo) window.draw(text4);
            if (isReloading) window.draw(text5);
            window.draw(text6);
            window.draw(text7);
        }else {
            window.draw(endText);
            window.draw(restartText);
            window.draw(loadText);
            if (chooseLoadFile) {
                window.draw(saveText);
            }
        }
        if (isGamePaused) {
            window.draw(pausedText);
            window.draw(restartText);
            window.draw(savingText);
            window.draw(loadText);
            if (chooseLoadFile) {
                window.draw(saveText);
            }
        }
        window.display();

        //fmt::println("{}",dt);
        //fmt::println("{}", aidKits.size());
        //fmt::println("{}", isGamePaused);
    }
}