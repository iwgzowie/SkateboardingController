
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreWidget.generated.h"

UCLASS()
class SKATECONTROLLER_API UScoreWidget : public UUserWidget
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "UI")
    void UpdateScore(int32 NewScore);

protected:

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* ScoreText;
};
