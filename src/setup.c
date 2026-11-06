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
    long lp;

    obj = &objects[24];
    obj->hit_points = 0;
    obj->draw_routine = DrawDummyItem;
    obj->control = KillAllCurrentItems;
    obj->save_flags = 1;

    obj = &objects[359];
    obj->control = MiniCopterControl;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[96];
    obj->collision = ObjectCollision;
    obj->control = GeneralControl;
    obj->save_anim = obj->save_flags = 1;
    obj->water_creature = 1;

    obj = &objects[127];
    obj->initialise = InitialiseRollingBall;
    obj->collision = DeathSlideCollision;
    obj->control = ControlDeathSlide;
    obj->save_position = obj->save_anim = obj->save_flags = 1;

    obj = &objects[114];
    obj->control = ControlSpikeWall;
    obj->collision = ObjectCollision;
    obj->save_position = obj->save_flags = 1;

    obj = &objects[116];
    obj->control = ControlCeilingSpikes;
    obj->collision = TrapCollision;
    obj->save_position = obj->save_flags = 1;

    obj = &objects[106];
    obj->control = HookControl;
    obj->collision = CreatureCollision;
    obj->save_flags = obj->save_anim = 1;

    obj = &objects[105];
    obj->control = PropellerControl;
    obj->collision = ObjectCollision;
    obj->save_flags = obj->save_anim = 1;

    obj = &objects[119];
    obj->water_creature = 1;
    obj->control = PropellerControl;
    obj->collision = TrapCollision;
    obj->save_flags = obj->save_anim = 1;

    obj = &objects[120];
    obj->control = PropellerControl;
    obj->collision = TrapCollision;
    obj->save_flags = obj->save_anim = 1;

    obj = &objects[108];
    obj->initialise = InitialiseKillerStatue;
    obj->control = SpinningBlade;
    obj->collision = ObjectCollision;
    obj->save_position = obj->save_anim = obj->save_flags = 1;

    obj = &objects[113];
    obj->control = IcicleControl;
    obj->collision = TrapCollision;
    obj->save_position = obj->save_anim = obj->save_flags = 1;

    obj = &objects[111];
    obj->initialise = InitialiseBlade;
    obj->control = BladeControl;
    obj->collision = TrapCollision;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[115];
    obj->control = SpringBoardControl;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[83];
    obj->control = FallingBlock;
    obj->floor = FallingBlockFloor;
    obj->ceiling = FallingBlockCeiling;
    obj->save_position = obj->save_anim = 1;
    obj->save_flags = 1;

    obj = &objects[84];
    obj->control = FallingBlock;
    obj->floor = FallingBlockFloor;
    obj->ceiling = FallingBlockCeiling;
    obj->save_position = obj->save_anim = 1;
    obj->save_flags = 1;

    obj = &objects[85];
    obj->control = FallingBlock;
    obj->floor = FallingBlockFloor;
    obj->ceiling = FallingBlockCeiling;
    obj->save_position = obj->save_anim = 1;
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
    obj->collision = TrapCollision;
    obj->control = TeethTrap;
    obj->save_flags = 1;
    obj->save_anim = 1;

    obj = &objects[112];
    obj->collision = RollingBallCollision;
    obj->control = RollingBallControl;
    obj->initialise = InitialiseRollingBall;
    obj->save_position = obj->save_flags = 1;
    obj->save_anim = 1;

    obj = &objects[126];
    obj->collision = RollingBallCollision;
    obj->control = RollingBallControl;
    obj->initialise = InitialiseRollingBall;
    obj->save_position = obj->save_flags = 1;
    obj->save_anim = 1;

    obj = &objects[88];
    obj->collision = RollingBallCollision;
    obj->control = RollingBallControl;
    obj->initialise = InitialiseRollingBall;
    obj->save_position = obj->save_flags = 1;
    obj->save_anim = 1;

    obj = &objects[89];
    obj->collision = RollingBallCollision;
    obj->control = RollingBallControl;
    obj->initialise = InitialiseRollingBall;
    obj->save_position = obj->save_flags = 1;
    obj->save_anim = 1;

    obj = &objects[87];
    obj->control = SpikeControl;
    obj->collision = SpikeCollision;

    obj = &objects[107];
    obj->control = FallingCeiling;
    obj->collision = TrapCollision;
    obj->save_position = obj->save_anim = 1;
    obj->save_flags = 1;
    for (lp = 97; lp <= 100; lp++) {
        obj = &objects[lp];
        obj->initialise = InitialiseMovingBlock;
        obj->control = MovableBlock;
        obj->collision = MovableBlockCollision;
        obj->draw_routine = DrawMovableBlock;
        obj->save_position = obj->save_anim = obj->save_flags = 1;
    }
    objects[91].control = DartEmitterControl;
    objects[91].draw_routine = DrawDummyItem;
    objects[91].save_flags = 1;
    objects[92].control = DartEmitterControl;
    objects[92].draw_routine = DrawDummyItem;
    objects[92].save_flags = 1;
    objects[48].initialise = InitialiseRaptorEmitter;
    objects[48].control = RaptorEmitterControl;
    objects[48].draw_routine = DrawDummyItem;
    objects[48].save_flags = 1;
    objects[47].control = FlyEmitterControl;
    objects[47].draw_routine = DrawDummyItem;
    objects[47].hit_points = 150;
    objects[47].save_flags = 1;

    obj = &objects[31];
    obj->control = BoomuteControl;
    obj->save_position = obj->save_anim = obj->save_flags = 1;

    obj = &objects[312];
    obj->control = ControlArea51Rocket;
    obj->save_position = obj->save_anim = obj->save_flags = 1;

    obj = &objects[313];
    obj->control = ControlArea51Rocket;
    obj->draw_routine = DrawDummyItem;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[314];
    obj->control = ControlArea51Struts;
    obj->initialise = InitialiseArea51Struts;
    obj->save_anim = 1;

    obj = &objects[291];
    obj->control = ControlArea51Laser;
    obj->initialise = InitialiseArea51Laser;
    obj->save_position = obj->save_anim = obj->save_flags = 1;

    obj = &objects[90];
    obj->control = DartsControl;
    obj->draw_routine = S_DrawDarts;
    obj->collision = ObjectCollision;
    obj->shadow_size = 128;

    obj = &objects[330];
    obj->control = FlameEmitterControl;
    obj->draw_routine = DrawDummyItem;
    obj->save_flags = 1;

    obj = &objects[331];
    obj->control = FlameEmitter2Control;
    obj->draw_routine = DrawDummyItem;
    obj->save_flags = 1;

    obj = &objects[332];
    obj->control = FlameEmitter3Control;
    obj->draw_routine = DrawDummyItem;
    obj->save_flags = 1;

    obj = &objects[333];
    obj->control = SideFlameEmitterControl;
    obj->draw_routine = DrawDummyItem;
    obj->save_flags = 1;
    objects[329].control = FlameControl;
    objects[329].draw_routine = DrawDummyItem;
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

