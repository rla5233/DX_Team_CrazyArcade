#include "PreCompile.h"
#include "Player.h"
#include <EngineBase/EngineRandom.h>
#include "MainPlayLevel.h"
#include "MapBase.h"
#include "CrazyArcadeCore.h"

std::map<int, bool> FPlayerInfo::IsDeads;
std::map<int, std::string> FPlayerInfo::Names;

APlayer::APlayer()
{
	DefaultComponent = CreateDefaultSubObject<UDefaultSceneComponent>("DefaultComponent");
	SetRoot(DefaultComponent);

	Renderer = CreateDefaultSubObject<USpriteRenderer>("Renderer");
	Renderer->SetupAttachment(DefaultComponent);

	ShadowRenderer = CreateDefaultSubObject<USpriteRenderer>("ShadowRenderer");
	ShadowRenderer->SetupAttachment(DefaultComponent);

	DebugRenderer = CreateDefaultSubObject<USpriteRenderer>("DebugRenderer");
	DebugRenderer->SetupAttachment(DefaultComponent);

	MPlayerItem.insert(std::pair(EItemType::Bubble, 0));
	MPlayerItem.insert(std::pair(EItemType::Fluid, 0));
	MPlayerItem.insert(std::pair(EItemType::Ultra, 0));
	MPlayerItem.insert(std::pair(EItemType::Roller, 0));
	MPlayerItem.insert(std::pair(EItemType::RedDevil, 0));
	MPlayerItem.insert(std::pair(EItemType::Glove, 0));
	MPlayerItem.insert(std::pair(EItemType::Shoes, 0));

	InputOn();
}

APlayer::~APlayer()
{
}

void APlayer::BeginPlay()
{
	Super::BeginPlay();

	PlayLevel = dynamic_cast<AMainPlayLevel*>(GetWorld()->GetGameMode().get());
	BlockSize = AMapBase::GetBlockSize();

	// 이미지 컷팅
	PlayerCreateCuttingBazzi("_R");
	PlayerCreateCuttingBazzi("_B");

	PlayerCreateCutting("Dao_R");
	PlayerCreateCutting("Dao_B");

	PlayerCreateCutting("luxMarid_R");
	PlayerCreateCutting("luxMarid_O");
	PlayerCreateCutting("luxMarid_B");

	// 애니메이션 생성
	//Bazzi
	PlayerCreateBazziAnimation("_R");
	PlayerCreateBazziAnimation("_B");

	PlayerCreateAnimation("Dao_R");
	PlayerCreateAnimation("Dao_B");

	PlayerCreateAnimation("luxMarid_R");
	PlayerCreateAnimation("luxMarid_O");
	PlayerCreateAnimation("luxMarid_B");

	CharacterTypeDataInit();

	Renderer->ChangeAnimation(Type + PlayerColorText + "_Idle_Down");
	Renderer->SetAutoSize(0.9f, true);
	Renderer->AddPosition({ 0.0f, BlockSize / 2.0f, 0.0f });

	ShadowRenderer->SetSprite("Shadow.png");
	ShadowRenderer->SetAutoSize(1.0f, true);
	ShadowRenderer->SetMulColor({ 1.0f, 1.0f, 1.0f, 0.7f });
	ShadowRenderer->SetOrder(ERenderOrder::Shadow);
	ShadowRenderer->AddPosition({ 0.0f, -BlockSize / 4.0f });

	DebugRenderer->SetScale({ 5,5,10 });
	DebugRenderer->SetOrder(9999);

	StateInit();
}

