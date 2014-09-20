#ifndef INCLUDED__Claws__GameOver_h
#define INCLUDED__Claws__GameOver_h

#include <bricabrac/Game/GameController.h>
#include "UI.h"
#include "Game.h"
#include "atlas.sprites.h"

#include <memory>

class GameOver : public brac::GameController {
public:
    GameOver(GameMode m, size_t score, size_t best);

    std::shared_ptr<Button> restart = makeButton(atlas.restart    , {1, -2.2}, 1.1);
    std::shared_ptr<Button> back    = makeButton(atlas.back       , {-1, -2.2}, 1.1);

private:
    size_t score_;
    size_t best_;

    virtual bool onUpdate(float dt) override;
    virtual void onDraw() override;
    virtual void onResize(brac::vec2 const & size) override;
    virtual std::unique_ptr<brac::TouchHandler> onTouch(brac::vec2 const & worldPos, float radius) override;
};

#endif // INCLUDED__Claws__GameOver_h
