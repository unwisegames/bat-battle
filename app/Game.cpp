#include "Game.h"
#include "bats.sprites.h"
#include "characters.sprites.h"
#include "character.sprites.h"

#include <bricabrac/Game/GameActorImpl.h>
#include <bricabrac/Game/Timer.h>
#include <bricabrac/Math/MathUtil.h>
#include <bricabrac/Math/Random.h>
#include <bricabrac/Logging/Logging.h>
#include <bricabrac/Data/Relation.h>

#include <unordered_set>

using namespace brac;
using namespace cpplinq;

enum Group : cpGroup { gr_bird = 1 };

enum Layer : cpLayers { l_all = 1<<0, l_character = 1<<1 };

enum CollisionType : cpCollisionType { ct_universe = 1, ct_abyss, ct_ground, ct_attack };

struct CharacterImpl : BodyShapes<Character> {
    vec2 launchVel_ = {0, 0};
    ShapePtr shape;

    CharacterImpl(cpSpace * space, int type, vec2 const & pos)
    : BodyShapes{space, newBody(1, INFINITY, pos), sensor(character.character), CP_NO_GROUP, l_all | l_character}
    {
        for (auto & shape : shapes()) {
            cpShapeSetElasticity(&*shape, 1);
        }

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
            switch (state()) {
                case Character::State::reloading:
                    setState(Character::State::ready);
                    break;
                case Character::State::shooting:
                    dontAim();
                    setState(Character::State::reloading);
                    break;
                default:
                    break;
            }
        }
    }

    void initState() {
        setState(randomChoice({biggrin, smile, smug, exclaim}));
    }

    void reload() {
        setVel({0, 0});
        setState(Character::State::reloading);
    }

    bool readyToFire() {
        return state() == Character::State::ready;
    }

    void shoot() {
        setState(Character::State::shooting);
    }

    void kidnap() {
        if (isAiming()) {
            dontAim();
        }
        if (state() != Character::State::dead) {
            setState(Character::State::yell);
            delay(1, [=]{ setState(Character::State::crying); }).cancel(destroyed);
        }
    }

    void rescue() {
        if (state() != Character::State::dead) {
            setState(Character::State::rescued);
        }
    }

    void startle() {
        setState(Character::State::startled);
        setVel({0, 1.5});
    }
};

struct BirdImpl : BodyShapes<Bird> {
    bool hasCaptive = false;
    vec2 escapeVel;

    BirdImpl(cpSpace * space, int type, vec2 const & pos)
    : BodyShapes{space, newBody(1, 1, pos), bats.bats[type], gr_bird, l_all}
    {
        setForce({0, -WORLD_GRAVITY});
    }

    void newTarget(vec2 targetPos) {
        setVel(to_vec2(cpvnormalize(to_cpVect(targetPos - pos()))));
    }

    void newState(size_t & loop) override {
        loop = size_t(state());
    }

