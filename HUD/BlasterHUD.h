// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"


USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	class UTexture2D *CrosshairsCenter;
	UTexture2D *CrosshairsLeft;
	UTexture2D *CrosshairsRight;
	UTexture2D *CrosshairsTop;
	UTexture2D *CrosshairsBottom;
	float CrosshairSpread;
	FLinearColor CrosshairColor;

};

/**
 * 
 */
UCLASS()
class BLUSTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()

public:

	/**
	*Return to main menu
	*/
	UPROPERTY(EditAnywhere, Category = HUD)
	TSubclassOf<class UUserWidget> ReturnToMainMenuWidgetAction;

	UPROPERTY()
	class UReturnToMainMenu* ReturnToMainMenu;



	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;

	UPROPERTY()
	class UCharacterOverlay *CharacterOverlay;

	void AddCharacterOverlay();

	UPROPERTY(EditAnywhere, Category = "Annonucements")
	TSubclassOf<class UUserWidget> AnnonucementClass;

	UPROPERTY()
	class UAnnonucement *Annonucement;

	void AddAnnonucement();
	void AddElimAnnonucement(FString AttackerName, FString VictimName);

protected:
	virtual void BeginPlay() override;
	

private:
	UPROPERTY()
	class APlayerController *OwningPlayer;

	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D *Texture,FVector2D ViewportCenter,
		FVector2D Spread,FLinearColor CrosshairColor);

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UElimAnnouncement> ElimAnnouncementClass;

	UPROPERTY(EditAnywhere)
	float ElimAnnonucementTime = 2.5f;

	UFUNCTION()
	void ElimAnnouncementTimerFinished(UElimAnnouncement *MsgToRemove);

	UPROPERTY()
	TArray<UElimAnnouncement*> ElimMessages;

public:
	FORCEINLINE void SetHUDPackage (const FHUDPackage &Package) { HUDPackage  = Package; }
};
