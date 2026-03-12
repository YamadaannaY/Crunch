#include "Widgets/LevelGauge.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/CHeroAttributeSet.h"

void ULevelGauge::NativeConstruct()
{
	Super::NativeConstruct();

	NumberFormattingOptions.SetMaximumFractionalDigits(0);

	APawn* OwnerPawn=GetOwningPlayerPawn();

	if (!OwnerPawn) return ;

	UAbilitySystemComponent* OwnerAbilitySystemComp=UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerPawn);
	if (!OwnerAbilitySystemComp) return ;
	OwnerASC=OwnerAbilitySystemComp;

	//先初始化一次，为游戏最开始的LevelUI
	UpdateGauge(FOnAttributeChangeData());

	//绑定经验值有关的属性值，一个回调函数统一进行UI处理
	OwnerAbilitySystemComp->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetExperienceAttribute()).AddUObject(this,&ThisClass::UpdateGauge);
	OwnerAbilitySystemComp->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetNextLevelExperienceAttribute()).AddUObject(this,&ThisClass::UpdateGauge);
	OwnerAbilitySystemComp->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetPrevLevelExperienceAttribute()).AddUObject(this,&ThisClass::UpdateGauge);
	OwnerAbilitySystemComp->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetLevelAttribute()).AddUObject(this,&ThisClass::UpdateGauge);
}

void ULevelGauge::UpdateGauge(const FOnAttributeChangeData& Data)
{
	bool bFound;

	float CurrentExperience=OwnerASC->GetGameplayAttributeValue(UCHeroAttributeSet::GetExperienceAttribute(),bFound);
	if (!bFound) return ;

	const float NextLevelExperience=OwnerASC->GetGameplayAttributeValue(UCHeroAttributeSet::GetNextLevelExperienceAttribute(),bFound);
	if (!bFound) return ;

	const float PrevLevelExperience=OwnerASC->GetGameplayAttributeValue(UCHeroAttributeSet::GetPrevLevelExperienceAttribute(),bFound);
	if (!bFound) return ;

	const float CurrentLevel=OwnerASC->GetGameplayAttributeValue(UCHeroAttributeSet::GetLevelAttribute(),bFound);
	if (!bFound) return ;

	LevelText->SetText(FText::AsNumber(CurrentLevel,&NumberFormattingOptions));

	//升级需要的经验值 && 基于当前等级多出的经验值
	const float LevelExpAmt=NextLevelExperience-PrevLevelExperience;
	const float Progress=CurrentExperience-PrevLevelExperience;
	float Percent=Progress/LevelExpAmt;

	if (NextLevelExperience==0)
	{
		Percent=1;
	}

	if (LevelProgressImage)
	{
		LevelProgressImage->GetDynamicMaterial()->SetScalarParameterValue(PercentMaterialParamName,Percent);
	}
}