    virtual void doUpdate(float) override {
        if (isFlying()) {
            cpVect v = cpvnormalize(to_cpVect(vel()));
            if (v.x > -0.5 && v.x < 0.5) {
                setState(Bird::State::front);
            } else {
                setState(Bird::State::side);
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

    array<ConstraintPtr, 2> grabCharacter(cpBody & b) {
        auto joint = [&](vec2 const & pos) {
            auto j = newPinJoint(body(), &b, pos, {0, 0});
            cpPinJointSetDist(&*j, 0.6);
            return j;
        };
        hasCaptive = true;
        return {joint({-0.2, 0}), joint({0.2, 0})};
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
    array<ConstraintPtr, 2> p;
    BirdImpl * b;

    bool operator==(CharacterJointBird const & cjb) const { return c == cjb.c && p == cjb.p && b == cjb.b; }
    size_t hash() const { return hash_of(c, p[0], p[1], b); }
};

struct BirdTargetCharacter {
    BirdImpl * b;
    CharacterImpl * c;

    bool operator==(BirdTargetCharacter const & targets) const { return b == targets.b && c == targets.c; }
    size_t hash() const { return hash_of(b, c); }
};

struct Game::Members : Game::State, GameImpl<CharacterImpl, BirdImpl, DartImpl> {
    ShapePtr worldBox{sensor(boxShape(20, 30, {0, 0}, 0), ct_universe)};
    ShapePtr abyssWalls[3];
    ShapePtr ground{segmentShape({-10, 2}, {10, 2})};
    ShapePtr attackLine{sensor(segmentShape({-10, ATTACK_LINE_Y}, {10, ATTACK_LINE_Y}), ct_attack)};
    size_t created_birds = 0;
    std::unique_ptr<Ticker> tick;
    Relation<CharacterJointBird> cjb;
    Relation<BirdTargetCharacter> targets;

    Members(SpaceTime & st) : Impl{st} { }

    bool isKidnappable(CharacterImpl const & c) {
        return (c.state() != Character::State::rescued &&
                !(from(cjb) >> any([&](CharacterJointBird const & cjb) { return cjb.c == &c; })));
    }

    bool isBeingTargeted(CharacterImpl const & c) {
        return (from(targets) >> any([&](BirdTargetCharacter const & t) { return t.c == &c; }));
    }

    bool hasCaptive(BirdImpl const & b) {
        return (from(cjb) >> any([&](CharacterJointBird const & cjb) { return cjb.b == &b; }));
    }
};

Game::Game(SpaceTime & st, GameMode mode, float top) : GameBase{st}, m{new Members{st}} {
    m->mode = mode;

    auto newTarget = [=](BirdImpl & b) {
        m->targets >> removeIf([&](BirdTargetCharacter const & target) { return target.b == &b; });

        auto available = (from(m->actors<CharacterImpl>())
                          >> mutable_ref()
                          >> where([&](CharacterImpl const & c) { return m->isKidnappable(c); }));

        if (available >> any()) {
            auto not_targeted = available >> where([&](CharacterImpl const & c) { return !m->isBeingTargeted(c); });
            if (not_targeted >> any()) {
                // get first untargeted character
                auto & c = (not_targeted >> first()).get();
                m->targets.insert(BirdTargetCharacter{&b, &c});
                b.newTarget(c.pos());
            } else {
                // otherwise pick any available target at random
                int i = 1; auto r = rand<int>(1, int(available >> count()));
                available >> for_each([&](CharacterImpl & c) {
                    if (i == r) {
                        m->targets.insert(BirdTargetCharacter{&b, &c});
                        b.newTarget(c.pos());
                    }
                    ++i;
                });
            }
        } else {
            // TODO: Fly around naturally, waiting for available character (actually, end of game?)
            b.setVel({0, 0});
        }
    };

    if (mode == m_menu) {
        delay(0, [=]{ show_menu(); }).cancel(destroyed);
    } else {
        m->setGravity({0, WORLD_GRAVITY});
        auto wall = [=] (vec2 v1, vec2 v2) { return m->sensor(m->segmentShape(v1, v2), ct_abyss); };
        m->abyssWalls[0] = wall({-10, top}, {-10, -10});  // left
        m->abyssWalls[1] = wall({10,  top}, {10,  -10});  // right
        m->abyssWalls[2] = wall({-10, top}, {10,  top});  // top
        m->rem_birds = BIRDS;
        m->rem_chars = CHARACTERS;
        m->back->setY(top - 0.8);
        m->restart->setY(top - 0.8);

        cpShapeSetCollisionType(&*m->ground, ct_ground);

        auto createCharacters = [=]{
            float min = -10;
            for (int i = 0; i < CHARACTERS; ++i) {
                float max = min + (20 / CHARACTERS);
                vec2 v{rand<float>(min + 0.5, max - 0.5), 2.3};
                m->emplace<CharacterImpl>(0, v);
                min = max;
            }
            for (auto & c : m->actors<CharacterImpl>()) c.initState();
        };

        auto createBird = [=]{
            auto & b = m->emplace<BirdImpl>(0, vec2{rand<float>(-10, 10), rand<float>(top, top - 2)});
            newTarget(b);
        };

        createCharacters();
        delay(3, [=]{
            for (auto & c : m->actors<CharacterImpl>()) c.startle();
        }).cancel(destroyed);

        // create birds
        m->tick.reset(new Ticker{BIRDFREQUENCY, [=]{
            delay(rand<float>(0, 1), [&]{
                if (m->created_birds < int(BIRDS)) {
                    if (from(m->actors<CharacterImpl>()) >> any([&](CharacterImpl const & c) { return m->isKidnappable(c); })) {
                        createBird();
                        ++m->created_birds;
                    }
                } else {
                //    m->tick.reset();
                }
            }).cancel(destroyed);
        }});
    }

    m->onCollision([=](CharacterImpl & character, BirdImpl & bird, cpArbiter * arb) {
        if (!(bird.canGrabCharacter() && m->isKidnappable(character) && cpArbiterIsFirstContact(arb))) {
            return false;
        }

        if (!(from(m->cjb) >> any([&](CharacterJointBird const & cjb) { return cjb.b == &bird; }))) {
            vec2 v = bird.vel();

            m->cjb.insert(CharacterJointBird{&character, bird.grabCharacter(*character.body()), &bird});
            m->targets >> removeIf([&](BirdTargetCharacter const & target) { return target.b == &bird; });

            character.kidnap();
            v.y = -v.y;
            bird.escapeVel = to_vec2(cpvnormalize(to_cpVect((v))));
            for (auto & shape : character.shapes()) cpShapeSetGroup(&*shape, gr_bird);

            // send birds after new target
            from(m->targets) >> for_each([&](BirdTargetCharacter const & targets) {
                if (targets.c == &character && targets.b != &bird && targets.b->isFlying()) {
                    if (targets.b->pos().y > ATTACK_LINE_Y) {
                        newTarget(*targets.b);
                    } else {
                        // flying too low to target new character
                        m->targets >> removeIf([&](BirdTargetCharacter const & target) { return target.b == targets.b; });
                        vec2 atp{rand<float>(-6, 6), ATTACK_LINE_Y};
                        targets.b->setVel(to_vec2(cpvnormalize(to_cpVect(atp - targets.b->pos()))));
                    }
                }
            });
        }
        return true;
    });

    m->onCollision([=](DartImpl &, BirdImpl & bird, cpArbiter * arb) {
        if (!bird.canBeShot()) {
            return false;
        }

        if(cpArbiterIsFirstContact(arb)) {
            m->score += 10;
        }
        return true;
    });

    m->onPostSolve([=](DartImpl & dart, BirdImpl & bird, cpArbiter * arb) {
        if (dart.active) {
            m->targets >> removeIf([&](BirdTargetCharacter const & target) { return target.b == &bird; });
            bird.setState(Bird::State::dying);
            auto matching = from(m->cjb) >> ref() >> where([&](CharacterJointBird const & cjb) { return cjb.b == &bird; });
            if (matching >> any()) {
                auto & cjb = (matching >> first()).get();
                cjb.b->dropCharacter();
                cjb.c->rescue();
                m->cjb.erase(cjb);
            }
            bird.setAngle(0);
            bird.setVel(vec2{0, -3});
            cpBodySetAngVel(bird.body(), 0);
            dart.setVel({0, 0});
            m->removeWhenSpaceUnlocked(dart);
            --m->rem_birds;

            if (m->rem_birds == 0) {
                // TODO: celebrate
                delay(2, [=]{ gameOver(); }).cancel(destroyed);
            }
        }
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
        if (dart.active) {
            character.setState(Character::State::dead);
            m->removeWhenSpaceUnlocked(dart);
        }
        return false;
    });

    m->onPostSolve([=](CharacterImpl & character, NoActor<ct_ground> &, cpArbiter *) {
        switch (character.state()) {
            case Character::State::startled:
                character.setVel({0, 0});
                character.setState(Character::State::determined);
                delay(rand<float>(0.1, 0.8), [&] {
                    character.setState(Character::State::reloading);
                }).cancel(destroyed);
                break;
            case Character::State::rescued:
                character.reload();
                break;
            case Character::State::dead:
                if (!(from(m->cjb) >> any([&](CharacterJointBird const & cjb) { return cjb.c == &character; }))) {
                    character.setVel({0, 0});
                }
                break;
            default:
                break;
        }
    });

    m->onCollision([=](BirdImpl & bird, NoActor<ct_ground> &, cpArbiter *) {
        if (bird.state() != Bird::State::dying) {
            return false;
        }

        bird.setState(Bird::State::puff);
        bird.setVel({0, 0});
        delay(0.85, [&]{ m->removeWhenSpaceUnlocked(bird); }).cancel(destroyed);
        return true;
    });

    m->onCollision([=](BirdImpl & bird, NoActor<ct_attack> &) {
        if (!(from(m->targets) >> any([&](BirdTargetCharacter const & t) { return t.b == &bird; })) &&
            !m->hasCaptive(bird) &&
            bird.state() != Bird::State::dying) {
            newTarget(bird);
        }
    });

    /*m->onSeparate([=](CharacterImpl & character, NoActor<ct_universe> &) {
        // This is broken
        size_t size = m->actors<CharacterImpl>().size();
        //m->removeWhenSpaceUnlocked(character);
        if (size == 1) {
            gameOver();
        }
    });*/

    m->onSeparate([=](BirdImpl & bird, NoActor<ct_abyss> &) {
        auto matching = from(m->cjb) >> ref() >> where([&](CharacterJointBird const & cjb) { return cjb.b == &bird; });
        if (matching >> any()) {
            auto & cjb = (matching >> first()).get();
            m->cjb.erase(cjb);
            //m->removeWhenSpaceUnlockedIf(from (m->actors<CharacterImpl>()) >> where([&](CharacterImpl const & c) { return &c == cjb.c; }));
            for (auto & c : m->actors<CharacterImpl>()) {
                if (&c == cjb.c) {
                    m->removeWhenSpaceUnlocked(c);
                }
            }
            m->targets >> removeIf([&](BirdTargetCharacter const & target) { return target.b == &bird; });
            m->removeWhenSpaceUnlocked(bird);
            --m->created_birds; // bird will be replaced
            --m->rem_chars;

            if (m->rem_chars == 0) {
                delay(2, [=]{ gameOver(); }).cancel(destroyed);
            }
        }
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

        character->setState(Character::State::aim);
        return std::unique_ptr<TouchHandler>{new CharacterAimAndFireHandler{*this, p, character}};
    }
    
    return {};
}

void Game::doUpdate(float dt) { m->update(dt); }

void Game::getActors(size_t actorId, void * buf) const { m->getActorsForController(actorId, buf); }
