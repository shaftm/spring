/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#include "WeaponProjectileFactory.h"
#include "EmgProjectile.h"
#include "ExplosiveProjectile.h"
#include "FlameProjectile.h"
#include "FireBallProjectile.h"
#include "LaserProjectile.h"
#include "MissileProjectile.h"
#include "BeamLaserProjectile.h"
#include "LargeBeamLaserProjectile.h"
#include "LightningProjectile.h"
#include "StarburstProjectile.h"
#include "TorpedoProjectile.h"
#include "Sim/Misc/GlobalConstants.h"
#include "Sim/Weapons/WeaponDef.h"

unsigned int WeaponProjectileFactory::LoadProjectile(const ProjectileParams& params) {
	const WeaponDef* weaponDef = params.weaponDef;
	const CWeaponProjectile* projectile = NULL;

	assert(weaponDef != NULL);

	switch (weaponDef->projectileType) {
		case WEAPON_EMG_PROJECTILE: {
			projectile = new CEmgProjectile(params, weaponDef->visuals.color, weaponDef->intensity);
		} break;

		case WEAPON_EXPLOSIVE_PROJECTILE: {
			projectile = new CExplosiveProjectile(params, weaponDef->damageAreaOfEffect, params.gravity);
		} break;

		case WEAPON_FLAME_PROJECTILE: {
			projectile = new CFlameProjectile(params, params.spread);
		} break;

		case WEAPON_FIREBALL_PROJECTILE: {
			projectile = new CFireBallProjectile(params);
		} break;

		case WEAPON_LASER_PROJECTILE: {
			projectile = new CLaserProjectile(params, weaponDef->duration * (weaponDef->projectilespeed * GAME_SPEED), weaponDef->visuals.color, weaponDef->visuals.color2, weaponDef->intensity);
		} break;

		case WEAPON_MISSILE_PROJECTILE: {
			projectile = new CMissileProjectile(params, weaponDef->damageAreaOfEffect, weaponDef->projectilespeed);
		} break;

		case WEAPON_BEAMLASER_PROJECTILE: {
			projectile = new CBeamLaserProjectile(params, params.startAlpha, params.endAlpha, weaponDef->visuals.color);
		} break;

		case WEAPON_LARGEBEAMLASER_PROJECTILE: {
			projectile = new CLargeBeamLaserProjectile(params, weaponDef->visuals.color, weaponDef->visuals.color2);
		} break;

		case WEAPON_LIGHTNING_PROJECTILE: {
			projectile = new CLightningProjectile(params, weaponDef->visuals.color);
		} break;

		case WEAPON_STARBURST_PROJECTILE: {
			projectile = new CStarburstProjectile(params, weaponDef->damageAreaOfEffect, weaponDef->projectilespeed, params.tracking, weaponDef->uptime * GAME_SPEED, params.maxRange, params.error);
		} break;

		case WEAPON_TORPEDO_PROJECTILE: {
			projectile = new CTorpedoProjectile(params, weaponDef->damageAreaOfEffect, weaponDef->projectilespeed, params.tracking);
		} break;

		default: {
			return -1u;
		} break;
	}

	return (projectile->id);
}

