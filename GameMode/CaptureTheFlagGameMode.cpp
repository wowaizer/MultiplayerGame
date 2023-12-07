#include "CaptureTheFlagGameMode.h"
#include "Bluster/Weapon/Flag.h"
#include "Bluster/CaptureTheFlag/FlagZone.h"
#include "Bluster/GameState/BlasterGameState.h"
//-------------------------------------------------------------------------------------------------------------------------
void ACaptureTheFlagGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	ABlasterGameMode::PlayerEliminated(ElimmedCharacter,VictimController,AttackerController);

}
//-------------------------------------------------------------------------------------------------------------------------
void ACaptureTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* Zone)
{

	bool bValidCapture = Flag->GetTeam() != Zone->Team;
	
	ABlasterGameState *BGameState = Cast<ABlasterGameState>(GameState);
	if(BGameState)
	{
		if(Zone->Team == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();
		}

		if(Zone->Team == ETeam::ET_RedTeam)
		{
			BGameState->RedTeamScores();
		}

	}
}
//-------------------------------------------------------------------------------------------------------------------------
