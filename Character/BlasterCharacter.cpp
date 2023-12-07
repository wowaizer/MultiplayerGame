// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Bluster/Weapon/Weapon.h"
#include "Bluster/BlasterComponents/CombatComponent.h"
#include "Bluster/BlasterComponents/BuffComponent.h"
#include "Bluster/BlasterComponents/LagCompensationComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "BlasterAnimInstance.h"
#include "Bluster/Bluster.h"
#include "Bluster/PlayerController/BlasterPlayerController.h"
#include "Bluster/GameMode/BlasterGameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Bluster/PlayerState/BlasterPlayerState.h"
#include "Bluster/Weapon/WeaponTypes.h"
#include "Components/BoxComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Bluster/GameState/BlasterGameState.h"
#include "Bluster/PlayerStart/TeamPlayerStart.h"


//-------------------------------------------------------------------------------------------------------------------------
ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	CameraBoom = CreateDefaultSubobject <USpringArmComponent> (TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject <UCameraComponent> (TEXT("FollowBoom"));
	FollowCamera->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverHeadWidget = CreateDefaultSubobject <UWidgetComponent> (TEXT("OverHeadWidget"));
	OverHeadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject <UCombatComponent> (TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Buff = CreateDefaultSubobject<UBuffComponent> (TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent> (TEXT("LagCompensation"));
	
	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent> (TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(),FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera,ECollisionResponse::ECR_Ignore);

	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera,ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility,ECollisionResponse::ECR_Block);

	GetCharacterMovement()->RotationRate = FRotator(0.f,0.f,850.f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent> (TEXT("DissolveTimelineComponent"));
		
	/**
	* Hit boxes used for server-side rewind
	*/

	head = CreateDefaultSubobject<UBoxComponent> (TEXT("head"));	
	head->SetupAttachment(GetMesh(),FName("head"));
	HitCollisionBoxes.Add(FName("head"),head);

	Pelvis = CreateDefaultSubobject<UBoxComponent> (TEXT("Pelvis"));	
	Pelvis->SetupAttachment(GetMesh(),FName("Pelvis"));
	HitCollisionBoxes.Add(FName("Pelvis"),Pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent> (TEXT("spine_02"));	
	spine_02->SetupAttachment(GetMesh(),FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"),spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent> (TEXT("spine_03"));	
	spine_03->SetupAttachment(GetMesh(),FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"),spine_03);

	UpperArm_L = CreateDefaultSubobject<UBoxComponent> (TEXT("UpperArm_L"));	
	UpperArm_L->SetupAttachment(GetMesh(),FName("UpperArm_L"));
	HitCollisionBoxes.Add(FName("UpperArm_L"),UpperArm_L);

	UpperArm_R = CreateDefaultSubobject<UBoxComponent> (TEXT("UpperArm_R"));	
	UpperArm_R->SetupAttachment(GetMesh(),FName("UpperArm_R"));
	HitCollisionBoxes.Add(FName("UpperArm_R"),UpperArm_R);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent> (TEXT("lowerarm_l"));	
	lowerarm_l->SetupAttachment(GetMesh(),FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"),lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent> (TEXT("lowerarm_r"));	
	lowerarm_r->SetupAttachment(GetMesh(),FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"),lowerarm_r);

	Hand_L = CreateDefaultSubobject<UBoxComponent> (TEXT("Hand_L"));	
	Hand_L->SetupAttachment(GetMesh(),FName("Hand_L"));
	HitCollisionBoxes.Add(FName("Hand_L"),Hand_L);

	Hand_R = CreateDefaultSubobject<UBoxComponent> (TEXT("Hand_R"));	
	Hand_R->SetupAttachment(GetMesh(),FName("Hand_R"));
	HitCollisionBoxes.Add(FName("Hand_R"),Hand_R);

	backpack = CreateDefaultSubobject<UBoxComponent> (TEXT("backpack"));	
	backpack->SetupAttachment(GetMesh(),FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"),backpack);

	blanket = CreateDefaultSubobject<UBoxComponent> (TEXT("blanket"));	
	blanket->SetupAttachment(GetMesh(),FName("backpack"));
	HitCollisionBoxes.Add(FName("blanket"),backpack);

	Thigh_L = CreateDefaultSubobject<UBoxComponent> (TEXT("Thigh_L"));	
	Thigh_L->SetupAttachment(GetMesh(),FName("Thigh_L"));
	HitCollisionBoxes.Add(FName("Thigh_L"),Thigh_L);

	Thigh_R = CreateDefaultSubobject<UBoxComponent> (TEXT("Thigh_R"));	
	Thigh_R->SetupAttachment(GetMesh(),FName("Thigh_R"));
	HitCollisionBoxes.Add(FName("Thigh_R"),Thigh_R);

	calf_l = CreateDefaultSubobject<UBoxComponent> (TEXT("calf_l"));	
	calf_l->SetupAttachment(GetMesh(),FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"),calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent> (TEXT("calf_r"));	
	calf_r->SetupAttachment(GetMesh(),FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"),calf_r);

	Foot_L = CreateDefaultSubobject<UBoxComponent> (TEXT("Foot_L"));	
	Foot_L->SetupAttachment(GetMesh(),FName("Foot_L"));
	HitCollisionBoxes.Add(FName("Foot_L"),Foot_L);

	Foot_R = CreateDefaultSubobject<UBoxComponent> (TEXT("Foot_R"));	
	Foot_R->SetupAttachment(GetMesh(),FName("Foot_R"));
	HitCollisionBoxes.Add(FName("Foot_R"),Foot_R);


	for(auto Box : HitCollisionBoxes)
	{
		if(Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECHitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECHitBox,ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		
	}


}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter,OverlappingWeapon,COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter,Health);
	DOREPLIFETIME(ABlasterCharacter,Shield);
	DOREPLIFETIME(ABlasterCharacter,bDisabledGameplay);

}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	SpawnDefaultWeapon();
	UpdateHUDAmmo();
	UpdateHUDHealth();
	UpdateHUDShield();

	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this,&ABlasterCharacter::RecieveDamage);

	}
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
	
}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::Elim(bool bPlayerLeftGame)
{
	
	DropOrDestroyWeapons();
	
	MulticastElim(bPlayerLeftGame);

	
}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;

	bElimmed = true;
	PlayElimMontage();


	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);
	}


	//Start dissolve effect
	if(DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance,this);

		GetMesh()->SetMaterial(0,DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"),0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"),200.f);
	}

	StartDissolve();

	//Disable character movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	bDisabledGameplay = true;
	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}

	//Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Spawn elim bot
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X,GetActorLocation().Y,
			GetActorLocation().Z + 200.f);

		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
		);
		
		if(ElimBotSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this,ElimBotSound,GetActorLocation());
		}
	}
	bool bHideSniperScope = IsLocallyControlled() && 
		Combat && 
		Combat->bAiming &&
		Combat->EquippedWeapon &&
		Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;

	if(bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}


	if(CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}

	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ABlasterCharacter::ElimTimerFinished,
		ElimDelay
	);

}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::ElimTimerFinished()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	if(BlasterGameMode && !bLeftGame)
	{
		BlasterGameMode->RequestRespawn(this,Controller);
		
	}
	if(bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::SeverLeaveGame_Implementation()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;

	BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	
	if(BlasterGameMode && BlasterPlayerState)
	{
		BlasterGameMode->PlayerLeftGame(BlasterPlayerState);

	}
}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;

	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;

	if(Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::MulticastGainedTheLead_Implementation()
{
	if(CrownSystem == nullptr) return;

	if(CrownComponent == nullptr)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetMesh(),
			FName(),
			GetActorLocation() + FVector(0.f,0.f,110.f),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
			);
	}
	if(CrownComponent)
	{
		CrownComponent->Activate();
	}

}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::MulticastLostTheLead_Implementation()
{
	if(CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}

}

