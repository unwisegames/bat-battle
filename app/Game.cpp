#include "Game.h"
#include "bats.sprites.h"
#include "characters.sprites.h"
#include "character1.sprites.h"
#include "character2.sprites.h"
#include "character3.sprites.h"
#include "character4.sprites.h"
#include "character5.sprites.h"
#include "character6.sprites.h"
#include "character7.sprites.h"
#include "character8.sprites.h"
#include "character9.sprites.h"
#include "character10.sprites.h"
#include "atlas2.sprites.h"
#include "bomb.sprites.h"
#include "blast.sprites.h"

#include <bricabrac/Data/Relation.h>
#include <bricabrac/Game/GameActorImpl.h>
#include <bricabrac/Logging/Logging.h>
#include <bricabrac/Math/MathUtil.h>
#include <bricabrac/Math/Random.h>
#include <bricabrac/Thread/quantize.h>
#include <bricabrac/Utility/Timer.h>

#include <math.h>
#include <unordered_set>
#include <iostream>

//#include "levels.h"

using namespace brac;
using namespace cpplinq;

enum Group : cpGroup { gr_bird = 1, gr_character };

enum Category : cpBitmask { cat_all = 1<<0, cat_character = 1<<1, cat_halo = 1<<2, cat_play = 1<<3 };

enum CollisionType : cpCollisionType { ct_universe = 1, ct_abyss, ct_ground, ct_attack, ct_startle, ct_barrier, ct_wall };

enum BirdType { bt_grey, bt_yellow, bt_bomb };

using CharacterSprites = SpriteLoopDef const[23];
using CharacterMugshot = SpriteDef const;
struct CharDef {
    CharacterSprites * sprites;
    CharacterMugshot * mug;
};

auto char_def = [](auto const & c) { return CharDef{&c.character, &c.mugshot}; };

static CharDef char_defs[] = {
    char_def(character1),
    char_def(character2),
    char_def(character3),
    char_def(character4),
    char_def(character5),
    char_def(character6),
    char_def(character7),
    char_def(character8),
    char_def(character9),
    char_def(character10),
};

struct PersonalSpaceImpl : BodyShapes<PersonalSpace> {
    PersonalSpaceImpl(cpSpace * space, vec2 const pos)
    : BodyShapes{space, newBody(1, INFINITY, pos), newCircleShape(0.6), {CP_NO_GROUP, cat_halo, cat_halo}}
    {
        for (auto & shape : shapes()) {
            cpShapeSetElasticity(&*shape, 0);
        }
    }

    ConstraintPtr attachCharacterBody(cpBody & b) {
        //cpPinJointSetDist(&*j, 0.2);
        //auto j = newPinJoint(body(), &b, {0, 0}, {0, 0});
        auto j = newPivotJoint(body(), &b, position());
        return j;
    }
};

struct GraveImpl : BodyShapes<Grave> {
    GraveImpl(cpSpace * space, vec2 const pos)
    : BodyShapes{space, newStaticBody(pos), sensor(atlas2.grave)}
    {
        setState(Grave::State::rising);
    }

    void newFrame(bool loopChanged) override {
        if (!loopChanged && frame() == 0) {
            switch (state()) {
                case Grave::State::rising:
                    setState(Grave::State::still);
                    break;
                default:
                    break;
            }
        }
    }
};

struct CharacterImpl : BodyShapes<Character> {
    vec2 launchVel_ = {0, 0};
    int type_;
    ShapePtr shape;
    CharacterStats stats;

    CharacterImpl(cpSpace * space, int type, vec2 const & pos)
    : BodyShapes{space, newBody(1, INFINITY, pos), sensor(*char_defs[type].sprites), {gr_character, cat_play | cat_character, cat_play | cat_character}} {
        for (auto & shape : shapes()) {
            cpShapeSetElasticity(&*shape, 1);
        }

        shape = newCircleShape(0.3, {0, 0})(this, body());
        cpShapeSetFriction(&*shape, 0.2);
        cpShapeSetFilter(&*shape, {gr_character, cat_play | cat_character, cat_play | cat_character});

        stats.mugshot = *char_defs[type].mug;

        type_ = type;

        spawn([ticks = chan::spawn_quantize(update_me(), 5.0)]{
            while (ticks >> nullptr) {
                //std::cerr << "Update\n";
            }
        });
    }

    virtual bool isAiming() const override {
        return launchVel_ && state() == Character::State::aim;
    }

    virtual bool isDead() const override {
        return state() == Character::State::dead;
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
                    setVelocity({0, 2});
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
        update_me(spawn_after(rand<double>(0.0, 0.3), [&]{
            setState(randomChoice({biggrin, smile, smug, mag, basketball, hum, wave}));
        }));
    }

    void reload() {
        setVelocity({0, 0});
        setState(Character::State::reloading);
    }

    virtual bool readyToFire() const override {
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
        if (!isDead()) {
            setState(Character::State::yell);
            update_me(spawn_after(1, [this]{
                if (state() == Character::State::yell) {
                    setState(Character::State::crying);
                }
            }));
        }
    }

    void rescue() {
        if (!isDead()) {
            setState(Character::State::rescued);
        }
    }

    void cry() {
        setState(Character::State::crying);
    }

    void celebrate() {
        if (!isDead()) {
            setAngle(0);
            setState(Character::State::celebrating);
            setVelocity({0, rand<float>(6, 8.5)});
        }
    }

    void startle() {
        setVelocity({0, 3});
        setState(Character::State::startled);
    }
};

constexpr float F = 1;

struct BirdImpl : BodyShapes<Bird> {
    bool hasCaptive = false;
    bool fromWhence = false;
    bool ignoreAbyss = false;
    vec2 escapeVel;
    vec2 desired_pos;
    BirdType bird_type;
    int resilience;
    float speed;

    BirdImpl(cpSpace * space, BirdType type, vec2 const & pos, float sp)
    : BodyShapes{space, newBody(1, 1, pos), bats.bats[type], {gr_bird, cat_play, cat_play}}
    {
        //cpBodySetType(body(), CP_BODY_TYPE_KINEMATIC);
        bird_type = type;
        resilience = int(type);
        speed = rand<float>(sp - (sp * 0.3), sp + (sp * 0.3));
        setForce(vec2{0, 10});
    }

    void newFrame(bool loopChanged) override {
        if (!loopChanged && frame() == 0) {
            if (isFlying() && bird_type == bt_yellow && resilience == 0) {
                setVelocity({0, 0});
                cpBodyApplyImpulseAtLocalPoint(body(), to_cpVect({rand<float>(-0.5, 0.5), rand<float>(-0.5, 0.5)}), to_cpVect({0, 0}));
            }
        }
    }

    void newState(size_t & loop) override {
        loop = size_t(state());
    }

    virtual void doUpdate(float) override {
        setForce(-WORLD_GRAVITY);
        if (isFlying()) {
            setAngle(0);
            auto v = unit(velocity());
            setState(fromWhence && v.y > 0  ? Bird::State::rear     :
                     fabsf(v.x) < 0.5       ? Bird::State::front
                     :                        Bird::State::side);

            if (isFlying() && !hasCaptive) {
//                setVelocity((unit(desired_pos - position()) * speed));// + -WORLD_GRAVITY);
//                setForce((unit(desired_pos - position()) * speed) + -WORLD_GRAVITY);
                auto dv = unit(desired_pos - position()) * speed - velocity();
                float epsilon = 0.01;
                setForce(((length_sq(dv) > epsilon) * F * unit(dv)) + -WORLD_GRAVITY);
            } else {
                // maintain velocity
                setVelocity((escapeVel * speed));// + -WORLD_GRAVITY);
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
            cpPinJointSetDist(&*j, bird_type == bt_grey ? 0.6 : 0.5);
            return j;
        };
        hasCaptive = true;
        return {joint({-0.2, 0}), joint({0.2, 0})};
    }

    void dropCharacter() {
        //        setForce({0, -WORLD_GRAVITY});
    }
};

struct BombBatImpl : BodyShapes<BombBat> {
    vec2 desired_pos;

