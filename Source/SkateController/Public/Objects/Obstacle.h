#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Obstacle.generated.h"

UCLASS()
class SKATECONTROLLER_API AObstacle : public AActor
{
	GENERATED_BODY()

public:

	AObstacle();

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Obstacle")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Obstacle")
	class UBoxComponent* CollisionBox;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

protected:

	virtual void BeginPlay() override;


};
