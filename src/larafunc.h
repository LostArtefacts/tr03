#ifndef LARAFUNC_H
#define LARAFUNC_H

#include "items.h"
#include "collide.h"

void lara_slide_slope(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_monkeyswing(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_stop(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_turn_r(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_stepright(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_jumper(ITEM_INFO *item, COLL_INFO *coll);
void LaraHangTest(ITEM_INFO *item, COLL_INFO *coll);
void GetLaraCollisionInfo(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_all4turnl(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_dashdive(ITEM_INFO *item, COLL_INFO *coll);
sint16 GetDirOctant(sint32 rot);
void lara_as_hangturnl(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_hangturnr(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_monkey180(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_monkey180(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_upjump(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_death(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_fastfall(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_reach(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_splat(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_slide(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_backjump(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_rightjump(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_leftjump(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_fallback(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_hangleft(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_hangright(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_slideback(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_pushblock(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_ppready(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_pickup(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_switchon(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_usekey(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_special(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_swandive(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_gymnast(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_waterout(ITEM_INFO *item, COLL_INFO *coll);
void extra_as_airlock(ITEM_INFO *item, COLL_INFO *coll);
void extra_as_gongbong(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_turn_l(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_land(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_stepleft(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_slide(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_backjump(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_rightjump(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_leftjump(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_hangleft(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_hangright(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_slideback(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_pushblock(ITEM_INFO *item, COLL_INFO *coll);
void lara_default_col(ITEM_INFO *item, COLL_INFO *coll);
void LaraSurfaceCollision(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_surfback(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_surfleft(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_surfright(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_dive(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_waterroll(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_swim(ITEM_INFO *item, COLL_INFO *coll);
void LaraSwimCollision(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_glide(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_tread(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_dive(ITEM_INFO *item, COLL_INFO *coll);
void lara_col_waterroll(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_climbing(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_climbend(ITEM_INFO *item, COLL_INFO *coll);
void lara_as_climbdown(ITEM_INFO *item, COLL_INFO *coll);

#endif
