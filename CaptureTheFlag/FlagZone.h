#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bluster/BlasterTypes/Team.h"
#include "FlagZone.generated.h"

UCLASS()
class BLUSTER_API AFlagZone : public AActor
{
	GENERATED_BODY()
	
public:	
	AFlagZone();

	UPROPERTY(EditAnywhere)
	ETeam Team;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
		virtual void OnSphereOverlap (UPrimitiveComponent *OverlappedComponent,AActor *OtherActor,
			UPrimitiveComponent *OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
			const FHitResult &SweepResult);

	UFUNCTION()
		virtual void OnSphereEndOverlap (UPrimitiveComponent *OverlappedComponent,AActor *OtherActor,
			UPrimitiveComponent *OtherComponent, int32 OtherBodyIndex);

private:
	UPROPERTY(EditAnywhere)
	class USphereComponent *ZoneSphere;

};