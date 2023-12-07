#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class BLUSTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;
	/**
	*Used with server-side rewind
	*/
	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000.f;

	//Only set this for grenades and rockets
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;
	//Doesnt matter for grenades and rockets
	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;

protected:
	
	virtual void BeginPlay() override;
	void StartDestroyTimer ();
	void DestroyTimerFinished();
	void SpawnTrailSystem ();
	void ExplodeDamage();

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);


	UPROPERTY(EditAnywhere)
	class UParticleSystem *ImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue *ImpactSound;

	UPROPERTY(EditAnywhere)
	class UBoxComponent *CollisionBox;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent *ProjectileMovementComponent;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem *TrailSystem;

	UPROPERTY()
	class UNiagaraComponent *TrailSystemComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent *ProjectileMesh;

	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;

	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;
	
private:

	UPROPERTY(EditAnywhere)
	class UParticleSystem *Tracer;

	UPROPERTY()
	class UParticleSystemComponent *TracerComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;

};