    BombBatImpl(cpSpace * space, vec2 const & pos, vec2 const & des_pos)
    : BodyShapes{space, newBody(1, 1, pos), bomb.bat, {gr_bird, cat_play, cat_play}}
    {
        //cpBodySetType(body(), CP_BODY_TYPE_KINEMATIC);
        //setVelocity({0.5, -0.5});
        desired_pos = des_pos;
        setForce(-WORLD_GRAVITY);
    }

    virtual void newState(size_t & loop) override {
        loop = size_t(state());
    }

    virtual void doUpdate(float) override {
        setAngle(0);
        //setForce(-WORLD_GRAVITY);
        /*auto dv = unit(desired_pos - position()) * 1 - velocity(); // "1" = speed
        float epsilon = 0.01;
        if (length_sq(dv) > epsilon) {
            setForce(F * unit(dv));
        }*/
        auto dv = unit(desired_pos - position()) * 1 - velocity(); // "1" = speed
        float epsilon = 0.01;
        setForce(((length_sq(dv) > epsilon) * F * unit(dv)) + -WORLD_GRAVITY);
    }

    array<ConstraintPtr, 2> holdBomb(cpBody & b) {
        auto joint = [&](vec2 const & pos) {
            auto j = newPinJoint(body(), &b, pos, {0, 0});
            cpPinJointSetDist(&*j, 0.8);
            return j;
        };
        return {joint({-0.2, 0}), joint({0.2, 0})};
    }

    void setDesiredPos(vec2 dp) {
        desired_pos = dp;
    }
};

struct BombBatCarrotImpl : BodyShapes<BombBatCarrot> {
    ShapePtr shape;

    BombBatCarrotImpl(cpSpace * space, vec2 const & pos)
    : BodyShapes{space, newBody(1, 1, pos), sensor(bomb.bomb), {CP_NO_GROUP, cat_play, cat_play}}
    {
        //cpBodySetType(body(), CP_BODY_TYPE_STATIC);
        setForce(-WORLD_GRAVITY);

        shape = newCircleShape(0.1, {0, 0})(this, body());
        cpShapeSetFilter(&*shape, {CP_NO_GROUP, cat_play, cat_play});
        cpShapeSetSensor(&*shape, true);
    }

    virtual void doUpdate(float) override {
        setForce(-WORLD_GRAVITY);
    }
};

struct BombImpl : BodyShapes<Bomb> {
    writer<> ticker_keepalive;
    bool offsetGravity = true;

    BombImpl(cpSpace * space, vec2 const & pos)
    : BodyShapes{space, newBody(1, 1, pos), bomb.bomb, {gr_bird, cat_play, cat_play}}
    {
        setForce(-WORLD_GRAVITY);
    }

    virtual void newState(size_t & loop) override {
        loop = size_t(state());
    }

    virtual void doUpdate(float) override {
        if (offsetGravity) {
            setForce(-WORLD_GRAVITY);
        }
    }
};

struct BlastImpl : BodyShapes<Blast> {
    BlastImpl(cpSpace * space, vec2 const & pos)
    : BodyShapes{space, newBody(1, 1, pos), sensor(blast.blast), {CP_NO_GROUP, cat_play, cat_play}}
    {
        setForce(-WORLD_GRAVITY);
    }

    void newState(size_t & loop) override {
        loop = size_t(state());
    }

    virtual void doUpdate(float) override {
        setForce(-WORLD_GRAVITY);
    }
};

struct CharacterExplosionImpl : BodyShapes<CharacterExplosion> {
    CharacterExplosionImpl(cpSpace * space, vec2 const & pos)
    : BodyShapes{space, newBody(1, 1, pos), sensor(blast.characterblast), {CP_NO_GROUP, cat_play, cat_play}}
    {
        setForce(-WORLD_GRAVITY);
    }

    virtual void doUpdate(float) override {
        setForce(-WORLD_GRAVITY);
    }
};

struct DartImpl : BodyShapes<Dart> {
    ConstraintPtr p;

    DartImpl(cpSpace * space, vec2 const & pos, vec2 const & vel)
    : BodyShapes{space, newBody(1, 1, pos), characters.dart, {CP_NO_GROUP, cat_play, cat_play}}
    {
        setVelocity(vel);
        active = true;

        for(int i = 0; i <= DART_TRAIL_SEGMENTS; ++i) {
            trail[i] = pos;
        }
    }

    virtual void doUpdate(float _dt) override {
        if (active) {
            score += SCORE_DART_INCREMENT;
            setAngle(::brac::angle(velocity()));

            for (int i = 0; i < DART_TRAIL_SEGMENTS; ++i) {
                trail[i] = trail[i + 1];
            }
            trail[DART_TRAIL_SEGMENTS] = position();
        }
        dt += _dt;
    }

    void attach(cpBody & b) { }
};

struct CharacterJointBird {
    CharacterImpl * const c;
    array<ConstraintPtr, 2> p;
    BirdImpl * const b;

    bool operator==(CharacterJointBird const & cjb) const { return c == cjb.c && p == cjb.p && b == cjb.b; }
    size_t hash() const { return hash_of(c, p[0], p[1], b); }
};

struct BirdTargetCharacter {
    BirdImpl * const b;
    CharacterImpl * const c;

    bool operator==(BirdTargetCharacter const & targets) const { return b == targets.b && c == targets.c; }
    size_t hash() const { return hash_of(b, c); }
};

struct CharacterShotDart {
    CharacterImpl * const c;
    DartImpl * const d;

    bool operator==(CharacterShotDart const & csd) const { return c == csd.c && d == csd.d; }
    size_t hash() const { return hash_of(c, d); }
};

struct CharacterPersonalSpace {
    CharacterImpl * const c;
    ConstraintPtr p;
    PersonalSpaceImpl * const ps;

    bool operator==(CharacterPersonalSpace const & cps) const { return c == cps.c && p == cps.p && ps == cps.ps; }
    size_t hash() const { return hash_of(c, p, ps); }
};

struct BatJointBomb {
    BombBatImpl * const bat;
    array<ConstraintPtr, 2> p;
    BombImpl * const bomb;

    bool operator==(BatJointBomb const & bjb) const { return bat == bjb.bat && p == bjb.p && bomb == bjb.bomb; }
    size_t hash() const { return hash_of(bat, p[0], p[1], bomb); }
};

struct BombBatCarrotRel {
    BombBatImpl * const bat;
    BombBatCarrotImpl * const carrot;

    bool operator==(BombBatCarrotRel const & bbcr) const { return bat == bbcr.bat && carrot == bbcr.carrot; }
    size_t hash() const { return hash_of(bat, carrot); }
};

struct CharacterDetonatedBomb {
    CharacterImpl * const c;
    BombImpl * const bomb;

