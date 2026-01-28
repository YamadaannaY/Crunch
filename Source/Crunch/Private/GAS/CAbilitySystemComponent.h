// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CGameplayAbilitiesType.h"
#include "GameplayEffectTypes.h"
#include "CAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class UCAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	//GAS构造函数常用于绑定对Attribute的ChangeDelegate
	UCAbilitySystemComponent();

	//找到DT中对应ASC拥有者的Class并获取其数据，为属性集赋值
	void InitializeBaseAttribute();

	//在CPlayerController的Possess函数中调用，即在服务端实现ASC的Init,对于AIC，在Character类的PossessedBy中专门判断调用
	void ServerSideInit();

	//遍历InitGE并应用
	void ApplyFullStatsEffectSelf();

	//获得Character特有的GA，不包括BasicGA
	const TMap<ECAbilityInputID,TSubclassOf<UGameplayAbility>>& GetAbilities() const ;

	//判断当前是否处于MaxLevel
	bool IsAtMaxLevel() const;

	//服务端调用RPC，在触发Upgrade的IA输入时执行技能升级逻辑，由于IA是本地的，所以需要一个RPC使得这个函数在服务端调用。
	UFUNCTION(Server,Reliable,WithValidation)
	void Server_UpgradeAbilityWithID(ECAbilityInputID InputID);

	//客户端接收改变后的GA信息，用于修改UI
	UFUNCTION(Client,Reliable)
	void Client_AbilitySpecLevelUpdated(FGameplayAbilitySpecHandle Handle,int Level);
private:
	//应用初始GE
	void ApplyInitialEffects();

	//对所有GA调用GiveAbility注册到ASC
	void GiveInitialAbilities();

	//将GE应用给自身的调用函数
	void AuthApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect>GameplayEffect,int Level=1);

	//监听Health、Mana属性值，变化时调用函数
	void HealthUpdated(const FOnAttributeChangeData& ChangeData);
	void ManaUpdated(const FOnAttributeChangeData& ChangeData);
	void ExperienceUpdated(const FOnAttributeChangeData& ChangeData);
	
	/***********	GA会带有一个ID，激活能力时依据ID找到要激活的GA，其中PassiveGA不需要ID，直接用None，表示不能主动触发	****************/

	//技能
	UPROPERTY(EditDefaultsOnly,Category="Gameplay Abilities")
	TMap<ECAbilityInputID,TSubclassOf<UGameplayAbility>> Abilities;

	//角色的ASC直接具有的基础能力
	UPROPERTY(EditDefaultsOnly,Category="Gameplay Abilities")
	TMap<ECAbilityInputID,TSubclassOf<UGameplayAbility>> BasicAbilities;

	//找到ASC中绑定好的PAGeneric,存储了每个Hero都会具有的InitGA和InitGE，AttrDT,LevelCT等
	//为了加强复用性用一个PA存储起来，创建新角色直接赋予这个PA。
	UPROPERTY(EditDefaultsOnly,Category="Gameplay Abilities")
	class UPA_AbilitySystemGeneric* AbilitySystemGeneric;
};
