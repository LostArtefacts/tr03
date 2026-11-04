#include "game.h"

typedef struct {
    sint32 Velocity;
    sint16 FrontRot;
    sint16 RearRot;
    sint32 Revs;
    sint32 EngineRevs;
    sint16 track_mesh;
    sint32 skidoo_turn;
    sint32 left_fallspeed;
    sint32 right_fallspeed;
    sint16 momentum_angle;
    sint16 extra_rotation;
    sint32 pitch;
    uint8 Flags;
} QUADINFO;

void InitialiseQuadBike(sint16 item_number)
{
    ITEM_INFO *v;
    QUADINFO *bike;

    v = &items[item_number];
    bike = v->data = game_malloc(sizeof(QUADINFO), 0);
    bike->Velocity = 0;
    bike->skidoo_turn = 0;
    bike->left_fallspeed = bike->right_fallspeed = 0;
    bike->momentum_angle = v->pos.y_rot;
    bike->extra_rotation = 0;
    bike->track_mesh = 0;
    bike->pitch = 0;
    bike->Flags = 0;
}

void TriggerUnderwaterExplosion(ITEM_INFO *item);
void TriggerExplosionSparks(sint32 x, sint32 y, sint32 z, sint32 extra_trigs, sint32 dyn, sint32 uw, sint16 roomnum);
int ExplodingDeath(sint16 item_number, sint32 mesh_bits, sint16 damage);

void QuadbikeExplode(ITEM_INFO *v)
{
    sint32 lp;

    if (room[v->room_number].flags & UNDERWATER) {
        TriggerUnderwaterExplosion(v);
    } else {
        TriggerExplosionSparks(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, 3, -2, 0, v->room_number);
        for (lp = 0; lp < 3; lp++)
            TriggerExplosionSparks(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, 3, -1, 0, v->room_number);
    }
    ExplodingDeath(lara.skidoo, 0xfffffffe, 1);
    KillItem(lara.skidoo);
    v->status = DEACTIVATED;
    SoundEffect(105, NULL, 0);
    SoundEffect(106, NULL, 0);
    lara.skidoo = NO_ITEM;
}
