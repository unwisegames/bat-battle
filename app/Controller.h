#ifndef INCLUDED__Claws__Controller_h
#define INCLUDED__Claws__Controller_h

#include "Game.h"

#include <bricabrac/Game/GameController.h>

#include <memory>

class Controller : public brac::GameController {
public:
    Controller();
    ~Controller();

    void newGame(GameMode mode);

private:
    struct Members;
    std::unique_ptr<Members> m;

    virtual bool onUpdate(float dt) override;
    virtual void onDraw() override;
    virtual bool wantTimerUpdate() const override;
    virtual void onResize(brac::vec2 const & size) override;
    virtual brac::TouchHandler onTouch(brac::vec2 const & worldPos, float radius) override;
};

#endif // INCLUDED__Claws__Controller_h