void APlayer::Tick(float _DeltaTime)
{
	Super::Tick(_DeltaTime);

	State.Update(_DeltaTime);

	int PlayerOrder = AMapBase::GetRenderOrder(GetActorLocation());
	Renderer->SetOrder(PlayerOrder);

	{
		std::string Msg = std::format("PlayerOrder : {}\n", PlayerOrder);
		UEngineDebugMsgWindow::PushMsg(Msg);
	}
	{
		std::string Msg = std::format("Bubble : {}\n", MPlayerItem[EItemType::Bubble]);
		UEngineDebugMsgWindow::PushMsg(Msg);
	}
	{
		std::string Msg = std::format("BombCount : {}\n", BombCount);
		UEngineDebugMsgWindow::PushMsg(Msg);
	}
	{
		std::string Msg = std::format("BombPower : {}\n", BombPower);
		UEngineDebugMsgWindow::PushMsg(Msg);
	}
	{
		std::string Msg = std::format("CurSpeed : {}\n", CurSpeed);
		UEngineDebugMsgWindow::PushMsg(Msg);
	}
	{
		std::string Msg = std::format("NeedleCount : {}\n", NeedleCount);
		UEngineDebugMsgWindow::PushMsg(Msg);
	}
	// 임의 적용 테스트
	if (true == IsDown('C'))
	{
		if (ECharacterColor::Red == PlayerColor)
		{
			SetPlayerColor(ECharacterColor::Blue);
		}
		else if (ECharacterColor::Blue == PlayerColor)
		{
			SetPlayerColor(ECharacterColor::Red);
		}
	}
	if (true == IsDown('B'))
	{
		SetCharacterType(ECharacterType::Bazzi);
	}
	if (true == IsDown('O'))
	{
		SetCharacterType(ECharacterType::Dao);
	}
	if (true == IsDown('M'))
	{
		SetCharacterType(ECharacterType::Marid);
	}

	PlayerPos = GetActorLocation();

	PickUpItem();

	Devil(_DeltaTime);

	Superman(_DeltaTime);
	CheckBombCount();

	PlayerInfoUpdate();
}

void APlayer::PlayerCreateCuttingBazzi(std::string _Color)
{
	UEngineSprite::CreateCutting("Bazzi" + _Color + "_1.png", 5, 18);
	UEngineSprite::CreateCutting("Bazzi" + _Color + "_2.png", 5, 2);
	UEngineSprite::CreateCutting("Bazzi" + _Color + "_3.png", 5, 4);
	UEngineSprite::CreateCutting("Bazzi" + _Color + "_4.png", 5, 7);
}

void APlayer::PlayerCreateCutting(std::string _CharacterType_Color)
{
	UEngineSprite::CreateCutting(_CharacterType_Color + "_1.png", 5, 12);
	UEngineSprite::CreateCutting(_CharacterType_Color + "_2.png", 5, 2);
	UEngineSprite::CreateCutting(_CharacterType_Color + "_3.png", 5, 4);
	UEngineSprite::CreateCutting(_CharacterType_Color + "_4.png", 5, 4);
	UEngineSprite::CreateCutting(_CharacterType_Color + "_5.png", 5, 6);
}

