// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "Bluster/Character/BlasterCharacter.h"
#include "Bluster/BlasterComponents/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent,OtherActor,OtherComponent,OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter *BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter)
	{
		UCombatComponent *CombatComponent = BlasterCharacter->GetCombat();
		if(CombatComponent)
		{
			CombatComponent->PickupAmmo(WeaponType,AmmoAmmount);
		}
		
	}

	Destroy();
}
