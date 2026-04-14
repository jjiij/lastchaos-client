
1864
%{
#include "StdH.h"

#include <Engine/Effect/CWorldTag.h>
#include <Engine/Effect/CEffectGroup.h>
#include <Engine/Effect/CEffectGroupManager.h>
#include <Engine/Effect/CSkaEffect.h>
#include <Engine/Graphics/Color.h>
#include <Engine/Entities/ItemData.h>
#include <Engine/Network/ItemTarget.h>
#include <deque>
#include <Engine/Entities/Skill.h>
#include <Engine/Network/MessageDefine.h>
#include <Engine/Network/CharacterTarget.h>
#include <Engine/Interface/UIManager.h>
#include <Engine/Interface/UIAutoHelp.h>
#include <Engine/Object/ActorMgr.h>
#include <Common/Packet/ptype_old_do_move.h>
#include <Common/Packet/ptype_old_do_skill.h>
#include <Engine/Contents/function/TitleData.h>

#include "EntitiesMP/RestrictedField.h"
#include "EntitiesMP/Common/RestrictedFieldContainer.h"
#include "EntitiesMP/Common/CharacterDefinition.h"

#define MOB_DOOMSLAYER	158
#define MOB_BAAL		152

#define MOB_ATTACK_MOTION_NUM		(4)

//-----------------------------------------------------------------------------
// Purpose: �����̳� ���� ��� ��ƼƼ�� �ѹ��� �������� �ݴϴ�.
// Input  : pEntity - 
//			sTI - 
//			vCenter - 
// Output : static void
//-----------------------------------------------------------------------------
void DamagedTargetsInRange(CEntity* pEntity, CSelectedEntities& dcEntities, enum DamageType dmtType, FLOAT fDamageAmmount, FLOAT3D vCenter, BOOL bSkill);

void ShotFallDown(FLOAT3D vStartPos, FLOAT3D vToTargetDir, FLOAT fMoveSpeed
				 , const char *szHitEffectName, const char *szFallObjEffectName
				 , bool bCritical);
void GetTargetDirection(CEntity *penMe, CEntity *penTarget, FLOAT3D &vTargetPos, FLOAT3D &vDirection)
{
	if(penTarget == NULL) {return;}

	if(penTarget == penMe)
	{
		vTargetPos = penMe->en_plPlacement.pl_PositionVector;
		vDirection = FLOAT3D(0,1,1);
		return;
	}
	FLOAT3D vCurrentCenter(((EntityInfo*)(penMe->GetEntityInfo()))->vTargetCenter[0],
	((EntityInfo*)(penMe->GetEntityInfo()))->vTargetCenter[1],
	((EntityInfo*)(penMe->GetEntityInfo()))->vTargetCenter[2] );
	FLOAT3D vCurrentPos = penMe->en_plPlacement.pl_PositionVector + vCurrentCenter;
	FLOAT3D vTargetCenter(0, 0, 0);
	vTargetPos = FLOAT3D(0, 0, 0);
	vDirection = FLOAT3D(0, 0, 0);
	FLOAT size = 0;
	if(penTarget != NULL && penTarget->GetFlags()&ENF_ALIVE)
	{
		if(penTarget->en_RenderType == CEntity::RT_SKAMODEL)
		{
			// Bounding Box�� �̿��Ͽ� Effect�� ���� ��ġ�� ã�´�.
			FLOATaabbox3D bbox;
			penTarget->GetModelInstance()->GetCurrentColisionBox(bbox);
			FLOAT3D vRadius = (bbox.maxvect - bbox.minvect) * 0.5f * 0.4f/*�ܼ��� ������ �ϸ� �ʹ� ŭ. ������ ���� ���*/;
			size = vRadius.Length();
		}
		vTargetCenter = FLOAT3D(((EntityInfo*)(penTarget->GetEntityInfo()))->vTargetCenter[0],
		((EntityInfo*)(penTarget->GetEntityInfo()))->vTargetCenter[1],
		((EntityInfo*)(penTarget->GetEntityInfo()))->vTargetCenter[2] );
		vTargetPos = penTarget->en_plPlacement.pl_PositionVector + vTargetCenter;
		vDirection = vTargetPos - vCurrentPos;
		vDirection.Normalize();
		vTargetPos -= vDirection * size;
	}
	vDirection.Normalize(); 				
};

ENGINE_API void SetDropItemModel(CEntity *penEntity, CItemData *pItemData, CItemTarget* pTarget);
%}

uses "EntitiesMP/EnemyBase";
uses "EntitiesMP/Global";
uses "EntitiesMP/BasicEffects";
uses "EntitiesMP/CharacterBase";
uses "EntitiesMP/Character";

event EAttackDamage {//0629
};

// ���� Ÿ��.
enum AttackType
{
	0 ATK_NORMAL	"Attack Normal",
	1 ATK_MAGIC		"Attack Magic",
};

enum MobType
{
	0 MOB_ENEMY		"Boss Mob",		// �Ϲ� ��.
	1 MOB_FOLLOWPC	"Follow PC",	// PC�� ���󰡴� NPC
};

%{
// info structure
static EntityInfo eiMobBoss = {
	EIBT_FLESH, 200.0f,
	//0.0f, 1.75f*m_fStretch, 0.0f,     // source (eyes)
	//0.0f, 1.30f*m_fStretch, 0.0f,     // target (body)
	0.0f, 1.75f*1.0f, 0.0f,     // source (eyes)
	0.0f, 1.30f*1.0f, 0.0f,     // target (body)
};

//������ ���� ����	//(Effect Add & Modify for Close Beta)(0.1)
static INDEX g_iNPCMinBrightness = 32;	//���� 0~240
void CEnemy_OnInitClass(void)
{
	_pShell->DeclareSymbol("persistent user INDEX g_iNPCMinBrightness;", &g_iNPCMinBrightness);
}
//������ ���� ��	//(Effect Add & Modify for Close Beta)(0.1)

void ShotMissile(CEntity *penShoter, const char *szShotPosTagName
				 , CEntity *penTarget, FLOAT fMoveSpeed
				 , const char *szHitEffectName, const char *szArrowEffectName
				 , bool bCritical
				 , FLOAT fHorizonalOffset = 0.0f, FLOAT fVerticalOffset = 0.0f	//-1.0f ~ 1.0f
				 , INDEX iArrowType = 0, const char *szMissileModel = NULL, CSelectedEntities* dcEntities = NULL);
void ShotMagicContinued(CEntity *penShoter, FLOAT3D vStartPos, FLOATquat3D qStartRot
				 , CEntity *penTarget, FLOAT fMoveSpeed
				 , const char *szHitEffectName, const char *szMagicEffectName
				 , bool bCritical, INDEX iOrbitType, BOOL bDirectTag = FALSE);
%}

class CEnemy: CEnemyBase {
name      "Enemy";
thumbnail "Mods\\test.tbn";
features  "ImplementsOnInitClass", "HasName", "IsTargetable";

properties:
	1 enum	MobType m_MobType "Type" 'Y'		= MOB_ENEMY,	
	3 FLOAT m_fBodyAnimTime		= -1.0f,
	91 INDEX m_nMobDBIndex "Enemy List" 'E'		= 0,
	21	INDEX		m_iRegenTime "Regen Time" 'R' = 300,
	5 FLOAT m_fStretch			= 1.0f,
	80 enum AttackType m_AttackType = ATK_NORMAL,	
	99  INDEX idMob_Start		= -1,
	100 INDEX idMob_Walk		= -1,
	101 INDEX idMob_Attack		= -1,
	102 INDEX idMob_Default		= -1,
	103 INDEX idMob_Death		= -1,
	104 INDEX idMob_Wound		= -1,
	105 INDEX idMob_NormalBox	= -1,
	106 INDEX idMob_Attack2		= -1,
	107 INDEX idMob_Run			= -1,
	108 INDEX idMob_Default2	= -1,
	109 INDEX idMob_Attack3		= -1,
	110 INDEX idMob_Attack4		= -1,
	150 INDEX m_iTotalNum	"Total Num"	= -1,
	151 FLOAT	m_fScale = 1.0f,
	152 BOOL	m_bMobChange = FALSE,
// FIXME : �̱۴����� ���ؼ��� �ʿ���...��.��
// FIXME : �������� �� �и��� �ش޶� �ؼ�....
	160 BOOL	m_bBoss			= FALSE,