void APlayer::PlayerCreateBazziAnimation(std::string _Color)
{

	Renderer->CreateAnimation("Bazzi" + _Color + "_Ready", "Bazzi" + _Color + "_1.png", 0.06f, false, 36, 53);
	Renderer->CreateAnimation("Bazzi" + _Color + "_Idle_Left", "Bazzi" + _Color + "_1.png", 1.0f, false, 0, 0);
	Renderer->CreateAnimation("Bazzi" + _Color + "_Idle_Right", "Bazzi" + _Color + "_1.png", 1.0f, false, 6, 6);
	Renderer->CreateAnimation("Bazzi" + _Color + "_Idle_Up", "Bazzi" + _Color + "_1.png", 1.0f, false, 12, 12);
	Renderer->CreateAnimation("Bazzi" + _Color + "_Idle_Down", "Bazzi" + _Color + "_1.png", 1.0f, false, 20, 20);

	Renderer->CreateAnimation("Bazzi" + _Color + "_Run_Left", "Bazzi" + _Color + "_1.png", 0.1f, true, 1, 5);
	Renderer->CreateAnimation("Bazzi" + _Color + "_Run_Right", "Bazzi" + _Color + "_1.png", 0.1f, true, 7, 11);
	Renderer->CreateAnimation("Bazzi" + _Color + "_Run_Up", "Bazzi" + _Color + "_1.png", 0.1f, true, 13, 19);
	Renderer->CreateAnimation("Bazzi" + _Color + "_Run_Down", "Bazzi" + _Color + "_1.png", 0.1f, true, 21, 28);

	Renderer->CreateAnimation("Bazzi" + _Color + "_Win", "Bazzi" + _Color + "_1.png", 0.1f, true, 29, 35);
	Renderer->CreateAnimation("Bazzi" + _Color + "_TrapStart", "Bazzi" + _Color + "_4.png", 0.07f, false, 6, 10);
	Renderer->CreateAnimation("Bazzi" + _Color + "_Traped", "Bazzi" + _Color + "_4.png", 0.2f, false, 11, 23);
	Renderer->CreateAnimation("Bazzi" + _Color + "_TrapEnd", "Bazzi" + _Color + "_4.png", 0.25f, false, 24, 31);
	Renderer->CreateAnimation("Bazzi" + _Color + "_Die", "Bazzi" + _Color + "_2.png", 0.15f, false, 0, 5);
	Renderer->CreateAnimation("Bazzi" + _Color + "_Revival", "Bazzi" + _Color + "_2.png", 0.15f, false, 6, 9);

	Renderer->CreateAnimation("Bazzi" + _Color + "_IdleOwl_Left", "Bazzi" + _Color + "_3.png", 0.15f, false, 0, 0);
	Renderer->CreateAnimation("Bazzi" + _Color + "_IdleOwl_Right", "Bazzi" + _Color + "_3.png", 0.15f, false, 2, 2);
	Renderer->CreateAnimation("Bazzi" + _Color + "_IdleOwl_Up", "Bazzi" + _Color + "_3.png", 0.15f, false, 4, 4);
	Renderer->CreateAnimation("Bazzi" + _Color + "_IdleOwl_Down", "Bazzi" + _Color + "_3.png", 0.15f, false, 6, 6);
	Renderer->CreateAnimation("Bazzi" + _Color + "_IdleTurtle_Left", "Bazzi" + _Color + "_3.png", 0.15f, false, 8, 8);
	Renderer->CreateAnimation("Bazzi" + _Color + "_IdleTurtle_Right", "Bazzi" + _Color + "_3.png", 0.15f, false, 10, 10);
	Renderer->CreateAnimation("Bazzi" + _Color + "_IdleTurtle_Down", "Bazzi" + _Color + "_3.png", 0.15f, false, 12, 12);
	Renderer->CreateAnimation("Bazzi" + _Color + "_IdleTurtle_Up", "Bazzi" + _Color + "_3.png", 0.15f, false, 14, 14);
	Renderer->CreateAnimation("Bazzi" + _Color + "_RidingOwl_Left", "Bazzi" + _Color + "_3.png", 0.15f, true, 0, 1);
	Renderer->CreateAnimation("Bazzi" + _Color + "_RidingOwl_Right", "Bazzi" + _Color + "_3.png", 0.15f, true, 2, 3);
	Renderer->CreateAnimation("Bazzi" + _Color + "_RidingOwl_Up", "Bazzi" + _Color + "_3.png", 0.15f, true, 4, 5);
	Renderer->CreateAnimation("Bazzi" + _Color + "_RidingOwl_Down", "Bazzi" + _Color + "_3.png", 0.15f, true, 6, 7);
	Renderer->CreateAnimation("Bazzi" + _Color + "_RidingTurtle_Left", "Bazzi" + _Color + "_3.png", 0.2f, true, 8, 9);
	Renderer->CreateAnimation("Bazzi" + _Color + "_RidingTurtle_Right", "Bazzi" + _Color + "_3.png", 0.2f, true, 10, 11);
	Renderer->CreateAnimation("Bazzi" + _Color + "_RidingTurtle_Up", "Bazzi" + _Color + "_3.png", 0.2f, true, 14, 15);
	Renderer->CreateAnimation("Bazzi" + _Color + "_RidingTurtle_Down", "Bazzi" + _Color + "_3.png", 0.2f, true, 12, 13);
	Renderer->CreateAnimation("Bazzi" + _Color + "_RidingUFO_Left", "Bazzi" + _Color + "_3.png", 0.09f, true, 16, 16);
	Renderer->CreateAnimation("Bazzi" + _Color + "_RidingUFO_Right", "Bazzi" + _Color + "_3.png", 0.09f, true, 17, 17);
	Renderer->CreateAnimation("Bazzi" + _Color + "_RidingUFO_Up", "Bazzi" + _Color + "_3.png", 0.09f, true, 18, 18);
	Renderer->CreateAnimation("Bazzi" + _Color + "_RidingUFO_Down", "Bazzi" + _Color + "_3.png", 0.09f, true, 19, 19);
}