void ControlFireHead(sint16 item_number);
void InitialiseFireHead(sint16 item_number);
void ControlTonyFireBall(sint16 item_number);
void ControlClawmutePlasmaBall(sint16 item_number);
void ControlWillbossPlasmaBall(sint16 item_number);
void ControlRotateyThing(sint16 item_number);
void ControlLaserBolts(sint16 item_number);
void S_DrawLaserBolts(ITEM_INFO *item);
void ControlLondBossPlasmaBall(sint16 item_number);
void ControlFusebox(sint16 item_number);
void ControlRocket(sint16 item_number);
void ControlGrenade(sint16 item_number);
void ControlHarpoonBolt(sint16 item_number);
void ControlMissile(sint16 item_number);
void KayakInitialise(sint16 item_number);
void KayakDraw(ITEM_INFO *item);
void KayakCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void InitialiseBoat(sint16 item_number);
void BoatControl(sint16 item_number);
void BoatCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void DrawBoat(ITEM_INFO *item);
void InitialiseQuadBike(sint16 item_number);
void QuadBikeCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void QuadBikeDraw(ITEM_INFO *item);
void MineCartInitialise(sint16 item_number);
void MineCartCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void BigGunInitialise(sint16 item_number);
void BigGunCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void BigGunDraw(ITEM_INFO *item);
void SubInitialise(sint16 item_number);
void SubEffects(sint16 item_number);
void SubCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void SubDraw(ITEM_INFO *item);
void TrainControl(sint16 item_number);
void TrainCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void FlareControl(sint16 item_number);
void DrawFlareInAir(ITEM_INFO *item);
void InitialiseWindow(sint16 item_number);
void WindowControl(sint16 item_number);
void CarcassControl(sint16 item_number);
void InitialiseLift(sint16 item_number);
void LiftControl(sint16 item_number);
void LiftFloor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void LiftCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void S_DrawFish(ITEM_INFO *item);
void ControlFish(sint16 item_number);
void BatEmitterControl(sint16 item_number);
void ControlElectricFence(sint16 item_number);
void ControlStrobeLight(sint16 item_number);
void ControlPulseLight(sint16 item_number);
void ControlOnOffLight(sint16 item_number);
void ControlElectricalLight(sint16 item_number);
void ControlBeaconLight(sint16 item_number);
void BridgeFlatFloor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void BridgeFlatCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void BridgeTilt1Floor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void BridgeTilt1Ceiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void BridgeTilt2Floor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void BridgeTilt2Ceiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void DrawBridgeCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void DrawBridgeFloor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void DrawBridgeCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void SwitchControl(sint16 item_number);
void SwitchCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void SwitchCollision2(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void InitialiseDoor(sint16 item_number);
void DoorControl(sint16 item_number);
void TrapDoorControl(sint16 item_number);
void TrapDoorFloor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void TrapDoorCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void AnimatingPickUp(sint16 item_number);
void KeyHoleCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void DetonatorCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void DetonatorControl(sint16 item_number);
void ControlAnimating_1_4(sint16 item_number);
void PuzzleHoleCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void ControlSmokeEmitter(sint16 item_number);
void ControlGhostGasEmitter(sint16 item_number);
void ControlColouredLights(sint16 item_number);
void ControlBubble1(sint16 item_number);
void ControlFlameThrower(sint16 item_number);
void WaterFall(sint16 item_number);
void LaserControl(sint16 item_number);
void S_DrawLaser(ITEM_INFO *item);
void ControlBodyPart(sint16 item_number);
void ControlClockChimes(sint16 item_number);
void FinalLevelCounter(sint16 item_number);
void ControlCutShotgun(sint16 item_number);
void EarthQuake(sint16 item_number);
void ControlGunShell(sint16 item_number);
void InitialiseGenPlayer(sint16 item_num);
void ControlCinematicPlayer(sint16 item_num);

void ObjectObjects(void)
{
    OBJECT_INFO *obj;
    long lp;

    objects[304].draw_routine = DrawDummyItem;

    obj = &objects[282];
    obj->control = ControlFireHead;
    obj->collision = ObjectCollision;
    obj->initialise = InitialiseFireHead;
    obj->save_flags = obj->save_anim = obj->save_hitpoints = 1;

    obj = &objects[283];
    obj->control = ControlTonyFireBall;
    obj->draw_routine = DrawDummyItem;

    obj = &objects[368];
    obj->control = ControlClawmutePlasmaBall;
    obj->draw_routine = DrawDummyItem;

    obj = &objects[369];
    obj->control = ControlWillbossPlasmaBall;
    obj->draw_routine = DrawDummyItem;

    obj = &objects[370];
    obj->control = ControlRotateyThing;
    obj->save_position = obj->save_flags = 1;

    obj = &objects[371];
    obj->control = ControlLaserBolts;
    obj->draw_routine = S_DrawLaserBolts;

    obj = &objects[372];
    obj->control = ControlLondBossPlasmaBall;
    obj->draw_routine = DrawDummyItem;

    obj = &objects[373];
    obj->control = ControlFusebox;
    obj->collision = ObjectCollision;
    obj->save_position = obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;

    obj = &objects[309];
    obj->control = ControlRocket;

    obj = &objects[311];
    obj->control = ControlGrenade;

    obj = &objects[310];
    obj->control = ControlHarpoonBolt;

    obj = &objects[308];
    obj->control = ControlMissile;

    obj = &objects[306];
    obj->control = ControlMissile;

    obj = &objects[14];
    obj->initialise = KayakInitialise;
    obj->draw_routine = KayakDraw;
    obj->collision = KayakCollision;
    obj->save_position = obj->save_flags = obj->save_anim = 1;

    obj = &objects[15];
    obj->initialise = InitialiseBoat;
    obj->control = BoatControl;
    obj->collision = BoatCollision;
    obj->draw_routine = DrawBoat;
    obj->save_position = obj->save_flags = obj->save_anim = 1;
    bones[obj->bone_index + 4] |= 16;

    obj = &objects[16];
    obj->initialise = InitialiseQuadBike;
    obj->collision = QuadBikeCollision;
    obj->draw_routine = QuadBikeDraw;
    obj->save_position = obj->save_flags = obj->save_anim = 1;

    obj = &objects[17];
    obj->initialise = MineCartInitialise;
    obj->collision = MineCartCollision;
    obj->save_position = obj->save_flags = obj->save_anim = 1;

    obj = &objects[18];
    obj->initialise = BigGunInitialise;
    obj->collision = BigGunCollision;
    obj->draw_routine = BigGunDraw;
    obj->save_position = obj->save_flags = obj->save_anim = 1;

    obj = &objects[19];
    obj->initialise = SubInitialise;
    obj->control = SubEffects;
    obj->collision = SubCollision;
    obj->draw_routine = SubDraw;
    obj->save_position = obj->save_flags = obj->save_anim = 1;

    obj = &objects[110];
    obj->control = TrainControl;
    obj->collision = TrainCollision;
    obj->save_position = obj->save_flags = obj->save_anim = 1;

    obj = &objects[179];
    obj->control = FlareControl;
    obj->collision = PickUpCollision;
    obj->draw_routine = DrawFlareInAir;
    obj->pivot_length = 256;
    obj->hit_points = 256;
    obj->save_position = obj->save_flags = 1;

    obj = &objects[101];
    obj->initialise = InitialiseWindow;
    obj->control = WindowControl;
    obj->collision = ObjectCollision;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[102];
    obj->initialise = InitialiseWindow;
    obj->control = WindowControl;
    obj->collision = ObjectCollision;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[103];
    obj->initialise = InitialiseWindow;
    obj->control = WindowControl;
    obj->collision = ObjectCollision;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[104];
    obj->initialise = InitialiseWindow;
    obj->control = WindowControl;
    obj->collision = ObjectCollision;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[38];
    obj->control = CarcassControl;
    obj->collision = ObjectCollision;
    obj->save_anim = obj->save_flags = obj->save_position = 1;

    obj = &objects[95];
    obj->initialise = InitialiseLift;
    obj->control = LiftControl;
    obj->floor = LiftFloor;
    obj->ceiling = LiftCeiling;
    obj->save_anim = obj->save_flags = obj->save_position = 1;

    obj = &objects[339];
    obj->draw_routine = S_DrawFish;
    obj->control = ControlFish;
    obj->hit_points = -1;
    obj->save_flags = obj->save_position = obj->save_hitpoints = 1;

    obj = &objects[338];
    obj->draw_routine = S_DrawFish;
    obj->control = ControlFish;
    obj->hit_points = -1;
    obj->save_flags = obj->save_position = obj->save_hitpoints = 1;

    obj = &objects[347];
    obj->control = BatEmitterControl;
    obj->draw_routine = DrawDummyItem;
    obj->save_flags = 1;

    obj = &objects[292];
    obj->control = ControlElectricFence;
    obj->draw_routine = DrawDummyItem;
    obj->save_flags = 1;
    objects[318].control = ControlStrobeLight;
    objects[318].save_flags = 1;
    objects[321].control = ControlPulseLight;
    objects[321].draw_routine = DrawDummyItem;
    objects[321].save_flags = 1;
    objects[320].control = ControlOnOffLight;
    objects[320].draw_routine = DrawDummyItem;
    objects[320].save_flags = 1;
    objects[319].control = ControlElectricalLight;
    objects[319].draw_routine = DrawDummyItem;
    objects[319].save_flags = 1;
    objects[322].control = ControlBeaconLight;
    objects[322].draw_routine = DrawDummyItem;
    objects[322].save_flags = 1;

    obj = &objects[142];
    obj->floor = BridgeFlatFloor;
    obj->ceiling = BridgeFlatCeiling;

    obj = &objects[143];
    obj->floor = BridgeTilt1Floor;
    obj->ceiling = BridgeTilt1Ceiling;

    obj = &objects[144];
    obj->floor = BridgeTilt2Floor;
    obj->ceiling = BridgeTilt2Ceiling;

    obj = &objects[93];
    if (obj->loaded) {
        obj->control = GeneralControl;
        obj->collision = DrawBridgeCollision;
        obj->floor = DrawBridgeFloor;
        obj->ceiling = DrawBridgeCeiling;
        obj->save_anim = obj->save_flags = 1;
    }

    obj = &objects[118];
    obj->control = SwitchControl;
    obj->collision = SwitchCollision;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[128];
    obj->control = SwitchControl;
    obj->collision = SwitchCollision;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[117];
    obj->control = SwitchControl;
    obj->collision = SwitchCollision;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[129];
    obj->control = SwitchControl;
    obj->collision = SwitchCollision;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[130];
    obj->control = SwitchControl;
    obj->collision = SwitchCollision2;
    obj->save_anim = obj->save_flags = 1;
    for (lp = 131; lp <= 138; lp++) {
        obj = &objects[lp];
        obj->initialise = InitialiseDoor;
        obj->draw_routine = DrawUnclippedItem;
        obj->control = DoorControl;
        obj->collision = DoorCollision;
        obj->save_flags = obj->save_anim = 1;
    }

    obj = &objects[139];
    obj->control = TrapDoorControl;
    obj->floor = TrapDoorFloor;
    obj->ceiling = TrapDoorCeiling;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[140];
    obj->control = TrapDoorControl;
    obj->floor = TrapDoorFloor;
    obj->ceiling = TrapDoorCeiling;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[236];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[237];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[224];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[225];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[226];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[227];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[205];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[206];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[207];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[208];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[160];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[161];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[164];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[166];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[167];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[165];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[162];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[163];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[178];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[168];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[169];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[170];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[171];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[172];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[173];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[174];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[175];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[176];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[177];
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[240];
    obj->control = AnimatingPickUp;
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[241];
    obj->control = AnimatingPickUp;
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[242];
    obj->control = AnimatingPickUp;
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[243];
    obj->control = AnimatingPickUp;
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[180];
    obj->control = AnimatingPickUp;
    obj->collision = PickUpCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[232];
    obj->collision = KeyHoleCollision;
    obj->save_flags = 1;

    obj = &objects[233];
    obj->collision = KeyHoleCollision;
    obj->save_flags = 1;

    obj = &objects[234];
    obj->collision = KeyHoleCollision;
    obj->save_flags = 1;

    obj = &objects[235];
    obj->collision = KeyHoleCollision;
    obj->save_flags = 1;

    obj = &objects[295];
    obj->collision = DetonatorCollision;
    obj->control = DetonatorControl;
    obj->save_flags = obj->save_anim = 1;

    obj = &objects[213];
    obj->control = ControlAnimating_1_4;
    obj->collision = PuzzleHoleCollision;
    obj->save_flags = obj->save_anim = 1;

    obj = &objects[214];
    obj->control = ControlAnimating_1_4;
    obj->collision = PuzzleHoleCollision;
    obj->save_flags = obj->save_anim = 1;

    obj = &objects[215];
    obj->control = ControlAnimating_1_4;
    obj->collision = PuzzleHoleCollision;
    obj->save_flags = obj->save_anim = 1;

    obj = &objects[216];
    obj->control = ControlAnimating_1_4;
    obj->collision = PuzzleHoleCollision;
    obj->save_flags = obj->save_anim = 1;
    for (lp = 217; lp <= 220; lp++) {
        obj = &objects[lp];
        obj->control = ControlAnimating_1_4;
        obj->save_flags = obj->save_anim = 1;
    }
    for (lp = 148; lp <= 157; lp++) {
        obj = &objects[lp];
        obj->initialise = InitialiseGenPlayer;
        obj->control = ControlCinematicPlayer;
        obj->hit_points = 1;
    }
    for (lp = 349; lp <= 354; lp++) {
        obj = &objects[lp];
        obj->control = ControlAnimating_1_4;
        obj->collision = ObjectCollision;
        obj->save_anim = obj->save_flags = 1;
    }
    for (lp = 334; lp <= 336; lp++) {
        obj = &objects[lp];
        obj->control = ControlSmokeEmitter;
        obj->draw_routine = DrawDummyItem;
        obj->save_flags = 1;
    }
    objects[337].control = ControlGhostGasEmitter;
    objects[337].draw_routine = DrawDummyItem;
    objects[337].save_flags = 1;
    for (lp = 324; lp <= 328; lp++) {
        obj = &objects[lp];
        obj->control = ControlColouredLights;
        obj->draw_routine = DrawDummyItem;
        obj->save_flags = 1;
    }
    objects[297].control = ControlBubble1;
    objects[297].draw_routine = DrawDummyItem;
    objects[307].control = ControlFlameThrower;
    objects[307].draw_routine = DrawDummyItem;
    objects[305].control = WaterFall;
    objects[305].draw_routine = DrawDummyItem;
    for (lp = 66; lp <= 68; lp++) {
        obj = &objects[lp];
        obj->initialise = LaserControl;
        obj->control = LaserControl;
        obj->draw_routine = S_DrawLaser;
        obj->save_flags = obj->save_hitpoints = 1;
    }
    objects[303].control = ControlBodyPart;
    objects[303].nmeshes = 0;
    objects[303].loaded = 1;
    objects[289].control = ControlBirdTweeter;
    objects[289].draw_routine = DrawDummyItem;
    objects[286].control = ControlBirdTweeter;
    objects[286].draw_routine = DrawDummyItem;
    objects[357].control = ControlDingDong;
    objects[357].draw_routine = DrawDummyItem;
    objects[358].control = ControlLaraAlarm;
    objects[358].draw_routine = DrawDummyItem;
    objects[358].save_flags = 1;
    objects[290].control = ControlClockChimes;
    objects[290].draw_routine = DrawDummyItem;
    objects[290].save_flags = 1;
    objects[363].draw_routine = DrawDummyItem;
    objects[363].control = FinalLevelCounter;
    objects[363].save_flags = 1;
    objects[364].control = ControlCutShotgun;
    objects[364].save_flags = objects[364].save_anim = 1;
    objects[365].draw_routine = DrawDummyItem;
    objects[365].control = EarthQuake;
    objects[365].save_flags = 1;
    objects[366].control = ControlGunShell;
    objects[367].control = ControlGunShell;
}

extern uint8 IsAtmospherePlaying;
extern uint32 CurrentLevel;
extern int DemoPlay;
extern int title_loaded;
extern char **GF_titlefilenames;
extern char **GF_levelfilenames;
extern char **GF_cutscenefilenames;
extern int health_bar_timer;
extern uint8 CurrentAtmosphere;
extern sint16 GF_CDtracks[16];
extern sint32 level_items;

int S_LoadLevelFile(char *name, int number, int type);
void InitialiseLara(int type);
void GetCarriedItems(void);
void GetAIPickups(void);
void InitialiseLOTarray(void);
void InitColours(void);
void InitialisePickUpDisplay(void);
void S_InitialiseScreen(int type);
void SOUND_Stop(void);
void ExtractSaveGameInfo(void);
void GF_ModifyInventory(int level, int type);
void InitialiseFinalLevel(void);
void S_CDPlay(sint16 track, int mode);
int Inv_AddItem(int item_num);
void RemoveDrawnItem(sint16 item_num);

int InitialiseLevel(int level_number, int type)
{
    IsAtmospherePlaying = 0;

    if (type != IL_TITLE && type != IL_CUTSCENE)
        CurrentLevel = level_number;

    if (type == IL_DEMO)
        DemoPlay = 1;
    else
        DemoPlay = 0;

    InitialiseGameFlags();
    lara.item_number = NO_ITEM;

    title_loaded = 0;

    switch (type) {
    case IL_CUTSCENE:
        if (!S_LoadLevelFile(GF_cutscenefilenames[level_number], level_number, type))
            return 0;
        break;

    case IL_TITLE:
        if (!S_LoadLevelFile(GF_titlefilenames[0], level_number, type))
            return 0;
        break;

    case IL_NORMAL:
    case IL_SAVED:
    case IL_DEMO:
    default:
        if (!S_LoadLevelFile(GF_levelfilenames[level_number], level_number, type))
            return 0;
        break;
    }

    if (lara.item_number != NO_ITEM)
        InitialiseLara(type);

    if (type == IL_NORMAL || type == IL_SAVED || type == IL_DEMO) {
        GetCarriedItems();
        GetAIPickups();
    }

    effects = (FX_INFO *)game_malloc(50 * sizeof(FX_INFO), 32);
    InitialiseFXArray();
    InitialiseLOTarray();

    InitColours();
    T_InitPrint();
    InitialisePickUpDisplay();
    S_InitialiseScreen(type);

    health_bar_timer = 100;

    SOUND_Stop();

    if (type == IL_SAVED)
        ExtractSaveGameInfo();
    else
        GF_ModifyInventory(CurrentLevel, 0);

    if (objects[363].loaded)
        InitialiseFinalLevel();

    if (type == IL_SAVED && savegame.mid_level_save) {
        S_CDPlay(CurrentAtmosphere, 1);
        IsAtmospherePlaying = 1;
    } else if ((type == IL_NORMAL || type == IL_SAVED || type == IL_DEMO) && CurrentLevel != 0) {
        if (GF_CDtracks[0]) {
            S_CDPlay(GF_CDtracks[0], 1);
            CurrentAtmosphere = (uint8)GF_CDtracks[0];
            IsAtmospherePlaying = 1;
        }
    }

    if (CurrentLevel == 0 && savegame.QuadbikeKeyFlag) {
        long lp;

        Inv_AddItem(224);
        for (lp = 0; lp < level_items; lp++) {
            if (items[lp].object_number == 224) {
                items[lp].status = INVISIBLE;
                RemoveDrawnItem((sint16)lp);
                break;
            }
        }
    }

    assault_timer_active = 0;
    assault_timer_display = 0;
    camera.underwater = 0;

    return 1;
}
