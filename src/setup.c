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

extern uint8 *OutsideRoomTable;
extern sint16 OutsideRoomOffsets[729];

void game_free(int size, int type);
int printf(const char *format, ...);
int memcmp(const void *buf1, const void *buf2, unsigned int count);
#pragma intrinsic(memcmp)

void BuildOutsideTable(void)
{
    uint8 *s;
    int x, y, i;
    sint32 max_slots = 0;

    OutsideRoomTable = (uint8 *)game_malloc(27 * 27 * 64, 0);
    memset(OutsideRoomTable, 0xff, 27 * 27 * 64);
    printf("X %d, Y %d, Z %d, Xs %d, Ys %d\n", room[0].x, room[0].y, room[0].z, room[0].x_size, room[0].y_size);
    for (y = 0; y < 108; y += 4) {
        for (x = 0; x < 108; x += 4) {
            for (i = 0; i < number_rooms; i++) {
                uint8 *d;
                int rx, ry, j, xl, yl;
                ROOM_INFO *r = &room[i];

                rx = (r->z >> 10) + 1;
                ry = (r->x >> 10) + 1;
                j = 0;
                for (yl = 0; yl < 4; yl++) {
                    for (xl = 0; xl < 4; xl++) {
                        if (x + xl >= rx && x + xl < rx + r->x_size - 2 && y + yl >= ry && y + yl < ry + r->y_size - 2) {
                            j = 1;
                            break;
                        }
                    }
                }
                if (!j)
                    continue;
                d = &OutsideRoomTable[(x >> 2) * 64 + (y >> 2) * (27 * 64)];
                for (j = 0; j < 64; j++) {
                    if (d[j] == 0xff) {
                        d[j] = i;
                        if (j > max_slots)
                            max_slots = j;
                        break;
                    }
                }
            }
        }
    }
    s = OutsideRoomTable;
    for (y = 0; y < 27; y++) {
        for (x = 0; x < 27; x++) {
            int z = 0;
            uint8 *d, *p;

            i = y * 27 + x;
            d = &OutsideRoomTable[x * 64 + y * (27 * 64)];
            while (d[z] != 0xff)
                z++;
            if (z == 0) {
                OutsideRoomOffsets[i] = -1;
            } else if (z == 1) {
                OutsideRoomOffsets[i] = *d | 0x8000;
            } else {
                p = OutsideRoomTable;
                while (p < s) {
                    if (memcmp(p, d, z) == 0) {
                        OutsideRoomOffsets[i] = (sint32)p - (sint32)OutsideRoomTable;
                        break;
                    } else {
                        int z2 = 0;

                        while (p[z2] != 0xff)
                            z2++;
                        p += z2 + 1;
                    }
                }
                if (p >= s) {
                    OutsideRoomOffsets[i] = (sint32)s - (sint32)OutsideRoomTable;
                    for (; z; z--)
                        *s++ = *d++;
                    *s++ = 0xff;
                }
            }
        }
    }
    game_free(((27 * 27 * 64) - ((sint32)s - (sint32)OutsideRoomTable) + 3) & ~3, 0);
    printf("Ouside room table = %d bytes, max_slots = %d\n", (sint32)s - (sint32)OutsideRoomTable, max_slots);
}

