
#include "JumpPickup.h"
#include "Bluster/Character/BlasterCharacter.h"
#include "Bluster/BlasterComponents/BuffComponent.h"

void AJumpPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent,OtherActor,OtherComponent,OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter *BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter)
	{
		UBuffComponent *Buff = BlasterCharacter->GetBuff();
		if(Buff)
		{
			Buff->BuffJump(JumpZVelocityBuff,JumpBuffTime);
			
		}
	}

	Destroy();
}