//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	RotateInPlace(DeltaTime);

	HideCameraIfCharacterClose();
	PollInit();

}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::RotateInPlace(float DeltaTime)
{
	if(Combat && Combat->bHoldingTheFlag)
	{
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	if(Combat && Combat->EquippedWeapon)
	{	
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = true;
	}


	if (bDisabledGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	if(GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}


}


//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump",IE_Pressed,this,&ABlasterCharacter::Jump);

	PlayerInputComponent->BindAction("Equip",IE_Pressed,this,&ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch",IE_Pressed,this,&ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim",IE_Pressed,this,&ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim",IE_Released,this,&ABlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire",IE_Pressed,this,&ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire",IE_Released,this,&ABlasterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload",IE_Pressed,this,&ABlasterCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("ThrowGrenade",IE_Pressed,this,&ABlasterCharacter::GrenadeButtonPressed);
	

	PlayerInputComponent->BindAxis("MoveForward",this,&ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight",this,&ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn",this,&ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp",this,&ABlasterCharacter::LookUp);
}

//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents ();

	if (Combat)
	{
		Combat->Character = this;
	}

	if (Buff)
	{
		Buff->Character = this;
		Buff->SetInitialSpeeds(
			GetCharacterMovement()->MaxWalkSpeed,
			GetCharacterMovement()->MaxWalkSpeedCrouched
		);
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}

	if(LagCompensation)
	{
		LagCompensation->Character = this;

		if(Controller)
		{
			LagCompensation->Controller = Cast<ABlasterPlayerController>(Controller);
		}
	}
}

//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::PlayFireMonatage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
		return;

	UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && FireAnimMontage)
	{
		AnimInstance->Montage_Play(FireAnimMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}

}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
		return;

	UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;

		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;

		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;

		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;

		case EWeaponType::EWT_SubMachineGun:
			SectionName = FName("Pistol");
			break;

		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;

		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("SniperRifle");
			break;

		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;
		}
		
		AnimInstance->Montage_JumpToSection(SectionName);
	}

}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
		
	}

}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
	
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::PlaySwapMontage()
{
	UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
	}

}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::SpawnDefaultWeapon()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	UWorld *World = GetWorld();
	if(BlasterGameMode && World && !bElimmed && DefaultWeaponClass)
	{
		AWeapon *StartringWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartringWeapon->bDestroyWeapon = true;
		if(Combat)
		{
			Combat->EquipWeapon(StartringWeapon);
		}
	}
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::SetTeamColor(ETeam Team)
{
	if(GetMesh() == nullptr || OriginalMaterial == nullptr ) return;
	
	switch (Team)
	{
	case ETeam::ET_NoTeam:
		GetMesh()->SetMaterial(0,OriginalMaterial);
		DissolveMaterialInstance = BlueDissolveMatInst;
		break;

	case ETeam::ET_RedTeam:
		GetMesh()->SetMaterial(0,RedMaterial);
		DissolveMaterialInstance = RedDissolveMatInst;
		break;

	case ETeam::ET_BlueTeam:
		GetMesh()->SetMaterial(0,BlueMaterial);
		DissolveMaterialInstance = BlueDissolveMatInst;
		break;
	}
}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::PlayHitReactMonatage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
		return;

	UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::RecieveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;

	if(bElimmed || BlasterGameMode == nullptr) return;
	
	Damage = BlasterGameMode->CalculateDamage(InstigatorController,Controller,Damage);

	float DamageToHealth = Damage;

	if(Shield > 0)
	{
		if(Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage,0.f,MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield,0.f,Damage);
			Shield = 0.f;
		}
	}
	Health = FMath::Clamp(Health - DamageToHealth,0.f,MaxHealth);

	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMonatage();

	if (Health == 0.f)
	{
		
		if(BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? 
				Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;

			ABlasterPlayerController *AttackerController = Cast<ABlasterPlayerController>(InstigatorController);

			BlasterGameMode->PlayerEliminated(this,BlasterPlayerController,AttackerController);
		
		}

	}

}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::MoveForward(float Value)
{
	if (bDisabledGameplay)
	{
		return;
	}

	if (Controller != 0 && Value != 0.f)
	{
		const FRotator YawRotation(0.f,Controller->GetControlRotation().Yaw,0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput (Direction,Value);
	}

}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::MoveRight(float Value)
{
	if (bDisabledGameplay)
	{
		return;
	}

	if (Controller != 0 && Value != 0.f)
	{
		const FRotator YawRotation(0.f,Controller->GetControlRotation().Yaw,0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput (Direction,Value);
	}


}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);

}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);

}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::EquipButtonPressed()
{
	if (bDisabledGameplay) return;
	
	if (Combat)
	{
		if(Combat->bHoldingTheFlag) return;
		
		if(Combat->CombatState == ECombatState::ECS_Unoccupied) ServerEquipButtonPressed();
		bool bSwap = Combat->ShouldSwapWeapons() && 
			!HasAuthority() && Combat->CombatState == ECombatState::ECS_Unoccupied && 
			OverlappingWeapon == nullptr;

		if(bSwap)
		{
			PlaySwapMontage();
			Combat->CombatState = ECombatState::ECS_SwappingWeapons;
			bFinishedSwapping = false;
		}
	}
}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::CrouchButtonPressed()
{
	if(Combat && Combat->bHoldingTheFlag) return;

	if (bDisabledGameplay)
	{
		return;
	}

	if (bIsCrouched)
	{
		UnCrouch ();
	}
	else
	{
		Crouch ();
		
		/*float Speed = CalculateSpeed();
	

		if(Speed == 0.f)
		{
			FVector CurrentCrouchLocation = GetActorLocation(); 
		
			FVector CorrectCrouchLocation = CurrentCrouchLocation + FVector(0.f,0.f,30.f);

			SetActorLocation(CurrentCrouchLocation);
		
	
		}*/

		ScaleCapsuleWhenCrouched();
		
	}
	
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::ScaleCapsuleWhenCrouched()
{
	
	
		const float ComponentScale = GetCapsuleComponent()->GetShapeScale();
		const float OldUnscaledHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
		GetCapsuleComponent()->SetCapsuleSize(GetCapsuleComponent()->GetUnscaledCapsuleRadius(),GetCharacterMovement()->CrouchedHalfHeight);
		float HalfHeightAdjust = OldUnscaledHalfHeight - GetCharacterMovement()->CrouchedHalfHeight;

		float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

		OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust * 3.f);
	



	//float Speed = CalculateSpeed();
	//if(Speed > 0.f)
	//{
	//	GetCapsuleComponent()->SetCapsuleSize(GetCapsuleComponent()->GetUnscaledCapsuleRadius(),GetCharacterMovement()->CrouchedHalfHeight);
	//	GetCapsuleComponent()->SetCapsuleHalfHeight(GetCharacterMovement()->CrouchedHalfHeight);


	//	float HalfHeightAdjust = OldUnscaledHalfHeight - GetCharacterMovement()->CrouchedHalfHeight;

	//	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	//	OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	//}

	//if(Speed == 0.f)
	//{
	//	GetCapsuleComponent()->SetCapsuleSize(GetCapsuleComponent()->GetUnscaledCapsuleRadius(),GetCharacterMovement()->CrouchedHalfHeight);
	//	GetCapsuleComponent()->SetCapsuleHalfHeight(GetCharacterMovement()->CrouchedHalfHeight - 50.f);

	//	float HalfHeightAdjust = OldUnscaledHalfHeight - GetCharacterMovement()->CrouchedHalfHeight;

	//	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	//	OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	//}
	//	

}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::ReloadButtonPressed()
{
	if(Combat && Combat->bHoldingTheFlag) return;

	if (bDisabledGameplay)
	{
		return;
	}

	if(Combat)
	{
		Combat->Reload();
	}
}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::AimButtonPressed()
{
	if(Combat && Combat->bHoldingTheFlag) return;

	if (bDisabledGameplay)
	{
		return;
	}

	if (bDisabledGameplay)
	{
		return;
	}

	if (Combat)
	{
		Combat->SetAiming(true);
	}

}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::AimButtonReleased()
{
	if(Combat && Combat->bHoldingTheFlag) return;

	if (bDisabledGameplay)
	{
		return;
	}

	if (Combat)
	{
		Combat->SetAiming(false); 
	}

}
//-------------------------------------------------------------------------------------------------------------------------
float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity ();
	Velocity.Z = 0.f;
	return Velocity.Size ();
}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) 
		return;

	float Speed = CalculateSpeed();

	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) //standing still, not jumping
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f,GetBaseAimRotation().Yaw,0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation,StartingAimRotation);

		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}

		AO_Yaw = DeltaAimRotation.Yaw;
		bUseControllerRotationYaw = true;
		TurnInPlace (DeltaTime);
	
	}

	if (Speed > 0.f || bIsInAir) //runnig or jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f,GetBaseAimRotation().Yaw,0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void ABlasterCharacter::CalculateAO_Pitch()
{

	AO_Pitch = GetBaseAimRotation().Pitch;

	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//map pitch from [270,360) to [90,0)

		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
		return;

	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if(Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}


	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation,ProxyRotationLastFrame).Yaw;

	if(FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if(ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if(ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::Jump()
{
	if(Combat && Combat->bHoldingTheFlag) return;


	if (bDisabledGameplay)
	{
		return;
	}

	if (bIsCrouched)
	{
		UnCrouch ();
	}
	else
	{
		Super::Jump();
	}

}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::FireButtonPressed()
{
	if(Combat && Combat->bHoldingTheFlag) return;

	if (bDisabledGameplay)
	{
		return;
	}

	if (Combat)
	{
		Combat->FireButtonPressed(true);

	}

}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::FireButtonReleased()
{
	if(Combat && Combat->bHoldingTheFlag) return;

	if (bDisabledGameplay)
	{
		return;
	}

	if (Combat)
	{
		Combat->FireButtonPressed(false);

	}

}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::GrenadeButtonPressed()
{

	if (Combat)
	{
		if(Combat->bHoldingTheFlag) return;

		Combat->ThrowGrenade();
	}

}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if(Weapon == nullptr) return;
	if(Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
	
}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::DropOrDestroyWeapons()
{
	if(Combat)
	{
		if (Combat->EquippedWeapon)
		{
			DropOrDestroyWeapon(Combat->EquippedWeapon);
		}
		if(Combat->SecondaryWeapon)
		{
			DropOrDestroyWeapon(Combat->SecondaryWeapon);
		}

		if (Combat->TheFlag)
		{
			Combat->TheFlag->Dropped();
		}
	}
	

}
//-------------------------------------------------------------------------------------------------------------------------



void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon *LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}

	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}

}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		if (OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else if(Combat->ShouldSwapWeapons())
		{
			Combat->SwapWeapons();
		}
	}
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	/*UE_LOG(LogTemp, Warning,TEXT("AO_Yaw: %f"),AO_Yaw);*/

	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}

	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw,0.f,DeltaTime,4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f,GetBaseAimRotation().Yaw,0.f);
		}

	}
}

