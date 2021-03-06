#ifndef INCLUDED__Claws__Game_h
#define INCLUDED__Claws__Game_h

#include "UI.h"
#include "atlas.sprites.h"
#include "characters.sprites.h"

#include <bricabrac/Game/GameActor.h>
#include <bricabrac/Utility/Signal.h>
#include <bricabrac/Utility/Timer.h>

constexpr float LAUNCH_OFFSET = 1;
const brac::vec2 WORLD_GRAVITY = {0, -10};
constexpr int   DART_TRAIL_SEGMENTS = 10;

constexpr int   SCORE_CHAR_SURVIVED = 1000;

struct GameParams {
    int characters;
    int grey_bats;
    int yellow_bats;
    float bird_interval = 3.5;
    float bird_speed = 1;
    int max_simul_bats = 1;
};

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
        basketball,
        hum,
        wave
    };

    virtual bool isAiming() const = 0;
    virtual brac::vec2 const & launchVel() const = 0;
    virtual bool isDead() const = 0;
    virtual bool readyToFire() const = 0;
};

struct Bird : brac::Actor {
    enum State { side, front, rear, dying, puff };

    virtual bool isFlying() const = 0;
};

struct Dart : brac::Actor {
    bool active;
    size_t score = 10;
    std::array<brac::vec2, DART_TRAIL_SEGMENTS + 1> trail;
    float dt = 0;
};

struct PersonalSpace : brac::Actor { };

struct Grave : brac::Actor {
    enum State { rising, still };
};

struct BombBat : brac::Actor {
    enum State { flying, dying };
};

struct BombBatCarrot : brac::Actor { };

struct Bomb : brac::Actor {
    int countdown = 0;
};

struct Blast : brac::Actor { };

struct CharacterExplosion : brac::Actor { };

struct CharacterStats {
    brac::SpriteDef mugshot;
    int dartsFired = 0;
    int dartsHit = 0;
    int birdsKilled = 0;
    int friendlies = 0;
    int rescues = 0;
    int kidnapped = 0;
    size_t score = 0;

    /*int score() {
        return dartsFired * SCORE_DART_FIRED
            + birdsKilled * SCORE_BIRD_KILLED
            + rescues * SCORE_CHAR_RESCUED;
    }*/
};

struct PlayerStats {
    // level-defining params
    int characters = 0;
    int birds = 0;

    // live stats
    size_t score = 0;
    int darts = 0;
    int hits = 0;
    int kills = 0;
    size_t remCharacters = 0;
    float time = 0;
};

struct TextAlert {
    std::string s;
    brac::vec2 pos;
    float scale;
    float alpha = 1;
};

enum GameMode { m_menu, m_play, m_arcade, m_buzzer };
constexpr GameMode MODE = m_menu;

class Game : public brac::GameBase, public std::enable_shared_from_this<Game> {
public:
    struct State {
        float dt = 0;
        bool started = false;
        bool show_char_score = false;
        size_t score = 0;
        GameMode mode;
        std::shared_ptr<Button> back_btn {std::make_shared<Button>(atlas.back      , brac::vec2{-9.2, 10}, 1)};
        std::shared_ptr<Button> pause_btn{std::make_shared<Button>(characters.pause, brac::vec2{-7.9, 10}, 1)};
        std::shared_ptr<Button> play_btn {std::make_shared<Button>(atlas.play      , brac::vec2{ 0, 5 }, 1)};
        //size_t rem_grey_bats;
        //size_t rem_yellow_bats;
        size_t grey_bats_killed = 0;
        size_t yellow_bats_killed = 0;
        size_t bomb_bats_killed = 0;
        size_t rem_chars;
        PlayerStats playerStats;
        std::vector<CharacterStats> characterStats;
        std::list<TextAlert> alerts;
        GameParams params;
        bool game_over = false;
        bool end_triggered = false;
    };

    brac::Signal<void()> show_menu;
    brac::Signal<void(Character const &, brac::vec2 const & impulse)> bounced;
    brac::Signal<void()> scored;
    brac::Signal<void()> aim;
    brac::Signal<void()> shoot;
    brac::Signal<void()> shot;
    brac::Signal<void()> yay;
    brac::Signal<void()> die;
    brac::Signal<void()> aah;
    brac::Signal<void()> tension_start;
    brac::Signal<void()> tension_stop;
    brac::Signal<void()> char_score;
    brac::Signal<void()> pumped;
    brac::Signal<void()> help;
    brac::Signal<void()> pop;
    brac::Signal<void()> alert;
    brac::Signal<void()> fall;
    brac::Signal<void()> failed;
    brac::Signal<void()> lose;
    brac::Signal<void()> dundundun;
    brac::Signal<void()> beep;
    brac::Signal<void()> tick;
    brac::Signal<void()> charblast;
    brac::Signal<void()> boom;
    brac::Signal<void()> bombwhistle_start;
    brac::Signal<void()> bombwhistle_stop;
    brac::Signal<void()> reloading;
    brac::Signal<void()> screech;
    brac::Signal<void()> screech2;
    brac::Signal<void()> yay1;
    brac::Signal<void()> yay2;
    brac::Signal<void()> yay3;
    brac::Signal<void()> yay4;
    brac::Signal<void()> yay5;
    brac::Signal<void()> yay6;
    brac::Signal<void()> yay7;
    brac::Signal<void()> yay8;
    brac::Signal<void()> yay9;
    brac::Signal<void()> aha;
    brac::Signal<void()> comeon;
    brac::Signal<void()> stopyays;

    // Achievement-related events
    brac::Signal<void(size_t n)> n_for_n; // n hoops from n hits
    brac::Signal<void()> sharpshot;

    Game(brac::SpaceTime & st, GameMode mode, float top, std::shared_ptr<brac::TimerImpl> timer);
    ~Game();

    State const & state() const;
    
    virtual brac::TouchHandler fingerTouch(brac::vec2 const & p, float radius) override;
    void gameOver();
    void continueGame();
    void archiveCharacterStats(CharacterStats s);

private:
    struct Members;
    std::unique_ptr<Members> m;

    virtual void doUpdate(float dt) override;
    virtual void getActors(size_t actorId, void * buf) const override;
};

#endif // INCLUDED__Claws__Game_h
