#include "ValueGauge.h"
#include "GAS/CAttributeSet.h"
#include "Widgets/OverHeadStatsGauge.h"
#include "Kismet/GameplayStatics.h"

void UOverHeadStatsGauge::ConfigureWithASC(class UAbilitySystemComponent* AbilitySystemComponent)
{
	if (AbilitySystemComponent)
	{
		HealthBar->SetAndBoundToGameplayAttribute(AbilitySystemComponent,UCAttributeSet::GetHealthAttribute(),UCAttributeSet::GetMaxHealthAttribute());
		HealthBar->SetAndBoundToShieldAttribute(AbilitySystemComponent);
		HealthBar->SetShieldFillColor(FLinearColor(1.0f, 0.8f, 0.0f));  // 金色护盾
		ManaBar->SetAndBoundToGameplayAttribute(AbilitySystemComponent,UCAttributeSet::GetManaAttribute(),UCAttributeSet::GetMaxManaAttribute());
	}
}

void UOverHeadStatsGauge::SetBarColorsByTeam(FGenericTeamId OwnerTeamID)
{
	APawn* LocalPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!LocalPawn) return;

	IGenericTeamAgentInterface* LocalTeamAgent = Cast<IGenericTeamAgentInterface>(LocalPawn);
	if (!LocalTeamAgent) return;

	const bool bIsEnemy = LocalTeamAgent->GetGenericTeamId() != OwnerTeamID;

	if (bIsEnemy)
	{
		HealthBar->SetFillColor(FLinearColor::Red);
		ManaBar->SetFillColor(FLinearColor(0.0f, 0.3f, 1.0f));
	}
	else
	{
		HealthBar->SetFillColor(FLinearColor::Green);
		ManaBar->SetFillColor(FLinearColor(0.0f, 0.3f, 1.0f));
	}
}