// Weapon ID #52. Used by weapons:
// Alucard shield, Unknown#214
#include "weapon_private.h"
extern u16* g_WeaponCluts[];
extern s32 g_HandId;
#include "shared.h"
#include "w_052_1.h"
#include "w_052_2.h"
#define g_Animset w_052_1
#define g_Animset2 w_052_2
#include "sfx.h"

extern SpriteParts D_170000_8017A040[];
extern s8 D_170000_8017AB3C[];
extern AnimationFrame* D_170000_8017AB84[];
extern s32 D_170000_8017ABBC;
extern s32 D_170000_8017ABC0;
extern u8 D_170000_8017ABC4[];
extern u8 D_170000_8017ABF0[];

extern s32 D_170000_8017CB84;
extern s16 D_170000_8017CB88[];
extern s16 D_170000_8017CBA0[];
extern s16 D_170000_8017CBB8[];
extern f32 D_170000_8017CBD0[11][4];

static void EntityWeaponAttack(Entity* self) {
    s32 anim;
    s32 anim2;
    s32 attackButton;

    anim = 0;
    anim2 = 0;
    if (g_HandId) {
        attackButton = PAD_CIRCLE;
    } else {
        attackButton = PAD_SQUARE;
    }
    // If we aren't pressing the attack button, lower the shield
    if (!(attackButton & g_Player.padPressed) && (self->step < 3)) {
        self->animFrameDuration = 0;
        self->animFrameIdx = 0;
        self->step = 3;
    }
    if (self->step != 4) {
        self->posX.val = PLAYER.posX.val;
        self->posY.val = PLAYER.posY.val;
        self->facingLeft = PLAYER.facingLeft;
    }
    if ((g_Player.unk0C & PLAYER_STATUS_UNK10000) && (self->step != 4)) {
        self->zPriority = PLAYER.zPriority + 2;
        self->step = 4;
        if (g_Player.pl_vram_flag & 1) {
            self->velocityX = PLAYER.velocityX;
        } else {
            self->velocityX = PLAYER.velocityX * 2;
        }
        self->velocityY = FIX(-3.5);
        self->ext.timer.t = 0x80;
        self->flags = FLAG_UNK_08000000;
        self->animCurFrame = 0x3E;
    }
    if ((PLAYER.step == 2) && (PLAYER.step_s != PLAYER.step)) {
        anim2++;
    }
    if ((D_170000_8017ABBC != 0) && !(self->params & 0x7F00) &&
        (self->step == 1 || self->step == 2) && (self->hitFlags != 0)) {
        g_api.func_8010E168(0, 0x60);
        if (g_Player.unk56 != 1) {
            g_Player.unk56 = 1;
            g_Player.unk58 = 8;
        }
        g_api.AddHearts(1);
        self->hitFlags = 0;
    }
    switch (self->step) {
    case 0:
        SetSpriteBank1(D_170000_8017A040);
        if (g_HandId != 0) {
            self->animSet = ANIMSET_OVL(0x12);
            self->palette = 0x128;
            self->unk5A = 0x66;
        } else {
            self->animSet = ANIMSET_OVL(0x10);
            self->palette = 0x110;
            self->unk5A = 0x64;
        }
        self->flags = FLAG_UNK_40000 | FLAG_UNK_20000;
        self->zPriority = PLAYER.zPriority - 2;
        g_Player.unk48 = 1;
        SetWeaponProperties(self, 0);
        D_170000_8017CB84 = self->attack;
        self->attack = 1;
        self->step++;
        /* fallthrough */
    case 1:
        self->ext.shield.anim = anim2 + 10;
        if (self->animFrameDuration < 0) {
            self->step++;
        }
        break;
    case 2:
        switch (PLAYER.ext.player.anim) {
        case 9:
        case 10:
        case 11:
            anim++;
            /* fallthrough */
        case 7:
            anim++;
            /* fallthrough */
        case 8:
            anim++;
            /* fallthrough */
        case 12:
            anim++;
            /* fallthrough */
        case 13:
            anim++;
            /* fallthrough */
        case 24:
        case 25:
            anim++;
            /* fallthrough */
        case 14:
        case 15:
            anim++;
            /* fallthrough */
        case 26:
            anim += 2;
            self->animFrameIdx = PLAYER.animFrameIdx;
            break;
        default:
            self->animFrameIdx = 0;
            anim += anim2;
            break;
        }
        self->ext.shield.anim = anim;
        self->animFrameDuration = 2;
        break;
    case 3:
        g_Player.unk48 = 0;
        self->ext.shield.anim = anim2 + 12;
        if (self->animFrameDuration < 0) {
            DestroyEntity(self);
            return;
        }
        break;
    case 4:
        self->hitboxState = 0;
        g_Player.unk48 = 0;
        self->drawFlags |= FLAG_DRAW_ROTZ;
        self->posY.val += self->velocityY;
        self->posX.val += self->velocityX;
        self->velocityY += FIX(20.0 / 128);
        self->rotZ += 0x80;
        if (--self->ext.timer.t < 0x10) {
            self->drawFlags |= FLAG_DRAW_UNK80;
        }
        if (--self->ext.timer.t == 0) {
            DestroyEntity(self);
            return;
        }
        break;
    }
    if (self->step != 4) {
        g_api.PlayAnimation(D_170000_8017AB3C, D_170000_8017AB84);
    }
    self->drawFlags = PLAYER.drawFlags;
    self->rotY = PLAYER.rotY;
    self->rotPivotY = PLAYER.rotPivotY;
    if ((D_170000_8017ABBC != 0) && !(self->params & 0x7F00)) {
        g_Status.D_80097BF8 |= 1;
        if (g_GameTimer % 16 == 0) {
            g_Status.mp--;
        }
        self->attack = D_170000_8017CB84;
        if (self->step != 1 && self->step != 2) {
            return;
        }
        self->hitboxState |= 2;
        // Spell is over. Timer either ran out or we ran out of MP
        if (--D_170000_8017ABC0 == 0 || (g_Status.mp <= 0)) {
            D_170000_8017ABBC = 0;
            SetWeaponProperties(self, 0);
            self->attack = 1;
            g_api.CreateEntFactoryFromEntity(
                self, ((g_HandId + 1) << 0xE) | 0x3A, 0);
        }
    }
}