void APlayer::PlayerCreateAnimation(std::string _CharacterType_Color)
{
	// Idle
	Renderer->CreateAnimation(_CharacterType_Color + "_Ready", _CharacterType_Color + "_4.png", 0.06f, false, 0, 17);
	Renderer->CreateAnimation(_CharacterType_Color + "_Idle_Right", _CharacterType_Color + "_1.png", 1.0f, false, 0, 0 );
	Renderer->CreateAnimation(_CharacterType_Color + "_Idle_Left", _CharacterType_Color + "_1.png", 1.f, false, 6, 6);
	Renderer->CreateAnimation(_CharacterType_Color + "_Idle_Up", _CharacterType_Color + "_1.png", 1.f, false, 12, 12);
	Renderer->CreateAnimation(_CharacterType_Color + "_Idle_Down", _CharacterType_Color + "_1.png", 1.f, false, 18, 18);

	// Move
	Renderer->CreateAnimation(_CharacterType_Color + "_Run_Right", _CharacterType_Color + "_1.png", 0.1f, true, 1, 5);
	Renderer->CreateAnimation(_CharacterType_Color + "_Run_Left", _CharacterType_Color + "_1.png", 0.1f, true, 7, 11);
	Renderer->CreateAnimation(_CharacterType_Color + "_Run_Up", _CharacterType_Color + "_1.png", 0.09f, true, 13, 17);
	Renderer->CreateAnimation(_CharacterType_Color + "_Run_Down", _CharacterType_Color + "_1.png", 0.09f, true, 19, 23);

	//Renderer->CreateAnimation(_CharacterType_Color + "_Win", _CharacterType_Color + "_1.png", 0.1f, true, 29, 36);
	Renderer->CreateAnimation(_CharacterType_Color + "_Win", _CharacterType_Color + "_4.png", 0.1f, true, 0, 12);
	Renderer->CreateAnimation(_CharacterType_Color + "_TrapStart", _CharacterType_Color + "_5.png", 0.07f, false, 0, 4); // 0.2   0.25
	Renderer->CreateAnimation(_CharacterType_Color + "_Traped", _CharacterType_Color + "_5.png", 0.2f, true, 5, 18); // 0.2   0.25
	Renderer->CreateAnimation(_CharacterType_Color + "_TrapEnd", _CharacterType_Color + "_5.png", 0.25f, false, 19, 25);
	Renderer->CreateAnimation(_CharacterType_Color + "_Die", _CharacterType_Color + "_2.png", 0.15f, false, 0, 5);
	Renderer->CreateAnimation(_CharacterType_Color + "_Revival", _CharacterType_Color + "_2.png", 0.15f, false, 6, 9);

	Renderer->CreateAnimation(_CharacterType_Color + "_IdleOwl_Left", _CharacterType_Color + "_3.png", 0.15f, false, 0, 0);
	Renderer->CreateAnimation(_CharacterType_Color + "_IdleOwl_Right", _CharacterType_Color + "_3.png", 0.15f, false, 2, 2);
	Renderer->CreateAnimation(_CharacterType_Color + "_IdleOwl_Up", _CharacterType_Color + "_3.png", 0.15f, false, 4, 4);
	Renderer->CreateAnimation(_CharacterType_Color + "_IdleOwl_Down", _CharacterType_Color + "_3.png", 0.15f, false, 6, 6);
	Renderer->CreateAnimation(_CharacterType_Color + "_IdleTurtle_Left", _CharacterType_Color + "_3.png", 0.15f, false, 8, 8);
	Renderer->CreateAnimation(_CharacterType_Color + "_IdleTurtle_Right", _CharacterType_Color + "_3.png", 0.15f, false, 10, 10);
	Renderer->CreateAnimation(_CharacterType_Color + "_IdleTurtle_Down", _CharacterType_Color + "_3.png", 0.15f, false, 12, 12);
	Renderer->CreateAnimation(_CharacterType_Color + "_IdleTurtle_Up", _CharacterType_Color + "_3.png", 0.15f, false, 14, 14);
	Renderer->CreateAnimation(_CharacterType_Color + "_RidingOwl_Left", _CharacterType_Color + "_3.png", 0.15f, true, 0, 1);
	Renderer->CreateAnimation(_CharacterType_Color + "_RidingOwl_Right", _CharacterType_Color + "_3.png", 0.15f, true, 2, 3);
	Renderer->CreateAnimation(_CharacterType_Color + "_RidingOwl_Up", _CharacterType_Color + "_3.png", 0.15f, true, 4, 5);
	Renderer->CreateAnimation(_CharacterType_Color + "_RidingOwl_Down", _CharacterType_Color + "_3.png", 0.15f, true, 6, 7);
	Renderer->CreateAnimation(_CharacterType_Color + "_RidingTurtle_Left", _CharacterType_Color + "_3.png", 0.2f, true, 8, 9);
	Renderer->CreateAnimation(_CharacterType_Color + "_RidingTurtle_Right", _CharacterType_Color + "_3.png", 0.2f, true, 10, 11);
	Renderer->CreateAnimation(_CharacterType_Color + "_RidingTurtle_Up", _CharacterType_Color + "_3.png", 0.2f, true, 14, 15);
	Renderer->CreateAnimation(_CharacterType_Color + "_RidingTurtle_Down", _CharacterType_Color + "_3.png", 0.2f, true, 12, 13);
	Renderer->CreateAnimation(_CharacterType_Color + "_RidingUFO_Left", _CharacterType_Color + "_3.png", 0.09f, true, 16, 16);
	Renderer->CreateAnimation(_CharacterType_Color + "_RidingUFO_Right", _CharacterType_Color + "_3.png", 0.09f, true, 17, 17);
	Renderer->CreateAnimation(_CharacterType_Color + "_RidingUFO_Up", _CharacterType_Color + "_3.png", 0.09f, true, 18, 18);
	Renderer->CreateAnimation(_CharacterType_Color + "_RidingUFO_Down", _CharacterType_Color + "_3.png", 0.09f, true, 19, 19);

	Renderer->CreateAnimation(_CharacterType_Color + "_OnOwl_", _CharacterType_Color + "_3.png", 0.035f, true, 0, 7);
	Renderer->CreateAnimation(_CharacterType_Color + "_OnTurtle_", _CharacterType_Color + "_3.png", 0.035f, true, 8, 15);
	Renderer->CreateAnimation(_CharacterType_Color + "_OnUFO_", _CharacterType_Color + "_3.png", 0.05f, true, 16, 19);
}

