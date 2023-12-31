// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

namespace MatchState
{
	//Match duration has been reached. Display a winner and begin cooldown timer
	extern BLUSTER_API const FName Cooldown;
}


/**
 * 
 */
UCLASS()
class BLUSTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ABlasterGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class ABlasterCharacter *ElimmedCharacter,
	class ABlasterPlayerController *VictimController,class ABlasterPlayerController *AttackerController);

	virtual float CalculateDamage(AController *Atacker,AController *Victim,float BaseDamage);

	virtual void RequestRespawn(class ACharacter *ElimmedCharacter,AController *ElimmedController);
	void PlayerLeftGame(class ABlasterPlayerState *PlayerLeaving);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartingTime = 0.f;

	bool bTeamsMatch = false;

	bool ClientsLeftGame = false;

protected:
	virtual void BeginPlay() override;

	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;

	UPROPERTY()
	class UMultiplayerSessionsSubsystem *MultiplayerSessionsSubsystem;

public:
	FORCEINLINE float GetCountdownTime() const {return CountdownTime;}
	FORCEINLINE bool GetClientsLeftGame() const {return ClientsLeftGame;}
};
