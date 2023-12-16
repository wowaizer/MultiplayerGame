#include "GrenadePickup.h"
#include "Bluster/Character/BlasterCharacter.h"
#include "Bluster/BlasterComponents/CombatComponent.h"


void AGrenadePickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent,OtherActor,OtherComponent,OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter *BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter)
	{
		
		if(BlasterCharacter->GetCombat())
		{
			if(BlasterCharacter->GetCombat()->GetGrenades() < BlasterCharacter->GetCombat()->GetMaxGrenades())
			{
				BlasterCharacter->GetCombat()->PickUpGrenade();

				Destroy();
			}
			
		}
	}

	
}
