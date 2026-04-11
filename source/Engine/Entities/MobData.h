#ifndef INCL_MOBDATA_H
#define INCL_MOBDATA_H
#ifdef PRAGMA_ONCE
#pragma once
#endif

#include <Engine/LocalDefine.h>
#include <Engine/Entities/Entity.h>
#include <Engine/Network/ChatMsgBuffer.h>
#include <Engine/Help/LoadLod.h>
#include <Common/header/def_npc.h>

#define		NPC_SHOPPER				(1 << 0)	// 상점주인여부?
#define		NPC_FIRSTATTACK			(1 << 1)	// 선공여부?
#define		NPC_ATTACK				(1 << 2)	// 반격가능?
#define		NPC_MOVING				(1 << 3)	// 이동가능?
#define		NPC_PEACEFUL			(1 << 4)	// 평화적인 몹(원클릭으로 공격 안 시킬때)
#define		NPC_ZONEMOVER			(1 << 5)	// 존 이동시켜주는 NPC
#define		NPC_CASTLE_GUARD		(1 << 6)	// 수호병
#define		NPC_REFINER				(1 << 7)	// 연금술사 NPC
#define		NPC_QUEST				(1 << 8)	// Quest NPC
#define		NPC_CASTLE_TOWER		(1 << 9)	// 공격혁 수호상
#define		NPC_MINERAL				(1 << 10)	// 미네랄
#define		NPC_CROPS				(1 << 11)	// 농작물
#define		NPC_ENERGY				(1 << 12)	// 에너지
#define		NPC_ETERNAL				(1 << 13)	// 지속
#define		NPC_LORD_SYMBOL			(1 << 14)	// 성주의 증명
#define		NPC_REMISSION			(1 << 15)	// 면죄부
#define		NPC_EVENT				(1 << 16)	// 이벤트
#define		NPC_GUARD				(1 << 17)	// 경비병
#define		NPC_KEEPER				(1 << 18)	// 창고
#define		NPC_GUILD				(1 << 19)	// 길드
#define		NPC_MBOSS				(1 << 20)	// 준보스
#define		NPC_BOSS				(1 << 21)	// 보스
#define		NPC_RESETSTAT			(1 << 22)	// 스탯 초기화
#define		NPC_CHANGEWEAPON		(1 << 23)	// 무기 교체
#define		NPC_WARCASTLE			(1 << 24)	// 공성 NPC
#define		NPC_DISPLAY_MAP			(1 << 25)	// 지도에 표시되는지 여부
#define		NPC_QUEST_COLLECT		(1 << 26)	// 수집 퀘스트용
#define		NPC_PARTY_MOB			(1 << 27)	// 파티몹 표시
#define		NPC_RAID				(1 << 28)	// 레이드 몹
#define		NPC_SUBCITY				(1 << 29)	// 서브 마을 NPC			// WSS_ADD_FLAG 2007/08/09 카오헌터 마을 NPC
#define		NPC_CHAOCITY			(1 << 30)	// 카오마을 전용 리젠 NPC
#define		NPC_HUNTERCITY			(1 << 31)	// 헌터마을 전용 리젠 NPC
// FLAG Extension	// 2009년 2월
#define		NPC_AUCTION				(1 << 0)	// 거래 대행 NPC
#define		NPC_COLLSION_ENABLE		(1 << 1)	// 캐릭터의 모델에 충돌 flag적용 유무
#define		NPC_FACTORY				(1 << 2)	// 아이템 제작 관련 NPC
#define		NPC_TRIGGER_CHOICE		(1 << 3)	// NPC와 대화하면 트리거 발동
#define		NPC_DONT_ATTACK				(1 << 6)	// Totem NPC
#define		NPC_AFFINITY			(1 << 7)	// 친화도 NPC
#define		NPC_SHADOW				(1 << 8)	// 그림자 몬스터 & NPC [2/11/2011 rumist]
#define		NPC_CRAFTING			(1 << 9)	// 제작
#define		NPC_TOTEM_ITEM			(1 << 10)	// 아이템으로 소환된 토템 NPC

#define		NPC_NO_SHADOW_CAST		(1 << 31)	// 그림자 캐스팅 제외함.
#define		NPC_MASK				( NPC_ZONEMOVER | NPC_SHOPPER | NPC_REFINER | NPC_QUEST | NPC_REMISSION | NPC_EVENT | NPC_KEEPER | NPC_GUILD | NPC_RESETSTAT | NPC_CHANGEWEAPON | NPC_WARCASTLE | NPC_GUARD)
#define		NPC_RESOURCE_MASK		( NPC_MINERAL | NPC_CROPS | NPC_ETERNAL | NPC_ENERGY | NPC_QUEST_COLLECT)