	200	CTString	m_strName	" Name" = "Enemy",		
//	201 INDEX	m_nMaxiHealth = 0,
//	202 INDEX	m_nCurrentHealth = 0,	
	203 INDEX	m_nMobClientIndex	= -1,
	//204 CSoundObject m_soSound2,
	//205 INDEX	m_nMobLevel = 0,
	//206 INDEX	m_nPreHealth = -1,	
	//235	INDEX m_iZoneMovingFlag "Zone Moving"		= 513,	// Zone Moving
	236 BOOL    m_bChanging = FALSE,
	237 zoneflags_ex m_zfZoneFlagBits	= 0,	// Zone Moving Flag
	238 zoneflags_ex m_efExtraBits		= 0,	// Extra Flag
	240 BOOL	m_bPreCreate				"Precreated NPC"	= FALSE,	// �̸� �����Ǿ�� �ϴ� NPC�� ���.
	245 BOOL	m_bHostageNpc				"Is hostage NPC?"	= FALSE,	// �����ؾ��ϴ� NPC�ΰ�?
	247 enum EventType	m_EventType			"Event Type"		= EVENT_NONE,
	248 CEntityPointer m_penEventTarget		"Event Target",					// Ʈ���Ÿ� ���ؼ� Blind�� �����ϵ��� ��.
	250 BOOL	m_bInit						= FALSE,
	251 BOOL	m_bIsRun					= FALSE,
	252 BOOL	m_bDie	= FALSE,
	253 INDEX   m_nChangeMonsterId = -1,
	254	FLOAT	m_fActionAnimTime = 0.0f,
	255 CSoundObject m_soMessage,  // message sounds (�ʿ��ұ?)
	//253 FLOAT m_fIdleTime = 0.0f,
	{
		CEffectGroup *m_pSkillReadyEffect;
		CStaticArray<FLOAT3D> m_aDeathItemPosDrop;
		CStaticArray< CItemTarget* > m_aDeathItemTargetDrop;
		CStaticArray<void *> m_aDeathItemDataDrop;
		INDEX	m_nMagicAttackSequence;
		sSkillEffectInfo	m_SkillEffectInfo;
		INDEX	m_nEffectStep;
		//INDEX	m_iSkillEffectStep;
		//CStaticArray<FLOAT> m_afHardCodeValue;
		//BOOL	m_bHardcodingSkillLoop;		
		CEnemyAnimation		m_AnimationIndices;
		INDEX	m_idMob_Action;
		DWORD	m_dwAniMationFlag;
	}

components:	
	// ���� ó���� ���� �����Ǵ� �κ�.
	1 sound SOUND_POLYMOPH				"Data\\sounds\\character\\public\\C_transformation.wav",
functions:

//������ ���� ����	//(Effect Add & Modify for Close Beta)(0.1)
	BOOL AdjustShadingParameters(FLOAT3D &vLightDirection, COLOR &colLight, COLOR &colAmbient)
	{
		//�� �ּ��� Ǯ�� ������� Fade-outó���� �ȴ�. �� �ణ Ƣ�� ���� ���� ����.
		//CEnemyBase::AdjustShadingParameters(vLightDirection, colLight, colAmbient);
		g_iNPCMinBrightness = Clamp(g_iNPCMinBrightness, (INDEX)0, (INDEX)240);
		BOOL bRecalcAmbient = FALSE;
		UBYTE ubAH, ubAS, ubAV;
		ColorToHSV(colAmbient, ubAH, ubAS, ubAV);
		if(ubAV < g_iNPCMinBrightness)
		{
			UBYTE ubAR, ubAG, ubAB;
			ColorToRGB(colAmbient, ubAR, ubAG, ubAB);
			ubAR = g_iNPCMinBrightness;
			ubAG = g_iNPCMinBrightness;
			ubAB = g_iNPCMinBrightness;
			ColorToHSV(RGBToColor(ubAR, ubAG, ubAB), ubAH, ubAS, ubAV);
			bRecalcAmbient = TRUE;
		}
		if(bRecalcAmbient) {colAmbient = HSVToColor(ubAH, ubAS, ubAV);}
		//return TRUE;
		return CEnemyBase::AdjustShadingParameters(vLightDirection, colLight, colAmbient);
	}
//������ ���� ��	//(Effect Add & Modify for Close Beta)(0.1)

	virtual FLOAT GetLockRotationSpeed(void) 
	{ 
		return AngleDeg(1800.0f); 
	};
	
	void OnInitialize(const CEntityEvent &eeInput)
	{
		CEnemyBase::OnInitialize(eeInput);
	};

	void OnEnd(void) 
	{
		CEnemyBase::OnEnd();		
	};	

	// ���ʹ̰� �����ϼ� �ִ��� �Ǵ���...
	virtual BOOL IsMovable()
	{
		// �� ��ȯ���̸�...
		if( m_bUseAI )
		{			
			if( m_bStuned || m_bHold )
			{
				return FALSE;
			}
		}
		return TRUE;
	};

	// ���ʹ̰� ������ �� �ִ� �����ΰ�?
	virtual BOOL IsAttackable()
	{
		if( IsFirstExtraFlagOn(ENF_EX1_NPC) )
		{
			return FALSE;		
		}
		else if( m_bUseAI )
		{
			if( m_bStuned || m_bHold )
			{
				return FALSE;
			}
		}

		return TRUE;
	};
	
	// describe how this enemy killed player
	virtual CTString GetPlayerKillDescription(const CTString &strPlayerName, const EDeath &eDeath)
	{
		CTString str;
		str.PrintF(TRANS("A %s sent %s into the halls of Valhalla") ,m_strName, strPlayerName);
		return str;
	}

	virtual BOOL IsBossMob()
	{
		return m_bBoss;
	};

	virtual BOOL IsSkillAttack( )
	{
		float frandom = FRnd();

		// ī�̶��� ��ų ���� Ȯ��.
		if( frandom > 0.2f )
		{
			// �Ϲ� ����
			return FALSE;
		}
			
		if( m_bUseAI )
		{
			// ������ �ɸ� ���¿��� ��ų ��� �Ұ�.
			if( m_bCannotUseSkill ) { return FALSE; }

			CMobData* MD = CMobData::getData(m_nMobDBIndex);

			if( MD->GetSkill0Index() <= 0 && MD->GetSkill1Index() <= 0 )
			{
				return FALSE;
			}
			
			float frandom = FRnd();
			if( frandom > 0.5f && MD->GetSkill1Index() > 0 )
			{
				m_nCurrentSkillNum	= MD->GetSkill1Index();
			}
			else
			{
				m_nCurrentSkillNum	= MD->GetSkill0Index();
			}

			CSkill& rSkill = _pNetwork->GetSkillData( m_nCurrentSkillNum );
			if( ( CalcDist(m_penEnemy) < rSkill.GetFireRange() && CanHitEnemy(m_penEnemy, Cos(AngleDeg(45.0f))) ) ) 
			{
				CPlacement3D pl		= GetLerpedPlacement();

				CNetworkMessage nmMobMove;
				RequestClient::moveForNormal* packet = reinterpret_cast<RequestClient::moveForNormal*>(nmMobMove.nm_pubMessage);
				packet->type = MSG_MOVE;
				packet->subType = 0;
				packet->charType = MSG_CHAR_NPC;
				packet->moveType = MSG_MOVE_STOP;
				packet->index = GetNetworkID();
				packet->speed = 0.0f;
				packet->x = pl.pl_PositionVector(1);
				packet->z = pl.pl_PositionVector(3);
				packet->h = pl.pl_PositionVector(2);
				packet->r = pl.pl_OrientationAngle(1);
				packet->ylayer = _pNetwork->MyCharacterInfo.yLayer;

				nmMobMove.setSize(sizeof(*packet));
				_pNetwork->SendToServerNew(nmMobMove);	
				
				{
					CNetworkMessage nmSkill(MSG_SKILL);
					RequestClient::skillReady* Readypacket = reinterpret_cast<RequestClient::skillReady*>(nmSkill.nm_pubMessage);

					Readypacket->type = MSG_SKILL;
					Readypacket->subType = MSG_SKILL_READY;

					Readypacket->skillIndex = m_nCurrentSkillNum;
					Readypacket->charType = MSG_CHAR_NPC;
					Readypacket->charIndex = GetNetworkID();
					Readypacket->targetType = MSG_CHAR_PC;
					Readypacket->targetIndex = _pNetwork->MyCharacterInfo.index;
					Readypacket->nDummySkillSpeed = 0;
					nmSkill.setSize( sizeof(*Readypacket) );

					_pNetwork->SendToServerNew(nmSkill);
				}
				{
					CNetworkMessage nmSkill(MSG_SKILL);
					RequestClient::skillFire* Firepacket = reinterpret_cast<RequestClient::skillFire*>(nmSkill.nm_pubMessage);

					Firepacket->type = MSG_SKILL;
					Firepacket->subType = MSG_SKILL_FIRE;

					Firepacket->skillIndex = m_nCurrentSkillNum;
					Firepacket->charType = MSG_CHAR_NPC;
					Firepacket->charIndex = GetNetworkID();
					Firepacket->targetType = MSG_CHAR_PC;
					Firepacket->targetIndex = _pNetwork->MyCharacterInfo.index;
					Firepacket->listCount = 0;
					nmSkill.setSize( sizeof(*Firepacket) );

					_pNetwork->SendToServerNew(nmSkill);					
				}				
				return TRUE;
			}
		}
		m_nCurrentSkillNum	= -1;
		return FALSE;
	};

