#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GenericTeamAgentInterface.h"
#include "OverHeadStatsGauge.generated.h"

class UValueGauge;
class UAbilitySystemComponent;

/**
 *
 */
UCLASS()
class UOverHeadStatsGauge : public UUserWidget
{
	GENERATED_BODY()

public:
	//为OverHeadBar调用SetAndBoundToGameplayAttribute，更新Percent和Text
	void ConfigureWithASC(UAbilitySystemComponent* AbilitySystemComponent);

	//根据队伍关系设置血条颜色：友方绿色，敌方红色
	void SetBarColorsByTeam(FGenericTeamId OwnerTeamID);
private:
	UPROPERTY(meta=(BindWidget))
	UValueGauge* HealthBar;
	
	UPROPERTY(meta=(BindWidget))
	UValueGauge* ManaBar;

};

