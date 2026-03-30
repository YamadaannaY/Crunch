// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MatchStatWidget.generated.h"

/**
 * 
 */
UCLASS()
class UMatchStatWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
private:
	UPROPERTY(meta=(BindWidget))
	class UImage* ProgressImage;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* TeamOneCountText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* TeamTwoCountText;

	UPROPERTY()
	class AStormCore* StormCore;

	UPROPERTY(EditDefaultsOnly,Category="Match Stat");
	float ProgressUpdateInterval=0.5f;

	UPROPERTY(EditDefaultsOnly,Category="Match Stat");
	FName ProgressDynamicMaterialParaName="Progress";
	
	//更新Team人数文本
	void UpdateTeamInfluence(int TeamOneCount ,int TeamTwoCount);
	
	//比赛完成时处理定时器，最后设置一次进度条值以更好显示
	void MatchFinished(AActor* ViewTarget,int WinnerTeam);

	FTimerHandle UpdateProgressTimerHandle;
	
	//循环定时器0.5s更新一次进度条
	void UpdateProgress();
};