//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled())
		return;

	if((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);

		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}

		if (Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}

	else
	{
		GetMesh()->SetVisibility(true);

		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}

		if (Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
	
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth)
	{
		PlayHitReactMonatage();
	}
	
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield)
	{
		PlayHitReactMonatage();
	}
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::UpdateHUDHealth()
{

	BlasterPlayerController = BlasterPlayerController == nullptr ?
		Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;

	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::UpdateHUDShield()
{

	BlasterPlayerController = BlasterPlayerController == nullptr ?
		Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;

	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDShield(Shield, MaxShield);
	}

}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::UpdateHUDAmmo()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ?
		Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;

	if (BlasterPlayerController && Combat && Combat->EquippedWeapon)
	{
		BlasterPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
		BlasterPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}


}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if(BlasterPlayerState)
		{
		
			OnPlayerStateInitialized();

			ABlasterGameState *BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			if(BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(BlasterPlayerState))
			{
				MulticastGainedTheLead();
			}

		}
	}
	
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::SetSpawnPoint()
{
	if(HasAuthority() && BlasterPlayerState->GetTeam() != ETeam::ET_NoTeam)
	{

		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this,ATeamPlayerStart::StaticClass(),PlayerStarts);
		TArray<ATeamPlayerStart*> TeamPlayerStarts;
		for(auto Start : PlayerStarts)
		{
			ATeamPlayerStart* TeamStart = Cast<ATeamPlayerStart>(Start);
			if(TeamStart && TeamStart->Team == BlasterPlayerState->GetTeam())
			{
				TeamPlayerStarts.Add(TeamStart);
			}
		}
		if(TeamPlayerStarts.Num() > 0)
		{
			ATeamPlayerStart* ChosenPlayerStart = TeamPlayerStarts[FMath::RandRange(0,TeamPlayerStarts.Num() - 1)];
			SetActorLocationAndRotation(
				ChosenPlayerStart->GetActorLocation(),
				ChosenPlayerStart->GetActorRotation()
			);
				
		}
	}
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::OnPlayerStateInitialized()
{
	BlasterPlayerState->AddToScore(0.f);
	BlasterPlayerState->AddToDefeats(0);
	SetTeamColor(BlasterPlayerState->GetTeam());
	SetSpawnPoint();
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if(DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"),DissolveValue);
	}
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this,&ABlasterCharacter::UpdateDissolveMaterial);

	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve,DissolveTrack);
		DissolveTimeline->Play();
	}
}

