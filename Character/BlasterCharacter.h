// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Bluster/BlasterTypes/TurningInPlace.h"
#include "Bluster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "Bluster/BlasterTypes/CombatState.h"
#include "Bluster/BlasterTypes/Team.h"
#include "BlasterCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

UCLASS()
class BLUSTER_API ABlasterCharacter : public ACharacter,public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	
	ABlasterCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDAmmo();
	virtual void PostInitializeComponents() override;
	void SpawnDefaultWeapon();

	void SetTeamColor(ETeam Team);
	/**
	*Play Montages
	*/
	void PlayFireMonatage(bool bAiming);
	void PlayReloadMontage();
	void PlayElimMontage();
	void PlayThrowGrenadeMontage();
	void PlaySwapMontage();
	
	virtual void OnRep_ReplicatedMovement() override;

	void Elim(bool bPlayerLeftGame);

	UFUNCTION(NetMulticast,Reliable)
	void MulticastElim(bool bPlayerLeftGame);

	virtual void Destroyed() override;

	UPROPERTY(Replicated)
	bool bDisabledGameplay = false;

	UFUNCTION(BlueprintImplementableEvent) 
	void ShowSniperScopeWidget(bool bShowScope);

	UFUNCTION(Server, Reliable)
	void SeverLeaveGame();

	FOnLeftGame OnLeftGame;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();

	bool bFinishedSwapping = false;

	/**
	* Hit boxes used for server-side rewind
	*/
	UPROPERTY(EditAnywhere)
	class UBoxComponent *head;

	UPROPERTY(EditAnywhere)
	UBoxComponent *Pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent *spine_02;

	UPROPERTY(EditAnywhere)
	UBoxComponent *spine_03;

	UPROPERTY(EditAnywhere)
	UBoxComponent *UpperArm_L;

	UPROPERTY(EditAnywhere)
	UBoxComponent *UpperArm_R;

	UPROPERTY(EditAnywhere)
	UBoxComponent *lowerarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent *lowerarm_r;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent *Hand_L;

	UPROPERTY(EditAnywhere)
	UBoxComponent *Hand_R;

	UPROPERTY(EditAnywhere)
	UBoxComponent *backpack;

	UPROPERTY(EditAnywhere)
	UBoxComponent *blanket;

	UPROPERTY(EditAnywhere)
	UBoxComponent *Thigh_L;

	UPROPERTY(EditAnywhere)
	UBoxComponent *Thigh_R;

	UPROPERTY(EditAnywhere)
	UBoxComponent *calf_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent *calf_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent *Foot_L;

	UPROPERTY(EditAnywhere)
	UBoxComponent *Foot_R;

	UPROPERTY()
	TMap<FName,class UBoxComponent*> HitCollisionBoxes; 

	

protected:
	
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void ReloadButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	float CalculateSpeed();
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	void SimProxiesTurn();
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();
	void GrenadeButtonPressed();
	void DropOrDestroyWeapon(AWeapon *Weapon);
	void DropOrDestroyWeapons();

	void PlayHitReactMonatage();
	void RotateInPlace(float DeltaTime);
	void SetSpawnPoint();
	void OnPlayerStateInitialized();

	UFUNCTION() 
	void RecieveDamage(AActor *DamagedActor,float Damage,const UDamageType *DamageType,class AController *InstigatorController, AActor *DamageCauser);

	//Poll for any relevant classes and initialize our HUD
	void PollInit();


