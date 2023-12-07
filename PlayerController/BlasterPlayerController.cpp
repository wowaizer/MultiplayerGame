// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"
#include "Bluster/HUD/BlasterHUD.h"
#include "Bluster/HUD/CharacterOverlay.h"
#include "Bluster/HUD/Annonucement.h"
#include "Bluster/HUD/ReturnToMainMenu.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Bluster/Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Bluster/GameMode/BlasterGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Bluster/BlasterComponents/CombatComponent.h"
#include "Bluster/GameState/BlasterGameState.h"
#include "Bluster/PlayerState/BlasterPlayerState.h"
#include "Components/Image.h"
#include "Bluster/BlasterTypes/Announcement.h"

//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	ServerCheckMatchState();
	
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController,MatchState);
	DOREPLIFETIME(ABlasterPlayerController,bShowTeamScores);
	//DOREPLIFETIME(ABlasterPlayerController,ClientsLeftGame);
	
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
	CheckPing(DeltaTime);




	
	//BlasterGameMode = BlasterGameMode  == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
	//if(BlasterGameMode)
	//{
	//	ClientsLeftGame = BlasterGameMode->GetClientsLeftGame();
	//}

}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker,Victim);
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState *Self = GetPlayerState<APlayerState>();
	if(Attacker && Victim && Self)
	{
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
		if(BlasterHUD)
		{
			if(Attacker == Self && Victim != Self)
			{
				BlasterHUD->AddElimAnnonucement("You",Victim->GetPlayerName());
				return;
			}
			if(Victim == Self && Attacker != Self)
			{
				BlasterHUD->AddElimAnnonucement(Attacker->GetPlayerName(),"you");
				return;
			}
			if(Attacker == Victim && Attacker == Self)
			{
				BlasterHUD->AddElimAnnonucement("You","yourself");
				return;
			}
			if(Attacker == Victim && Attacker != Self)
			{
				BlasterHUD->AddElimAnnonucement(Attacker->GetPlayerName(),"themselves");
				return;
			}
			BlasterHUD->AddElimAnnonucement(Attacker->GetPlayerName(),Victim->GetPlayerName());

		}
	}

}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunnigTime += DeltaTime;
	if(HighPingRunnigTime > CheckPingFrequence)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
		if(PlayerState)
		{
			/*UE_LOG(LogTemp, Warning, TEXT("PING %d"),PlayerState->GetPing() * 4)*/
			if(PlayerState->GetPing() * 4 > HighPingThreshold) // ping is compressed. Its actually ping / 4
			{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunnigTime = 0.f;
	}

	bool bHighPingAnimationPlaying = BlasterHUD && 
			BlasterHUD->CharacterOverlay &&
			BlasterHUD->CharacterOverlay->HighPingAnimation &&
			BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation);

		if(bHighPingAnimationPlaying)
		{
			PingAnimationRunningTime += DeltaTime;
			if(PingAnimationRunningTime > HighPingDuration)
			{
				StopHighPingWarning();
			}
		}

	
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::OnRep_ShowTeamScores()
{
	if(bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}

}




//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	//is the ping too high?


	HighPingDelegate.Broadcast(bHighPing);
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if(IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}

}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::HighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HighPingIcon 
		&& BlasterHUD->CharacterOverlay->HighPingAnimation;

	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingIcon->SetOpacity(1.f);
		BlasterHUD->CharacterOverlay->PlayAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation,
			0.f,
			5
			);
	}
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::StopHighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HighPingIcon 
		&& BlasterHUD->CharacterOverlay->HighPingAnimation;

	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingIcon->SetOpacity(0.f);
		if(BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation))
		{
			BlasterHUD->CharacterOverlay->StopAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation);
		}
			
	}
}


//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	ABlasterGameMode *GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if(GameMode)
	{
		
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		CooldownTime = GameMode->CooldownTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidGame(MatchState,WarmupTime,MatchTime,CooldownTime,LevelStartingTime);

	}

}