extern sint32 *bones;
void DrawDummyItem(ITEM_INFO *item);
void CreatureCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void ObjectCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void AIPickupCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void InitialiseLaraLoad(sint16 item_number);
void ControlLaraExtra(sint16 item_number);
void InitialiseCobra(sint16 item_number);
void CobraControl(sint16 item_number);
void DrawMonkey(ITEM_INFO *item);
void InitialiseMonkey(sint16 item_number);
void MonkeyControl(sint16 item_number);
void InitialiseAutogun(sint16 item_number);
void AutogunControl(sint16 item_number);
void DiverControl(sint16 item_number);
void OrcaControl(sint16 item_number);
void DinoControl(sint16 item_number);
void RaptorControl(sint16 item_number);
void InitialiseDog(sint16 item_number);
void DogControl(sint16 item_number);
void MouseControl(sint16 item_number);
void InitialiseCompy(sint16 item_number);
void CompyControl(sint16 item_number);
void InitialiseTribeBoss(sint16 item_number);
void TribeBossControl(sint16 item_number);
void S_DrawTribeBoss(ITEM_INFO *item);
void InitialiseTonyBoss(sint16 item_number);
void TonyBossControl(sint16 item_number);
void S_DrawTonyBoss(ITEM_INFO *item);
void InitialiseLondonBoss(sint16 item_number);
void LondonBossControl(sint16 item_number);
void S_DrawLondonBoss(ITEM_INFO *item);
void InitialiseWillBoss(sint16 item_number);
void WillBossControl(sint16 item_number);
void S_DrawWillBoss(ITEM_INFO *item);
void TigerControl(sint16 item_number);
void InitialiseShiva(sint16 item_number);
void DrawShiva(ITEM_INFO *item);
void ShivaControl(sint16 item_number);
void InitialiseOilSMG(sint16 item_number);
void OilSMGControl(sint16 item_number);
void ClawmuteControl(sint16 item_number);
void HybridControl(sint16 item_number);
void SealmuteControl(sint16 item_number);
void InitialiseSwat(sint16 item_number);
void SwatControl(sint16 item_number);
void InitialiseArmySMG(sint16 item_number);
void ArmySMGControl(sint16 item_number);
void LizManControl(sint16 item_number);
void InitialiseBaton(sint16 item_number);
void BatonControl(sint16 item_number);
void InitialiseCivvy(sint16 item_number);
void CivvyControl(sint16 item_number);
void InitialisePrisoner(sint16 item_number);
void PrisonerControl(sint16 item_number);
void FlamerControl(sint16 item_number);
void TribeAxeControl(sint16 item_number);
void BlowpipeControl(sint16 item_number);
void InitialiseWingmute(sint16 item_number);
void WingmuteControl(sint16 item_number);
void InitialiseVulture(sint16 item_number);
void VultureControl(sint16 item_number);
void OilRedControl(sint16 item_number);
void LondSecControl(sint16 item_number);
void InitialiseCleaner(sint16 item_number);
void CleanerControl(sint16 item_number);
void MPGunControl(sint16 item_number);
void InitialisePunk(sint16 item_number);
void PunkControl(sint16 item_number);
void CrocControl(sint16 item_number);
void OldWinstonControl(sint16 item_number);
void WinstonControl(sint16 item_number);
void TargetControl(sint16 item_number);

