#include "specific/standard.h"
#include "game.h"

extern sint32 flipmap[10];
extern char cd_flags[128];
extern sint32 SunsetTimer;
extern TEXTSTRING *ammotext;
extern sint32 monks_attack_lara;

extern sint32 compy_scared_timer;
extern sint32 compys_attack_lara;
extern sint16 CarcassItem;

void InitialiseGameFlags(void)
{
    int i;

    flip_status = 0;
    memset(flipmap, 0, sizeof(flipmap));
    memset(cd_flags, 0, sizeof(cd_flags));
    for (i = 0; i < NUMBER_OBJECTS; i++)
        objects[i].loaded = 0;
    SunsetTimer = 0;
    ammotext = NULL;
    level_complete = 0;
    flipeffect = -1;
    monks_attack_lara = 0;
    CarcassItem = NO_ITEM;
    compys_attack_lara = 0;
}

void InitialiseLevelFlags(void)
{
    savegame.secrets = 0;
    savegame.timer = 0;
    savegame.kills = 0;
    savegame.distance_travelled = 0;
    savegame.ammo_used = 0;
    savegame.ammo_hit = 0;
    savegame.health_used = 0;
    assault_timer_active = 0;
    QuadbikeLapTime = QuadbikeLapTimeDisplayTimer = 0;
    compy_scared_timer = 0;
    compys_attack_lara = 0;
    CarcassItem = -1;
}

void BaddyObjects(void);
void TrapObjects(void);
void ObjectObjects(void);
void InitialiseSparks(void);

void InitialiseObjects(void)
{
    int i;
    OBJECT_INFO *obj;

    for (i = 0, obj = objects; i < NUMBER_OBJECTS; i++, obj++) {
        obj->initialise = NULL;
        obj->collision = NULL;
        obj->control = NULL;
        obj->draw_routine = DrawAnimatingItem;
        obj->floor = obj->ceiling = NULL;
        obj->pivot_length = 0;
        obj->radius = 10;
        obj->shadow_size = 0;
        obj->hit_points = DONT_TARGET;
        obj->intelligent = obj->water_creature = 0;
        obj->save_position = obj->save_hitpoints = obj->save_flags = obj->save_anim = 0;
    }

    BaddyObjects();
    TrapObjects();
    ObjectObjects();
    InitialiseHair();
    InitialiseSparks();
}

void MiniCopterControl(sint16 item_number);
void GeneralControl(sint16 item_number);
void ObjectCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void InitialiseRollingBall(sint16 item_number);
void ControlDeathSlide(sint16 item_number);
void DeathSlideCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void ControlSpikeWall(sint16 item_number);
void ControlCeilingSpikes(sint16 item_number);
void TrapCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void HookControl(sint16 item_number);
void CreatureCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void PropellerControl(sint16 item_number);
void InitialiseKillerStatue(sint16 item_number);
void SpinningBlade(sint16 item_number);
void IcicleControl(sint16 item_number);
void InitialiseBlade(sint16 item_number);
void BladeControl(sint16 item_number);
void SpringBoardControl(sint16 item_number);
void FallingBlock(sint16 item_number);
void FallingBlockFloor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void FallingBlockCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void Pendulum(sint16 item_number);
void TeethTrap(sint16 item_number);
void RollingBallControl(sint16 item_number);
void RollingBallCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void SpikeControl(sint16 item_number);
void SpikeCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void FallingCeiling(sint16 item_number);
void DartEmitterControl(sint16 item_number);
void InitialiseRaptorEmitter(sint16 item_number);
void RaptorEmitterControl(sint16 item_number);
void FlyEmitterControl(sint16 item_number);
void BoomuteControl(sint16 item_number);
void ControlArea51Rocket(sint16 item_number);
void InitialiseArea51Struts(sint16 item_number);
void ControlArea51Struts(sint16 item_number);
void InitialiseArea51Laser(sint16 item_number);
void ControlArea51Laser(sint16 item_number);
void DartsControl(sint16 item_number);
void S_DrawDarts(ITEM_INFO *item);
void FlameEmitterControl(sint16 item_number);
void FlameEmitter2Control(sint16 item_number);
void FlameEmitter3Control(sint16 item_number);
void SideFlameEmitterControl(sint16 item_number);
void FlameControl(sint16 item_number);

void DrawDummyItem(ITEM_INFO *item);
void InitialiseMovingBlock(sint16 item_number);
void MovableBlock(sint16 item_number);
void MovableBlockCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void DrawMovableBlock(ITEM_INFO *item);

