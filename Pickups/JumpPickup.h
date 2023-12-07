#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "JumpPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLUSTER_API AJumpPickup : public APickup
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
	float JumpZVelocityBuff = 4000.f;

	UPROPERTY(EditAnywhere)
	float JumpBuffTime = 30.f;
};