void BaddyObjects(void)
{
    OBJECT_INFO *obj;

    obj = &objects[0];
    obj->initialise = InitialiseLaraLoad;
    obj->shadow_size = 160;
    obj->hit_points = 1000;
    obj->draw_routine = DrawDummyItem;
    obj->save_position = obj->save_hitpoints = 1;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[13];
    obj->control = ControlLaraExtra;

    obj = &objects[69];
    if (obj->loaded) {
        obj->initialise = InitialiseCobra;
        obj->control = CobraControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 8;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->non_lot = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        bones[obj->bone_index] |= 8;
        bones[obj->bone_index + 24] |= 8;
    }

    obj = &objects[71];
    if (obj->loaded) {
        if (!objects[123].loaded)
            S_ExitSystem("FATAL: Monkey requires MESHSWAP2 (Monkey + Pickups)");
        obj->draw_routine = DrawMonkey;
        obj->initialise = InitialiseMonkey;
        obj->control = MonkeyControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 8;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 0;
        bones[obj->bone_index] |= 16;
        bones[obj->bone_index + 28] |= 4;
        bones[obj->bone_index + 28] |= 8;
    }

    obj = &objects[64];
    if (obj->loaded) {
        obj->initialise = InitialiseAutogun;
        obj->control = AutogunControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 0;
        obj->hit_points = 100;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->non_lot = 1;
        obj->bite_offset = 4;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        bones[obj->bone_index] |= 8;
        bones[obj->bone_index + 4] |= 4;
    }

    obj = &objects[26];
    if (obj->loaded) {
        obj->control = DiverControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 20;
        obj->radius = 341;
        obj->intelligent = 1;
        obj->water_creature = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 50;
    }

    obj = &objects[25];
    if (obj->loaded) {
        obj->control = OrcaControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = -16384;
        obj->pivot_length = 200;
        obj->radius = 341;
        obj->intelligent = 1;
        obj->water_creature = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
    }

    obj = &objects[287];
    if (obj->loaded) {
        obj->control = DinoControl;
        obj->collision = CreatureCollision;
        obj->hit_points = 800;
        obj->shadow_size = 128;
        obj->pivot_length = 1800;
        obj->radius = 512;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        bones[obj->bone_index + 36] |= 8;
        bones[obj->bone_index + 44] |= 8;
        bones[obj->bone_index + 80] |= 8;
        bones[obj->bone_index + 88] |= 8;
    }

    obj = &objects[288];
    if (obj->loaded) {
        obj->control = RaptorControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 90;
        obj->pivot_length = 600;
        obj->radius = 341;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        bones[obj->bone_index + 80] |= 8;
        bones[obj->bone_index + 84] |= 8;
        bones[obj->bone_index + 92] |= 8;
        bones[obj->bone_index + 100] |= 8;
    }

    obj = &objects[22];
    if (obj->loaded) {
        obj->initialise = InitialiseDog;
        obj->control = DogControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 16;
        obj->pivot_length = 300;
        obj->radius = 341;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        bones[obj->bone_index + 8] |= 8;
        bones[obj->bone_index + 8] |= 4;
    }

    obj = &objects[41];
    if (obj->loaded) {
        obj->initialise = InitialiseDog;
        obj->control = DogControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 16;
        obj->pivot_length = 300;
        obj->radius = 341;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        bones[obj->bone_index + 8] |= 8;
        bones[obj->bone_index + 8] |= 4;
    }

    obj = &objects[23];
    if (obj->loaded) {
        obj->control = MouseControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 4;
        obj->pivot_length = 50;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        bones[obj->bone_index + 12] |= 8;
    }

    obj = &objects[34];
    if (obj->loaded) {
        obj->initialise = InitialiseCompy;
        obj->control = CompyControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 85;
        obj->hit_points = 10;
        obj->pivot_length = 50;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->non_lot = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        bones[obj->bone_index + 4] |= 8;
        bones[obj->bone_index + 8] |= 8;
    }

    obj = &objects[36];
    if (obj->loaded) {
        obj->initialise = InitialiseTribeBoss;
        obj->control = TribeBossControl;
        obj->draw_routine = S_DrawTribeBoss;
        obj->collision = CreatureCollision;
        obj->shadow_size = 0;
        obj->hit_points = 200;
        obj->pivot_length = 50;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        bones[obj->bone_index + 16] |= 8;
        bones[obj->bone_index + 28] |= 12;
    }

    obj = &objects[73];
    if (obj->loaded) {
        obj->initialise = InitialiseTonyBoss;
        obj->control = TonyBossControl;
        obj->draw_routine = S_DrawTonyBoss;
        obj->collision = CreatureCollision;
        obj->shadow_size = 0;
        obj->hit_points = 200;
        obj->pivot_length = 50;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        bones[obj->bone_index + 24] |= 8;
        bones[obj->bone_index + 24] |= 4;
        bones[obj->bone_index + 52] |= 8;
    }

    obj = &objects[57];
    if (obj->loaded) {
        obj->initialise = InitialiseLondonBoss;
        obj->control = LondonBossControl;
        obj->draw_routine = S_DrawLondonBoss;
        obj->collision = CreatureCollision;
        obj->shadow_size = 0;
        obj->hit_points = 300;
        obj->pivot_length = 50;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        bones[obj->bone_index + 24] |= 8;
        bones[obj->bone_index + 24] |= 4;
        bones[obj->bone_index + 52] |= 8;
    }

    obj = &objects[49];
    if (obj->loaded) {
        obj->initialise = InitialiseWillBoss;
        obj->control = WillBossControl;
        obj->draw_routine = S_DrawWillBoss;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 200;
        obj->pivot_length = 50;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
    }

    obj = &objects[28];
    if (obj->loaded) {
        obj->control = TigerControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 24;
        obj->pivot_length = 200;
        obj->radius = 341;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        bones[obj->bone_index + 84] |= 8;
    }

    obj = &objects[70];
    if (obj->loaded) {
        if (!objects[122].loaded)
            S_ExitSystem("FATAL: Shiva requires MESHSWAP1 (Shiva statue)");
        obj->initialise = InitialiseShiva;
        obj->draw_routine = DrawShiva;
        obj->control = ShivaControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 100;
        obj->radius = 341;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 0;
        bones[obj->bone_index + 24] |= 8;
        bones[obj->bone_index + 24] |= 4;
        bones[obj->bone_index + 100] |= 8;
        bones[obj->bone_index + 100] |= 4;
    }

    obj = &objects[40];
    if (obj->loaded) {
        obj->initialise = InitialiseOilSMG;
        obj->control = OilSMGControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 30;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 0;
        obj->bite_offset = 1;
        bones[obj->bone_index] |= 8;
        bones[obj->bone_index] |= 4;
        bones[obj->bone_index + 28] |= 8;
    }

    obj = &objects[45];
    if (obj->loaded) {
        obj->control = ClawmuteControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 130;
        obj->radius = 256;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 0;
        bones[obj->bone_index] |= 16;
        bones[obj->bone_index] |= 4;
        bones[obj->bone_index + 28] |= 8;
    }

    obj = &objects[46];
    if (obj->loaded) {
        obj->control = HybridControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 90;
        obj->radius = 128;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 0;
        bones[obj->bone_index] |= 16;
        bones[obj->bone_index] |= 4;
        bones[obj->bone_index + 28] |= 8;
    }

    obj = &objects[42];
    if (obj->loaded) {
        obj->control = SealmuteControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 50;
        obj->radius = 204;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 0;
        bones[obj->bone_index + 32] |= 16;
        bones[obj->bone_index + 32] |= 4;
        bones[obj->bone_index + 36] |= 8;
    }

    obj = &objects[63];
    if (obj->loaded) {
        obj->initialise = InitialiseSwat;
        obj->control = SwatControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 45;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 0;
        obj->bite_offset = 2;
        bones[obj->bone_index] |= 8;
        bones[obj->bone_index] |= 4;
        bones[obj->bone_index + 28] |= 8;
    }

    obj = &objects[37];
    if (obj->loaded) {
        obj->initialise = InitialiseArmySMG;
        obj->control = ArmySMGControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 30;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 0;
        obj->bite_offset = 6;
        bones[obj->bone_index] |= 8;
        bones[obj->bone_index] |= 4;
        bones[obj->bone_index + 28] |= 8;
    }

    obj = &objects[35];
    if (obj->loaded) {
        obj->control = LizManControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 36;
        obj->radius = 204;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 0;
        bones[obj->bone_index + 4] |= 16;
        bones[obj->bone_index + 36] |= 16;
    }

    obj = &objects[60];
    if (obj->loaded) {
        obj->initialise = InitialiseBaton;
        obj->control = BatonControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 25;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 0;
        bones[obj->bone_index + 24] |= 8;
        bones[obj->bone_index + 24] |= 4;
        bones[obj->bone_index + 52] |= 8;
    }

    obj = &objects[65];
    if (obj->loaded) {
        obj->initialise = InitialiseCivvy;
        obj->control = CivvyControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 15;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 0;
        bones[obj->bone_index + 24] |= 8;
        bones[obj->bone_index + 24] |= 4;
        bones[obj->bone_index + 52] |= 8;
    }

    obj = &objects[62];
    if (obj->loaded) {
        obj->initialise = InitialisePrisoner;
        obj->control = PrisonerControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 20;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 0;
        bones[obj->bone_index + 24] |= 8;
        bones[obj->bone_index + 24] |= 4;
        bones[obj->bone_index + 52] |= 8;
    }

    obj = &objects[50];
    if (obj->loaded) {
        obj->control = FlamerControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 36;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 0;
        bones[obj->bone_index] |= 8;
        bones[obj->bone_index] |= 4;
        bones[obj->bone_index + 28] |= 8;
    }

    obj = &objects[20];
    if (obj->loaded) {
        obj->control = TribeAxeControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 28;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 0;
        bones[obj->bone_index + 52] |= 8;
        bones[obj->bone_index + 24] |= 8;
    }

    obj = &objects[21];
    if (obj->loaded) {
        obj->control = BlowpipeControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 28;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        bones[obj->bone_index + 24] |= 8;
        bones[obj->bone_index + 24] |= 4;
        bones[obj->bone_index + 52] |= 8;
        bones[obj->bone_index + 52] |= 4;
    }

    obj = &objects[44];
    if (obj->loaded) {
        obj->initialise = InitialiseWingmute;
        obj->control = WingmuteControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 24;
        obj->radius = 204;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 0;
    }

    obj = &objects[29];
    if (obj->loaded) {
        obj->initialise = InitialiseVulture;
        obj->control = VultureControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 18;
        obj->radius = 204;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 0;
    }

    obj = &objects[27];
    if (obj->loaded) {
        obj->initialise = InitialiseVulture;
        obj->control = VultureControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 8;
        obj->radius = 204;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 0;
    }

    obj = &objects[39];
    if (obj->loaded) {
        obj->control = OilRedControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 34;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 0;
        obj->bite_offset = 0;
        bones[obj->bone_index + 24] |= 8;
        bones[obj->bone_index + 24] |= 4;
        bones[obj->bone_index + 52] |= 8;
    }

    obj = &objects[56];
    if (obj->loaded) {
        obj->control = LondSecControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 28;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 0;
        obj->bite_offset = 0;
        bones[obj->bone_index + 24] |= 8;
        bones[obj->bone_index + 24] |= 4;
        bones[obj->bone_index + 52] |= 8;
    }

    obj = &objects[58];
    if (obj->loaded) {
        obj->initialise = InitialiseCleaner;
        obj->control = CleanerControl;
        obj->collision = ObjectCollision;
        obj->shadow_size = 128;
        obj->hit_points = -16384;
        obj->radius = 512;
        obj->intelligent = 1;
        obj->non_lot = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
    }

    obj = &objects[61];
    if (obj->loaded) {
        obj->control = MPGunControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 28;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 0;
        obj->bite_offset = 0;
        bones[obj->bone_index + 24] |= 8;
        bones[obj->bone_index + 24] |= 4;
        bones[obj->bone_index + 52] |= 8;
    }

    obj = &objects[51];
    if (obj->loaded) {
        obj->initialise = InitialiseSwat;
        obj->control = SwatControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 45;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 0;
        obj->bite_offset = 2;
        bones[obj->bone_index] |= 8;
        bones[obj->bone_index] |= 4;
        bones[obj->bone_index + 28] |= 8;
    }

    obj = &objects[53];
    if (obj->loaded) {
        obj->initialise = InitialisePunk;
        obj->control = PunkControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 20;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 0;
        bones[obj->bone_index + 24] |= 8;
        bones[obj->bone_index + 24] |= 4;
        bones[obj->bone_index + 52] |= 8;
    }

    obj = &objects[32];
    if (obj->loaded) {
        obj->control = CrocControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = 128;
        obj->hit_points = 42;
        obj->radius = 341;
        obj->intelligent = 1;
        obj->water_creature = 1;
        obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
        obj->pivot_length = 200;
        bones[obj->bone_index + 28] |= 8;
    }

    obj = &objects[360];
    if (obj->loaded) {
        obj->control = OldWinstonControl;
        obj->collision = ObjectCollision;
        obj->hit_points = 20;
        obj->shadow_size = 64;
        obj->radius = 102;
        obj->intelligent = 1;
    }

    obj = &objects[361];
    if (obj->loaded) {
        obj->control = WinstonControl;
        obj->collision = ObjectCollision;
        obj->hit_points = 20;
        obj->shadow_size = 64;
        obj->radius = 102;
        obj->intelligent = 1;
    }

    obj = &objects[30];
    if (obj->loaded) {
        obj->control = TargetControl;
        obj->collision = ObjectCollision;
        obj->hit_points = 8;
        obj->shadow_size = 128;
        obj->radius = 102;
        obj->intelligent = 1;
    }

    obj = &objects[74];
    if (obj->loaded) {
        obj->draw_routine = DrawDummyItem;
        obj->collision = AIPickupCollision;
        obj->hit_points = 0;
    }

    obj = &objects[75];
    if (obj->loaded) {
        obj->draw_routine = DrawDummyItem;
        obj->collision = AIPickupCollision;
        obj->hit_points = 0;
    }

    obj = &objects[76];
    if (obj->loaded) {
        obj->draw_routine = DrawDummyItem;
        obj->collision = AIPickupCollision;
        obj->hit_points = 0;
    }

    obj = &objects[79];
    if (obj->loaded) {
        obj->draw_routine = DrawDummyItem;
        obj->collision = AIPickupCollision;
        obj->hit_points = 0;
    }

    obj = &objects[78];
    if (obj->loaded) {
        obj->draw_routine = DrawDummyItem;
        obj->collision = AIPickupCollision;
        obj->hit_points = 0;
    }

    obj = &objects[80];
    if (obj->loaded) {
        obj->draw_routine = DrawDummyItem;
        obj->collision = AIPickupCollision;
        obj->hit_points = 0;
    }

    obj = &objects[81];
    if (obj->loaded) {
        obj->draw_routine = DrawDummyItem;
        obj->collision = AIPickupCollision;
        obj->hit_points = 0;
    }

    obj = &objects[82];
    if (obj->loaded) {
        obj->draw_routine = DrawDummyItem;
        obj->collision = AIPickupCollision;
        obj->hit_points = 0;
    }

    obj = &objects[77];
    if (obj->loaded) {
        obj->draw_routine = DrawDummyItem;
        obj->collision = AIPickupCollision;
        obj->hit_points = 0;
    }
}
