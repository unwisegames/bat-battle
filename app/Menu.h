#ifndef INCLUDED__Claws__Menu_h
#define INCLUDED__Claws__Menu_h

#include <bricabrac/Game/GameController.h>
#include "Game.h"
#include "UI.h"
#include "atlas.sprites.h"
#include "atlas2.sprites.h"

#include <memory>

class Menu : public brac::GameController {
public:
    bool newGame = false;
    GameMode mode = m_menu;

    std::shared_ptr<Button> play        = makeButton(atlas.play,        { 0, -1.5 });
    std::shared_ptr<Button> gamecenter  = makeButton(atlas2.gamecenter, { -2.5, -1.5 });
    std::shared_ptr<Button> twitter     = makeButton(atlas2.twitter,    { 2.5, -1.5 });

    Menu();

private:
    virtual bool onUpdate(float dt) override;
    virtual void onDraw() override;
    virtual void onResize(brac::vec2 const & size) override;
    virtual std::unique_ptr<brac::TouchHandler> onTouch(brac::vec2 const & worldPos, float radius) override;
};

#endif // INCLUDED__Claws__Menu_h