static void func_ptr_80170008(Entity* self) {}

static void func_ptr_8017000C(Entity* self) {}

static s32 func_ptr_80170010(Entity* self) {}

static s32 func_ptr_80170014(Entity* self) {}

static int GetWeaponId(void) { return 52; }

static void EntityWeaponShieldSpell(Entity* self) {
    Primitive* prim;
    s16 angle;
    s16 selfX;
    s16 selfY;
    s16 primXScale;
    s16 primYScale;
    s16 playerXOffset;
    s16 playerYOffset;
    s16 width;
    s32 var_s8;
    s32 i;
    s16 primXCenter;
    s16 primYCenter;
    s16 height;
    u8* uvPtr;

    playerXOffset = 0x1C;
    if (PLAYER.facingLeft) {
        playerXOffset = -playerXOffset;
    }
    playerYOffset = 0x30;
    var_s8 = 0;
    switch (self->step) {
    case 0:
        self->primIndex = g_api.AllocPrimitives(PRIM_GT4, 23);
        if (self->primIndex == -1) {
            DestroyEntity(self);
            return;
        }
        SetSpriteBank1(D_170000_8017A040);
        if (g_HandId != 0) {
            self->animSet = ANIMSET_OVL(0x12);
            self->palette = 0x128;
            self->ext.shield.unk86 = 0x129;
            self->unk5A = 0x66;
            self->ext.shield.unk7C = 0;
            self->ext.shield.unk7D = 0x80;
        } else {
            self->animSet = ANIMSET_OVL(0x10);
            self->palette = 0x110;
            self->ext.shield.unk86 = 0x111;
            self->unk5A = 0x64;
            self->ext.shield.unk7C = 0x80;
            self->ext.shield.unk7D = 0;
        }
        self->posY.i.hi -= 8;
        self->flags = FLAG_UNK_04000000 | FLAG_HAS_PRIMS | FLAG_UNK_10000;
        self->zPriority = PLAYER.zPriority - 2;
        self->facingLeft = PLAYER.facingLeft;
        self->animCurFrame = 0x3E;
        self->drawFlags = FLAG_DRAW_ROTX | FLAG_DRAW_ROTY;

        self->rotX = self->rotY = 0;
        prim = &g_PrimBuf[self->primIndex];
        prim->clut = 0x19F;
        prim->tpage = 0x19;

        prim->u0 = prim->u2 = 0x80;
        prim->u1 = prim->u3 = 0x80 + 0x4F;
        prim->v0 = prim->v1 = self->ext.shield.unk7C + 0;
        prim->v2 = prim->v3 = self->ext.shield.unk7C + 0x4F;
        self->ext.shield.unk82 = 0;
        prim->r0 = prim->g0 = prim->b0 = prim->r1 = prim->g1 = prim->b1 =
            prim->r2 = prim->g2 = prim->b2 = prim->r3 = prim->g3 = prim->b3 =
                0x80;
        prim->priority = self->zPriority - 4;
        prim->drawMode =
            DRAW_UNK_40 | DRAW_TPAGE2 | DRAW_TPAGE | DRAW_COLORS | DRAW_TRANSP;
        prim = prim->next;
        uvPtr = &D_170000_8017ABC4;
        for (i = 0; i < 11; i++) {
            prim->u0 = prim->u2 = *uvPtr++;
            prim->v0 = prim->v1 = *uvPtr++;
            prim->u1 = prim->u3 = *uvPtr++;
            prim->v2 = prim->v3 = *uvPtr++;
            prim->v0 += self->ext.shield.unk7D;
            prim->v1 += self->ext.shield.unk7D;
            prim->v2 += self->ext.shield.unk7D;
            prim->v3 += self->ext.shield.unk7D;
            prim->tpage = 0x19;
            prim->clut = self->ext.shield.unk86 + i;
            prim->priority = self->zPriority - 4;
            prim->drawMode = DRAW_HIDE;
            prim->drawMode = DRAW_DEFAULT;
            prim->r0 = (rand() & 0xF) + 1;
            prim = prim->next;
        }
        for (i = 0; i < 11; i++) {
            prim->r0 = prim->r2 = D_170000_8017ABF0[i * 4 + 0];
            prim->g0 = prim->g2 = D_170000_8017ABF0[i * 4 + 1];
            prim->b0 = prim->b2 = D_170000_8017ABF0[i * 4 + 2];
            prim->r1 = prim->r3 = D_170000_8017ABF0[(i + 1) % 11 * 4 + 0];
            prim->g1 = prim->g3 = D_170000_8017ABF0[(i + 1) % 11 * 4 + 1];
            prim->b1 = prim->b3 = D_170000_8017ABF0[(i + 1) % 11 * 4 + 2];
            prim->drawMode = DRAW_HIDE;
            prim->type = PRIM_G4;
            prim = prim->next;
        }
        self->ext.shield.unk90 = 0x78;
        self->ext.shield.unk92 = 0;
        self->ext.shield.unk94 = 0;
        self->ext.shield.unk86 = 0x40;
        self->ext.shield.unk98 = 0x100;
        SetSpeedX(FIX(-2));
        self->velocityY = 0;
        DestroyEntityWeapon(1);
        g_api.PlaySfx(SFX_ANIME_SWORD_C);
        g_unkGraphicsStruct.unk20 = 1;
        self->step++;
        break;
    case 1:
        DecelerateX(FIX(1.0 / 16));
        self->velocityY -= FIX(20.0 / 128);
        self->posX.val += self->velocityX;
        self->posY.val += self->velocityY;
        self->rotX += 12;
        self->rotY = self->rotX;
        self->ext.shield.unk82 = self->rotX * 0x28 / 256;
        if (self->rotX >= 0x100) {
            self->ext.shield.unk82 = 0x28;
            self->rotY = self->rotX = 0x100;
            self->ext.shield.unk80 = 0x40;
            self->step++;
        }
        self->ext.shield.unk94 = self->rotX;
        self->ext.shield.unk92 += self->ext.shield.unk86;
        self->ext.shield.unk90 -= 3;
        break;
    case 2:
        if (--self->ext.shield.unk80 == 0) {
            self->ext.shield.unk80 = 0x10;
            g_api.PlaySfx(SFX_TRANSFORM);
            self->step++;
        }
        if (self->ext.shield.unk86 > 0) {
            self->ext.shield.unk86--;
        }
        self->ext.shield.unk92 += self->ext.shield.unk86;
        break;
    case 3:
        prim = &g_PrimBuf[self->primIndex];
        if (!(self->ext.shield.unk80 & 1)) {
            prim->priority = 0x1FC;
        } else {
            prim->priority = self->zPriority - 2;
        }
        self->rotX -= 0x10;
        if (self->rotX <= 0) {
            self->rotX = 0;
        }
        self->rotY = self->rotX;
        self->ext.shield.unk94 -= 2;
        self->ext.shield.unk98 -= 2;
        if (self->ext.shield.unk98 < 0x80) {
            self->ext.shield.unk98 = 0x80;
        }

        self->ext.shield.unk86 += 3;
        self->ext.shield.unk92 += self->ext.shield.unk86;
        // This doesn't work as -- or even -= 1. Weird!
        self->ext.shield.unk90 = self->ext.shield.unk90 - 1;
        if (--self->ext.shield.unk80 == 0) {
            self->animCurFrame = 0;
            self->ext.shield.unk80 = 0x20;
            self->step++;
        }
        break;
    case 4:
        self->ext.shield.unk94 -= 2;
        self->ext.shield.unk98 -= 2;
        if (self->ext.shield.unk98 < 0x80) {
            self->ext.shield.unk98 = 0x80;
        }
        self->ext.shield.unk86++;
        self->ext.shield.unk92 += self->ext.shield.unk86;
        self->ext.shield.unk82 += 8;
        self->rotX += 0x10;
        if (self->rotX >= 0x100) {
            self->rotX = 0x100;
        }
        self->ext.shield.childPalette = self->rotX * 36 / 256;
        self->ext.shield.unk8A = self->rotX * 28 / 256;
        prim = &g_PrimBuf[self->primIndex];
        if (prim->b3 > 8) {
            prim->b3 -= 4;
        }
        prim->r0 = prim->g0 = prim->b0 = prim->r1 = prim->g1 = prim->b1 =
            prim->r2 = prim->g2 = prim->b2 = prim->r3 = prim->g3 = prim->b3;
        self->ext.shield.unk86 += 4;
        self->ext.shield.unk92 += self->ext.shield.unk86;
        if (--self->ext.shield.unk80 == 0) {
            prim = prim->next;
            for (i = 0; i < 11; i++) {
                prim->g0 = 5;
                prim = prim->next;
            }
            // Alucard Shield plays the same sound as the warp!
            g_api.PlaySfx(SFX_TELEPORT_BANG_B);
            self->step++;
            self->step = 6;
        }
        break;
    case 6:
        var_s8 = 1;
        if (self->ext.shield.unk96 > 0xE0) {
            var_s8 = 2;
            self->step++;
            self->ext.shield.unk80 = 0x20;
            // Blueprint 44 has child 11, EntityPlayerBlinkWhite
            g_api.CreateEntFactoryFromEntity(self, FACTORY(0x100, 44), 0);
            D_170000_8017ABBC = 1;
        }
        self->ext.shield.unk96 += 12;
        break;
    case 7:
        var_s8 = 2;
        if (--self->ext.shield.unk80 == 0) {
            self->step++;
            D_170000_8017ABBC = 1;
            D_170000_8017ABC0 = 0x1000;
        }
        break;
    case 8:
        g_unkGraphicsStruct.unk20 = 0;
        DestroyEntity(self);
        return;
    }
    selfX = self->posX.i.hi;
    selfY = self->posY.i.hi;
    prim = &g_PrimBuf[self->primIndex];
    if (prim->b3 > 8) {
        if (g_Timer & 1) {
            prim->drawMode |= DRAW_HIDE;
        } else {
            prim->drawMode &= ~DRAW_HIDE;
        }
    }
    prim->x0 = prim->x2 = selfX - self->ext.shield.unk82;
    prim->x1 = prim->x3 = (selfX + self->ext.shield.unk82) - 1;
    prim->y0 = prim->y1 = selfY - self->ext.shield.unk82;
    prim->y2 = prim->y3 = (selfY + self->ext.shield.unk82) - 1;
    prim = prim->next;
    selfX = PLAYER.posX.i.hi - playerXOffset;
    selfY = PLAYER.posY.i.hi - playerYOffset;
    primXScale = self->ext.shield.unk90;
    primYScale = self->ext.shield.unk90 * self->ext.shield.unk98 / 256;
    for (i = 0; i < 11; i++) {
        switch (prim->g0) {
        case 0:
            angle = self->ext.shield.unk92 + ((i << 0xC) / 11);
            primXCenter = selfX + (((rcos(angle) >> 4) * primXScale) >> 8);
            primYCenter = selfY - (((rsin(angle) >> 4) * primYScale) >> 8);
            if (i == 6) {
                height = self->ext.shield.unk94 * 12 / 256;
            } else {
                height = self->ext.shield.unk94 * 16 / 256;
            }
            width = self->ext.shield.unk94 * 12 / 256;
            prim->x0 = prim->x2 = primXCenter - width;
            prim->x1 = prim->x3 = primXCenter + width;
            prim->y0 = prim->y1 = primYCenter - height;
            prim->y2 = prim->y3 = primYCenter + height;
            D_170000_8017CBB8[i] = prim->priority;
            D_170000_8017CB88[i] = primXCenter;
            D_170000_8017CBA0[i] = primYCenter;
            break;
        case 1:
            if (--prim->r0 == 0) {
                prim->g0++;
            }
            break;
        case 5:
            if ((prim->x1 - prim->x0) > 2) {
                prim->x0++;
                prim->x1--;
            } else {
                prim->x0 = prim->x1;
            }
            prim->y0 -= 12;
            prim->y2 += 12;
            prim->y1 = prim->y0;
            prim->y3 = prim->y2;
            prim->x2 = prim->x0;
            prim->x3 = prim->x1;
            if ((prim->y0 < 0) && (prim->y2 > 0xF0)) {
                prim->drawMode |= DRAW_HIDE;
                prim->g0++;
            }
            break;
        }
        prim = prim->next;
    }

    if (var_s8 == 0) {
        return;
    }
    for (i = 0; i < 11; i++) {
        if (var_s8 == 2) {
            if (prim->r0 > 8) {
                prim->r0 -= 4;
            }
            if (prim->r1 > 8) {
                prim->r1 -= 4;
            }
            if (prim->r2 > 8) {
                prim->r2 -= 4;
            }
            if (prim->r3 > 8) {
                prim->r3 -= 4;
            }
            if (prim->b0 > 8) {
                prim->b0 -= 4;
            }
            if (prim->b1 > 8) {
                prim->b1 -= 4;
            }
            if (prim->b2 > 8) {
                prim->b2 -= 4;
            }
            if (prim->b3 > 8) {
                prim->b3 -= 4;
            }
            if (prim->g0 > 8) {
                prim->g0 -= 4;
            }
            if (prim->g1 > 8) {
                prim->g1 -= 4;
            }
            if (prim->g2 > 8) {
                prim->g2 -= 4;
            }
            if (prim->g3 > 8) {
                prim->g3 -= 4;
            }
        } else {
            prim->x0 = prim->x2 = D_170000_8017CB88[i];
            prim->x1 = prim->x3 = D_170000_8017CB88[(i + 1) % 11];
            prim->y0 = D_170000_8017CBA0[i] - self->ext.shield.unk96;
            prim->y2 = D_170000_8017CBA0[i] + self->ext.shield.unk96;
            prim->y1 = D_170000_8017CBA0[(i + 1) % 11] - self->ext.shield.unk96;
            prim->y3 = D_170000_8017CBA0[(i + 1) % 11] + self->ext.shield.unk96;
            prim->priority = D_170000_8017CBB8[i];
        }
        prim->drawMode = DRAW_TPAGE2 | DRAW_TPAGE | DRAW_TRANSP;
        prim = prim->next;
    }
}