void TrapObjects(void)
{
    OBJECT_INFO *obj;
    int i;

    obj = &objects[24];
    obj->hit_points = 0;
    obj->draw_routine = DrawDummyItem;
    obj->control = KillAllCurrentItems;
    obj->save_flags = 1;

    obj = &objects[359];
    obj->control = MiniCopterControl;
    obj->save_position = 1;
    obj->save_flags = 1;

    obj = &objects[96];
    obj->collision = ObjectCollision;
    obj->control = GeneralControl;
    obj->save_flags = 1;
    obj->save_anim = 1;
    obj->water_creature = 1;

    obj = &objects[127];
    obj->initialise = InitialiseRollingBall;
    obj->collision = DeathSlideCollision;
    obj->control = ControlDeathSlide;
    obj->save_flags = 1;
    obj->save_anim = 1;
    obj->save_position = 1;

    obj = &objects[114];
    obj->control = ControlSpikeWall;
    obj->collision = ObjectCollision;
    obj->save_flags = 1;
    obj->save_position = 1;

    obj = &objects[116];
    obj->control = ControlCeilingSpikes;
    obj->collision = TrapCollision;
    obj->save_flags = 1;
    obj->save_position = 1;

    obj = &objects[106];
    obj->control = HookControl;
    obj->collision = CreatureCollision;
    obj->save_anim = 1;
    obj->save_flags = 1;

    obj = &objects[105];
    obj->control = PropellerControl;
    obj->collision = ObjectCollision;
    obj->save_anim = 1;
    obj->save_flags = 1;

    obj = &objects[119];
    obj->control = PropellerControl;
    obj->collision = TrapCollision;
    obj->save_anim = 1;
    obj->water_creature = 1;
    obj->save_flags = 1;

    obj = &objects[120];
    obj->control = PropellerControl;
    obj->collision = TrapCollision;
    obj->save_anim = 1;
    obj->save_flags = 1;

    obj = &objects[108];
    obj->initialise = InitialiseKillerStatue;
    obj->control = SpinningBlade;
    obj->collision = ObjectCollision;
    obj->save_flags = 1;
    obj->save_anim = 1;
    obj->save_position = 1;

    obj = &objects[113];
    obj->control = IcicleControl;
    obj->collision = TrapCollision;
    obj->save_flags = 1;
    obj->save_anim = 1;
    obj->save_position = 1;

    obj = &objects[111];
    obj->initialise = InitialiseBlade;
    obj->control = BladeControl;
    obj->collision = TrapCollision;
    obj->save_flags = 1;
    obj->save_anim = 1;

    obj = &objects[115];
    obj->control = SpringBoardControl;
    obj->save_flags = 1;
    obj->save_anim = 1;

    obj = &objects[83];
    obj->floor = FallingBlockFloor;
    obj->ceiling = FallingBlockCeiling;
    obj->control = FallingBlock;
    obj->save_anim = 1;
    obj->save_position = 1;
    obj->save_flags = 1;

    obj = &objects[84];
    obj->floor = FallingBlockFloor;
    obj->ceiling = FallingBlockCeiling;
    obj->control = FallingBlock;
    obj->save_anim = 1;
    obj->save_position = 1;
    obj->save_flags = 1;

    obj = &objects[85];
    obj->floor = FallingBlockFloor;
    obj->ceiling = FallingBlockCeiling;
    obj->control = FallingBlock;
    obj->save_anim = 1;
    obj->save_position = 1;
    obj->save_flags = 1;

    obj = &objects[86];
    obj->shadow_size = 128;
    obj->control = Pendulum;
    obj->collision = ObjectCollision;
    obj->save_flags = 1;
    obj->save_anim = 1;

    obj = &objects[121];
    obj->shadow_size = 128;
    obj->control = Pendulum;
    obj->collision = ObjectCollision;
    obj->save_flags = 1;
    obj->save_anim = 1;

    obj = &objects[94];
    obj->control = TeethTrap;
    obj->collision = TrapCollision;
    obj->save_flags = 1;
    obj->save_anim = 1;

    obj = &objects[112];
    obj->collision = RollingBallCollision;
    obj->control = RollingBallControl;
    obj->initialise = InitialiseRollingBall;
    obj->save_flags = 1;
    obj->save_position = 1;
    obj->save_anim = 1;

    obj = &objects[126];
    obj->collision = RollingBallCollision;
    obj->control = RollingBallControl;
    obj->initialise = InitialiseRollingBall;
    obj->save_flags = 1;
    obj->save_position = 1;
    obj->save_anim = 1;

    obj = &objects[88];
    obj->collision = RollingBallCollision;
    obj->control = RollingBallControl;
    obj->initialise = InitialiseRollingBall;
    obj->save_flags = 1;
    obj->save_position = 1;
    obj->save_anim = 1;

    obj = &objects[89];
    obj->collision = RollingBallCollision;
    obj->control = RollingBallControl;
    obj->initialise = InitialiseRollingBall;
    obj->save_flags = 1;
    obj->save_position = 1;
    obj->save_anim = 1;

    obj = &objects[87];
    obj->control = SpikeControl;
    obj->collision = SpikeCollision;

    obj = &objects[107];
    obj->control = FallingCeiling;
    obj->collision = TrapCollision;
    obj->save_anim = 1;
    obj->save_position = 1;
    obj->save_flags = 1;
    for (i = 97; i <= 100; i++) {
        obj = &objects[i];
        obj->initialise = InitialiseMovingBlock;
        obj->control = MovableBlock;
        obj->collision = MovableBlockCollision;
        obj->draw_routine = DrawMovableBlock;
        obj->save_position = 1;
        obj->save_flags = 1;
        obj->save_anim = 1;
    }

    obj = &objects[91];
    obj->draw_routine = DrawDummyItem;
    obj->control = DartEmitterControl;
    obj->save_flags = 1;

    obj = &objects[92];
    obj->draw_routine = DrawDummyItem;
    obj->control = DartEmitterControl;
    obj->save_flags = 1;

    obj = &objects[48];
    obj->draw_routine = DrawDummyItem;
    obj->initialise = InitialiseRaptorEmitter;
    obj->control = RaptorEmitterControl;
    obj->save_flags = 1;

    obj = &objects[47];
    obj->draw_routine = DrawDummyItem;
    obj->control = FlyEmitterControl;
    obj->hit_points = 150;
    obj->save_flags = 1;

    obj = &objects[31];
    obj->control = BoomuteControl;
    obj->save_anim = 1;
    obj->save_position = 1;
    obj->save_flags = 1;

    obj = &objects[312];
    obj->control = ControlArea51Rocket;
    obj->save_anim = 1;
    obj->save_position = 1;
    obj->save_flags = 1;

    obj = &objects[313];
    obj->draw_routine = DrawDummyItem;
    obj->control = ControlArea51Rocket;
    obj->save_anim = 1;
    obj->save_flags = 1;

    obj = &objects[314];
    obj->control = ControlArea51Struts;
    obj->initialise = InitialiseArea51Struts;
    obj->save_anim = 1;

    obj = &objects[291];
    obj->control = ControlArea51Laser;
    obj->initialise = InitialiseArea51Laser;
    obj->save_anim = 1;
    obj->save_position = 1;
    obj->save_flags = 1;

    obj = &objects[90];
    obj->control = DartsControl;
    obj->draw_routine = S_DrawDarts;
    obj->collision = ObjectCollision;
    obj->shadow_size = 128;

    obj = &objects[330];
    obj->draw_routine = DrawDummyItem;
    obj->control = FlameEmitterControl;
    obj->save_flags = 1;

    obj = &objects[331];
    obj->draw_routine = DrawDummyItem;
    obj->control = FlameEmitter2Control;
    obj->save_flags = 1;

    obj = &objects[332];
    obj->draw_routine = DrawDummyItem;
    obj->control = FlameEmitter3Control;
    obj->save_flags = 1;

    obj = &objects[333];
    obj->draw_routine = DrawDummyItem;
    obj->control = SideFlameEmitterControl;
    obj->save_flags = 1;

    obj = &objects[329];
    obj->draw_routine = DrawDummyItem;
    obj->control = FlameControl;
}