enum eNpcAnimation
{
	NPC_ANIM_START = 0, 
	NPC_ANIM_WALK_1,
	NPC_ANIM_ATTACK_1,
	NPC_ANIM_ATTACK_2,
	NPC_ANIM_ATTACK_3,
	NPC_ANIM_ATTACK_4,
	NPC_ANIM_IDLE_1,	
	NPC_ANIM_IDLE_2,	
	NPC_ANIM_DIE,
	NPC_ANIM_DAMAGE,	
	NPC_ANIM_RUN_1,		
	NPC_ANIM_END = NPC_ANIM_RUN_1,
	NPC_ANIM_TOTAL,
};
// WSS_DRATAN_SEIGEWARFARE 2007/08/13 --------------->>
enum eSiegewarfareQuarters
{
	DRATAN_SIEGEWARFARE_QUARTER1 = 390,
	DRATAN_SIEGEWARFARE_QUARTER2,
	DRATAN_SIEGEWARFARE_QUARTER3,
	DRATAN_SIEGEWARFARE_QUARTER4,
	DRATAN_SIEGEWARFARE_QUARTER5,
	DRATAN_SIEGEWARFARE_QUARTER6,
	DRATAN_SIEGEWARFARE_QUARTER7,
	DRATAN_SIEGEWARFARE_QUARTER8,
	DRATAN_SIEGEWARFARE_QUARTER9,
	DRATAN_SIEGEWARFARE_QUARTER10,	
};

enum eSiegewarfareQuartersSetup
{
	QUARTER_NONE,
	QUARTER_MINE,
	QUARTER_INSTALL,	
	QUARTER_ATTACK,
};
// ---------------------------------------------------<<
class ENGINE_API CMobData : public stNpc, public LodLoader<CMobData>
{	
private:
	UBYTE	attrdef;
	UBYTE	attratt;
	UBYTE	attrdefLv;
	UBYTE	attrattLv;		

	std::string	name;
	std::string desc;
	int		transFlag;	//[sora] 미번역 스트링 index 표시
private:
	ZONEFLAGS	mob_ulZoneFlag;
	ZONEFLAGS	mob_ulExtraFlag;

	char		motion_Attack3[DEF_ANI_LENGTH];
	char		motion_Attack4[DEF_ANI_LENGTH];
	BOOL		mob_bBoss;
		
	//---------------------------------------------
	// 판매 대행 NPC
	INDEX		Owner_Index;
	short		channel_flag;
public:
	/* Default constructor. */
	CMobData(void);
	/* Destructor. */
	~CMobData(void);

	inline BOOL IsBoss()	const
	{	return mob_bBoss;		}

	BOOL IsNPC() const ;	// WSS_DRATAN_SEIGEWARFARE 2007/08/08 수정함...

	int  IsMyQuarter() const ;	// WSS_DRATAN_SEIGEWARFARE 2007/08/13

	inline BOOL IsResource()	const
	{	return (flag & NPC_RESOURCE_MASK);		}

	inline BOOL	IsShopper()		const
	{ 
		return (flag & NPC_SHOPPER);	
	}	

	inline BOOL IsMovable()		const
	{
		return (flag & NPC_MOVING);
	}

	inline BOOL IsZoneMoving()	const
	{ 
		return (flag & NPC_ZONEMOVER);
	}

	inline BOOL IsCastleGuard() const
	{
		return (flag & NPC_CASTLE_GUARD);
	}

	inline BOOL IsLordSymbol() const
	{
		return (flag & NPC_LORD_SYMBOL);
	}

	inline BOOL IsCastleTower() const
	{
		return (flag & NPC_CASTLE_TOWER);
	}
		
	inline BOOL IsRefiner()		const
	{ 
		return (flag & NPC_REFINER); 
	}

	inline BOOL IsPeaceful()	const
	{
		return (flag & NPC_PEACEFUL); 
	}	

	inline BOOL IsQuest()		const			// 퀘스트 NPC
	{
		return (flag & NPC_QUEST);
	}
	
	inline BOOL IsSkillMaster() const			// 스킬 마스터
	{
		return (skillmaster != -1);
	}

	inline BOOL IsMineral()		const			// 광물
	{
		return (flag & NPC_MINERAL);
	}
	
	inline BOOL IsCrops()		const			// 작물
	{
		return (flag & NPC_CROPS);
	}

	inline BOOL IsEnergy()		const			// 에너지
	{
		return (flag & NPC_ENERGY);
	}

	inline BOOL IsCollect()		const			// 수집물
	{
		return (flag & NPC_QUEST_COLLECT);
	}

	inline BOOL IsEternal()		const			// 지속 자원
	{
		return (flag & NPC_ETERNAL);
	}

	inline BOOL IsSSkillMaster()	const		//특수 스킬 마스터.
	{
		return (sskillmaster != -1);
	}

	inline BOOL IsRemission()		const		// 면죄부
	{
		return (flag & NPC_REMISSION);
	}
	
