// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate,bool,bPingTooHigh);
/**
 * 
 */
UCLASS()
class BLUSTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
	void SetHUDHealth(float Health,float MaxHealth);
	void SetHUDShield(float Shield,float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetHUDGrenades(int32 Grenades);
	void HideTeamScores();
	void InitTeamScores();
	void SetHUDRedTeamScore(int32 RedScore);
	void SetHUDBlueTeamScore(int32 BlueScore);

	virtual void OnPossess(APawn *InPawn) override;
	virtual void Tick(float DeltaTime) override;

	virtual float GetServerTime(); //Synced with server world clock
	virtual void  ReceivedPlayer() override; //Synced with server clock as soon as possible
	void OnMatchStateSet(FName State,bool bTeamsMatch = false);
	void HandleMatchHasStarted(bool bTeamsMatch = false);
	void HandleCooldown();

	float SingleTripTime = 0.f;

	FHighPingDelegate HighPingDelegate;

	void BroadcastElim(APlayerState *Attacker,APlayerState *Victim);

protected:

	virtual void BeginPlay() override;
	void SetHUDTime();
	void PollInit();
	virtual void SetupInputComponent() override;
	void ShowReturnToMainMenu();
	/**
	* Sync time between client and server
	*/

	//Requests the current server time, passing in the clients time when the request was sent
	UFUNCTION (Server,Reliable)
	void ServerRequestServerTime (float TimeOfClientRequest);

	//Reports the current server time to the client in response to ServerRequestServerTime 
	UFUNCTION (Client,Reliable)
	void ClientReportServerTime (float TimeOfClientRequest,float TimeServerRecievedClientRequest);
		
	float ClientServerDelta = 0.f; //difference between client and server time

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 1.f;

	float TimeSyncRunningTime = 0.f;

	void CheckTimeSync (float DeltaTime);

	UFUNCTION (Server,Reliable)
	void ServerCheckMatchState ();

	UFUNCTION (Client,Reliable)
	void ClientJoinMidGame(FName StateOfMatch,float Warmup,float Match,float Cooldown,float StartingTime);

	void HighPingWarning();
	void StopHighPingWarning();
	void CheckPing(float DeltaTime);

	UFUNCTION (Client,Reliable)
	void ClientElimAnnouncement(APlayerState *Attacker,APlayerState *Victim);

	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamScores)
	bool bShowTeamScores = false;

	UFUNCTION ()
	void OnRep_ShowTeamScores();

	FString GetInfoText(const TArray<class ABlasterPlayerState*>& Players);
	FString GetTeamsInfoText(class ABlasterGameState *BlasterGameState);



	UPROPERTY(ReplicatedUsing = OnRep_ClientsLeftGame);
	bool ClientsLeftGame = false;

	UFUNCTION()
	void OnRep_ClientsLeftGame();

private:
	UPROPERTY()
	class ABlasterHUD *BlasterHUD;


	/**
	*Return to main menu
	*/
	//UPROPERTY(EditAnywhere, Category = HUD)
	//TSubclassOf<class UUserWidget> ReturnToMainMenuWidgetAction;

	//UPROPERTY()
	//class UReturnToMainMenu* ReturnToMainMenu;

	bool bReturnToMainMenuOpen = false;


	UPROPERTY()
	class ABlasterGameMode *BlasterGameMode;

	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountdownInt = 0;



	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();
	
	UPROPERTY()
	class UCharacterOverlay *CharacterOverlay;

	//bool bInitializeCharacterOverlay = false;

	float HUDHealth;
	bool bInitializeHealth = false;

	float HUDMaxHealth;

	float HUDShield;
	bool bInitializeShield = false;

	float HUDMaxShield;

	float HUDScore;
	bool bInitializeScore = false;

	int32 HUDDefeats;
	bool bInitializeDefeats = false;

	int32 HUDGrenades;
	bool bInitializeGrenades = false;

	float HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;

	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;

	float HighPingRunnigTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;

	float PingAnimationRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float CheckPingFrequence = 20.f;

	UFUNCTION (Server,Reliable)
	void ServerReportPingStatus(bool bHighPing);

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;
};
