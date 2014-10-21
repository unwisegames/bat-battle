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

    std::shared_ptr<Button> back    = makeButton(atlas.back       , {-5, 0}, 1.1);
    std::shared_ptr<Button> restart = makeButton(atlas.restart    , {0,  -2.2}, 1.1);
    std::shared_ptr<Button> next    = makeButton(atlas.play       , {5,  0}, 0.58);

private:
    size_t score_;
    size_t best_;
    bool passed_;
    int level_;

    virtual bool onUpdate(float dt) override;
    virtual void onDraw() override;
    virtual void onResize(brac::vec2 const & size) override;
    virtual std::unique_ptr<brac::TouchHandler> onTouch(brac::vec2 const & worldPos, float radius) override;
};

#endif // INCLUDED__Claws__GameOver_h
