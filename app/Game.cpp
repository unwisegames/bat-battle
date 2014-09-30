#include "Game.h"
#include "bats.sprites.h"
#include "characters.sprites.h"
#include "character.sprites.h"

#include <bricabrac/Game/GameActorImpl.h>
#include <bricabrac/Game/Timer.h>
#include <bricabrac/Math/MathUtil.h>
#include <bricabrac/Math/Random.h>
#include <bricabrac/Logging/Logging.h>

using namespace brac;

enum Group : cpGroup { gr_bird = 1 };

enum Layer : cpLayers { l_all = 1<<0, l_character = 1<<1 };

enum CollisionType : cpCollisionType { ct_universe = 1, ct_ground };

struct CharacterImpl : BodyShapes<Character> {
    vec2 launchVel_ = {0, 0};
    ShapePtr shape;
    bool isCaptive = false;

    CharacterImpl(cpSpace * space, int type, vec2 const & pos)
    : BodyShapes{space, newBody(1, INFINITY, pos), sensor(character.character), CP_NO_GROUP, l_all | l_character}
    {
        for (auto & shape : shapes()) cpShapeSetElasticity(&*shape, 1);

        shape = newCircleShape(0.3, {0, 0})(body());
    }

    virtual bool isAiming() const override {
        return !!launchVel_ && state() == Character::State::aim;;
    }

    virtual vec2 const & launchVel() const override {
        return launchVel_;
    }

    void aim(vec2 const & v) {
        launchVel_ = v;
        setAngle(brac::angle(v) - M_PI_2);
    }

    void dontAim() {
        launchVel_ = {0, 0};
        setAngle(0);
        reload();
    }

    void newFrame(bool loopChanged) override {
        if (!loopChanged && frame() == 0) {
            if (state() == Character::State::reloading) {
                *this << Character::State::ready;
            }
            if (state() == Character::State::shooting) {
                dontAim();
                *this << Character::State::reloading;
            }
        }
    }

    void initState() {
        Character::State s [] = { biggrin, smile, smug, exclaim };
        *this << s[rand<int>(0, 3)];
    }

    void reload() {
        setVel({0, 0});
        *this << Character::State::reloading;
    }

    bool readyToFire() {
        return state() == Character::State::ready;
    }

    void shoot() {
        *this << Character::State::shooting;
    }

    bool canBeKidnapped() {
        return !isCaptive && !(state() == Character::State::rescued);
    }

    void kidnapped() {
        if (isAiming()) {
            dontAim();
        }
        isCaptive = true;
        *this << Character::State::yell;
        delay(1, [=]{ *this << Character::State::crying; }).cancel(destroyed);
    }

    void rescued() {
        isCaptive = false;
        *this << Character::State::rescued;
    }

    void startle() {
        *this << Character::State::startled;
        setVel({0, 1.5});
    }
};

struct BirdImpl : BodyShapes<Bird> {
    bool hasCaptive = false;
    vec2 escapeVel;

    BirdImpl(cpSpace * space, int type, vec2 const & pos, vec2 const & vel, vec2 const & tar)
    : BodyShapes{space, newBody(1, 1, pos), bats.bats[type], gr_bird, l_all}
    {
        setForce({0, -WORLD_GRAVITY});
        setVel(to_vec2(cpvnormalize(to_cpVect(tar - pos))));
    }

    void newState(size_t & loop) override {
        loop = size_t(state());
    }

    virtual void doUpdate(float) override {
        if (isFlying()) {
            cpVect v = cpvnormalize(to_cpVect(vel()));
            if (v.x > -0.5 && v.x < 0.5) {
                *this << Bird::State::front;
            } else {
                *this << Bird::State::side;
            }
            if (hasCaptive) {
                // maintain velocity
                setVel(escapeVel);
                setAngle(0);
            }
        }
    }

    virtual bool isFlying() const override {
        return !(state() == Bird::State::dying || state() == Bird::State::puff);
    }

    bool canGrabCharacter() {
        return isFlying() && !hasCaptive;
    }

    bool canBeShot() {
        return isFlying();
    }

    ConstraintPtr * grabCharacter(cpBody & b) {
//        setForce({0, -WORLD_GRAVITY*2});
        static ConstraintPtr joints[2];

        joints[0] = newPinJoint(body(), &b, {-0.2, 0}, {0, 0});
        joints[1] = newPinJoint(body(), &b, {0.2, 0}, {0, 0});
        cpPinJointSetDist(&*joints[0], 0.6);
        cpPinJointSetDist(&*joints[1], 0.6);

        return joints;
    }

    void dropCharacter() {
//        setForce({0, -WORLD_GRAVITY});
    }
};