private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent *CameraBoom; 

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent *FollowCamera; 

	UPROPERTY(EditAnywhere,BlueprintReadOnly, meta = (AllowPrivateAccess = "true"));
	class UWidgetComponent *OverHeadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon *OverlappingWeapon;

	UPROPERTY()
	class ABlasterGameMode *BlasterGameMode;

	void ScaleCapsuleWhenCrouched();

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon *LastWeapon);

	/**
	* Blaster components
	*/

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,meta = (AllowPrivateAccess = "true"))
	class UCombatComponent *Combat;

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent *Buff;

	UPROPERTY(VisibleAnywhere)
	class ULagCompensationComponent *LagCompensation;


	UFUNCTION(Server,Reliable)
	void ServerEquipButtonPressed ();

	float AO_Pitch;
	float AO_Yaw;
	float InterpAO_Yaw;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	//Animation montages

	UPROPERTY(EditAnywhere,Category = Combat)
	class UAnimMontage *FireAnimMontage;

	UPROPERTY(EditAnywhere,Category = Combat)
	UAnimMontage *ReloadMontage;

	UPROPERTY(EditAnywhere,Category = Combat)
	UAnimMontage *HitReactMontage;

	UPROPERTY(EditAnywhere,Category = Combat)
	UAnimMontage *ElimMontage;

	UPROPERTY(EditAnywhere,Category = Combat)
	UAnimMontage *ThrowGrenadeMontage;

	UPROPERTY(EditAnywhere,Category = Combat)
	UAnimMontage *SwapMontage;

	void HideCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;

	//Player Health
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health,VisibleAnywhere,Category = "Player Stats")
	/*UPROPERTY(EditAnywhere, Category = "Player Stats")*/
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	/**
	* Player Shield
	*/
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield,EditAnywhere,Category = "Player Stats")
	float Shield = 0.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);

	UPROPERTY()
	class ABlasterPlayerController *BlasterPlayerController;

	bool bElimmed = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	void ElimTimerFinished();

	bool bLeftGame = false;

	
	//Dissolve effect

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent *DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat *DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	void StartDissolve();

	//Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere,Category = "Elim")
	UMaterialInstanceDynamic *DynamicDissolveMaterialInstance;

	//Material Instance set on the BluePrint,used with the dynamic material
	UPROPERTY(VisibleAnywhere,Category = "Elim")
	UMaterialInstance *DissolveMaterialInstance;

	/**
	*Team Colors
	*/
	UPROPERTY(EditAnywhere,Category = "Elim")
	UMaterialInstance *RedDissolveMatInst;

	UPROPERTY(EditAnywhere,Category = "Elim")
	UMaterialInstance *RedMaterial;

	UPROPERTY(EditAnywhere,Category = "Elim")
	UMaterialInstance *BlueDissolveMatInst;

	UPROPERTY(EditAnywhere,Category = "Elim")
	UMaterialInstance *BlueMaterial;

	UPROPERTY(EditAnywhere,Category = "Elim")
	UMaterialInstance *OriginalMaterial;

	//Elim effects
	UPROPERTY(EditAnywhere) 
	UParticleSystem *ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent *ElimBotComponent;

	UPROPERTY(EditAnywhere)  
	class USoundCue *ElimBotSound;

	UPROPERTY()
	class ABlasterPlayerState *BlasterPlayerState;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem *CrownSystem;

	UPROPERTY()
	class UNiagaraComponent *CrownComponent;

	/**
	*  Grenade
	*/
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent *AttachedGrenade;

	/**
	*  Default weapon
	*/
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon>DefaultWeaponClass;



public:

	void SetOverlappingWeapon (AWeapon *Weapon);
	bool IsWeaponEquipped ();
	bool IsAiming ();
	FORCEINLINE float GetAO_Yaw() const {return AO_Yaw;}
	FORCEINLINE float GetAO_Pitch() const {return AO_Pitch;}

	AWeapon *GetEquippedWeapon ();

	FORCEINLINE ETurningInPlace GetTurningInPlace() const {return TurningInPlace;}

	FVector GetHitTarget() const;

	FORCEINLINE UCameraComponent *GetFollowCamera() const {return FollowCamera;}

	FORCEINLINE bool ShouldRotateRootBone() const {return bRotateRootBone;}
	
	FORCEINLINE bool IsElimmed() const {return bElimmed;}

	FORCEINLINE float GetHealth() const {return Health;}
	FORCEINLINE float GetMaxHealth() const {return MaxHealth;}
	FORCEINLINE void SetHealth(float Amount) {Health = Amount;}

	FORCEINLINE float GetShield() const {return Shield;}
	FORCEINLINE float GetMaxShield() const {return MaxShield;}
	FORCEINLINE void SetShield(float Amount) {Shield = Amount;}

	ECombatState GetCombatState() const;

	FORCEINLINE UCombatComponent *GetCombat() const {return Combat;}

	FORCEINLINE bool GetDisabledGameplay() const {return bDisabledGameplay;}
		
	FORCEINLINE UAnimMontage *GetReloadMontage() const {return ReloadMontage;}
	
	FORCEINLINE UStaticMeshComponent *GetAttachedGrenade() const {return AttachedGrenade;}
	
	FORCEINLINE UBuffComponent *GetBuff() const {return Buff;}
	
	bool IsLocallyReloading();

	FORCEINLINE ULagCompensationComponent *GetLagCompensation() const {return LagCompensation;}

	FORCEINLINE bool IsHoldingTheFlag() const;

	ETeam GetTeam();

	void SetHoldingTheFlag(bool bHolding);
};
