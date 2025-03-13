#include "Objects/Obstacle.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/SkateControllerCharacter.h"

AObstacle::AObstacle()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Overlap);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetBoxExtent(FVector(50, 50, 50));
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Overlap);
	CollisionBox->SetGenerateOverlapEvents(true);

	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AObstacle::OnOverlapBegin);
}

void AObstacle::BeginPlay()
{
	Super::BeginPlay();
}

void AObstacle::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	ASkateControllerCharacter* SkateChar = Cast<ASkateControllerCharacter>(OtherActor);
	if (SkateChar && SkateChar->GetCharacterMovement()->IsFalling())
	{
		SkateChar->AddScore(10);
		UE_LOG(LogTemp, Warning, TEXT("Obstacle passed, awarded 10 points."));
	}
}