void APlayer::CharacterTypeDataInit()
{
	//Bazzi
	MCharacterTypeData[ECharacterType::Bazzi].Type = "Bazzi";
	MCharacterTypeData[ECharacterType::Bazzi].DataBaseBombCount = 1;
	MCharacterTypeData[ECharacterType::Bazzi].DataMaxBombCount = 6;
	MCharacterTypeData[ECharacterType::Bazzi].DataBaseBombPower = 0;
	MCharacterTypeData[ECharacterType::Bazzi].DataMaxBombPower = 6;
	MCharacterTypeData[ECharacterType::Bazzi].DataBaseSpeed = 40.0f * 5.0f;
	MCharacterTypeData[ECharacterType::Bazzi].DataMaxSpeed = 40.0f * 9.0f;

	//Dao
	MCharacterTypeData[ECharacterType::Dao].Type = "Dao";
	MCharacterTypeData[ECharacterType::Dao].DataBaseBombCount = 1;
	MCharacterTypeData[ECharacterType::Dao].DataMaxBombCount = 10;
	MCharacterTypeData[ECharacterType::Dao].DataBaseBombPower = 0;
	MCharacterTypeData[ECharacterType::Dao].DataMaxBombPower = 6;
	MCharacterTypeData[ECharacterType::Dao].DataBaseSpeed = 40.0f * 5.0f;
	MCharacterTypeData[ECharacterType::Dao].DataMaxSpeed = 40.0f * 7.0f;

	//luxMarid
	MCharacterTypeData[ECharacterType::Marid].Type = "luxMarid";
	MCharacterTypeData[ECharacterType::Marid].DataBaseBombCount = 2;
	MCharacterTypeData[ECharacterType::Marid].DataMaxBombCount = 9;
	MCharacterTypeData[ECharacterType::Marid].DataBaseBombPower = 0;
	MCharacterTypeData[ECharacterType::Marid].DataMaxBombPower = 6;
	MCharacterTypeData[ECharacterType::Marid].DataBaseSpeed = 40.0f * 5.0f;
	MCharacterTypeData[ECharacterType::Marid].DataMaxSpeed = 40.0f * 9.0f;
}

