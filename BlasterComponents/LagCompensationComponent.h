#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

		UPROPERTY()
		FVector Location;

	UPROPERTY()
		FRotator Rotation;

	UPROPERTY()
		FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName,FBoxInformation> HitBoxInfo;

	UPROPERTY()
	ABlasterCharacter *Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewinResult 
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult 
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<ABlasterCharacter*,uint32> HeadShots;
	UPROPERTY()
	TMap<ABlasterCharacter*,uint32> BodyShots;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLUSTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	friend class ABlasterCharacter;
	void ShowFramePackage (const FFramePackage &Package,const FColor Color);

	//HitScan
	FServerSideRewinResult ServerSideRewind(class ABlasterCharacter *HitCharacter,const FVector_NetQuantize &TraceStart,
		const FVector_NetQuantize &HitLocation,float HitTime);

	//Projectile
	FServerSideRewinResult ProjectileServerSideRewind(ABlasterCharacter *HitCharacter,
		const FVector_NetQuantize &TraceStart,
		const FVector_NetQuantize100 &InitialVelocity,
		float HitTime);


	//Shotgun
	FShotgunServerSideRewindResult ShotgunServerSideRewind(const TArray<ABlasterCharacter*> &HitCharacters,
		const FVector_NetQuantize &TraceStart,
		const TArray<FVector_NetQuantize> &HitLocations,
		float HitTime);

	UFUNCTION(Server,Reliable)
	void ServerScoreRequest(
		class ABlasterCharacter *HitCharacter,
		const FVector_NetQuantize &TraceStart,
		const FVector_NetQuantize &HitLocation,
		float HitTime
	);

	UFUNCTION(Server,Reliable)
		void ProjectileServerScoreRequest(
			class ABlasterCharacter *HitCharacter,
			const FVector_NetQuantize &TraceStart,
			const FVector_NetQuantize100 &InitialVelocity,
			float HitTime
		);


	UFUNCTION(Server,Reliable)
		void ShotgunServerScoreRequest(
			const TArray<ABlasterCharacter*> &HitCharacters,
			const FVector_NetQuantize &TraceStart,
			const TArray<FVector_NetQuantize> &HitLocations,
			float HitTime
		);

protected:
	virtual void BeginPlay() override;

	void SaveFramePackage(FFramePackage &Package);
	FFramePackage InterpBetweenFrames(const FFramePackage &OlderFrame,const FFramePackage &YongerFrame,float HitTime);

	void CacheBoxPositions(ABlasterCharacter *HitCharacter,FFramePackage &OutFramePackage);
	void MoveBoxes(ABlasterCharacter *HitCharacter, const FFramePackage &Package);
	void ResetMoveBoxes(ABlasterCharacter *HitCharacter, const FFramePackage &Package);
	void EnableCharacterMeshCollision(ABlasterCharacter *HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	void SaveFramePackage();
	FFramePackage GetFrameToCheck(ABlasterCharacter *HitCharacter,float HitTime);

	/**
	* HitScan
	*/
	FServerSideRewinResult ConfirmedHit(const FFramePackage &Package,ABlasterCharacter *HitCharacter,
		const FVector_NetQuantize &TraceStart,const FVector_NetQuantize &HitLocation);

	/**
	* Projectile
	*/
	FServerSideRewinResult ProjectileConfirmedHit(const FFramePackage &Package,ABlasterCharacter *HitCharacter,
		const FVector_NetQuantize &TraceStart,const FVector_NetQuantize100& InitialVelocity, float HitTime);

	/**
	* Shotgun
	*/

	FShotgunServerSideRewindResult ShotgunConfirmedHit(const TArray<FFramePackage>&FramePackages,
		const FVector_NetQuantize &TraceStart,
		const TArray<FVector_NetQuantize> &HitLocations);

private:
	UPROPERTY()
	ABlasterCharacter *Character;

	UPROPERTY()
	class ABlasterPlayerController *Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;
};
