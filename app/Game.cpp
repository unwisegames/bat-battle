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

enum Group : cpGroup { gr_bird = 1, gr_character = 2 };

enum Layer : cpLayers { l_all = 1<<0, l_character = 1<<1, l_halo = 1<<2, l_play = 1<<3 };

enum CollisionType : cpCollisionType { ct_universe = 1, ct_abyss, ct_ground, ct_attack, ct_startle };

struct PersonalSpaceImpl : BodyShapes<PersonalSpace> {
    PersonalSpaceImpl(cpSpace * space, vec2 const pos)
    : BodyShapes{space, newBody(1, INFINITY, pos), newCircleShape(0.8), CP_NO_GROUP, l_halo}
    {
        for (auto & shape : shapes()) {
            cpShapeSetElasticity(&*shape, 0);
        }
    }

    ConstraintPtr attachCharacterBody(cpBody & b) {
        //cpPinJointSetDist(&*j, 0.2);
        //auto j = newPinJoint(body(), &b, {0, 0}, {0, 0});
        auto j = newPivotJoint(body(), &b, pos());
        return j;
    }
};

struct CharacterImpl : BodyShapes<Character> {
    vec2 launchVel_ = {0, 0};
    ShapePtr shape;

    CharacterStats stats;

    CharacterImpl(cpSpace * space, int type, vec2 const & pos)
    : BodyShapes{space, newBody(1, INFINITY, pos), sensor(character.character), gr_character, l_play | l_character}
    {
        for (auto & shape : shapes()) {
            cpShapeSetElasticity(&*shape, 1);
        }

        shape = newCircleShape(0.3, {0, 0})(body());
        cpShapeSetFriction(&*shape, 0.2);
        cpShapeSetLayers(&*shape, l_play);
        cpShapeSetGroup(&*shape, gr_character);

        stats.mugshot = character.mugshot;
    }

    virtual bool isAiming() const override {
        return !!launchVel_ && state() == Character::State::aim;
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
                    setVel({0, 2});
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
        ++stats.dartsFired;
        setState(Character::State::shooting);
    }

    void kidnap() {
        ++stats.kidnapped;
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

    void celebrate() {
        if (state() != Character::State::dead) {
            setAngle(0);
            setState(Character::State::celebrating);
            setVel({0, rand<float>(3, 4.5)});
        }
    }

    void startle() {
        setVel({0, 3});
        setState(Character::State::startled);
    }
};

constexpr float F = 1;
struct BirdImpl : BodyShapes<Bird> {
    bool hasCaptive = false;
    bool fromWhence = false;
    vec2 escapeVel;
    vec2 desired_pos;

    BirdImpl(cpSpace * space, int type, vec2 const & pos)
    : BodyShapes{space, newBody(1, 1, pos), bats.bats[type], gr_bird, l_play}
    {
        setForce({0, -WORLD_GRAVITY});
    }

    void newState(size_t & loop) override {
        loop = size_t(state());
    }

