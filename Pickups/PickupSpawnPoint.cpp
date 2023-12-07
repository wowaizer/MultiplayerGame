#include "PickupSpawnPoint.h"
#include "Pickup.h"
//-------------------------------------------------------------------------------------------------------------------------
APickupSpawnPoint::APickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}
//-------------------------------------------------------------------------------------------------------------------------
void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	StartSpawnPickupTimer((AActor*)nullptr);
	
}
//-------------------------------------------------------------------------------------------------------------------------
void APickupSpawnPoint::SpawnPickup()
{
	int32 NumPickupClasses = PickupClasses.Num();
	if(NumPickupClasses > 0)
	{
		int32 Selection = FMath::FRandRange(0,NumPickupClasses);
		SpawnedPickup = GetWorld()->SpawnActor<APickup>(PickupClasses[Selection],GetActorTransform());

		if(HasAuthority() && SpawnedPickup)
		{
			SpawnedPickup->OnDestroyed.AddDynamic(this,&APickupSpawnPoint::StartSpawnPickupTimer);
		}
	}

}
//-------------------------------------------------------------------------------------------------------------------------
void APickupSpawnPoint::StartSpawnPickupTimer(AActor *DestroyedActor)
{
	const float SpawnTime = FMath::FRandRange(SpawnPickupTimeMin,SpawnPickupTimeMax);
	GetWorldTimerManager().SetTimer(
		SpawnPickupTimer,
		this,
		&APickupSpawnPoint::SpawnPickupTimerFinished,
		SpawnTime
	);
}
//-------------------------------------------------------------------------------------------------------------------------
void APickupSpawnPoint::SpawnPickupTimerFinished()
{
	if(HasAuthority())
	{
		SpawnPickup();
	}

}
//-------------------------------------------------------------------------------------------------------------------------
void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
//-------------------------------------------------------------------------------------------------------------------------