	/* Entity info */
	void *GetEntityInfo(void)
	{
			return &eiMobBoss;
	};

	void Precache(void) 
	{
		CEnemyBase::Precache();		

		//-------------------------------------------------------------------------------
		if(m_bPreCreate)
		{
			int max = CMobData::getsize();
			if(_pNetwork && _pNetwork->m_bSingleMode && max)
			{
				en_pwoWorld->m_vectorPreCreateNPC.push_back(this);
			}
		}
		//-------------------------------------------------------------------------------

//������ ���� ����	//(Fix Snd Effect Bug)(0.1)
		if(en_pmiModelInstance) { en_pmiModelInstance->m_tmSkaTagManager.SetOwner(this); }
//������ ���� ��	//(Fix Snd Effect Bug)(0.1)
	};

	virtual BOOL IsEnemy(void) const
	{
		return TRUE;
	}

	void InflictDirectDamage(CEntity *penToDamage, CEntity *penInflictor, enum DamageType dmtType,
	FLOAT fDamageAmmount, const FLOAT3D &vHitPoint, const FLOAT3D &vDirection)
	{
		if(penToDamage)
		{
			if( !(penToDamage->GetFlags()&ENF_ALIVE ) )	{ return; }

			SE_Get_UIManagerPtr()->ShowDamage( penToDamage->en_ulID );
			// Date : 2005-11-16(���� 2:05:09), By Lee Ki-hwan

			INDEX preHealth, curHealth;
			preHealth = ((CUnit*)penToDamage)->m_nPreHealth;
			curHealth = ((CUnit*)penToDamage)->m_nCurrentHealth;

			if(preHealth!= -1)// && curHealth > preHealth)
			{
				fDamageAmmount = 1;
				((CUnit*)penToDamage)->m_nCurrentHealth = ((CUnit*)penToDamage)->m_nPreHealth;			
			}
			else fDamageAmmount = 0;
	
			if (penToDamage->IsEnemy() && preHealth >= 0) // ?? Enemy�� Enemy(����)�� ������ ���� �ִ°�?
			{
				const INDEX iMobIndex = ((CEnemy*)penToDamage)->m_nMobDBIndex;

				if (iMobIndex == LORD_SYMBOL_INDEX) // �޶�ũ ���� ���� NPC(������� �ٲ��� �ʰ�, ���� �ٲ۴�)
				{
					CMobData* MD = CMobData::getData(iMobIndex);
					FLOAT fMaxHealth = ((CUnit*)penToDamage)->m_nMaxiHealth;

					if ((curHealth <= fMaxHealth * 0.25f))
					{
						if (((CUnit*)penToDamage)->m_nPlayActionNum != 0)
						{
							penToDamage->SetSkaModel("data\\npc\\Gguard\\sword04.smc");
							penToDamage->GetModelInstance()->RefreshTagManager();
							penToDamage->GetModelInstance()->StretchModel(FLOAT3D(MD->GetScale(), MD->GetScale(), MD->GetScale()));
							((CUnit*)penToDamage)->m_nPlayActionNum = 0;
						}
					}
					else if ((curHealth <= fMaxHealth * 0.50f))
					{
						if (((CUnit*)penToDamage)->m_nPlayActionNum != 1)
						{
							penToDamage->SetSkaModel("data\\npc\\Gguard\\sword03.smc");
							penToDamage->GetModelInstance()->RefreshTagManager();
							penToDamage->GetModelInstance()->StretchModel(FLOAT3D(MD->GetScale(), MD->GetScale(), MD->GetScale()));
							((CUnit*)penToDamage)->m_nPlayActionNum = 1;
						}
					}
					else if ((curHealth <= fMaxHealth * 0.75f))
					{
						if (((CUnit*)penToDamage)->m_nPlayActionNum != 2)
						{
							penToDamage->SetSkaModel("data\\npc\\Gguard\\sword02.smc");
							penToDamage->GetModelInstance()->RefreshTagManager();
							penToDamage->GetModelInstance()->StretchModel(FLOAT3D(MD->GetScale(), MD->GetScale(), MD->GetScale()));
							((CUnit*)penToDamage)->m_nPlayActionNum = 2;
						}
					}
				}
			}

			((CUnit*)penToDamage)->m_nPreHealth = -1; //1103
		}

		CEntity::InflictDirectDamage(penToDamage, penInflictor, dmtType,
			fDamageAmmount,vHitPoint, vDirection);	

		if(penToDamage )
		{
			if(((CUnit*)penToDamage)->m_nCurrentHealth <= 0 && penToDamage != this)
			{
				((CUnit*)((CEntity*)penToDamage))->DeathNow();
			}
		}
	};
 
	//--------------------------------------------------------------
	// �������� �Ծ��� ����� ó��...
	//--------------------------------------------------------------
	/* Receive damage */
	void ReceiveDamage(CEntity *penInflictor, enum DamageType dmtType,
		FLOAT fDamageAmmount, const FLOAT3D &vHitPoint, const FLOAT3D &vDirection) 
	{
		if(dmtType != DMT_NONE && fDamageAmmount > 0)
		{
			FLOAT3D vAxisY(0,1,0);
			FLOAT angle = acos(vDirection % vAxisY);
			FLOAT3D axis = vAxisY * vDirection;
			axis.Normalize();
			FLOATquat3D quat;
			quat.FromAxisAngle(axis, angle);

			INDEX iHitType = 0;
			INDEX iJob = 0;

			if (m_enAttackerID == _pNetwork->MyCharacterInfo.index)
			{
				iJob = _pNetwork->MyCharacterInfo.job;
				iHitType = _pNetwork->MyCharacterInfo.iHitEffectType;
			}

			ObjectBase* pObj = ACTORMGR()->GetObject(eOBJ_CHARACTER, m_enAttackerID);
			if (pObj != NULL)
			{
				CCharacterTarget* pCT = static_cast<CCharacterTarget*>(pObj);

				if (pCT != NULL)
				{
					iJob = pCT->GetType();
					iHitType = pCT->cha_iHitType;
				}
			}
			
			if (iHitType < 0 || iHitType >= DEF_HIT_TYPE_MAX)
			{
				iHitType = 0;
			}				

			StartEffectGroup(szHitEffect[iJob][iHitType], 
				_pTimer->GetLerpedCurrentTick(), vHitPoint, quat);
		}
		SE_Get_UIManagerPtr()->ShowDamage( en_ulID );

		// ��ȭ NPC(���� ��� ����)�� �ƴ� ��쿡�� Ÿ���� ���� ������ ����ǵ��� ��.
		if( m_bUseAI && !( GetFirstExFlags() & ENF_EX1_PEACEFUL ) )
		{
			// ������ ���� ������ �ٲ�� �ȵɰ� ���Ƽ�, �����ϰ� �ٲ�� ������.
			FLOAT fRand = FRnd();		
			if( fRand > 0.5f )
			{
				// ���� �� �߿��� �÷��̾ ��ȯ���� ��쿡��...
				if( penInflictor->IsPlayer() || penInflictor->IsSlave() || penInflictor->IsWildPet())
				{
					// ���� ������ Ÿ���� �����մϴ�.
					SetTargetEntity(penInflictor);				
				}
			}			
		}
		
		CEnemyBase::ReceiveDamage(penInflictor, dmtType, fDamageAmmount, vHitPoint, vDirection);
	};

	//--------------------------------------------------------------
	// ������ �ִϸ��̼��� �����ϴ� �Լ���...
	//--------------------------------------------------------------
	// damage anim
	INDEX AnimForDamage(FLOAT fDamage) 
	{
		ASSERT(idMob_Wound		!= -1	&& "Invalid Animation");
		ASSERT(GetModelInstance() != NULL && "Invalid ModelInstance");

		// NOTE : ����ž�� ��쿡�� ������ �ִϸ��̼��� �������� �ʴ´�...
		CMobData* MD = CMobData::getData(m_nMobDBIndex);
		if(MD->IsCastleTower())
		{
			return -1;
		}

		//0820 ������ ���������� ������ ����� ���;� �ȴ�. ��!
		if( !IsAnimationPlaying(idMob_Default) && 
			!IsAnimationPlaying(idMob_Default2) )
		{
			INDEX animSet, animIdx;
			static INDEX animId = ska_StringToID("mldam");//HardCoding
			if(this->en_pmiModelInstance
			&& this->en_pmiModelInstance->FindAnimationByID(animId, &animSet, &animIdx))
			{
				//Ÿ�� ���
				this->en_pmiModelInstance->AddAnimation(animId, AN_REMOVE_ON_END, 1.0f, 0);
			}
			return animId;
		}

		NewClearState(CLEAR_STATE_LENGTH);    
		GetModelInstance()->AddAnimation(idMob_Wound,AN_CLEAR,1,0);	
		
		return idMob_Wound;
	};

