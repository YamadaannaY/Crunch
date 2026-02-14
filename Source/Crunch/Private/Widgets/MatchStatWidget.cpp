// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/MatchStatWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Framework/StormCore.h"
#include "Kismet/GameplayStatics.h"

void UMatchStatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//工作原理是遍历当前世界（World）中所有活动的 Actor，返回它找到的第一个匹配指定类型的实例
	StormCore=Cast<AStormCore>(UGameplayStatics::GetActorOfClass(this,AStormCore::StaticClass()));
	if (StormCore)
	{
		//绑定响应回调，在CountUpdate和GoalReached分别更新控件和处理Match结束时Widget的显示
		StormCore->OnTeamInfluenceCountUpdatedDelegate.AddUObject(this,&ThisClass::UpdateTeamInfluence);
		StormCore->OnGoalReachedDelegate.AddUObject(this,&ThisClass::MatchFinished);

		//设置循环计时器0.5s更新一次Widget
		GetWorld()->GetTimerManager().SetTimer(UpdateProgressTimerHandle,this,&ThisClass::UpdateProgress,ProgressUpdateInterval,true);
	}
	
}

void UMatchStatWidget::UpdateTeamInfluence(int TeamOneCount, int TeamTwoCount)
{
	//更新CountText
	TeamOneCountText->SetText(FText::AsNumber(TeamOneCount));
	TeamTwoCountText->SetText(FText::AsNumber(TeamTwoCount));
}

void UMatchStatWidget::MatchFinished(AActor* ViewTarget, int WinnerTeam)
{
	//获胜时的Progress置为满
	float Progress =WinnerTeam == 0 ? 1: 0;
	GetWorld()->GetTimerManager().ClearTimer(UpdateProgressTimerHandle);

	//最后一次更新手动进行，完全置为1/0
	ProgressImage->GetDynamicMaterial()->SetScalarParameterValue(ProgressDynamicMaterialParaName,Progress);
}

void UMatchStatWidget::UpdateProgress()
{
	if (StormCore)
	{
		//获得TeamOne相对于总长度的比例，赋予DynamicMaterialPara
		float Progress=StormCore->GetProgress();
		ProgressImage->GetDynamicMaterial()->SetScalarParameterValue(ProgressDynamicMaterialParaName,Progress);
	}
}