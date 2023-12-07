
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMainMenu.generated.h"

/**
 * 
 */
UCLASS()
class BLUSTER_API UReturnToMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:

	void MenuSetup();
	void MenuTearDown();

protected:

	virtual bool Initialize() override;

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessfull);

	UFUNCTION()
	void OnPlayerLeftGame();

private:
	UPROPERTY(meta = (BindWidget))
	class UButton *ReturnButton;

	UFUNCTION()
	void ReturnButtonClicked();

	UPROPERTY()
	class UMultiplayerSessionsSubsystem *MultiplayerSessionsSubsystem;

	UPROPERTY()
	class APlayerController *PlayerController;

};
