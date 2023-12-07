// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Annonucement.generated.h"

/**
 * 
 */
UCLASS()
class BLUSTER_API UAnnonucement : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(meta = (BindWidget))
	class UTextBlock *WarmupTime;

	UPROPERTY(meta = (BindWidget))
	UTextBlock *AnnoncementText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock *InfoText;
};
