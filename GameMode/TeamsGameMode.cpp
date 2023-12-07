#include "TeamsGameMode.h"
#include "Bluster/GameState/BlasterGameState.h"
#include "Bluster/PlayerState/BlasterPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Bluster/PlayerController/BlasterPlayerController.h"

//-------------------------------------------------------------------------------------------------------------------------
ATeamsGameMode::ATeamsGameMode()
{
	bTeamsMatch = true;
}

//-------------------------------------------------------------------------------------------------------------------------
void ATeamsGameMode::PostLogin(APlayerController * NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ABlasterGameState *BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if(BGameState)
	{
		ABlasterPlayerState *BPState = NewPlayer->GetPlayerState<ABlasterPlayerState>();
		if(BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
		{
			if(BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
			{
				BGameState->RedTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				BGameState->BlueTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_BlueTeam);
			}
		}

	}
}
//-------------------------------------------------------------------------------------------------------------------------
void ATeamsGameMode::Logout(AController* Exiting)
{

	Super::Logout(Exiting);
	ABlasterGameState *BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	ABlasterPlayerState *BPState = Exiting->GetPlayerState<ABlasterPlayerState>();
	if(BGameState && BPState)
	{
		if(BGameState->RedTeam.Contains(BPState))
		{
			BGameState->RedTeam.Remove(BPState);
		}


		if(BGameState->BlueTeam.Contains(BPState))
		{
			BGameState->BlueTeam.Remove(BPState);
		}


		/*if(BPState->GetTeam() == ETeam::ET_RedTeam)
		{
		BGameState->RedTeam.Remove(BPState);
		}
		if(BPState->GetTeam() == ETeam::ET_BlueTeam)
		{
		BGameState->BlueTeam.Remove(BPState);
		}*/		

	}
}
//-------------------------------------------------------------------------------------------------------------------------
float ATeamsGameMode::CalculateDamage(AController* Atacker, AController* Victim, float BaseDamage)
{
	ABlasterPlayerState *AtackerPState = Atacker->GetPlayerState<ABlasterPlayerState>();
	ABlasterPlayerState *VictimPState = Victim->GetPlayerState<ABlasterPlayerState>();

	if(AtackerPState == nullptr || VictimPState == nullptr) return BaseDamage;

	if(VictimPState == AtackerPState) return BaseDamage;

	if(AtackerPState->GetTeam() == VictimPState->GetTeam())
	{
		return 0.f;
	}
	return BaseDamage;

}
//-------------------------------------------------------------------------------------------------------------------------
void ATeamsGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	Super::PlayerEliminated(ElimmedCharacter,VictimController,AttackerController);

	ABlasterGameState *BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	ABlasterPlayerState *AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
		
	if(BGameState && AttackerPlayerState)
	{
		if(AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			BGameState->RedTeamScores();
		}
		if(AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();
		}

	}

}
//-------------------------------------------------------------------------------------------------------------------------
void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	ABlasterGameState *BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if(BGameState)
	{
		for(auto PState : BGameState->PlayerArray)
		{
			ABlasterPlayerState *BPState = Cast<ABlasterPlayerState>(PState.Get());
			if(BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
			{
				if(BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
				{
					BGameState->RedTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					BGameState->BlueTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}
//-------------------------------------------------------------------------------------------------------------------------