	// death
	INDEX AnimForDeath(void)
	{	
		ASSERT(idMob_Death		!= -1	&& "Invalid Animation");
	
		DropDeathItem();//1019
				
		ASSERT(GetModelInstance() != NULL && "Invalid ModelInstance");
		NewClearState(CLEAR_STATE_LENGTH);    
		GetModelInstance()->AddAnimation(idMob_Death,AN_NORESTART,1,0);

		return idMob_Death;
	};

	// ������ ������...
	FLOAT WaitForDust(FLOAT3D &vStretch)
	{
		//0317 kwon
		vStretch=FLOAT3D(1,1,1);		
		if(GetModelInstance()->IsAnimationPlaying(idMob_Death)) 
		{
			return 0.5f;
		} 
		return -1.0f;
	};

	void DeathNotify(void) 
	{
		// ���� ����...
		en_fDensity = 500.0f;

		if(m_EventType == EVENT_MOBDESTROY)
		{
			SendToTarget(m_penEventTarget, EET_TRIGGER, this);
		}

		if(_pNetwork->m_bSingleMode)
		{
			--_pNetwork->wo_dwEnemyCount;

			// NOTE : �������ġ�� ��쿡�� ī��Ʈ���� ����.
			if( m_nMobDBIndex != 220 )
			{
				_pNetwork->wo_dwKilledEnemyCount++;
			}

			if (g_slZone == 6) // Ʃ�丮�� ��������
			{
				if (_pNetwork->wo_dwKilledEnemyCount == 1)
				{
					_UIAutoHelp->SetGMNotice(_S( 5037, "���콺 ���� ��ư���� ���ϴ� �������� Ŭ�� �ϰų� Ű���� F2 ��ư�� ��������."));
				}
				else if (_pNetwork->wo_dwKilledEnemyCount == 5)
				{
					_UIAutoHelp->SetGMNotice(_S( 5036, "Ÿ���� Ŭ�� �� Ű���� F1��ư���� ��ų�� ������ ������."));
				}
				else if (m_nMobDBIndex == 1129) // ���� ���� �׿��� ��
				{
					_pNetwork->SendQuestMessage(MSG_QUEST_COMPLETE, 45);
				}
			}
			
			// Ư�� ���� ���� Ȯ���մϴ�.
			CRestrictedField *pField = FindPlayerInRestrictedField((CPlayer*)CEntity::GetPlayerEntity(0));
			if(pField)
			{
				if( _pNetwork->wo_dwEnemyCount <= pField->iMobCount )
				{
					pField->m_bActive	= FALSE;
					SendToTarget(pField->m_penTarget, EET_TRIGGER, CEntity::GetPlayerEntity(0));
				}
			}

			if(_pNetwork->m_ubGMLevel > 1)
			{
				CTString strMessage;
				strMessage.PrintF("=====current enemies : %d=====\n", _pNetwork->wo_dwEnemyCount);
				_pNetwork->ClientSystemMessage(strMessage);
			}
		}
	};

	// Ŭ���������� �̺�Ʈ.
	void ClickedEvent(void)
	{
		if(m_EventType == EVENT_MOBCLICK)
		{
			SendToTarget(m_penEventTarget, EET_TRIGGER, this);
		}
	};

	void StartAnim(void)
	{
		ASSERT(idMob_Start	!= -1	&& "Invalid Animation");
		ASSERT(GetModelInstance() != NULL && "Invalid ModelInstance");
		GetModelInstance()->AddAnimation(idMob_Start,AN_NORESTART|AN_CLEAR,1,0);
	};

	// virtual anim functions
	void StandingAnim(void) 
	{		
		m_bIsRun = FALSE;

		float fRnd = FRnd();
		INDEX iIdleAni;

		if (fRnd < 0.4f)
		{
			iIdleAni = idMob_Default2;
		}
		else
		{
			iIdleAni = idMob_Default;
		}

		ASSERT(iIdleAni	!= -1	&& "Invalid Animation");
		ASSERT(GetModelInstance() != NULL && "Invalid ModelInstance");

		GetModelInstance()->AddAnimation(idMob_Default,AN_LOOPING|AN_NORESTART|AN_CLEAR,1,0);
	};
	void RunningAnim(void) 
	{
		m_bIsRun = TRUE;
		ASSERT(idMob_Run		!= -1	&& "Invalid Animation");
		ASSERT(GetModelInstance() != NULL && "Invalid ModelInstance");
		GetModelInstance()->AddAnimation(idMob_Run,AN_LOOPING|AN_NORESTART|AN_CLEAR,1,0);
	};
	void WalkingAnim(void) 
	{
		m_bIsRun = FALSE;
		ASSERT(idMob_Walk		!= -1	&& "Invalid Animation");
		ASSERT(GetModelInstance() != NULL && "Invalid ModelInstance");
		GetModelInstance()->AddAnimation(idMob_Walk,AN_LOOPING|AN_NORESTART|AN_CLEAR,1,0);		
	};
	void RotatingAnim(void) 
	{
		if(m_bIsRun)
		{
			RunningAnim();
		}
		else
		{
			WalkingAnim();
		}
	};
	void AttackAnim(void) 
	{
		ASSERT(idMob_Attack		!= -1		&& "Invalid Animation");
		ASSERT(GetModelInstance() != NULL	&& "Invalid ModelInstance");

		static FLOAT	start_attack_time=0;
		static float  m_fBodyAnimTime=0;
		
		if(start_attack_time==0)
		{
			start_attack_time = _pTimer->CurrentTick();
			GetModelInstance()->AddAnimation(idMob_Attack,AN_NORESTART|AN_CLEAR,1.0f,0);
			m_fBodyAnimTime = GetModelInstance()->GetAnimLength(idMob_Attack);
		}
		if(start_attack_time !=0 && (_pTimer->CurrentTick() - start_attack_time >= m_fBodyAnimTime))
		{
			m_bAttack = FALSE;	
			start_attack_time = 0;		
		}

	};

	// adjust sound and watcher parameters here if needed
	void EnemyPostInit(void) 
	{
		// set sound default parameters
		m_soSound.Set3DParameters(30.0f, 5.0f, 1.0f, 1.0f);
	};

	
//������ ���� ����	//(5th Closed beta)(0.2)
	void AddDeathItem(FLOAT3D pos, class CItemTarget *pItemTarget, class CItemData *pItemData)
	{
		INDEX cnt = m_aDeathItemTargetDrop.Count();
		m_aDeathItemPosDrop.Expand(cnt + 1);
		m_aDeathItemPosDrop[cnt] = pos;
		m_aDeathItemTargetDrop.Expand(cnt + 1);
		m_aDeathItemTargetDrop[cnt] = pItemTarget;
		m_aDeathItemDataDrop.Expand(cnt + 1);
		m_aDeathItemDataDrop[cnt] = pItemData;
	}

	virtual void DropDeathItem()
	{
		for(INDEX i = 0; i < m_aDeathItemTargetDrop.Count(); ++i )
		{
			CItemTarget* pTarget = NULL;
			pTarget = m_aDeathItemTargetDrop[i];

			CItemData* pItemData = ((CItemData*)m_aDeathItemDataDrop[i]);
			const char* szItemData = _pNetwork->GetItemName( pItemData->GetItemIndex() );
			CEntity* penEntity = NULL;
			
			CPlacement3D plPlacement;
			plPlacement.pl_PositionVector = m_aDeathItemPosDrop[i];
			plPlacement.pl_OrientationAngle = ANGLE3D(0.0f,0.0f,0.0f);
			
			ASSERT(pTarget->m_pEntity == NULL);
			penEntity = _pNetwork->ga_World.CreateEntity_t(plPlacement, CLASS_ITEM,-1,TRUE);
			if(penEntity == NULL) {continue;}

			penEntity->SetNetworkID(pTarget->GetSIndex());

			pTarget->m_pEntity = penEntity;
			pTarget->m_nIdxClient = penEntity->en_ulID;
			penEntity->en_strItemName = szItemData;
			SetDropItemModel(penEntity, pItemData, pTarget);

			ACTORMGR()->AddObject(pTarget);

			//��� ���� �÷���
			if( pItemData->GetType() == CItemData::ITEM_ETC
				&& pItemData->GetType() == CItemData::ITEM_ETC_MONEY )
			{
				((CPlayerEntity*)CEntity::GetPlayerEntity(0))->PlayItemSound(FALSE, TRUE);
			}
			else
			{
				((CPlayerEntity*)CEntity::GetPlayerEntity(0))->PlayItemSound(FALSE, FALSE);
			}
		}
		m_aDeathItemPosDrop.Clear();
		m_aDeathItemTargetDrop.Clear();
		m_aDeathItemDataDrop.Clear();
	}

