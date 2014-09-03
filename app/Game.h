#ifndef INCLUDED__Bounce__Game_h
#define INCLUDED__Bounce__Game_h

#include <bricabrac/Game/GameActor.h>
#include <bricabrac/Utility/Signal.h>

constexpr float THREE_LINE_Y = 0;
constexpr float SHOT_LINE_Y = 4.7;

struct Character : brac::Actor {
    enum State { neutral, happy, sad, excited };
};

struct Bird : brac::Actor {
    enum State { hover, dive, ascend };

    Character * captive() const;
};

enum GameMode { m_menu, m_arcade, m_buzzer };
constexpr GameMode MODE = m_menu;

class Game : public brac::GameBase, public std::enable_shared_from_this<Game> {
public:
    enum HoopState { hoop_on = 1, hoop_off = 0 };
    enum ShotlineState { line_default = 0, line_red = 1 };

    struct State {
        size_t score = 0;
        HoopState hoop_state = hoop_off;
        ShotlineState line_state = line_default;
        std::string alert = "";
        GameMode mode;
        size_t clock = 0;
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

    Game(GameMode mode);
    ~Game();

    State const & state() const;
    
    virtual std::unique_ptr<brac::TouchHandler> fingerTouch(brac::vec2 const & p, float radius) override;

private:
    struct Members;
    std::unique_ptr<Members> m;

    virtual void doUpdate(float dt) override;
    virtual void getActors(size_t actorId, void * buf) const override;
};

#endif // INCLUDED__Bounce__Game_h