void APlayer::PickUpItem()
{
	EItemType ItemType = PlayLevel->GetMap()->IsItemTile(GetActorLocation());

	AddItemCount(ItemType);

	switch (ItemType)
	{
	case EItemType::Bubble:
		if (BombCount < MaxBombCount)
		{
			++BombCount;
		}
		else
		{
			BombCount = MaxBombCount;
		}
		break;
	case EItemType::Devil:
		IsDevil = true;
		MoveDevil = UEngineRandom::MainRandom.RandomInt(0, 1);
		break;
	case EItemType::Fluid:
		if (BombPower < MaxBombPower)
		{
			++BombPower;
		}
		else
		{
			BombPower = MaxBombPower;
		}
		break;
	case EItemType::Ultra:
		BombPower = MaxBombPower;
		break;
	case EItemType::Roller:
		Speed += 40.0f;
		CurSpeed = BaseSpeed + Speed;
		if (MaxSpeed < CurSpeed)
		{
			CurSpeed = MaxSpeed;
		}
		break;
	case EItemType::RedDevil:
		Speed = MaxSpeed - BaseSpeed;
		CurSpeed = MaxSpeed;
		break;
	case EItemType::Glove:
		Throw = true;
		break;
	case EItemType::Shoes:
		Push = true;
		break;
	case EItemType::Superman:
		IsSuperman = true;

		BombCount = MaxBombCount;

		BombPower = MaxBombPower;

		Speed = MaxSpeed - BaseSpeed;
		CurSpeed = MaxSpeed;
		break;
	case EItemType::Owl:
		Riding = ERiding::Owl;
		break;
	case EItemType::Turtle:
		Riding = ERiding::Turtle;
		break;
	case EItemType::UFO:
		Riding = ERiding::UFO;
		break;
	case EItemType::Needle:
		NeedleCount++;
		break;
	default:
		break;
	}
}

void APlayer::AddItemCount(EItemType _ItemType)
{
	int Count = MPlayerItem[_ItemType];
	++Count;
	MPlayerItem[_ItemType] = Count;
}

void APlayer::Devil(float _DeltaTime)
{
	if (true == IsDevil)
	{
		if (0.0f <= RenderChangeTime && RenderChangeTime < 0.5f)
		{
			Renderer->SetMulColor({ 0.7f, 0.0f, 1.0f, 1.0f });
		}
		else if (0.5f <= RenderChangeTime && RenderChangeTime < 1.0f)
		{
			Renderer->SetMulColor(FVector::One);
		}
		else
		{
			//FSpriteInfo SpriteInfo = Renderer->GetCurInfo();
			RenderChangeTime = 0.0f;
		}

		RenderChangeTime += _DeltaTime;

		DevilTime -= _DeltaTime;

		if (0.0f >= DevilTime)
		{
			IsDevil = false;
			Renderer->SetMulColor(FVector::One);
			DevilTime = 10.0f;
		}
	}
}