//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}


	OverlappingWeapon = Weapon;

	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}
//-------------------------------------------------------------------------------------------------------------------------

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}
//-------------------------------------------------------------------------------------------------------------------------
bool ABlasterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}
AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}
//-------------------------------------------------------------------------------------------------------------------------
FVector ABlasterCharacter::GetHitTarget() const
{
	if (Combat == nullptr)
		return FVector();

	return Combat->HitTarget;
}
//-------------------------------------------------------------------------------------------------------------------------

ECombatState ABlasterCharacter::GetCombatState() const
{
	if (Combat == nullptr)
	{
		return ECombatState::ECS_MAX;
	}
	return Combat->CombatState;
		
}
//-------------------------------------------------------------------------------------------------------------------------
bool ABlasterCharacter::IsLocallyReloading()
{
	if (Combat == nullptr) return false;
	return Combat->bLocallyReloading;
}
//-------------------------------------------------------------------------------------------------------------------------
bool ABlasterCharacter::IsHoldingTheFlag() const
{
	if (Combat == nullptr) return false;
	return Combat->bHoldingTheFlag;
}
//-------------------------------------------------------------------------------------------------------------------------
ETeam ABlasterCharacter::GetTeam()
{
	BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	if(BlasterPlayerState == nullptr) return ETeam::ET_NoTeam;
	
	return BlasterPlayerState->GetTeam();
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::SetHoldingTheFlag(bool bHolding)
{
	if(Combat == nullptr) return;
	
	Combat->bHoldingTheFlag = bHolding; 
	
	
}
//-------------------------------------------------------------------------------------------------------------------------