    virtual void doUpdate(float) override {
        if (isFlying()) {
            auto v = unit(vel());
            if (fromWhence && v.y > 0) {
                setState(Bird::State::rear);
            } else {
                if (v.x > -0.5 && v.x < 0.5) {
                    setState(Bird::State::front);
                } else {
                    setState(Bird::State::side);
                }
            }

            if (isFlying() && !hasCaptive) {
                auto dv = unit(desired_pos - pos()) * BIRD_SPEED - vel();
                float epsilon = 0.01;
                setForce(((length_sq(dv) > epsilon) * F * unit(dv)) + vec2{0, -WORLD_GRAVITY});
            } else {
                // maintain velocity
                setVel(escapeVel * BIRD_SPEED);
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

    void setDesiredPos(vec2 dp) {
        desired_pos = dp;
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
    : BodyShapes{space, newBody(1, 1, pos), characters.dart, CP_NO_GROUP, l_play}
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

struct CharacterShotDart {
    CharacterImpl * c;
    DartImpl * d;

    bool operator==(CharacterShotDart const & csd) const { return c == csd.c && d == csd.d; }
    size_t hash() const { return hash_of(c, d); }
};

struct CharacterPersonalSpace {
    CharacterImpl * c;
    ConstraintPtr p;
    PersonalSpaceImpl * ps;

    bool operator==(CharacterPersonalSpace const & cps) const { return c == cps.c && p == cps.p && ps == cps.ps; }
    size_t hash() const { return hash_of(c, p, ps); }
};

struct Game::Members : Game::State, GameImpl<CharacterImpl, BirdImpl, DartImpl, PersonalSpaceImpl> {
    ShapePtr worldBox{sensor(boxShape(20, 30, {0, 0}, 0), ct_universe)};
    ShapePtr abyssWalls[3];
    ShapePtr ground{segmentShape({-10, 2}, {10, 2})};
    ShapePtr attackLine{sensor(segmentShape({-10, ATTACK_LINE_Y}, {10, ATTACK_LINE_Y}), ct_attack)};
    ShapePtr startleLine;
    ShapePtr lbarrier{segmentShape({-9, 2}, {-9, 2.5})};
    ShapePtr rbarrier{segmentShape({9, 2}, {9, 2.5})};
    size_t created_birds = 0;
    std::unique_ptr<Ticker> tick;
    Relation<CharacterJointBird> cjb;
    Relation<BirdTargetCharacter> targets;
    Relation<CharacterShotDart> csd;
    Relation<CharacterPersonalSpace> cps;
    brac::Stopwatch watch{false};

    Members(SpaceTime & st) : Impl{st} { }

    bool isKidnappable(CharacterImpl const & c) {
        return (c.state() != Character::State::rescued &&
                !(from(cjb) >> any([&](auto && cjb) { return cjb.c == &c; })));
    }

    bool isBeingTargeted(CharacterImpl const & c) {
        return (from(targets) >> any([&](auto && t) { return t.c == &c; }));
    }

    bool hasCaptive(BirdImpl const & b) {
        return (from(cjb) >> any([&](auto && cjb) { return cjb.b == &b; }));
    }

    bool isCaptive(CharacterImpl const & c) {
        return (from(cjb) >> any([&](auto && cjb) { return cjb.c == &c; }));
    }

    bool firedDart(CharacterImpl const & c, DartImpl const & d) {
        return (from(csd) >> any([&](auto && csd) { return csd.c == &c && csd.d == &d; }));
    }
};

Game::Game(SpaceTime & st, GameMode mode, int level, float top) : GameBase{st}, m{new Members{st}} {
    m->mode = mode;
    m->level = level;

    auto removeCharacter = [=](CharacterImpl & c) {
        // remove PersonalSpaceImpl
        auto matching = from(m->cps) >> mutable_ref() >> where([&](auto && cps) { return cps.get().c == &c; });
        if (matching >> any()) {
            auto & cps = (matching >> first()).get();
            m->removeWhenSpaceUnlocked(*cps.ps);
        }
        m->cps >> removeIf([&](auto && cps) { return cps.c == &c; });

        archiveCharacterStats(c.stats);
        m->removeWhenSpaceUnlocked(c);
    };

    auto newTarget = [=](BirdImpl & b) {
        m->targets >> removeIf([&](auto && target) { return target.b == &b; });

        auto available = (from(m->actors<CharacterImpl>())
                          >> mutable_ref()
                          >> where([&](auto &&c) { return m->isKidnappable(c); }));

        if (available >> any()) {
            auto not_targeted = available >> where([&](auto && c) { return !m->isBeingTargeted(c); });
            if (not_targeted >> any()) {
                // get first untargeted character
                auto & c = (not_targeted >> first()).get();
                m->targets.insert(BirdTargetCharacter{&b, &c});
                b.setDesiredPos(c.pos());
            } else {
                // otherwise pick any available target at random
                int i = 1; auto r = rand<int>(1, int(available >> count()));
                available >> for_each([&](auto && c) {
                    if (i == r) {
                        m->targets.insert(BirdTargetCharacter{&b, &c.get()});
                        b.setDesiredPos(c.get().pos());
                    }
                    ++i;
                });
            }
        } else {
            b.fromWhence = true;
            b.setDesiredPos({rand<float>(-6, 6), rand<float>(top, top + 2)});
        }
    };

    // send random birds after character
    auto targetCharacter = [=](CharacterImpl & c) {
        auto available = (from(m->actors<BirdImpl>())
                          >> mutable_ref()
                          >> where([&](BirdImpl const & b) { return !m->hasCaptive(b) && b.pos().y > ATTACK_LINE_Y && b.isFlying(); }));
        if (available >> any()) {
            // send first available bird after c for now
            auto & b = (available >> first()).get();
            m->targets >> removeIf([&](auto && target) { return target.b == &b; });
            m->targets.insert(BirdTargetCharacter{&b, &c});
            b.setDesiredPos(c.pos());
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
        m->startleLine = m->sensor(m->segmentShape({-10, top - 1}, {10, top - 1}), ct_startle);
        m->rem_birds = BIRDS;
        m->rem_chars = CHARACTERS;
        m->back->setY(top - 0.8);
        m->restart->setY(top - 0.8);
        // Temporary
        m->playerStats.characters = CHARACTERS;
        m->playerStats.birds = BIRDS;

        cpShapeSetCollisionType(&*m->ground, ct_ground);
        cpShapeSetFriction(&*m->ground, 1);
        cpShapeSetLayers(&*m->ground, l_play);

        auto createCharacter = [=](vec2 const v) {
            auto & c = m->emplace<CharacterImpl>(0, v);
            auto & ps = m->emplace<PersonalSpaceImpl>(v);
            m->cps.insert(CharacterPersonalSpace{&c, ps.attachCharacterBody(*c.body()), &ps});
        };

        auto createCharacters = [=]{
            float min = -9;
            for (int i = 0; i < CHARACTERS; ++i) {
                float max = min + (18 / CHARACTERS);
                vec2 v{rand<float>(min + 0.5, max - 0.5), 2.3};
                createCharacter(v);
                min = max;
            }
            for (auto & c : m->actors<CharacterImpl>()) c.initState();
        };

        auto createBird = [=]{
            auto & b = m->emplace<BirdImpl>(0, vec2{rand<float>(-10, 10), rand<float>(top, top - 1)});
            newTarget(b);
        };

        createCharacters();

        // create birds
        m->tick.reset(new Ticker{BIRDFREQUENCY, [=]{
            delay(rand<float>(0, 1), [&]{
                if (m->created_birds < int(BIRDS)) {
                    if (from(m->actors<CharacterImpl>()) >> any([&](auto && c) { return m->isKidnappable(c); })) {
                        createBird();
                        ++m->created_birds;
                    }
                } else {
                //    m->tick.reset();
                }
            }).cancel(destroyed);
        }});
    }

    m->onCollision([=](BirdImpl & bird, CharacterImpl & character, cpArbiter * arb) {
        if (!(bird.canGrabCharacter() && m->isKidnappable(character) && cpArbiterIsFirstContact(arb))) {
            return false;
        }

        if (!(from(m->cjb) >> any([&](auto && cjb) { return cjb.b == &bird; }))) {
            character.kidnap();

            m->cjb.insert(CharacterJointBird{&character, bird.grabCharacter(*character.body()), &bird});
            m->targets >> removeIf([&](auto && target) { return target.b == &bird; });

            vec2 p = {rand<float>(-10, 10), rand<float>(top, top + 1)};
            bird.escapeVel = unit(p - bird.pos());

            for (auto & shape : character.shapes()) cpShapeSetGroup(&*shape, gr_bird);

            // send other birds after new target
            from(m->targets) >> for_each([&](auto && targets) {
                if (targets.c == &character && targets.b != &bird && targets.b->isFlying()) {
                    if (targets.b->pos().y > ATTACK_LINE_Y) {
                        newTarget(*targets.b);
                    } else {
                        // flying too low to target new character
                        m->targets >> removeIf([&](auto && target) { return target.b == targets.b; });
                        vec2 atp{rand<float>(-6, 6), ATTACK_LINE_Y};
                        targets.b->setDesiredPos(atp);
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
            m->targets >> removeIf([&](auto && target) { return target.b == &bird; });
            bird.setState(Bird::State::dying);

            // Score kill against character
            auto shooter = (from(m->actors<CharacterImpl>())
                            >> mutable_ref()
                            >> where([&](auto && c) { return m->firedDart(c, dart); }));
            if (shooter >> any()) {
                auto & c = (shooter >> first()).get();
                ++c.stats.birdsKilled;
                if (from(m->cjb) >> any([&](auto && cjb) { return cjb.b == &bird; })) {
                    ++c.stats.rescues;
                }
            }

            // free captive, if necessary
            auto matching = from(m->cjb) >> ref() >> where([&](auto && cjb) { return cjb.get().b == &bird; });
            if (matching >> any()) {
                auto & cjb = (matching >> first()).get();
                cjb.b->dropCharacter();
                cjb.c->rescue();
                m->cjb.erase(cjb);
                m->score += SCORE_BIRD_KILLED;
            } else {
                m->score += SCORE_CHAR_RESCUED;
            }
            
            bird.setAngle(0);
            bird.setVel({0, -3});
            cpBodySetAngVel(bird.body(), 0);
            dart.setVel({0, 0});

            m->csd >> removeIf([&](auto && csd) { return csd.d == &dart; });
            m->removeWhenSpaceUnlocked(dart);
            --m->rem_birds;
            ++m->playerStats.kills;

            if (m->rem_birds == 0) {
                for (auto & c : m->actors<CharacterImpl>()) {
                    c.celebrate();
                }
                delay(4, [=]{ gameOver(true); }).cancel(destroyed);
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
            // Score friendly fire against character
            auto shooter = (from(m->actors<CharacterImpl>())
                            >> mutable_ref()
                            >> where([&](auto && c) { return m->firedDart(c, dart); }));
            if (shooter >> any()) {
                auto & c = (shooter >> first()).get();
                ++c.stats.friendlies;
            }

            m->csd >> removeIf([&](auto && csd) { return csd.d == &dart; });
            m->removeWhenSpaceUnlocked(dart);

            if (character.state() != Character::State::dead) {
                character.setState(Character::State::dead);
                --m->rem_chars;
                if (m->rem_chars == 0) {
                    delay(2, [=]{ gameOver(false); }).cancel(destroyed);
                }
            }
        }
        return false;
    });

    m->onPostSolve([=](CharacterImpl & character, NoActor<ct_ground> &, cpArbiter * arb) {
        if (cpArbiterIsFirstContact(arb)) {
            character.setAngle(0);
            switch (character.state()) {
                case Character::State::startled:
                    character.setState(Character::State::determined);
                    delay(rand<float>(0.1, 0.8), [&] {
                        character.reload();
                    }).cancel(destroyed);
                    break;
                case Character::State::rescued:
                    character.reload();
                    targetCharacter(character);
                    break;
                case Character::State::celebrating:
                    character.setVel({0, rand<float>(3, 4.5)});
                    break;
                case Character::State::ready:
                    character.setVel({0, 2});
                    break;
                case Character::State::dead:
                    if (!m->isCaptive(character)) {
                        character.setVel({0, 0});
                    }
                default:
                    break;
            }
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

    m->onCollision([=](BirdImpl & bird, NoActor<ct_startle> &) {
        if (!m->started) {
            m->watch.start();
            for (auto & c : m->actors<CharacterImpl>()) c.startle();
            m->started = true;
        }
    });

    m->onCollision([=](PersonalSpaceImpl & ps1, PersonalSpaceImpl & ps2, cpArbiter *) {
        auto whosePersonalSpace = [=](PersonalSpaceImpl & ps) {
            auto matching1 = from(m->cps) >> mutable_ref() >> where([&](CharacterPersonalSpace const & cps) { return cps.ps == &ps; });
            auto & cps1 = (matching1 >> first()).get();
            return cps1.c;
        };

        auto c1 = whosePersonalSpace(ps1);
        auto c2 = whosePersonalSpace(ps2);

        // return false if either character is currently kidnapped
        return !((from(m->cjb) >> any([&](CharacterJointBird const & cjb) { return cjb.c == c1; }))
                 || (from(m->cjb) >> any([&](CharacterJointBird const & cjb) { return cjb.c == c2; })));
    });

    m->onCollision([=](BirdImpl & bird, NoActor<ct_abyss> &, cpArbiter * arb) {
        if (cpArbiterIsFirstContact(arb)) {
            auto matching = from(m->cjb) >> ref() >> where([&](CharacterJointBird const & cjb) { return cjb.b == &bird; });
            if (matching >> any()) {
                auto & cjb = (matching >> first()).get();
                m->cjb.erase(cjb);
                //m->removeWhenSpaceUnlockedIf(from (m->actors<CharacterImpl>()) >> where([&](CharacterImpl const & c) { return &c == cjb.c; }));
                for (auto & c : m->actors<CharacterImpl>()) {
                    if (&c == cjb.c) {
                        removeCharacter(c);
                        if (c.state() != Character::State::dead) {
                            --m->rem_chars;
                            if (m->rem_chars == 0) {
                                delay(2, [=]{ gameOver(false); }).cancel(destroyed);
                            }
                        }
                    }
                }
                m->targets >> removeIf([&](auto && target) { return target.b == &bird; });
                m->removeWhenSpaceUnlocked(bird);
                --m->created_birds; // bird will be replaced
            } else {
                if (bird.fromWhence) {
                    --m->created_birds;
                    m->removeWhenSpaceUnlocked(bird);
                }
            }
        }
    });
}

void Game::gameOver(bool passed) {
    m->tick.reset();
    m->level_passed = passed;
    m->watch.stop();
    if (passed) {
        m->playerStats.time = m->watch.time();
        m->playerStats.remCharacters = m->rem_chars;
        m->score += m->rem_chars * SCORE_CHAR_SURVIVED;
        for (auto & c : m->actors<CharacterImpl>()) {
            archiveCharacterStats(c.stats);
        }
    }
    end();
}

void Game::archiveCharacterStats(CharacterStats s) {
    m->characterStats.emplace_back(s);
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
                            auto & dart = self->m->emplace<DartImpl>(character->pos() + LAUNCH_OFFSET * unit(vel), vel);
                            self->m->csd.insert(CharacterShotDart{character, &dart});
                            character->shoot();
                            ++self->m->playerStats.darts;
                            self->m->score += SCORE_DART_FIRED;
                        }
                    }
                }
            }

            virtual void moved(vec2 const & p, bool) {
                if (auto self = weak_self.lock()) {
                    if (self->m->isCaptive(*character)) {
                        // captured; end touch event
                        return;
                    } else {
                        auto v = first_p - p;
                        character->setState(Character::State::aim);
                        if (float s = length(v)) {
                            character->aim((14 + 2 * s) / s * v);
                        }
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
