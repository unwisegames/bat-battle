#ifndef INCLUDED__Bounce__Game_h
#define INCLUDED__Bounce__Game_h

#include "UI.h"
#include "atlas.sprites.h"

#include <bricabrac/Game/GameActor.h>
#include <bricabrac/Utility/Signal.h>

constexpr float GRAVITY = -30;
constexpr float LAUNCH_OFFSET = 1;
constexpr float WORLD_GRAVITY = -10;
constexpr int   CHARACTERS = 4; // Temporary: move to GameParams struct and passed as param to newGame?
constexpr int   BIRDS = 5;
constexpr float BIRDFREQUENCY = 3.5;

struct Character : brac::Actor {
    enum State {
        biggrin,
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

enum GameMode { m_menu, m_play, m_arcade, m_buzzer };
constexpr GameMode MODE = m_menu;

class Game : public brac::GameBase, public std::enable_shared_from_this<Game> {
public:
    struct State {
        size_t score = 0;
        GameMode mode;
        std::shared_ptr<Button> back{std::make_shared<Button>(atlas.back, brac::vec2{-9.2, 10}, 1)};
        std::shared_ptr<Button> restart{std::make_shared<Button>(atlas.restart, brac::vec2{-7.9, 10}, 1)};
        size_t rem_birds;
        size_t rem_chars;
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

    Game(brac::SpaceTime & st, GameMode mode, float top);
    ~Game();

    State const & state() const;
    
    virtual std::unique_ptr<brac::TouchHandler> fingerTouch(brac::vec2 const & p, float radius) override;
    void gameOver();

private:
    struct Members;
    std::unique_ptr<Members> m;

    virtual void doUpdate(float dt) override;
    virtual void getActors(size_t actorId, void * buf) const override;
};

#endif // INCLUDED__Bounce__Game_h