    bool operator==(CharacterDetonatedBomb const & cdb) const { return c == cdb.c && bomb == cdb.bomb; }
    size_t hash() const { return hash_of(c, bomb); }
};

struct Game::Members : Game::State, GameImpl<CharacterImpl, BirdImpl, DartImpl, PersonalSpaceImpl, GraveImpl, BombBatImpl, BombImpl, BlastImpl, CharacterExplosionImpl, BombBatCarrotImpl> {
    ShapePtr worldBox{sensor(boxShape(20, 30, {0, 0}, 0), ct_universe)};
    ShapePtr abyssWalls[3];
    ShapePtr ground{segmentShape({-10, 2}, {10, 2})};
    ShapePtr attackLine{sensor(segmentShape({-10, ATTACK_LINE_Y}, {10, ATTACK_LINE_Y}), ct_attack)};
    ShapePtr startleLine;
    ShapePtr lbarrier{segmentShape({-9, 2}, {-9, 2.5})};
    ShapePtr rbarrier{segmentShape({9, 2}, {9, 2.5})};
    ShapePtr lslope{segmentShape({-9, 2.5}, {-10, 4})};
    ShapePtr rslope{segmentShape({9, 2.5}, {10, 4})};
    ShapePtr leftWall;
    ShapePtr rightWall;
    size_t created_grey_bats = 0;
    size_t created_yellow_bats = 0;
    channel<> ticker_keepalive;
    Relation<CharacterJointBird>        cjb;
    Relation<BirdTargetCharacter>       targets;
    Relation<CharacterShotDart>         csd;
    Relation<CharacterPersonalSpace>    cps;
    Relation<BatJointBomb>              bjb;
    Relation<BombBatCarrotRel>          bbcr;
    Relation<CharacterDetonatedBomb>    cdb;
    brac::Stopwatch watch;
    std::vector<int> deadChars;

    Members(SpaceTime & st, std::shared_ptr<TimerImpl> timer) : Impl{st}, watch{timer, false} { }

    bool isKidnappable(CharacterImpl const & c) {
        return (c.state() != Character::State::rescued && !c.isDead() &&
                !(from(cjb) >> any([&](auto && cjb) { return cjb.c == &c; })));
    }

    bool isBeingTargeted(CharacterImpl const & c) {
        return from(targets) >> any([&](auto && t) { return t.c == &c; });
    }

    bool hasCaptive(BirdImpl const & b) {
        return from(cjb) >> any([&](auto && cjb) { return cjb.b == &b; });
    }

    bool isCaptive(CharacterImpl const & c) {
        return from(cjb) >> any([&](auto && cjb) { return cjb.c == &c; });
    }

    bool firedDart(CharacterImpl const & c, DartImpl const & d) {
        return from(csd) >> any([&](auto && csd) { return csd.c == &c && csd.d == &d; });
    }

    bool anybodyLeft() {
        auto iAm = [=](CharacterImpl const & c) {
            return !c.isDead() && !(from(cjb) >> any([&](auto && cjb) { return cjb.c == &c; }));
        };

        return from(actors<CharacterImpl>()) >> any(iAm);
    }

    /*bool levelOver() {
        return true; //level_passed || level_failed;
    }*/

    void alertsHousekeeping() {
        alerts.erase(std::remove_if(alerts.begin(), alerts.end(), [&](const TextAlert & a){ return !a.alpha; }), alerts.end());
    }