struct DartImpl : BodyShapes<Dart> {
    bool active;
    ConstraintPtr p;

    DartImpl(cpSpace * space, vec2 const & pos, vec2 const & vel)
    : BodyShapes{space, newBody(1, 1, pos), characters.dart, CP_NO_GROUP, l_all}
    {
        setVel(vel);
        active = true;
    }

    virtual void doUpdate(float) override {
        if (active) {
            setAngle(::brac::angle(vel()));
        }
    }

    void attach(cpBody & b) { }
};

struct CharacterJointBird {
    CharacterImpl * c;
    ConstraintPtr p_[2];
    BirdImpl * b;

    CharacterJointBird(CharacterImpl & c, ConstraintPtr p[2], BirdImpl & b) : c(&c), b(&b) {
        p_[0] = std::move(p[0]);
        p_[1] = std::move(p[1]);
    }
};

struct Game::Members : Game::State, GameImpl<CharacterImpl, BirdImpl, DartImpl> {
    ShapePtr worldBox{sensor(boxShape(20, 30, {0, 0}, 0), ct_universe)};
    ShapePtr ground{segmentShape({-10, 2}, {10, 2})};
    ShapePtr walls[3], hoop[2];
    size_t n_for_n = 0;
    bool touched_sides = false;
    int bounced_walls = 0;
    size_t score_modifier = 0;
    std::unique_ptr<ExpTicker> tick;
    std::vector<CharacterJointBird> cjb;

    Members(SpaceTime & st) : Impl{st} { }
};

Game::Game(SpaceTime & st, GameMode mode, float top) : GameBase{st}, m{new Members{st}} {
    m->mode = mode;

    if (mode == m_menu) {
        delay(0, [=]{ show_menu(); }).cancel(destroyed);
    } else {
        m->setGravity({0, WORLD_GRAVITY});

        cpShapeSetCollisionType(&*m->ground, ct_ground);

        auto createCharacters = [=]{
            float min = -10;
            for (int i = 0; i < CHARACTERS; ++i) {
                float max = min + (20 / CHARACTERS);
                vec2 v = {rand<float>(min + 0.5, max - 0.5), 2.3};
                m->emplace<CharacterImpl>(0, v);
                min = max;
            }
            for (auto & c : m->actors<CharacterImpl>()) c.initState();
        };

        auto createBird = [=]{
            auto pos = vec2{rand<float>(-10, 10), rand<float>(top, top - 2)};
            vec2 target{0, 0};

            auto t = rand<int>(0, m->actors<CharacterImpl>().size() - 1);
            int i = 0;
            for (auto & c : m->actors<CharacterImpl>()) {
                if (i == t) {
                    target = c.pos();
                }
                ++i;
            }
            m->emplace<BirdImpl>(0, pos, vec2{1, -1}, target);
        };

        m->back->setY(top - 0.8);
        m->restart->setY(top - 0.8);

        createCharacters();
        delay(3, [=]{
            for (auto & c : m->actors<CharacterImpl>()) c.startle();
        }).cancel(destroyed);

        m->tick.reset(new ExpTicker{BIRDFREQUENCY, [=]{
            createBird();
        }});
    }

    m->onCollision([=](CharacterImpl & character, BirdImpl & bird, cpArbiter * arb) {
        if (bird.canGrabCharacter() && character.canBeKidnapped()) {
            m->cjb.emplace_back(CharacterJointBird{character, bird.grabCharacter(*character.body()), bird});

            character.kidnapped();
            bird.hasCaptive = true;
            vec2 v = bird.vel();
            v.y = -v.y;
            bird.escapeVel = to_vec2(cpvnormalize(to_cpVect((v))));
            for (auto & shape : character.shapes()) cpShapeSetGroup(&*shape, gr_bird);
        } else {
            return false;
        }
        return true;
    });

    m->onCollision([=](DartImpl &, BirdImpl & bird, cpArbiter * arb) {
        if (bird.canBeShot()) {
            if(cpArbiterIsFirstContact(arb)) {
                m->score += 10;
            }
        } else {
            return false;
        }
        return true;
    });

    m->onPostSolve([=](DartImpl & dart, BirdImpl & bird, cpArbiter * arb) {
        bird << Bird::State::dying;
        if (bird.hasCaptive) {
            std::vector<CharacterJointBird>::const_iterator it;
            for(it = m->cjb.begin(); it != m->cjb.end(); ++it)
            {
                if ((it)->b == &bird) {
                    (it)->b->dropCharacter();
                    (it)->c->rescued();
                    m->cjb.erase(it);
                    break;
                }
            }
        }
        bird.setAngle(0);
        bird.setVel(vec2{0, -3});
        cpBodySetAngVel(bird.body(), 0);
        dart.setVel({0, 0});
        m->removeWhenSpaceUnlocked(dart);
    });

    m->onCollision([=](DartImpl & dart, NoActor<ct_ground> &, cpArbiter *) {
        dart.active = false;
        float angle = brac::angle(dart.vel());
        m->whenSpaceUnlocked([&] {
            cpSpaceRemoveBody(dart.space(), dart.body());
            cpSpaceConvertBodyToStatic(dart.space(), dart.body());
            dart.setAngle(angle);
        }, &dart);
    });

    m->onCollision([=](DartImpl & dart, CharacterImpl & character, cpArbiter * arb) {
        character << Character::State::dead;
        m->removeWhenSpaceUnlocked(dart);
        m->whenSpaceUnlocked([&] {
            cpSpaceRemoveBody(character.space(), character.body());
            cpSpaceConvertBodyToStatic(character.space(), character.body());
        }, &dart);
        return false;
    });

    m->onPostSolve([=](CharacterImpl & character, NoActor<ct_ground> &, cpArbiter *) {
        if (character.state() == Character::State::startled) {
            character << Character::State::determined;
            delay(rand<float>(0.1, 0.8), [&] {
                character << Character::State::reloading;
            }).cancel(destroyed);
        } else if (character.state() == Character::State::rescued) {
            character.reload();
        }
    });

    m->onCollision([=](BirdImpl & bird, NoActor<ct_ground> &, cpArbiter *) {
        if (bird.state() == Bird::State::dying) {
            bird << Bird::State::puff;
            bird.setVel({0, 0});
            delay(0.85, [&]{ m->removeWhenSpaceUnlocked(bird); }).cancel(destroyed);
        } else {
            return false;
        }
        return true;
    });

    m->onSeparate([=](CharacterImpl & character, NoActor<ct_universe> &) {
        // This is broken
        size_t size = m->actors<CharacterImpl>().size();
        m->removeWhenSpaceUnlocked(character);
        if (size == 1) {
            gameOver();
        }
    });

    m->onSeparate([=](BirdImpl & bird, NoActor<ct_universe> &) {
        //m->removeWhenSpaceUnlocked(bird);
    });
}

