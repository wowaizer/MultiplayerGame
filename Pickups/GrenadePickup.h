#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "GrenadePickup.generated.h"

/**
 * 
 */
UCLASS()
class BLUSTER_API AGrenadePickup : public APickup
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
};