	void SkillAndAttackFire(int num)
	{
		//ULONG MobIndex = penEntity->en_ulID;		

		if( !(m_penEnemy.ep_pen != NULL
			&& m_penEnemy->en_pmiModelInstance != NULL
			&& m_penEnemy->GetFlags() & ENF_ALIVE) )
		{
			return;
		}
		

		this->InflictDirectDamage(m_penEnemy, this, DMT_NONE, 1, FLOAT3D(0,0,0), FLOAT3D(0,0,0));
		switch(m_SkillEffectInfo.iMissileType)
		{
		case 0/*MT_NONE*/:
			{
				FLOAT3D vHitPoint;
				FLOAT3D vHitDir;
				GetTargetDirection(this, m_penEnemy, vHitPoint, vHitDir);

				if( m_dcEnemies.Count() > 0 )
				{
					DamagedTargetsInRange(this, m_dcEnemies, DMT_EXPLOSION, 1, vHitPoint, TRUE);
					m_dcEnemies.Clear();
				}
				else
				{
					//damage effect ó��
					this->InflictDirectDamage(m_penEnemy, this, DMT_NONE, 1, vHitPoint, vHitDir);
				}

				FLOAT3D vAxisY(0,1,0);
				FLOAT angle = acos(vHitDir % vAxisY);
				FLOAT3D axis = vAxisY * vHitDir;
				axis.Normalize();
				FLOATquat3D quat;
				quat.FromAxisAngle(axis, angle);
				StartEffectGroup(m_SkillEffectInfo.szEffectNameHit
								, _pTimer->GetLerpedCurrentTick()
								, vHitPoint, quat);
			} break;
		case 1/*MT_ARROW*/:
			{
				if( m_dcEnemies.Count() > 0 )
				{
					for( ENTITIES_ITERATOR it = m_dcEnemies.vectorSelectedEntities.begin();
						it != m_dcEnemies.vectorSelectedEntities.end(); ++it )
					{																		
						CEntity *pEn = (*it);
						if(pEn != NULL && pEn->IsFlagOff(ENF_DELETED))
						{
							ShotMissile( this, "RHAND", pEn, m_SkillEffectInfo.fMissileSpeed, "Normal Hit", "Normal Arrow Trace", false );
						}
					}
					m_dcEnemies.Clear();
				}
				else
				{
					ShotMissile( this, "RHAND", m_penEnemy, m_SkillEffectInfo.fMissileSpeed, "Normal Hit", "Normal Arrow Trace", false );
				}
			} break;
		case 2/*MT_DIRECT*/:
			{
				StartEffectGroup(m_SkillEffectInfo.szEffectNameHit
					, &m_penEnemy->en_pmiModelInstance->m_tmSkaTagManager
					, _pTimer->GetLerpedCurrentTick());
			} break;
		case 3/*MT_CONTINUE*/:
			{
				FLOAT3D lastEffectInfo = m_penEnemy->en_plPlacement.pl_PositionVector;
				lastEffectInfo(2) += 1;
				if(m_pSkillReadyEffect != NULL && CEffectGroupManager::Instance().IsValidCreated(m_pSkillReadyEffect))
				{
					for(INDEX i=0; i<m_pSkillReadyEffect->GetEffectCount(); ++i)
					{
						CEffect *pEffect = m_pSkillReadyEffect->GetEffectKeyVector()[i].m_pCreatedEffect;
						if(pEffect != NULL && pEffect->GetType() == ET_SKA)
						{
							CSkaEffect *pSkaEffect = (CSkaEffect*)pEffect;
							lastEffectInfo = pSkaEffect->GetInstancePosition();
							break;
						}
					}
					if(num == m_SkillEffectInfo.iFireDelayCount)
					{
						m_pSkillReadyEffect->Stop(0.04f);
					}
				}
				if( m_dcEnemies.Count() > 0 )
				{
					for( ENTITIES_ITERATOR it = m_dcEnemies.vectorSelectedEntities.begin();
						it != m_dcEnemies.vectorSelectedEntities.end(); ++it )
					{
						CEntity *pEn = (*it);
						if(pEn != NULL && pEn->IsFlagOff(ENF_DELETED))
						{
							ShotMagicContinued(this, lastEffectInfo, FLOATquat3D(1,0,0,0)
										, pEn, m_SkillEffectInfo.fMissileSpeed
										, m_SkillEffectInfo.szEffectNameHit, m_SkillEffectInfo.szEffectNameMissile
										, false, 3);
						}
					}
					m_dcEnemies.Clear();
				}
				else
				{
					ShotMagicContinued(this, lastEffectInfo, FLOATquat3D(1,0,0,0)
										, m_penEnemy, m_SkillEffectInfo.fMissileSpeed
										, m_SkillEffectInfo.szEffectNameHit, m_SkillEffectInfo.szEffectNameMissile
										, false, 3);
				}
			} break;
		case 4/*MT_INVISIBLE*/:		{} break;//�Ⱦ���. ĳ����, �÷��̾��ʵ� �̱���.
		case 5/*MT_MAGIC*/:			{} break;//���� �Ⱦ���
		case 6/*MT_INVERT*/:		{} break;//���� �Ⱦ���
		case 7/*MT_MAGECUTTER*/:	{} break;//�Ⱦ���, ĳ�� ����
		case 8/*MT_DIRECTDAMAGE*/:
			{
				StartEffectGroup(m_SkillEffectInfo.szEffectNameHit
					, &m_penEnemy->en_pmiModelInstance->m_tmSkaTagManager
					, _pTimer->GetLerpedCurrentTick());
				if( m_dcEnemies.Count() > 0 )
				{
					DamagedTargetsInRange(this, m_dcEnemies, DMT_EXPLOSION, 1, FLOAT3D(0,0,0), TRUE);
					m_dcEnemies.Clear();
				}
				else
				{
					this->InflictDirectDamage( m_penEnemy, this, DMT_NONE, 1, FLOAT3D(0,0,0), FLOAT3D(0,0,0) );
				}
			} break;
		case 9/*MT_NOTHING*/:		{} break;
		//-----boss mob hardcoding area begin-----//
		case 21://baal skill 01, like meteo strike
			{
				CSkill &skill = _pNetwork->GetSkillData(m_nCurrentSkillNum);
				static INDEX s_iFireBallCount = 10;
				static FLOAT s_fFallHeight = 3;
				static FLOAT s_fFallHeightVariation = 3;
				static FLOAT s_fSpeed = 5;
				static FLOAT s_fSpeedVariation = 5;
				for(INDEX i=0; i<s_iFireBallCount; ++i)
				{
					FLOAT3D pos = CRandomTable::Instance().GetRndCylinderPos() * skill.GetAppRange();
					pos(2) = CRandomTable::Instance().GetRndFactor() * s_fFallHeightVariation + s_fFallHeight;
					pos += m_penEnemy->GetPlacement().pl_PositionVector;
					ShotFallDown(pos, FLOAT3D(0,1,0), s_fSpeed + s_fSpeedVariation * CRandomTable::Instance().GetRndFactor()
								, m_SkillEffectInfo.szEffectNameHit, m_SkillEffectInfo.szEffectNameMissile, FALSE);
				}
			} break;
		//-----boss mob hardcoding area end-----//
		default:
			{
				ASSERTALWAYS("Something wrong... Plz check fire obj type of this skill.");
			} break;
		}
	}
// �׳� Character.es���� ������ ���
	void MinMaximize()
	{
		static BOOL bMinimize = TRUE;

		if(bMinimize)//�۾�����. 
		{
			static int cnt=0;//���� ī��Ʈ

			static unsigned int tmStartTime = unsigned int(_pTimer->GetLerpedCurrentTick()*1000);

			if(cnt==0)
			{  	
				tmStartTime = unsigned int(_pTimer->GetLerpedCurrentTick()*1000);		
				cnt++;
			}

			int time = ((unsigned int(_pTimer->GetLerpedCurrentTick()*1000) - tmStartTime )%1000)/100;
			if(time < 1)
			{
				return;
			}

			cnt = cnt+ time;

			if(cnt > 9)
			{
				cnt = 9;
			}		
			m_fScale = 0.1f*(10-cnt);
			GetModelInstance()->StretchModel(FLOAT3D( m_fScale,m_fScale,m_fScale ));

			if(cnt==9)
			{				
				if(!m_bMobChange)
				{
					//MonsterChange(m_nChangeMonsterId,FALSE);//�� �۾������� �����Ѵ�.
				}
				else
				{
					m_nChangeMonsterId = -1;
					//ReturnChange(FALSE);	//�� �۾������� ��������� ���ƿ´�.
				}

				bMinimize = FALSE;	
				cnt = 0;			
			}			
		}
		else//���� Ŀ�����Ѵ�.
		{

			static int cnt=10;//���� ī��Ʈ

			static unsigned int tmStartTime = unsigned int(_pTimer->GetLerpedCurrentTick()*1000);	
			
			if(cnt==10)
			{
				tmStartTime = unsigned int(_pTimer->GetLerpedCurrentTick()*1000);		
				cnt--;
				StartEffectGroup("Polymorph", &GetModelInstance()->m_tmSkaTagManager, _pTimer->GetLerpedCurrentTick());
				PlaySound(m_soMessage, SOUND_POLYMOPH, SOF_3D | SOF_VOLUMETRIC);	
			}
	

			int time = ((unsigned int(_pTimer->GetLerpedCurrentTick()*1000) - tmStartTime )%1000)/100;
			if(time < 1)
			{
				return;
			}

			cnt = cnt - time;

			if(cnt < 1)
			{
				cnt = 1;
			}	
			
			GetModelInstance()->StretchModel(FLOAT3D( 0.1f*(11-cnt),0.1f*(11-cnt),0.1f*(11-cnt) ));

			if(cnt==1)
			{
			
				SetSkaColisionInfo(); //�������� �浹�ڽ��� �ٽ� ��������� ������ ũ��� ���õȴ�.
				m_bChanging = FALSE;	//������ ���� ������.	
				bMinimize = TRUE;	
				cnt = 10;						
			}
		}		

	}

