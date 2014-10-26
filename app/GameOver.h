#ifndef INCLUDED__Claws__GameOver_h
#define INCLUDED__Claws__GameOver_h

#include <bricabrac/Game/GameController.h>
#include "UI.h"
#include "Game.h"
#include "atlas.sprites.h"

#include <memory>

class GameOver : public brac::GameController {
public:
    GameOver(GameMode m, int level, bool passed, size_t score, size_t best, PlayerStats ps, std::vector<CharacterStats> cs);

    std::shared_ptr<Button> back        = makeButton(atlas.back     , {-5, -0.8}, 1.1);
    std::shared_ptr<Button> restart     = makeButton(atlas.restart  , {-5, 0.8}, 1.1);
    std::shared_ptr<Button> next        = makeButton(atlas.play     , {5,  0}, 0.58);
    std::shared_ptr<Button> backf       = makeButton(atlas.back     , {-1, -1}, 1.1);
    std::shared_ptr<Button> restartf    = makeButton(atlas.restart  , {1, -1}, 1.1);

private:
    size_t score_;
    size_t best_;
    bool passed_;
    int level_;
    PlayerStats ps_;
    std::vector<CharacterStats> cs_;
    int mom_ = 0;
    std::string formatted_time_ = "";

    virtual bool onUpdate(float dt) override;
    virtual void onDraw() override;
    virtual void onResize(brac::vec2 const & size) override;
    virtual std::unique_ptr<brac::TouchHandler> onTouch(brac::vec2 const & worldPos, float radius) override;

    void awardManOfMatch();
    void formatTime();
};

#endif // INCLUDED__Claws__GameOver_h
