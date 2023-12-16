
#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"
#include "Bluster/Character/BlasterCharacter.h"
#include "Bluster/Bluster.h"
//-------------------------------------------------------------------------------------------------------------------------
AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility,ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic,ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh,ECollisionResponse::ECR_Block);

}

//-------------------------------------------------------------------------------------------------------------------------
void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}
	
	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this,&AProjectile::OnHit);
	}

}

//-------------------------------------------------------------------------------------------------------------------------
void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	
	Destroy();
}
//-------------------------------------------------------------------------------------------------------------------------

void AProjectile::SpawnTrailSystem()
{

	if(TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}

}
//-------------------------------------------------------------------------------------------------------------------------

void AProjectile::ExplodeDamage()
{
	APawn *FiringPawn = GetInstigator();
	if(FiringPawn && HasAuthority())
	{
		AController *FiringController = FiringPawn->GetController();
		if(FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,//Word context object
				Damage,//Base Damage
				10.f,//Min damage
				GetActorLocation(),//Origin of the damage radii
				DamageInnerRadius,//DamageInnerRadius
				DamageOuterRadius,//DamageOuterRadius
				1.f,//Damage Falloff
				UDamageType::StaticClass(),//Damage Type Class
				TArray <AActor*>(),//IgnoreActors
				this,//DamageCauser
				FiringController//Instigator Controller
			);
		}
	}

}
//-------------------------------------------------------------------------------------------------------------------------
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
//-------------------------------------------------------------------------------------------------------------------------
void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectile::DestroyTimerFinished,
		DestroyTime
	);
}
//-------------------------------------------------------------------------------------------------------------------------
void AProjectile::DestroyTimerFinished()
{
	Destroy();
}
//-------------------------------------------------------------------------------------------------------------------------

void AProjectile::Destroyed()
{
	Super::Destroyed();

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),ImpactParticles,GetActorTransform());
	}

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this,ImpactSound,GetActorLocation());
	}

}
//-------------------------------------------------------------------------------------------------------------------------