	void SetNickNameDamageEffect(INDEX iNickIndex, NickNameEffectType iType)
	{
		CTString  strNickDamageFile;

		if(iNickIndex > 0 && iType == NICKNAME_ATTACK_EFFECT)	// ȣĪ�� ������
		{
			strNickDamageFile = TitleStaticData::getData(iNickIndex)->GetAttackEffectFile();
		}
		else if(iNickIndex > 0 && iType == NICKNAME_DAMAGE_EFFECT)
		{
			strNickDamageFile = TitleStaticData::getData(iNickIndex)->GetDamageEffectFile();
		}

		if( strNickDamageFile != CTString(""))
		{
			StartEffectGroup(strNickDamageFile
						, &this->en_pmiModelInstance->m_tmSkaTagManager
						, _pTimer->GetLerpedCurrentTick());
		}
	}

procedures:
/************************************************************
 *                       M  A  I  N                         *
 ************************************************************/
	Main(EVoid) 
	{
		CMobData* MD = CMobData::getData(m_nMobDBIndex);

		/*if (m_nMobDBIndex > 0)
		{
			SetSkaModel(MD->GetMobSmcFileName());
			GetModelInstance()->m_tmSkaTagManager.SetOwner(this);
			FallDownToFloor();
		}*/

		if(GetModelInstance() == NULL)
		{ 
			InitAsSkaEditorModel();
			SetSkaModel("Models\\Editor\\Ska\\Axis.smc");

			if(!m_bInit)
			{
				if(m_nMobDBIndex > 0)
				{					
					// Mob Respawn Message
					if(_cmiComm.IsNetworkOn())
					{
						CPlacement3D pl = GetLerpedPlacement();
						CNetworkMessage nmMobSpawn(MSG_NPC_REGEN);
						INDEX iIndex	= -1;
						INDEX iYLayer	= 0;
						INDEX iMobType	= m_nMobDBIndex;
						INDEX iEntityID = en_ulID;					
						_pNetwork->AddRegenList(iIndex, iMobType, 
						pl.pl_PositionVector(1), pl.pl_PositionVector(2), pl.pl_PositionVector(3), 
						pl.pl_OrientationAngle(1), iYLayer, iEntityID);

						en_RenderType = RT_SKAMODEL;
					}
				}
			}
			return EEnd();
		}
	
		// declare yourself as a model
		SetPhysicsFlags(EPF_ONBLOCK_CLIMBORSLIDE
			| EPF_TRANSLATEDBYGRAVITY
			| EPF_PUSHABLE
			| EPF_MOVABLE			
			| EPF_ABSOLUTETRANSLATE /*| EPF_RT_SYNCHRONIZED*/);//|EPF_STICKYFEET|EPF_RT_SYNCHRONIZED);
		SetPhysicsFlags(GetPhysicsFlags() &~EPF_PUSHABLE);
// NOTE : �������õǼ�...
		// �̺�Ʈ NPC�� �浹 üũ

		// [070607: Su-won] ���忡���Ϳ����� �׳� ���.
		// �����ͻ󿡼��� CNetworkLibrary::ga_World.wo_aMobData �� NPC �����Ͱ� ������� �ʾƼ�
		// CNetworkLibrary::GetMobData() �Լ��� ���ؼ� ����ε� NPC �����͸� �������� ���ϱ⶧����
		// ���忡 ��ġ�� NPC�� ���� ��Ÿ������ �� �� �����߻���.
		extern BOOL _bWorldEditorApp;
		if( !_bWorldEditorApp )
		{
			if(MD->IsEvent() || MD->IsLordSymbol() || MD->IsCollsionModel())
			{
				SetCollisionFlags(ECF_MODEL);		
			}
			else 
			{
				SetCollisionFlags(ECF_MODEL_NO_COLLISION);
			}
		}

		en_sbNetworkType = MSG_CHAR_NPC;

//������ ���� ���� �׽�Ʈ Ŭ���̾�Ʈ �۾�	06.22
		if(!_pNetwork->m_bSingleMode || m_bPreCreate)
		{		
			// ���� ���õǼ�... hp�� 0�̶��...
			// ���� NPC������ ��������� �ʴ� NPC��...(�׸��� Ÿ���õ� �ȵǵ���...)
			/*if( (MD->IsCastleTower()) && m_nCurrentHealth <= 0 || m_nMobDBIndex == 491)
			{
				SetFlagOff(ENF_ALIVE);
			}
			else*/
			{
				SetFlagOn(ENF_ALIVE);
			}
		}
		else
		{
			// �̱� ����� ���.
			m_bUseAI				= TRUE;
			m_fIgnoreRange			= 1000.0f;
			SetFlagOff(ENF_ALIVE);
		}
//������ ���� �� �׽�Ʈ Ŭ���̾�Ʈ �۾�		06.22
		en_tmMaxHoldBreath = 5.0f;
		en_fDensity = 2000.0f;
		if(	idMob_Walk			== -1 ||
			idMob_Run			== -1 ||
			idMob_Attack		== -1 ||
			idMob_Default		== -1 ||
			idMob_Death			== -1 ||
			idMob_Wound			== -1 ||
			idMob_NormalBox		== -1)
			{ 
				return EEnd();
			}

		SetFlagOn(ENF_RENDERREFLECTION);

		// setup moving speed
		//m_fWalkSpeed			= FRnd() + 0.5f;
		//m_fWalkSpeed			= 4.0f;
		m_aWalkRotateSpeed		= AngleDeg(1800.0f);
		//m_fAttackRunSpeed		= FRnd() + 1.0f;
		//m_fAttackRunSpeed		= 4.0f;
		m_aAttackRotateSpeed	= AngleDeg(1800.0f);
		//m_fCloseRunSpeed		= FRnd() + 1.0f;
		//m_fCloseRunSpeed		= 4.0f;
		m_aCloseRotateSpeed		= AngleDeg(1800.0f);

		// setup attack distances
		//m_fAttackDistance		= 5.0f;
		//m_fCloseDistance		= 20.0f;		// �ٰŸ� ���????
		//m_fStopDistance			= 1.5f;			// (Stop Distance���� �����ٸ� ���� �Ѿư��� ����.)
		m_fAttackFireTime		= 2.0f;
		m_fCloseFireTime		= 2.0f;
		m_fBlowUpAmount			= 80.0f;
		m_fBodyParts			= 4;
		m_fDamageWounded		= 0.0f;
//				m_iScore				= 0;

		m_nMobClientIndex		= en_ulID;	

//������ ���� ����	//(Effect Add & Modify for Close Beta)(0.1)
		if(GetModelInstance() != NULL)
		{
			// set stretch factors for height and width
			GetModelInstance()->StretchModel(FLOAT3D(m_fStretch, m_fStretch, m_fStretch));

			CSkaTag tag;
			tag.SetName("__ROOT");
			tag.SetOffsetRot(GetEulerAngleFromQuaternion(en_pmiModelInstance->mi_qvOffset.qRot));
			GetModelInstance()->m_tmSkaTagManager.Register(&tag);
			FLOATaabbox3D aabb;
			GetModelInstance()->GetAllFramesBBox(aabb);
			tag.SetName("CENTER");
			tag.SetOffsetRot(GetEulerAngleFromQuaternion(en_pmiModelInstance->mi_qvOffset.qRot));
			tag.SetOffsetPos(0, aabb.Size()(2) * 0.5f * GetModelInstance()->mi_vStretch(2), 0);
			GetModelInstance()->m_tmSkaTagManager.Register(&tag);
			tag.SetName("__TOP");
			tag.SetOffsetRot(GetEulerAngleFromQuaternion(en_pmiModelInstance->mi_qvOffset.qRot));
			tag.SetOffsetPos(0, aabb.Size()(2) * GetModelInstance()->mi_vStretch(2) + 0.5f, 0);
			GetModelInstance()->m_tmSkaTagManager.Register(&tag);
		}
//������ ���� ��	//(Effect Add & Modify for Close Beta)(0.1)

		extern BOOL	_bWorldEditorApp;
		if( !_bWorldEditorApp )
		{
			// ���̾�Ʈ ������ ��� �׸��� �ȳ�������...
			CMobData* MD = CMobData::getData(m_nMobDBIndex);

			// TRUE �� ���� �Ѿ�´�. 
			// �׸��� Cast ������ ���� FALSE ���� �ʿ�
			if (MD->IsNPCFlag2nd(NPC_NO_SHADOW_CAST) == FALSE)
			{
				GetModelInstance()->mi_bRenderShadow = TRUE;
			}
			else
			{
				GetModelInstance()->mi_bRenderShadow = FALSE;
			}
		}

		SetHealth(10000.0f);//0807 ���� ü��.
//		m_soSound2.Set3DParameters(25.0f, 5.0f, 1.0f, 1.0f);

		if(idMob_Start != -1)
		{
			FLOAT fAnimLength = GetAnimLength(idMob_Start);
			//GetModelInstance()->AddAnimation(idMob_Start,AN_NORESTART,1,0);
			StartAnim();
//			PlaySound(m_soSound, SOUND_ZOMBI_APPEAR, SOF_3D|SOF_VOLUMETRIC);
			autowait(fAnimLength);
		}

		ModelChangeNotify();
		//StandingAnim();

		m_dwAniMationFlag = 0;
		m_idMob_Action = 0;

		// continue behavior in base class
		jump CEnemyBase::MainLoop();
	};

/************************************************************
 *                A T T A C K   E N E M Y                   *
 ************************************************************/
	Action(EVoid) : CEnemyBase::Action
	{
		if (m_idMob_Action > 0)
		{
			m_bAction = FALSE;
			GetModelInstance()->AddAnimation(m_idMob_Action, m_dwAniMationFlag, 1.0f, 0);
			
			if (IsFlagOn(ENF_INVISIBLE))
			{
				SetFlagOff(ENF_INVISIBLE);
			}

			autowait(GetAnimLength(m_idMob_Action)*0.8f);
			m_bStop = TRUE;
		}

		return EReturn();
	}

