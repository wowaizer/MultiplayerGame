#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "ShieldPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLUSTER_API AShieldPickup : public APickup
{
	GENERATED_BODY()
	

protected:
	virtual void OnSphereOverlap (
		UPrimitiveComponent *OverlappedComponent,
		AActor *OtherActor,
		UPrimitiveComponent *OtherComponent, 
		int32 OtherBodyIndex, 
		bool bFromSweep,
		const FHitResult &SweepResult
	);

private:

	UPROPERTY(EditAnywhere)
	float ShieldReplenishAmount = 100.f;

	UPROPERTY(EditAnywhere)
	float ShieldReplenishTime = 5.f;

};