extern sint32 level_items;

void PickUpCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void RemoveDrawnItem(sint16 item_num);

void GetCarriedItems(void)
{
    int i;
    sint16 pickup_number;
    ITEM_INFO *item, *pickup;

    for (i = 0, item = items; i < level_items; i++, item++) {
        if (objects[item->object_number].intelligent) {
            item->carried_item = NO_ITEM;
            pickup_number = room[item->room_number].item_number;
            do {
                pickup = &items[pickup_number];
                if (pickup->pos.x_pos == item->pos.x_pos && pickup->pos.y_pos == item->pos.y_pos && pickup->pos.z_pos == item->pos.z_pos && objects[pickup->object_number].collision == PickUpCollision) {
                    pickup->carried_item = item->carried_item;
                    item->carried_item = pickup_number;
                    RemoveDrawnItem(pickup_number);
                    pickup->room_number = NO_ROOM;
                }
                pickup_number = pickup->next_item;
            } while (pickup_number != NO_ITEM);
        }
    }
}

extern uint32 CurrentLevel;

void DrawDummyItem(ITEM_INFO *item);

void GetAIPickups(void)
{
    int i;
    sint16 pickup_number;
    ITEM_INFO *item, *pickup;

    for (i = 0, item = items; i < level_items; i++, item++) {
        if (objects[item->object_number].intelligent) {
            item->ai_bits = 0;
            pickup_number = room[item->room_number].item_number;
            do {
                pickup = &items[pickup_number];
                if (pickup->pos.x_pos == item->pos.x_pos && pickup->pos.z_pos == item->pos.z_pos &&
                    objects[pickup->object_number].collision == (void *)DrawDummyItem && pickup->object_number < 79) {
                    item->ai_bits |= 1 << (pickup->object_number - 74);
                    item->item_flags[3] = pickup->pos.y_rot;
                    if (!(pickup->object_number == 76 && (CurrentLevel == 15 || CurrentLevel == 14))) {
                        KillItem(pickup_number);
                        pickup->room_number = NO_ROOM;
                    }
                } else if (pickup->object_number == 330 && item->object_number == PUNK1) {
                    item->item_flags[2] = 3;
                    KillItem(pickup_number);
                    pickup->room_number = NO_ROOM;
                }
                pickup_number = pickup->next_item;
            } while (pickup_number != NO_ITEM);
        }
    }
}
