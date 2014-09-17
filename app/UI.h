#ifndef INCLUDED__Bounce__UI_h
#define INCLUDED__Bounce__UI_h

#include "atlas.sprites.h"

#include <bricabrac/Game/GameController.h>
#include <bricabrac/Game/GameActor.h>

#include <memory>

class Button : public std::enable_shared_from_this<Button> {
public:
    struct TouchHandler : brac::TouchHandler {
        std::weak_ptr<Button> weak_btn;

        TouchHandler(std::shared_ptr<Button> const & btn) : weak_btn(btn) {
            btn->pressed = true;
        }

        virtual void moved(brac::vec2 const & p, bool) {
            if (auto btn = weak_btn.lock()) {
                btn->pressed = btn->contains(p);
            }
        }

        virtual void ended() /*override*/ {
            if (auto btn = weak_btn.lock()) {
                if (btn->pressed) {
                    btn->pressed = false;
                    btn->clicked();
                }
            }
        }
    };

    Button(brac::SpriteDef const s[], brac::vec2 p) : sprites_{s[0], s[1]}, pos_(p) { }

    void draw(brac::mat4 pmv);
    bool contains(brac::vec2 v);
    void setY(float y) { pos_.y = y; };

    bool pressed = false;

    brac::Signal<void()> clicked;

    std::unique_ptr<brac::TouchHandler> handleTouch(brac::vec2 const & p) {
        if (contains(p)) {
            return std::unique_ptr<brac::TouchHandler>{new TouchHandler{shared_from_this()}};
        }
        return {};
    }

    static std::unique_ptr<brac::TouchHandler> handleTouch(brac::vec2 const & p, std::initializer_list<std::shared_ptr<Button>> buttons) {
        for (auto const & button : buttons) {
            if (auto handler = button->handleTouch(p)) {
                return handler;
            }
        }
        return {};
    }
    
private:
    brac::SpriteDef sprites_[2]; // default/pressed
    brac::vec2 pos_;
};

inline std::shared_ptr<Button> makeButton(brac::SpriteDef const s[], brac::vec2 p) {
    return std::make_shared<Button>(s, p);
}

#endif // INCLUDED__Bounce__UI_h
