#ifndef SE_INCL_ENTITIESMP_GLOBAL_H
#define SE_INCL_ENTITIESMP_GLOBAL_H

// Temporary portability stub.
// Original Windows build path appears to generate this header from entity scripts.

enum EffectParticlesType { EPT_NONE = 0 };
enum SprayParticlesType { SPT_NONE = 0 };
enum EntityInfoBodyType { EIBT_NONE = 0 };
enum BoolEType { BET_FALSE = 0, BET_TRUE = 1 };
enum EventEType { EET_NONE = 0 };
enum DebrisParticlesType { DPT_NONE = 0 };
enum BasicEffectType { BFX_NONE = 0 };
enum SoundType { ST_NONE = 0 };
enum BulletHitType { BHT_NONE = 0 };

class CEntity;
class CModelInstance;

class CPlayerEntity : public CEntity {
public:
  void ChangeHairMesh(const CModelInstance&, int, int);
};

#endif