void Game::gameOver() {
    m->tick.reset();
    end();
}

Game::~Game() { }

Game::State const & Game::state() const { return *m; }

std::unique_ptr<TouchHandler> Game::fingerTouch(vec2 const & p, float radius) {
    if (auto backHandler = m->back->handleTouch(p)) {
        return backHandler;
    }
    if (auto restartHandler = m->restart->handleTouch(p)) {
        return restartHandler;
    }

    CharacterImpl * character = nullptr;
    float dist_sq = INFINITY;
    AabbQuery(&m->spaceTime, cpBBNewForCircle(to_cpVect(p), radius), l_character, CP_NO_GROUP,
              [&](cpShape *shape) {
                  if (auto c = static_cast<CharacterImpl *>(cpShapeGetUserData(shape))) {
                      float dsq = length_sq(p - c->pos());
                      if (dist_sq > dsq) {
                          dist_sq = dsq;
                          character = c;
                      }
                  }
              });

    if (character && character->readyToFire()) {
        struct CharacterAimAndFireHandler : TouchHandler {
            std::weak_ptr<Game> weak_self;
            CharacterImpl * character;
            vec2 first_p, vel;

            CharacterAimAndFireHandler(Game & self, vec2 const & p, CharacterImpl * character)
            : weak_self{self.shared_from_this()}
            , character{character}
            , first_p{p}
            {
                *character << Character::State::aim;
            }

            ~CharacterAimAndFireHandler() {
                if (auto self = weak_self.lock()) {
                    // TODO: Return smoothly to upright posture.
                    if (character->isAiming()) {
                        if (vec2 const & vel = character->launchVel()) {
                            self->m->emplace<DartImpl>(character->pos() + LAUNCH_OFFSET * unit(vel), vel);
                            character->shoot();
                        }
                    }
                }
            }

            virtual void moved(vec2 const & p, bool) {
                if (auto self = weak_self.lock()) {
                    auto v = first_p - p;
                    if (float s = length(v)) {
                        character->aim((14 + 2 * s) / s * v);
                    }
                }
            }
        };
        
        return std::unique_ptr<TouchHandler>{new CharacterAimAndFireHandler{*this, p, character}};
    }

    return {};
}

void Game::doUpdate(float dt) { m->update(dt); }

void Game::getActors(size_t actorId, void * buf) const { m->getActorsForController(actorId, buf); }
