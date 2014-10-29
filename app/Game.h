#ifndef INCLUDED__Claws__Game_h
#define INCLUDED__Claws__Game_h

#include "UI.h"
#include "atlas.sprites.h"

#include <bricabrac/Game/GameActor.h>
#include <bricabrac/Utility/Signal.h>

constexpr float GRAVITY = -30;
constexpr float LAUNCH_OFFSET = 1;
constexpr float WORLD_GRAVITY = -10;
constexpr float ATTACK_LINE_Y = 6;

// Temporary: move to GameParams struct and passed as param to newGame?
constexpr int   CHARACTERS = 2;
constexpr int   BIRDS = 10;
constexpr float BIRDFREQUENCY = 3.5;
constexpr float BIRD_SPEED = 1;

// Scoring
constexpr int   SCORE_DART_FIRED = 20;
constexpr int   SCORE_BIRD_KILLED = 100;
constexpr int   SCORE_CHAR_RESCUED = 150;
constexpr int   SCORE_CHAR_SURVIVED = 1000;

struct Character : brac::Actor {
    enum State {
        biggrin,
        celebrating,
        rescued,
        confused,
        dead,
        determined,
        exclaim,
        hurt,
        shrug,
        sad,
        smile,
        smug,
        yell,
        startled,
        mag,
        aim,
        ready,
        shooting,
        crying,
        reloading,
    };

    virtual bool isAiming() const = 0;
    virtual brac::vec2 const & launchVel() const = 0;
};

struct Bird : brac::Actor {
    enum State { side, front, rear, dying, puff };

    virtual bool isFlying() const = 0;
};

struct Dart : brac::Actor { };

struct PersonalSpace : brac::Actor { };

struct CharacterStats {
    brac::SpriteDef mugshot;
    int dartsFired = 0;
    int birdsKilled = 0;
    int friendlies = 0;
    int rescues = 0;
    int kidnapped = 0;

    int score() {
        return dartsFired * SCORE_DART_FIRED
            + birdsKilled * SCORE_BIRD_KILLED
            + rescues * SCORE_CHAR_RESCUED;
    }
};

struct PlayerStats {
    // level-defining params
    int characters = 0;
    int birds = 0;

    // live stats
    size_t score = 0;
    int darts = 0;
    int kills = 0;
    int remCharacters = 0;
    float time = 0;
};

enum GameMode { m_menu, m_play, m_arcade, m_buzzer };
constexpr GameMode MODE = m_menu;

class Game : public brac::GameBase, public std::enable_shared_from_this<Game> {
public:
    struct State {
        bool started = false;
        bool level_passed = false;
        size_t score = 0;
        GameMode mode;
        int level;
        std::shared_ptr<Button> back{std::make_shared<Button>(atlas.back, brac::vec2{-9.2, 10}, 1)};
        std::shared_ptr<Button> restart{std::make_shared<Button>(atlas.restart, brac::vec2{-7.9, 10}, 1)};
        size_t rem_birds;
        size_t rem_chars;
        PlayerStats playerStats;
        std::vector<CharacterStats> characterStats;
    };

    brac::Signal<void()> show_menu;
    brac::Signal<void(Character const &, brac::vec2 const & impulse)> bounced;
    brac::Signal<void()> scored;
    brac::Signal<void()> touched_sides;
    brac::Signal<void()> door_open;
    brac::Signal<void()> bounced_wall;
    brac::Signal<void()> clock_beep;
    brac::Signal<void()> release_ball;
    brac::Signal<void()> foul;

    // Achievement-related events
    brac::Signal<void(size_t n)> n_for_n; // n hoops from n hits
    brac::Signal<void()> sharpshot;

    Game(brac::SpaceTime & st, GameMode mode, int level, float top);
    ~Game();

    State const & state() const;
    
    virtual std::unique_ptr<brac::TouchHandler> fingerTouch(brac::vec2 const & p, float radius) override;
    void gameOver(bool passed);
    void archiveCharacterStats(CharacterStats s);

private:
    struct Members;
    std::unique_ptr<Members> m;

    virtual void doUpdate(float dt) override;
    virtual void getActors(size_t actorId, void * buf) const override;
};

#endif // INCLUDED__Claws__Game_h
