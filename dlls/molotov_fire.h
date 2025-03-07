//just expirimenting
//this is from sticky gibs, turning this into flame 

//#if 0

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "soundent.h"
#include "decals.h"
#include "animation.h"
#include "weapons.h"
#include "func_break.h"

extern Vector VecBModelOrigin(entvars_t* pevBModel);

class CFlames : public CBaseEntity
{
public:
	void Spawn(const char* szFlameModel);
	void EXPORT FlameTouch(CBaseEntity* pOther);
	void EXPORT WaitTillLand();
	void LimitVelocity();

	int ObjectCaps() override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }
	static void SpawnFlames(entvars_t* pevVictim, Vector vecOrigin, int cFlames);

	int m_bloodColor;
	int m_cBloodDecals;
	int m_material;
	float m_lifeTime;
};


// HACKHACK -- The gib velocity equations don't work
void CFlames::LimitVelocity()
{
	float length = pev->velocity.Length();

	// ceiling at 1500.  The gib velocity equation is not bounded properly.  Rather than tune it
	// in 3 separate places again, I'll just limit it here.
	if (length > 1500.0)
		pev->velocity = pev->velocity.Normalize() * 1500; // This should really be sv_maxvelocity * 0.75 or something
}


void CFlames::SpawnFlames(entvars_t* pevVictim, Vector vecOrigin, int cFlames)
{
	int i;

	for (i = 0; i < cFlames; i++)
	{
		CFlames* pFlames = GetClassPtr((CFlames*)NULL);

		pFlames->Spawn("sprites/fire.spr");
		pFlames->pev->body = RANDOM_LONG(0, 2);


			pFlames->pev->origin.x = vecOrigin.x + RANDOM_FLOAT(-3, 3);
			pFlames->pev->origin.y = vecOrigin.y + RANDOM_FLOAT(-3, 3);
			pFlames->pev->origin.z = vecOrigin.z + RANDOM_FLOAT(-3, 3);

			/*
			pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * (RANDOM_FLOAT ( 0 , 1 ) );
			*/

			// make the gib fly away from the attack vector
			pFlames->pev->velocity = g_vecAttackDir * -1;

			// mix in some noise
			pFlames->pev->velocity.x += RANDOM_FLOAT(-0.15, 0.15);
			pFlames->pev->velocity.y += RANDOM_FLOAT(-0.15, 0.15);
			pFlames->pev->velocity.z += RANDOM_FLOAT(-0.15, 0.15);

			pFlames->pev->velocity = pFlames->pev->velocity * 900;

			pFlames->pev->avelocity.x = RANDOM_FLOAT(250, 400);
			pFlames->pev->avelocity.y = RANDOM_FLOAT(250, 400);


				pFlames->pev->velocity = pFlames->pev->velocity * 4;
	


			pFlames->pev->movetype = MOVETYPE_TOSS;
			pFlames->pev->solid = SOLID_BBOX;
			UTIL_SetSize(pFlames->pev, Vector(0, 0, 0), Vector(0, 0, 0));
			pFlames->SetTouch(&CFlames::FlameTouch);
			pFlames->SetThink(NULL);
			//		}
		pFlames->LimitVelocity();
	}
}



void CFlames::FlameTouch(CBaseEntity* pOther)
{
	Vector vecSpot;
	TraceResult tr;

	SetThink(&CFlames::SUB_Remove);
	pev->nextthink = gpGlobals->time + 10;

	if (!FClassnameIs(pOther->pev, "worldspawn"))
	{
		pev->nextthink = gpGlobals->time;
		return;
	}

	UTIL_TraceLine(pev->origin, pev->origin + pev->velocity * 32, ignore_monsters, ENT(pev), &tr);

	//UTIL_BloodDecalTrace(&tr, m_bloodColor);

	pev->velocity = tr.vecPlaneNormal * -1;
	pev->angles = UTIL_VecToAngles(pev->velocity);
	pev->velocity = g_vecZero;
	pev->avelocity = g_vecZero;
	pev->movetype = MOVETYPE_NONE;
}

void CFlames::WaitTillLand()
{
	if (!IsInWorld())
	{
		UTIL_Remove(this);
		return;
	}

	if (pev->velocity == g_vecZero)
	{
		SetThink(&CFlames::SUB_StartFadeOut);
		pev->nextthink = gpGlobals->time + m_lifeTime;

		// If you bleed, you stink!
//		if (m_bloodColor != DONT_BLEED)
//		{
//			// ok, start stinkin!
//			CSoundEnt::InsertSound(bits_SOUND_MEAT, pev->origin, 384, 25);
//		}
	}
	else
	{
		// wait and check again in another half second.
		pev->nextthink = gpGlobals->time + 0.5;
	}
}

void CFlames::Spawn(const char* szFlameModel)
{
	pev->movetype = MOVETYPE_BOUNCE;
	pev->friction = 0.55; // deading the bounce a bit

	// sometimes an entity inherits the edict from a former piece of glass,
	// and will spawn using the same render FX or rendermode! bad!
	pev->renderamt = 255;
	pev->rendermode = kRenderNormal;
	pev->renderfx = kRenderFxNone;
	pev->solid = SOLID_SLIDEBOX; /// hopefully this will fix the VELOCITY TOO LOW crap
	pev->classname = MAKE_STRING("flame");

	SET_MODEL(ENT(pev), szFlameModel);
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	pev->nextthink = gpGlobals->time + 4;
	m_lifeTime = 25;
	SetThink(&CFlames::WaitTillLand);
	SetTouch(&CFlames::FlameTouch);

	m_material = matNone;
//	m_cBloodDecals = 5; // how many blood decals this gib can place (1 per bounce until none remain).
}

//#endif