#ifndef INCLUDED__Claws__GameOver_h
#define INCLUDED__Claws__GameOver_h

#include <bricabrac/Game/GameController.h>
#include "UI.h"
#include "Game.h"
#include "atlas.sprites.h"

#include <memory>

class GameOver : public brac::GameController {
public:
    GameOver(GameMode m, size_t score, size_t best, PlayerStats ps, std::vector<CharacterStats> cs);

    std::shared_ptr<Button> back        = makeButton(atlas.back     , {-5, 0}, 1.1);
    std::shared_ptr<Button> restart     = makeButton(atlas.restart  , {5, 0}, 1.1);
    std::shared_ptr<Button> cont        = makeButton(atlas.play     , {1.7, -0.35}, 0.7);

private:
    size_t score_;
    size_t best_;
    PlayerStats ps_;
    std::vector<CharacterStats> cs_;
    int mom_ = 0;
    std::string formatted_time_ = "";

    virtual bool onUpdate(float dt) override;
    virtual void onDraw() override;
    virtual void onResize(brac::vec2 const & size) override;
    virtual brac::TouchHandler onTouch(brac::vec2 const & worldPos, float radius) override;

    void awardManOfMatch();
    void formatTime();
};

#endif // INCLUDED__Claws__GameOver_h