	inline char GetSkillMaster()	const
	{
		return skillmaster;
	}

	inline char GetSpecialSkillMaster() const
	{
		return sskillmaster;
	}

	inline BOOL IsEvent()			const		// 이벤트
	{
		return (flag & NPC_EVENT);
	}

	inline BOOL IsGuard() const
	{
		return ( flag & NPC_GUARD );
	}

	inline BOOL IsWareHouse() const				// 창고
	{
		return ( flag & NPC_KEEPER );
	}

	inline BOOL IsGuild() const					// 길드
	{
		return ( flag & NPC_GUILD );
	}

	inline BOOL IsChangeWeapon() const			// 무기 교환
	{
		return ( flag & NPC_CHANGEWEAPON );
	}

	inline BOOL IsCollectQuest() const			// 수집 퀘스트 NPC
	{
		return ( flag & NPC_QUEST_COLLECT );
	}

	inline BOOL IsPartyMob() const			// 파티몹 
	{
		return ( flag & NPC_PARTY_MOB );
	}

	inline BOOL IsWarCastle() const
	{
		return ( flag & NPC_WARCASTLE );
	}

	inline BOOL IsResetStat() const				// 스탯 초기화
	{
		return ( flag & NPC_RESETSTAT );
	}

	inline BOOL IsChaoVill() const				// 카오 마을  // WSS_REMOVE_NAMEPOPUP 2007/08/14
	{
		return ( flag & NPC_CHAOCITY );
	}

	inline BOOL IsHunterVill() const				// 헌터 마을 // WSS_REMOVE_NAMEPOPUP 2007/08/14
	{
		return ( flag & NPC_HUNTERCITY );
	}

	inline BOOL IsAuctionChar() const				// 거래 대행NPC
	{
		return ( flag1 & NPC_AUCTION );
	}

	inline BOOL IsCollsionModel()					// 모델에 충돌 적용될 NPC
	{
		return ( flag1 & NPC_COLLSION_ENABLE);
	}

	inline BOOL IsMakeItemNPC()						// 아이템 제작 관련 NPC
	{
		return ( flag1 & NPC_FACTORY);
	}

	inline BOOL	IsAffinityNPC()						// 친화도 NPC
	{
		return ( flag1 & NPC_AFFINITY );
	}

	inline BOOL	IsDontAttack()					// Totem MOB
	{
		return ( flag1 & NPC_DONT_ATTACK );
	}
	
	inline BOOL	IsShadowNPC() const					// shadow type npc.
	{
		return ( flag1 & NPC_SHADOW );
	}

	inline BOOL IsCrafting() const
	{
		return ( flag1 & NPC_CRAFTING );
	}
	
	inline BOOL IsTotemItem() const
	{
		return ( flag1 & NPC_TOTEM_ITEM );
	}

	inline BOOL IsNPCFlag2nd(ULONG flag) const
	{
		return (flag1 & flag);
	}

	CTString	GetAnimAttackNameByID(int iAttackID);	
	inline void SetZoneMovingFlag(ZONEFLAGS ulZoneFlag)
	{	mob_ulZoneFlag = ulZoneFlag; };
	
	inline void SetExtraFlag(ZONEFLAGS ulExtraFlag)
	{	mob_ulExtraFlag = ulExtraFlag;	};

	inline int GetMobIndex()			const
	{	return index;}
	
	inline const char* GetMobSmcFileName()
	{	return fileSMC;}

	inline void SetMobSmcFileName(CTString MobName)
	{
		ZeroMemory(fileSMC, strlen(fileSMC));
		strcpy(fileSMC, MobName.str_String);
	}
	
	inline int GetLevel()				const
	{	return level;}
	
	inline float GetWalkSpeed()			const
	{	return walkSpeed;}

	inline float GetRunSpeed()			const
	{	return runSpeed;}

	inline float GetAttackDistance()	const
	{	return attackArea;	}
	
	inline float GetHealth()			const
	{	return hp;}
	
	inline int	GetMP()					const
	{	return mp;}

	inline int	GetAttackSpeed()		const
	{	return attackSpeed;	}

	inline float GetScale()				const
	{	return scale;}

	inline float GetSize()				const
	{	return size;}

	inline float GetScaledSize()		const
	{	return fBound * scale;}

	inline int	GetType()				const
	{	return flag;	}
	inline ZONEFLAGS GetZoneMovingFlag()const	
	{	return mob_ulZoneFlag; }

	inline ZONEFLAGS GetExtraFlag()		const
	{	return mob_ulExtraFlag;	};
	
	// NOTE : 월드 에디터에서만 쓰여야 합니다.
	void SetName(const char* str)	{ name = str; }
	inline const char*	GetName()
	{	return name.c_str();}
	
	inline const char*	GetAnimWalkName()
	{	return motion_Walk;}
	
