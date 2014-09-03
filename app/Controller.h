#ifndef INCLUDED__Bounce__Controller_h
#define INCLUDED__Bounce__Controller_h

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
    virtual void onResize(brac::vec2 const & size) override;
    virtual std::unique_ptr<brac::TouchHandler> onTouch(brac::vec2 const & worldPos, float radius) override;
};

#endif // INCLUDED__Bounce__Controller_h
