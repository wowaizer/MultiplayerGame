// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "Bluster/Character/BlasterCharacter.h"
#include "Bluster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Bluster/PlayerState/BlasterPlayerState.h"
#include "Bluster/GameState/BlasterGameState.h"
#include "MultiplayerSessionsSubsystem.h"


namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}
//-------------------------------------------------------------------------------------------------------------------------
ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;

}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();
	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

//-------------------------------------------------------------------------------------------------------------------------
void ABlasterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(MatchState == MatchState::WaitingToStart)
	{

		ClientsLeftGame = false;


		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;

		if(CountdownTime <= 0.f)
		{
			StartMatch();
		}
	
	}
	else if (MatchState == MatchState::InProgress)
	{
		ClientsLeftGame = false;
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if(CountdownTime <= 0.f)
		{

			
			SetMatchState(MatchState::Cooldown);

		}
	}
			
	
	else if (MatchState == MatchState::Cooldown)
	{
		
		ClientsLeftGame = false;
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if(CountdownTime <= 0.f)
		{
			ClientsLeftGame = true;

			for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{

				ABlasterPlayerController *BlasterPlayer = Cast<ABlasterPlayerController>(*It);
				if(BlasterPlayer)
				{
					BlasterPlayer->ClientReturnToMainMenu(FString());
			
				}
				
			}

		  /* RestartGame();*/
			
			
		}
	}
	
	
	
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController *BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		if(BlasterPlayer)
		{
			BlasterPlayer->OnMatchStateSet(MatchState, bTeamsMatch);
		}
	}
	
}
//-------------------------------------------------------------------------------------------------------------------------
float ABlasterGameMode::CalculateDamage(AController* Atacker, AController* Victim, float BaseDamage)
{
	return BaseDamage;
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{

	ABlasterPlayerState *AtackerPlayerState = AttackerController ? 
		Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	
	ABlasterPlayerState *VictimPlayerState = VictimController ? 
		Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;


	ABlasterGameState *BlasterGameState = GetGameState<ABlasterGameState>();

	if(AtackerPlayerState && AtackerPlayerState != VictimPlayerState && BlasterGameState)
	{
		TArray<ABlasterPlayerState*> PlayersCurrentlyInTheLead;
		for (auto LeadPlayer : BlasterGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}

		AtackerPlayerState->AddToScore(1.0f);
		BlasterGameState->UpdateTopScore(AtackerPlayerState);

		if(BlasterGameState->TopScoringPlayers.Contains(AtackerPlayerState))
		{
			ABlasterCharacter *Leader = Cast<ABlasterCharacter>(AtackerPlayerState->GetPawn());
			if(Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}

		for(int32 i = 0; i < PlayersCurrentlyInTheLead.Num(); i++)
		{
			if(!BlasterGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))
			{
				ABlasterCharacter *Loser = Cast<ABlasterCharacter>(PlayersCurrentlyInTheLead[i]->GetPawn());
				if(Loser)
				{
					Loser->MulticastLostTheLead();
				}
			}
		}
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	if (ElimmedCharacter)
	{

		ElimmedCharacter->Elim(false);
	}

	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController *BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		if(BlasterPlayer && AtackerPlayerState && VictimPlayerState)
		{
			BlasterPlayer->BroadcastElim(AtackerPlayerState,VictimPlayerState);
		}
	}

}

void ABlasterGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}

	if (ElimmedCharacter)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this,APlayerStart::StaticClass(),PlayerStarts);
		int32 Selection = FMath::RandRange(0,PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController,PlayerStarts[Selection]);
		
	}
}
//-------------------------------------------------------------------------------------------------------------------------
void ABlasterGameMode::PlayerLeftGame(ABlasterPlayerState *PlayerLeaving)
{
	if(PlayerLeaving == nullptr) return;

	ABlasterGameState *BlasterGameState = GetGameState<ABlasterGameState>();
	if(BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		BlasterGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	ABlasterCharacter *CharacterLeaving = Cast<ABlasterCharacter>(PlayerLeaving->GetPawn());
	if(CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}
//-------------------------------------------------------------------------------------------------------------------------