	Fire(EVoid) : CEnemyBase::Fire
	{
		jump CEnemyBase::Fire();
	}

	Hit(EVoid) : CEnemyBase::Hit 
	{
		if( !IsAttackable() )
		{
			return EReturn();
		}

		if (m_MobType == MOB_ENEMY) 
		{
			autocall NormalAttack() EEnd;
		// should never get here
		} 
		else
		{
			ASSERT(FALSE);
		}
		return EReturn();
	};
		
	//0628 kwon
	AttackTarget(EVoid) : CEnemyBase::AttackTarget // �� ���ν����� call�ϱ����� SetTargetEntity()�� ����Ǿ�� �Ѵ�.
	{
		m_vDesiredPosition = PlayerDestinationPos();
		
		INDEX attackAnimID = -1;
		const int iAttackMotion = m_nAttackCnt % MOB_ATTACK_MOTION_NUM;
		if( iAttackMotion == 0 )
		{
			attackAnimID = idMob_Attack;
		}
		else if( iAttackMotion == 1 )
		{
			attackAnimID = idMob_Attack2;
		}
		else if( iAttackMotion == 2 )
		{
			attackAnimID = idMob_Attack3;
		}
		else if( iAttackMotion == 3 )
		{
			attackAnimID = idMob_Attack4;
		}

		m_SkillEffectInfo.InitForNormalAttack(CMobData::getData(m_nMobDBIndex), attackAnimID);

		if(strlen(m_SkillEffectInfo.szEffectNameHit) == 0) {m_SkillEffectInfo.szEffectNameHit = "Normal Hit";}
		if(m_SkillEffectInfo.iFireDelayCount == 0)
		{
			m_SkillEffectInfo.iMissileType = CSkill::MT_NONE;
			m_SkillEffectInfo.iFireDelayCount = 1;
			m_SkillEffectInfo.fFireDelay[0] = GetAnimLength(idMob_Attack)/3;
		}
		autocall SkillAndMagicAnimation() EReturn;

		return EReturn();
	};

	SkillingTarget(EVoid) : CEnemyBase::SkillingTarget // �� ���ν����� call�ϱ����� SetTargetEntity()�� ����Ǿ�� �Ѵ�.
	{
		m_vDesiredPosition = PlayerDestinationPos();
		
		CSkill &skill = _pNetwork->GetSkillData(m_nCurrentSkillNum);

		m_SkillEffectInfo.InitForSkillAttack(skill);
		autocall SkillAndMagicAnimation() EReturn;
	
		return EReturn();
	};

	IdleAnimation(EVoid) : CEnemyBase::IdleAnimation
	{
		autowait(15.0f + FRnd()*5.0f);
		GetModelInstance()->AddAnimation(idMob_Default2,AN_CLEAR,1.0f,0);
		//m_fIdleTime = _pTimer->CurrentTick();

		FLOAT animtime = GetAnimLength(idMob_Default2);
		autowait(animtime);
		StandingAnim();
		return EReturn();
		
	}

	AttackAnimation(EVoid) //0628
	{
		const int iAttackMotion = m_nAttackCnt % MOB_ATTACK_MOTION_NUM;
		if( iAttackMotion == 0 )
		{
			GetModelInstance()->AddAnimation(idMob_Attack, AN_CLEAR, 1.0f, 0);	
		}
		else if( iAttackMotion == 1 )
		{
			GetModelInstance()->AddAnimation(idMob_Attack2, AN_CLEAR, 1.0f, 0);	
		}
		else if( iAttackMotion == 2 )
		{
			GetModelInstance()->AddAnimation(idMob_Attack3, AN_CLEAR, 1.0f, 0);	
		}
		else if( iAttackMotion == 3 )
		{
			GetModelInstance()->AddAnimation(idMob_Attack4, AN_CLEAR, 1.0f, 0);	
		}

		m_fAttackFrequency = 0.25f;
		m_fAttackStartTime = _pTimer->CurrentTick();
		m_fImpactTime = GetAnimLength(idMob_Attack)/3;//0630 �켱 ���� �ִϸ��̼��� ���ݿ��� ����Ʈ.

		while(_pTimer->CurrentTick() - m_fAttackStartTime < GetAnimLength(idMob_Attack)*0.8f){

			wait(m_fAttackFrequency) 
			{
				on (EBegin) : 
				{					
					if(m_bMoving)
					{						
					//	m_bStop = FALSE;		
					//	m_bAttack = FALSE;
					//	SetNoTargetEntity();	
						return EReturn();
					}

					m_fMoveSpeed = 0.0f; 
					ULONG ulFlags = SetDesiredMovement(); 				

					if(_pTimer->CurrentTick() - m_fAttackStartTime >m_fImpactTime) 
					{					
						SendEvent(EAttackDamage());
						m_fImpactTime = 1000.0f;//����� ��ð�.
					}

					resume; 
				}
				on (EEnemyBaseDamage eEBDamage) : 
				{
					// if confused
					m_fDamageConfused -= eEBDamage.fAmount;
					if (m_fDamageConfused < 0.001f) 
					{
						m_fDamageConfused = m_fDamageWounded;
						// play wounding animation
						INDEX animSet, animIdx;
						static INDEX animId = ska_StringToID("mldam");//HardCoding
						if(this->en_pmiModelInstance
						&& this->en_pmiModelInstance->FindAnimationByID(animId, &animSet, &animIdx))
						{
							//Ÿ�� ���
							this->en_pmiModelInstance->AddAnimation(animId, AN_REMOVE_ON_END, 1.0f, 0);
						}
					}
					resume;
				}
				on (EAttackDamage eAttackDamage) : 
				{
						CEntity *pen = m_penEnemy;
						FLOAT3D vCurrentCenter(((EntityInfo*)(this->GetEntityInfo()))->vTargetCenter[0],
							((EntityInfo*)(this->GetEntityInfo()))->vTargetCenter[1],
							((EntityInfo*)(this->GetEntityInfo()))->vTargetCenter[2] );
						FLOAT3D vCurrentPos = this->en_plPlacement.pl_PositionVector + vCurrentCenter;
						FLOAT3D vTargetCenter(0, 0, 0);
						FLOAT3D vTargetPos(0, 0, 0);
						FLOAT3D vDirection(0, 0, 0);
						FLOAT size = 0;
						if(pen != NULL && pen->GetFlags()&ENF_ALIVE)
						{
							if(pen->en_RenderType == RT_SKAMODEL)
							{
								// Bounding Box�� �̿��Ͽ� Effect�� ���� ��ġ�� ã�´�.
								FLOATaabbox3D bbox;
								pen->GetModelInstance()->GetCurrentColisionBox(bbox);
								FLOAT3D vRadius = (bbox.maxvect - bbox.minvect) * 0.5f * 0.4f/*�ܼ��� ������ �ϸ� �ʹ� ŭ. ������ ���� ���*/;
								size = vRadius.Length();
							}
							vTargetCenter = FLOAT3D(((EntityInfo*)(pen->GetEntityInfo()))->vTargetCenter[0],
								((EntityInfo*)(pen->GetEntityInfo()))->vTargetCenter[1],
								((EntityInfo*)(pen->GetEntityInfo()))->vTargetCenter[2] );
							vTargetPos = pen->en_plPlacement.pl_PositionVector + vTargetCenter;
							vDirection = vTargetPos - vCurrentPos;
							vDirection.Normalize();
							vTargetPos -= vDirection * size;
						}
						vDirection.Normalize();

//������ ���� ���� �̱� ���� �۾�	07.25
					// Enemy�� ReceiveDamage�� �����մϴ�.
					// ReceiveDamage���� Health�� 0���� ������ ��ƼƼ�� Destroy�ϴ� �κ��� ����.
					this->InflictDirectDamage(m_penEnemy, this , DMT_CLOSERANGE, (int)1, vTargetPos, vDirection);
					// FIXME:
					//((CLiveEntity*)((CEntity*)m_penEnemy))->SetHealth(1000.0f);
//������ ���� �� �̱� ���� �۾�		07.25
						m_fImpactTime = 1000.0f;//����� ��ð�.

						stop;
				}
				on (ETimer) : { stop; }			
			}
		}

		if(m_penKillEnemy!=NULL)
		{			
			if(((CUnit*)((CEntity*)m_penKillEnemy))->m_nCurrentHealth <= 0)
			{
				((CUnit*)((CEntity*)m_penKillEnemy))->DeathNow();
			}		
			m_penKillEnemy = NULL;
		}	

		m_bAttack = FALSE;
		m_bMoving = FALSE;
		m_bStop = FALSE;		
		StandingAnim();
		SetNoTargetEntity();	

		return EReturn();
	};

