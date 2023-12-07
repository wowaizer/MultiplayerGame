#include "BuffComponent.h"
#include "Bluster/Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
//-------------------------------------------------------------------------------------------------------------------------
UBuffComponent::UBuffComponent()
{
	
	PrimaryComponentTick.bCanEverTick = true;

	
}
//-------------------------------------------------------------------------------------------------------------------------
void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

}
//-------------------------------------------------------------------------------------------------------------------------

void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if(Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer,
		this,
		&UBuffComponent::ResetSpeeds,
		BuffTime
	);

	if(Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;
	}
	MulticastSpeedBuff(BuffBaseSpeed,BuffCrouchSpeed);	
}
//-------------------------------------------------------------------------------------------------------------------------
void UBuffComponent::ResetSpeeds()
{
	if(Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;

	MulticastSpeedBuff(InitialBaseSpeed,InitialCrouchSpeed);	

}
//-------------------------------------------------------------------------------------------------------------------------
void UBuffComponent::BuffJump(float JumpBuffVelocity, float BuffTime)
{
	if(Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(
		JumpBuffTimer,
		this,
		&UBuffComponent::ResetJump,
		BuffTime
	);

	if(Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = JumpBuffVelocity;
	
	}

	MulticastJumpBuff(JumpBuffVelocity);
}
//-------------------------------------------------------------------------------------------------------------------------
void UBuffComponent::ResetJump()
{
	if(Character == nullptr || Character->GetCharacterMovement() == nullptr) return;

	Character->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;

	MulticastJumpBuff(InitialJumpVelocity);
}
//-------------------------------------------------------------------------------------------------------------------------
void UBuffComponent::MulticastJumpBuff_Implementation(float JumpBuffVelocity)
{
	if(Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = JumpBuffVelocity;
	}
	

}
//-------------------------------------------------------------------------------------------------------------------------
void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{

	if(Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	}
	
}

//-------------------------------------------------------------------------------------------------------------------------
void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}
//-------------------------------------------------------------------------------------------------------------------------
void UBuffComponent::SetInitialJumpVelocity(float Velocity)
{
	InitialJumpVelocity = Velocity;
}

//-------------------------------------------------------------------------------------------------------------------------
void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;

}
//-------------------------------------------------------------------------------------------------------------------------
void UBuffComponent::ReplenishShield(float ShieldAmount, float ShieldReplenishTime)
{
	bReplenishingShield = true;
	ShieldReplenishRate = ShieldAmount / ShieldReplenishTime;
	ShieldReplenishAmount += ShieldAmount;
}


//-------------------------------------------------------------------------------------------------------------------------
void UBuffComponent::HealRampUp(float DeltaTime)
{
	if(!bHealing || Character == nullptr || Character->IsElimmed()) return;

	const float HealThisFrame = HealingRate * DeltaTime;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame,0,Character->GetMaxHealth()));
	Character->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;
	if(AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0;
	}
}
//-------------------------------------------------------------------------------------------------------------------------
void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if(!bReplenishingShield || Character == nullptr || Character->IsElimmed()) return;

	const float ShieldReplenishThisFrame = ShieldReplenishRate * DeltaTime;
	Character->SetShield(FMath::Clamp(Character->GetShield() + ShieldReplenishThisFrame,0,Character->GetMaxShield()));
	Character->UpdateHUDShield();
	ShieldReplenishAmount -= ShieldReplenishThisFrame;
	if(ShieldReplenishAmount <= 0.f || Character->GetShield() >= Character->GetMaxShield())
	{
		bReplenishingShield = false;
		ShieldReplenishAmount = 0;
	}


}

//-------------------------------------------------------------------------------------------------------------------------
void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
}
//-------------------------------------------------------------------------------------------------------------------------
