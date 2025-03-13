

#include "UI/ScoreWidget.h"
#include "Components/TextBlock.h"

void UScoreWidget::UpdateScore(int32 NewScore)
{
    if (ScoreText)
    {
        ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), NewScore)));
    }
}