	SkillAndMagicAnimation(EVoid)
	{
		if(m_SkillEffectInfo.dwValidValue != 0)
		{
			m_SkillEffectInfo.dwValidValue = 0xBAD0BEEF;
			return EReturn();
		}

		m_fAttackFrequency = 0.05f;
		m_fAttackStartTime = _pTimer->GetLerpedCurrentTick();
		m_fImpactTime = GetAnimLength(idMob_Attack)/3;

		GetModelInstance()->AddAnimation(m_SkillEffectInfo.iAnimatioID,AN_CLEAR,1.0f,0);

		m_nEffectStep = 1;

		if (m_pSkillReadyEffect != NULL)
		{
			DestroyEffectGroup(m_pSkillReadyEffect);
		}
		
		m_pSkillReadyEffect = StartEffectGroup(m_SkillEffectInfo.szEffectNameCast
			, &en_pmiModelInstance->m_tmSkaTagManager
			, _pTimer->GetLerpedCurrentTick());

		while(_pTimer->GetLerpedCurrentTick() - m_fAttackStartTime < GetAnimLength(m_SkillEffectInfo.iAnimatioID)*0.8f)
		{
			wait(m_fAttackFrequency) 
			{
				on (EBegin) : 
				{					
					if(m_bMoving || m_bStop)
					{
						DestroyEffectGroupIfValid(m_pSkillReadyEffect);
						GetModelInstance()->AddAnimation(idMob_Default,AN_LOOPING|AN_NORESTART|AN_CLEAR,1,0);							
						m_SkillEffectInfo.dwValidValue = 0xBAD0BEEF;
						return EReturn();
					}
											
					m_fMoveSpeed = 0.0f;
					if(m_penEnemy.ep_pen != this)
					{
						SetDesiredMovement();
					}

					FLOAT time = _pTimer->GetLerpedCurrentTick() - m_fAttackStartTime;
					if(time >= m_SkillEffectInfo.fFireDelay[0] && m_nEffectStep == 1 && m_SkillEffectInfo.iFireDelayCount > 0)
					{
						++m_nEffectStep;
						SkillAndAttackFire(1);
					}
					if(time >= m_SkillEffectInfo.fFireDelay[1] && m_nEffectStep == 2 && m_SkillEffectInfo.iFireDelayCount > 1)
					{
						++m_nEffectStep;
						SkillAndAttackFire(2);
					}
					if(time >= m_SkillEffectInfo.fFireDelay[2] && m_nEffectStep == 3 && m_SkillEffectInfo.iFireDelayCount > 2)
					{
						++m_nEffectStep;
						SkillAndAttackFire(3);
					}
					if(time >= m_SkillEffectInfo.fFireDelay[3] && m_nEffectStep == 4 && m_SkillEffectInfo.iFireDelayCount > 3)
					{
						++m_nEffectStep;
						SkillAndAttackFire(4);
					}
					resume;
				}
				on (EEnemyBaseDamage eEBDamage) : 
				{
					// if confused
					m_fDamageConfused -= eEBDamage.fAmount;
					if (m_fDamageConfused < 0.001f) 
					{
						m_fDamageConfused = m_fDamageWounded;
						// play wounding animation
						INDEX animSet, animIdx;
						static INDEX animId = ska_StringToID("mldam");//HardCoding
						if(this->en_pmiModelInstance
						&& this->en_pmiModelInstance->FindAnimationByID(animId, &animSet, &animIdx))
						{
							//Ÿ�� ���
							this->en_pmiModelInstance->AddAnimation(animId, AN_REMOVE_ON_END, 1.0f, 0);
						}
					}
					resume;
				}
				on (ETimer) : { stop; }
			}
		}

		m_nEffectStep = 0;
		m_SkillEffectInfo.dwValidValue = 0xBAD0BEEF;

		if( !m_bUseAI )
		{
			m_bAttack = FALSE;
			m_bMoving = FALSE;
			m_bStop = TRUE;		
			//StandingAnim();
			SetNoTargetEntity();	
		}

		return EReturn();
	};

	// Normal attack
	NormalAttack(EVoid)
	{
		StandingAnim();
		autowait(0.8f + FRnd()*0.25f);

		INDEX attackAnimID = -1;
		const int iAttackMotion = rand() % MOB_ATTACK_MOTION_NUM;
		if( iAttackMotion == 0 )
		{
			attackAnimID = idMob_Attack;
		}
		else if( iAttackMotion == 1 )
		{
			attackAnimID = idMob_Attack2;
		}
		else if( iAttackMotion == 2 )
		{
			attackAnimID = idMob_Attack3;
		}
		else if( iAttackMotion == 3 )
		{
			attackAnimID = idMob_Attack4;
		}

		m_nAttackCnt = attackAnimID;

		if(m_bUseAI)
		{
// EDIT : BS : BEGIN
//			CPlacement3D pl		= GetLerpedPlacement();
//			_pNetwork->AddMoveList( 
//				GetNetworkID(),
//				pl.pl_PositionVector(1), 
//				pl.pl_PositionVector(3), 
//				pl.pl_PositionVector(2),
//				pl.pl_OrientationAngle(1) );
			_pNetwork->AddMoveList(*this);
			_pNetwork->SendMoveList();
// EDIT : BS : END
			INDEX iAttackerIndex	= GetNetworkID();
			_pNetwork->AddAttackList( 0, iAttackerIndex, m_penEnemy->GetNetworkType(), m_penEnemy->GetNetworkID() );			
		}

		autocall AttackTarget() EEnd;

		//autowait(0.60f);

		return EEnd();
	};
// Character.es���� ������ ���
	Polymoph(EVoid): CCharacterBase::Polymoph //����.
	{
		m_fActionAnimTime = _pTimer->CurrentTick();
		
		m_vDesiredPosition = GetPlacement().pl_PositionVector;
		StopMoving(); 

		while(_pTimer->CurrentTick() - m_fActionAnimTime < 1.8f)//���� �ð��� ���� 1.8��
		{

			wait(0.1f) {
				on (EBegin) : {
					if(!m_bChanging)
					{
						return EReturn();
					}
					MinMaximize();
					resume; 
				}

				on (ETimer) : { stop; }	
			}
		}

		return EReturn();
	};
};