void APlayer::Superman(float _DeltaTime)
{
	if (true == IsSuperman)
	{
		if (0.0f <= RenderChangeTime && RenderChangeTime < 0.1f)
		{
			Renderer->SetMulColor({ 1.0f, 0.0f, 0.0f, 1.0f });
		}
		else if (0.1f <= RenderChangeTime && RenderChangeTime < 0.2f)
		{
			Renderer->SetMulColor({ 1.0f, 1.0f, 0.0f, 1.0f });
		}
		else if (0.2f <= RenderChangeTime && RenderChangeTime < 0.3f)
		{
			Renderer->SetMulColor({ 0.0f, 0.0f, 1.0f, 1.0f });
		}
		else
		{
			//FSpriteInfo SpriteInfo = Renderer->GetCurInfo();
			RenderChangeTime = 0.0f;
		}

		RenderChangeTime += _DeltaTime;

		SupermanTime -= _DeltaTime;

		if (0.0f >= SupermanTime)
		{
			SetSupermanOff();

			BombCount = BaseBombCount + MPlayerItem[EItemType::Bubble];
			if (BombCount > MaxBombCount)
			{
				BombCount = MaxBombCount;
			}

			if (0 != MPlayerItem[EItemType::Ultra])
			{
				BombPower = BaseBombPower + MPlayerItem[EItemType::Fluid];
				if (BombPower > MaxBombPower)
				{
					BombPower = MaxBombPower;
				}
			}
			else
			{
				BombPower = MaxBombPower;
			}

			if (0 != MPlayerItem[EItemType::RedDevil])
			{
				Speed = 40.0f * static_cast<float>(MPlayerItem[EItemType::Roller]);
				if (Speed > (MaxSpeed - BaseSpeed))
				{
					Speed = MaxSpeed - BaseSpeed;
				}
				CurSpeed = BaseSpeed + Speed;
			}
			else
			{
				Speed = MaxSpeed - BaseSpeed;
				CurSpeed = MaxSpeed;
			}
		}
	}
}

void APlayer::CheckBombCount()
{
	if (true != IsSuperman && BombCount > (MPlayerItem[EItemType::Bubble] + 1))
	{
		BombCount = MPlayerItem[EItemType::Bubble] + 1;
	}
}

void APlayer::SetPlayerDead()
{
	IsDead = true;
	PlayerInfoUpdate();
}

void APlayer::SetCharacterType(ECharacterType _Character)
{
	PlayerType = _Character;
	Type = MCharacterTypeData[_Character].Type;
	BaseBombCount = MCharacterTypeData[_Character].DataBaseBombCount;
	MaxBombCount = MCharacterTypeData[_Character].DataMaxBombCount;
	BaseBombPower = MCharacterTypeData[_Character].DataBaseBombPower;
	MaxBombPower = MCharacterTypeData[_Character].DataMaxBombPower;
	BaseSpeed = MCharacterTypeData[_Character].DataBaseSpeed;
	MaxSpeed = MCharacterTypeData[_Character].DataMaxSpeed;

	BombCount = BaseBombCount;
	BombPower = BaseBombPower;
	CurSpeed = BaseSpeed + Speed;
}

void APlayer::SetPlayerColor(ECharacterColor _Color)
{
	switch (_Color)
	{
	case ECharacterColor::Red:
		PlayerColor = ECharacterColor::Red;
		PlayerColorText = "_R";
		break;
	case ECharacterColor::Yellow:
		PlayerColor = ECharacterColor::Yellow;
		PlayerColorText = "_Y";
		break;
	case ECharacterColor::Green:
		PlayerColor = ECharacterColor::Green;
		PlayerColorText = "_G";
		break;
	case ECharacterColor::Blue:
		PlayerColor = ECharacterColor::Blue;
		PlayerColorText = "_B";
		break;
	default:
		PlayerColorText = "";
		break;
	}
}

void APlayer::PlayerInfoUpdate()
{
	if (nullptr != UCrazyArcadeCore::Net)
	{
		// 0번 세션
		// 스폰을 할 때 0~999
		// 플레이어 액터도 오브젝트 토큰이 0~999 사이고
		// 오브젝트 토큰 / 1000 => 0

		// 1번 세션
		// 스폰을 할 때 1000~1999
		// 플레이어 액터도 오브젝트 토큰이 1000~1999 사이
		// 오브젝트 토큰 / 1000 => 1

		int ObjectToken = GetObjectToken();
		int SessionToken = ObjectToken / 1000;
		FPlayerInfo::IsDeads[SessionToken] = IsDead;
	}
	else
	{
		return;
	}
}