s32 func_ptr_80170004(Entity* self) {
    Primitive* prim;
    s16 playerY;
    s16 primWidth;
    s16 primXCenter;
    s16 primYCenter;
    u16 playerX;
    s16 primHeight;
    u8* uvPtr;

    s32 i;

    switch (self->step) {
    case 0:
        self->primIndex = g_api.AllocPrimitives(PRIM_GT4, 11);
        if (self->primIndex == -1) {
            DestroyEntity(self);
            return;
        }
        if (g_HandId) {
            self->ext.shield.unk86 = 0x129;
            self->ext.shield.unk7D = 0x80;
        } else {
            self->ext.shield.unk86 = 0x111;
            self->ext.shield.unk7D = 0;
        }
        uvPtr = &D_170000_8017ABC4;
        playerX = PLAYER.posX.i.hi;
        playerY = PLAYER.posY.i.hi - 8;
        self->flags = FLAG_UNK_04000000 | FLAG_HAS_PRIMS | FLAG_UNK_10000;
        prim = &g_PrimBuf[self->primIndex];
        for (i = 0; i < 11; i++) {
            prim->u0 = prim->u2 = *uvPtr++;
            prim->v0 = prim->v1 = *uvPtr++;
            prim->u1 = prim->u3 = *uvPtr++;
            prim->v2 = prim->v3 = *uvPtr++;
            prim->v0 += self->ext.shield.unk7D;
            prim->v1 += self->ext.shield.unk7D;
            prim->v2 += self->ext.shield.unk7D;
            prim->v3 += self->ext.shield.unk7D;
            prim->tpage = 0x19;
            prim->clut = self->ext.shield.unk86 + i;
            prim->priority = self->zPriority - 2;
            prim->drawMode = DRAW_DEFAULT;
            prim->r0 = (rand() & 0xF) + 1;
            D_170000_8017CBD0[i][0].i.hi = playerX;
            D_170000_8017CBD0[i][1].i.hi = playerY;
            D_170000_8017CBD0[i][2].val = (rand() * 6) + FIX(-1.5);
            D_170000_8017CBD0[i][3].val = FIX(-2) - (rand() * 4);
            prim = prim->next;
        }
        self->ext.shield.unk94 = 0x20;
        self->ext.shield.unk80 = 0x20;
        self->step++;
        break;
    case 1:
        self->ext.shield.unk94 += 0x10;
        if (self->ext.shield.unk94 > 0xC0) {
            self->ext.shield.unk94 = 0xC0;
        }
        for (i = 0; i < 11; i++) {
            D_170000_8017CBD0[i][0].val += D_170000_8017CBD0[i][2].val;
            D_170000_8017CBD0[i][1].val += D_170000_8017CBD0[i][3].val;
            D_170000_8017CBD0[i][3].val += 0x2000;
        }
        if (--self->ext.shield.unk80 == 0) {
            prim = &g_PrimBuf[self->primIndex];
            for (i = 0; i < 11; i++) {
                prim->g0 = 1;
                prim = prim->next;
            }
            self->ext.shield.unk80 = 0x60;
            self->step++;
        }
        break;
    case 2:
        if ((self->ext.shield.unk80 >= 0x48) &&
            (self->ext.shield.unk80 % 4 == 0)) {
            g_api.PlaySfx(SFX_WEAPON_SWISH_C);
        }
        if (--self->ext.shield.unk80 == 0) {
            DestroyEntity(self);
            return;
        }
        break;
    }
    prim = &g_PrimBuf[self->primIndex];
    for (i = 0; i < 11; i++) {
        switch (prim->g0) {
        case 0:
            primXCenter = D_170000_8017CBD0[i][0].i.hi;
            primYCenter = D_170000_8017CBD0[i][1].i.hi;
            if (i == 6) {
                primHeight = self->ext.shield.unk94 * 12 / 256;
            } else {
                primHeight = self->ext.shield.unk94 * 16 / 256;
            }
            primWidth = self->ext.shield.unk94 * 12 / 256;
            prim->x0 = prim->x2 = primXCenter - primWidth;
            prim->x1 = prim->x3 = primXCenter + primWidth;
            prim->y0 = prim->y1 = primYCenter - primHeight;
            prim->y2 = prim->y3 = primYCenter + primHeight;
            break;
        case 1:
            if (--prim->r0 == 0) {
                prim->g0++;
            }
            break;
        case 2:
            if ((prim->x1 - prim->x0) >= 5) {
                prim->x0++;
                prim->x1--;
            } else {
                prim->x0 = prim->x1 - 2;
            }
            prim->y0 -= 18;
            prim->y2 -= 6;
            prim->y1 = prim->y0;
            prim->y3 = prim->y2;
            prim->x2 = prim->x0;
            prim->x3 = prim->x1;
            if (prim->y2 < 0) {
                prim->g0++;
            }
            break;
        case 3:
            prim->drawMode = DRAW_HIDE;
            break;
        }
        prim = prim->next;
    }
}

static void func_ptr_80170024(Entity* self) {}

static void func_ptr_80170028(Entity* self) {}

static void WeaponUnused2C(void) {}

static void WeaponUnused30(void) {}

static void WeaponUnused34(void) {}

static void WeaponUnused38(void) {}

static void WeaponUnused3C(void) {}