	inline const char* GetAnimRunName()
	{	return motion_Run;	}
	
	inline const char*	GetAnimAttackName()
	{	return motion_Attack;}
	
	inline const char*	GetAnimDefaultName()
	{	return motion_Idle;}
	
	inline const char*	GetAnimDeathName()
	{	return motion_Die;}
	
	inline const char*	GetAnimWoundName()
	{	return motion_Dam;}
	
	inline const char* GetNormalBoxName()
	{	return ("Default");}	

	inline const char* GetAnimAttack2Name()
	{	return motion_Attack2;}

	inline const char*	GetAnimDefault2Name()
	{	return motion_Idle2;}

	inline const char* GetAnimAttack3Name()
	{	return motion_Attack3;}

	inline const char* GetAnimAttack4Name()
	{	return motion_Attack4;}

	inline char GetAttackType() const
	{	return attackType;	}	

	inline char GetDelayCount()		const
	{	return fireDelayCount;	}

	inline float GetDelay(int i)		const
	{	return fireDelay[i];	}

	inline const char* GetFireEffect0()
	{	return fireEffect0;	}

	inline const char* GetFireEffect1()
	{	return fireEffect1;	}

	inline const char* GetFireEffect2()
	{	return fireEffect2;	}

	inline int GetMissileType()	const
	{	return (int)fireObject;	}	

	inline float GetMissileSpeed()	const
	{	return fireSpeed;	}

	inline int GetSkill0Index()	const
	{	return skill0_index;	}

	inline char	GetSkill0Level()	const
	{	return skill0_level;	}

	inline int GetSkill1Index()	const
	{	return skill1_index;	}

	inline char	GetSkill1Level()	const
	{	return skill1_level;	}

	inline void	SetOwnerIndex(INDEX Ownerindex)
	{
		Owner_Index = Ownerindex;
	}

	inline INDEX GetOwnerIndex()
	{
		return Owner_Index;
	}

	inline UBYTE GetAttratt() const
	{
		return attratt;
	}
	
	inline UBYTE GetAttrdef() const
	{
		return attrdef;
	}

	inline UBYTE GetAttrattLv() const
	{
		return attrattLv;
	}
	
	inline UBYTE GetAttrdefLv() const
	{
		return attrdefLv;
	}
	
	inline void	SetAttribute(UBYTE ubAttratt, UBYTE ubAttrdef, UBYTE ubAttrattLv, UBYTE ubAttrdefLv)
	{
		attratt = ubAttratt;
		attrdef = ubAttrdef;
		attrattLv = ubAttrattLv;
		attrdefLv = ubAttrdefLv;
	}

	int	GetSyndicateType()
	{
		return rvr_value;
	}

	int GetSyndicateGrade()
	{
		return rvr_grade;
	}

	float GetBound()
	{
		return fBound;
	}

	void SetChannelFlag(short channelFlag)	{ channel_flag = channelFlag;	}
	bool IsChannelFlag(int nChannel)	
	{ 
		if( channel_flag & ( 1 << (nChannel - 1) )) 
			return true;
		return false; 
	}

public:
	void SetNoTranslate();
	void ClearNoTranslate();

	int& GetTranslateFlag()		{ return transFlag; }
	void SetTranslateFlag(int flag);

	inline const char* GetDesc()
	{
		return desc.c_str();
	}
	
	void SetDesc(const char* strDesc) { desc = strDesc; }

	//-----------------------------------------------------------------------------
	// Purpose: NPC 데이터를 읽어들여서 MobData 배열을 생성합니다.
	// Input  : &apMobData - 데이터가 저장될 배열.
	//			FileName - 읽어들일 화일명 MOB월드번호.lod
	// Output : int - 화일로부터 읽어들인 NPC 데이터의 갯수를 반환합니다.
	//-----------------------------------------------------------------------------
#if		(_MSC_VER <= 1200)
	
	static CMobData* getData(int idx) {
		
		std::map<int, CMobData*>::iterator it = _mapdata.find(idx);
		
		if (it != _mapdata.end())
			return (*it).second;
		
		return m_dummy;
	}
	
	static _map* getmapPtr() { return &_mapdata; } 

	static int getsize()	{ return (int)_mapdata.size(); }

	static CMobData* getDataSeq(int Idx)
	{
		if (Idx < 0 || Idx >= _vecdata.size())
			return m_dummy;
		
		return _vecdata[Idx];
	}

#endif	// (_MSC_VER <= 1200)

	static bool loadEx(const char* FileName);
	static bool LoadZoneData(const char* ZNFileName);
	static void SetSDNpc(); // 싱글던전 NPC 셋팅
	static bool LoadChannelData(const char* FileName);

	static int SetMobDataToNPC(CEntity* penEntity, CMobData* MD, const char* szMobName);	
};

#endif