//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch,float Warmup,float Match,float Cooldown,float StartingTime)
{
	MatchState = StateOfMatch;
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	OnMatchStateSet(MatchState);

	if(BlasterHUD && MatchState == MatchState::WaitingToStart)
	{
		BlasterHUD->AddAnnonucement();
	}


}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ABlasterCharacter *BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if(BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetHealth(),BlasterCharacter->GetMaxHealth());
	}
}

//-------------------------------------------------------------------------------------------------------------------------



void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HealthBar 
		&& BlasterHUD->CharacterOverlay->HealthText;

	
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);

		FString HealthText = FString::Printf(TEXT("%d/%d"),
			FMath::CeilToInt(Health),FMath::CeilToInt(MaxHealth));

		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterPlayerController::SetHUDShield(float Shield, float MaxShield)
{

	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->ShieldBar 
		&& BlasterHUD->CharacterOverlay->ShieldText;

	if (bHUDValid)
	{
		const float ShieldPercent = Shield / MaxShield;
		BlasterHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);

		FString ShieldText = FString::Printf(TEXT("%d/%d"),
			FMath::CeilToInt(Shield),FMath::CeilToInt(MaxShield));

		BlasterHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}

}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterPlayerController::SetHUDScore(float Score)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay
		&& BlasterHUD->CharacterOverlay->ScoreAmount;

	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"),
			FMath::FloorToInt(Score));

		BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}

	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}

}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterPlayerController::SetHUDDefeats(int32 Defeats)
{

	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay
		&& BlasterHUD->CharacterOverlay->DefeatsAmount;

	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"),Defeats);

		BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}

	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}


}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay
		&& BlasterHUD->CharacterOverlay->WeaponAmmoAmount;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"),Ammo);

		BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}

	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}

}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay
		&& BlasterHUD->CharacterOverlay->CarriedAmmoAmount;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"),Ammo);

		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}

	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}

}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay
		&& BlasterHUD->CharacterOverlay->MatchCountdownText;

	if (bHUDValid)
	{

		if(CountdownTime < 0.f)
		{
			BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"),Minutes,Seconds);

		BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && BlasterHUD->Annonucement
		&& BlasterHUD->Annonucement->WarmupTime;

	if (bHUDValid)
	{
		if(CountdownTime < 0.f)
		{
			BlasterHUD->Annonucement->WarmupTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"),Minutes,Seconds);

		BlasterHUD->Annonucement->WarmupTime->SetText(FText::FromString(CountdownText));
	}

}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::SetHUDGrenades(int32 Grenades)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay
		&& BlasterHUD->CharacterOverlay->GrenadesText;

	if (bHUDValid)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"),Grenades);

		BlasterHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}

	else
	{
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}

}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::HideTeamScores()
{

	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && 
		BlasterHUD->CharacterOverlay && 
		BlasterHUD->CharacterOverlay->RedTeamScore && 
		BlasterHUD->CharacterOverlay->BlueTeamScore &&
		BlasterHUD->CharacterOverlay->ScoreSpacerText;

	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->RedTeamScore->SetText(FText());
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetText(FText());
		BlasterHUD->CharacterOverlay->ScoreSpacerText->SetText(FText());
	}
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::InitTeamScores()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && 
		BlasterHUD->CharacterOverlay && 
		BlasterHUD->CharacterOverlay->RedTeamScore && 
		BlasterHUD->CharacterOverlay->BlueTeamScore &&
		BlasterHUD->CharacterOverlay->ScoreSpacerText;

	if (bHUDValid)
	{
		FString Zero ("0");
		FString Spacer ("|");
		BlasterHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(Zero));
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(Zero));
		BlasterHUD->CharacterOverlay->ScoreSpacerText->SetText(FText::FromString(Spacer));
	}
}
void ABlasterPlayerController::SetHUDRedTeamScore(int32 RedScore)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && 
		BlasterHUD->CharacterOverlay && 
		BlasterHUD->CharacterOverlay->RedTeamScore; 
		
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"),RedScore);
		BlasterHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(ScoreText));
		
	}
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::SetHUDBlueTeamScore(int32 BlueScore)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && 
		BlasterHUD->CharacterOverlay && 
		BlasterHUD->CharacterOverlay->BlueTeamScore; 

	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"),BlueScore);
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(ScoreText));

	}
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if(MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	}
	else if(MatchState == MatchState::InProgress)
	{
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::Cooldown)
	{
		TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if(HasAuthority())
	{
		if (BlasterGameMode == nullptr)
		{
			BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
			LevelStartingTime = BlasterGameMode->LevelStartingTime;
		}
		
		BlasterGameMode = BlasterGameMode  == nullptr ? 
			Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;

		if(BlasterGameMode)
		{
			SecondsLeft = FMath::CeilToInt(BlasterGameMode->GetCountdownTime()+ LevelStartingTime);
		}
	}


	if(CountdownInt != SecondsLeft)
	{
		if(MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if(MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
		
	}

	CountdownInt = SecondsLeft;
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::PollInit()
{
	if(CharacterOverlay == nullptr) 
	{
		if(BlasterHUD && BlasterHUD->CharacterOverlay)
		{
			CharacterOverlay = BlasterHUD->CharacterOverlay;
			if(CharacterOverlay)
			{
				if (bInitializeHealth)  SetHUDHealth(HUDHealth,HUDMaxHealth);
				if (bInitializeShield)  SetHUDShield(HUDShield,HUDMaxShield);
				if (bInitializeScore)   SetHUDScore(HUDScore);
				if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);

				ABlasterCharacter *BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
				if(BlasterCharacter && BlasterCharacter->GetCombat())
				{
					if (bInitializeGrenades) 
					{
						SetHUDGrenades(BlasterCharacter->GetCombat()->GetGrenades());
					
					}
					
					BlasterCharacter->UpdateHUDAmmo();
					BlasterCharacter->UpdateHUDShield();
				}

			}

		}
	}
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if(InputComponent == nullptr) return;

	InputComponent->BindAction("Quit",IE_Pressed,this,&ABlasterPlayerController::ShowReturnToMainMenu);
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::ShowReturnToMainMenu()
{

	if(BlasterHUD == nullptr) return;
	
	if(BlasterHUD->ReturnToMainMenuWidgetAction == nullptr)
	{
		return;
	}
		
	//	UE_LOG(LogTemp, Warning, TEXT("ReturnToMainMenuWidget is null !!!!"));
	
	
	if(BlasterHUD->ReturnToMainMenu == nullptr)
	{
		BlasterHUD->ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this,BlasterHUD->ReturnToMainMenuWidgetAction);	
	}

	if(BlasterHUD->ReturnToMainMenu)
	{
		
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if(bReturnToMainMenuOpen)
		{
			BlasterHUD->ReturnToMainMenu->MenuSetup();
			
		}
		else
		{
			BlasterHUD->ReturnToMainMenu->MenuTearDown();
			
		}

	}
}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest,ServerTimeOfReceipt);
}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerRecievedClientRequest)
{
	
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimeServerRecievedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}
//-------------------------------------------------------------------------------------------------------------------------
float ABlasterPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
	
}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if(IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	
	}

}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterPlayerController::OnMatchStateSet(FName State,bool bTeamsMatch)
{
	MatchState = State;

	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
		
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();

		/*if(BlasterGameMode)
		{
			for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{

				ABlasterPlayerController *BlasterPlayer = Cast<ABlasterPlayerController>(*It);

				BlasterPlayer->ClientReturnToMainMenu(FString());
			}

		}*/
	}
	
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::OnRep_ClientsLeftGame()
{

	/*if(BlasterGameMode)
	{
		for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{

			ABlasterPlayerController *BlasterPlayer = Cast<ABlasterPlayerController>(*It);

			BlasterPlayer->ClientReturnToMainMenu(FString());
		}

	}*/
}

