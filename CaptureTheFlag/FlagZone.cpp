#include "FlagZone.h"
#include "Components/SphereComponent.h"
#include "Bluster/Weapon/Flag.h"
#include "Bluster/GameMode/CaptureTheFlagGameMode.h"
#include "Bluster/Character/BlasterCharacter.h"
//-------------------------------------------------------------------------------------------------------------------------
AFlagZone::AFlagZone()
{	PrimaryActorTick.bCanEverTick = false;

	ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphere"));
	SetRootComponent(ZoneSphere);
	

}
//-------------------------------------------------------------------------------------------------------------------------
void AFlagZone::BeginPlay()
{
	Super::BeginPlay();
	ZoneSphere->OnComponentBeginOverlap.AddDynamic(this,&AFlagZone::OnSphereOverlap);

}
//-------------------------------------------------------------------------------------------------------------------------
void AFlagZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFlag *OverlappingFlag = Cast<AFlag>(OtherActor);
	if(OverlappingFlag && OverlappingFlag->GetTeam() != Team)
	{
		ACaptureTheFlagGameMode *GameMode =  GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>();
		if(GameMode)
		{
			GameMode->FlagCaptured(OverlappingFlag,this);
			OverlappingFlag->ResetFlag();
		}
		OverlappingFlag->ResetFlag();
	}

}
//-------------------------------------------------------------------------------------------------------------------------
void AFlagZone::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{

}
//-------------------------------------------------------------------------------------------------------------------------