    void popup (std::string s, vec2 pos, float duration, float scale) {
        alertsHousekeeping();
        auto a = alerts.insert(alerts.begin(), {s, pos, scale});

        update_me(spawn_consumer<double>([=](auto r) {
            if (sleep(duration, r) <= 0) {
                for (double dt; r >> dt && (a->alpha -= 2 * dt) > 0;) { }
                a->alpha = 0;
            }
        }));
    }
};

Game::Game(SpaceTime & st, GameMode mode, float top, std::shared_ptr<TimerImpl> timer) : GameBase{st}, m{new Members{st, timer}} {
    m->mode = mode;

    auto registerTextAlert = [=](std::string s, vec2 pos, float duration, float scale) {
        m->alertsHousekeeping();
        auto a = m->alerts.insert(m->alerts.begin(), {s, pos, scale});

        m->update_me(spawn_consumer<double>([=](auto r) {
            if (sleep(duration, r) <= 0) {
                for (double dt; r >> dt && (a->alpha -= 2 * dt) > 0;) { }
                a->alpha = 0;
            }
        }));
        alert();
    };

    auto newTarget = [=](BirdImpl & b) {
        m->targets >> removeIf([&](auto && target) { return target.b == &b; });

        auto available = (from(m->actors<CharacterImpl>()) >> mutable_ref()
                          >> where([&](auto &&c) { return m->isKidnappable(c); }));

        if (available >> any()) {
            auto not_targeted = available >> where([&](auto && c) { return !m->isBeingTargeted(c); });
            if (not_targeted >> any()) {
                // get first untargeted character
                auto & c = (not_targeted >> first()).get();
                m->targets.insert(BirdTargetCharacter{&b, &c});
                b.setDesiredPos(c.position());
            } else {
                // otherwise pick any available target at random
                int i = 1; auto r = rand<int>(1, int(available >> count()));
                available >> for_each([&](auto && c) {
                    if (i == r) {
                        m->targets.insert(BirdTargetCharacter{&b, &c.get()});
                        b.setDesiredPos(c.get().position());
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
        auto available = (from(m->actors<BirdImpl>()) >> mutable_ref()
                          >> where([&](BirdImpl const & b) { return !m->hasCaptive(b) && b.position().y > ATTACK_LINE_Y && b.isFlying(); }));
        if (available >> any()) {
            // send first available bird after c for now
            auto & b = (available >> first()).get();
            m->targets >> removeIf([&](auto && target) { return target.b == &b; });
            m->targets.insert(BirdTargetCharacter{&b, &c});
            b.setDesiredPos(c.position());
        }
    };

    auto retargetBirdsChasingMe = [=](CharacterImpl & character) {
        // send birds after new target
        from(m->targets) >> for_each([&](auto targets) {
            if (targets.c == &character && targets.b->isFlying()) {
                if (targets.b->position().y > ATTACK_LINE_Y) {
                    newTarget(*targets.b);
                } else {
                    // flying too low to target new character
                    m->targets >> removeIf([&](auto && target) { return target.b == targets.b; });
                    vec2 atp{rand<float>(-6, 6), ATTACK_LINE_Y};
                    targets.b->setDesiredPos(atp);
                }
            }
        });
    };

    auto removeCharacter = [=](CharacterImpl & c) {
        // remove PersonalSpaceImpl
        auto matching = from(m->cps) >> mutable_ref() >> where([&](auto && cps) { return cps.get().c == &c; });
        if (matching >> any()) {
            auto & cps = (matching >> first()).get();
            m->removeWhenSpaceUnlocked(*cps.ps);
        }
        m->cps >> removeIf([&](auto && cps) { return cps.c == &c; });

        archiveCharacterStats(c.stats);
        m->deadChars.emplace_back(c.type_);
        retargetBirdsChasingMe(c);
        m->removeWhenSpaceUnlocked(c);
    };

    auto characterKilled = [=]() {
        --m->rem_chars;
        if (m->rem_chars == 0) {
            m->game_over = true;

            m->update_me(spawn_after(0.5, [&]{
                tension_stop();
                lose();
            }));
            m->update_me(spawn_after(3.0, [&]{ gameOver(); }));

            /*spawn([=, sleep = sleeper(m->update_me())]{
                if (sleep(0.5)) {
                    tension_stop();
                    lose();
                    //if (sleep(2)) {
                        gameOver();
                    //}
                }
            });*/
        }
    };

    auto batKilled = [=](BirdType bt) {
        ++m->playerStats.kills;

        if (bt == bt_grey) {
            ++m->grey_bats_killed;
        } else if (bt == bt_yellow) {
            ++m->yellow_bats_killed;
        }
        /*if (m->rem_grey_bats == 0 && m->rem_yellow_bats == 0) {
            if (!m->level_failed) {
                m->level_passed = true;
                for (auto & c : m->actors<CharacterImpl>()) {
                    if (!c.isDead())
                        yay();
                    c.celebrate();
                }
                tension_stop();

                spawn([=, sleep = sleeper(m->update_me())]{
                    if (sleep(1)) {
                        char_score();
                        m->show_char_score = true;
                        if (sleep(2)) {
                            gameOver(true);
                        }
                    }
                });
            }
        }*/
    };

    auto playCharacterExplosion = [=](vec2 pos) {
        auto expl = &m->emplace<CharacterExplosionImpl>(pos);
        charblast();
        m->update_me(spawn_after(1.1, [=]{
            m->removeWhenSpaceUnlocked(*expl);
        }));
    };

    auto detonate = [=](BombImpl & bomb) {
        bombwhistle_stop();

        // who caused detonation?
        CharacterImpl * character;
        auto characterCausedDetonation = [&](BombImpl & bomb) {
            auto matching = from(m->cdb) >> mutable_ref() >> where([&](auto && cdb) { return cdb.get().bomb == &bomb; });
            if (matching >> any()) {
                auto & cdb = (matching >> first()).get();
                character = cdb.c;
                return true;
            } else {
                return false;
            }
        };
        auto characterCaused = characterCausedDetonation(bomb);

        auto blastPos = bomb.position();
        auto blast = &m->emplace<BlastImpl>(blastPos);
        boom();
        m->update_me(spawn_after(1.1, [=]{
            m->removeWhenSpaceUnlocked(*blast);
        }));
        m->removeWhenSpaceUnlocked(bomb);

        auto inBlastRadius = [=](vec2 pos) {
            return length_sq(pos - blastPos) < 2.5;
        };

        if (!m->game_over) {
            for (auto & c : m->actors<CharacterImpl>()) {
                if (inBlastRadius(c.position())) {
                    if (characterCaused) {
                        ++character->stats.friendlies;
                    }

                    // is character currently kidnapped? If so, release character.
                    auto matching = from(m->cjb) >> ref() >> where([&](auto && cjb) { return cjb.get().c == &c; });
                    if (matching >> any()) {
                        auto & cjb = (matching >> first()).get();
                        m->cjb.erase(cjb);
                        cjb.b->hasCaptive = false;

                        // send bird after new target
                        m->targets >> removeIf([&](auto && target) { return target.b == cjb.b; });
                        if (cjb.b->position().y > ATTACK_LINE_Y) {
                            newTarget(*cjb.b);
                        } else {
                            // flying too low to target new character
                            vec2 atp{rand<float>(-6, 6), ATTACK_LINE_Y};
                            cjb.b->setDesiredPos(atp);
                        }
                    }
                    auto pos = c.position();
                    m->update_me(spawn_after(rand<double>(0.01, 0.2), [=]{
                        playCharacterExplosion(pos);
                    }));
                    removeCharacter(c);
                    retargetBirdsChasingMe(c);

                    if (!c.isDead()) {
                        characterKilled();
                    }
                }
            }

            if (!m->game_over) { // above didn't wipe out last of characters
                for (auto & b : m->actors<BirdImpl>()) {
                    if (b.isFlying()) {
                        if (inBlastRadius(b.position())) {
                            if (characterCaused) {
                                ++character->stats.birdsKilled;
                            }

                            // is bat carrying character? If so, rescue character.
                            auto matching = from(m->cjb) >> ref() >> where([&](auto && cjb) { return cjb.get().b == &b; });
                            if (matching >> any()) {
                                auto & cjb = (matching >> first()).get();
                                m->cjb.erase(cjb);
                                cjb.b->hasCaptive = false;
                                cjb.c->rescue();

                                if (characterCaused) ++character->stats.rescues;
                            }

                            batKilled(b.bird_type);
                            auto pos = b.position();
                            m->update_me(spawn_after(rand<double>(0.01, 0.2), [=]{
                                playCharacterExplosion(pos);
                            }));
                            m->removeWhenSpaceUnlocked(b);
                        }
                    }
                }
            }
        }
    };
    
    if (mode == m_menu) {
        m->update_me(spawn_after(0, [=]{
            show_menu();
        }));
    } else {
        m->setGravity(WORLD_GRAVITY);
        auto seg = [=] (vec2 v1, vec2 v2, CollisionType ct) { return m->sensor(m->segmentShape(v1, v2), ct); };
        m->abyssWalls[0] = seg({-10, top    }, {-10, -10    }, ct_abyss);  // left
        m->abyssWalls[1] = seg({ 10, top    }, { 10, -10    }, ct_abyss);  // right
        m->abyssWalls[2] = seg({-10, top    }, { 10, top    }, ct_abyss);  // top
        m->startleLine   = seg({-10, top - 1}, { 10, top - 1}, ct_startle);
        m->leftWall = m->segmentShape({-10, top}, {-10, -10});
        m->rightWall = m->segmentShape({10, top}, {10, -10});
        m->back_btn->setY(top - 0.8);
        m->pause_btn->setY(top - 0.78);

        //generateLevel();

        cpShapeSetCollisionType (&*m->ground, ct_ground);
        cpShapeSetFriction      (&*m->ground, 1);
        cpShapeSetFilter        (&*m->ground, {CP_NO_GROUP, cat_play, cat_play});

        cpShapeSetCollisionType (&*m->lslope, ct_barrier);
        cpShapeSetCollisionType (&*m->rslope, ct_barrier);
        cpShapeSetCollisionType (&*m->lbarrier, ct_barrier);
        cpShapeSetCollisionType (&*m->rbarrier, ct_barrier);

        cpShapeSetCollisionType (&*m->leftWall, ct_wall);
        cpShapeSetCollisionType (&*m->rightWall, ct_wall);
        cpShapeSetFilter(&*m->leftWall, {CP_NO_GROUP, cat_character, cat_character});
        cpShapeSetFilter(&*m->rightWall, {CP_NO_GROUP, cat_character, cat_character});
        cpShapeSetFriction(&*m->leftWall, 0.5);
        cpShapeSetFriction(&*m->rightWall, 0.5);

        auto createCharacter = [=](vec2 const v, int i) {
            auto & c = m->emplace<CharacterImpl>(i, v);
            auto & ps = m->emplace<PersonalSpaceImpl>(v);
            m->cps.insert(CharacterPersonalSpace{&c, ps.attachCharacterBody(*c.body()), &ps});
        };

        auto createCharacters = [=]{
            m->rem_chars = CHARACTERS;

            std::random_shuffle(std::begin(char_defs), std::end(char_defs)); // shuffle characters
            
            float min = -9;
            for (int i = 0; i < CHARACTERS; ++i) {
                float max = min + (18.0f / float(CHARACTERS));
                vec2 v{rand<float>(min + 0.5, max - 0.5), 2.3};
                createCharacter(v, i);
                min = max;
            }
            for (auto & c : m->actors<CharacterImpl>()) c.initState();
        };

        auto createBird = [=](BirdType type) {
            auto & b = m->emplace<BirdImpl>(type, vec2{rand<float>(-10, 10), rand<float>(top, top - 1)}, BIRD_SPEED);
            newTarget(b);
            if (type == bt_grey) ++m->created_grey_bats;
            else if (type == bt_yellow) ++m->created_yellow_bats;
        };

        auto createBombBat = [=] {
            auto STARTPOS = vec2{rand<float>(-10, 10), top + 2};
            auto CARROTPOS = vec2{clamp(rand<float>(STARTPOS.x - 5, STARTPOS.x + 5), -8, 8), rand<float>(top - 2.5, top - 5)};
            auto & bb = m->emplace<BombBatImpl>(STARTPOS, CARROTPOS);
            auto & bmb = m->emplace<BombImpl>(STARTPOS - vec2{0, 0.8});
            m->bjb.insert(BatJointBomb{&bb, bb.holdBomb(*bmb.body()), &bmb});
            auto & car = m->emplace<BombBatCarrotImpl>(CARROTPOS);
            m->bbcr.insert(BombBatCarrotRel{&bb, &car});
        };

        auto createCharacterRescueOpportunity = [=]() {
            // select random persona from deadChars, defaulted to 1 for testing - method should only be called after
            // at least one character has been kidnapped
            auto persona = m->deadChars.size() > 0 ? m->deadChars[rand<int>(0, static_cast<int>(m->deadChars.size()) - 1)] : 1;
            auto height = rand<float>(7.5, 9);
            auto dir = randomChoice({-1, 1});

            m->deadChars.erase(std::remove(m->deadChars.begin(), m->deadChars.end(), persona), m->deadChars.end());

            auto & b = m->emplace<BirdImpl>(bt_grey, vec2{static_cast<float>(-10 * dir), height}, 1.0f);
            b.hasCaptive = true; // prevent immediate bird/character collision

            // this is a dodgy way of avoiding the initial bird-abyss collision
            // that would immediately remove both impls. Not happy with this sol'n
            // but does the job for now.
            b.ignoreAbyss = true;
            m->update_me(spawn_after(5, [&]{
                b.ignoreAbyss = false;
            }));

            vec2 charPos = vec2{static_cast<float>(-10 * dir), static_cast<float>(height - 0.5)};

            auto & c = m->emplace<CharacterImpl>(persona, charPos);
            auto & ps = m->emplace<PersonalSpaceImpl>(charPos);
            m->cps.insert(CharacterPersonalSpace{&c, ps.attachCharacterBody(*c.body()), &ps});
            c.kidnap();
            m->cjb.insert(CharacterJointBird{&c, b.grabCharacter(*c.body()), &b});

            b.escapeVel = vec2{static_cast<float>(1.5 * dir), 0};

            for (auto & shape : c.shapes()) {
                auto filter = cpShapeGetFilter(&*shape);
                filter.group = gr_bird;
                cpShapeSetFilter(&*shape, filter);
            }

            ++m->rem_chars;
        };

        createCharacters();

        m->update_me(spawn_after(6, [=]{
            createBombBat();
            createCharacterRescueOpportunity();
            createBird(bt_grey);
        }));

        // create birds
        auto createBirds = [=](BirdType bird_type, double arrival_rate) {
            auto expSleep = [=] {
                auto sleep = sleeper(chan::spawn_killswitch(m->update_me(), -m->ticker_keepalive));

                std::random_device rd;
                std::mt19937 gen{rd()};

                const double log_scale = ARRIVAL_RATE_GROWTH_FACTOR * arrival_rate / log(2);

                return [=](double t) mutable {
                    double λ = log_scale * log(t + 2);
                    double dt = std::exponential_distribution<>{λ}(gen);
                    return sleep(dt) ? dt : 0;
                };
            }();

            spawn([=]() mutable {
                for (double t = 0; double dt = expSleep(t); t += dt) {
                    m->update_me(spawn_after(rand<double>(0, 1), [&] {
                        if (from(m->actors<CharacterImpl>()) >> any([&](auto && c) { return m->isKidnappable(c); })) {
                            if (bird_type == bt_bomb) {
                                createBombBat();
                            } else {
                                createBird(bird_type);
                            }
                        }
                    }));
                }
            });
        };

        createBirds(bt_grey, GREY_BAT_INITIAL_ARRIVAL_RATE);
        createBirds(bt_yellow, YELLOW_BAT_INITIAL_ARRIVAL_RATE);
        createBirds(bt_bomb, BOMB_BAT_INITIAL_ARRIVAL_RATE);

        // TEMPORARY: hard-coded creation of bomb bat
        /*m->update_me(spawn_after(4, [=]{
            auto STARTPOS = vec2{rand<float>(-10, 10), top + 2};
            auto CARROTPOS = vec2{clamp(rand<float>(STARTPOS.x - 5, STARTPOS.x + 5), -8, 8), rand<float>(top - 2.5, top - 5)};
            auto & bb = m->emplace<BombBatImpl>(STARTPOS, CARROTPOS);
            auto & bmb = m->emplace<BombImpl>(STARTPOS - vec2{0, 0.8});
            m->bjb.insert(BatJointBomb{&bb, bb.holdBomb(*bmb.body()), &bmb});
            auto & car = m->emplace<BombBatCarrotImpl>(CARROTPOS);
            m->bbcr.insert(BombBatCarrotRel{&bb, &car});
        }));*/
    }

    m->onCollision([=](BirdImpl & bird, CharacterImpl & character, cpArbiter * arb) {
        if (!(bird.canGrabCharacter() && m->isKidnappable(character) && cpArbiterIsFirstContact(arb))) {
            return false;
        }

        if (!(from(m->cjb) >> any([&](auto && cjb) { return cjb.b == &bird; }))) {
            character.kidnap();
            if (!character.isDead()) {
                help();
            }

            m->cjb.insert(CharacterJointBird{&character, bird.grabCharacter(*character.body()), &bird});
            m->targets >> removeIf([&](auto && target) { return target.b == &bird; });

            vec2 p = {rand<float>(-10, 10), rand<float>(top, top + 1)};
            bird.escapeVel = unit(p - bird.position());

            for (auto & shape : character.shapes()) {
                auto filter = cpShapeGetFilter(&*shape);
                filter.group = gr_bird;
                cpShapeSetFilter(&*shape, filter);
            }

            if (!m->anybodyLeft()) {
                m->update_me(spawn_after(0.5, [&]{
                    tension_stop();
                    lose();
                }));
                m->update_me(spawn_after(3.0, [&]{ gameOver(); }));

                /*spawn([=, sleep = sleeper(m->update_me())]{
                    if (sleep(0.5)) {
                        tension_stop();
                        lose();
                        //if (sleep(2)) { // <= 0) {
                            // NOT GETTING IN HERE AFTER CONTINUATION
                        //m->update_me(spawn_after(2, [&]{
                            gameOver();
                        //}));
                    }
                });*/
                for (auto & bat : m->actors<BirdImpl>()) {
                    if (!bat.hasCaptive) {
                        m->targets >> removeIf([&](auto && target) { return target.b == &bat; });
                        bat.fromWhence = true;
                        bat.setDesiredPos({rand<float>(-6, 6), rand<float>(top, top + 2)});
                    }
                }
            } else {
                try {
                    // send other birds after new target
                    from(m->targets) >> for_each([&](auto && targets) {
                        if (targets.c == &character && targets.b != &bird && targets.b->isFlying()) {
                            if (targets.b->position().y > ATTACK_LINE_Y) {
                                newTarget(*targets.b);
                            } else {
                                // flying too low to target new character
                                m->targets >> removeIf([&](auto && target) { return target.b == targets.b; });
                                vec2 atp{rand<float>(-6, 6), ATTACK_LINE_Y};
                                targets.b->setDesiredPos(atp);
                            }
                        }
                    });
                } catch(int e) {}
            }
        }
        return true;
    });

    m->onCollision([=](DartImpl & dart, BombImpl &, cpArbiter * arb) {
        return !m->game_over && dart.active;
    });

    m->onPostSolve([=](DartImpl & dart, BombImpl & bomb, cpArbiter * arb) {
        if (cpArbiterIsFirstContact(arb)) {
            ++m->playerStats.hits;

            // if bat is holding bomb, remove from BatJointBomb
            auto matching = from(m->bjb) >> mutable_ref() >> where([&](auto && bjb) { return bjb.get().bomb == &bomb; });
            if (matching >> any()) {
                auto & bjb = (matching >> first()).get();
                m->bjb.erase(bjb);

                // destroy bat (as he would be)
                for (auto & bat : m->actors<BombBatImpl>()) {
                    if (&bat == bjb.bat) {
                        auto pos = bat.position();
                        m->update_me(spawn_after(rand<double>(0.01, 0.2), [=]{
                            playCharacterExplosion(pos);
                        }));
                        m->bbcr >> removeIf([&](auto && bbcr) { return bbcr.bat == &bat; });
                        m->removeWhenSpaceUnlocked(bat);
                    }
                }
            }
            bomb.ticker_keepalive = {};

            // who caused the detonation?
            auto shooter = (from(m->actors<CharacterImpl>()) >> mutable_ref()
                            >> where([&](auto && c) { return m->firedDart(c, dart); }));
            if (shooter >> any()) {
                m->cdb >> removeIf([&](auto && cdb) { return cdb.bomb == &bomb; });
                auto & c = (shooter >> first()).get();
                m->cdb.insert(CharacterDetonatedBomb{&c, &bomb});
                ++c.stats.dartsHit;
            }

            detonate(bomb);
            m->removeWhenSpaceUnlocked(dart);
        }
    });

    m->onCollision([=](BombBatImpl & bat, BombBatCarrotImpl & carrot, cpArbiter * arb) {
        if (cpArbiterIsFirstContact(arb)) {
            if (from(m->bbcr) >> any([&](auto && bbcr) { return bbcr.bat == &bat && bbcr.carrot == &carrot; })) {
                // bat has reached carrot
                auto newCarrotPos = vec2{rand<float>(-8, 8), rand<float>(top - float(2.5), top - 5)};
                carrot.setPosition(newCarrotPos);
                bat.desired_pos = newCarrotPos;
            }
        }
        return false;
    });

    m->onCollision([=](BombBatImpl & bat, NoActor<ct_abyss> &, cpArbiter * arb) {
        if (cpArbiterIsFirstContact(arb)) {
            if (!(from(m->bjb) >> any([&](auto && bjb) { return bjb.bat == &bat; }))) {
                // bat is not carrying bomb, remove
                m->bbcr >> removeIf([&](auto && bbcr) { return bbcr.bat == &bat; });
                m->removeWhenSpaceUnlocked(bat);
            }
        }
    });

    m->onCollision([=](BombImpl & bomb, NoActor<ct_startle> &, cpArbiter * arb) {
        if (cpArbiterIsFirstContact(arb)) {
            if (from(m->bjb) >> any([&](auto && bjb) { return bjb.bomb == &bomb; })) {
                dundundun();
            }
        }
    });

    m->onCollision([=](BombBatImpl & bat, NoActor<ct_startle> &, cpArbiter * arb) {
        if (cpArbiterIsFirstContact(arb)) {
            auto matching = from(m->bjb) >> mutable_ref() >> where([&](auto && bjb) { return bjb.get().bat == &bat; });
            if (matching >> any()) {
                auto bjb = &(matching >> first()).get();

                // arm bomb
                bjb->bomb->countdown = 10; beep();
                reader<double> ticker = chan::spawn_quantize(bjb->bomb->update_me(), 1.0);
                bjb->bomb->ticker_keepalive = {};
                ticker = chan::spawn_killswitch(ticker, --bjb->bomb->ticker_keepalive); // killable
                spawn([=, bat = &bat, ticker = ticker]{
                    while (ticker >> nullptr) {
                        if (--bjb->bomb->countdown) {
                            tick();
                        } else {
                            m->bbcr >> removeIf([&](auto && bbcr) { return bbcr.bat == bat; });
                            bjb->bat->desired_pos = vec2{bjb->bat->position().x, 20};
                            bjb->bomb->offsetGravity = false; // allow application of gravity
                            m->bjb.erase(*bjb);
                            bombwhistle_start();
                            return;
                        }
                    }
                });
            }
        }
    });

    m->onCollision([=](DartImpl &, BombBatImpl & bat, cpArbiter * arb) {
        return !(bat.state() == BombBat::State::dying || m->game_over);
    });

    m->onPostSolve([=](DartImpl & dart, BombBatImpl & bat, cpArbiter * arb) {
        if (dart.active) {
            if (cpArbiterIsFirstContact(arb)) {
                ++m->playerStats.hits;
                //screech2();

                // increment character stats
                auto shooter = (from(m->actors<CharacterImpl>()) >> mutable_ref()
                                >> where([&](auto && c) { return m->firedDart(c, dart); }));
                if (shooter >> any()) {
                    auto & c = (shooter >> first()).get();
                    ++c.stats.dartsHit;
                }

                // if bat holding bomb, drop it
                auto matching = from(m->bjb) >> mutable_ref() >> where([&](auto && bjb) { return bjb.get().bat == &bat; });
                if (matching >> any()) {
                    m->bbcr >> removeIf([&](auto && bbcr) { return bbcr.bat == &bat; });
                    auto & bjb = (matching >> first()).get();
                    bjb.bomb->ticker_keepalive = {};
                    bjb.bomb->countdown = 0;
                    bjb.bomb->setVelocity({0, 0});
                    bjb.bomb->offsetGravity = false; // allow application of gravity
                    m->bjb.erase(bjb);
                    bombwhistle_start();

                    // who caused the detonation?
                    auto shooter = (from(m->actors<CharacterImpl>()) >> mutable_ref()
                                    >> where([&](auto && c) { return m->firedDart(c, dart); }));
                    if (shooter >> any()) {
                        auto & c = (shooter >> first()).get();
                        m->cdb.insert(CharacterDetonatedBomb{&c, bjb.bomb});
                    }
                }
                bat.setVelocity({0, 0});
                bat.setState(BombBat::State::dying); shot();
                m->update_me(spawn_after(0.6, [=, bat = &bat]{
                    m->removeWhenSpaceUnlocked(*bat);
                }));
                m->removeWhenSpaceUnlocked(dart);
            }
        }
    });

    m->onPostSolve([=](BombImpl & bomb, NoActor<ct_ground> &, cpArbiter * arb) {
        if (cpArbiterIsFirstContact(arb)) {
            detonate(bomb);
        }
    });

    m->onCollision([=](BombImpl & bomb, CharacterImpl & character, cpArbiter * arb) {
        return m->isKidnappable(character);
    });

    m->onPostSolve([=](BombImpl & bomb, CharacterImpl &, cpArbiter * arb) {
        if (cpArbiterIsFirstContact(arb)) {
            detonate(bomb);
        }
    });

    m->onCollision([=](DartImpl &, BirdImpl & bird, cpArbiter * arb) {
        if (!bird.canBeShot()) {
            return false;
        }
        return true;
    });

    m->onPostSolve([=](DartImpl & dart, BirdImpl & bird, cpArbiter * arb) {
        if (dart.active && !m->game_over) {
            ++m->playerStats.hits;
            shot();

            // Score hit against character
            auto shooter = (from(m->actors<CharacterImpl>()) >> mutable_ref()
                            >> where([&](auto && c) { return m->firedDart(c, dart); }));
            if (shooter >> any()) {
                auto & c = (shooter >> first()).get();
                ++c.stats.dartsHit;
                if (bird.resilience == 0) {
                    ++c.stats.birdsKilled;
                    c.stats.score += dart.score;
                    if (from(m->cjb) >> any([&](auto && cjb) { return cjb.b == &bird; })) {
                        ++c.stats.rescues;
                        c.stats.score += SCORE_CHAR_RESCUED;
                    }
                }
            }

            auto matching = from(m->cjb) >> ref() >> where([&](auto && cjb) { return cjb.get().b == &bird; });
            auto freeCaptive = [&]() {
                auto & cjb = (matching >> first()).get();
                cjb.b->dropCharacter();
                cjb.c->rescue();
                m->cjb.erase(cjb);
                cjb.b->hasCaptive = false;
                m->score += dart.score + SCORE_CHAR_RESCUED;
            };

            if (bird.resilience > 0) {
                bird.setAngle(0);
                bird.setVelocity({0, 0});

                --bird.resilience;

                if (matching >> any()) {
                    freeCaptive();
                    registerTextAlert(std::to_string(SCORE_CHAR_RESCUED), vec2{bird.position().x, bird.position().y + float(1)}, 1, 0.3);
                    newTarget(bird);
                }
            } else {
                m->targets >> removeIf([&](auto && target) { return target.b == &bird; });
                bird.setState(Bird::State::dying);

                m->update_me(spawn_after(0.2, [=]{
                    pumped();
                }));

                // free captive, if necessary
                if (matching >> any()) {
                    freeCaptive();
                    registerTextAlert(std::to_string(dart.score + SCORE_CHAR_RESCUED), vec2{bird.position().x, bird.position().y + float(1)}, 1, 0.3);
                } else {
                    m->score += dart.score; // SCORE_BIRD_KILLED;
                    registerTextAlert(std::to_string(dart.score), vec2{bird.position().x, bird.position().y + float(1)}, 1, 0.3);
                }
                
                bird.setAngle(0);
                bird.setVelocity({0, -4});
                cpBodySetAngularVelocity(bird.body(), 0);
                dart.setVelocity({0, 0});

                batKilled(bird.bird_type);
            }

            m->csd >> removeIf([&](auto && csd) { return csd.d == &dart; });
            m->removeWhenSpaceUnlocked(dart);
        }
    });

    m->onCollision([=](DartImpl & dart, NoActor<ct_ground> &, cpArbiter *) {
        dart.active = false;
        float angle = brac::angle(dart.velocity());
        m->whenSpaceUnlocked([&] {
            cpSpaceRemoveBody(dart.space(), dart.body());
            cpBodySetType(dart.body(), CP_BODY_TYPE_STATIC);
            dart.setAngle(angle);
        }, &dart);
    });

    m->onCollision([=](DartImpl & dart, CharacterImpl & character, cpArbiter * arb) {
        if (dart.active && !m->game_over) {
            // Score friendly fire against character
            auto shooter = (from(m->actors<CharacterImpl>()) >> mutable_ref()
                            >> where([&](auto && c) { return m->firedDart(c, dart); }));
            if (shooter >> any()) {
                auto & c = (shooter >> first()).get();
                ++c.stats.friendlies;
            }

            m->csd >> removeIf([&](auto && csd) { return csd.d == &dart; });
            m->removeWhenSpaceUnlocked(dart);

            if (!character.isDead()) {
                die();
                character.setState(Character::State::dead);
                character.setVelocity({0, 1});

                characterKilled();

                retargetBirdsChasingMe(character);
            }
        }
        return false;
    });

    m->onCollision([=](CharacterImpl & character, NoActor<ct_ground> &, cpArbiter * arb) {
        return character.velocity().y < 1;
    });

    m->onPostSolve([=](CharacterImpl & character, NoActor<ct_ground> &, cpArbiter * arb) {
        if (cpArbiterIsFirstContact(arb)) {
            character.setAngle(0);
            cpBodySetAngularVelocity(character.body(), 0);
            switch (character.state()) {
                case Character::State::startled:
                    character.setState(Character::State::determined);
                    m->update_me(spawn_after(rand<double>(0.1, 0.8), [&]{
                        character.reload();
                    }));
                    break;
                case Character::State::rescued:
                    character.reload();
                    targetCharacter(character);
                    stopyays();
                    tension_start();
                    break;
                case Character::State::celebrating:
                    yay();
                    //character.setVelocity({0, rand<float>(3, 4.5)});
                    character.setVelocity({0, rand<float>(6, 8.5)});
                    break;
                case Character::State::ready:
                    character.setVelocity({0, 2});
                    break;
                case Character::State::dead:
                    if (!m->isCaptive(character)) {
                        character.setVelocity({0, 0});
                        m->emplace<GraveImpl>(vec2{character.position().x, 2.6});
                        removeCharacter(character);
                    }
                    break;
                default:
                    break;
            }
        }
    });

    m->onCollision([=](BirdImpl & bird, NoActor<ct_ground> &, cpArbiter * arb) {
        if (cpArbiterIsFirstContact(arb)) {
            if (bird.state() == Bird::State::dying) {
                bird.setState(Bird::State::puff);
                bird.setVelocity({0, 0});
                m->update_me(spawn_after(0.85, [=, bird = &bird]{
                    m->removeWhenSpaceUnlocked(*bird);
                }));
                fall();
            } else {
                // bird has missed all targets and hit ground; send back to attack line
                m->targets >> removeIf([&](auto && target) { return target.b == &bird; });
                vec2 atp{rand<float>(-6, 6), ATTACK_LINE_Y};
                bird.setDesiredPos(atp);
            }
        }
        return true;
    });

    m->onCollision([=](BirdImpl & bird, NoActor<ct_attack> &) {
        if (!(from(m->targets) >> any([&](auto && t) { return t.b == &bird; })) &&
            !m->hasCaptive(bird) &&
            bird.state() != Bird::State::dying &&
            !bird.fromWhence) {
            newTarget(bird);
        }
    });

    m->onCollision([=](BirdImpl & bird, NoActor<ct_startle> &) {
        if (!m->started) {
            m->watch.start();
            for (auto & c : m->actors<CharacterImpl>()) {
                //aah();
                c.startle();
                tension_start();
            }
            m->started = true;
        }
    });

    m->onCollision([=](DartImpl &, NoActor<ct_barrier> &) {
        return false;
    });

    m->onCollision([=](BirdImpl &, NoActor<ct_barrier> &) {
        return false;
    });

    m->onCollision([=](CharacterImpl & c, NoActor<ct_wall> &) {
        // prevent rescued character from flying off the screen
        return (!c.isDead() &&
                !(from(m->cjb) >> any([&](auto && cjb) { return cjb.c == &c; })));
    });

    m->onCollision([=](CharacterImpl &, BombBatImpl &) {
        return false;
    });

    m->onCollision([=](CharacterImpl &, BombImpl &) {
        return false;
    });

    m->onCollision([=](BirdImpl &, BombBatImpl &) {
        return false;
    });

    m->onCollision([=](BirdImpl &, BombImpl &) {
        return false;
    });

    m->onCollision([=](PersonalSpaceImpl & ps1, PersonalSpaceImpl & ps2, cpArbiter *) {
        auto whosePersonalSpace = [=](PersonalSpaceImpl & ps) {
            auto matching1 = from(m->cps) >> ref() >> where([&](auto && cps) { return cps.get().ps == &ps; });
            auto & cps1 = (matching1 >> first()).get();
            return cps1.c;
        };

        auto c1 = whosePersonalSpace(ps1);
        auto c2 = whosePersonalSpace(ps2);

        // return false if either character is currently kidnapped
        return !((from(m->cjb) >> any([&](auto && cjb) { return cjb.c == c1; }))
                 || (from(m->cjb) >> any([&](auto && cjb) { return cjb.c == c2; })));
    });

    m->onCollision([=](BirdImpl & bird, NoActor<ct_abyss> &, cpArbiter * arb) {
        auto forceBirdReplace = [&] {
            if (bird.bird_type == bt_grey) {
                --m->created_grey_bats;
            } else if (bird.bird_type == bt_yellow) {
                --m->created_yellow_bats;
            }
        };

        if (cpArbiterIsFirstContact(arb) && !bird.ignoreAbyss) {
            auto matching = from(m->cjb) >> ref() >> where([&](auto && cjb) { return cjb.get().b == &bird; });
            if (matching >> any()) {
                auto & cjb = (matching >> first()).get();
                m->cjb.erase(cjb);
                //m->removeWhenSpaceUnlockedIf(from (m->actors<CharacterImpl>()) >> where([&](auto && c) { return &c == cjb.c; }));
                for (auto & c : m->actors<CharacterImpl>()) {
                    if (&c == cjb.c) {
                        removeCharacter(c);
                        if (!c.isDead()) {
                            characterKilled();
                        }
                    }
                }
                m->targets >> removeIf([&](auto && target) { return target.b == &bird; });
                m->removeWhenSpaceUnlocked(bird);
                forceBirdReplace();
            } else {
                if (bird.fromWhence) {
                    m->targets >> removeIf([&](auto && target) { return target.b == &bird; });
                    forceBirdReplace();
                    m->removeWhenSpaceUnlocked(bird);
                }
            }
        }
    });
}

void Game::gameOver() {
    //m->ticker_keepalive = {};
    m->game_over = true;
    m->watch.stop();
    m->playerStats.time = m->watch.time();
    //m->playerStats.remCharacters = m->rem_chars;
    //m->score += m->rem_chars * SCORE_CHAR_SURVIVED;
    for (auto & c : m->actors<CharacterImpl>()) {
        archiveCharacterStats(c.stats);
    }

    tension_stop();
    //failed();
    ended();
    //end();
}

void Game::continueGame() {
    m->game_over = false;
    //m->watch.start();

    auto newCharacter = [=](int persona, vec2 pos) {
        auto & c = m->emplace<CharacterImpl>(persona, pos);
        auto & ps = m->emplace<PersonalSpaceImpl>(pos);
        m->cps.insert(CharacterPersonalSpace{&c, ps.attachCharacterBody(*c.body()), &ps});
        c.setState(Character::State::rescued);
        cpBodyApplyImpulseAtLocalPoint(c.body(), to_cpVect({0, 20}), to_cpVect({0, 0}));
        cpBodySetAngularVelocity(c.body(), 20);
        ++m->rem_chars;
    };

    std::random_shuffle(std::begin(char_defs), std::end(char_defs)); // shuffle characters
    newCharacter(1, {-7, 0});
    newCharacter(2, {-2.5, 0});
    newCharacter(3, {2.5, 0});
    newCharacter(4, {7, 0});
    m->popup("GAME ON", vec2{0, 7}, 3, 1);

    m->update_me(spawn_after(0.1, [&]{ yay2(); }));
    m->update_me(spawn_after(0.2, [&]{ yay6(); }));
    m->update_me(spawn_after(0.3, [&]{ yay4(); }));
    m->update_me(spawn_after(0.4, [&]{ yay5(); }));
    m->update_me(spawn_after(0.2, [&]{ yay1(); }));
    m->update_me(spawn_after(0.7, [&]{ yay3(); }));
    m->update_me(spawn_after(0.9, [&]{ yay7(); }));
    m->update_me(spawn_after(0.4, [&]{ yay8(); }));

    //tension_start();
}

void Game::archiveCharacterStats(CharacterStats s) {
    m->characterStats.emplace_back(s);
}

Game::~Game() { }

Game::State const & Game::state() const { return *m; }

TouchHandler Game::fingerTouch(vec2 const & p, float radius) {
    if (auto backHandler = m->back_btn->handleTouch(p)) {
        return backHandler;
    }
    if (auto pauseHandler = m->pause_btn->handleTouch(p)) {
        return pauseHandler;
    }
    if (auto playHandler = m->play_btn->handleTouch(p)) {
        return playHandler;
    }

    CharacterImpl * character = nullptr;
    float dist_sq = INFINITY;
    AabbQuery(m->spaceTime.space(),
              cpBBNewForCircle(to_cpVect(p), radius),
              {CP_NO_GROUP, cat_character, cat_character},
              [&](cpShape *shape) {
                  if (auto c = static_cast<CharacterImpl *>(cpShapeGetUserData(shape))) {
                      float dsq = length_sq(p - c->position());
                      if (dist_sq > dsq) {
                          dist_sq = dsq;
                          character = c;
                      }
                  }
              });

    if (character) {
        if (character->readyToFire()) {
            return spawn_touchHandler([=, weak_self = std::weak_ptr<Game>{shared_from_this()}](auto moved, auto cancelled) {
                vec2 first_p = p;

                for (TouchMove m; moved >> m;) {
                    if (auto self = weak_self.lock()) {
                        if (self->m->isCaptive(*character)) {
                            // captured; end touch event
                            return;
                        } else {
                            auto v = first_p - m.pos;
                            character->setState(Character::State::aim);
                            //self->aim();
                            if (v) {
                                character->aim((14 * rcp_length(v) + 2) * v);
                            }
                        }
                    }
                }
                if (!(cancelled >> poke)) {
                    if (auto self = weak_self.lock()) {
                        // TODO: Return smoothly to upright posture.
                        if (character->isAiming()) {
                            if (vec2 const & vel = character->launchVel()) {
                                auto & dart = self->m->emplace<DartImpl>(character->position() + LAUNCH_OFFSET * unit(vel), vel);
                                self->m->csd.insert(CharacterShotDart{character, &dart});
                                character->shoot();
                                ++self->m->playerStats.darts;
                                self->m->score += SCORE_DART_FIRED;
                                self->shoot();
                            }
                        }
                    }
                }
            });
        } else {
            if (character->state() == Character::State::reloading) {
                reloading();
            }
        }
    }

    return {};
}

void Game::doUpdate(float dt) {
    m->update(dt);
    m->dt += dt;
}

void Game::getActors(size_t actorId, void * buf) const { m->getActorsForController(actorId, buf); }