//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::OnRep_MatchState()
{
	
	if(MatchState == MatchState::InProgress)
	{
	
		HandleMatchHasStarted();
	}

	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();

		//if(BlasterGameMode)
		//{
		//	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		//	{

		//		ABlasterPlayerController *BlasterPlayer = Cast<ABlasterPlayerController>(*It);

		//		BlasterPlayer->ClientReturnToMainMenu(FString());
		//	}

		//}
	}

	

}

//-------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::HandleMatchHasStarted(bool bTeamsMatch)
{
	if(HasAuthority()) bShowTeamScores = bTeamsMatch;

	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if(BlasterHUD)
	{
		if(BlasterHUD->CharacterOverlay == nullptr) BlasterHUD->AddCharacterOverlay();

		if(BlasterHUD->Annonucement)
		{
			BlasterHUD->Annonucement->SetVisibility(ESlateVisibility::Hidden);
		}

		if(!HasAuthority()) return;

		if(bTeamsMatch)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamScores();
		}
	}
}
//-------------------------------------------------------------------------------------------------------------------------

void ABlasterPlayerController::HandleCooldown()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if(BlasterHUD)
	{

		BlasterHUD->CharacterOverlay->RemoveFromParent();

		bool bHUDValid = BlasterHUD->Annonucement && BlasterHUD->Annonucement->AnnoncementText &&
			BlasterHUD->Annonucement->InfoText;

		if(bHUDValid)
		{
			BlasterHUD->Annonucement->SetVisibility(ESlateVisibility::Visible);
			FString AnnoncementText = Announcement::NewMatchStartsIn;
			BlasterHUD->Annonucement->AnnoncementText->SetText(FText::FromString(AnnoncementText));

			ABlasterGameState *BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			ABlasterPlayerState *BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
			
			if(BlasterGameState && BlasterPlayerState)
			{
				TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;

				FString InfoTextString = bShowTeamScores ? GetTeamsInfoText(BlasterGameState) : GetInfoText(TopPlayers);
				
				
				BlasterHUD->Annonucement->InfoText->SetText(FText::FromString(InfoTextString));
			}

		}

	}

	ABlasterCharacter *BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
	if (BlasterCharacter && BlasterCharacter->GetCombat())
	{
		BlasterCharacter->bDisabledGameplay = true;
		BlasterCharacter->GetCombat()->FireButtonPressed(false);
	}


}
//-------------------------------------------------------------------------------------------------------------------------
FString ABlasterPlayerController::GetInfoText(const TArray<class ABlasterPlayerState*>& Players)
{
	ABlasterPlayerState *BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
	if(BlasterPlayerState == nullptr) return FString();

	FString InfoTextString;

	if(Players.Num() == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;

	}
	else if(Players.Num() == 1 && Players[0] == BlasterPlayerState)
	{
		InfoTextString = Announcement::YouAreTheWinner;
	}

	else if(Players.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("Winner: \n%s"),*Players[0]->GetPlayerName());
	}

	else if(Players.Num() > 1)
	{
		InfoTextString = Announcement::PlayersAreTiedForTheWin;
		InfoTextString.Append(FString("\n"));

		for (auto TiedPlayer : Players)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"),*TiedPlayer->GetPlayerName()));
		}
	}


	return InfoTextString;
}
//-------------------------------------------------------------------------------------------------------------------------
FString ABlasterPlayerController::GetTeamsInfoText(ABlasterGameState* BlasterGameState)
{
	if (BlasterGameState == nullptr) return FString();
	FString InfoTextString;

	const int32 RedTeamScore = BlasterGameState->RedTeamScore;
	const int32 BlueTeamScore = BlasterGameState->BlueTeamScore;

	if(RedTeamScore == 0 && BlueTeamScore== 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if(RedTeamScore == BlueTeamScore)
	{
		InfoTextString = FString::Printf(TEXT("%s\n"),*Announcement::TeamsTiedForTheWin);
		InfoTextString.Append(Announcement::RedTeam);
		InfoTextString.Append(FString("\n"));
		InfoTextString.Append(Announcement::BlueTeam);
		InfoTextString.Append(FString("\n"));
	}
	else if(RedTeamScore > BlueTeamScore)
	{
		InfoTextString = Announcement::RedTeamWins;
		InfoTextString.Append(FString("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"),*Announcement::RedTeam,RedTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"),*Announcement::BlueTeam,BlueTeamScore));
	}
	else if(BlueTeamScore > RedTeamScore)
	{
		InfoTextString = Announcement::BlueTeamWins;
		InfoTextString.Append(FString("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"),*Announcement::BlueTeam,BlueTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"),*Announcement::RedTeam,RedTeamScore));
		
	}

	return InfoTextString;
}
//-------------------------------------------------------------------------------------------------------------------------
